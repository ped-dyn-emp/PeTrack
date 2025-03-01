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

#ifndef MARKERHERMES_H
#define MARKERHERMES_H

#include "ellipse.h"

#include <QColor>
#include <QList>

class TrackPoint;

class MarkerHermes
{
private:
    MyEllipse        mHead;
    bool             mHasHead;
    QList<MyEllipse> mSpots;
    QList<int>       mSpotCount;
    int              mCenterIndex; // index in mSpots list from black marker -1 or 0
public:
    MarkerHermes(MyEllipse head);
    MarkerHermes();
    ~MarkerHermes();

    inline const MyEllipse        &head() const { return mHead; }
    inline const QList<MyEllipse> &spots() const // reference to be faster
    {
        return mSpots;
    }

    inline bool hasHead() const { return mHasHead; }

    bool isOverlappingHead(const MyEllipse &e) const;
    bool isInsideHead(const Vec2F &p) const;

    // returns spot number in spots list when inside, otherwise returns -1
    int isOverlappingSpots(const MyEllipse &e) const;
    int isInsideSpots(const Vec2F &p) const;

    void modifyHead(const MyEllipse &head);

    void modifySpot(int i, const MyEllipse &spot);
    void addSpot(const MyEllipse &spot);
    void deleteSpot(int i);

    void organize(const cv::Mat &img, bool autoWB);

    MyEllipse getCenterSpot() const;

    void draw(cv::Mat &img) const;
};

//----------------------------------------------------------------------------

class MarkerHermesList : public QList<MarkerHermes>
{
public:
    bool mayAddEllipse(const cv::Mat &img, const MyEllipse &e, bool blackInside);

    // organize every marker and delete marker without head
    void organize(const cv::Mat &img, bool autoWB);

    void draw(cv::Mat &img) const;

    void toCrossList(QList<TrackPoint> *crossList, bool ignoreWithoutMarker) const;
};

#endif
