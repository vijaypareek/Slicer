/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Csaba Pinter, PerkLab, Queen's University
  and was supported through the Applied Cancer Research Unit program of Cancer Care
  Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care

==============================================================================*/

// Terminologies includes
#include "qSlicerTerminologyNavigatorWidget.h"

#include "ui_qSlicerTerminologyNavigatorWidget.h"

#include "vtkSlicerTerminologiesModuleLogic.h"
#include "vtkSlicerTerminologyEntry.h"

// Slicer includes
#include <qSlicerCoreApplication.h>
#include <qSlicerModuleManager.h>
#include <qSlicerAbstractCoreModule.h>

// VTK includes
#include <vtkSmartPointer.h>

// Qt includes
#include <QDebug>
#include <QTableWidgetItem>
#include <QColor>

//-----------------------------------------------------------------------------
class qSlicerTerminologyNavigatorWidgetPrivate: public Ui_qSlicerTerminologyNavigatorWidget
{
  Q_DECLARE_PUBLIC(qSlicerTerminologyNavigatorWidget);

protected:
  qSlicerTerminologyNavigatorWidget* const q_ptr;
public:
  qSlicerTerminologyNavigatorWidgetPrivate(qSlicerTerminologyNavigatorWidget& object);
  ~qSlicerTerminologyNavigatorWidgetPrivate();
  void init();

  /// Get terminology module logic
  static vtkSlicerTerminologiesModuleLogic* terminologyLogic();

  /// Reset current category name and container object
  void resetCurrentCategory();
  /// Reset current type name and container object
  void resetCurrentType();
  /// Reset current type modifier name and container object
  void resetCurrentTypeModifier();

  // Set recommended color from current selection to color picker
  // Note: will only set it if does not contain modifiers, because in that case it does not include
  //   recommended RGB color member. If modifier is selected then the color will be set from that.
  void setRecommendedColorFromCurrentTerminology();

  /// Reset current region name and container object
  void resetCurrentRegion();
  /// Reset current region modifier name and container object
  void resetCurrentRegionModifier();

  /// Find item in category table widget corresponding to a given category
  QTableWidgetItem* findTableWidgetItemForCategory(vtkSlicerTerminologyCategory* category);
  /// Find item in (type or region) table widget corresponding to a given type
  QTableWidgetItem* findTableWidgetItemForType(QTableWidget* tableWidget, vtkSlicerTerminologyType* type);
  /// Find index in (type or region) modifier combobox corresponding to a given modifier
  /// \return -1 if not found
  int findComboBoxIndexForModifier(ctkComboBox* comboBox, vtkSlicerTerminologyType* modifier);

public:
  /// Name (SegmentationCategoryTypeContextName) of the current terminology
  QString CurrentTerminologyName;

  /// Object containing the details of the current category
  vtkSlicerTerminologyCategory* CurrentCategoryObject;
  /// Object containing the details of the current type
  vtkSlicerTerminologyType* CurrentTypeObject;
  /// Object containing the details of the current type modifier if any
  vtkSlicerTerminologyType* CurrentTypeModifierObject;

  /// Name (AnatomicContextName) of the current anatomic context
  QString CurrentAnatomicContextName;

  /// Object containing the details of the current region
  vtkSlicerTerminologyType* CurrentRegionObject;
  /// Object containing the details of the current region modifier if any
  vtkSlicerTerminologyType* CurrentRegionModifierObject;

  /// Custom color selected by the user
  QColor CustomColor;
};

//-----------------------------------------------------------------------------
qSlicerTerminologyNavigatorWidgetPrivate::qSlicerTerminologyNavigatorWidgetPrivate(qSlicerTerminologyNavigatorWidget& object)
  : q_ptr(&object)
{
  this->CurrentCategoryObject = vtkSlicerTerminologyCategory::New();
  this->CurrentTypeObject = vtkSlicerTerminologyType::New();
  this->CurrentTypeModifierObject = vtkSlicerTerminologyType::New();

  this->CurrentRegionObject = vtkSlicerTerminologyType::New();
  this->CurrentRegionModifierObject = vtkSlicerTerminologyType::New();
}

//-----------------------------------------------------------------------------
qSlicerTerminologyNavigatorWidgetPrivate::~qSlicerTerminologyNavigatorWidgetPrivate()
{
  if (this->CurrentCategoryObject)
    {
    this->CurrentCategoryObject->Delete();
    this->CurrentCategoryObject = NULL;
    }
  if (this->CurrentTypeObject)
    {
    this->CurrentTypeObject->Delete();
    this->CurrentTypeObject = NULL;
    }
  if (this->CurrentTypeModifierObject)
    {
    this->CurrentTypeModifierObject->Delete();
    this->CurrentTypeModifierObject = NULL;
    }

  if (this->CurrentRegionObject)
    {
    this->CurrentRegionObject->Delete();
    this->CurrentRegionObject = NULL;
    }
  if (this->CurrentRegionModifierObject)
    {
    this->CurrentRegionModifierObject->Delete();
    this->CurrentRegionModifierObject = NULL;
    }
}

//-----------------------------------------------------------------------------
void qSlicerTerminologyNavigatorWidgetPrivate::init()
{
  Q_Q(qSlicerTerminologyNavigatorWidget);
  this->setupUi(q);

  // Make connections
  QObject::connect(this->ComboBox_Terminology, SIGNAL(currentIndexChanged(int)),
    q, SLOT(onTerminologySelectionChanged(int)) );
  QObject::connect(this->tableWidget_Category, SIGNAL(itemClicked(QTableWidgetItem*)),
    q, SLOT(onCategoryClicked(QTableWidgetItem*)) );
  QObject::connect(this->tableWidget_Type, SIGNAL(itemClicked(QTableWidgetItem*)),
    q, SLOT(onTypeClicked(QTableWidgetItem*)) );
  QObject::connect(this->ComboBox_TypeModifier, SIGNAL(currentIndexChanged(int)),
    q, SLOT(onTypeModifierSelectionChanged(int)) );
  QObject::connect(this->SearchBox_Category, SIGNAL(textChanged(QString)),
    q, SLOT(onCategorySearchTextChanged(QString)) );
  QObject::connect(this->SearchBox_Type, SIGNAL(textChanged(QString)),
    q, SLOT(onTypeSearchTextChanged(QString)) );

  QObject::connect(this->ComboBox_AnatomicContext, SIGNAL(currentIndexChanged(int)),
    q, SLOT(onAnatomicContextSelectionChanged(int)) );
  QObject::connect(this->tableWidget_AnatomicRegion, SIGNAL(itemClicked(QTableWidgetItem*)),
    q, SLOT(onRegionClicked(QTableWidgetItem*)) );
  QObject::connect(this->ComboBox_AnatomicRegionModifier, SIGNAL(currentIndexChanged(int)),
    q, SLOT(onRegionModifierSelectionChanged(int)) );
  QObject::connect(this->SearchBox_AnatomicRegion, SIGNAL(textChanged(QString)),
    q, SLOT(onRegionSearchTextChanged(QString)) );

  QObject::connect(this->ColorPickerButton_RecommendedRGB, SIGNAL(colorChanged(QColor)),
    q, SLOT(onColorChanged(QColor)) );

  // Set default settings for widgets
  this->tableWidget_Category->setEnabled(false);
  this->SearchBox_Category->setEnabled(false);
  this->tableWidget_Type->setEnabled(false);
  this->SearchBox_Type->setEnabled(false);
  this->ComboBox_TypeModifier->setEnabled(false);
  this->ColorPickerButton_RecommendedRGB->setEnabled(false);

  this->SearchBox_AnatomicRegion->setEnabled(false);
  this->tableWidget_AnatomicRegion->setEnabled(false);
  this->ComboBox_AnatomicRegionModifier->setEnabled(false);

  // Use the CTK color picker
  ctkColorPickerButton::ColorDialogOptions options = ctkColorPickerButton::UseCTKColorDialog;
  this->ColorPickerButton_RecommendedRGB->setDialogOptions(options);

  // Populate terminology combobox with the loaded terminologies
  q->populateTerminologyComboBox();
  // Populate anatomic context combobox with the loaded anatomic contexts
  q->populateAnatomicContextComboBox();
}

//-----------------------------------------------------------------------------
vtkSlicerTerminologiesModuleLogic* qSlicerTerminologyNavigatorWidgetPrivate::terminologyLogic()
{
  if (!qSlicerCoreApplication::application()
    || !qSlicerCoreApplication::application()->moduleManager())
    {
    qCritical() << Q_FUNC_INFO << ": Module manager is not found";
    return NULL;
    }
  qSlicerAbstractCoreModule* terminologiesModule = qSlicerCoreApplication::application()->moduleManager()->module("Terminologies");
  if (!terminologiesModule)
    {
    return NULL; // No error log because it makes test fail
    }
  vtkSlicerTerminologiesModuleLogic* terminologyLogic =
    vtkSlicerTerminologiesModuleLogic::SafeDownCast(terminologiesModule->logic());
  if (!terminologyLogic)
    {
    qCritical() << Q_FUNC_INFO << ": Terminologies module logic is invalid";
    }
  return terminologyLogic;
}

