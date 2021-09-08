/*
 * PeTrack - Software for tracking pedestrians movement in videos
 * Copyright (C) 2010-2020 Forschungszentrum Jülich GmbH,
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

#ifndef GRIDITEM_H
#define GRIDITEM_H

class Petrack;
class Control;

#include "extrCalibration.h"

#include <QGraphicsItem>

class GridItem : public QGraphicsItem
{
private:
    Petrack *        mMainWindow;
    ExtrCalibration *extCalib;
    Control *        mControlWidget;
    float            mouse_x, mouse_y;
    int              gridTrans_x, gridTrans_y;
    int              gridDimension;

public:
    inline void setGridDimension(int gDimension) { this->gridDimension = gDimension; }
    inline int  getGridDimension() const { return this->gridDimension; }
    GridItem(QWidget *wParent, QGraphicsItem *parent = nullptr);
    QRectF boundingRect() const;
    void   mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void   mousePressEvent(QGraphicsSceneMouseEvent *event);
    int    drawLine(QPainter *painter, cv::Point2f *p, int y_offset);
    void   paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
};

#endif
