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

#ifndef COORDITEM_H
#define COORDITEM_H

#include <QGraphicsItem>
#include <QtWidgets>
#include <opencv2/core/types.hpp>

class Petrack;
class Control;
class ExtrCalibration;
class CoordinateSystemBox;
struct CoordPose2D;
struct CoordPose3D;

class CoordItem : public QGraphicsObject
{
private:
    Petrack             *mMainWindow;
    ExtrCalibration     *extCalib;
    Control             *mControlWidget;
    CoordinateSystemBox *mCoordSys;
    cv::Point3f          x3D, y3D, z3D;
    float                mouse_x, mouse_y;
    int                  coordTrans_x, coordTrans_y;
    int                  coordDimension;

public:
    inline void setCoordDimension(int dim) { this->coordDimension = dim; }
    inline int  getCoordDimension() const { return this->coordDimension; }

    CoordItem(QWidget *wParent, QGraphicsItem *parent, CoordinateSystemBox *coordSys);

    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;


    // defines the minimal rect which contains the coordinate system
    QRectF boundingRect() const override;

    // paint method to draw the perspectiv coordinate-system
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

private slots:
    // Update the transformation matrix
    void updateData();
};

#endif
