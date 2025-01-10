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

#ifndef MULTICOLORMARKERWIDGET_H
#define MULTICOLORMARKERWIDGET_H

#include "codeMarkerWidget.h"
#include "imageItem.h"
#include "multiColorMarkerItem.h"
#include "ui_multiColorMarker.h"
#include "worldImageCorrespondence.h"

#include <QtWidgets>
class Petrack;

class MultiColorMarkerWidget : public QWidget, public Ui::MultiColorMarker
{
    Q_OBJECT

public:
    MultiColorMarkerWidget(QWidget *parent = nullptr);

    // store data in xml node
    void setXml(QDomElement &elem);

    // read data from xml node
    void getXml(QDomElement &elem);

private slots:

    void on_useDot_stateChanged(int /*i*/);
    void on_dotSize_valueChanged(double /*d*/);
    void on_useCodeMarker_stateChanged(int /* i */);
    void on_CodeMarkerParameter_clicked();
    void on_ignoreWithoutDot_stateChanged(int /*i*/);
    void on_useColor_stateChanged(int /*i*/);
    void on_restrictPosition_stateChanged(int /*i*/);
    void on_autoCorrect_stateChanged(int /*i*/);
    void on_autoCorrectOnlyExport_stateChanged(int /*i*/);

    // Functions which just modifies the visual
    void on_showMask_stateChanged(int i);
    void on_maskMask_stateChanged(int /*i*/);
    void on_opacity_valueChanged(int /*i*/);

    // functions which force a new recognition
    void on_useOpen_stateChanged(int /*i*/);
    void on_useClose_stateChanged(int /*i*/);

    void on_closeRadius_valueChanged(int /*i*/);
    void on_openRadius_valueChanged(int /*i*/);

    void on_minArea_valueChanged(int /*i*/);
    void on_maxArea_valueChanged(int /*i*/);
    void on_useHeadSize_stateChanged(int i);

    void on_maxRatio_valueChanged(double /*d*/);

private:
    Petrack *mMainWindow;

    int mOldMinArea;
    int mOldMaxArea;
};

#endif
