/*
 * PeTrack - Software for tracking pedestrians movement in videos
 * Copyright (C) 2010-2022 Forschungszentrum Jülich GmbH,
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

#ifndef ROIITEM_H
#define ROIITEM_H

#include <QGraphicsRectItem>
#include <QObject>

class Petrack;

class RoiItem : public QObject, public QGraphicsRectItem
{
    Q_OBJECT

    inline static constexpr int DISTANCE_TO_BORDER = 5;
    inline static constexpr int MIN_SIZE           = 10;

    enum class PressLocation
    {
        inside,
        top,
        bottom,
        left,
        right,
        topLeft,
        topRight,
        bottomLeft,
        bottomRight
    };

private:
    Petrack      *mMainWindow;
    QRect         mPressRect;
    QPointF       mPressPos;
    PressLocation mPressLocation{PressLocation::inside};
    bool          mIsFixed{false};

public:
    RoiItem(QWidget *wParent, const QColor &color);
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void hoverMoveEvent(QGraphicsSceneHoverEvent *event) override;
    void restoreSize();
    void setFixed(bool fixed) { mIsFixed = fixed; }

signals:
    void changed();
};

#endif // ROIITEM_H
