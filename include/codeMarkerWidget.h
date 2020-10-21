#ifndef CODEMARKERWIDGET_H
#define CODEMARKERWIDGET_H

#include <QtWidgets>
#include "opencv2/aruco.hpp"

#include "ui_codeMarker.h"

#include "petrack.h"
#include "codeMarkerItem.h"


class CodeMarkerWidget: public QWidget, public Ui::CodeMarker
{
    Q_OBJECT

public:
    CodeMarkerWidget(QWidget *parent = 0);

    // store data in xml node
    void setXml(QDomElement &elem);

    // read data from xml node
    void getXml(QDomElement &elem);

#if 0 // Maik temporaer, damit es auf dem Mac laeuft
    aruco::Dictionary usedDictionary;
    aruco::DetectorParameters detectParams;
#endif

private slots:

    void on_showDetectedCandidates_stateChanged(int i)
    {

        mMainWindow->getCodeMarkerItem()->setVisible(i);
//        mMainWindow->setRecognitionChanged(true);// flag indicates that changes of recognition parameters happens
//        mMainWindow->getCodeMarkerItem()->setVisible(mMainWindow->getCodeMarkerWidget()->showDetectedCandidates->isChecked() || mMainWindow->getCodeMarkerWidget()->showMask->isChecked());
//        mMainWindow->getScene()->update();
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