//-----------------------------------------------------------------------------
void qSlicerTerminologyNavigatorWidgetPrivate::resetCurrentCategory()
{
  if (this->CurrentCategoryObject)
    {
    this->CurrentCategoryObject->Delete();
    this->CurrentCategoryObject = NULL;
    }
  this->CurrentCategoryObject = vtkSlicerTerminologyCategory::New();
}

//-----------------------------------------------------------------------------
void qSlicerTerminologyNavigatorWidgetPrivate::resetCurrentType()
{
  if (this->CurrentTypeObject)
    {
    this->CurrentTypeObject->Delete();
    this->CurrentTypeObject = NULL;
    }
  this->CurrentTypeObject = vtkSlicerTerminologyType::New();
}

//-----------------------------------------------------------------------------
void qSlicerTerminologyNavigatorWidgetPrivate::resetCurrentTypeModifier()
{
  if (this->CurrentTypeModifierObject)
    {
    this->CurrentTypeModifierObject->Delete();
    this->CurrentTypeModifierObject = NULL;
    }
  this->CurrentTypeModifierObject = vtkSlicerTerminologyType::New();
}

//-----------------------------------------------------------------------------
void qSlicerTerminologyNavigatorWidgetPrivate::setRecommendedColorFromCurrentTerminology()
{
  // Set 'invalid' gray color if type is not selected,
  // or the selected type has modifiers but no modifier is selected
  unsigned char r=127, g=127, b=127;
  if ( !this->CurrentTypeObject ||
       (this->CurrentTypeObject->GetHasModifiers() && !this->CurrentTypeModifierObject) )
    {
    this->ColorPickerButton_RecommendedRGB->blockSignals(true); // The callback function is to save the user's custom color selection
    this->ColorPickerButton_RecommendedRGB->setColor(QColor(r,g,b));
    this->ColorPickerButton_RecommendedRGB->setEnabled(false);
    return;
    }

  // Valid color is present, enable color picker
  this->ColorPickerButton_RecommendedRGB->setEnabled(true);
  // Clear custom color
  this->CustomColor = QColor();

  // If the current type has no modifiers then set color form the type
  if (!this->CurrentTypeObject->GetHasModifiers())
    {
    this->CurrentTypeObject->GetRecommendedDisplayRGBValue(r,g,b);
    }
  else
    {
    this->CurrentTypeModifierObject->GetRecommendedDisplayRGBValue(r,g,b);
    }
  this->ColorPickerButton_RecommendedRGB->blockSignals(true); // The callback function is to save the user's custom color selection
  this->ColorPickerButton_RecommendedRGB->setColor(QColor(r,g,b));
  this->ColorPickerButton_RecommendedRGB->blockSignals(false);
}

//-----------------------------------------------------------------------------
void qSlicerTerminologyNavigatorWidgetPrivate::resetCurrentRegion()
{
  if (this->CurrentRegionObject)
    {
    this->CurrentRegionObject->Delete();
    this->CurrentRegionObject = NULL;
    }
  this->CurrentRegionObject = vtkSlicerTerminologyType::New();
}

//-----------------------------------------------------------------------------
void qSlicerTerminologyNavigatorWidgetPrivate::resetCurrentRegionModifier()
{
  if (this->CurrentRegionModifierObject)
    {
    this->CurrentRegionModifierObject->Delete();
    this->CurrentRegionModifierObject = NULL;
    }
  this->CurrentRegionModifierObject = vtkSlicerTerminologyType::New();
}

//-----------------------------------------------------------------------------
QTableWidgetItem* qSlicerTerminologyNavigatorWidgetPrivate::findTableWidgetItemForCategory(vtkSlicerTerminologyCategory* category)
{
  if (!category)
    {
    return NULL;
    }

  QString categoryName(category->GetCodeMeaning());
  Qt::MatchFlags flags = Qt::MatchExactly | Qt::MatchCaseSensitive;
  QList<QTableWidgetItem*> items = this->tableWidget_Category->findItems(categoryName, flags);
  if (items.count() == 0)
    {
    return NULL;
    }

  foreach (QTableWidgetItem* item, items)
    {
    QString codingSchemeDesignator = item->data(qSlicerTerminologyNavigatorWidget::CodingSchemeDesignatorRole).toString();
    QString codeValue = item->data(qSlicerTerminologyNavigatorWidget::CodeValueRole).toString();
    if ( category->GetCodingScheme() && !codingSchemeDesignator.compare(category->GetCodingScheme())
      && category->GetCodeValue() && !codeValue.compare(category->GetCodeValue()) )
      {
      return item;
      }
    }

    return NULL;
}

//-----------------------------------------------------------------------------
QTableWidgetItem* qSlicerTerminologyNavigatorWidgetPrivate::findTableWidgetItemForType(QTableWidget* tableWidget, vtkSlicerTerminologyType* type)
{
  if (!tableWidget || !type)
    {
    return NULL;
    }

  QString typeName(type->GetCodeMeaning());
  Qt::MatchFlags flags = Qt::MatchExactly | Qt::MatchCaseSensitive;
  QList<QTableWidgetItem*> items = tableWidget->findItems(typeName, flags);
  if (items.count() == 0)
    {
    return NULL;
    }

  foreach (QTableWidgetItem* item, items)
    {
    QString codingSchemeDesignator = item->data(qSlicerTerminologyNavigatorWidget::CodingSchemeDesignatorRole).toString();
    QString codeValue = item->data(qSlicerTerminologyNavigatorWidget::CodeValueRole).toString();
    if ( type->GetCodingScheme() && !codingSchemeDesignator.compare(type->GetCodingScheme())
      && type->GetCodeValue() && !codeValue.compare(type->GetCodeValue()) )
      {
      return item;
      }
    }

    return NULL;
}

//-----------------------------------------------------------------------------
int qSlicerTerminologyNavigatorWidgetPrivate::findComboBoxIndexForModifier(ctkComboBox* comboBox, vtkSlicerTerminologyType* modifier)
{
  if (!comboBox || !modifier)
    {
    return -1;
    }

  QString modifierName(modifier->GetCodeMeaning());
  int modifierIndex = comboBox->findText(modifierName);
  return modifierIndex;
}

//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// qSlicerTerminologyNavigatorWidget methods

//-----------------------------------------------------------------------------
qSlicerTerminologyNavigatorWidget::qSlicerTerminologyNavigatorWidget(QWidget* _parent)
  : qMRMLWidget(_parent)
  , d_ptr(new qSlicerTerminologyNavigatorWidgetPrivate(*this))
{
  Q_D(qSlicerTerminologyNavigatorWidget);
  d->init();

  // Connect logic modified event (cannot call QVTK from private implementation)
  vtkSlicerTerminologiesModuleLogic* logic = qSlicerTerminologyNavigatorWidgetPrivate::terminologyLogic();
  if (logic)
    {
    qvtkConnect( logic, vtkCommand::ModifiedEvent, this, SLOT(onLogicModified()) );
    }
}

//-----------------------------------------------------------------------------
qSlicerTerminologyNavigatorWidget::~qSlicerTerminologyNavigatorWidget()
{
}

//-----------------------------------------------------------------------------
bool qSlicerTerminologyNavigatorWidget::terminologyEntry(vtkSlicerTerminologyEntry* entry)
{
  Q_D(qSlicerTerminologyNavigatorWidget);

  if (!entry)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid terminology entry object";
    return false;
    }

  // Terminology name
  if (d->CurrentTerminologyName.isEmpty())
    {
    qCritical() << Q_FUNC_INFO << ": No terminology selected";
    return false;
    }
  entry->SetTerminologyContextName(d->CurrentTerminologyName.toLatin1().constData());

  // Terminology category
  if (!d->CurrentCategoryObject)
    {
    qCritical() << Q_FUNC_INFO << ": No terminology category selected";
    return false;
    }
  entry->SetCategoryObject(d->CurrentCategoryObject);

  // Terminology type
  if (!d->CurrentTypeObject)
    {
    qCritical() << Q_FUNC_INFO << ": No terminology type selected";
    return false;
    }
  entry->SetTypeObject(d->CurrentTypeObject);

  // Terminology type modifier
  if (d->CurrentTypeModifierObject)
    {
    entry->SetTypeModifierObject(d->CurrentTypeModifierObject);
    }

  // Anatomic context name
  if (!d->CurrentAnatomicContextName.isEmpty())
    {
    entry->SetAnatomicContextName(d->CurrentAnatomicContextName.toLatin1().constData());
    }

  // Anatomic region
  if (d->CurrentRegionObject)
    {
    entry->SetAnatomicRegionObject(d->CurrentRegionObject);
    }

  // Anatomic region modifier
  if (d->CurrentRegionModifierObject)
    {
    entry->SetAnatomicRegionModifierObject(d->CurrentRegionModifierObject);
    }

  return true;
}

