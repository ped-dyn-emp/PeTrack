/********************************************************************************
** Form generated from reading UI file 'colorMarker.ui'
**
** Created by: Qt User Interface Compiler version 5.14.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_COLORMARKER_H
#define UI_COLORMARKER_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
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
            ColorMarker->setObjectName(QString::fromUtf8("ColorMarker"));
        ColorMarker->resize(267, 287);
        verticalLayout = new QVBoxLayout(ColorMarker);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        groupBox = new QGroupBox(ColorMarker);
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

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        showMask = new QCheckBox(ColorMarker);
        showMask->setObjectName(QString::fromUtf8("showMask"));

        horizontalLayout_2->addWidget(showMask);


        verticalLayout->addLayout(horizontalLayout_2);

        gridLayout = new QGridLayout();
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        label = new QLabel(ColorMarker);
        label->setObjectName(QString::fromUtf8("label"));

        gridLayout->addWidget(label, 1, 0, 1, 1);

        closeRadius = new QSpinBox(ColorMarker);
        closeRadius->setObjectName(QString::fromUtf8("closeRadius"));
        closeRadius->setKeyboardTracking(false);
        closeRadius->setMinimum(0);
        closeRadius->setMaximum(99);
        closeRadius->setSingleStep(1);
        closeRadius->setValue(5);

        gridLayout->addWidget(closeRadius, 1, 1, 1, 1);

        label_2 = new QLabel(ColorMarker);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        gridLayout->addWidget(label_2, 2, 0, 1, 1);

        openRadius = new QSpinBox(ColorMarker);
        openRadius->setObjectName(QString::fromUtf8("openRadius"));
        openRadius->setKeyboardTracking(false);
        openRadius->setMinimum(0);
        openRadius->setMaximum(99);
        openRadius->setSingleStep(1);
        openRadius->setValue(5);

        gridLayout->addWidget(openRadius, 2, 1, 1, 1);

        useOpen = new QCheckBox(ColorMarker);
        useOpen->setObjectName(QString::fromUtf8("useOpen"));
        useOpen->setChecked(true);

        gridLayout->addWidget(useOpen, 2, 2, 1, 1);

        label_3 = new QLabel(ColorMarker);
        label_3->setObjectName(QString::fromUtf8("label_3"));

        gridLayout->addWidget(label_3, 3, 0, 1, 1);

        minArea = new QSpinBox(ColorMarker);
        minArea->setObjectName(QString::fromUtf8("minArea"));
        minArea->setKeyboardTracking(false);
        minArea->setMinimum(1);
        minArea->setMaximum(100000);
        minArea->setSingleStep(100);
        minArea->setValue(1000);

        gridLayout->addWidget(minArea, 3, 1, 1, 1);

        label_4 = new QLabel(ColorMarker);
        label_4->setObjectName(QString::fromUtf8("label_4"));

        gridLayout->addWidget(label_4, 4, 0, 1, 1);

        label_5 = new QLabel(ColorMarker);
        label_5->setObjectName(QString::fromUtf8("label_5"));

        gridLayout->addWidget(label_5, 8, 0, 1, 1);

        opacity = new QSpinBox(ColorMarker);
        opacity->setObjectName(QString::fromUtf8("opacity"));
        opacity->setMaximum(100);
        opacity->setValue(100);

        gridLayout->addWidget(opacity, 8, 1, 1, 1);

        maskMask = new QCheckBox(ColorMarker);
        maskMask->setObjectName(QString::fromUtf8("maskMask"));
        maskMask->setChecked(true);

        gridLayout->addWidget(maskMask, 8, 2, 1, 1);

        maxRatio = new QDoubleSpinBox(ColorMarker);
        maxRatio->setObjectName(QString::fromUtf8("maxRatio"));
        maxRatio->setMinimum(1.000000000000000);
        maxRatio->setValue(2.000000000000000);

        gridLayout->addWidget(maxRatio, 4, 1, 1, 1);

        useClose = new QCheckBox(ColorMarker);
        useClose->setObjectName(QString::fromUtf8("useClose"));
        useClose->setChecked(true);

        gridLayout->addWidget(useClose, 1, 2, 1, 1);

        maxArea = new QSpinBox(ColorMarker);
        maxArea->setObjectName(QString::fromUtf8("maxArea"));
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
        groupBox->setTitle(QCoreApplication::translate("ColorMarker", "HSV color range", nullptr));
        fromColor->setText(QCoreApplication::translate("ColorMarker", "from", nullptr));
        toColor->setText(QCoreApplication::translate("ColorMarker", "to", nullptr));
        inversHue->setText(QCoreApplication::translate("ColorMarker", "invers hue", nullptr));
#if QT_CONFIG(tooltip)
        showMask->setToolTip(QCoreApplication::translate("ColorMarker", "show mask in main window", nullptr));
#endif // QT_CONFIG(tooltip)
        showMask->setText(QCoreApplication::translate("ColorMarker", "show mask", nullptr));
        label->setText(QCoreApplication::translate("ColorMarker", "close radius", nullptr));
        label_2->setText(QCoreApplication::translate("ColorMarker", "open radius", nullptr));
        useOpen->setText(QCoreApplication::translate("ColorMarker", "use", nullptr));
        label_3->setText(QCoreApplication::translate("ColorMarker", "area", nullptr));
        label_4->setText(QCoreApplication::translate("ColorMarker", "max ratio", nullptr));
        label_5->setText(QCoreApplication::translate("ColorMarker", "opacity", nullptr));
        maskMask->setText(QCoreApplication::translate("ColorMarker", "mask", nullptr));
        useClose->setText(QCoreApplication::translate("ColorMarker", "use", nullptr));
        (void)ColorMarker;
    } // retranslateUi

};

namespace Ui {
    class ColorMarker: public Ui_ColorMarker {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_COLORMARKER_H
