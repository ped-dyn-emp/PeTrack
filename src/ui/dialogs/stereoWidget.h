/*
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
    void onStereoUseForHeightStateChanged();
    void onStereoUseForHeightEverStateChanged();
    void onStereoShowDisparityStateChanged(int i);
    void onStereoColorCurrentIndexChanged();
    void onStereoDispAlgoCurrentIndexChanged();
    void onHideWrongStateChanged();
    void onStereoMaskSizeValueChanged(int i);
    void onOpacityValueChanged();
    void onEdgeMaskSizeValueChanged(int i);
    void onUseEdgeStateChanged();
    void onMaxDisparityValueChanged();
    void onMinDisparityValueChanged();
    void onStereoExportClicked();

private:
    Petrack *mMainWindow;
};

#endif
