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

#ifndef RECOGNITIONROIITEM_H
#define RECOGNITIONROIITEM_H

#include <QGraphicsRectItem>

class Petrack;
class Control;


class RecognitionRoiItem : public QGraphicsRectItem
{
    inline static constexpr int DISTANCE_TO_BORDER = 5;
    inline static constexpr int MIN_SIZE = 10;
    enum pressLocation{inside, top, bottom, left, right, topLeft, topRight, bottomLeft, bottomRight};

private:
    Petrack *mMainWindow;
    Control *mControlWidget;
    QRect mPressRect;
    QPointF mPressPos;
    enum pressLocation mPressLocation;

public:
    RecognitionRoiItem(QWidget *wParent, QGraphicsItem * parent = nullptr);
    void mousePressEvent(QGraphicsSceneMouseEvent * event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent * event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void hoverMoveEvent(QGraphicsSceneHoverEvent *event);
    void checkRect();
};

#endif
