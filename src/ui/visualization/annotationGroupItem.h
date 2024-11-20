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

#ifndef PETRACK_ANNOTATIONGROUPITEM_H
#define PETRACK_ANNOTATIONGROUPITEM_H


#include <QGraphicsItem>

class Petrack;
class Animation;
class PersonStorage;
class AnnotationGroupManager;

/**
 * This item will, if the group visualization is enabled in the group widget, draw a colored ellipse above each
 * TrackPerson. The color is determined by the Annotation groups it belongs to at that exact frame. If no group is
 * assigned, nothing will be drawn.
 */
class AnnotationGroupItem : public QGraphicsItem
{
private:
    Petrack                      &mPetrack;
    const PersonStorage          &mPersonStorage;
    const AnnotationGroupManager &mGroupManager;
    const Animation              &mAnimation;

public:
    AnnotationGroupItem(
        const AnnotationGroupManager &manager,
        Petrack                      &petrack,
        const PersonStorage          &storage,
        const Animation              &animation,
        QGraphicsItem                *parent = nullptr);
    QRectF boundingRect() const;
    void   paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
};


#endif // PETRACK_ANNOTATIONGROUPITEM_H
