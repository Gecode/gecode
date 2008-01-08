/*
 *  Main authors:
 *     Niko Paltzer <nikopp@ps.uni-sb.de>
 *
 *  Copyright:
 *     Niko Paltzer, 2007
 *
 *  Last modified:
 *     $Date: 2007-11-29 19:46:48 +0100 (Thu, 29 Nov 2007) $ by $Author: schulte $
 *     $Revision: 5506 $
 *
 *  This file is part of Gecode, the generic constraint
 *  development environment:
 *     http://www.gecode.org
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

/********************************************************************************
 ** Form generated from reading ui file 'addvisualisationdialog.ui'
 **
 ** Created: Tue Jan 8 11:36:56 2008
 **      by: Qt User Interface Compiler version 4.3.2
 **
 ** WARNING! All changes made in this file will be lost when recompiling ui file!
 ********************************************************************************/

#ifndef GECODE_GIST_UI_ADDVISUALISATIONDIALOG_HH
#define GECODE_GIST_UI_ADDVISUALISATIONDIALOG_HH

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDialog>
#include <QtGui/QGridLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QListWidget>
#include <QtGui/QPushButton>
#include <QtGui/QSpacerItem>

namespace Gecode { namespace Gist {

  class Ui_AddVisualisationDialogClass
  {
  public:
    QGridLayout *gridLayout;
    QListWidget *visualisationsListWidget;
    QListWidget *variablesListWidget;
    QHBoxLayout *hboxLayout;
    QSpacerItem *spacerItem;
    QPushButton *OKpushButton;
    QPushButton *cancelPushButton;
    QHBoxLayout *hboxLayout1;
    QLabel *nameLabel;
    QLineEdit *nameEdit;

    void setupUi(QDialog *AddVisualisationDialogClass)
      {
        if (AddVisualisationDialogClass->objectName().isEmpty())
          AddVisualisationDialogClass->setObjectName(QString::fromUtf8("AddVisualisationDialogClass"));
        AddVisualisationDialogClass->resize(400, 300);
        gridLayout = new QGridLayout(AddVisualisationDialogClass);
        gridLayout->setSpacing(6);
        gridLayout->setMargin(11);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        visualisationsListWidget = new QListWidget(AddVisualisationDialogClass);
        visualisationsListWidget->setObjectName(QString::fromUtf8("visualisationsListWidget"));

        gridLayout->addWidget(visualisationsListWidget, 0, 0, 1, 1);

        variablesListWidget = new QListWidget(AddVisualisationDialogClass);
        variablesListWidget->setObjectName(QString::fromUtf8("variablesListWidget"));
        variablesListWidget->setSelectionMode(QAbstractItemView::MultiSelection);

        gridLayout->addWidget(variablesListWidget, 0, 1, 1, 1);

        hboxLayout = new QHBoxLayout();
        hboxLayout->setSpacing(6);
        hboxLayout->setObjectName(QString::fromUtf8("hboxLayout"));
        spacerItem = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        hboxLayout->addItem(spacerItem);

        OKpushButton = new QPushButton(AddVisualisationDialogClass);
        OKpushButton->setObjectName(QString::fromUtf8("OKpushButton"));
        OKpushButton->setEnabled(false);

        hboxLayout->addWidget(OKpushButton);

        cancelPushButton = new QPushButton(AddVisualisationDialogClass);
        cancelPushButton->setObjectName(QString::fromUtf8("cancelPushButton"));

        hboxLayout->addWidget(cancelPushButton);


        gridLayout->addLayout(hboxLayout, 2, 0, 1, 2);

        hboxLayout1 = new QHBoxLayout();
        hboxLayout1->setSpacing(6);
        hboxLayout1->setObjectName(QString::fromUtf8("hboxLayout1"));
        nameLabel = new QLabel(AddVisualisationDialogClass);
        nameLabel->setObjectName(QString::fromUtf8("nameLabel"));

        hboxLayout1->addWidget(nameLabel);

        nameEdit = new QLineEdit(AddVisualisationDialogClass);
        nameEdit->setObjectName(QString::fromUtf8("nameEdit"));

        hboxLayout1->addWidget(nameEdit);


        gridLayout->addLayout(hboxLayout1, 1, 0, 1, 2);


        retranslateUi(AddVisualisationDialogClass);
        QObject::connect(OKpushButton, SIGNAL(clicked()), AddVisualisationDialogClass, SLOT(accept()));
        QObject::connect(cancelPushButton, SIGNAL(clicked()), AddVisualisationDialogClass, SLOT(reject()));

        QMetaObject::connectSlotsByName(AddVisualisationDialogClass);
      } // setupUi

    void retranslateUi(QDialog *AddVisualisationDialogClass)
      {
        AddVisualisationDialogClass->setWindowTitle(QApplication::translate("AddVisualisationDialogClass", "Add Visualisation Dialog", 0, QApplication::UnicodeUTF8));
        OKpushButton->setText(QApplication::translate("AddVisualisationDialogClass", "OK", 0, QApplication::UnicodeUTF8));
        cancelPushButton->setText(QApplication::translate("AddVisualisationDialogClass", "Cancel", 0, QApplication::UnicodeUTF8));
        nameLabel->setText(QApplication::translate("AddVisualisationDialogClass", "Name", 0, QApplication::UnicodeUTF8));
        Q_UNUSED(AddVisualisationDialogClass);
      } // retranslateUi

  };

  namespace Ui {
    class AddVisualisationDialogClass: public Ui_AddVisualisationDialogClass {};
  } // namespace Ui


#endif

}}

// STATISTICS: gist-any
