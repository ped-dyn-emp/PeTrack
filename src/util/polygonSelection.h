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

#ifndef PETRACK_POLYGONSELECTION_H
#define PETRACK_POLYGONSELECTION_H

#include "logger.h"

#include <QGraphicsSceneMouseEvent>
#include <QObject>
#include <QPainter>
#include <QPoint>
#include <QWidget>

class PolygonSelection : public QObject
{
    Q_OBJECT
private:
    QVector<QPointF>                      points;
    std::function<void(QVector<QPointF>)> completionCallback = [=](QVector<QPointF>) {};

    float completionDist = 10;
    bool  completed      = false;

    void setCompleted(bool comp);

public:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void paint(QPainter &painter);
    bool isCompleted() { return completed; }

    void reset();
    void setCompletionCallback(std::function<void(QVector<QPointF>)> callback) { completionCallback = callback; }

    const QVector<QPointF> &getPoints() const { return points; }

signals:

    void completionStatusChanged(bool value);
};


#endif // PETRACK_POLYGONSELECTION_H
