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

#include "polygonSelection.h"

#include <QObject>
#include <QPainterPath>
#include <QVector2D>


void PolygonSelection::paint(QPainter &painter)
{
    QPen pen;
    pen.setColor(QColor(255, 0, 255));
    painter.setPen(pen);
    if(completed)
    {
        QBrush brush;
        brush.setStyle(Qt::SolidPattern);
        brush.setColor(QColor(255, 0, 255, 100));
        pen.setBrush(brush);
        painter.setBrush(brush);
        QPolygonF polygon(points);
        painter.drawPolygon(polygon, Qt::FillRule::WindingFill);
        return;
    }
    if(points.empty())
    {
        return;
    }
    for(int i = 0; i < (points.size() - 1); i++)
    {
        painter.drawEllipse(points.at(i), 5, 5);
        painter.drawLine(points.at(i), points.at(i + 1));
        painter.drawEllipse(points.at(i + 1), 5, 5);
    }
    pen.setColor(QColor(0, 255, 0));
    painter.setPen(pen);
    painter.drawEllipse(points.front(), this->completionDist, this->completionDist);
}

void PolygonSelection::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if(completed)
    {
        return;
    }
    QPointF position = event->pos();
    if(points.empty())
    {
        points.push_back(position);
        return;
    }
    points.push_back(position);

    // detect finished drawing
    auto first = points.front();
    auto v1    = QVector2D(position);
    auto v2    = QVector2D(first);

    if(v1.distanceToPoint(v2) <= this->completionDist)
    {
        // replace last for perfect match
        points.pop_back();
        points.push_back(first);
        setCompleted(true);
        this->completionCallback(points);
        return;
    }
}

void PolygonSelection::reset()
{
    points.clear();
    setCompleted(false);
}


void PolygonSelection::setCompleted(bool value)
{
    if(value == this->completed)
    {
        return;
    }
    SPDLOG_DEBUG("state changed");
    this->completed = value;
    emit completionStatusChanged(value);
}