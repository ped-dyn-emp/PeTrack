#ifndef MARKERHERMES_H
#define MARKERHERMES_H

#include <QList>
#include <QColor>

#include "ellipse.h"

class TrackPoint;

class MarkerHermes
{
private:
    MyEllipse mHead;
    bool mHasHead;
    QList<MyEllipse> mSpots;
    QList<int> mSpotCount;
    int mCenterIndex; // index in mSpots list from black marker -1 or 0
public:
    MarkerHermes(MyEllipse head);
    MarkerHermes();
    ~MarkerHermes();
    
    inline const MyEllipse& head() const
    {
        return mHead;
    }
    inline const QList<MyEllipse>& spots() const // reference to be faster
    {
        return mSpots;
    }

    inline bool hasHead() const
    {
        return mHasHead;
    }

    bool isOverlappingHead(const MyEllipse& e) const;
    bool isInsideHead(const Vec2F& p) const;

    // returns spot number in spots list when inside, otherwise returns -1
    int isOverlappingSpots(const MyEllipse& e) const;
    int isInsideSpots(const Vec2F& p) const;

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
    bool mayAddEllipse(const cv::Mat &img, const MyEllipse& e, bool blackInside);

    // organize every marker and delete marker without head
    void organize(const cv::Mat &img, bool autoWB);

    void draw(cv::Mat &img) const;

    void toCrossList(QList<TrackPoint> *crossList, bool ignoreWithoutMarker) const;
};

#endif