//-----------------------------------------------------------------------------
bool qSlicerTerminologyNavigatorWidget::setTerminologyEntry(vtkSlicerTerminologyEntry* entry)
{
  Q_D(qSlicerTerminologyNavigatorWidget);

  if (!entry)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid terminology entry object";
    return false;
    }

  // Select terminology
  char* terminologyContextName = entry->GetTerminologyContextName();
  if (!terminologyContextName)
    {
    return false; // The terminology is not invalid but empty
    }
  int terminologyIndex = d->ComboBox_Terminology->findText(terminologyContextName);
  if (terminologyIndex == -1)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to find terminology with context name " << terminologyContextName;
    return false;
    }
  if (terminologyIndex != d->ComboBox_Terminology->currentIndex())
    {
    this->setCurrentTerminology(d->ComboBox_Terminology->itemText(terminologyIndex));
    }
  d->ComboBox_Terminology->blockSignals(true);
  d->ComboBox_Terminology->setCurrentIndex(terminologyIndex);
  d->ComboBox_Terminology->blockSignals(false);

  // Select category
  vtkSlicerTerminologyCategory* categoryObject = entry->GetCategoryObject();
  if (!categoryObject)
    {
    return false; // The terminology is not invalid but empty
    }
  bool returnValue = true;
  if (!this->setCurrentCategory(categoryObject))
    {
    qCritical() << Q_FUNC_INFO << ": Failed to find category with name " << (categoryObject->GetCodeMeaning()?categoryObject->GetCodeMeaning():"NULL");
    returnValue = false;
    }

  // Select type
  vtkSlicerTerminologyType* typeObject = entry->GetTypeObject();
  if (!typeObject)
    {
    qCritical() << Q_FUNC_INFO << ": No type object in terminology entry";
    returnValue = false;
    }
  else if (!this->setCurrentType(typeObject))
    {
    qCritical() << Q_FUNC_INFO << ": Failed to find type with name " << (typeObject->GetCodeMeaning()?typeObject->GetCodeMeaning():"NULL");
    returnValue = false;
    }

  // Select type modifier
  vtkSlicerTerminologyType* typeModifierObject = entry->GetTypeModifierObject();
  if (typeObject && typeObject->GetHasModifiers() && typeModifierObject)
    {
    if (!this->setCurrentTypeModifier(typeModifierObject))
      {
      qCritical() << Q_FUNC_INFO << ": Failed to find type modifier with name " << (typeModifierObject->GetCodeMeaning()?typeModifierObject->GetCodeMeaning():"NULL");
      returnValue = false;
      }
    }

  // Set anatomic context selection if category allows
  if (categoryObject->GetShowAnatomy())
    {
    // Select anatomic context
    char* anatomicContextName = entry->GetAnatomicContextName();
    if (anatomicContextName) // Optional
      {
      int anatomicContextIndex = d->ComboBox_AnatomicContext->findText(anatomicContextName);
      if (anatomicContextIndex == -1)
        {
        qCritical() << Q_FUNC_INFO << ": Failed to find anatomic context with context name " << anatomicContextName;
        returnValue = false;
        }
      if (anatomicContextIndex != d->ComboBox_AnatomicContext->currentIndex())
        {
        this->setCurrentAnatomicContext(d->ComboBox_AnatomicContext->itemText(anatomicContextIndex));
        }
      d->ComboBox_AnatomicContext->blockSignals(true);
      d->ComboBox_AnatomicContext->setCurrentIndex(anatomicContextIndex);
      d->ComboBox_AnatomicContext->blockSignals(false);
      }

    // Select region
    vtkSlicerTerminologyType* regionObject = entry->GetAnatomicRegionObject();
    if (regionObject) // Optional
      {
      if (!this->setCurrentRegion(regionObject))
        {
        qCritical() << Q_FUNC_INFO << ": Failed to find region with name " << (regionObject->GetCodeMeaning()?regionObject->GetCodeMeaning():"NULL");
        returnValue = false;
        }

      // Select region modifier
      vtkSlicerTerminologyType* regionModifierObject = entry->GetAnatomicRegionModifierObject();
      if (regionObject->GetHasModifiers() && regionModifierObject)
        {
        if (!this->setCurrentRegionModifier(regionModifierObject))
          {
          qCritical() << Q_FUNC_INFO << ": Failed to find region modifier with name " << (regionModifierObject->GetCodeMeaning()?regionModifierObject->GetCodeMeaning():"NULL");
          returnValue = false;
          }
        }
      } // If region is selected
    } // If showAnatomy is true

  // Set color to color picker if no custom color is used
  if (!d->CustomColor.isValid())
    {
    d->setRecommendedColorFromCurrentTerminology();
    }

  return returnValue;
}

//-----------------------------------------------------------------------------
QColor qSlicerTerminologyNavigatorWidget::customColor()
{
  Q_D(qSlicerTerminologyNavigatorWidget);

  QColor color = d->ColorPickerButton_RecommendedRGB->color();

  bool isCustomColor = false;
  QColor recommendedColor = this->recommendedColorFromCurrentTerminology();
  if (recommendedColor.isValid() && color != recommendedColor)
    {
    isCustomColor = true;
    }

  if (isCustomColor)
    {
    return color;
    }
  // Return invalid color if terminology is invalid or color is custom (different from recommended color in terminology)
  return QColor();
}

//-----------------------------------------------------------------------------
void qSlicerTerminologyNavigatorWidget::setColor(QColor color)
{
  Q_D(qSlicerTerminologyNavigatorWidget);

  if (d->CurrentAnatomicContextName.isEmpty() || !d->CurrentCategoryObject || !d->CurrentTypeObject)
    {
    qCritical() << Q_FUNC_INFO << ": Color can only be set if current terminology is valid (it was set before)";
    return;
    }

  // Only set custom color if different from recommended color in current terminology
  QColor recommendedColor = this->recommendedColorFromCurrentTerminology();
  if (color != recommendedColor)
    {
    d->CustomColor = color;
    d->ColorPickerButton_RecommendedRGB->setColor(d->CustomColor);
    }
  else
    {
    // Invalidate custom color if set color is the same as recommended (to indicate that no custom color is set when queried later)
    d->CustomColor = QColor();
    d->ColorPickerButton_RecommendedRGB->setColor(color);
    }
}

//-----------------------------------------------------------------------------
QString qSlicerTerminologyNavigatorWidget::serializeTerminologyEntry(vtkSlicerTerminologyEntry* entry)
{
  if (!entry)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid terminology given";
    return QString();
    }

  // Serialized terminology entry consists of the following: terminologyContextName, category (codingScheme,  
  // codeValue, codeMeaning triple), type, typeModifier, anatomicContextName, anatomicRegion, anatomicRegionModifier
  QString serializedEntry;
  serializedEntry += QString(entry->GetTerminologyContextName()) + "~"; // Invalid string if NULL
  serializedEntry += QString(entry->GetCategoryObject() ? entry->GetCategoryObject()->GetCodingScheme() : NULL) + "^"
    + QString(entry->GetCategoryObject() ? entry->GetCategoryObject()->GetCodeValue() : NULL) + "^"
    + QString(entry->GetCategoryObject() ? entry->GetCategoryObject()->GetCodeMeaning() : NULL) + "~";
  serializedEntry += QString(entry->GetTypeObject() ? entry->GetTypeObject()->GetCodingScheme() : NULL) + "^"
    + QString(entry->GetTypeObject() ? entry->GetTypeObject()->GetCodeValue() : NULL) + "^"
    + QString(entry->GetTypeObject() ? entry->GetTypeObject()->GetCodeMeaning() : NULL) + "~";
  serializedEntry += QString(entry->GetTypeModifierObject() ? entry->GetTypeModifierObject()->GetCodingScheme() : NULL) + "^"
    + QString(entry->GetTypeModifierObject() ? entry->GetTypeModifierObject()->GetCodeValue() : NULL) + "^"
    + QString(entry->GetTypeModifierObject() ? entry->GetTypeModifierObject()->GetCodeMeaning() : NULL) + "~";

  serializedEntry += QString(entry->GetAnatomicContextName()) + "~";
  serializedEntry += QString(entry->GetAnatomicRegionObject() ? entry->GetAnatomicRegionObject()->GetCodingScheme() : NULL) + "^"
    + QString(entry->GetAnatomicRegionObject() ? entry->GetAnatomicRegionObject()->GetCodeValue() : NULL) + "^"
    + QString(entry->GetAnatomicRegionObject() ? entry->GetAnatomicRegionObject()->GetCodeMeaning() : NULL) + "~";
  serializedEntry += QString(entry->GetAnatomicRegionModifierObject() ? entry->GetAnatomicRegionModifierObject()->GetCodingScheme() : NULL) + "^"
    + QString(entry->GetAnatomicRegionModifierObject() ? entry->GetAnatomicRegionModifierObject()->GetCodeValue() : NULL) + "^"
    + QString(entry->GetAnatomicRegionModifierObject() ? entry->GetAnatomicRegionModifierObject()->GetCodeMeaning() : NULL);

  return serializedEntry;
}

