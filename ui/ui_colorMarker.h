/********************************************************************************
** Form generated from reading UI file 'colorMarker.ui'
**
** Created by: Qt User Interface Compiler version 5.5.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_COLORMARKER_H
#define UI_COLORMARKER_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include "qtColorTriangle.h"

QT_BEGIN_NAMESPACE

class Ui_ColorMarker
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
    QHBoxLayout *horizontalLayout_2;
    QCheckBox *showMask;
    QGridLayout *gridLayout;
    QLabel *label;
    QSpinBox *closeRadius;
    QLabel *label_2;
    QSpinBox *openRadius;
    QCheckBox *useOpen;
    QLabel *label_3;
    QSpinBox *minArea;
    QLabel *label_4;
    QLabel *label_5;
    QSpinBox *opacity;
    QCheckBox *maskMask;
    QDoubleSpinBox *maxRatio;
    QCheckBox *useClose;
    QSpinBox *maxArea;

    void setupUi(QWidget *ColorMarker)
    {
        if (ColorMarker->objectName().isEmpty())
            ColorMarker->setObjectName(QStringLiteral("ColorMarker"));
        ColorMarker->resize(267, 287);
        verticalLayout = new QVBoxLayout(ColorMarker);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        groupBox = new QGroupBox(ColorMarker);
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

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QStringLiteral("horizontalLayout_2"));
        showMask = new QCheckBox(ColorMarker);
        showMask->setObjectName(QStringLiteral("showMask"));

        horizontalLayout_2->addWidget(showMask);


        verticalLayout->addLayout(horizontalLayout_2);

        gridLayout = new QGridLayout();
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        label = new QLabel(ColorMarker);
        label->setObjectName(QStringLiteral("label"));

        gridLayout->addWidget(label, 1, 0, 1, 1);

        closeRadius = new QSpinBox(ColorMarker);
        closeRadius->setObjectName(QStringLiteral("closeRadius"));
        closeRadius->setKeyboardTracking(false);
        closeRadius->setMinimum(0);
        closeRadius->setMaximum(99);
        closeRadius->setSingleStep(1);
        closeRadius->setValue(5);

        gridLayout->addWidget(closeRadius, 1, 1, 1, 1);

        label_2 = new QLabel(ColorMarker);
        label_2->setObjectName(QStringLiteral("label_2"));

        gridLayout->addWidget(label_2, 2, 0, 1, 1);

        openRadius = new QSpinBox(ColorMarker);
        openRadius->setObjectName(QStringLiteral("openRadius"));
        openRadius->setKeyboardTracking(false);
        openRadius->setMinimum(0);
        openRadius->setMaximum(99);
        openRadius->setSingleStep(1);
        openRadius->setValue(5);

        gridLayout->addWidget(openRadius, 2, 1, 1, 1);

        useOpen = new QCheckBox(ColorMarker);
        useOpen->setObjectName(QStringLiteral("useOpen"));
        useOpen->setChecked(true);

        gridLayout->addWidget(useOpen, 2, 2, 1, 1);

        label_3 = new QLabel(ColorMarker);
        label_3->setObjectName(QStringLiteral("label_3"));

        gridLayout->addWidget(label_3, 3, 0, 1, 1);

        minArea = new QSpinBox(ColorMarker);
        minArea->setObjectName(QStringLiteral("minArea"));
        minArea->setKeyboardTracking(false);
        minArea->setMinimum(1);
        minArea->setMaximum(100000);
        minArea->setSingleStep(100);
        minArea->setValue(1000);

        gridLayout->addWidget(minArea, 3, 1, 1, 1);

        label_4 = new QLabel(ColorMarker);
        label_4->setObjectName(QStringLiteral("label_4"));

        gridLayout->addWidget(label_4, 4, 0, 1, 1);

        label_5 = new QLabel(ColorMarker);
        label_5->setObjectName(QStringLiteral("label_5"));

        gridLayout->addWidget(label_5, 8, 0, 1, 1);

        opacity = new QSpinBox(ColorMarker);
        opacity->setObjectName(QStringLiteral("opacity"));
        opacity->setMaximum(100);
        opacity->setValue(100);

        gridLayout->addWidget(opacity, 8, 1, 1, 1);

        maskMask = new QCheckBox(ColorMarker);
        maskMask->setObjectName(QStringLiteral("maskMask"));
        maskMask->setChecked(true);

        gridLayout->addWidget(maskMask, 8, 2, 1, 1);

        maxRatio = new QDoubleSpinBox(ColorMarker);
        maxRatio->setObjectName(QStringLiteral("maxRatio"));
        maxRatio->setMinimum(1);
        maxRatio->setValue(2);

        gridLayout->addWidget(maxRatio, 4, 1, 1, 1);

        useClose = new QCheckBox(ColorMarker);
        useClose->setObjectName(QStringLiteral("useClose"));
        useClose->setChecked(true);

        gridLayout->addWidget(useClose, 1, 2, 1, 1);

        maxArea = new QSpinBox(ColorMarker);
        maxArea->setObjectName(QStringLiteral("maxArea"));
        maxArea->setMinimum(1);
        maxArea->setMaximum(100000);
        maxArea->setSingleStep(100);
        maxArea->setValue(5000);

        gridLayout->addWidget(maxArea, 3, 2, 1, 1);


        verticalLayout->addLayout(gridLayout);


        retranslateUi(ColorMarker);

        QMetaObject::connectSlotsByName(ColorMarker);
    } // setupUi

    void retranslateUi(QWidget *ColorMarker)
    {
        groupBox->setTitle(QApplication::translate("ColorMarker", "HSV color range", 0));
        fromColor->setText(QApplication::translate("ColorMarker", "from", 0));
        toColor->setText(QApplication::translate("ColorMarker", "to", 0));
        inversHue->setText(QApplication::translate("ColorMarker", "invers hue", 0));
#ifndef QT_NO_TOOLTIP
        showMask->setToolTip(QApplication::translate("ColorMarker", "show mask in main window", 0));
#endif // QT_NO_TOOLTIP
        showMask->setText(QApplication::translate("ColorMarker", "show mask", 0));
        label->setText(QApplication::translate("ColorMarker", "close radius", 0));
        label_2->setText(QApplication::translate("ColorMarker", "open radius", 0));
        useOpen->setText(QApplication::translate("ColorMarker", "use", 0));
        label_3->setText(QApplication::translate("ColorMarker", "area", 0));
        label_4->setText(QApplication::translate("ColorMarker", "max ratio", 0));
        label_5->setText(QApplication::translate("ColorMarker", "opacity", 0));
        maskMask->setText(QApplication::translate("ColorMarker", "mask", 0));
        useClose->setText(QApplication::translate("ColorMarker", "use", 0));
        Q_UNUSED(ColorMarker);
    } // retranslateUi

};

namespace Ui {
    class ColorMarker: public Ui_ColorMarker {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_COLORMARKER_H
