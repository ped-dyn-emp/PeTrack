﻿/*
 * PeTrack - Software for tracking pedestrians movement in videos
 * Copyright (C) 2025 Forschungszentrum Jülich GmbH, IAS-7
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "codeMarkerWidget.h"

#include "importHelper.h"
#include "pMessageBox.h"
#include "petrack.h"
#include "recognition.h"
#include "ui_codeMarker.h"

#include <opencv2/core/version.hpp>

CodeMarkerWidget::CodeMarkerWidget(QWidget *parent, reco::CodeMarkerOptions &codeMarkerOpt, Ui::CodeMarker *ui) :
    QWidget(parent), mCodeMarkerOpt(codeMarkerOpt)
{
    mMainWindow = (class Petrack *) parent;
    if(!ui)
    {
        mUi = new Ui::CodeMarker();
    }
    else
    {
        mUi = ui;
    }

    mUi->setupUi(this);

    mUi->dictList->addItem("DICT_4X4_50");   // 0
    mUi->dictList->addItem("DICT_4X4_100");  // 1
    mUi->dictList->addItem("DICT_4X4_250");  // 2
    mUi->dictList->addItem("DICT_4X4_1000"); // 3

    mUi->dictList->addItem("DICT_5X5_50");   // 4
    mUi->dictList->addItem("DICT_5X5_100");  // 5
    mUi->dictList->addItem("DICT_5X5_250");  // 6
    mUi->dictList->addItem("DICT_5X5_1000"); // 7

    mUi->dictList->addItem("DICT_6X6_50");   // 8
    mUi->dictList->addItem("DICT_6X6_100");  // 9
    mUi->dictList->addItem("DICT_6X6_250");  // 10
    mUi->dictList->addItem("DICT_6X6_1000"); // 11

    mUi->dictList->addItem("DICT_7X7_50");   // 12
    mUi->dictList->addItem("DICT_7X7_100");  // 13
    mUi->dictList->addItem("DICT_7X7_250");  // 14
    mUi->dictList->addItem("DICT_7X7_1000"); // 15

    mUi->dictList->addItem("DICT_ARUCO_ORGINAL"); // 16
    mUi->dictList->addItem("DICT_mip_36h12");     // 17

    connect(
        &mCodeMarkerOpt, &reco::CodeMarkerOptions::detectorParamsChanged, this, &CodeMarkerWidget::readDetectorParams);
    connect(
        &mCodeMarkerOpt,
        &reco::CodeMarkerOptions::indexOfMarkerDictChanged,
        this,
        &CodeMarkerWidget::readDictListIndex);

    // get default values from Options
    readDetectorParams();
    readDictListIndex();


    // NOTE: No parameter validation done here
    // Can result in crash, when invalid params are chosen (e.g. min larger than max)
    auto changedParams = [&]()
    {
        try
        {
            sendDetectorParams(packDetectorParams(mUi));
            notifyChanged();
        }
        catch(std::invalid_argument &e)
        {
            PWarning(this, "Wrong Value", e.what());
            readDetectorParams();
        }
    };
    connect(
        mUi->showDetectedCandidates,
        &QCheckBox::checkStateChanged,
        this,
        &CodeMarkerWidget::onShowDetectedCandidatesStateChanged);
    connect(mUi->minMarkerPerimeter, QOverload<double>::of(&QDoubleSpinBox::valueChanged), changedParams);
    connect(mUi->maxMarkerPerimeter, QOverload<double>::of(&QDoubleSpinBox::valueChanged), changedParams);

    connect(mUi->adaptiveThreshWinSizeMin, QOverload<int>::of(&QSpinBox::valueChanged), changedParams);
    connect(mUi->adaptiveThreshWinSizeMax, QOverload<int>::of(&QSpinBox::valueChanged), changedParams);
    connect(mUi->adaptiveThreshWinSizeStep, QOverload<int>::of(&QSpinBox::valueChanged), changedParams);
    connect(mUi->adaptiveThreshConstant, QOverload<int>::of(&QSpinBox::valueChanged), changedParams);

    connect(mUi->polygonalApproxAccuracyRate, QOverload<double>::of(&QDoubleSpinBox::valueChanged), changedParams);
    connect(mUi->minCornerDistance, QOverload<double>::of(&QDoubleSpinBox::valueChanged), changedParams);
    connect(mUi->minDistanceToBorder, QOverload<int>::of(&QSpinBox::valueChanged), changedParams);
    connect(mUi->minMarkerDistance, QOverload<double>::of(&QDoubleSpinBox::valueChanged), changedParams);

    connect(mUi->doCornerRefinement, &QGroupBox::clicked, changedParams);
    connect(mUi->cornerRefinementWinSize, QOverload<int>::of(&QSpinBox::valueChanged), changedParams);
    connect(mUi->cornerRefinementMaxIterations, QOverload<int>::of(&QSpinBox::valueChanged), changedParams);
    connect(mUi->cornerRefinementMinAccuracy, QOverload<double>::of(&QDoubleSpinBox::valueChanged), changedParams);

    connect(mUi->markerBorderBits, QOverload<int>::of(&QSpinBox::valueChanged), changedParams);
    connect(mUi->perspectiveRemovePixelPerCell, QOverload<int>::of(&QSpinBox::valueChanged), changedParams);
    connect(
        mUi->perspectiveRemoveIgnoredMarginPerCell,
        QOverload<double>::of(&QDoubleSpinBox::valueChanged),
        changedParams);
    connect(mUi->maxErroneousBitsInBorderRate, QOverload<double>::of(&QDoubleSpinBox::valueChanged), changedParams);
    connect(mUi->errorCorrectionRate, QOverload<double>::of(&QDoubleSpinBox::valueChanged), changedParams);
    connect(mUi->minOtsuStdDev, QOverload<double>::of(&QDoubleSpinBox::valueChanged), changedParams);
    connect(mUi->moreInfosButton, &QPushButton::clicked, this, &CodeMarkerWidget::onMoreInfosButtonClicked);
    connect(
        mUi->dictList,
        QOverload<int>::of(&QComboBox::currentIndexChanged),
        this,
        &CodeMarkerWidget::onDictListCurrentIndexChanged);
}

//<COLOR_MARKER>
//    <MASK SHOW="0" OPACITY="100" MASK="1"/>
//    <FROM_COLOR HUE="0" SATURATION="0" VALUE="0"/>
//    <TO_COLOR HUE="0" SATURATION="0" VALUE="0"/>
//    <PARAM CLOSE_RADIUS="0" CLOSE_USED="0" OPEN_RADIUS="0" OPEN_USED="0" MIN_AREA="0" MAX_AREA="0" MAX_RATIO="0.0"/>
//</COLOR_MARKER>

/// store data in xml node
void CodeMarkerWidget::setXml(QDomElement &elem)
{
    QDomElement subElem;

    subElem = (elem.ownerDocument()).createElement("DICTIONARY");
    subElem.setAttribute("ID", mUi->dictList->currentIndex());
    elem.appendChild(subElem);

    subElem = (elem.ownerDocument()).createElement("PARAM");
    subElem.setAttribute("ADAPTIVE_THRESH_WIN_SIZE_MIN", mUi->adaptiveThreshWinSizeMin->value());
    subElem.setAttribute("ADAPTIVE_THRESH_WIN_SIZE_MAX", mUi->adaptiveThreshWinSizeMax->value());
    subElem.setAttribute("ADAPTIVE_THRESH_WIN_SIZE_STEP", mUi->adaptiveThreshWinSizeStep->value());
    subElem.setAttribute("ADAPTIVE_THRESH_CONSTANT", mUi->adaptiveThreshConstant->value());
    subElem.setAttribute("MIN_MARKER_PERIMETER", mUi->minMarkerPerimeter->value());
    subElem.setAttribute("MAX_MARKER_PERIMETER", mUi->maxMarkerPerimeter->value());
    subElem.setAttribute("MAX_RATIO_ERROR", mUi->polygonalApproxAccuracyRate->value());
    subElem.setAttribute("MIN_CORNER_DISTANCE", mUi->minCornerDistance->value());
    subElem.setAttribute("MIN_DISTANCE_TO_BORDER", mUi->minDistanceToBorder->value());
    subElem.setAttribute("MIN_MARKER_DISTANCE", mUi->minMarkerDistance->value());
    subElem.setAttribute("CORNER_REFINEMENT", mUi->doCornerRefinement->isChecked());
    subElem.setAttribute("CORNER_REFINEMENT_WIN_SIZE", mUi->cornerRefinementWinSize->value());
    subElem.setAttribute("CORNER_REFINEMENT_MAX_ITERATIONS", mUi->cornerRefinementMaxIterations->value());
    subElem.setAttribute("CORNER_REFINEMENT_MIN_ACCURACY", mUi->cornerRefinementMinAccuracy->value());
    subElem.setAttribute("MARKER_BORDER_BITS", mUi->markerBorderBits->value());
    subElem.setAttribute("PERSPECTIVE_REMOVE_PIXEL_PER_CELL", mUi->perspectiveRemovePixelPerCell->value());
    subElem.setAttribute(
        "PERSPECTIVE_REMOVE_IGNORED_MARGIN_PER_CELL", mUi->perspectiveRemoveIgnoredMarginPerCell->value());
    subElem.setAttribute("MAX_ERRONEOUS_BITS_IN_BORDER_RATE", mUi->maxErroneousBitsInBorderRate->value());
    subElem.setAttribute("MIN_OTSU_STD_DEV", mUi->minOtsuStdDev->value());
    subElem.setAttribute("ERROR_CORRECTION_RATE", mUi->errorCorrectionRate->value());
    subElem.setAttribute("SHOW_DETECTED_CANDIDATES", mUi->showDetectedCandidates->isChecked());
    elem.appendChild(subElem);
}

/// read data from xml node
void CodeMarkerWidget::getXml(QDomElement &elem)
{
    QDomElement subElem;
    QString     styleString;

    for(subElem = elem.firstChildElement(); !subElem.isNull(); subElem = subElem.nextSiblingElement())
    {
        if(subElem.tagName() == "DICTIONARY")
        {
            loadActiveIndex(subElem, "ID", mUi->dictList, 0);
        }

        if(subElem.tagName() == "PARAM")
        {
            loadIntValue(subElem, "ADAPTIVE_THRESH_WIN_SIZE_MIN", mUi->adaptiveThreshWinSizeMin);
            loadIntValue(subElem, "ADAPTIVE_THRESH_WIN_SIZE_MAX", mUi->adaptiveThreshWinSizeMax);
            loadIntValue(subElem, "ADAPTIVE_THRESH_WIN_SIZE_STEP", mUi->adaptiveThreshWinSizeStep);
            loadIntValue(subElem, "ADAPTIVE_THRESH_CONSTANT", mUi->adaptiveThreshConstant);
            loadDoubleValue(subElem, "MIN_MARKER_PERIMETER", mUi->minMarkerPerimeter);
            loadDoubleValue(subElem, "MAX_MARKER_PERIMETER", mUi->maxMarkerPerimeter);
            loadDoubleValue(subElem, "MAX_RATIO_ERROR", mUi->polygonalApproxAccuracyRate);
            loadDoubleValue(subElem, "MIN_CORNER_DISTANCE", mUi->minCornerDistance);
            loadIntValue(subElem, "MIN_DISTANCE_TO_BORDER", mUi->minDistanceToBorder);
            loadDoubleValue(subElem, "MIN_MARKER_DISTANCE", mUi->minMarkerDistance);
            loadBoolValue(subElem, "CORNER_REFINEMENT", mUi->doCornerRefinement);
            loadIntValue(subElem, "CORNER_REFINEMENT_WIN_SIZE", mUi->cornerRefinementWinSize);
            loadIntValue(subElem, "CORNER_REFINEMENT_MAX_ITERATIONS", mUi->cornerRefinementMaxIterations);
            loadDoubleValue(subElem, "CORNER_REFINEMENT_MIN_ACCURACY", mUi->cornerRefinementMinAccuracy);
            loadIntValue(subElem, "MARKER_BORDER_BITS", mUi->markerBorderBits);
            loadIntValue(subElem, "PERSPECTIVE_REMOVE_PIXEL_PER_CELL", mUi->perspectiveRemovePixelPerCell);
            loadDoubleValue(
                subElem, "PERSPECTIVE_REMOVE_IGNORED_MARGIN_PER_CELL", mUi->perspectiveRemoveIgnoredMarginPerCell);
            loadDoubleValue(subElem, "MAX_ERRONEOUS_BITS_IN_BORDER_RATE", mUi->maxErroneousBitsInBorderRate);
            loadDoubleValue(subElem, "MIN_OTSU_STD_DEV", mUi->minOtsuStdDev);
            loadDoubleValue(subElem, "ERROR_CORRECTION_RATE", mUi->errorCorrectionRate);
            loadBoolValue(subElem, "SHOW_DETECTED_CANDIDATES", mUi->showDetectedCandidates);
        }
    }
}

reco::ArucoCodeParams packDetectorParams(const Ui::CodeMarker *ui)
{
    reco::ArucoCodeParams params;
    params.setAdaptiveThreshConstant(ui->adaptiveThreshConstant->value());
    params.setAdaptiveThreshWinSizeMax(ui->adaptiveThreshWinSizeMax->value());
    params.setAdaptiveThreshWinSizeMin(ui->adaptiveThreshWinSizeMin->value());
    params.setAdaptiveThreshWinSizeStep(ui->adaptiveThreshWinSizeStep->value());
    params.setCornerRefinementMaxIterations(ui->cornerRefinementMaxIterations->value());
    params.setCornerRefinementMinAccuracy(ui->cornerRefinementMinAccuracy->value());
    params.setCornerRefinementWinSize(ui->cornerRefinementWinSize->value());
    params.setDoCornerRefinement(ui->doCornerRefinement->isChecked());
    params.setErrorCorrectionRate(ui->errorCorrectionRate->value());
    params.setMarkerBorderBits(ui->markerBorderBits->value());
    params.setMaxErroneousBitsInBorderRate(ui->maxErroneousBitsInBorderRate->value());
    params.setMinDistanceToBorder(ui->minDistanceToBorder->value());
    params.setMinMarkerDistance(ui->minMarkerDistance->value());
    params.setMaxMarkerPerimeter(ui->maxMarkerPerimeter->value());
    params.setMinMarkerPerimeter(ui->minMarkerPerimeter->value());
    params.setMinOtsuStdDev(ui->minOtsuStdDev->value());
    params.setPerspectiveRemoveIgnoredMarginPerCell(ui->perspectiveRemoveIgnoredMarginPerCell->value());
    params.setPerspectiveRemovePixelPerCell(ui->perspectiveRemovePixelPerCell->value());
    params.setPolygonalApproxAccuracyRate(ui->polygonalApproxAccuracyRate->value());

    return params;
}

bool CodeMarkerWidget::showDetectedCandidates()
{
    return mUi->showDetectedCandidates->isChecked();
}

void CodeMarkerWidget::onShowDetectedCandidatesStateChanged(int i)
{
    mMainWindow->getCodeMarkerItem()->setVisible(i);
    if(!mMainWindow->isLoading())
    {
        mMainWindow->getScene()->update();
    }
}

void CodeMarkerWidget::onMoreInfosButtonClicked()
{
#define CV_NUMERIC_VERSION                                                                                             \
    CVAUX_STR(CV_VERSION_MAJOR) "." CVAUX_STR(CV_VERSION_MINOR) "." CVAUX_STR(CV_VERSION_REVISION)
    QDesktopServices::openUrl(QUrl(
        "http://docs.opencv.org/" CV_NUMERIC_VERSION "/d1/dcd/structcv_1_1aruco_1_1DetectorParameters.html#details",
        QUrl::TolerantMode));
    QDesktopServices::openUrl(
        QUrl("http://docs.opencv.org/" CV_NUMERIC_VERSION "/d5/dae/tutorial_aruco_detection.html", QUrl::TolerantMode));
}

void CodeMarkerWidget::onDictListCurrentIndexChanged(int i)
{
    mCodeMarkerOpt.setIndexOfMarkerDict(i);
    notifyChanged();
}


void CodeMarkerWidget::notifyChanged()
{
    mMainWindow->setRecognitionChanged(true); // flag indicates that changes of recognition parameters happens
    if(!mMainWindow->isLoading())
    {
        mMainWindow->updateImage();
    }
}

void CodeMarkerWidget::readDetectorParams()
{
    auto params = mCodeMarkerOpt.getDetectorParams();
    mUi->adaptiveThreshConstant->setValue(params.getAdaptiveThreshConstant());
    mUi->adaptiveThreshWinSizeMax->setValue(params.getAdaptiveThreshWinSizeMax());
    mUi->adaptiveThreshWinSizeMin->setValue(params.getAdaptiveThreshWinSizeMin());
    mUi->adaptiveThreshWinSizeStep->setValue(params.getAdaptiveThreshWinSizeStep());
    mUi->cornerRefinementMaxIterations->setValue(params.getCornerRefinementMaxIterations());
    mUi->cornerRefinementMinAccuracy->setValue(params.getCornerRefinementMinAccuracy());
    mUi->cornerRefinementWinSize->setValue(params.getCornerRefinementWinSize());
    mUi->doCornerRefinement->setChecked(params.getDoCornerRefinement());
    mUi->errorCorrectionRate->setValue(params.getErrorCorrectionRate());
    mUi->markerBorderBits->setValue(params.getMarkerBorderBits());
    mUi->maxErroneousBitsInBorderRate->setValue(params.getMaxErroneousBitsInBorderRate());
    mUi->minDistanceToBorder->setValue(params.getMinDistanceToBorder());
    mUi->minMarkerDistance->setValue(params.getMinMarkerDistance());
    mUi->maxMarkerPerimeter->setValue(params.getMaxMarkerPerimeter());
    mUi->minMarkerPerimeter->setValue(params.getMinMarkerPerimeter());
    mUi->minOtsuStdDev->setValue(params.getMinOtsuStdDev());
    mUi->perspectiveRemoveIgnoredMarginPerCell->setValue(params.getPerspectiveRemoveIgnoredMarginPerCell());
    mUi->perspectiveRemovePixelPerCell->setValue(params.getPerspectiveRemovePixelPerCell());
    mUi->polygonalApproxAccuracyRate->setValue(params.getPolygonalApproxAccuracyRate());
}

void CodeMarkerWidget::readDictListIndex()
{
    mUi->dictList->setCurrentIndex(mCodeMarkerOpt.getIndexOfMarkerDict());
}

void CodeMarkerWidget::sendDetectorParams(reco::ArucoCodeParams params)
{
    mCodeMarkerOpt.setDetectorParams(params);
}

#include "moc_codeMarkerWidget.cpp"