//-----------------------------------------------------------------------------
QString qSlicerTerminologyNavigatorWidget::serializeTerminologyEntry(
  QString terminologyContextName,
  QString categoryValue, QString categorySchemeDesignator, QString categoryMeaning,
  QString typeValue, QString typeSchemeDesignator, QString typeMeaning,
  QString modifierValue, QString modifierSchemeDesignator, QString modifierMeaning,
  QString anatomicContextName,
  QString regionValue, QString regionSchemeDesignator, QString regionMeaning,
  QString regionModifierValue, QString regionModifierSchemeDesignator, QString regionModifierMeaning )
{
  QString serializedEntry;
  serializedEntry += terminologyContextName + "~";
  serializedEntry += categorySchemeDesignator + "^" + categoryValue + "^" + categoryMeaning + "~";
  serializedEntry += typeSchemeDesignator + "^" + typeValue + "^" + typeMeaning + "~";
  serializedEntry += modifierSchemeDesignator + "^" + modifierValue + "^" + modifierMeaning + "~";

  serializedEntry += anatomicContextName + "~";
  serializedEntry += regionSchemeDesignator + "^" + regionValue + "^" + regionMeaning + "~";
  serializedEntry += regionModifierSchemeDesignator + "^" + regionModifierValue + "^" + regionModifierMeaning;

  return serializedEntry;
}

//-----------------------------------------------------------------------------
bool qSlicerTerminologyNavigatorWidget::deserializeTerminologyEntry(QString serializedEntry, vtkSlicerTerminologyEntry* entry)
{
  // Clear terminology entry object
  entry->SetTerminologyContextName(NULL);
  entry->SetCategoryObject(NULL);
  entry->SetTypeObject(NULL);
  entry->SetTypeModifierObject(NULL);
  entry->SetAnatomicContextName(NULL);
  entry->SetAnatomicRegionObject(NULL);
  entry->SetAnatomicRegionModifierObject(NULL);

  // Serialized terminology entry consists of the following: terminologyContextName, category (codingScheme,  
  // codeValue, codeMeaning triple), type, typeModifier, anatomicContextName, anatomicRegion, anatomicRegionModifier
  QStringList entryComponents = serializedEntry.split("~");
  if (entryComponents.count() != 7)
    {
    return false;
    }

  // Get terminology logic
  vtkSlicerTerminologiesModuleLogic* terminologyLogic =
      qSlicerTerminologyNavigatorWidgetPrivate::terminologyLogic();
  if (!terminologyLogic)
    {
    qCritical() << Q_FUNC_INFO << ": Unable to access terminology logic";
    return false;
    }

  // Terminology context name
  if (entryComponents[0].isEmpty())
    {
    return false;
    }
  std::string terminologyName(entryComponents[0].toLatin1().constData());
  entry->SetTerminologyContextName(terminologyName.empty()?NULL:terminologyName.c_str());

  // Category
  QStringList categoryIds = entryComponents[1].split("^");
  if (categoryIds.count() != 3)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid category component";
    return false;
    }
  vtkSlicerTerminologiesModuleLogic::CodeIdentifier categoryId(
    categoryIds[0].toLatin1().constData(), categoryIds[1].toLatin1().constData(), categoryIds[2].toLatin1().constData() );
  vtkSmartPointer<vtkSlicerTerminologyCategory> categoryObject = vtkSmartPointer<vtkSlicerTerminologyCategory>::New();
  if ( !terminologyLogic->GetCategoryInTerminology(terminologyName, categoryId, categoryObject) )
    {
    qCritical() << Q_FUNC_INFO << ": Failed to get terminology category";
    return false;
   }
  entry->SetCategoryObject(categoryObject);

  // Type
  QStringList typeIds = entryComponents[2].split("^");
  if (typeIds.count() != 3)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid type component";
    return false;
    }
  vtkSlicerTerminologiesModuleLogic::CodeIdentifier typeId(
    typeIds[0].toLatin1().constData(), typeIds[1].toLatin1().constData(), typeIds[2].toLatin1().constData() );
  vtkSmartPointer<vtkSlicerTerminologyType> typeObject = vtkSmartPointer<vtkSlicerTerminologyType>::New();
  if (!terminologyLogic->GetTypeInTerminologyCategory(terminologyName, categoryId, typeId, typeObject) )
    {
    qCritical() << Q_FUNC_INFO << ": Failed to get terminology type";
    return false;
    }
  entry->SetTypeObject(typeObject);

  // Type modifier (optional)
  QStringList typeModifierIds = entryComponents[3].split("^");
  if (typeModifierIds.count() == 3)
    {
    vtkSlicerTerminologiesModuleLogic::CodeIdentifier typeModifierId(
      typeModifierIds[0].toLatin1().constData(), typeModifierIds[1].toLatin1().constData(), typeModifierIds[2].toLatin1().constData() );
    vtkSmartPointer<vtkSlicerTerminologyType> typeModifierObject = vtkSmartPointer<vtkSlicerTerminologyType>::New();
    if ( terminologyLogic->GetTypeModifierInTerminologyType(terminologyName, categoryId, typeId, typeModifierId, typeModifierObject) )
      {
      entry->SetTypeModifierObject(typeModifierObject);
      }
    }

  // Anatomic context name (optional)
  std::string anatomicContextName = entryComponents[4].toLatin1().constData();
  entry->SetAnatomicContextName(anatomicContextName.empty()?NULL:anatomicContextName.c_str());

  // Anatomic region (optional)
  QStringList regionIds = entryComponents[5].split("^");
  if (regionIds.count() == 3)
    {
    vtkSlicerTerminologiesModuleLogic::CodeIdentifier regionId(
      regionIds[0].toLatin1().constData(), regionIds[1].toLatin1().constData(), regionIds[2].toLatin1().constData() );
    vtkSmartPointer<vtkSlicerTerminologyType> regionObject = vtkSmartPointer<vtkSlicerTerminologyType>::New();
    if ( terminologyLogic->GetRegionInAnatomicContext(anatomicContextName, regionId, regionObject) )
      {
      entry->SetAnatomicRegionObject(regionObject);
      }

    // Anatomic region modifier (optional)
    QStringList regionModifierIds = entryComponents[6].split("^");
    if (regionModifierIds.count() == 3)
      {
      vtkSlicerTerminologiesModuleLogic::CodeIdentifier regionModifierId(
        regionModifierIds[0].toLatin1().constData(), regionModifierIds[1].toLatin1().constData(), regionModifierIds[2].toLatin1().constData() );
      vtkSmartPointer<vtkSlicerTerminologyType> regionModifierObject = vtkSmartPointer<vtkSlicerTerminologyType>::New();
      if ( terminologyLogic->GetRegionModifierInAnatomicRegion(anatomicContextName, regionId, regionModifierId, regionModifierObject ))
        {
        entry->SetAnatomicRegionModifierObject(regionModifierObject);
        }
      }
    }

  return true;
}

//-----------------------------------------------------------------------------
QColor qSlicerTerminologyNavigatorWidget::recommendedColorFromTerminology(vtkSlicerTerminologyEntry* entry)
{
  QColor color;
  if (!entry || !entry->GetTypeObject())
    {
    return color;
    }

  vtkSlicerTerminologyType* typeObject = entry->GetTypeObject();
  if (typeObject->GetHasModifiers())
    {
    // Get color from modifier if any
    typeObject = entry->GetTypeModifierObject();
    }

  unsigned char colorChar[3] = {0,0,0};
  typeObject->GetRecommendedDisplayRGBValue(colorChar);
  color.setRgb(colorChar[0], colorChar[1], colorChar[2]);
  return color;
}

