/*
 * PeTrack - Software for tracking pedestrians movement in videos
 * Copyright (C) 2010-2021 Forschungszentrum Jülich GmbH,
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

#ifndef TRACKERITEM_H
#define TRACKERITEM_H

#include <QGraphicsItem>

class Petrack;
class Control;
class PersonStorage;

class TrackerItem : public QGraphicsItem
{
private:
    Petrack *      mMainWindow;
    Control *      mControlWidget;
    PersonStorage &mPersonStorage;

public:
    TrackerItem(QWidget *wParent, PersonStorage &tracker, QGraphicsItem *parent = nullptr);
    void   contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
    QRectF boundingRect() const;
    void   paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
};

#endif
