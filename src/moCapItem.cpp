/*
 * PeTrack - Software for tracking pedestrians movement in videos
 * Copyright (C) 2023 Forschungszentrum Jülich GmbH, IAS-7
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

#include "moCapItem.h"

#include "animation.h"
#include "moCapController.h"
#include "petrack.h"

#include <QPainter>
#include <QtMath> // for M_PI


/**
 * @brief Constructor of moCapItem
 * @throws std::bad_cast when wParent is no Petrack-type or Petrack-child
 * */
MoCapItem::MoCapItem(QWidget &wParent, Animation &animation, MoCapController &moCapController, QGraphicsItem *parent) :
    QGraphicsItem(parent),
    mMainWindow(dynamic_cast<Petrack &>(wParent)),
    mAnimation(animation),
    mController(moCapController)
{
}

QRectF MoCapItem::boundingRect() const
{
    if(mMainWindow.getImage())
    {
        return QRectF(
            -mMainWindow.getImageBorderSize(),
            -mMainWindow.getImageBorderSize(),
            mMainWindow.getImage()->width(),
            mMainWindow.getImage()->height());
    }
    return QRectF(0, 0, 0, 0);
}

void MoCapItem::paint(QPainter *painter, const QStyleOptionGraphicsItem * /* option */, QWidget * /* widget */)
{
    if(mController.getShowMoCap())
    {
        std::vector<SegmentRenderData> allRenderData =
            mController.getRenderData(mAnimation.getCurrentFrameNum(), mAnimation.getOriginalFPS());
        for(SegmentRenderData &renderData : allRenderData)
        {
            drawLine(painter, renderData);
            if(renderData.mDirected)
            {
                drawArrowHead(painter, renderData);
            }
        }
    }
}
/**
 * @brief Sets the painter's given specifications and draws the line
 *
 * @param painter
 * @param renderData
 */
void MoCapItem::drawLine(QPainter *painter, SegmentRenderData &renderData)
{
    QPen pen{renderData.mColor};
    pen.setWidth(renderData.mThickness);
    painter->setPen(pen);
    painter->drawLine(renderData.mLine);
}

/**
 * @brief Draws the arrow at point p2 with the current painter's specifications
 * The arrowhead is a triangle. Point p2 is the top, the triangle sides' length
 * is 1/5 of the arrow length.
 * @param painter
 * @param renderData
 */
void MoCapItem::drawArrowHead(QPainter *painter, SegmentRenderData &renderData)
{
    QPolygonF arrowHead;
    auto      lineItem = QGraphicsLineItem(renderData.mLine);
    double angle = std::atan2(-lineItem.line().dy(), lineItem.line().dx()); // angle between x-axis and line in radians
    qreal  arrowLength              = QLineF(lineItem.line()).length() / 5;
    constexpr double arrowHeadAngle = M_PI / 3;
    // sin(angle + arrowHeadAngle) == x-coordinate of point (e.g. unit circle)
    // cos(angle + arrowHeadAngle) == y-coordinate of point (e.g. unit circle)
    QPointF arrowHeadRight =
        lineItem.line().p2() -
        QPointF(sin(angle + arrowHeadAngle) * arrowLength, cos(angle + arrowHeadAngle) * arrowLength);
    // turned first position around another arrowHeadAngle
    QPointF arrowHeadLeft =
        lineItem.line().p2() -
        QPointF(sin(angle + 2 * arrowHeadAngle) * arrowLength, cos(angle + 2 * arrowHeadAngle) * arrowLength);
    arrowHead << lineItem.line().p2() << arrowHeadRight << arrowHeadLeft;
    painter->drawPolygon(arrowHead);
}