//-----------------------------------------------------------------------------
QColor qSlicerTerminologyNavigatorWidget::recommendedColorFromCurrentTerminology()
{
  Q_D(qSlicerTerminologyNavigatorWidget);

  QColor color;
  if (d->CurrentAnatomicContextName.isEmpty() || !d->CurrentCategoryObject || !d->CurrentTypeObject)
    {
    qWarning() << Q_FUNC_INFO << ": Invalid current terminology";
    return color;
    }

  vtkSlicerTerminologyType* typeObject = d->CurrentTypeObject;
  if (d->CurrentTypeObject->GetHasModifiers())
    {
    // Get color from modifier if any
    typeObject = d->CurrentTypeModifierObject;
    }

  unsigned char colorChar[3] = {0,0,0};
  typeObject->GetRecommendedDisplayRGBValue(colorChar);
  color.setRgb(colorChar[0], colorChar[1], colorChar[2]);
  return color;
}

//-----------------------------------------------------------------------------
bool qSlicerTerminologyNavigatorWidget::anatomicRegionSectionVisible() const
{
  Q_D(const qSlicerTerminologyNavigatorWidget);

  return d->CollapsibleGroupBox_AnatomicRegion->isVisible();
}

//-----------------------------------------------------------------------------
void qSlicerTerminologyNavigatorWidget::setAnatomicRegionSectionVisible(bool visible)
{
  Q_D(qSlicerTerminologyNavigatorWidget);

  d->CollapsibleGroupBox_AnatomicRegion->setVisible(visible);
}

//-----------------------------------------------------------------------------
void qSlicerTerminologyNavigatorWidget::populateTerminologyComboBox()
{
  Q_D(qSlicerTerminologyNavigatorWidget);

  d->ComboBox_Terminology->clear();

  vtkSlicerTerminologiesModuleLogic* logic = d->terminologyLogic();
  if (!logic)
    {
    return;
    }

  std::vector<std::string> terminologyNames;
  logic->GetLoadedTerminologyNames(terminologyNames);
  for (std::vector<std::string>::iterator termIt=terminologyNames.begin(); termIt!=terminologyNames.end(); ++termIt)
    {
    d->ComboBox_Terminology->addItem(termIt->c_str());
    }
}

//-----------------------------------------------------------------------------
void qSlicerTerminologyNavigatorWidget::populateCategoryTable()
{
  Q_D(qSlicerTerminologyNavigatorWidget);

  d->tableWidget_Category->clearContents();

  if (d->CurrentTerminologyName.isEmpty())
    {
    d->tableWidget_Category->setRowCount(0);
    return;
    }

  vtkSlicerTerminologiesModuleLogic* logic = d->terminologyLogic();
  if (!logic)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access terminology logic";
    return;
    }

  // Get category names containing the search string. If no search string then add every category
  std::vector<vtkSlicerTerminologiesModuleLogic::CodeIdentifier> categories;
  logic->FindCategoriesInTerminology(
    d->CurrentTerminologyName.toLatin1().constData(), categories, d->SearchBox_Category->text().toLatin1().constData() );

  QTableWidgetItem* selectedItem = NULL;
  d->tableWidget_Category->setRowCount(categories.size());
  int index = 0;
  std::vector<vtkSlicerTerminologiesModuleLogic::CodeIdentifier>::iterator idIt;
  for (idIt=categories.begin(); idIt!=categories.end(); ++idIt, ++index)
    {
    vtkSlicerTerminologiesModuleLogic::CodeIdentifier addedCategoryId = (*idIt);
    QString addedCategoryName(addedCategoryId.CodeMeaning.c_str());
    QTableWidgetItem* addedCategoryItem = new QTableWidgetItem(addedCategoryName);
    addedCategoryItem->setData(CodingSchemeDesignatorRole, QString(addedCategoryId.CodingSchemeDesignator.c_str()));
    addedCategoryItem->setData(CodeValueRole, QString(addedCategoryId.CodeValue.c_str()));
    d->tableWidget_Category->setItem(index, 0, addedCategoryItem);

    if ( d->CurrentCategoryObject->GetCodingScheme() && !addedCategoryId.CodingSchemeDesignator.compare(d->CurrentCategoryObject->GetCodingScheme())
      && d->CurrentCategoryObject->GetCodeValue() && !addedCategoryId.CodeValue.compare(d->CurrentCategoryObject->GetCodeValue()) )
      {
      selectedItem = addedCategoryItem;
      }
    }

  // Select category if selection was valid and item shows up in search
  if (selectedItem)
    {
    d->tableWidget_Category->setCurrentItem(selectedItem);
    }
}

//-----------------------------------------------------------------------------
void qSlicerTerminologyNavigatorWidget::populateTypeTable()
{
  Q_D(qSlicerTerminologyNavigatorWidget);

  d->tableWidget_Type->clearContents();

  if (d->CurrentTerminologyName.isEmpty() || !d->CurrentCategoryObject || !d->CurrentCategoryObject->GetCodeValue())
    {
    d->tableWidget_Type->setRowCount(0);
    return;
    }

  vtkSlicerTerminologiesModuleLogic* logic = d->terminologyLogic();
  if (!logic)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access terminology logic";
    return;
    }

  // Get type names containing the search string. If no search string then add every type
  std::vector<vtkSlicerTerminologiesModuleLogic::CodeIdentifier> types;
  logic->FindTypesInTerminologyCategory(
    d->CurrentTerminologyName.toLatin1().constData(),
    vtkSlicerTerminologiesModuleLogic::CodeIdentifierFromTerminologyCategory(d->CurrentCategoryObject),
    types, d->SearchBox_Type->text().toLatin1().constData() );

  QTableWidgetItem* selectedItem = NULL;
  d->tableWidget_Type->setRowCount(types.size());
  int index = 0;
  std::vector<vtkSlicerTerminologiesModuleLogic::CodeIdentifier>::iterator idIt;
  for (idIt=types.begin(); idIt!=types.end(); ++idIt, ++index)
    {
    vtkSlicerTerminologiesModuleLogic::CodeIdentifier addedTypeId = (*idIt);
    QString addedTypeName(addedTypeId.CodeMeaning.c_str());
    QTableWidgetItem* addedTypeItem = new QTableWidgetItem(addedTypeName);
    addedTypeItem->setData(CodingSchemeDesignatorRole, QString(addedTypeId.CodingSchemeDesignator.c_str()));
    addedTypeItem->setData(CodeValueRole, QString(addedTypeId.CodeValue.c_str()));
    d->tableWidget_Type->setItem(index, 0, addedTypeItem);

    if ( d->CurrentTypeObject->GetCodingScheme() && !addedTypeId.CodingSchemeDesignator.compare(d->CurrentTypeObject->GetCodingScheme())
      && d->CurrentTypeObject->GetCodeValue() && !addedTypeId.CodeValue.compare(d->CurrentTypeObject->GetCodeValue()) )
      {
      selectedItem = addedTypeItem;
      }
    }

  // Select type if selection was valid and item shows up in search
  if (selectedItem)
    {
    d->tableWidget_Type->setCurrentItem(selectedItem);
    }
}

//-----------------------------------------------------------------------------
void qSlicerTerminologyNavigatorWidget::populateTypeModifierComboBox()
{
  Q_D(qSlicerTerminologyNavigatorWidget);

  d->ComboBox_TypeModifier->clear();

  if (d->CurrentTerminologyName.isEmpty() || !d->CurrentCategoryObject || !d->CurrentTypeObject || !d->CurrentTypeObject->GetCodeValue())
    {
    d->ComboBox_TypeModifier->setEnabled(false);
    return;
    }
  // If current type has no modifiers then leave it empty and disable
  if (!d->CurrentTypeObject->GetHasModifiers())
    {
    d->ComboBox_TypeModifier->setEnabled(false);
    return;
    }

  vtkSlicerTerminologiesModuleLogic* logic = d->terminologyLogic();
  if (!logic)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access terminology logic";
    return;
    }

  // Get type modifier names
  std::vector<vtkSlicerTerminologiesModuleLogic::CodeIdentifier> typeModifiers;
  logic->GetTypeModifiersInTerminologyType(
    d->CurrentTerminologyName.toLatin1().constData(),
    vtkSlicerTerminologiesModuleLogic::CodeIdentifierFromTerminologyCategory(d->CurrentCategoryObject),
    vtkSlicerTerminologiesModuleLogic::CodeIdentifierFromTerminologyType(d->CurrentTypeObject),
    typeModifiers );

  int selectedIndex = -1;
  int index = 0;
  std::vector<vtkSlicerTerminologiesModuleLogic::CodeIdentifier>::iterator idIt;
  for (idIt=typeModifiers.begin(); idIt!=typeModifiers.end(); ++idIt, ++index)
    {
    vtkSlicerTerminologiesModuleLogic::CodeIdentifier addedTypeModifierId = (*idIt);
    QString addedTypeModifierName(addedTypeModifierId.CodeMeaning.c_str());

    QMap<QString, QVariant> userData;
    userData[QString::number(CodingSchemeDesignatorRole)] = QString(addedTypeModifierId.CodingSchemeDesignator.c_str());
    userData[QString::number(CodeValueRole)] = QString(addedTypeModifierId.CodeValue.c_str());
    d->ComboBox_TypeModifier->addItem(addedTypeModifierName, QVariant(userData));
    
    if (!addedTypeModifierName.compare(d->CurrentTypeModifierObject->GetCodeMeaning()))
      {
      selectedIndex = index;
      }
    }

  // Select modifier if selection was valid
  if (selectedIndex != -1)
    {
    d->ComboBox_TypeModifier->setCurrentIndex(selectedIndex);
    }
}

