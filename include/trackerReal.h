#ifndef TRACKERREAL_H
#define TRACKERREAL_H

#include <QList>
//#include <QTextStream>

#include "vector.h"
#include "tracker.h"
#include "imageItem.h"
#include "colorPlot.h"

// point in x/y in cm
class TrackPointReal : public Vec3F
{
private:
    int mFrameNum; // frame number in animation sequence possibly with missing frames
    // TrackPoint* mTrackPoint // NULL, wenn nicht einem TrackPoint zugeordnet, sondern durch Interpolation entstanden
    Vec2F mViewDir;
    float mAngleOfView; // angle of view of camera to point
    int mMarkerID;
public:
//     TrackPointReal();
    TrackPointReal(const Vec3F &p, int frameNum);
    TrackPointReal(const Vec3F &p, int frameNum, const Vec2F &d);

    inline int frameNum() const
    {
        return mFrameNum;
    }
    inline void setFrameNum(int frameNum)
    {
        mFrameNum = frameNum;
    }
    inline Vec2F viewDir() const
    {
        return mViewDir;
    }
    inline float angleOfView() const
    {
        return mAngleOfView;
    }
    inline void setAngleOfView(float a)
    {
        mAngleOfView = a;
    }
    inline int getMarkerID() const
    {
        return mMarkerID;
    }
    inline void setMarkerID(int markerID)
    {
        mMarkerID = markerID;
    }

//     const TrackPointReal& operator=(const TrackPointReal& tp); // Zuweisungsoperator
//     const TrackPointReal& operator=(const Vec2F& v);
//     const TrackPointReal& operator+=(const Vec2F& v);
//     const TrackPointReal operator+(const Vec2F& v) const;
};

// inline ostream& operator<< (ostream& s, const TrackPointReal& tp)
// {
//     s << "(" << tp.x() << ", " << tp.y() << ") " << tp.qual() << " - (" << tp.colPoint().x() << ", " << tp.colPoint().y() << ") (" << tp.color().red() << ", " << tp.color().green() << ", " << tp.color().blue() << ")";
//     return s;
// }

//inline QTextStream& operator>>(QTextStream& s, TrackPointReal& tp)
//{
//     double d;
//     int i;

//     s >> i;
//     tp.setFrameNum(i);
//     s >> d;
//     tp.setX(d);
//     s >> d;
//     tp.setY(d);
//     s >> d;
//     tp.setZ(d);

//     return s;
//}
inline QTextStream& operator<<(QTextStream& s, const TrackPointReal& tp)
{
    s << tp.x() << " " << tp.y(); // nur x, y wird ausgegeben, z wird nur bei Bedarf mit ausgegeben!!!
    return s;
}
// inline ostream& operator<<(ostream& s, const TrackPointReal& tp)
// {
//     s << tp.x() << " " << tp.y() << " " << tp.qual() << " " << tp.colPoint().x() << " " << tp.colPoint().y() << " " << tp.color();
//     return s;
// }

//--------------------------------------------------------------------------

// the list index is the frame number plus mFirstFrame 0..mLastFrame-mFirstFrame
// no frame is left blank
class TrackPersonReal : public QList<TrackPointReal>
{
private:
    double mHeight; // height of the person
    int mFirstFrame; // 0.. frame where the person was tracked the first time
    int mLastFrame; // 0..
    int mMarkerID = -1; //set to -1 as -1 does not naturally occur as a ArucoMarkerNumber-value

public:
    TrackPersonReal();
//     TrackPersonReal(int nr, int frame, const Vec2F &p);
    TrackPersonReal(int frame, const TrackPointReal &p);

    inline double height() const
    {
        return mHeight;
    }
    inline void setHeight(double h)
    {
        mHeight = h;
    }
    inline int firstFrame() const
    {
        return mFirstFrame;
    }
    inline void setFirstFrame(int f)
    {
        mFirstFrame = f;
    }
    inline int lastFrame() const
    {
        return mLastFrame;
    }
    inline void setLastFrame(int f)
    {
        mLastFrame = f;
    }
    inline int getMarkerID() const
    {
        return mMarkerID;
    }
    inline void setMarkerID(int markerID)
    {
        mMarkerID = markerID;
    }
    bool trackPointExist(int frame) const;
    const TrackPointReal& trackPointAt(int frame) const; // & macht bei else probleme, sonst mit [] zugreifbar
    // gibt -1 zurueck, wenn frame oder naechster frame nicht existiert
    // entfernung ist absolut
    double distanceToNextFrame(int frame) const;
//     double distanceBetweenFrames(int fromFrame, int toFrame) const;
//     double distanceXBetweenFrames(int fromFrame, int toFrame) const;
//     double distanceYBetweenFrames(int fromFrame, int toFrame) const;
//     double velocityToNextFrame(int frame) const;
    void init(int firstFrame, double height, int markerID);
    void addEnd(const QPointF& pos, int frame);
    void addEnd(const Vec3F& pos, int frame);
    void addEnd(const QPointF& pos, int frame, const QPointF &dir);
};

