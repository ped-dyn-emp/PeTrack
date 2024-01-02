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
#ifndef MOCAPITEM_H
#define MOCAPITEM_H

#include <QGraphicsItem>

class MoCapController;
class Animation;
class Petrack;
struct SegmentRenderData;

/**
 * @brief The MoCapItem class draws stick figures to visalize the MoCap data.
 *
 * This class draws stick figures based on the motion capturing data loaded
 * into PeTrack. The actual building of the stick figure is done in
 * MoCapController::getRenderData(), this class just gets the data and draws it.
 * The orientation of the head is visualized as well.
 */
class MoCapItem : public QGraphicsItem
{
public:
    MoCapItem(
        QWidget         &wParent,
        Animation       &animation,
        MoCapController &moCapController,
        QGraphicsItem   *parent = nullptr);
    QRectF boundingRect() const override;
    void   paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

private:
    Petrack         &mMainWindow;
    Animation       &mAnimation;
    MoCapController &mController;
    static void      drawLine(QPainter *painter, SegmentRenderData &renderData);
    static void      drawArrowHead(QPainter *painter, SegmentRenderData &renderData);
};

#endif // MOCAPITEM_H