//-----------------------------------------------------------------------------
void qSlicerTerminologyNavigatorWidget::setCurrentTerminology(QString terminologyName)
{
  Q_D(qSlicerTerminologyNavigatorWidget);

  // If no change then nothing to do
  if (d->CurrentTerminologyName == terminologyName)
    {
    return;
    }

  // Reset current category, type, and type modifier
  d->resetCurrentCategory();
  d->resetCurrentType();
  d->resetCurrentTypeModifier();

  // Set current terminology
  d->CurrentTerminologyName = terminologyName;
  if (terminologyName.isEmpty())
    {
    return;
    }

  // Populate category table, and reset type table and type modifier combobox
  this->populateCategoryTable();
  this->populateTypeTable();
  this->populateTypeModifierComboBox();

  // Only enable category table if there are items in it
  if (d->tableWidget_Category->rowCount() == 0)
    {
    d->tableWidget_Category->setEnabled(false);
    if (d->SearchBox_Category->text().isEmpty())
      {
      // Table might be empty because of a search
      d->SearchBox_Category->setEnabled(false);
      }
    d->tableWidget_Type->setEnabled(false);
    d->SearchBox_Type->setEnabled(false);
    d->ComboBox_TypeModifier->setEnabled(false);
    }
  else
    {
    d->tableWidget_Category->setEnabled(true);
    d->SearchBox_Category->setEnabled(true);
    }

  // Selection is invalid until category then type is selected
  emit selectionValidityChanged(false);
}

//-----------------------------------------------------------------------------
void qSlicerTerminologyNavigatorWidget::onTerminologySelectionChanged(int index)
{
  Q_D(qSlicerTerminologyNavigatorWidget);

  QApplication::setOverrideCursor(QCursor(Qt::BusyCursor));

  // Set current terminology
  this->setCurrentTerminology(d->ComboBox_Terminology->itemText(index));

  QApplication::restoreOverrideCursor();
}

//-----------------------------------------------------------------------------
bool qSlicerTerminologyNavigatorWidget::setCurrentCategory(vtkSlicerTerminologyCategory* category)
{
  Q_D(qSlicerTerminologyNavigatorWidget);

  // Reset current type and type modifier
  d->resetCurrentType();
  d->resetCurrentTypeModifier();
  // Reset anatomic region information as well
  d->resetCurrentRegion();
  d->resetCurrentRegionModifier();

  if (!category)
    {
    d->resetCurrentCategory();
    qCritical() << Q_FUNC_INFO << ": Invalid category object set";
    return false;
    }

  // Set current category
  d->CurrentCategoryObject->Copy(category);

  // Populate type table, and reset type modifier combobox and region widgets
  this->populateTypeTable();
  this->populateTypeModifierComboBox();
  d->tableWidget_AnatomicRegion->setCurrentItem(NULL);
  this->populateRegionModifierComboBox();

  // Only enable type table if there are items in it
  if (d->tableWidget_Type->rowCount() == 0)
    {
    d->tableWidget_Type->setEnabled(false);
    if (d->SearchBox_Type->text().isEmpty())
      {
      // Table might be empty because of a search
      d->SearchBox_Type->setEnabled(false);
      }
    d->ComboBox_TypeModifier->setEnabled(false);
    }
  else
    {
    d->tableWidget_Type->setEnabled(true);
    d->SearchBox_Type->setEnabled(true);
    }

  // Enable anatomic region controls if related flag is on
  d->ComboBox_AnatomicContext->setEnabled(d->CurrentCategoryObject->GetShowAnatomy());
  d->tableWidget_AnatomicRegion->setEnabled(d->CurrentCategoryObject->GetShowAnatomy());
  d->SearchBox_AnatomicRegion->setEnabled(d->CurrentCategoryObject->GetShowAnatomy());
  d->ComboBox_AnatomicRegionModifier->setEnabled(false); // Disabled until valid region selection

  // Selection is invalid until type is selected
  emit selectionValidityChanged(false);

  // Select category if found
  QTableWidgetItem* categoryItem = d->findTableWidgetItemForCategory(category);
  if (categoryItem)
    {
    d->tableWidget_Category->blockSignals(true);
    d->tableWidget_Category->setCurrentItem(categoryItem);
    d->tableWidget_Category->blockSignals(false);
    }
  return categoryItem; // Return true if category found and selected
}

//-----------------------------------------------------------------------------
void qSlicerTerminologyNavigatorWidget::onCategoryClicked(QTableWidgetItem* item)
{
  Q_D(qSlicerTerminologyNavigatorWidget);

  QApplication::setOverrideCursor(QCursor(Qt::BusyCursor));

  // Get current category object
  vtkSlicerTerminologiesModuleLogic* logic = d->terminologyLogic();
  if (!logic)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access terminology logic";
    return;
    }
  vtkSlicerTerminologiesModuleLogic::CodeIdentifier categoryId(
    item->data(CodingSchemeDesignatorRole).toString().toLatin1().constData(),
    item->data(CodeValueRole).toString().toLatin1().constData(),
    item->text().toLatin1().constData() );
  vtkSmartPointer<vtkSlicerTerminologyCategory> category = vtkSmartPointer<vtkSlicerTerminologyCategory>::New();
  if (!logic->GetCategoryInTerminology(
    d->CurrentTerminologyName.toLatin1().constData(), categoryId, category) )
    {
    qCritical() << Q_FUNC_INFO << ": Failed to find category '" << item->text();
    return;
    }

  // Set category from item
  this->setCurrentCategory(category);

  QApplication::restoreOverrideCursor();
}

//-----------------------------------------------------------------------------
bool qSlicerTerminologyNavigatorWidget::setCurrentType(vtkSlicerTerminologyType* type)
{
  Q_D(qSlicerTerminologyNavigatorWidget);

  // Reset current type modifier
  d->resetCurrentTypeModifier();

  if (!type)
    {
    d->resetCurrentType();
    qCritical() << Q_FUNC_INFO << ": Invalid type object set";
    return false;
    }

  // Set current type
  d->CurrentTypeObject->Copy(type);

  // Populate type modifier combobox
  this->populateTypeModifierComboBox();

  // Only enable type modifier combobox if there are items in it
  d->ComboBox_TypeModifier->setEnabled(d->ComboBox_TypeModifier->count());

  // With valid type selected, terminology selection becomes also valid
  emit selectionValidityChanged(true);

  // Select type if found
  QTableWidgetItem* typeItem = d->findTableWidgetItemForType(d->tableWidget_Type, type);
  if (typeItem)
    {
    d->tableWidget_Type->blockSignals(true);
    d->tableWidget_Type->setCurrentItem(typeItem);
    d->tableWidget_Type->blockSignals(false);
    }
  return typeItem; // Return true if category found and selected
}

//-----------------------------------------------------------------------------
void qSlicerTerminologyNavigatorWidget::onTypeClicked(QTableWidgetItem* item)
{
  Q_D(qSlicerTerminologyNavigatorWidget);

  QApplication::setOverrideCursor(QCursor(Qt::BusyCursor));

  // Get current type object
  vtkSlicerTerminologiesModuleLogic* logic = d->terminologyLogic();
  if (!logic)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access terminology logic";
    return;
    }
  vtkSlicerTerminologiesModuleLogic::CodeIdentifier typeId(
    item->data(CodingSchemeDesignatorRole).toString().toLatin1().constData(),
    item->data(CodeValueRole).toString().toLatin1().constData(),
    item->text().toLatin1().constData() );
  vtkSmartPointer<vtkSlicerTerminologyType> type = vtkSmartPointer<vtkSlicerTerminologyType>::New();
  if (!logic->GetTypeInTerminologyCategory(
    d->CurrentTerminologyName.toLatin1().constData(),
    vtkSlicerTerminologiesModuleLogic::CodeIdentifierFromTerminologyCategory(d->CurrentCategoryObject),
    typeId, type) )
    {
    qCritical() << Q_FUNC_INFO << ": Failed to find type '" << item->text();
    return;
    }

  // Set type from item
  this->setCurrentType(type);

  // Set recommended color to color picker
  d->setRecommendedColorFromCurrentTerminology();

  QApplication::restoreOverrideCursor();
}