// // keine Konsistenzueberpruefung
// inline QTextStream& operator>>(QTextStream& s, TrackPersonReal& tp)
// {
//     double d;
//     QColor col;
//     int n;
//     TrackPointReal p;

//     s >> n; 
//     tp.setNr(n);
//     s >> d; 
//     tp.setHeight(d);
//     s >> n; 
//     tp.setFirstFrame(n);
//     s >> n; 
//     tp.setLastFrame(n);
//     s >> n; 
//     tp.setColCount(n);
//     s >> col; 
//     tp.setColor(col);
//     s >> n; // size of list

//     for (int i = 0; i < n; ++i)
//     {
//         s >> p;
//         tp.append(p);
//     }
//     return s;
// }

inline QTextStream& operator<<(QTextStream& s, const TrackPersonReal& tp)
{
    int firstFrame = tp.firstFrame();
    for (int i = 0; i < tp.size(); ++i)
        s << firstFrame+i << tp.at(i) << tp.height() << Qt::endl;
    return s;
}


// inline ostream& operator<<(ostream& s, const TrackPersonReal& tp)
// {
//     s << tp.nr() << " " << tp.height() << " " << tp.firstFrame() << " " << tp.lastFrame() << " " << tp.colCount() << " " << tp.color() << " " << tp.size() << endl;
//     for (int i = 0; i < tp.size(); ++i)
//         s << tp.at(i) << endl;
//     return s;
// }

//----------------------------------------------------------------------------

// using tracker:
// 1. initial recognition
// 2. next frame track existing track points
// 3. new recognition and check if exist otherwise include new
// (4. delete inner tracking point, which does not recognized over a longer time)
// 5. backward tracking from firstFrame() on
// 6. calculate color over tracking (accumulated over tracking while procedure above) path and set height
// 7. recalc coord with real coord with known height

class TrackerReal : public QList<TrackPersonReal>
{
private:
    double mXMin, mXMax, mYMin, mYMax;
    Petrack *mMainWindow;
    // int double mFps;
//     Tracker* mTracker;
//     bool mMissingFramesInserted;

public:
    inline double xMin() const
    {
        return mXMin;
    }
    inline double xMax() const
    {
        return mXMax;
    }
    inline double yMin() const
    {
        return mYMin;
    }
    inline double yMax() const
    {
        return mYMax;
    }

    TrackerReal(QWidget *wParent);

//     ~TrackerReal();

//     inline const Tracker* tracker() const
//     {
//         return mTracker;
//     }
//     inline void setTracker(Tracker* tracker)
//     {
//         mTracker = tracker;
//     }
//     inline const bool missingFramesInserted() const
//     {
//         return mMissingFramesInserted;
//     }
//     inline void setMissingFramesInserted(bool b)
//     {
//         mMissingFramesInserted = b;
//     }

//     void init(CvSize size);
//     void resize(CvSize size);

    // calculate height of person

    // convert all trajectorie coordinates in real coordinate (height needed)
    // vorstellbar, dass von..bis angegeben wird, was neu berechnet werden soll
    // Rueckgabe wieviel tracking pfade bestimmt wurden
    // petrack...getImageBorderSize()
    // mControlWidget->getColorPlot()
    // petrack...mImageItem
    int calculate(Tracker *tracker, ImageItem *imageItem, ColorPlot *colorPlot, int imageBorderSize = 0, bool missingFramesInserted = true, bool useTrackpoints = false,
                  bool alternateHeight = false, double altitude = 0, bool useCalibrationCenter = true,
                  bool exportElimTp = false, bool exportElimTrj = false, bool exportSmooth = true,
                  bool exportViewingDirection = false, bool exportAngleOfView = false, bool exportMarkerID = false, bool exportAutoCorrect = false);

    void calcMinMax();
    int largestFirstFrame();
    int largestLastFrame();
    int smallestFirstFrame();
    int smallestLastFrame();

    // alternateHeight true, wenn keine eindeutige personengroesse ausgegeben wird, sondern fuer jeden pounkt andere
    void exportTxt(QTextStream &out,    bool alternateHeight, bool useTrackpoints, bool exportViewingDirection, bool exportAngleOfView, bool exportUseM, bool exportMarkerID);
    void exportDat(QTextStream &out,    bool alternateHeight, bool useTrackpoints); // fuer gnuplot
    void exportXml(QTextStream &outXml, bool alternateHeight, bool useTrackpoints);
};

//inline QTextStream& operator<<(QTextStream& s, const TrackerReal& trackerReal)
//{
//    for (int i = 0; i < trackerReal.size(); ++i)
//        for (int j = 0; j < trackerReal.at(i).size(); ++j)
//            s << i+1 << " " << trackerReal.at(i).firstFrame()+j << " " << trackerReal.at(i).at(j) << " " << trackerReal.at(i).height() <<endl; //<< " " << tp.qual();
//    return s;
//}

#endif
