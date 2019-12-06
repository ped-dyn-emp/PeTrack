#ifndef COLORMARKERWIDGET_H
#define COLORMARKERWIDGET_H

#include <QtWidgets>
#include "ui_colorMarker.h"

#include "petrack.h"
#include "colorMarkerItem.h"

class ColorMarkerWidget: public QWidget, public Ui::ColorMarker
{
    Q_OBJECT

public:
    ColorMarkerWidget(QWidget *parent = 0);

    // store data in xml node
    void setXml(QDomElement &elem);

    // read data from xml node
    void getXml(QDomElement &elem);

    int fromHue;
    int fromSat;
    int fromVal;
    int toHue;
    int toSat;
    int toVal;

private slots:

    // Functions which just modifies the visual
    void on_showMask_stateChanged(int i)
    {
        mMainWindow->getColorMarkerItem()->setVisible(i);
        mMainWindow->getScene()->update();
        // mMainWindow->updateImage();
    }
//    void on_colorMarkerColor_currentIndexChanged(int i)
//    {
//        if (mMainWindow && (mMainWindow->getScene()))
//            mMainWindow->getScene()->update();
//    }
    void on_maskMask_stateChanged(int i)
    {
        mMainWindow->getScene()->update();
    }
    void on_opacity_valueChanged(int i)
    {
        mMainWindow->getScene()->update();
    }

    // functions which force a new recognition
    void on_inversHue_stateChanged(int i)
    {
        mMainWindow->setRecognitionChanged(true);// flag indicates that changes of recognition parameters happens
        if( !mMainWindow->isLoading() )
            mMainWindow->updateImage();
    }
    void on_useOpen_stateChanged(int i)
    {
        mMainWindow->setRecognitionChanged(true);// flag indicates that changes of recognition parameters happens
        if( !mMainWindow->isLoading() )
            mMainWindow->updateImage();
    }
    void on_useClose_stateChanged(int i)
    {
        mMainWindow->setRecognitionChanged(true);// flag indicates that changes of recognition parameters happens
        if( !mMainWindow->isLoading() )
            mMainWindow->updateImage();
    }

    void on_closeRadius_valueChanged(int i)
    {
        mMainWindow->setRecognitionChanged(true);// flag indicates that changes of recognition parameters happens
        if( !mMainWindow->isLoading() )
            mMainWindow->updateImage();
    }
    void on_openRadius_valueChanged(int i)
    {
        mMainWindow->setRecognitionChanged(true);// flag indicates that changes of recognition parameters happens
        if( !mMainWindow->isLoading() )
            mMainWindow->updateImage();
    }
    void on_minArea_valueChanged(int i)
    {
        mMainWindow->setRecognitionChanged(true);// flag indicates that changes of recognition parameters happens
        if( !mMainWindow->isLoading() )
            mMainWindow->updateImage();
    }
    void on_maxArea_valueChanged(int i)
    {
        mMainWindow->setRecognitionChanged(true);// flag indicates that changes of recognition parameters happens
        if( !mMainWindow->isLoading() )
            mMainWindow->updateImage();
    }
    void on_maxRatio_valueChanged(double d)
    {
        mMainWindow->setRecognitionChanged(true);// flag indicates that changes of recognition parameters happens
        if( !mMainWindow->isLoading() )
            mMainWindow->updateImage();
    }

    void on_fromTriangle_colorChanged(const QColor &col);
    void on_toTriangle_colorChanged(const QColor &col);

    void on_fromColor_clicked();
    void on_toColor_clicked();

private:
    Petrack *mMainWindow;
};

#endif
