/********************************************************************************
** Form generated from reading UI file 'codeMarker.ui'
**
** Created by: Qt User Interface Compiler version 5.5.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CODEMARKER_H
#define UI_CODEMARKER_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
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

QT_BEGIN_NAMESPACE

class Ui_CodeMarker
{
public:
    QVBoxLayout *verticalLayout_2;
    QHBoxLayout *horizontalLayout;
    QLabel *label_6;
    QComboBox *dictList;
    QVBoxLayout *verticalLayout;
    QCheckBox *showDetectedCandidates;
    QGroupBox *groupBox;
    QGridLayout *gridLayout;
    QDoubleSpinBox *maxMarkerPerimeter;
    QDoubleSpinBox *minMarkerPerimeter;
    QSpinBox *adaptiveThreshWinSizeMax;
    QLabel *label;
    QSpinBox *adaptiveThreshConstant;
    QSpinBox *markerBorderBits;
    QLabel *label_2;
    QSpinBox *adaptiveThreshWinSizeStep;
    QDoubleSpinBox *polygonalApproxAccuracyRate;
    QLabel *label_4;
    QDoubleSpinBox *maxErroneousBitsInBorderRate;
    QLabel *label_3;
    QDoubleSpinBox *minCornerDistance;
    QSpinBox *adaptiveThreshWinSizeMin;
    QLabel *label_9;
    QSpinBox *minDistanceToBorder;
    QLabel *label_7;
    QLabel *label_15;
    QLabel *label_17;
    QLabel *label_8;
    QDoubleSpinBox *minMarkerDistance;
    QDoubleSpinBox *errorCorrectionRate;
    QLabel *label_16;
    QDoubleSpinBox *minOtsuStdDev;
    QGroupBox *groupBox_2;
    QGridLayout *gridLayout_3;
    QSpinBox *perspectiveRemovePixelPerCell;
    QLabel *label_13;
    QLabel *label_14;
    QDoubleSpinBox *perspectiveRemoveIgnoredMarginPerCell;
    QGroupBox *doCornerRefinement;
    QGridLayout *gridLayout_2;
    QSpinBox *cornerRefinementWinSize;
    QLabel *label_11;
    QSpinBox *cornerRefinementMaxIterations;
    QLabel *label_12;
    QDoubleSpinBox *cornerRefinementMinAccuracy;
    QLabel *label_10;
    QPushButton *moreInfosButton;

    void setupUi(QWidget *CodeMarker)
    {
        if (CodeMarker->objectName().isEmpty())
            CodeMarker->setObjectName(QStringLiteral("CodeMarker"));
        CodeMarker->resize(397, 637);
        verticalLayout_2 = new QVBoxLayout(CodeMarker);
        verticalLayout_2->setObjectName(QStringLiteral("verticalLayout_2"));
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        label_6 = new QLabel(CodeMarker);
        label_6->setObjectName(QStringLiteral("label_6"));

        horizontalLayout->addWidget(label_6);

        dictList = new QComboBox(CodeMarker);
        dictList->setObjectName(QStringLiteral("dictList"));
        dictList->setSizeAdjustPolicy(QComboBox::AdjustToContents);

        horizontalLayout->addWidget(dictList);

        horizontalLayout->setStretch(1, 1);

        verticalLayout_2->addLayout(horizontalLayout);

        verticalLayout = new QVBoxLayout();
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        showDetectedCandidates = new QCheckBox(CodeMarker);
        showDetectedCandidates->setObjectName(QStringLiteral("showDetectedCandidates"));

        verticalLayout->addWidget(showDetectedCandidates);


        verticalLayout_2->addLayout(verticalLayout);

        groupBox = new QGroupBox(CodeMarker);
        groupBox->setObjectName(QStringLiteral("groupBox"));
        gridLayout = new QGridLayout(groupBox);
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        maxMarkerPerimeter = new QDoubleSpinBox(groupBox);
        maxMarkerPerimeter->setObjectName(QStringLiteral("maxMarkerPerimeter"));
        maxMarkerPerimeter->setFocusPolicy(Qt::WheelFocus);
        maxMarkerPerimeter->setMinimum(1);
        maxMarkerPerimeter->setMaximum(100);
        maxMarkerPerimeter->setSingleStep(1);
        maxMarkerPerimeter->setValue(15);

        gridLayout->addWidget(maxMarkerPerimeter, 2, 2, 1, 1);

        minMarkerPerimeter = new QDoubleSpinBox(groupBox);
        minMarkerPerimeter->setObjectName(QStringLiteral("minMarkerPerimeter"));
        minMarkerPerimeter->setMinimum(0.1);
        minMarkerPerimeter->setSingleStep(1);
        minMarkerPerimeter->setValue(5);

        gridLayout->addWidget(minMarkerPerimeter, 2, 1, 1, 1);

        adaptiveThreshWinSizeMax = new QSpinBox(groupBox);
        adaptiveThreshWinSizeMax->setObjectName(QStringLiteral("adaptiveThreshWinSizeMax"));
        adaptiveThreshWinSizeMax->setMinimum(3);
        adaptiveThreshWinSizeMax->setValue(23);

        gridLayout->addWidget(adaptiveThreshWinSizeMax, 0, 2, 1, 1);

        label = new QLabel(groupBox);
        label->setObjectName(QStringLiteral("label"));

        gridLayout->addWidget(label, 9, 0, 1, 1);

        adaptiveThreshConstant = new QSpinBox(groupBox);
        adaptiveThreshConstant->setObjectName(QStringLiteral("adaptiveThreshConstant"));
        adaptiveThreshConstant->setValue(7);

        gridLayout->addWidget(adaptiveThreshConstant, 1, 2, 1, 1);

        markerBorderBits = new QSpinBox(groupBox);
        markerBorderBits->setObjectName(QStringLiteral("markerBorderBits"));
        markerBorderBits->setMinimum(1);
        markerBorderBits->setValue(1);

        gridLayout->addWidget(markerBorderBits, 9, 1, 1, 1);

        label_2 = new QLabel(groupBox);
        label_2->setObjectName(QStringLiteral("label_2"));

        gridLayout->addWidget(label_2, 0, 0, 1, 1);

        adaptiveThreshWinSizeStep = new QSpinBox(groupBox);
        adaptiveThreshWinSizeStep->setObjectName(QStringLiteral("adaptiveThreshWinSizeStep"));
        adaptiveThreshWinSizeStep->setMinimum(1);
        adaptiveThreshWinSizeStep->setValue(10);

        gridLayout->addWidget(adaptiveThreshWinSizeStep, 1, 1, 1, 1);

        polygonalApproxAccuracyRate = new QDoubleSpinBox(groupBox);
        polygonalApproxAccuracyRate->setObjectName(QStringLiteral("polygonalApproxAccuracyRate"));
        polygonalApproxAccuracyRate->setMinimum(0.01);
        polygonalApproxAccuracyRate->setSingleStep(0.01);
        polygonalApproxAccuracyRate->setValue(0.03);

        gridLayout->addWidget(polygonalApproxAccuracyRate, 3, 1, 1, 1);

        label_4 = new QLabel(groupBox);
        label_4->setObjectName(QStringLiteral("label_4"));

        gridLayout->addWidget(label_4, 3, 0, 1, 1);

        maxErroneousBitsInBorderRate = new QDoubleSpinBox(groupBox);
        maxErroneousBitsInBorderRate->setObjectName(QStringLiteral("maxErroneousBitsInBorderRate"));
        maxErroneousBitsInBorderRate->setMinimum(0.01);
        maxErroneousBitsInBorderRate->setSingleStep(0.01);
        maxErroneousBitsInBorderRate->setValue(0.35);

        gridLayout->addWidget(maxErroneousBitsInBorderRate, 11, 1, 1, 1);

        label_3 = new QLabel(groupBox);
        label_3->setObjectName(QStringLiteral("label_3"));

        gridLayout->addWidget(label_3, 2, 0, 1, 1);

        minCornerDistance = new QDoubleSpinBox(groupBox);
        minCornerDistance->setObjectName(QStringLiteral("minCornerDistance"));
        minCornerDistance->setMinimum(0);
        minCornerDistance->setMaximum(100);
        minCornerDistance->setSingleStep(0.01);
        minCornerDistance->setValue(0.05);

        gridLayout->addWidget(minCornerDistance, 4, 1, 1, 1);

        adaptiveThreshWinSizeMin = new QSpinBox(groupBox);
        adaptiveThreshWinSizeMin->setObjectName(QStringLiteral("adaptiveThreshWinSizeMin"));
        adaptiveThreshWinSizeMin->setMinimum(3);
        adaptiveThreshWinSizeMin->setValue(3);

        gridLayout->addWidget(adaptiveThreshWinSizeMin, 0, 1, 1, 1);

        label_9 = new QLabel(groupBox);
        label_9->setObjectName(QStringLiteral("label_9"));

        gridLayout->addWidget(label_9, 6, 0, 1, 1);

        minDistanceToBorder = new QSpinBox(groupBox);
        minDistanceToBorder->setObjectName(QStringLiteral("minDistanceToBorder"));
        minDistanceToBorder->setMinimum(0);
        minDistanceToBorder->setValue(3);

        gridLayout->addWidget(minDistanceToBorder, 5, 1, 1, 1);

        label_7 = new QLabel(groupBox);
        label_7->setObjectName(QStringLiteral("label_7"));

        gridLayout->addWidget(label_7, 4, 0, 1, 1);

        label_15 = new QLabel(groupBox);
        label_15->setObjectName(QStringLiteral("label_15"));

        gridLayout->addWidget(label_15, 11, 0, 1, 1);

        label_17 = new QLabel(groupBox);
        label_17->setObjectName(QStringLiteral("label_17"));

        gridLayout->addWidget(label_17, 14, 0, 1, 1);

        label_8 = new QLabel(groupBox);
        label_8->setObjectName(QStringLiteral("label_8"));

        gridLayout->addWidget(label_8, 5, 0, 1, 1);

        minMarkerDistance = new QDoubleSpinBox(groupBox);
        minMarkerDistance->setObjectName(QStringLiteral("minMarkerDistance"));
        minMarkerDistance->setMinimum(0);
        minMarkerDistance->setSingleStep(0.5);
        minMarkerDistance->setValue(0.05);

        gridLayout->addWidget(minMarkerDistance, 6, 1, 1, 1);

        errorCorrectionRate = new QDoubleSpinBox(groupBox);
        errorCorrectionRate->setObjectName(QStringLiteral("errorCorrectionRate"));
        errorCorrectionRate->setMinimum(0.01);
        errorCorrectionRate->setSingleStep(0.1);
        errorCorrectionRate->setValue(0.6);

        gridLayout->addWidget(errorCorrectionRate, 14, 1, 1, 1);

        label_16 = new QLabel(groupBox);
        label_16->setObjectName(QStringLiteral("label_16"));

        gridLayout->addWidget(label_16, 13, 0, 1, 1);

        minOtsuStdDev = new QDoubleSpinBox(groupBox);
        minOtsuStdDev->setObjectName(QStringLiteral("minOtsuStdDev"));
        minOtsuStdDev->setMinimum(0.01);
        minOtsuStdDev->setSingleStep(0.5);
        minOtsuStdDev->setValue(5);

        gridLayout->addWidget(minOtsuStdDev, 13, 1, 1, 1);

        groupBox_2 = new QGroupBox(groupBox);
        groupBox_2->setObjectName(QStringLiteral("groupBox_2"));
        gridLayout_3 = new QGridLayout(groupBox_2);
        gridLayout_3->setObjectName(QStringLiteral("gridLayout_3"));
        perspectiveRemovePixelPerCell = new QSpinBox(groupBox_2);
        perspectiveRemovePixelPerCell->setObjectName(QStringLiteral("perspectiveRemovePixelPerCell"));
        perspectiveRemovePixelPerCell->setMinimum(1);
        perspectiveRemovePixelPerCell->setValue(4);

        gridLayout_3->addWidget(perspectiveRemovePixelPerCell, 0, 1, 1, 1);

        label_13 = new QLabel(groupBox_2);
        label_13->setObjectName(QStringLiteral("label_13"));

        gridLayout_3->addWidget(label_13, 0, 0, 1, 1);

        label_14 = new QLabel(groupBox_2);
        label_14->setObjectName(QStringLiteral("label_14"));

        gridLayout_3->addWidget(label_14, 1, 0, 1, 1);

        perspectiveRemoveIgnoredMarginPerCell = new QDoubleSpinBox(groupBox_2);
        perspectiveRemoveIgnoredMarginPerCell->setObjectName(QStringLiteral("perspectiveRemoveIgnoredMarginPerCell"));
        perspectiveRemoveIgnoredMarginPerCell->setMinimum(0.01);
        perspectiveRemoveIgnoredMarginPerCell->setSingleStep(0.01);
        perspectiveRemoveIgnoredMarginPerCell->setValue(0.13);

        gridLayout_3->addWidget(perspectiveRemoveIgnoredMarginPerCell, 1, 1, 1, 1);


        gridLayout->addWidget(groupBox_2, 10, 0, 1, 2);

        doCornerRefinement = new QGroupBox(groupBox);
        doCornerRefinement->setObjectName(QStringLiteral("doCornerRefinement"));
        doCornerRefinement->setCheckable(true);
        doCornerRefinement->setChecked(false);
        gridLayout_2 = new QGridLayout(doCornerRefinement);
        gridLayout_2->setObjectName(QStringLiteral("gridLayout_2"));
        cornerRefinementWinSize = new QSpinBox(doCornerRefinement);
        cornerRefinementWinSize->setObjectName(QStringLiteral("cornerRefinementWinSize"));
        cornerRefinementWinSize->setMinimum(1);
        cornerRefinementWinSize->setValue(5);

        gridLayout_2->addWidget(cornerRefinementWinSize, 0, 2, 1, 1);

        label_11 = new QLabel(doCornerRefinement);
        label_11->setObjectName(QStringLiteral("label_11"));

        gridLayout_2->addWidget(label_11, 1, 1, 1, 1);

        cornerRefinementMaxIterations = new QSpinBox(doCornerRefinement);
        cornerRefinementMaxIterations->setObjectName(QStringLiteral("cornerRefinementMaxIterations"));
        cornerRefinementMaxIterations->setMinimum(1);
        cornerRefinementMaxIterations->setValue(30);

        gridLayout_2->addWidget(cornerRefinementMaxIterations, 1, 2, 1, 1);

        label_12 = new QLabel(doCornerRefinement);
        label_12->setObjectName(QStringLiteral("label_12"));

        gridLayout_2->addWidget(label_12, 2, 1, 1, 1);

        cornerRefinementMinAccuracy = new QDoubleSpinBox(doCornerRefinement);
        cornerRefinementMinAccuracy->setObjectName(QStringLiteral("cornerRefinementMinAccuracy"));
        cornerRefinementMinAccuracy->setMinimum(0.01);
        cornerRefinementMinAccuracy->setSingleStep(0.1);
        cornerRefinementMinAccuracy->setValue(0.1);

        gridLayout_2->addWidget(cornerRefinementMinAccuracy, 2, 2, 1, 1);

        label_10 = new QLabel(doCornerRefinement);
        label_10->setObjectName(QStringLiteral("label_10"));
        label_10->setLayoutDirection(Qt::LeftToRight);

        gridLayout_2->addWidget(label_10, 0, 1, 1, 1);


        gridLayout->addWidget(doCornerRefinement, 8, 0, 1, 2);

        moreInfosButton = new QPushButton(groupBox);
        moreInfosButton->setObjectName(QStringLiteral("moreInfosButton"));

        gridLayout->addWidget(moreInfosButton, 15, 1, 1, 1);


        verticalLayout_2->addWidget(groupBox);


        retranslateUi(CodeMarker);

        QMetaObject::connectSlotsByName(CodeMarker);
    } // setupUi

    void retranslateUi(QWidget *CodeMarker)
    {
        label_6->setText(QApplication::translate("CodeMarker", "dictionary: ", 0));
        showDetectedCandidates->setText(QApplication::translate("CodeMarker", "show detected candidates", 0));
        groupBox->setTitle(QApplication::translate("CodeMarker", "detection parameters", 0));
        maxMarkerPerimeter->setPrefix(QApplication::translate("CodeMarker", "max: ", 0));
        maxMarkerPerimeter->setSuffix(QApplication::translate("CodeMarker", " cm", 0));
        minMarkerPerimeter->setPrefix(QApplication::translate("CodeMarker", "min: ", 0));
        minMarkerPerimeter->setSuffix(QApplication::translate("CodeMarker", " cm", 0));
        adaptiveThreshWinSizeMax->setSuffix(QApplication::translate("CodeMarker", " px", 0));
        adaptiveThreshWinSizeMax->setPrefix(QApplication::translate("CodeMarker", "max: ", 0));
        label->setText(QApplication::translate("CodeMarker", "marker border size", 0));
        adaptiveThreshConstant->setPrefix(QApplication::translate("CodeMarker", "constant: ", 0));
        markerBorderBits->setSuffix(QApplication::translate("CodeMarker", " px", 0));
        label_2->setText(QApplication::translate("CodeMarker", "adaptive thresholding window:", 0));
        adaptiveThreshWinSizeStep->setPrefix(QApplication::translate("CodeMarker", "step: ", 0));
        label_4->setText(QApplication::translate("CodeMarker", "max ratio error:", 0));
        maxErroneousBitsInBorderRate->setPrefix(QApplication::translate("CodeMarker", "max: ", 0));
        label_3->setText(QApplication::translate("CodeMarker", "marker perimeter:", 0));
        minCornerDistance->setPrefix(QApplication::translate("CodeMarker", "min: ", 0));
        minCornerDistance->setSuffix(QString());
        adaptiveThreshWinSizeMin->setSuffix(QApplication::translate("CodeMarker", " px", 0));
        adaptiveThreshWinSizeMin->setPrefix(QApplication::translate("CodeMarker", "min: ", 0));
        label_9->setText(QApplication::translate("CodeMarker", "marker distance:", 0));
        minDistanceToBorder->setSuffix(QApplication::translate("CodeMarker", " px", 0));
        minDistanceToBorder->setPrefix(QApplication::translate("CodeMarker", "min: ", 0));
        label_7->setText(QApplication::translate("CodeMarker", "corner distance:", 0));
        label_15->setText(QApplication::translate("CodeMarker", "error bits in border rate:", 0));
        label_17->setText(QApplication::translate("CodeMarker", "error correction rate:", 0));
        label_8->setText(QApplication::translate("CodeMarker", "distance to border:", 0));
        minMarkerDistance->setPrefix(QApplication::translate("CodeMarker", "min: ", 0));
        minMarkerDistance->setSuffix(QString());
        label_16->setText(QApplication::translate("CodeMarker", "otsu std. dev.: ", 0));
        minOtsuStdDev->setPrefix(QApplication::translate("CodeMarker", "min: ", 0));
        groupBox_2->setTitle(QApplication::translate("CodeMarker", "perspective remove", 0));
        perspectiveRemovePixelPerCell->setSuffix(QApplication::translate("CodeMarker", " px", 0));
        perspectiveRemovePixelPerCell->setPrefix(QString());
        label_13->setText(QApplication::translate("CodeMarker", "per cell:", 0));
        label_14->setText(QApplication::translate("CodeMarker", "ignored margin per cell:", 0));
        doCornerRefinement->setTitle(QApplication::translate("CodeMarker", "use subpixel refinement", 0));
        cornerRefinementWinSize->setSuffix(QApplication::translate("CodeMarker", " px", 0));
        label_11->setText(QApplication::translate("CodeMarker", "max iterations:", 0));
        label_12->setText(QApplication::translate("CodeMarker", "min accuracy:", 0));
        label_10->setText(QApplication::translate("CodeMarker", "win size:", 0));
        moreInfosButton->setText(QApplication::translate("CodeMarker", "More infos...", 0));
        Q_UNUSED(CodeMarker);
    } // retranslateUi

};

namespace Ui {
    class CodeMarker: public Ui_CodeMarker {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CODEMARKER_H
