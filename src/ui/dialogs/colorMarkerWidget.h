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

#ifndef COLORMARKERWIDGET_H
#define COLORMARKERWIDGET_H

#include "ui_colorMarker.h"

#include <QDomElement>

class QDomElement;
class Petrack;

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
    void on_showMask_stateChanged(int i);

    void on_maskMask_stateChanged(int /*i*/);
    void on_opacity_valueChanged(int /*i*/);

    // functions which force a new recognition
    void on_inversHue_stateChanged(int /*i*/);

    void on_useOpen_stateChanged(int /*i*/);

    void on_useClose_stateChanged(int /*i*/);

    void on_closeRadius_valueChanged(int /*i*/);

    void on_openRadius_valueChanged(int /*i*/);

    void on_minArea_valueChanged(int /*i*/);

    void on_maxArea_valueChanged(int /*i*/);

    void on_maxRatio_valueChanged(double /*d*/);

    void on_fromTriangle_colorChanged(const QColor &col);
    void on_toTriangle_colorChanged(const QColor &col);

    void on_fromColor_clicked();
    void on_toColor_clicked();

private:
    Petrack *mMainWindow;
};

#endif
