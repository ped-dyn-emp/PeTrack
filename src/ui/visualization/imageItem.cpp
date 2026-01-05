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

#include "imageItem.h"

#include "petrack.h"

#include <QGraphicsScene>
#include <QGraphicsSceneHoverEvent>
#include <QPainter>
#include <cmath>

ImageItem::ImageItem(QWidget *wParent, QGraphicsItem *parent) : QGraphicsObject(parent)
{
    mMainWindow = (class Petrack *) wParent;
    mImage      = nullptr;
    setCursor(Qt::CrossCursor);
    setAcceptHoverEvents(true);
}

/**
 * @brief Bounding box of drawn to area.
 *
 * This bounding box is used to determine if this Item needs to be redrawn or not.
 * See the official Qt Docs for QGraphicsItem
 *
 * @return (updated) bounding rect of this item
 */
QRectF ImageItem::boundingRect() const
{
    if(mImage)
    {
        return QRectF(0, 0, mImage->width(), mImage->height());
    }
    else
    {
        return QRectF(0, 0, 0, 0);
    }
}

void ImageItem::paint(QPainter *painter, const QStyleOptionGraphicsItem * /*option*/, QWidget * /*widget*/)
{
    if(mImage)
    {
        painter->drawImage(0, 0, *mImage);
    }
}

void ImageItem::setImage(QImage *img)
{
    if(img->isNull())
    {
        mImage = nullptr;
        setTransform(QTransform());
        mMainWindow->updateSceneRect();
        return;
    }
    mImage = img;


    QTransform matrix;
    matrix.translate(
        1, 1); // FEHLER IN QT ????? noetig, damit trotz recognitionroiitem auch image auch ohne border komplett neu
               // gezeichnet wird // wird 2 zeilen weiter zurueckgesetzt, aber mit 0, 0 geht es nicht
    setTransform(matrix);
    matrix.translate(-mMainWindow->getImageBorderSize() - 1, -mMainWindow->getImageBorderSize() - 1);
    setTransform(matrix);
    emit imageChanged(mImage->width(), mImage->height(), 0);
}

// event, of moving mouse when mouse button is pressed (grid is over image)
void ImageItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsItem::mouseMoveEvent(event);
}

/**
 * @brief Updates the mousePosOnImage in Petrack
 * @param event
 */
void ImageItem::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    // sets pixel coord on image for further use
    // QPointF pos = event->pos();
    mMainWindow->setMousePosOnImage(event->pos());

    QGraphicsItem::hoverMoveEvent(event);
}