//-----------------------------------------------------------------------------
bool qSlicerTerminologyNavigatorWidget::setCurrentTypeModifier(vtkSlicerTerminologyType* modifier)
{
  Q_D(qSlicerTerminologyNavigatorWidget);

  if (!modifier)
    {
    d->resetCurrentTypeModifier();
    qCritical() << Q_FUNC_INFO << ": Invalid type modifier object set";
    return false;
    }

  // Set current type modifier
  d->CurrentTypeModifierObject->Copy(modifier);

  // Select modifier if found
  int modifierIndex = d->findComboBoxIndexForModifier(d->ComboBox_TypeModifier, modifier);
  if (modifierIndex != -1)
    {
    d->ComboBox_TypeModifier->blockSignals(true);
    d->ComboBox_TypeModifier->setCurrentIndex(modifierIndex);
    d->ComboBox_TypeModifier->blockSignals(false);
    }
  return (modifierIndex != -1);
}

//-----------------------------------------------------------------------------
void qSlicerTerminologyNavigatorWidget::onTypeModifierSelectionChanged(int index)
{
  Q_UNUSED(index);
  Q_D(qSlicerTerminologyNavigatorWidget);

  QApplication::setOverrideCursor(QCursor(Qt::BusyCursor));

  // Get current modifier object
  vtkSlicerTerminologiesModuleLogic* logic = d->terminologyLogic();
  if (!logic)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access terminology logic";
    return;
    }
  QMap<QString, QVariant> userData = d->ComboBox_TypeModifier->itemData(index).toMap();
  vtkSlicerTerminologiesModuleLogic::CodeIdentifier modifierId(
    userData[QString::number(CodingSchemeDesignatorRole)].toString().toLatin1().constData(),
    userData[QString::number(CodeValueRole)].toString().toLatin1().constData(),
    d->ComboBox_TypeModifier->itemText(index).toLatin1().constData() );
  vtkSmartPointer<vtkSlicerTerminologyType> modifier = vtkSmartPointer<vtkSlicerTerminologyType>::New();
  if (!logic->GetTypeModifierInTerminologyType(
    d->CurrentTerminologyName.toLatin1().constData(),
    vtkSlicerTerminologiesModuleLogic::CodeIdentifierFromTerminologyCategory(d->CurrentCategoryObject),
    vtkSlicerTerminologiesModuleLogic::CodeIdentifierFromTerminologyType(d->CurrentTypeObject),
    modifierId, modifier) )
    {
    qCritical() << Q_FUNC_INFO << ": Failed to find modifier '" << d->ComboBox_TypeModifier->itemText(index);
    return;
    }

  // Set current region modifier
  this->setCurrentTypeModifier(modifier);

  // Set recommended color to color picker
  d->setRecommendedColorFromCurrentTerminology();

  QApplication::restoreOverrideCursor();
}

//-----------------------------------------------------------------------------
void qSlicerTerminologyNavigatorWidget::onCategorySearchTextChanged(QString search)
{
  Q_UNUSED(search);

  this->populateCategoryTable();
}

//-----------------------------------------------------------------------------
void qSlicerTerminologyNavigatorWidget::onTypeSearchTextChanged(QString search)
{
  Q_UNUSED(search);

  this->populateTypeTable();
}

//-----------------------------------------------------------------------------
void qSlicerTerminologyNavigatorWidget::onColorChanged(QColor color)
{
  Q_D(qSlicerTerminologyNavigatorWidget);

  // Save selected color as custom color
  // (the recommended color coming from type and type modifier are already stored in they current entry)
  d->CustomColor = color;
}

//-----------------------------------------------------------------------------
void qSlicerTerminologyNavigatorWidget::populateAnatomicContextComboBox()
{
  Q_D(qSlicerTerminologyNavigatorWidget);

  d->ComboBox_AnatomicContext->clear();

  vtkSlicerTerminologiesModuleLogic* logic = d->terminologyLogic();
  if (!logic)
    {
    return;
    }

  std::vector<std::string> anatomicRegionContextNames;
  logic->GetLoadedAnatomicContextNames(anatomicRegionContextNames);
  for (std::vector<std::string>::iterator anIt=anatomicRegionContextNames.begin(); anIt!=anatomicRegionContextNames.end(); ++anIt)
    {
    d->ComboBox_AnatomicContext->addItem(anIt->c_str());
    }

  // Hide anatomic context combobox if there is only one option
  if (d->ComboBox_AnatomicContext->count() == 1)
    {
    this->onAnatomicContextSelectionChanged(0);
    d->ComboBox_AnatomicContext->setVisible(false);
    }
  else if (d->ComboBox_AnatomicContext->count() > 1)
    {
    d->ComboBox_AnatomicContext->setVisible(true);
    }
}

//-----------------------------------------------------------------------------
void qSlicerTerminologyNavigatorWidget::populateRegionTable()
{
  Q_D(qSlicerTerminologyNavigatorWidget);

  d->tableWidget_AnatomicRegion->clearContents();

  if (d->CurrentAnatomicContextName.isEmpty())
    {
    d->tableWidget_AnatomicRegion->setRowCount(0);
    return;
    }

  vtkSlicerTerminologiesModuleLogic* logic = d->terminologyLogic();
  if (!logic)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access terminology logic";
    return;
    }

  // Get region names containing the search string. If no search string then add every region
  std::vector<vtkSlicerTerminologiesModuleLogic::CodeIdentifier> regions;
  logic->FindRegionsInAnatomicContext(
    d->CurrentAnatomicContextName.toLatin1().constData(),
    regions, d->SearchBox_Type->text().toLatin1().constData() );

  QTableWidgetItem* selectedItem = NULL;
  d->tableWidget_AnatomicRegion->setRowCount(regions.size());
  int index = 0;
  std::vector<vtkSlicerTerminologiesModuleLogic::CodeIdentifier>::iterator idIt;
  for (idIt=regions.begin(); idIt!=regions.end(); ++idIt, ++index)
    {
    vtkSlicerTerminologiesModuleLogic::CodeIdentifier addedRegionId = (*idIt);
    QString addedRegionName(addedRegionId.CodeMeaning.c_str());
    QTableWidgetItem* addedRegionItem = new QTableWidgetItem(addedRegionName);
    addedRegionItem->setData(CodingSchemeDesignatorRole, QString(addedRegionId.CodingSchemeDesignator.c_str()));
    addedRegionItem->setData(CodeValueRole, QString(addedRegionId.CodeValue.c_str()));
    d->tableWidget_AnatomicRegion->setItem(index, 0, addedRegionItem);

    if ( d->CurrentRegionObject->GetCodingScheme() && !addedRegionId.CodingSchemeDesignator.compare(d->CurrentRegionObject->GetCodingScheme())
      && d->CurrentRegionObject->GetCodeValue() && !addedRegionId.CodeValue.compare(d->CurrentRegionObject->GetCodeValue()) )
      {
      selectedItem = addedRegionItem;
      }
    }

  // Select region if selection was valid and item shows up in search
  if (selectedItem)
    {
    d->tableWidget_AnatomicRegion->setCurrentItem(selectedItem);
    }
}

