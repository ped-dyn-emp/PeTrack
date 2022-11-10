/*
 * PeTrack - Software for tracking pedestrians movement in videos
 * Copyright (C) 2022 Forschungszentrum Jülich GmbH, IAS-7
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

#ifndef COLORMARKERITEM_H
#define COLORMARKERITEM_H

#include "vector.h"

#include <QGraphicsItem>

class Petrack;
class Control;
class Tracker;

class ColorMarkerItem : public QGraphicsItem
{
private:
    Petrack *mMainWindow;
    QImage  *mImage;
    cv::Mat  mMask;
    Vec2F    mUlc; // upper left corner to draw

public:
    ColorMarkerItem(QWidget *wParent, QGraphicsItem *parent = nullptr);
    QRectF  boundingRect() const;
    void    setRect(Vec2F &v);
    void    paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    cv::Mat getMask() { return mMask; }
    void    setMask(cv::Mat &mask);
    cv::Mat createMask(int w, int h);
};

#endif
