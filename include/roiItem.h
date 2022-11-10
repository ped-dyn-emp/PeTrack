/*
 * PeTrack - Software for tracking pedestrians movement in videos
 * Copyright (C) 2022 Forschungszentrum Jülich GmbH, IAS-7
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

#include "imageItem.h"
#include "petrack.h"

#include <QGraphicsRectItem>
#include <QObject>

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
    void setToFullImageSize();

    /**
     * @brief Sets the size and position of the rect relative to other.
     *
     * @tparam BinaryFunction
     * @param other other ROI which is used as source for new position and size
     * @param defaultHeight default height to determine the cm per pixel
     * @param f binary function: in this case should be plus or minus
     */
    template <typename BinaryFunction>
    void adjustToOtherROI(const RoiItem &other, BinaryFunction f)
    {
        // Compute head sizes at the four corners to get the offset
        auto otherTopLeft    = other.rect().topLeft();
        auto headSizeTopLeft = mMainWindow->getHeadSize(&otherTopLeft);

        auto otherBottomLeft    = other.rect().bottomLeft();
        auto headSizeBottomLeft = mMainWindow->getHeadSize(&otherBottomLeft);

        auto otherTopRight    = other.rect().topRight();
        auto headSizeTopRight = mMainWindow->getHeadSize(&otherTopRight);

        auto otherBottomRight    = other.rect().bottomRight();
        auto headSizeBottomRight = mMainWindow->getHeadSize(&otherBottomRight);

        constexpr auto headFactor = 2.;
        auto           offset =
            headFactor * std::max({headSizeTopLeft, headSizeBottomLeft, headSizeTopRight, headSizeBottomRight});

        auto borderSize = static_cast<double>(mMainWindow->getImageBorderSize());
        auto topLeftX   = std::clamp(
            f(otherTopLeft.x(), -offset), -borderSize, static_cast<double>(mMainWindow->getImage()->width()));
        auto topLeftY = std::clamp(
            f(otherTopLeft.y(), -offset), -borderSize, static_cast<double>(mMainWindow->getImage()->height()));
        auto width = std::clamp(
            f(other.rect().width(), 2 * offset), 0., mMainWindow->getImage()->width() - topLeftX - borderSize);
        auto height = std::clamp(
            f(other.rect().height(), 2 * offset), 0., mMainWindow->getImage()->height() - topLeftY - borderSize);
        setRect(topLeftX, topLeftY, width, height);
        emit changed();
    }

signals:
    void changed();
};

#endif // ROIITEM_H
