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
#include "qSlicerTerminologyItemDelegate.h"

#include "qSlicerTerminologySelectorButton.h"
#include "qSlicerTerminologyNavigatorWidget.h"

#include "vtkSlicerTerminologyEntry.h"

// VTK includes
#include <vtkSmartPointer.h>

// Qt includes
#include <QDebug>

//-----------------------------------------------------------------------------
qSlicerTerminologyItemDelegate::qSlicerTerminologyItemDelegate(QObject *parent)
  : QStyledItemDelegate(parent) { }

//-----------------------------------------------------------------------------
QWidget* qSlicerTerminologyItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &/* option */, const QModelIndex &/* index */) const
{
  qSlicerTerminologySelectorButton* terminologyButton = new qSlicerTerminologySelectorButton(parent);
  terminologyButton->setProperty("changeDataOnSet", true);
  connect(terminologyButton, SIGNAL(terminologyChanged()), this, SLOT(commitAndClose()), Qt::QueuedConnection);
  connect(terminologyButton, SIGNAL(canceled()), this, SLOT(close()), Qt::QueuedConnection);
  return terminologyButton;
}

//-----------------------------------------------------------------------------
void qSlicerTerminologyItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
  // Set terminology to button
  qSlicerTerminologySelectorButton* terminologyButton = qobject_cast<qSlicerTerminologySelectorButton*>(editor);
  if (terminologyButton->property("changeDataOnSet").toBool())
    {
    terminologyButton->setProperty("changeDataOnSet", false);

    // Get string list value from model index
    QString terminologyString = index.model()->data(index, Qt::WhatsThisRole).toString();

    // Convert string list to VTK terminology entry. Do not check success, as an empty terminology is also a valid starting point
    vtkSmartPointer<vtkSlicerTerminologyEntry> terminologyEntry = vtkSmartPointer<vtkSlicerTerminologyEntry>::New();
    qSlicerTerminologyNavigatorWidget::deserializeTerminologyEntry(terminologyString, terminologyEntry);
    // Get color
    QColor color = index.model()->data(index, Qt::DecorationRole).value<QColor>();

    terminologyButton->setTerminologyEntry(terminologyEntry, false);
    terminologyButton->setColor(color);

    terminologyButton->changeTerminology();
    }
}

//-----------------------------------------------------------------------------
void qSlicerTerminologyItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
  // Get terminology (changed by the user) from the terminology button
  qSlicerTerminologySelectorButton* terminologyButton = qobject_cast<qSlicerTerminologySelectorButton*>(editor);
  vtkSlicerTerminologyEntry* terminologyEntry = terminologyButton->terminologyEntry();

  // Get recommended color from terminology if custom color is invalid (which means it was not changed by the user)
  QColor color = terminologyButton->color();
  if (!color.isValid())
    {
    color = qSlicerTerminologyNavigatorWidget::recommendedColorFromTerminology(terminologyEntry);
    }

  // Set color to model
  model->setData(index, color, Qt::DecorationRole);
  // Set terminology string to model
  model->setData(index, qSlicerTerminologyNavigatorWidget::serializeTerminologyEntry(terminologyEntry), Qt::WhatsThisRole);
}

//-----------------------------------------------------------------------------
void qSlicerTerminologyItemDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &/* index */) const
{
  editor->setGeometry(option.rect);
}

//------------------------------------------------------------------------------
void qSlicerTerminologyItemDelegate::commitSenderData()
{
  QWidget* editor = qobject_cast<QWidget*>(this->sender());
  emit commitData(editor);
}

//------------------------------------------------------------------------------
void qSlicerTerminologyItemDelegate::commitAndClose()
{
  QWidget* editor = qobject_cast<QWidget*>(this->sender());
  emit commitData(editor);
  emit closeEditor(editor);
}

//------------------------------------------------------------------------------
void qSlicerTerminologyItemDelegate::close()
{
  QWidget* editor = qobject_cast<QWidget*>(this->sender());
  emit closeEditor(editor);
}
