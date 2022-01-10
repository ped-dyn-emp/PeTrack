/*
 * PeTrack - Software for tracking pedestrians movement in videos
 * Copyright (C) 2010-2022 Forschungszentrum Jülich GmbH,
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

#ifndef COLORMARKERWIDGET_H
#define COLORMARKERWIDGET_H

#include "colorMarkerItem.h"
#include "petrack.h"
#include "ui_colorMarker.h"

#include <QtWidgets>

class ColorMarkerWidget : public QWidget, public Ui::ColorMarker
{
    Q_OBJECT

public:
    ColorMarkerWidget(QWidget *parent = nullptr);

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
    }

    void on_maskMask_stateChanged(int /*i*/) { mMainWindow->getScene()->update(); }
    void on_opacity_valueChanged(int /*i*/) { mMainWindow->getScene()->update(); }

    // functions which force a new recognition
    void on_inversHue_stateChanged(int /*i*/)
    {
        mMainWindow->setRecognitionChanged(true); // flag indicates that changes of recognition parameters happens
        if(!mMainWindow->isLoading())
        {
            mMainWindow->updateImage();
        }
    }

    void on_useOpen_stateChanged(int /*i*/)
    {
        mMainWindow->setRecognitionChanged(true); // flag indicates that changes of recognition parameters happens
        if(!mMainWindow->isLoading())
        {
            mMainWindow->updateImage();
        }
    }

    void on_useClose_stateChanged(int /*i*/)
    {
        mMainWindow->setRecognitionChanged(true); // flag indicates that changes of recognition parameters happens
        if(!mMainWindow->isLoading())
        {
            mMainWindow->updateImage();
        }
    }

    void on_closeRadius_valueChanged(int /*i*/)
    {
        mMainWindow->setRecognitionChanged(true); // flag indicates that changes of recognition parameters happens
        if(!mMainWindow->isLoading())
        {
            mMainWindow->updateImage();
        }
    }

    void on_openRadius_valueChanged(int /*i*/)
    {
        mMainWindow->setRecognitionChanged(true); // flag indicates that changes of recognition parameters happens
        if(!mMainWindow->isLoading())
        {
            mMainWindow->updateImage();
        }
    }

    void on_minArea_valueChanged(int /*i*/)
    {
        mMainWindow->setRecognitionChanged(true); // flag indicates that changes of recognition parameters happens
        if(!mMainWindow->isLoading())
        {
            mMainWindow->updateImage();
        }
    }

    void on_maxArea_valueChanged(int /*i*/)
    {
        mMainWindow->setRecognitionChanged(true); // flag indicates that changes of recognition parameters happens
        if(!mMainWindow->isLoading())
        {
            mMainWindow->updateImage();
        }
    }

    void on_maxRatio_valueChanged(double /*d*/)
    {
        mMainWindow->setRecognitionChanged(true); // flag indicates that changes of recognition parameters happens
        if(!mMainWindow->isLoading())
        {
            mMainWindow->updateImage();
        }
    }

    void on_fromTriangle_colorChanged(const QColor &col);
    void on_toTriangle_colorChanged(const QColor &col);

    void on_fromColor_clicked();
    void on_toColor_clicked();

private:
    Petrack *mMainWindow;
};

#endif
