/*
 * PeTrack - Software for tracking pedestrians movement in videos
 * Copyright (C) 2025 Forschungszentrum Jülich GmbH, IAS-7
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

#ifndef CODEMARKERITEM_H
#define CODEMARKERITEM_H

#include "vector.h"

#include <QGraphicsItem>

class Petrack;
class Control;
class Tracker;
namespace reco
{
class CodeMarkerOptions;
}


struct OffsetMarker
{
    std::vector<cv::Point2f> corners;
    Vec2F                    offset;

    OffsetMarker(std::vector<cv::Point2f> corn, Vec2F off) : corners(corn), offset(off) {};
};

class CodeMarkerItem : public QGraphicsItem
{
private:
    Petrack                       *mMainWindow;
    const reco::CodeMarkerOptions &mArucoOptions;
    const QColor                   mRejectedColor = QColor(255, 0, 0); // red
    const QColor                   mCornerColor   = QColor(0, 0, 255); // blue
    const QColor                   mAcceptedColor = QColor(0, 255, 0); // green

    std::vector<int>          mIds;
    std::vector<OffsetMarker> mCorners, mRejected;
    Vec2F                     mUlc; // upper left corner to draw

public:
    CodeMarkerItem(QWidget *wParent, const reco::CodeMarkerOptions &options, QGraphicsItem *parent = nullptr);
    QRectF boundingRect() const override;
    void   setRect(Vec2F &v);
    void   paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    void
    addDetectedMarkers(std::vector<std::vector<cv::Point2f>> corners, std::vector<int> ids, Vec2F offset = Vec2F(0, 0));
    void addRejectedMarkers(std::vector<std::vector<cv::Point2f>> rejected, Vec2F offset = Vec2F(0, 0));
    void resetSavedMarkers();

private:
    void drawMarker(const OffsetMarker &currentMarker, int id, const QColor &borderColor, QPainter *painter);
    static constexpr int numCorners = 4;
};

#endif
