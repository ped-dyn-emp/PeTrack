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

#ifndef COLORRANGEWIDGET_H
#define COLORRANGEWIDGET_H

#include "colorPlot.h"
#include "ui_colorRange.h"

#include <QtWidgets>

class Petrack;

class ColorRangeWidget : public QWidget, public Ui::ColorRange
{
    Q_OBJECT

public:
    ColorRangeWidget(QWidget *parent = nullptr);

    int mFromHue;
    int mFromSat;
    int mFromVal;
    int mToHue;
    int mToSat;
    int mToVal;

    void setInvHue(bool b);
    void setFromColor(const QColor &col);
    void setToColor(const QColor &col);
    void setControlWidget(int toHue, int fromHue, int toSat, int fromSat);

private slots:
    void onInversHueStateChanged(int i);
    void onFromTriangleColorChanged(const QColor &col);
    void onToTriangleColorChanged(const QColor &col);
    void onFromColorClicked();
    void onToColorClicked();

private:
    Petrack   *mMainWindow;
    ColorPlot *mColorPlot;
};

#endif
