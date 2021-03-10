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

#ifndef MARKERCASERN_H
#define MARKERCASERN_H

#include <QList>
#include <QColor>

#include "ellipse.h"


class TrackPoint;

class MarkerCasern
{
private:
    MyEllipse mHead;
    bool mHasHead;
    Vec2F mQuadrangle[4];
    bool mHasQuadrangle;
    QList<MyEllipse> mSpots;
    QList<int> mSpotCount;
    QColor mCol;
    int mCenterIndex; // index in mSpots list from cross marker
    int mColorIndex; // index in mSpots list from color marker
    int mOtherIndex; // index in mSpots list from round marker
public:
    MarkerCasern(MyEllipse head);
    MarkerCasern();
    ~MarkerCasern();
    
    inline const MyEllipse& head() const
    {
        return mHead;
    }
    inline const Vec2F* quadrangle() const
    {
        return mQuadrangle;
    }
    inline const QList<MyEllipse>& spots() const // reference to be faster
    {
        return mSpots;
    }
    inline const QColor& color() const
    {
        return mCol;
    }

    inline bool hasHead() const
    {
        return mHasHead;
    }
    inline bool hasQuadrangle() const
    {
        return mHasQuadrangle;
    }

    bool isOverlappingHead(const MyEllipse& e) const;
    bool isInsideHead(const Vec2F& p) const;

    // returns spot number in spots list when inside, otherwise returns -1
    int isOverlappingSpots(const MyEllipse& e) const;
    int isInsideSpots(const Vec2F& p) const;

    void modifyHead(const MyEllipse &head);

    void modifyQuadrangle(const Vec2F v[4]);

    void modifySpot(int i, const MyEllipse &spot);
    void addSpot(const MyEllipse &spot);
    void deleteSpot(int i);

    void organize(const cv::Mat &img, bool autoWB);

    MyEllipse getCenterSpot() const;
    MyEllipse getColorSpot() const;

    void draw(cv::Mat &img) const;
};

//----------------------------------------------------------------------------

class MarkerCasernList : public QList<MarkerCasern>
{
public:
    bool mayAddEllipse(const cv::Mat &img, const MyEllipse& e, bool blackInside);
    bool mayAddQuadrangle(const Vec2F v[4]);

    // organize every marker and delete marker without head
    void organize(const cv::Mat &img, bool autoWB);

    //draw ... Qt

    void draw(cv::Mat &img) const;

    void toCrossList(QList<TrackPoint> *crossList, bool ignoreWithoutMarker) const;
};

#endif
