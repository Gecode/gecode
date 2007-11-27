/*
 *  Main authors:
 *     Niko Paltzer <nikopp@ps.uni-sb.de>
 *
 *  Copyright:
 *     Niko Paltzer, 2007
 *
 *  Last modified:
 *     $Date: 2007-11-26 17:01:39 +0100 (Mon, 26 Nov 2007) $ by $Author: nikopp $
 *     $Revision: 5439 $
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

#ifndef GECODE_GIST_UI_ADDCHILD_HH
#define GECODE_GIST_UI_ADDCHILD_HH

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDialog>
#include <QtGui/QHBoxLayout>
#include <QtGui/QListWidget>
#include <QtGui/QPushButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QSpinBox>
#include <QtGui/QVBoxLayout>

namespace Gecode { namespace Gist {

class Ui_AddChildClass
{
public:
    QVBoxLayout *vboxLayout;
    QHBoxLayout *hboxLayout;
    QListWidget *varList;
    QListWidget *relList;
    QSpinBox *valueSpinBox;
    QSpacerItem *spacerItem;
    QHBoxLayout *hboxLayout1;
    QSpacerItem *spacerItem1;
    QPushButton *okPushButton;
    QPushButton *cancelPushButton;

    void setupUi(QDialog *AddChildClass)
    {
    if (AddChildClass->objectName().isEmpty())
        AddChildClass->setObjectName(QString::fromUtf8("AddChildClass"));
    AddChildClass->resize(425, 251);
    QSizePolicy sizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(AddChildClass->sizePolicy().hasHeightForWidth());
    AddChildClass->setSizePolicy(sizePolicy);
    vboxLayout = new QVBoxLayout(AddChildClass);
    vboxLayout->setSpacing(6);
    vboxLayout->setMargin(11);
    vboxLayout->setObjectName(QString::fromUtf8("vboxLayout"));
    hboxLayout = new QHBoxLayout();
    hboxLayout->setSpacing(6);
    hboxLayout->setObjectName(QString::fromUtf8("hboxLayout"));
    varList = new QListWidget(AddChildClass);
    varList->setObjectName(QString::fromUtf8("varList"));

    hboxLayout->addWidget(varList);

    relList = new QListWidget(AddChildClass);
    relList->setObjectName(QString::fromUtf8("relList"));

    hboxLayout->addWidget(relList);

    valueSpinBox = new QSpinBox(AddChildClass);
    valueSpinBox->setObjectName(QString::fromUtf8("valueSpinBox"));

    hboxLayout->addWidget(valueSpinBox);


    vboxLayout->addLayout(hboxLayout);

    spacerItem = new QSpacerItem(382, 16, QSizePolicy::Minimum, QSizePolicy::Expanding);

    vboxLayout->addItem(spacerItem);

    hboxLayout1 = new QHBoxLayout();
    hboxLayout1->setSpacing(6);
    hboxLayout1->setObjectName(QString::fromUtf8("hboxLayout1"));
    spacerItem1 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

    hboxLayout1->addItem(spacerItem1);

    okPushButton = new QPushButton(AddChildClass);
    okPushButton->setObjectName(QString::fromUtf8("okPushButton"));
    okPushButton->setEnabled(false);

    hboxLayout1->addWidget(okPushButton);

    cancelPushButton = new QPushButton(AddChildClass);
    cancelPushButton->setObjectName(QString::fromUtf8("cancelPushButton"));

    hboxLayout1->addWidget(cancelPushButton);


    vboxLayout->addLayout(hboxLayout1);


    retranslateUi(AddChildClass);
    QObject::connect(okPushButton, SIGNAL(clicked()), AddChildClass, SLOT(accept()));
    QObject::connect(cancelPushButton, SIGNAL(clicked()), AddChildClass, SLOT(reject()));

    QMetaObject::connectSlotsByName(AddChildClass);
    } // setupUi

    void retranslateUi(QDialog *AddChildClass)
    {
    AddChildClass->setWindowTitle(QApplication::translate("AddChildClass", "Add Child Node", 0, QApplication::UnicodeUTF8));
    okPushButton->setText(QApplication::translate("AddChildClass", "OK", 0, QApplication::UnicodeUTF8));
    cancelPushButton->setText(QApplication::translate("AddChildClass", "Cancel", 0, QApplication::UnicodeUTF8));
    Q_UNUSED(AddChildClass);
    } // retranslateUi

};

namespace Ui {
    class AddChildClass: public Ui_AddChildClass {};
} // namespace Ui

#endif

}}

// STATISTICS: gist-any
