/********************************************************************************
** Form generated from reading UI file 'colorRange.ui'
**
** Created by: Qt User Interface Compiler version 5.5.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_COLORRANGE_H
#define UI_COLORRANGE_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHeaderView>
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
            ColorRange->setObjectName(QStringLiteral("ColorRange"));
        ColorRange->resize(267, 287);
        verticalLayout = new QVBoxLayout(ColorRange);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        groupBox = new QGroupBox(ColorRange);
        groupBox->setObjectName(QStringLiteral("groupBox"));
        gridLayout_2 = new QGridLayout(groupBox);
        gridLayout_2->setObjectName(QStringLiteral("gridLayout_2"));
        fromColor = new QPushButton(groupBox);
        fromColor->setObjectName(QStringLiteral("fromColor"));

        gridLayout_2->addWidget(fromColor, 0, 0, 1, 1);

        toColor = new QPushButton(groupBox);
        toColor->setObjectName(QStringLiteral("toColor"));

        gridLayout_2->addWidget(toColor, 0, 1, 1, 1);

        fromTriangle = new QtColorTriangle(groupBox);
        fromTriangle->setObjectName(QStringLiteral("fromTriangle"));

        gridLayout_2->addWidget(fromTriangle, 1, 0, 1, 1);

        toTriangle = new QtColorTriangle(groupBox);
        toTriangle->setObjectName(QStringLiteral("toTriangle"));

        gridLayout_2->addWidget(toTriangle, 1, 1, 1, 1);

        inversHue = new QCheckBox(groupBox);
        inversHue->setObjectName(QStringLiteral("inversHue"));

        gridLayout_2->addWidget(inversHue, 2, 0, 1, 1);


        verticalLayout->addWidget(groupBox);


        retranslateUi(ColorRange);

        QMetaObject::connectSlotsByName(ColorRange);
    } // setupUi

    void retranslateUi(QWidget *ColorRange)
    {
        groupBox->setTitle(QApplication::translate("ColorRange", "HSV color range", 0));
        fromColor->setText(QApplication::translate("ColorRange", "from", 0));
        toColor->setText(QApplication::translate("ColorRange", "to", 0));
        inversHue->setText(QApplication::translate("ColorRange", "invers hue", 0));
        Q_UNUSED(ColorRange);
    } // retranslateUi

};

namespace Ui {
    class ColorRange: public Ui_ColorRange {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_COLORRANGE_H
