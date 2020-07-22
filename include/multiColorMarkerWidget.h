#ifndef MULTICOLORMARKERWIDGET_H
#define MULTICOLORMARKERWIDGET_H

#include <QtWidgets>
#include "ui_multiColorMarker.h"

#include "petrack.h"
#include "multiColorMarkerItem.h"
#include "codeMarkerWidget.h"
#include "imageItem.h"
#include "control.h"

class MultiColorMarkerWidget: public QWidget, public Ui::MultiColorMarker
{
    Q_OBJECT

public:
    MultiColorMarkerWidget(QWidget *parent = 0);

    // store data in xml node
    void setXml(QDomElement &elem);

    // read data from xml node
    void getXml(QDomElement &elem);

private slots:

    void on_useDot_stateChanged(int i)
    {
        mMainWindow->setRecognitionChanged(true);// flag indicates that changes of recognition parameters happens
        mMainWindow->updateImage();
    }
    void on_dotSize_valueChanged(double d)
    {
        mMainWindow->setRecognitionChanged(true);// flag indicates that changes of recognition parameters happens
        mMainWindow->updateImage();
    }
    void on_useCodeMarker_stateChanged(int /* i */)
    {
        mMainWindow->setRecognitionChanged(true);
        mMainWindow->updateImage();
    }
    void on_CodeMarkerParameter_clicked()
    {
        mMainWindow->getCodeMarkerWidget()->show();
    }
    void on_ignoreWithoutDot_stateChanged(int i)
    {
        mMainWindow->setRecognitionChanged(true);// flag indicates that changes of recognition parameters happens
        mMainWindow->updateImage();
    }
    void on_useColor_stateChanged(int i) // eigentlich nichts noetig, da nur beim Tracing aktiv
    {
        mMainWindow->setRecognitionChanged(true);// flag indicates that changes of recognition parameters happens
        mMainWindow->updateImage();
    }
    void on_restrictPosition_stateChanged(int i)
    {
        mMainWindow->setRecognitionChanged(true);// flag indicates that changes of recognition parameters happens
        mMainWindow->updateImage();
    }
    void on_autoCorrect_stateChanged(int i)
    {
        mMainWindow->setRecognitionChanged(true);// flag indicates that changes of recognition parameters happens
        mMainWindow->updateImage();
    }
    void on_autoCorrectOnlyExport_stateChanged(int i)
    {
        mMainWindow->setRecognitionChanged(true);// flag indicates that changes of recognition parameters happens
        mMainWindow->updateImage();
    }

    // Functions which just modifies the visual
    void on_showMask_stateChanged(int i)
    {
        mMainWindow->getMultiColorMarkerItem()->setVisible(i);
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
    void on_useHeadSize_stateChanged(int i)
    {
        if (i)
        {
            if (mMainWindow->getImageItem() && mMainWindow->getImage() && mMainWindow->getControlWidget())
            {
                QPointF cmPerPixel1 = mMainWindow->getImageItem()->getCmPerPixel(0, 0, mMainWindow->getControlWidget()->mapDefaultHeight->value());
                QPointF cmPerPixel2 = mMainWindow->getImageItem()->getCmPerPixel(mMainWindow->getImage()->width()-1, mMainWindow->getImage()->height()-1, mMainWindow->getControlWidget()->mapDefaultHeight->value());
                double cmPerPixelAvg = (cmPerPixel1.x()+cmPerPixel1.y()+cmPerPixel2.x()+cmPerPixel2.y())/4.;
                //debout <<cmPerPixel1.x()<<" "<<cmPerPixel1.y()<<" "<<cmPerPixel2.x()<<" "<<cmPerPixel2.y()<<endl;
                //debout << 21./cmPerPixelAvg<<endl;
                if (cmPerPixelAvg > 0)
                {
                    double area = PI*0.25 * HEAD_SIZE/cmPerPixelAvg * 14./cmPerPixelAvg; // 14. Kopfbreite // Elipse: A=Pi*a*b (a,b Halbachsen)
                    //debout <<area <<endl;
                    mOldMinArea = minArea->value();
                    mOldMaxArea = maxArea->value();

                    //debout << oldMinArea <<endl;
                    //debout << oldMaxArea <<endl;
                    minArea->setValue(area*0.75);
                    maxArea->setValue(area*2.5);
                }
            }
            minArea->setDisabled(true);
            maxArea->setDisabled(true);
        }
        else
        {
            minArea->setDisabled(false);
            maxArea->setDisabled(false);
            minArea->setValue(mOldMinArea);
            maxArea->setValue(mOldMaxArea);
        }

        mMainWindow->setRecognitionChanged(true);// flag indicates that changes of recognition parameters happens
        mMainWindow->updateImage();
    }

    void on_maxRatio_valueChanged(double d)
    {
        mMainWindow->setRecognitionChanged(true);// flag indicates that changes of recognition parameters happens
        if( !mMainWindow->isLoading() )
            mMainWindow->updateImage();
    }

private:
    Petrack *mMainWindow;

    int mOldMinArea;
    int mOldMaxArea;
};

#endif
