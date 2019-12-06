#ifndef MarkerJapan_H
#define MarkerJapan_H

#include <QList>
#include <QColor>

#include "ellipse.h"

class TrackPoint;

class MarkerJapan
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
    MarkerJapan(MyEllipse head);
    MarkerJapan();
    ~MarkerJapan();
    
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

    inline bool hasSpots() const
    {
        return mSpots.size() > 1; // mindestens 2 spots werden benoetigt, damit ein gueltiger Marker!
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

    void modifyHead(const MyEllipse &head, float headSize);

    void modifyQuadrangle(const Vec2F v[4]);

    void modifySpot(int i, const MyEllipse &spot);
    void addSpot(const MyEllipse &spot);
    void deleteSpot(int i);

    void organize(const cv::Mat &img, bool autoWB);

    Vec2F getCenter() const;
    MyEllipse getCenterSpot() const;
    MyEllipse getColorSpot() const;

    void draw(cv::Mat &img) const;
};

//----------------------------------------------------------------------------

class MarkerJapanList : public QList<MarkerJapan>
{
private:
    float mHeadSize;
public:
    MarkerJapanList(float headSize)
        : mHeadSize(headSize)
    {
    }
    bool mayAddEllipse(const cv::Mat &img, const MyEllipse& e, bool blackInside);
    bool mayAddQuadrangle(const Vec2F v[4]);

    // organize every marker and delete marker without head
    void organize(const cv::Mat &img, bool autoWB);

    //draw ... Qt

    void draw(cv::Mat &img) const;

    void toCrossList(QList<TrackPoint> *crossList, bool ignoreWithoutMarker) const;
};

#endif
