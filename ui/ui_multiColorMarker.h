/********************************************************************************
** Form generated from reading UI file 'multiColorMarker.ui'
**
** Created by: Qt User Interface Compiler version 5.5.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MULTICOLORMARKER_H
#define UI_MULTICOLORMARKER_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
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
            MultiColorMarker->setObjectName(QStringLiteral("MultiColorMarker"));
        MultiColorMarker->resize(369, 370);
        verticalLayout = new QVBoxLayout(MultiColorMarker);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        verticalLayout_2 = new QVBoxLayout();
        verticalLayout_2->setObjectName(QStringLiteral("verticalLayout_2"));
        gridLayout_3 = new QGridLayout();
        gridLayout_3->setObjectName(QStringLiteral("gridLayout_3"));
        useDot = new QCheckBox(MultiColorMarker);
        useDot->setObjectName(QStringLiteral("useDot"));
        useDot->setChecked(true);

        gridLayout_3->addWidget(useDot, 0, 0, 1, 1);

        dotSize = new QDoubleSpinBox(MultiColorMarker);
        dotSize->setObjectName(QStringLiteral("dotSize"));
        dotSize->setMinimum(0.1);
        dotSize->setValue(5);

        gridLayout_3->addWidget(dotSize, 0, 1, 1, 1);

        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout_3->addItem(horizontalSpacer_2, 0, 2, 1, 1);


        verticalLayout_2->addLayout(gridLayout_3);

        gridLayout_2 = new QGridLayout();
        gridLayout_2->setObjectName(QStringLiteral("gridLayout_2"));
        ignoreWithoutDot = new QCheckBox(MultiColorMarker);
        ignoreWithoutDot->setObjectName(QStringLiteral("ignoreWithoutDot"));
        ignoreWithoutDot->setChecked(true);

        gridLayout_2->addWidget(ignoreWithoutDot, 0, 1, 1, 1);

        horizontalSpacer = new QSpacerItem(20, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);

        gridLayout_2->addItem(horizontalSpacer, 0, 0, 1, 1);

        useColor = new QCheckBox(MultiColorMarker);
        useColor->setObjectName(QStringLiteral("useColor"));

        gridLayout_2->addWidget(useColor, 1, 1, 1, 1);

        restrictPosition = new QCheckBox(MultiColorMarker);
        restrictPosition->setObjectName(QStringLiteral("restrictPosition"));

        gridLayout_2->addWidget(restrictPosition, 2, 1, 1, 1);


        verticalLayout_2->addLayout(gridLayout_2);


        verticalLayout->addLayout(verticalLayout_2);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        autoCorrect = new QCheckBox(MultiColorMarker);
        autoCorrect->setObjectName(QStringLiteral("autoCorrect"));

        horizontalLayout->addWidget(autoCorrect);

        autoCorrectOnlyExport = new QCheckBox(MultiColorMarker);
        autoCorrectOnlyExport->setObjectName(QStringLiteral("autoCorrectOnlyExport"));

        horizontalLayout->addWidget(autoCorrectOnlyExport);


        verticalLayout->addLayout(horizontalLayout);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QStringLiteral("horizontalLayout_2"));
        showMask = new QCheckBox(MultiColorMarker);
        showMask->setObjectName(QStringLiteral("showMask"));

        horizontalLayout_2->addWidget(showMask);


        verticalLayout->addLayout(horizontalLayout_2);

        gridLayout = new QGridLayout();
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        label = new QLabel(MultiColorMarker);
        label->setObjectName(QStringLiteral("label"));

        gridLayout->addWidget(label, 1, 0, 1, 1);

        closeRadius = new QSpinBox(MultiColorMarker);
        closeRadius->setObjectName(QStringLiteral("closeRadius"));
        closeRadius->setKeyboardTracking(false);
        closeRadius->setMinimum(0);
        closeRadius->setMaximum(99);
        closeRadius->setSingleStep(1);
        closeRadius->setValue(5);

        gridLayout->addWidget(closeRadius, 1, 1, 1, 1);

        label_2 = new QLabel(MultiColorMarker);
        label_2->setObjectName(QStringLiteral("label_2"));

        gridLayout->addWidget(label_2, 2, 0, 1, 1);

        openRadius = new QSpinBox(MultiColorMarker);
        openRadius->setObjectName(QStringLiteral("openRadius"));
        openRadius->setKeyboardTracking(false);
        openRadius->setMinimum(0);
        openRadius->setMaximum(99);
        openRadius->setSingleStep(1);
        openRadius->setValue(5);

        gridLayout->addWidget(openRadius, 2, 1, 1, 1);

        useOpen = new QCheckBox(MultiColorMarker);
        useOpen->setObjectName(QStringLiteral("useOpen"));
        useOpen->setChecked(true);

        gridLayout->addWidget(useOpen, 2, 2, 1, 1);

        label_3 = new QLabel(MultiColorMarker);
        label_3->setObjectName(QStringLiteral("label_3"));

        gridLayout->addWidget(label_3, 3, 0, 1, 1);

        minArea = new QSpinBox(MultiColorMarker);
        minArea->setObjectName(QStringLiteral("minArea"));
        minArea->setKeyboardTracking(false);
        minArea->setMinimum(1);
        minArea->setMaximum(100000);
        minArea->setSingleStep(100);
        minArea->setValue(1000);

        gridLayout->addWidget(minArea, 3, 1, 1, 1);

        label_4 = new QLabel(MultiColorMarker);
        label_4->setObjectName(QStringLiteral("label_4"));

        gridLayout->addWidget(label_4, 4, 0, 1, 1);

        label_5 = new QLabel(MultiColorMarker);
        label_5->setObjectName(QStringLiteral("label_5"));

        gridLayout->addWidget(label_5, 8, 0, 1, 1);

        opacity = new QSpinBox(MultiColorMarker);
        opacity->setObjectName(QStringLiteral("opacity"));
        opacity->setMaximum(100);
        opacity->setValue(100);

        gridLayout->addWidget(opacity, 8, 1, 1, 1);

        maskMask = new QCheckBox(MultiColorMarker);
        maskMask->setObjectName(QStringLiteral("maskMask"));
        maskMask->setChecked(true);

        gridLayout->addWidget(maskMask, 8, 2, 1, 1);

        maxRatio = new QDoubleSpinBox(MultiColorMarker);
        maxRatio->setObjectName(QStringLiteral("maxRatio"));
        maxRatio->setMinimum(1);
        maxRatio->setValue(2);

        gridLayout->addWidget(maxRatio, 4, 1, 1, 1);

        useClose = new QCheckBox(MultiColorMarker);
        useClose->setObjectName(QStringLiteral("useClose"));
        useClose->setChecked(true);

        gridLayout->addWidget(useClose, 1, 2, 1, 1);

        maxArea = new QSpinBox(MultiColorMarker);
        maxArea->setObjectName(QStringLiteral("maxArea"));
        maxArea->setMinimum(1);
        maxArea->setMaximum(100000);
        maxArea->setSingleStep(100);
        maxArea->setValue(5000);

        gridLayout->addWidget(maxArea, 3, 2, 1, 1);

        useHeadSize = new QCheckBox(MultiColorMarker);
        useHeadSize->setObjectName(QStringLiteral("useHeadSize"));

        gridLayout->addWidget(useHeadSize, 3, 3, 1, 1);


        verticalLayout->addLayout(gridLayout);


        retranslateUi(MultiColorMarker);

        QMetaObject::connectSlotsByName(MultiColorMarker);
    } // setupUi

    void retranslateUi(QWidget *MultiColorMarker)
    {
#ifndef QT_NO_TOOLTIP
        useDot->setToolTip(QApplication::translate("MultiColorMarker", "using black dot on top of the hat for tracking", 0));
#endif // QT_NO_TOOLTIP
        useDot->setText(QApplication::translate("MultiColorMarker", "use black dot, size [cm]:", 0));
#ifndef QT_NO_TOOLTIP
        ignoreWithoutDot->setToolTip(QApplication::translate("MultiColorMarker", "ignore head without black dot, if not visible / surrounded by color", 0));
#endif // QT_NO_TOOLTIP
        ignoreWithoutDot->setText(QApplication::translate("MultiColorMarker", "ignore head without black dot", 0));
#ifndef QT_NO_TOOLTIP
        useColor->setToolTip(QApplication::translate("MultiColorMarker", "use color marker while tracking for big tracking error", 0));
#endif // QT_NO_TOOLTIP
        useColor->setText(QApplication::translate("MultiColorMarker", "use color marker for big error", 0));
#ifndef QT_NO_TOOLTIP
        restrictPosition->setToolTip(QApplication::translate("MultiColorMarker", "restrict position of black dot on centre of head according to viewing angle", 0));
#endif // QT_NO_TOOLTIP
        restrictPosition->setText(QApplication::translate("MultiColorMarker", "restrict position of black dot", 0));
#ifndef QT_NO_TOOLTIP
        autoCorrect->setToolTip(QApplication::translate("MultiColorMarker", "automatically correct the position of the person according to angle of view", 0));
#endif // QT_NO_TOOLTIP
        autoCorrect->setText(QApplication::translate("MultiColorMarker", "auto correct perspective view", 0));
#ifndef QT_NO_TOOLTIP
        autoCorrectOnlyExport->setToolTip(QApplication::translate("MultiColorMarker", "if auto correct is enabled only the exported real trajectories will be corrected", 0));
#endif // QT_NO_TOOLTIP
        autoCorrectOnlyExport->setText(QApplication::translate("MultiColorMarker", "only for export", 0));
#ifndef QT_NO_TOOLTIP
        showMask->setToolTip(QApplication::translate("MultiColorMarker", "show mask in main window", 0));
#endif // QT_NO_TOOLTIP
        showMask->setText(QApplication::translate("MultiColorMarker", "show mask", 0));
        label->setText(QApplication::translate("MultiColorMarker", "close radius", 0));
        label_2->setText(QApplication::translate("MultiColorMarker", "open radius", 0));
        useOpen->setText(QApplication::translate("MultiColorMarker", "use", 0));
        label_3->setText(QApplication::translate("MultiColorMarker", "area", 0));
        label_4->setText(QApplication::translate("MultiColorMarker", "max ratio", 0));
        label_5->setText(QApplication::translate("MultiColorMarker", "opacity", 0));
        maskMask->setText(QApplication::translate("MultiColorMarker", "mask", 0));
        useClose->setText(QApplication::translate("MultiColorMarker", "use", 0));
        useHeadSize->setText(QApplication::translate("MultiColorMarker", "head size", 0));
        Q_UNUSED(MultiColorMarker);
    } // retranslateUi

};

namespace Ui {
    class MultiColorMarker: public Ui_MultiColorMarker {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MULTICOLORMARKER_H
