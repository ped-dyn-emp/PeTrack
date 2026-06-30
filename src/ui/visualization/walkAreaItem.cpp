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

#include "walkAreaItem.h"

#include "calibration/walkAreaManager.h"
#include "penUtils.h"
#include "petrack.h"

#include <QGraphicsSceneMouseEvent>
#include <QPainter>

WalkAreaItem::WalkAreaItem(Petrack *mw, WalkAreaManager *mgr, QGraphicsItem *parent) :
    QGraphicsObject(parent), mMainWindow(mw), mManager(mgr)
{
    setAcceptedMouseButtons(Qt::LeftButton);
    setAcceptHoverEvents(true);
    setVisible(mManager->isVisible());
    connect(
        mManager,
        &WalkAreaManager::visibilityChanged,
        this,
        [this](bool v)
        {
            setVisible(v);
            update();
        });
    connect(mManager, &WalkAreaManager::imageUpdate, this, [this]() { update(); });
    connect(mManager, &WalkAreaManager::geometryChanged, this, [this]() { update(); });
}

QRectF WalkAreaItem::boundingRect() const
{
    if(mMainWindow->getImage())
    {
        return QRectF(
            -mMainWindow->getImageBorderSize(),
            -mMainWindow->getImageBorderSize(),
            mMainWindow->getImage()->width(),
            mMainWindow->getImage()->height());
    }
    return QRectF(0, 0, 0, 0);
}

void WalkAreaItem::paint(QPainter *p, const QStyleOptionGraphicsItem *, QWidget *)
{
    if(!mManager->isVisible())
    {
        return;
    }

    auto drawPoly = [&](const walkarea::Polygon &poly)
    {
        if(poly.disabled())
        {
            return;
        }

        if(poly.type() == walkarea::PolygonType::Walkable && !mManager->isWalkableVisible())
        {
            return;
        }
        if(poly.type() == walkarea::PolygonType::Obstacle && !mManager->isObstacleVisible())
        {
            return;
        }

        const bool isObs = (poly.type() == walkarea::PolygonType::Obstacle);
        QPen       pen(isObs ? QColor(220, 80, 80, 220) : QColor(60, 160, 240, 220));
        pen.setWidth(2);
        QColor fill = isObs ? QColor(220, 80, 80, 90) : QColor(60, 160, 240, 90);
        p->setPen(scaledPen(pen));
        p->setBrush(QBrush(fill));
        p->drawPolygon(poly, Qt::WindingFill);
    };

    // Draw walkable area
    if(mManager->getWalkableArea().has_value())
    {
        drawPoly(*mManager->getWalkableArea());
    }

    // Draw all obstacles
    for(const auto &kv : mManager->getObstacles())
    {
        drawPoly(kv.second);
    }

    // active selection
    const bool nextIsObs = mManager->getDrawingType() == walkarea::PolygonType::Obstacle;
    QColor     fill      = nextIsObs ? QColor(220, 80, 80, 90) : QColor(60, 160, 240, 90);
    p->setBrush(QBrush(fill));
    mManager->selection().paint(*p);
}

void WalkAreaItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    if(mManager->isEditMode())
    {
        // Forward to PolygonSelection
        mManager->selection().mousePressEvent(event);
        update();
    }
    else
    {
        QGraphicsObject::mouseDoubleClickEvent(event);
    }
}

void WalkAreaItem::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    if(mManager->isEditMode())
    {
        mManager->selection().setCurrentMousePos(event->pos());
        update();
        return;
    }

    QGraphicsObject::hoverMoveEvent(event);
}
