/*
 * PeTrack - Software for tracking pedestrians movement in videos
 * Copyright (C) 2010-2020 Forschungszentrum Jülich GmbH,
 * Maik Boltes, Juliane Adrian, Ricardo Martin Brualla, Arne Graf, Paul Häger, Daniel Hillebrand,
 * Deniz Kilic, Paul Lieberenz, Daniel Salden, Tobias Schrödter, Ann Katrin Seemann
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

#ifndef CODEMARKERWIDGET_H
#define CODEMARKERWIDGET_H

#include <QtWidgets>
#include <opencv2/aruco.hpp>

#include "ui_codeMarker.h"

#include "petrack.h"
#include "codeMarkerItem.h"


class CodeMarkerWidget: public QWidget, public Ui::CodeMarker
{
    Q_OBJECT

public:
    CodeMarkerWidget(QWidget *parent = nullptr);

    // store data in xml node
    void setXml(QDomElement &elem);

    // read data from xml node
    void getXml(QDomElement &elem);


private slots:

    void on_showDetectedCandidates_stateChanged(int i)
    {

        mMainWindow->getCodeMarkerItem()->setVisible(i);
        if( !mMainWindow->isLoading() )
            mMainWindow->getScene()->update();
    }

    void on_moreInfosButton_clicked()
    {
        QDesktopServices::openUrl(QUrl("http://docs.opencv.org/trunk/d1/dcd/structcv_1_1aruco_1_1DetectorParameters.html#details", QUrl::TolerantMode));
        QDesktopServices::openUrl(QUrl("http://docs.opencv.org/3.1.0/d5/dae/tutorial_aruco_detection.html", QUrl::TolerantMode));
    }
    void on_dictList_currentIndexChanged(int /*index*/)
    {
        mMainWindow->setRecognitionChanged(true);// flag indicates that changes of recognition parameters happens
        if( !mMainWindow->isLoading() )
            mMainWindow->updateImage();
    }
    void on_minMarkerPerimeter_valueChanged(double /*val*/)
    {
        mMainWindow->setRecognitionChanged(true);// flag indicates that changes of recognition parameters happens
        if( !mMainWindow->isLoading() )
            mMainWindow->updateImage();
    }
    void on_maxMarkerPerimeter_valueChanged(double /*val*/)
    {
        mMainWindow->setRecognitionChanged(true);// flag indicates that changes of recognition parameters happens
        if( !mMainWindow->isLoading() )
            mMainWindow->updateImage();
    }
    void on_adaptiveThreshWinSizeMin_valueChanged(int /*i*/)
    {
        mMainWindow->setRecognitionChanged(true);// flag indicates that changes of recognition parameters happens
        if( !mMainWindow->isLoading() )
            mMainWindow->updateImage();
    }
    void on_adaptiveThreshWinSizeMax_valueChanged(int /*i*/)
    {
        mMainWindow->setRecognitionChanged(true);// flag indicates that changes of recognition parameters happens
        if( !mMainWindow->isLoading() )
            mMainWindow->updateImage();
    }
    void on_adaptiveThreshWinSizeStep_valueChanged(int /*i*/)
    {
        mMainWindow->setRecognitionChanged(true);// flag indicates that changes of recognition parameters happens
        if( !mMainWindow->isLoading() )
            mMainWindow->updateImage();
    }
    void on_adaptiveThreshConstant_valueChanged(int /*i*/)
    {
        mMainWindow->setRecognitionChanged(true);// flag indicates that changes of recognition parameters happens
        if( !mMainWindow->isLoading() )
            mMainWindow->updateImage();
    }
    void on_polygonalApproxAccuracyRate_valueChanged(double /*val*/)
    {
        mMainWindow->setRecognitionChanged(true);// flag indicates that changes of recognition parameters happens
        if( !mMainWindow->isLoading() )
            mMainWindow->updateImage();
    }
    void on_minCornerDistance_valueChanged(double /*val*/)
    {
        mMainWindow->setRecognitionChanged(true);// flag indicates that changes of recognition parameters happens
        if( !mMainWindow->isLoading() )
            mMainWindow->updateImage();
    }
    void on_minDistanceToBorder_valueChanged(int /*i*/)
    {
        mMainWindow->setRecognitionChanged(true);// flag indicates that changes of recognition parameters happens
        if( !mMainWindow->isLoading() )
            mMainWindow->updateImage();
    }
    void on_minMarkerDistance_valueChanged(double /*val*/)
    {
        mMainWindow->setRecognitionChanged(true);// flag indicates that changes of recognition parameters happens
        if( !mMainWindow->isLoading() )
            mMainWindow->updateImage();
    }
    void on_doCornerRefinement_clicked()
    {
        mMainWindow->setRecognitionChanged(true);// flag indicates that changes of recognition parameters happens
        if( !mMainWindow->isLoading() )
            mMainWindow->updateImage();
    }
    void on_cornerRefinementWinSize_valueChanged(int /*i*/)
    {
        mMainWindow->setRecognitionChanged(true);// flag indicates that changes of recognition parameters happens
        if( !mMainWindow->isLoading() )
            mMainWindow->updateImage();
    }
    void on_cornerRefinementMaxIterations_valueChanged(int /*i*/)
    {
        mMainWindow->setRecognitionChanged(true);// flag indicates that changes of recognition parameters happens
        if( !mMainWindow->isLoading() )
            mMainWindow->updateImage();
    }
    void on_cornerRefinementMinAccuracy_valueChanged(double /*val*/)
    {
        mMainWindow->setRecognitionChanged(true);// flag indicates that changes of recognition parameters happens
        if( !mMainWindow->isLoading() )
            mMainWindow->updateImage();
    }
    void on_markerBorderBits_valueChanged(int /*i*/)
    {
        mMainWindow->setRecognitionChanged(true);// flag indicates that changes of recognition parameters happens
        if( !mMainWindow->isLoading() )
            mMainWindow->updateImage();
    }
    void on_perspectiveRemovePixelPerCell_valueChanged(int /*i*/)
    {
        mMainWindow->setRecognitionChanged(true);// flag indicates that changes of recognition parameters happens
        if( !mMainWindow->isLoading() )
            mMainWindow->updateImage();
    }
    void on_perspectiveRemoveIgnoredMarginPerCell_valueChanged(double /*val*/)
    {
        mMainWindow->setRecognitionChanged(true);// flag indicates that changes of recognition parameters happens
        if( !mMainWindow->isLoading() )
            mMainWindow->updateImage();
    }
    void on_maxErroneousBitsInBorderRate_valueChanged(double /*val*/)
    {
        mMainWindow->setRecognitionChanged(true);// flag indicates that changes of recognition parameters happens
        if( !mMainWindow->isLoading() )
            mMainWindow->updateImage();
    }
    void on_errorCorrectionRate_valueChanged(double /*val*/)
    {
        mMainWindow->setRecognitionChanged(true);// flag indicates that changes of recognition parameters happens
        if( !mMainWindow->isLoading() )
            mMainWindow->updateImage();
    }
    void on_minOtsuStdDev_valueChanged(double /*val*/)
    {
        mMainWindow->setRecognitionChanged(true);// flag indicates that changes of recognition parameters happens
        if( !mMainWindow->isLoading() )
            mMainWindow->updateImage();
    }



private:
    Petrack *mMainWindow;
};

#endif
