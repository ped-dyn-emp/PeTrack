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


#include "annotationGroupItem.h"

#include "animation.h"
#include "annotationGroupManager.h"
#include "personStorage.h"
#include "petrack.h"

#include <QPainter>

AnnotationGroupItem::AnnotationGroupItem(
    const AnnotationGroupManager &manager,
    Petrack                      &petrack,
    const PersonStorage          &storage,
    const Animation              &animation,
    QGraphicsItem                *parent) :
    QGraphicsItem(parent), mPetrack(petrack), mPersonStorage(storage), mGroupManager(manager), mAnimation(animation)
{
}

QRectF AnnotationGroupItem::boundingRect() const
{
    if(mPetrack.getImage())
    {
        return QRectF(
            -mPetrack.getImageBorderSize(),
            -mPetrack.getImageBorderSize(),
            mPetrack.getImage()->width(),
            mPetrack.getImage()->height());
    }

    return QRectF(0, 0, 0, 0);
}

void AnnotationGroupItem::paint(QPainter *painter, const QStyleOptionGraphicsItem * /*option*/, QWidget * /*widget*/)
{
    if(!mGroupManager.isVisualizationEnabled())
    {
        return;
    }
    QPen   pen;
    QBrush brush;
    brush.setStyle(Qt::SolidPattern);
    pen.setColor(Qt::black);
    pen.setBrush(brush);

    const int currentFrame = mAnimation.getCurrentFrameNum();

    for(size_t i = 0; i < mPersonStorage.nbPersons(); i++)
    {
        const auto &groups = mPersonStorage.getGroupList(i);

        if(groups.contains(currentFrame))
        {
            const auto &trajectory = mPersonStorage.at(i);
            if(trajectory.trackPointExist(currentFrame))
            {
                const auto   group = mGroupManager.getGroup(groups.getValue(currentFrame));
                const QColor color = group.color;

                brush.setColor(color);
                pen.setBrush(brush);
                pen.setColor(color);
                unsigned int radius = mGroupManager.getVisualizationRadius();
                painter->setPen(pen);
                painter->setBrush(brush);
                painter->drawEllipse(trajectory.trackPointAt(currentFrame).toQPointF(), radius, radius);
            }
        }
    }
}