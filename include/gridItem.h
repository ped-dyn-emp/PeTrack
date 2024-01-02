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

#ifndef GRIDITEM_H
#define GRIDITEM_H

class Petrack;
class CoordinateSystemBox;
class AlignmentGridBox;
struct Grid2D;
struct Grid3D;

#include "extrCalibration.h"

#include <QGraphicsItem>
#include <utility>


class GridItem : public QGraphicsItem
{
private:
    Petrack             *mMainWindow;
    ExtrCalibration     *mExtrCalib;
    CoordinateSystemBox *mCoordSys;
    AlignmentGridBox    *mGridBox;
    float                mMouseX, mMouseY;
    int                  mGridTransX, mGridTransY;
    int                  mGridDimension;

public:
    inline void setGridDimension(int gDimension) { this->mGridDimension = gDimension; }
    inline int  getGridDimension() const { return this->mGridDimension; }
    GridItem(QWidget *wParent, QGraphicsItem *parent, CoordinateSystemBox *coordSys, AlignmentGridBox *gridBox);
    QRectF boundingRect() const override;
    void   mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void   mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void   drawLine(QPainter *painter, const std::array<cv::Point2f, 2> &p);
    void   paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

private:
    void draw2DGrid(QPainter *painter, const Grid2D &params, int imageHeight, int imageWidth, int borderSize);
    void draw3DGrid(
        QPainter     *painter,
        const Grid3D &params,
        int           imageHeight,
        int           imageWidth,
        int           borderSize,
        bool          vanishPointYIsInsideImage,
        bool          vanishPointXIsInsideImage,
        cv::Point2f   vanishPointY,
        cv::Point2f   vanishPointX);
};

#endif
