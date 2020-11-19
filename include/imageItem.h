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

#ifndef IMAGEITEM_H
#define IMAGEITEM_H

#include <QGraphicsItem>
//#include <QtWidgets>
//#include <QDragEnterEvent>
//#include <QDropEvent>

class Petrack;
class Control;

class ImageItem: public QGraphicsItem
{
private:
    Petrack *mMainWindow;
    Control *mControlWidget;
//     QPixmap *mImage;
    QImage *mImage;
    QGraphicsItem *mCoordItem;
public:
    ImageItem(QWidget *wParent, QGraphicsItem * parent = 0);

    QRectF boundingRect() const; 
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

//     void setLogo(QImage *img);
    void setImage(QImage *img);
    void setCoordItem(QGraphicsItem *ci);
    double getCmPerPixel();
    QPointF getCmPerPixel(float px, float py, float h = 0.);
    double getAngleToGround(float px, float py, float h = 0);
    QPointF getPosImage(QPointF pos, float height = 0.);
    QPointF getPosReal(QPointF pos, double height = 0.);

//    void dragEnterEvent(QDragEnterEvent *event);
//    void dropEvent(QDropEvent * event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent * event);
    void hoverMoveEvent(QGraphicsSceneHoverEvent * event);
//     bool sceneEvent(QEvent * event);
};

#endif
