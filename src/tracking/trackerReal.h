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

#ifndef TRACKERREAL_H
#define TRACKERREAL_H

#include "colorPlot.h"
#include "imageItem.h"
#include "tracker.h"
#include "vector.h"

#include <QList>
#include <utility>

class PersonStorage;
class WorldImageCorrespondence;

struct MissingFrame
{
    size_t mNumber; ///< frame number, where mCount of frames are missing
    int    mCount;  ///< count of frames missing between frame mNumber and mNumber+1
};

class MissingFrames : public QObject
{
    Q_OBJECT

private:
    bool                      mExecuted{false}; ///< already computed missing frames
    std::vector<MissingFrame> mMissingFrames{}; ///< vector of missing frames

public:
    MissingFrames(bool executed, std::vector<MissingFrame> &&missingFrames) :
        mExecuted(executed), mMissingFrames(std::move(missingFrames))
    {
    }

    bool                       isExecuted() const { return mExecuted; }
    std::vector<MissingFrame> &getMissingFrames() { return mMissingFrames; }

public slots: // NOLINT (Qt needs the public slots, so the keyword public repeats)
    void reset()
    {
        setExecuted(false);
        mMissingFrames.clear();
    }

    void setExecuted(bool executed)
    {
        mExecuted = executed;
        emit executeChanged(executed);
    }
    void setMissingFrames(std::vector<MissingFrame> missingFrames) { mMissingFrames = std::move(missingFrames); }

signals:
    void executeChanged(bool); // NOLINT (Qt signals don't need an implementation)
};

// point in x/y in cm
class TrackPointReal : public Vec3F
{
private:
    int   mFrameNum; // frame number in animation sequence possibly with missing frames
    Vec2F mViewDir;
    float mAngleOfView; // angle of view of camera to point
    int   mMarkerID;

public:
    TrackPointReal(const Vec3F &p, int frameNum);
    TrackPointReal(const Vec3F &p, int frameNum, const Vec2F &d);

    inline int   frameNum() const { return mFrameNum; }
    inline void  setFrameNum(int frameNum) { mFrameNum = frameNum; }
    inline Vec2F viewDir() const { return mViewDir; }
    inline float angleOfView() const { return mAngleOfView; }
    inline void  setAngleOfView(float a) { mAngleOfView = a; }
    inline int   getMarkerID() const { return mMarkerID; }
    inline void  setMarkerID(int markerID) { mMarkerID = markerID; }
    inline void  setViewDirection(const Vec2F &dir) { mViewDir = dir; }
};

inline QTextStream &operator<<(QTextStream &s, const TrackPointReal &tp)
{
    s << tp.x() << " " << tp.y(); // nur x, y wird ausgegeben, z wird nur bei Bedarf mit ausgegeben!!!
    return s;
}

//--------------------------------------------------------------------------

// the list index is the frame number plus mFirstFrame 0..mLastFrame-mFirstFrame
// no frame is left blank
class TrackPersonReal : public QList<TrackPointReal>
{
private:
    double mHeight;        // height of the person
    int    mFirstFrame;    // 0.. frame where the person was tracked the first time
    int    mLastFrame;     // 0..
    int    mMarkerID = -1; // set to -1 as -1 does not naturally occur as a ArucoMarkerNumber-value

public:
    TrackPersonReal();
    TrackPersonReal(int frame, const TrackPointReal &p);

    inline double         height() const { return mHeight; }
    inline void           setHeight(double h) { mHeight = h; }
    inline int            firstFrame() const { return mFirstFrame; }
    inline void           setFirstFrame(int f) { mFirstFrame = f; }
    inline int            lastFrame() const { return mLastFrame; }
    inline void           setLastFrame(int f) { mLastFrame = f; }
    inline int            getMarkerID() const { return mMarkerID; }
    inline void           setMarkerID(int markerID) { mMarkerID = markerID; }
    bool                  trackPointExist(int frame) const;
    const TrackPointReal &trackPointAt(int frame) const; // & macht bei else probleme, sonst mit [] zugreifbar
    // gibt -1 zurueck, wenn frame oder naechster frame nicht existiert
    // entfernung ist absolut
    double distanceToNextFrame(int frame) const;
    void   init(int firstFrame, double height, int markerID);
    void   addEnd(const QPointF &pos, int frame);
    void   addEnd(const Vec3F &pos, int frame);
    void   addEnd(const QPointF &pos, int frame, const QPointF &dir);
};

inline QTextStream &operator<<(QTextStream &s, const TrackPersonReal &tp)
{
    int firstFrame = tp.firstFrame();
    for(int i = 0; i < tp.size(); ++i)
    {
        s << firstFrame + i << tp.at(i) << tp.height() << Qt::endl;
    }
    return s;
}

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
    double         mXMin, mXMax, mYMin, mYMax;
    Petrack       *mMainWindow;
    PersonStorage &mPersonStorage;

public:
    inline double xMin() const { return mXMin; }
    inline double xMax() const { return mXMax; }
    inline double yMin() const { return mYMin; }
    inline double yMax() const { return mYMax; }

    TrackerReal(QWidget *wParent, PersonStorage &storage);


    // calculate height of person

    // convert all trajectorie coordinates in real coordinate (height needed)
    // vorstellbar, dass von..bis angegeben wird, was neu berechnet werden soll
    // Rueckgabe wieviel tracking pfade bestimmt wurden
    // petrack...getImageBorderSize()
    // mControlWidget->getColorPlot()
    // petrack...mImageItem
    int calculate(
        Petrack                        *petrack,
        Tracker                        *tracker,
        const WorldImageCorrespondence *worldImageCorr,
        ColorPlot                      *colorPlot,
        MissingFrames                  &missing,
        int                             imageBorderSize        = 0,
        bool                            missingFramesInserted  = true,
        bool                            useTrackpoints         = false,
        bool                            alternateHeight        = false,
        double                          altitude               = 0,
        bool                            useCalibrationCenter   = true,
        bool                            exportElimTp           = false,
        bool                            exportElimTrj          = false,
        bool                            exportSmooth           = true,
        bool                            exportViewingDirection = false,
        bool                            exportAngleOfView      = false,
        bool                            exportMarkerID         = false,
        bool                            exportAutoCorrect      = false);

    void calcMinMax();
    int  largestFirstFrame();
    int  largestLastFrame();
    int  smallestFirstFrame();
    int  smallestLastFrame();

    // alternateHeight true, wenn keine eindeutige personengroesse ausgegeben wird, sondern fuer jeden pounkt andere
    void exportTxt(
        QTextStream &out,
        bool         alternateHeight,
        bool         useTrackpoints,
        bool         exportViewingDirection,
        bool         exportAngleOfView,
        bool         exportUseM,
        bool         exportMarkerID);
    void                      exportDat(QTextStream &out, bool alternateHeight, bool useTrackpoints); // fuer gnuplot
    void                      exportXml(QTextStream &outXml, bool alternateHeight, bool useTrackpoints);
    std::vector<MissingFrame> computeDroppedFrames(Petrack *petrack);
};

namespace utils
{
std::vector<MissingFrame>
detectMissingFrames(const std::vector<std::unordered_map<int, double>> &displacementsPerFrame);

std::vector<std::unordered_map<int, double>> computeDisplacement(
    int                                          minFrameNum,
    int                                          maxFrameNum,
    Petrack                                     *petrack,
    const std::vector<std::vector<cv::Point2f>> &personsInFrame,
    const std::vector<std::vector<int>>         &idsInFrame);
} // namespace utils

#endif
