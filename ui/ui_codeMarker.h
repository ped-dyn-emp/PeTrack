/********************************************************************************
** Form generated from reading UI file 'codeMarker.ui'
**
** Created by: Qt User Interface Compiler version 5.14.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CODEMARKER_H
#define UI_CODEMARKER_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
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
            CodeMarker->setObjectName(QString::fromUtf8("CodeMarker"));
        CodeMarker->resize(397, 637);
        verticalLayout_2 = new QVBoxLayout(CodeMarker);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        label_6 = new QLabel(CodeMarker);
        label_6->setObjectName(QString::fromUtf8("label_6"));

        horizontalLayout->addWidget(label_6);

        dictList = new QComboBox(CodeMarker);
        dictList->setObjectName(QString::fromUtf8("dictList"));
        dictList->setSizeAdjustPolicy(QComboBox::AdjustToContents);

        horizontalLayout->addWidget(dictList);

        horizontalLayout->setStretch(1, 1);

        verticalLayout_2->addLayout(horizontalLayout);

        verticalLayout = new QVBoxLayout();
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        showDetectedCandidates = new QCheckBox(CodeMarker);
        showDetectedCandidates->setObjectName(QString::fromUtf8("showDetectedCandidates"));

        verticalLayout->addWidget(showDetectedCandidates);


        verticalLayout_2->addLayout(verticalLayout);

        groupBox = new QGroupBox(CodeMarker);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        gridLayout = new QGridLayout(groupBox);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        maxMarkerPerimeter = new QDoubleSpinBox(groupBox);
        maxMarkerPerimeter->setObjectName(QString::fromUtf8("maxMarkerPerimeter"));
        maxMarkerPerimeter->setFocusPolicy(Qt::WheelFocus);
        maxMarkerPerimeter->setMinimum(1.000000000000000);
        maxMarkerPerimeter->setMaximum(100.000000000000000);
        maxMarkerPerimeter->setSingleStep(1.000000000000000);
        maxMarkerPerimeter->setValue(15.000000000000000);

        gridLayout->addWidget(maxMarkerPerimeter, 2, 2, 1, 1);

        minMarkerPerimeter = new QDoubleSpinBox(groupBox);
        minMarkerPerimeter->setObjectName(QString::fromUtf8("minMarkerPerimeter"));
        minMarkerPerimeter->setMinimum(0.100000000000000);
        minMarkerPerimeter->setSingleStep(1.000000000000000);
        minMarkerPerimeter->setValue(5.000000000000000);

        gridLayout->addWidget(minMarkerPerimeter, 2, 1, 1, 1);

        adaptiveThreshWinSizeMax = new QSpinBox(groupBox);
        adaptiveThreshWinSizeMax->setObjectName(QString::fromUtf8("adaptiveThreshWinSizeMax"));
        adaptiveThreshWinSizeMax->setMinimum(3);
        adaptiveThreshWinSizeMax->setValue(23);

        gridLayout->addWidget(adaptiveThreshWinSizeMax, 0, 2, 1, 1);

        label = new QLabel(groupBox);
        label->setObjectName(QString::fromUtf8("label"));

        gridLayout->addWidget(label, 9, 0, 1, 1);

        adaptiveThreshConstant = new QSpinBox(groupBox);
        adaptiveThreshConstant->setObjectName(QString::fromUtf8("adaptiveThreshConstant"));
        adaptiveThreshConstant->setValue(7);

        gridLayout->addWidget(adaptiveThreshConstant, 1, 2, 1, 1);

        markerBorderBits = new QSpinBox(groupBox);
        markerBorderBits->setObjectName(QString::fromUtf8("markerBorderBits"));
        markerBorderBits->setMinimum(1);
        markerBorderBits->setValue(1);

        gridLayout->addWidget(markerBorderBits, 9, 1, 1, 1);

        label_2 = new QLabel(groupBox);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        gridLayout->addWidget(label_2, 0, 0, 1, 1);

        adaptiveThreshWinSizeStep = new QSpinBox(groupBox);
        adaptiveThreshWinSizeStep->setObjectName(QString::fromUtf8("adaptiveThreshWinSizeStep"));
        adaptiveThreshWinSizeStep->setMinimum(1);
        adaptiveThreshWinSizeStep->setValue(10);

        gridLayout->addWidget(adaptiveThreshWinSizeStep, 1, 1, 1, 1);

        polygonalApproxAccuracyRate = new QDoubleSpinBox(groupBox);
        polygonalApproxAccuracyRate->setObjectName(QString::fromUtf8("polygonalApproxAccuracyRate"));
        polygonalApproxAccuracyRate->setMinimum(0.010000000000000);
        polygonalApproxAccuracyRate->setSingleStep(0.010000000000000);
        polygonalApproxAccuracyRate->setValue(0.030000000000000);

        gridLayout->addWidget(polygonalApproxAccuracyRate, 3, 1, 1, 1);

        label_4 = new QLabel(groupBox);
        label_4->setObjectName(QString::fromUtf8("label_4"));

        gridLayout->addWidget(label_4, 3, 0, 1, 1);

        maxErroneousBitsInBorderRate = new QDoubleSpinBox(groupBox);
        maxErroneousBitsInBorderRate->setObjectName(QString::fromUtf8("maxErroneousBitsInBorderRate"));
        maxErroneousBitsInBorderRate->setMinimum(0.010000000000000);
        maxErroneousBitsInBorderRate->setSingleStep(0.010000000000000);
        maxErroneousBitsInBorderRate->setValue(0.350000000000000);

        gridLayout->addWidget(maxErroneousBitsInBorderRate, 11, 1, 1, 1);

        label_3 = new QLabel(groupBox);
        label_3->setObjectName(QString::fromUtf8("label_3"));

        gridLayout->addWidget(label_3, 2, 0, 1, 1);

        minCornerDistance = new QDoubleSpinBox(groupBox);
        minCornerDistance->setObjectName(QString::fromUtf8("minCornerDistance"));
        minCornerDistance->setMinimum(0.000000000000000);
        minCornerDistance->setMaximum(100.000000000000000);
        minCornerDistance->setSingleStep(0.010000000000000);
        minCornerDistance->setValue(0.050000000000000);

        gridLayout->addWidget(minCornerDistance, 4, 1, 1, 1);

        adaptiveThreshWinSizeMin = new QSpinBox(groupBox);
        adaptiveThreshWinSizeMin->setObjectName(QString::fromUtf8("adaptiveThreshWinSizeMin"));
        adaptiveThreshWinSizeMin->setMinimum(3);
        adaptiveThreshWinSizeMin->setValue(3);

        gridLayout->addWidget(adaptiveThreshWinSizeMin, 0, 1, 1, 1);

        label_9 = new QLabel(groupBox);
        label_9->setObjectName(QString::fromUtf8("label_9"));

        gridLayout->addWidget(label_9, 6, 0, 1, 1);

        minDistanceToBorder = new QSpinBox(groupBox);
        minDistanceToBorder->setObjectName(QString::fromUtf8("minDistanceToBorder"));
        minDistanceToBorder->setMinimum(0);
        minDistanceToBorder->setValue(3);

        gridLayout->addWidget(minDistanceToBorder, 5, 1, 1, 1);

        label_7 = new QLabel(groupBox);
        label_7->setObjectName(QString::fromUtf8("label_7"));

        gridLayout->addWidget(label_7, 4, 0, 1, 1);

        label_15 = new QLabel(groupBox);
        label_15->setObjectName(QString::fromUtf8("label_15"));

        gridLayout->addWidget(label_15, 11, 0, 1, 1);

        label_17 = new QLabel(groupBox);
        label_17->setObjectName(QString::fromUtf8("label_17"));

        gridLayout->addWidget(label_17, 14, 0, 1, 1);

        label_8 = new QLabel(groupBox);
        label_8->setObjectName(QString::fromUtf8("label_8"));

        gridLayout->addWidget(label_8, 5, 0, 1, 1);

        minMarkerDistance = new QDoubleSpinBox(groupBox);
        minMarkerDistance->setObjectName(QString::fromUtf8("minMarkerDistance"));
        minMarkerDistance->setMinimum(0.000000000000000);
        minMarkerDistance->setSingleStep(0.500000000000000);
        minMarkerDistance->setValue(0.050000000000000);

        gridLayout->addWidget(minMarkerDistance, 6, 1, 1, 1);

        errorCorrectionRate = new QDoubleSpinBox(groupBox);
        errorCorrectionRate->setObjectName(QString::fromUtf8("errorCorrectionRate"));
        errorCorrectionRate->setMinimum(0.010000000000000);
        errorCorrectionRate->setSingleStep(0.100000000000000);
        errorCorrectionRate->setValue(0.600000000000000);

        gridLayout->addWidget(errorCorrectionRate, 14, 1, 1, 1);

        label_16 = new QLabel(groupBox);
        label_16->setObjectName(QString::fromUtf8("label_16"));

        gridLayout->addWidget(label_16, 13, 0, 1, 1);

        minOtsuStdDev = new QDoubleSpinBox(groupBox);
        minOtsuStdDev->setObjectName(QString::fromUtf8("minOtsuStdDev"));
        minOtsuStdDev->setMinimum(0.010000000000000);
        minOtsuStdDev->setSingleStep(0.500000000000000);
        minOtsuStdDev->setValue(5.000000000000000);

        gridLayout->addWidget(minOtsuStdDev, 13, 1, 1, 1);

        groupBox_2 = new QGroupBox(groupBox);
        groupBox_2->setObjectName(QString::fromUtf8("groupBox_2"));
        gridLayout_3 = new QGridLayout(groupBox_2);
        gridLayout_3->setObjectName(QString::fromUtf8("gridLayout_3"));
        perspectiveRemovePixelPerCell = new QSpinBox(groupBox_2);
        perspectiveRemovePixelPerCell->setObjectName(QString::fromUtf8("perspectiveRemovePixelPerCell"));
        perspectiveRemovePixelPerCell->setMinimum(1);
        perspectiveRemovePixelPerCell->setValue(4);

        gridLayout_3->addWidget(perspectiveRemovePixelPerCell, 0, 1, 1, 1);

        label_13 = new QLabel(groupBox_2);
        label_13->setObjectName(QString::fromUtf8("label_13"));

        gridLayout_3->addWidget(label_13, 0, 0, 1, 1);

        label_14 = new QLabel(groupBox_2);
        label_14->setObjectName(QString::fromUtf8("label_14"));

        gridLayout_3->addWidget(label_14, 1, 0, 1, 1);

        perspectiveRemoveIgnoredMarginPerCell = new QDoubleSpinBox(groupBox_2);
        perspectiveRemoveIgnoredMarginPerCell->setObjectName(QString::fromUtf8("perspectiveRemoveIgnoredMarginPerCell"));
        perspectiveRemoveIgnoredMarginPerCell->setMinimum(0.010000000000000);
        perspectiveRemoveIgnoredMarginPerCell->setSingleStep(0.010000000000000);
        perspectiveRemoveIgnoredMarginPerCell->setValue(0.130000000000000);

        gridLayout_3->addWidget(perspectiveRemoveIgnoredMarginPerCell, 1, 1, 1, 1);


        gridLayout->addWidget(groupBox_2, 10, 0, 1, 2);

        doCornerRefinement = new QGroupBox(groupBox);
        doCornerRefinement->setObjectName(QString::fromUtf8("doCornerRefinement"));
        doCornerRefinement->setCheckable(true);
        doCornerRefinement->setChecked(false);
        gridLayout_2 = new QGridLayout(doCornerRefinement);
        gridLayout_2->setObjectName(QString::fromUtf8("gridLayout_2"));
        cornerRefinementWinSize = new QSpinBox(doCornerRefinement);
        cornerRefinementWinSize->setObjectName(QString::fromUtf8("cornerRefinementWinSize"));
        cornerRefinementWinSize->setMinimum(1);
        cornerRefinementWinSize->setValue(5);

        gridLayout_2->addWidget(cornerRefinementWinSize, 0, 2, 1, 1);

        label_11 = new QLabel(doCornerRefinement);
        label_11->setObjectName(QString::fromUtf8("label_11"));

        gridLayout_2->addWidget(label_11, 1, 1, 1, 1);

        cornerRefinementMaxIterations = new QSpinBox(doCornerRefinement);
        cornerRefinementMaxIterations->setObjectName(QString::fromUtf8("cornerRefinementMaxIterations"));
        cornerRefinementMaxIterations->setMinimum(1);
        cornerRefinementMaxIterations->setValue(30);

        gridLayout_2->addWidget(cornerRefinementMaxIterations, 1, 2, 1, 1);

        label_12 = new QLabel(doCornerRefinement);
        label_12->setObjectName(QString::fromUtf8("label_12"));

        gridLayout_2->addWidget(label_12, 2, 1, 1, 1);

        cornerRefinementMinAccuracy = new QDoubleSpinBox(doCornerRefinement);
        cornerRefinementMinAccuracy->setObjectName(QString::fromUtf8("cornerRefinementMinAccuracy"));
        cornerRefinementMinAccuracy->setMinimum(0.010000000000000);
        cornerRefinementMinAccuracy->setSingleStep(0.100000000000000);
        cornerRefinementMinAccuracy->setValue(0.100000000000000);

        gridLayout_2->addWidget(cornerRefinementMinAccuracy, 2, 2, 1, 1);

        label_10 = new QLabel(doCornerRefinement);
        label_10->setObjectName(QString::fromUtf8("label_10"));
        label_10->setLayoutDirection(Qt::LeftToRight);

        gridLayout_2->addWidget(label_10, 0, 1, 1, 1);


        gridLayout->addWidget(doCornerRefinement, 8, 0, 1, 2);

        moreInfosButton = new QPushButton(groupBox);
        moreInfosButton->setObjectName(QString::fromUtf8("moreInfosButton"));

        gridLayout->addWidget(moreInfosButton, 15, 1, 1, 1);


        verticalLayout_2->addWidget(groupBox);


        retranslateUi(CodeMarker);

        QMetaObject::connectSlotsByName(CodeMarker);
    } // setupUi

    void retranslateUi(QWidget *CodeMarker)
    {
        label_6->setText(QCoreApplication::translate("CodeMarker", "dictionary: ", nullptr));
        showDetectedCandidates->setText(QCoreApplication::translate("CodeMarker", "show detected candidates", nullptr));
        groupBox->setTitle(QCoreApplication::translate("CodeMarker", "detection parameters", nullptr));
        maxMarkerPerimeter->setPrefix(QCoreApplication::translate("CodeMarker", "max: ", nullptr));
        maxMarkerPerimeter->setSuffix(QCoreApplication::translate("CodeMarker", " cm", nullptr));
        minMarkerPerimeter->setPrefix(QCoreApplication::translate("CodeMarker", "min: ", nullptr));
        minMarkerPerimeter->setSuffix(QCoreApplication::translate("CodeMarker", " cm", nullptr));
        adaptiveThreshWinSizeMax->setSuffix(QCoreApplication::translate("CodeMarker", " px", nullptr));
        adaptiveThreshWinSizeMax->setPrefix(QCoreApplication::translate("CodeMarker", "max: ", nullptr));
        label->setText(QCoreApplication::translate("CodeMarker", "marker border size", nullptr));
        adaptiveThreshConstant->setPrefix(QCoreApplication::translate("CodeMarker", "constant: ", nullptr));
        markerBorderBits->setSuffix(QCoreApplication::translate("CodeMarker", " px", nullptr));
        label_2->setText(QCoreApplication::translate("CodeMarker", "adaptive thresholding window:", nullptr));
        adaptiveThreshWinSizeStep->setPrefix(QCoreApplication::translate("CodeMarker", "step: ", nullptr));
        label_4->setText(QCoreApplication::translate("CodeMarker", "max ratio error:", nullptr));
        maxErroneousBitsInBorderRate->setPrefix(QCoreApplication::translate("CodeMarker", "max: ", nullptr));
        label_3->setText(QCoreApplication::translate("CodeMarker", "marker perimeter:", nullptr));
        minCornerDistance->setPrefix(QCoreApplication::translate("CodeMarker", "min: ", nullptr));
        minCornerDistance->setSuffix(QString());
        adaptiveThreshWinSizeMin->setSuffix(QCoreApplication::translate("CodeMarker", " px", nullptr));
        adaptiveThreshWinSizeMin->setPrefix(QCoreApplication::translate("CodeMarker", "min: ", nullptr));
        label_9->setText(QCoreApplication::translate("CodeMarker", "marker distance:", nullptr));
        minDistanceToBorder->setSuffix(QCoreApplication::translate("CodeMarker", " px", nullptr));
        minDistanceToBorder->setPrefix(QCoreApplication::translate("CodeMarker", "min: ", nullptr));
        label_7->setText(QCoreApplication::translate("CodeMarker", "corner distance:", nullptr));
        label_15->setText(QCoreApplication::translate("CodeMarker", "error bits in border rate:", nullptr));
        label_17->setText(QCoreApplication::translate("CodeMarker", "error correction rate:", nullptr));
        label_8->setText(QCoreApplication::translate("CodeMarker", "distance to border:", nullptr));
        minMarkerDistance->setPrefix(QCoreApplication::translate("CodeMarker", "min: ", nullptr));
        minMarkerDistance->setSuffix(QString());
        label_16->setText(QCoreApplication::translate("CodeMarker", "otsu std. dev.: ", nullptr));
        minOtsuStdDev->setPrefix(QCoreApplication::translate("CodeMarker", "min: ", nullptr));
        groupBox_2->setTitle(QCoreApplication::translate("CodeMarker", "perspective remove", nullptr));
        perspectiveRemovePixelPerCell->setSuffix(QCoreApplication::translate("CodeMarker", " px", nullptr));
        perspectiveRemovePixelPerCell->setPrefix(QString());
        label_13->setText(QCoreApplication::translate("CodeMarker", "per cell:", nullptr));
        label_14->setText(QCoreApplication::translate("CodeMarker", "ignored margin per cell:", nullptr));
        doCornerRefinement->setTitle(QCoreApplication::translate("CodeMarker", "use subpixel refinement", nullptr));
        cornerRefinementWinSize->setSuffix(QCoreApplication::translate("CodeMarker", " px", nullptr));
        label_11->setText(QCoreApplication::translate("CodeMarker", "max iterations:", nullptr));
        label_12->setText(QCoreApplication::translate("CodeMarker", "min accuracy:", nullptr));
        label_10->setText(QCoreApplication::translate("CodeMarker", "win size:", nullptr));
        moreInfosButton->setText(QCoreApplication::translate("CodeMarker", "More infos...", nullptr));
        (void)CodeMarker;
    } // retranslateUi

};

namespace Ui {
    class CodeMarker: public Ui_CodeMarker {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CODEMARKER_H
