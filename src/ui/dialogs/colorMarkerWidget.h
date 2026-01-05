/*
 * PeTrack - Software for tracking pedestrians movement in videos
 * Copyright (C) 2026 Forschungszentrum Jülich GmbH, IAS-7
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

    void onShowMaskStateChanged(int state);
    void onMaskMaskStateChanged();
    void onOpacityValueChanged();

    void onInversHueStateChanged();
    void onUseOpenStateChanged();
    void onUseCloseStateChanged();
    void onCloseRadiusValueChanged();
    void onOpenRadiusValueChanged();
    void onMinAreaValueChanged();
    void onMaxAreaValueChanged();
    void onMaxRatioValueChanged();

    void onFromTriangleColorChanged(const QColor &color);
    void onToTriangleColorChanged(const QColor &color);

    void onFromColorClicked();
    void onToColorClicked();

private:
    Petrack *mMainWindow;
};

#endif
