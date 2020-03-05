/********************************************************************************
** Form generated from reading UI file 'multiColorMarker.ui'
**
** Created by: Qt User Interface Compiler version 5.14.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MULTICOLORMARKER_H
#define UI_MULTICOLORMARKER_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MultiColorMarker
{
public:
    QVBoxLayout *verticalLayout;
    QVBoxLayout *verticalLayout_2;
    QGridLayout *gridLayout_3;
    QCheckBox *useDot;
    QDoubleSpinBox *dotSize;
    QSpacerItem *horizontalSpacer_2;
    QGridLayout *gridLayout_2;
    QCheckBox *ignoreWithoutDot;
    QSpacerItem *horizontalSpacer;
    QCheckBox *useColor;
    QCheckBox *restrictPosition;
    QHBoxLayout *horizontalLayout;
    QCheckBox *autoCorrect;
    QCheckBox *autoCorrectOnlyExport;
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
    QCheckBox *useHeadSize;

    void setupUi(QWidget *MultiColorMarker)
    {
        if (MultiColorMarker->objectName().isEmpty())
            MultiColorMarker->setObjectName(QString::fromUtf8("MultiColorMarker"));
        MultiColorMarker->resize(369, 370);
        verticalLayout = new QVBoxLayout(MultiColorMarker);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        verticalLayout_2 = new QVBoxLayout();
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        gridLayout_3 = new QGridLayout();
        gridLayout_3->setObjectName(QString::fromUtf8("gridLayout_3"));
        useDot = new QCheckBox(MultiColorMarker);
        useDot->setObjectName(QString::fromUtf8("useDot"));
        useDot->setChecked(true);

        gridLayout_3->addWidget(useDot, 0, 0, 1, 1);

        dotSize = new QDoubleSpinBox(MultiColorMarker);
        dotSize->setObjectName(QString::fromUtf8("dotSize"));
        dotSize->setMinimum(0.100000000000000);
        dotSize->setValue(5.000000000000000);

        gridLayout_3->addWidget(dotSize, 0, 1, 1, 1);

        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout_3->addItem(horizontalSpacer_2, 0, 2, 1, 1);


        verticalLayout_2->addLayout(gridLayout_3);

        gridLayout_2 = new QGridLayout();
        gridLayout_2->setObjectName(QString::fromUtf8("gridLayout_2"));
        ignoreWithoutDot = new QCheckBox(MultiColorMarker);
        ignoreWithoutDot->setObjectName(QString::fromUtf8("ignoreWithoutDot"));
        ignoreWithoutDot->setChecked(true);

        gridLayout_2->addWidget(ignoreWithoutDot, 0, 1, 1, 1);

        horizontalSpacer = new QSpacerItem(20, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);

        gridLayout_2->addItem(horizontalSpacer, 0, 0, 1, 1);

        useColor = new QCheckBox(MultiColorMarker);
        useColor->setObjectName(QString::fromUtf8("useColor"));

        gridLayout_2->addWidget(useColor, 1, 1, 1, 1);

        restrictPosition = new QCheckBox(MultiColorMarker);
        restrictPosition->setObjectName(QString::fromUtf8("restrictPosition"));

        gridLayout_2->addWidget(restrictPosition, 2, 1, 1, 1);


        verticalLayout_2->addLayout(gridLayout_2);


        verticalLayout->addLayout(verticalLayout_2);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        autoCorrect = new QCheckBox(MultiColorMarker);
        autoCorrect->setObjectName(QString::fromUtf8("autoCorrect"));

        horizontalLayout->addWidget(autoCorrect);

        autoCorrectOnlyExport = new QCheckBox(MultiColorMarker);
        autoCorrectOnlyExport->setObjectName(QString::fromUtf8("autoCorrectOnlyExport"));

        horizontalLayout->addWidget(autoCorrectOnlyExport);


        verticalLayout->addLayout(horizontalLayout);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        showMask = new QCheckBox(MultiColorMarker);
        showMask->setObjectName(QString::fromUtf8("showMask"));

        horizontalLayout_2->addWidget(showMask);


        verticalLayout->addLayout(horizontalLayout_2);

        gridLayout = new QGridLayout();
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        label = new QLabel(MultiColorMarker);
        label->setObjectName(QString::fromUtf8("label"));

        gridLayout->addWidget(label, 1, 0, 1, 1);

        closeRadius = new QSpinBox(MultiColorMarker);
        closeRadius->setObjectName(QString::fromUtf8("closeRadius"));
        closeRadius->setKeyboardTracking(false);
        closeRadius->setMinimum(0);
        closeRadius->setMaximum(99);
        closeRadius->setSingleStep(1);
        closeRadius->setValue(5);

        gridLayout->addWidget(closeRadius, 1, 1, 1, 1);

        label_2 = new QLabel(MultiColorMarker);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        gridLayout->addWidget(label_2, 2, 0, 1, 1);

        openRadius = new QSpinBox(MultiColorMarker);
        openRadius->setObjectName(QString::fromUtf8("openRadius"));
        openRadius->setKeyboardTracking(false);
        openRadius->setMinimum(0);
        openRadius->setMaximum(99);
        openRadius->setSingleStep(1);
        openRadius->setValue(5);

        gridLayout->addWidget(openRadius, 2, 1, 1, 1);

        useOpen = new QCheckBox(MultiColorMarker);
        useOpen->setObjectName(QString::fromUtf8("useOpen"));
        useOpen->setChecked(true);

        gridLayout->addWidget(useOpen, 2, 2, 1, 1);

        label_3 = new QLabel(MultiColorMarker);
        label_3->setObjectName(QString::fromUtf8("label_3"));

        gridLayout->addWidget(label_3, 3, 0, 1, 1);

        minArea = new QSpinBox(MultiColorMarker);
        minArea->setObjectName(QString::fromUtf8("minArea"));
        minArea->setKeyboardTracking(false);
        minArea->setMinimum(1);
        minArea->setMaximum(100000);
        minArea->setSingleStep(100);
        minArea->setValue(1000);

        gridLayout->addWidget(minArea, 3, 1, 1, 1);

        label_4 = new QLabel(MultiColorMarker);
        label_4->setObjectName(QString::fromUtf8("label_4"));

        gridLayout->addWidget(label_4, 4, 0, 1, 1);

        label_5 = new QLabel(MultiColorMarker);
        label_5->setObjectName(QString::fromUtf8("label_5"));

        gridLayout->addWidget(label_5, 8, 0, 1, 1);

        opacity = new QSpinBox(MultiColorMarker);
        opacity->setObjectName(QString::fromUtf8("opacity"));
        opacity->setMaximum(100);
        opacity->setValue(100);

        gridLayout->addWidget(opacity, 8, 1, 1, 1);

        maskMask = new QCheckBox(MultiColorMarker);
        maskMask->setObjectName(QString::fromUtf8("maskMask"));
        maskMask->setChecked(true);

        gridLayout->addWidget(maskMask, 8, 2, 1, 1);

        maxRatio = new QDoubleSpinBox(MultiColorMarker);
        maxRatio->setObjectName(QString::fromUtf8("maxRatio"));
        maxRatio->setMinimum(1.000000000000000);
        maxRatio->setValue(2.000000000000000);

        gridLayout->addWidget(maxRatio, 4, 1, 1, 1);

        useClose = new QCheckBox(MultiColorMarker);
        useClose->setObjectName(QString::fromUtf8("useClose"));
        useClose->setChecked(true);

        gridLayout->addWidget(useClose, 1, 2, 1, 1);

        maxArea = new QSpinBox(MultiColorMarker);
        maxArea->setObjectName(QString::fromUtf8("maxArea"));
        maxArea->setMinimum(1);
        maxArea->setMaximum(100000);
        maxArea->setSingleStep(100);
        maxArea->setValue(5000);

        gridLayout->addWidget(maxArea, 3, 2, 1, 1);

        useHeadSize = new QCheckBox(MultiColorMarker);
        useHeadSize->setObjectName(QString::fromUtf8("useHeadSize"));

        gridLayout->addWidget(useHeadSize, 3, 3, 1, 1);


        verticalLayout->addLayout(gridLayout);


        retranslateUi(MultiColorMarker);

        QMetaObject::connectSlotsByName(MultiColorMarker);
    } // setupUi

    void retranslateUi(QWidget *MultiColorMarker)
    {
#if QT_CONFIG(tooltip)
        useDot->setToolTip(QCoreApplication::translate("MultiColorMarker", "using black dot on top of the hat for tracking", nullptr));
#endif // QT_CONFIG(tooltip)
        useDot->setText(QCoreApplication::translate("MultiColorMarker", "use black dot, size [cm]:", nullptr));
#if QT_CONFIG(tooltip)
        ignoreWithoutDot->setToolTip(QCoreApplication::translate("MultiColorMarker", "ignore head without black dot, if not visible / surrounded by color", nullptr));
#endif // QT_CONFIG(tooltip)
        ignoreWithoutDot->setText(QCoreApplication::translate("MultiColorMarker", "ignore head without black dot", nullptr));
#if QT_CONFIG(tooltip)
        useColor->setToolTip(QCoreApplication::translate("MultiColorMarker", "use color marker while tracking for big tracking error", nullptr));
#endif // QT_CONFIG(tooltip)
        useColor->setText(QCoreApplication::translate("MultiColorMarker", "use color marker for big error", nullptr));
#if QT_CONFIG(tooltip)
        restrictPosition->setToolTip(QCoreApplication::translate("MultiColorMarker", "restrict position of black dot on centre of head according to viewing angle", nullptr));
#endif // QT_CONFIG(tooltip)
        restrictPosition->setText(QCoreApplication::translate("MultiColorMarker", "restrict position of black dot", nullptr));
#if QT_CONFIG(tooltip)
        autoCorrect->setToolTip(QCoreApplication::translate("MultiColorMarker", "automatically correct the position of the person according to angle of view", nullptr));
#endif // QT_CONFIG(tooltip)
        autoCorrect->setText(QCoreApplication::translate("MultiColorMarker", "auto correct perspective view", nullptr));
#if QT_CONFIG(tooltip)
        autoCorrectOnlyExport->setToolTip(QCoreApplication::translate("MultiColorMarker", "if auto correct is enabled only the exported real trajectories will be corrected", nullptr));
#endif // QT_CONFIG(tooltip)
        autoCorrectOnlyExport->setText(QCoreApplication::translate("MultiColorMarker", "only for export", nullptr));
#if QT_CONFIG(tooltip)
        showMask->setToolTip(QCoreApplication::translate("MultiColorMarker", "show mask in main window", nullptr));
#endif // QT_CONFIG(tooltip)
        showMask->setText(QCoreApplication::translate("MultiColorMarker", "show mask", nullptr));
        label->setText(QCoreApplication::translate("MultiColorMarker", "close radius", nullptr));
        label_2->setText(QCoreApplication::translate("MultiColorMarker", "open radius", nullptr));
        useOpen->setText(QCoreApplication::translate("MultiColorMarker", "use", nullptr));
        label_3->setText(QCoreApplication::translate("MultiColorMarker", "area", nullptr));
        label_4->setText(QCoreApplication::translate("MultiColorMarker", "max ratio", nullptr));
        label_5->setText(QCoreApplication::translate("MultiColorMarker", "opacity", nullptr));
        maskMask->setText(QCoreApplication::translate("MultiColorMarker", "mask", nullptr));
        useClose->setText(QCoreApplication::translate("MultiColorMarker", "use", nullptr));
        useHeadSize->setText(QCoreApplication::translate("MultiColorMarker", "head size", nullptr));
        (void)MultiColorMarker;
    } // retranslateUi

};

namespace Ui {
    class MultiColorMarker: public Ui_MultiColorMarker {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MULTICOLORMARKER_H
