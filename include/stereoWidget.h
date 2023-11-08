/*
 * PeTrack - Software for tracking pedestrians movement in videos
 * Copyright (C) 2023 Forschungszentrum Jülich GmbH, IAS-7
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

#ifndef STEREOWIDGET_H
#define STEREOWIDGET_H

#include "petrack.h"
#include "stereoItem.h"
#include "ui_stereo.h"

#include <QtWidgets>

class StereoWidget : public QWidget, public Ui::Stereo
{
    Q_OBJECT

public:
    StereoWidget(QWidget *parent = nullptr);

    // store data in xml node
    void setXml(QDomElement &elem);

    // read data from xml node
    void getXml(QDomElement &elem);

private slots:
    void on_stereoUseForHeight_stateChanged(int /*i*/)
    {
        if(stereoUseForHeightEver->isChecked() && stereoUseForHeight->isChecked())
            mMainWindow->updateImage();
    }
    void on_stereoUseForHeightEver_stateChanged(int /*i*/)
    {
        if(stereoUseForHeightEver->isChecked() && stereoUseForHeight->isChecked())
            mMainWindow->updateImage();
    }

    void on_stereoShowDisparity_stateChanged(int i)
    {
        mMainWindow->getStereoItem()->setVisible(i);
        mMainWindow->getScene()->update();
    }
    void on_stereoColor_currentIndexChanged(int /*i*/)
    {
        if(mMainWindow && (mMainWindow->getScene()))
            mMainWindow->getScene()->update();
    }
    void on_stereoDispAlgo_currentIndexChanged(int /*i*/)
    {
        if(mMainWindow && (mMainWindow->getScene()) && mMainWindow->getStereoContext())
        {
            mMainWindow->getStereoContext()->indicateNewValues();
            mMainWindow->getScene()->update();
        }
    }
    void on_hideWrong_stateChanged(int /*i*/)
    {
        if(mMainWindow && (mMainWindow->getScene()) && mMainWindow->getStereoContext())
        {
            mMainWindow->getStereoContext()->indicateNewValues();
            mMainWindow->getScene()->update();
        }
        // mMainWindow->getScene()->update();
    }
    void on_stereoMaskSize_valueChanged(int i)
    {
        if(i % 2 == 0)
        {
            stereoMaskSize->setValue(i - 1);
            return;
        }
        if(mMainWindow->getStereoContext())
        {
            mMainWindow->getStereoContext()->indicateNewValues();
            mMainWindow->getScene()->update();
        }
    }
    void on_opacity_valueChanged(int /*i*/) { mMainWindow->getScene()->update(); }
    void on_edgeMaskSize_valueChanged(int i)
    {
        if(i % 2 == 0)
        {
            edgeMaskSize->setValue(i - 1);
            return;
        }
        if(mMainWindow->getStereoContext() && !mMainWindow->isLoading())
        {
            mMainWindow->getStereoContext()->preprocess();
            mMainWindow->getStereoContext()->indicateNewValues();
            mMainWindow->getScene()->update();
        }
    }
    // funkioniert noch nicht richtig, da trotz des sprungs nach preprocess
    // zuerst das ergebniss des zuletzt berechneten frames mit edge angezeigt wird!!!!!!!!!
    void on_useEdge_stateChanged(int /*i*/)
    {
        if(mMainWindow->getStereoContext() && !mMainWindow->isLoading())
        {
            mMainWindow->getStereoContext()->preprocess();
            mMainWindow->getStereoContext()->indicateNewValues();
            mMainWindow->getScene()->update();
        }
    }
    void on_maxDisparity_valueChanged(int /*i*/)
    {
        if(mMainWindow->getStereoContext())
        {
            mMainWindow->getStereoContext()->indicateNewValues();
            mMainWindow->getScene()->update();
        }
    }
    void on_minDisparity_valueChanged(int /*i*/)
    {
        if(mMainWindow->getStereoContext())
        {
            mMainWindow->getStereoContext()->indicateNewValues();
            mMainWindow->getScene()->update();
        }
    }

    void on_stereoExport_clicked() { mMainWindow->getStereoContext()->exportPointCloud(); }

private:
    Petrack *mMainWindow;
};

#endif
