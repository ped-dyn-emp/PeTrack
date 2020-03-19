#ifndef STEREOWIDGET_H
#define STEREOWIDGET_H

#include <QtWidgets>
#include "ui_stereo.h"

#include "petrack.h"
#include "stereoItem.h"

class StereoWidget: public QWidget, public Ui::Stereo
{
    Q_OBJECT

public:
    StereoWidget(QWidget *parent = 0);

    // store data in xml node
    void setXml(QDomElement &elem);

    // read data from xml node
    void getXml(QDomElement &elem);

private slots:
#ifndef STEREO_DISABLED
    void on_stereoUseForHeight_stateChanged(int i)
    {
        if (stereoUseForHeightEver->isChecked() && stereoUseForHeight->isChecked())  
            mMainWindow->updateImage();
    }
    void on_stereoUseForHeightEver_stateChanged(int i)
    {
        if (stereoUseForHeightEver->isChecked() && stereoUseForHeight->isChecked())
            mMainWindow->updateImage();
    }

    void on_stereoShowDisparity_stateChanged(int i)
    {
        mMainWindow->getStereoItem()->setVisible(i);
        mMainWindow->getScene()->update();
    }
    void on_stereoColor_currentIndexChanged(int i)
    {
        if (mMainWindow && (mMainWindow->getScene()))
            mMainWindow->getScene()->update();
    }
    void on_stereoDispAlgo_currentIndexChanged(int i)
    {
        if (mMainWindow && (mMainWindow->getScene()) && mMainWindow->getStereoContext())
        {
            mMainWindow->getStereoContext()->indicateNewValues();
            mMainWindow->getScene()->update();
        }
    }
    void on_hideWrong_stateChanged(int i)
    {
        if (mMainWindow && (mMainWindow->getScene()) && mMainWindow->getStereoContext())
        {
            mMainWindow->getStereoContext()->indicateNewValues();
            mMainWindow->getScene()->update();
        }
        //mMainWindow->getScene()->update();
    }
    void on_stereoMaskSize_valueChanged(int i)
    {
        if (i%2 == 0)
        {
            stereoMaskSize->setValue(i-1);
            return;
        }
        if (mMainWindow->getStereoContext())
        {
            mMainWindow->getStereoContext()->indicateNewValues();
            mMainWindow->getScene()->update();
        }
    }
    void on_opacity_valueChanged(int i)
    {
        mMainWindow->getScene()->update();
    }
    void on_edgeMaskSize_valueChanged(int i)
    {
        if (i%2 == 0)
        {
            edgeMaskSize->setValue(i-1);
            return;
        }
        if (mMainWindow->getStereoContext())
        {
            mMainWindow->getStereoContext()->preprocess();
            mMainWindow->getStereoContext()->indicateNewValues();
            mMainWindow->getScene()->update();
        }
    }
    // funkioniert noch nicht richtig, da trotz des sprungs nach preprocess
    // zuerst das ergebniss des zulöetzt berechneten frames mit edge angezeigt wird!!!!!!!!!
    void on_useEdge_stateChanged(int i)
    {
        if (mMainWindow->getStereoContext())
        {
            mMainWindow->getStereoContext()->preprocess();
            mMainWindow->getStereoContext()->indicateNewValues();
            mMainWindow->getScene()->update();
        }
    }
    void on_maxDisparity_valueChanged(int i)
    {
        if (mMainWindow->getStereoContext())
        {
            mMainWindow->getStereoContext()->indicateNewValues();
            mMainWindow->getScene()->update();
        }
    }
    void on_minDisparity_valueChanged(int i)
    {
        if (mMainWindow->getStereoContext())
        {
            mMainWindow->getStereoContext()->indicateNewValues();
            mMainWindow->getScene()->update();
        }
    }

    void on_stereoExport_clicked()
    {
        mMainWindow->getStereoContext()->exportPointCloud();
    }
#endif

private:
    Petrack *mMainWindow;
};

#endif
