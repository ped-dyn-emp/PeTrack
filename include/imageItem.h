/*
 * PeTrack - Software for tracking pedestrians movement in videos
 * Copyright (C) 2023 Forschungszentrum Jülich GmbH, IAS-7
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

#ifndef IMAGEITEM_H
#define IMAGEITEM_H

#include <QGraphicsItem>

class Petrack;
class Control;

class ImageItem : public QGraphicsItem
{
private:
    Petrack       *mMainWindow;
    Control       *mControlWidget;
    QImage        *mImage;
    QGraphicsItem *mCoordItem;

public:
    ImageItem(QWidget *wParent, QGraphicsItem *parent = nullptr);

    QRectF boundingRect() const;
    void   paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    void    setImage(QImage *img);
    void    setCoordItem(QGraphicsItem *ci);
    double  getCmPerPixel();
    QPointF getCmPerPixel(float px, float py, float h = 0.);
    double  getAngleToGround(float px, float py, float h = 0);
    QPointF getPosImage(QPointF pos, float height = 0.);
    QPointF getPosReal(QPointF pos, double height = 0.);

    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void hoverMoveEvent(QGraphicsSceneHoverEvent *event);
};

#endif
