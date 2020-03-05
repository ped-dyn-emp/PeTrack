/********************************************************************************
** Form generated from reading UI file 'colorRange.ui'
**
** Created by: Qt User Interface Compiler version 5.14.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_COLORRANGE_H
#define UI_COLORRANGE_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include "qtColorTriangle.h"

QT_BEGIN_NAMESPACE

class Ui_ColorRange
{
public:
    QVBoxLayout *verticalLayout;
    QGroupBox *groupBox;
    QGridLayout *gridLayout_2;
    QPushButton *fromColor;
    QPushButton *toColor;
    QtColorTriangle *fromTriangle;
    QtColorTriangle *toTriangle;
    QCheckBox *inversHue;

    void setupUi(QWidget *ColorRange)
    {
        if (ColorRange->objectName().isEmpty())
            ColorRange->setObjectName(QString::fromUtf8("ColorRange"));
        ColorRange->resize(267, 287);
        verticalLayout = new QVBoxLayout(ColorRange);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        groupBox = new QGroupBox(ColorRange);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        gridLayout_2 = new QGridLayout(groupBox);
        gridLayout_2->setObjectName(QString::fromUtf8("gridLayout_2"));
        fromColor = new QPushButton(groupBox);
        fromColor->setObjectName(QString::fromUtf8("fromColor"));

        gridLayout_2->addWidget(fromColor, 0, 0, 1, 1);

        toColor = new QPushButton(groupBox);
        toColor->setObjectName(QString::fromUtf8("toColor"));

        gridLayout_2->addWidget(toColor, 0, 1, 1, 1);

        fromTriangle = new QtColorTriangle(groupBox);
        fromTriangle->setObjectName(QString::fromUtf8("fromTriangle"));

        gridLayout_2->addWidget(fromTriangle, 1, 0, 1, 1);

        toTriangle = new QtColorTriangle(groupBox);
        toTriangle->setObjectName(QString::fromUtf8("toTriangle"));

        gridLayout_2->addWidget(toTriangle, 1, 1, 1, 1);

        inversHue = new QCheckBox(groupBox);
        inversHue->setObjectName(QString::fromUtf8("inversHue"));

        gridLayout_2->addWidget(inversHue, 2, 0, 1, 1);


        verticalLayout->addWidget(groupBox);


        retranslateUi(ColorRange);

        QMetaObject::connectSlotsByName(ColorRange);
    } // setupUi

    void retranslateUi(QWidget *ColorRange)
    {
        groupBox->setTitle(QCoreApplication::translate("ColorRange", "HSV color range", nullptr));
        fromColor->setText(QCoreApplication::translate("ColorRange", "from", nullptr));
        toColor->setText(QCoreApplication::translate("ColorRange", "to", nullptr));
        inversHue->setText(QCoreApplication::translate("ColorRange", "invers hue", nullptr));
        (void)ColorRange;
    } // retranslateUi

};

namespace Ui {
    class ColorRange: public Ui_ColorRange {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_COLORRANGE_H
