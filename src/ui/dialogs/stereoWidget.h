/*
 * PeTrack - Software for tracking pedestrians movement in videos
 * Copyright (C) 2024 Forschungszentrum Jülich GmbH, IAS-7
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

#include "ui_stereo.h"

#include <QtWidgets>
class Petrack;
class QDomElement;

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
    void on_stereoUseForHeight_stateChanged(int /*i*/);
    void on_stereoUseForHeightEver_stateChanged(int /*i*/);

    void on_stereoShowDisparity_stateChanged(int i);
    void on_stereoColor_currentIndexChanged(int /*i*/);
    void on_stereoDispAlgo_currentIndexChanged(int /*i*/);
    void on_hideWrong_stateChanged(int /*i*/);
    void on_stereoMaskSize_valueChanged(int i);
    void on_opacity_valueChanged(int /*i*/);
    void on_edgeMaskSize_valueChanged(int i);
    // funkioniert noch nicht richtig, da trotz des sprungs nach preprocess
    // zuerst das ergebniss des zuletzt berechneten frames mit edge angezeigt wird!!!!!!!!!
    void on_useEdge_stateChanged(int /*i*/);
    void on_maxDisparity_valueChanged(int /*i*/);
    void on_minDisparity_valueChanged(int /*i*/);

    void on_stereoExport_clicked();

private:
    Petrack *mMainWindow;
};

#endif