//-----------------------------------------------------------------------------
void qSlicerTerminologyNavigatorWidget::populateRegionModifierComboBox()
{
  Q_D(qSlicerTerminologyNavigatorWidget);

  d->ComboBox_AnatomicRegionModifier->clear();

  if (d->CurrentAnatomicContextName.isEmpty() || !d->CurrentRegionObject || !d->CurrentRegionObject->GetCodeValue())
    {
    d->ComboBox_AnatomicRegionModifier->setEnabled(false);
    return;
    }
  // If current region has no modifiers then leave it empty and disable
  if (!d->CurrentRegionObject->GetHasModifiers())
    {
    d->ComboBox_AnatomicRegionModifier->setEnabled(false);
    return;
    }

  vtkSlicerTerminologiesModuleLogic* logic = d->terminologyLogic();
  if (!logic)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access terminology logic";
    return;
    }

  // Get region modifier names
  std::vector<vtkSlicerTerminologiesModuleLogic::CodeIdentifier> regionModifiers;
  logic->GetRegionModifiersInAnatomicRegion(
    d->CurrentAnatomicContextName.toLatin1().constData(),
    vtkSlicerTerminologiesModuleLogic::CodeIdentifierFromTerminologyType(d->CurrentRegionObject),
    regionModifiers );

  int selectedIndex = -1;
  int index = 0;
  std::vector<vtkSlicerTerminologiesModuleLogic::CodeIdentifier>::iterator idIt;
  for (idIt=regionModifiers.begin(); idIt!=regionModifiers.end(); ++idIt, ++index)
    {
    vtkSlicerTerminologiesModuleLogic::CodeIdentifier addedRegionModifierId = (*idIt);
    QString addedRegionModifierName(addedRegionModifierId.CodeMeaning.c_str());

    QMap<QString, QVariant> userData;
    userData[QString::number(CodingSchemeDesignatorRole)] = QString(addedRegionModifierId.CodingSchemeDesignator.c_str());
    userData[QString::number(CodeValueRole)] = QString(addedRegionModifierId.CodeValue.c_str());
    d->ComboBox_AnatomicRegionModifier->addItem(addedRegionModifierName, QVariant(userData));

    if (!addedRegionModifierName.compare(d->CurrentRegionModifierObject->GetCodeMeaning()))
      {
      selectedIndex = index;
      }
    }

  // Select modifier if selection was valid
  if (selectedIndex != -1)
    {
    d->ComboBox_AnatomicRegionModifier->setCurrentIndex(selectedIndex);
    }
}

//-----------------------------------------------------------------------------
void qSlicerTerminologyNavigatorWidget::setCurrentAnatomicContext(QString contextName)
{
  Q_D(qSlicerTerminologyNavigatorWidget);

  // Reset current region and region modifier
  d->resetCurrentRegion();
  d->resetCurrentRegionModifier();

  // Set current anatomic context
  d->CurrentAnatomicContextName = contextName;
  if (contextName.isEmpty())
    {
    return;
    }

  // Populate region table and reset region modifier combobox
  this->populateRegionTable();
  this->populateRegionModifierComboBox();

  // Only enable region table if there are items in it
  if (d->tableWidget_AnatomicRegion->rowCount() == 0)
    {
    d->tableWidget_AnatomicRegion->setEnabled(false);
    if (d->SearchBox_AnatomicRegion->text().isEmpty())
      {
      // Table might be empty because of a search
      d->SearchBox_AnatomicRegion->setEnabled(false);
      }
    d->ComboBox_AnatomicRegionModifier->setEnabled(false);
    }
  else if (d->CurrentCategoryObject->GetShowAnatomy())
    {
    d->tableWidget_AnatomicRegion->setEnabled(true);
    d->SearchBox_AnatomicRegion->setEnabled(true);
    }
}

//-----------------------------------------------------------------------------
void qSlicerTerminologyNavigatorWidget::onAnatomicContextSelectionChanged(int index)
{
  Q_D(qSlicerTerminologyNavigatorWidget);

  QApplication::setOverrideCursor(QCursor(Qt::BusyCursor));

  // Set current anatomic context
  this->setCurrentAnatomicContext(d->ComboBox_AnatomicContext->itemText(index));

  QApplication::restoreOverrideCursor();
}

//-----------------------------------------------------------------------------
bool qSlicerTerminologyNavigatorWidget::setCurrentRegion(vtkSlicerTerminologyType* region)
{
  Q_D(qSlicerTerminologyNavigatorWidget);

  // Reset current region modifier
  d->resetCurrentRegionModifier();

  if (!region)
    {
    d->resetCurrentRegion();
    qCritical() << Q_FUNC_INFO << ": Invalid region object set";
    return false;
    }

  // Set current region
  d->CurrentRegionObject->Copy(region);

  // Populate region modifier combobox
  this->populateRegionModifierComboBox();

  // Only enable region modifier combobox if there are items in it
  d->ComboBox_AnatomicRegionModifier->setEnabled(d->ComboBox_AnatomicRegionModifier->count());

  // Select region if found
  QTableWidgetItem* regionItem = d->findTableWidgetItemForType(d->tableWidget_AnatomicRegion, region);
  if (regionItem)
    {
    d->tableWidget_AnatomicRegion->blockSignals(true);
    d->tableWidget_AnatomicRegion->setCurrentItem(regionItem);
    d->tableWidget_AnatomicRegion->blockSignals(false);
    }
  return regionItem; // Return true if category found and selected
}

//-----------------------------------------------------------------------------
void qSlicerTerminologyNavigatorWidget::onRegionClicked(QTableWidgetItem* item)
{
  Q_D(qSlicerTerminologyNavigatorWidget);

  QApplication::setOverrideCursor(QCursor(Qt::BusyCursor));

  // Get current region object
  vtkSlicerTerminologiesModuleLogic* logic = d->terminologyLogic();
  if (!logic)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access terminology logic";
    return;
    }
  vtkSlicerTerminologiesModuleLogic::CodeIdentifier regionId(
    item->data(CodingSchemeDesignatorRole).toString().toLatin1().constData(),
    item->data(CodeValueRole).toString().toLatin1().constData(),
    item->text().toLatin1().constData() );
  vtkSmartPointer<vtkSlicerTerminologyType> region = vtkSmartPointer<vtkSlicerTerminologyType>::New();
  if (!logic->GetRegionInAnatomicContext(
    d->CurrentAnatomicContextName.toLatin1().constData(),
    regionId, region) )
    {
    qCritical() << Q_FUNC_INFO << ": Failed to find region '" << item->text();
    return;
    }

  // Set current region
  this->setCurrentRegion(region);

  QApplication::restoreOverrideCursor();
}

//-----------------------------------------------------------------------------
bool qSlicerTerminologyNavigatorWidget::setCurrentRegionModifier(vtkSlicerTerminologyType* modifier)
{
  Q_D(qSlicerTerminologyNavigatorWidget);

  if (!modifier)
    {
    d->resetCurrentRegionModifier();
    qCritical() << Q_FUNC_INFO << ": Invalid region modifier object set";
    return false;
    }

  // Set current type modifier
  d->CurrentRegionModifierObject->Copy(modifier);

  // Select modifier if found
  int modifierIndex = d->findComboBoxIndexForModifier(d->ComboBox_AnatomicRegionModifier, modifier);
  if (modifierIndex != -1)
    {
    d->ComboBox_AnatomicRegionModifier->blockSignals(true);
    d->ComboBox_AnatomicRegionModifier->setCurrentIndex(modifierIndex);
    d->ComboBox_AnatomicRegionModifier->blockSignals(false);
    }
  return (modifierIndex != -1);
}

//-----------------------------------------------------------------------------
void qSlicerTerminologyNavigatorWidget::onRegionModifierSelectionChanged(int index)
{
  Q_UNUSED(index);
  Q_D(qSlicerTerminologyNavigatorWidget);

  QApplication::setOverrideCursor(QCursor(Qt::BusyCursor));

  // Get current modifier object
  vtkSlicerTerminologiesModuleLogic* logic = d->terminologyLogic();
  if (!logic)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access terminology logic";
    return;
    }
  QMap<QString, QVariant> userData = d->ComboBox_AnatomicRegionModifier->itemData(index).toMap();
  vtkSlicerTerminologiesModuleLogic::CodeIdentifier modifierId(
    userData[QString::number(CodingSchemeDesignatorRole)].toString().toLatin1().constData(),
    userData[QString::number(CodeValueRole)].toString().toLatin1().constData(),
    d->ComboBox_AnatomicRegionModifier->itemText(index).toLatin1().constData() );
  vtkSmartPointer<vtkSlicerTerminologyType> modifier = vtkSmartPointer<vtkSlicerTerminologyType>::New();
  if (!logic->GetRegionModifierInAnatomicRegion(
    d->CurrentAnatomicContextName.toLatin1().constData(),
    vtkSlicerTerminologiesModuleLogic::CodeIdentifierFromTerminologyType(d->CurrentRegionObject),
    modifierId, modifier) )
    {
    qCritical() << Q_FUNC_INFO << ": Failed to find modifier '" << d->ComboBox_AnatomicRegionModifier->itemText(index);
    return;
    }

  // Set current region modifier
  this->setCurrentRegionModifier(modifier);

  QApplication::restoreOverrideCursor();
}

//-----------------------------------------------------------------------------
void qSlicerTerminologyNavigatorWidget::onRegionSearchTextChanged(QString search)
{
  Q_UNUSED(search);

  this->populateRegionTable();
}

//-----------------------------------------------------------------------------
void qSlicerTerminologyNavigatorWidget::onLogicModified()
{
  Q_D(qSlicerTerminologyNavigatorWidget);

  this->populateTerminologyComboBox();
  d->resetCurrentCategory();

  this->populateAnatomicContextComboBox();
  d->resetCurrentRegion();
}
