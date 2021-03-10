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

#ifndef COORDITEM_H
#define COORDITEM_H

#include <QGraphicsItem>
#include <QtWidgets>
//#include <opencv.hpp>
#include <petrack.h>

class Petrack;
class Control;
class ExtrCalibration;

class CoordItem : public QGraphicsItem
{
private:
    Petrack *mMainWindow;
    ExtrCalibration *extCalib;
    Control *mControlWidget;
    //QGraphicsScene *mScene;
    cv::Point2f ursprung, x, y, z;
    cv::Point2f calibPointsMin, calibPointsMax;
    cv::Point3f x3D, y3D, z3D;
    float mouse_x,mouse_y;
    int coordTrans_x,coordTrans_y;
    //Dimension *coordDimension
    int coordDimension;

public:
    inline void setCoordDimension(int dim/*Dimension *cDimension*/)
    {
        this->coordDimension = dim;
    }
    inline int /*Dimension*/ getCoordDimension()
    {
        return this->coordDimension;
    }

    // Constructor
    CoordItem(QWidget *wParent/*, QGraphicsScene * scene*/, QGraphicsItem * parent = nullptr);
    // mouseMoveEvent-Trigger
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    //void hoverMoveEvent(QGraphicsSceneHoverEvent *event);
    // Update the transformation matrix
    void updateData();
    // defines the minimal rect which contains the coordinate system
    QRectF boundingRect() const;
    // paint method to draw the perspectiv coordinate-system
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
};

#endif
