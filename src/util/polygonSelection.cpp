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

#include "polygonSelection.h"

#include "logger.h"
#include "penUtils.h"

#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QVector2D>


void PolygonSelection::paint(QPainter &painter)
{
    QPen pen;
    pen = scaledPen(pen);
    pen.setColor(QColor(255, 0, 255));
    painter.setPen(pen);
    if(mCompleted)
    {
        QBrush brush;
        brush.setStyle(Qt::SolidPattern);
        brush.setColor(QColor(255, 0, 255, 100));
        pen.setBrush(brush);
        painter.setBrush(brush);
        QPolygonF polygon(mPoints);
        painter.drawPolygon(polygon, Qt::FillRule::WindingFill);
        return;
    }
    if(mPoints.empty())
    {
        return;
    }
    // draw all lines
    if(mPoints.size() > 0)
    {
        for(int i = 0; i < mPoints.size() - 1; ++i)
        {
            painter.drawLine(mPoints.at(i), mPoints.at(i + 1));
        }
    }
    // draw mouse tracking line
    if(!mCompleted && mShowCurMousePos)
    {
        const QPointF nextPoint =
            QVector2D(mPoints.front()).distanceToPoint(QVector2D(mCurrentMousePos)) <= COMPLETION_DIST ?
                mPoints.front() :
                mCurrentMousePos;

        painter.drawLine(mPoints.back(), nextPoint);
    }
    // draw each ellipse
    for(const QPointF &point : mPoints)
    {
        painter.drawEllipse(point, 5, 5);
    }

    pen.setColor(QColor(0, 255, 0));
    painter.setPen(pen);
    painter.drawEllipse(mPoints.front(), COMPLETION_DIST, COMPLETION_DIST);
}

void PolygonSelection::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if(mCompleted)
    {
        return;
    }
    QPointF position = event->pos();
    if(mPoints.empty())
    {
        mPoints.push_back(position);
        return;
    }
    mPoints.push_back(position);

    // detect finished drawing
    auto first = mPoints.front();
    auto v1    = QVector2D(position);
    auto v2    = QVector2D(first);

    if(v1.distanceToPoint(v2) <= COMPLETION_DIST)
    {
        // replace last for perfect match
        mPoints.pop_back();
        mPoints.push_back(first);
        setCompleted(true);
        mCompletionCallback(mPoints);
        return;
    }
}

void PolygonSelection::reset()
{
    mPoints.clear();
    setCompleted(false);
}


void PolygonSelection::setCompleted(bool comp)
{
    if(comp == mCompleted)
    {
        return;
    }
    SPDLOG_DEBUG("state changed");
    mCompleted = comp;
    emit completionStatusChanged(comp);
}

void PolygonSelection::removeLastPoint()
{
    if(!mPoints.isEmpty())
    {
        mPoints.removeLast();
        SPDLOG_INFO("Removed last point. Points remaining: {}", mPoints.size());
    }
}
