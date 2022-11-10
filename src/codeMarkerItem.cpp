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

#include "codeMarkerItem.h"

#include "codeMarkerWidget.h"
#include "control.h"
#include "petrack.h"
#include "tracker.h"
#include "view.h"

#include <QtWidgets>

// in x und y gleichermassen skaliertes koordinatensystem,
// da von einer vorherigen intrinsischen kamerakalibrierung ausgegenagen wird,
// so dass pixel quadratisch
CodeMarkerItem::CodeMarkerItem(QWidget *wParent, const reco::CodeMarkerOptions &options, QGraphicsItem *parent) :
    QGraphicsItem(parent), mArucoOptions(options)
{
    mMainWindow = (class Petrack *) wParent;
}


/**
 * @brief Bounding box of drawn to area.
 *
 * This bounding box is used to determine if this Item needs to be redrawn or not.
 * See the official Qt Docs for QGraphicsItem
 *
 * @return (updated) bounding rect of this item
 */
QRectF CodeMarkerItem::boundingRect() const
{
    if(mMainWindow->getImage())
    {
        return QRectF(
            -mMainWindow->getImageBorderSize(),
            -mMainWindow->getImageBorderSize(),
            mMainWindow->getImage()->width(),
            mMainWindow->getImage()->height());
    }
    else
    {
        return QRectF(0, 0, 0, 0);
    }
}

void CodeMarkerItem::setRect(Vec2F &v)
{
    mUlc = v; // upper left corner to draw
}

/**
 * @brief draws colored shapes at heads in image to indicate detection status
 *
 * different calculations of position depending on whether function is called out of findCodeMarker() Function
 * only (== recoMethod 6) or if findCodeMarker() Function is called out of findMulticolorMarker() Function (==
 * recoMethod 5). In the first case the offset is added automatically via 'offset' and 'v'. In the second case the
 * offset from cropRect to ROI has to be added manually.
 *
 * @param painter
 * @param option
 * @param widget
 */
void CodeMarkerItem::paint(QPainter *painter, const QStyleOptionGraphicsItem * /*option*/, QWidget * /*widget*/)
{
    if(mMainWindow->getCodeMarkerWidget()->showDetectedCandidates())
    {
        int nMarkers  = static_cast<int>(mCorners.size());
        int nRejected = static_cast<int>(mRejected.size());

        cv::Point2f p0, p1;
        for(int i = 0; i < nMarkers; i++) // draws green square/ circle around head if person recognized
        {
            drawMarker(mCorners.at(i), mIds.at(i), mAcceptedColor, painter);
        }
        for(int i = 0; i < nRejected; i++)
        {
            drawMarker(mRejected.at(i), -1, mRejectedColor, painter);
        }
    }

    if(false) // Show min/max marker size
    {
        int    minPerimeter = mArucoOptions.getDetectorParams().getMinMarkerPerimeter();
        int    maxPerimeter = mArucoOptions.getDetectorParams().getMaxMarkerPerimeter();
        double height       = mMainWindow->getStatusPosRealHeight();

        cv::Point2f p0, p1, p2, p3;

        p0 = mMainWindow->getExtrCalibration()->getImagePoint(cv::Point3f(0, 0, height));
        p1 = mMainWindow->getExtrCalibration()->getImagePoint(cv::Point3f(maxPerimeter, 0, height));
        p2 = mMainWindow->getExtrCalibration()->getImagePoint(cv::Point3f(maxPerimeter, maxPerimeter, height));
        p3 = mMainWindow->getExtrCalibration()->getImagePoint(cv::Point3f(0, maxPerimeter, height));

        painter->setPen(qRgb(0, 0, 0));
        painter->drawLine(p0.x, +p0.y, p1.x, p1.y);
        painter->drawLine(p1.x, +p1.y, p2.x, p2.y);
        painter->drawLine(p2.x, +p2.y, p3.x, p3.y);
        painter->drawLine(p3.x, p3.y, p0.x, p0.y);


        p0 = mMainWindow->getExtrCalibration()->getImagePoint(cv::Point3f(0, 0, height));
        p1 = mMainWindow->getExtrCalibration()->getImagePoint(cv::Point3f(minPerimeter, 0, height));
        p2 = mMainWindow->getExtrCalibration()->getImagePoint(cv::Point3f(minPerimeter, minPerimeter, height));
        p3 = mMainWindow->getExtrCalibration()->getImagePoint(cv::Point3f(0, minPerimeter, height));

        painter->setPen(qRgb(255, 255, 255));
        painter->drawLine(p0.x, p0.y, p1.x, p1.y);
        painter->drawLine(p1.x, p1.y, p2.x, p2.y);
        painter->drawLine(p2.x, p2.y, p3.x, p3.y);
        painter->drawLine(p3.x, p3.y, p0.x, p0.y);
    }
}

/**
 * @brief Draws a aruco code marker
 *
 * Draws a rect with the corners of the marker, the id, and the upper left corner.
 *
 * @param currentMarker marker to draw
 * @param id id of the marker to draw
 * @param borderColor color of the rect drawn
 * @param painter painter with which to draw
 */
void CodeMarkerItem::drawMarker(const OffsetMarker &currentMarker, int id, const QColor &borderColor, QPainter *painter)
{
    Vec2F offset = currentMarker.offset;
    // draw marker sides
    for(int j = 0; j < numCorners; j++)
    {
        Vec2F p0 = currentMarker.corners.at(j);
        Vec2F p1 = currentMarker.corners.at((j + 1) % numCorners);
        painter->setPen(borderColor);
        painter->drawLine((mUlc + p0 + offset).toQPointF(), (mUlc + p1 + offset).toQPointF());
    }
    Vec2F topLeftCorner{currentMarker.corners.at(0)};
    painter->setPen(mCornerColor);
    if(id != -1)
    {
        painter->drawText((mUlc + topLeftCorner + offset + Vec2F(10, 10)).toQPointF(), QString::number(id));
    }
    painter->drawRect(QRectF((mUlc + topLeftCorner + offset - Vec2F(3, 3)).toQPointF(), QSize(6.0, 6.0)));
}

/**
 * @brief Adds given markers as detected markers to visualization
 *
 * @pre ids[i] is the id of corners[i]
 *
 * @param corners corners of the markers
 * @param ids ids of the detected markers
 * @param offset offset from marker-coords to recognition ROI
 */
void CodeMarkerItem::addDetectedMarkers(
    std::vector<std::vector<cv::Point2f>> corners,
    std::vector<int>                      ids,
    Vec2F                                 offset /* = (0,0)*/)
{
    for(std::vector<cv::Point2f> singleMarkerCorners : corners)
    {
        mCorners.emplace_back(singleMarkerCorners, offset);
    }
    mIds.insert(mIds.end(), ids.begin(), ids.end());
}

/**
 * @brief Adds given markers as rejected markers to visualization
 *
 * @param rejected corners of the rejected markers
 * @param offset offset from marker-coords to recognition ROI
 */
void CodeMarkerItem::addRejectedMarkers(std::vector<std::vector<cv::Point2f>> rejected, Vec2F offset /* = (0,0)*/)
{
    for(std::vector<cv::Point2f> singleMarkerCorners : rejected)
    {
        mRejected.emplace_back(singleMarkerCorners, offset);
    }
}

/**
 * @brief Resets detected and rejected markers for visualization
 */
void CodeMarkerItem::resetSavedMarkers()
{
    mCorners.clear();
    mIds.clear();
    mRejected.clear();
}
