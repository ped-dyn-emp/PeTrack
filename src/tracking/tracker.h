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

#ifndef TRACKER_H
#define TRACKER_H

#include "annotationGrouping.h"
#include "intervalList.h"
#include "recognition.h"
#include "trackPoint.h"
#include "trackerConstants.h"
#include "trcparser.h"
#include "vector.h"

#include <QColor>
#include <QList>
#include <QRegularExpression>
#include <QSet>
#include <QTextStream>
#include <spdlog/fmt/bundled/format.h>

class PersonStorage;
class Petrack;


/**
 * @brief Stores all tracking information for a whole trajectory, as markerID, color, comment and also the
 * corresponding TrackPoints.
 *
 * A TrackPerson contains all TrackPoints from mFirstFrame to mLastFrame. The index in mData is the frame number plus
 * mFirstFrame.
 *
 * Important: It is always a continuous range of frames for each person! No gaps in the middle!
 */
class TrackPerson
{
private:
    int               mNr;            //< person number
    int               mMarkerID = -1; //< unique marker ID of person, -1 indicates no found/used marker
    double            mHeight;        //< height of the person
    int               mHeightCount;   //< number of colors where mHeight is averaged
    int               mFirstFrame;    //< frame where the person was tracked the first time
    bool              mNewReco;       //< true if person was just recognized
    QColor            mColor;         //< average color of person
    QString           mComment;       //< comment for person
    int               mNrInBg;        //< number of successive frames in the background
    int               mColorCount;    //< number of colors where mColor is average from
    QList<TrackPoint> mData{};        //< TrackPoints from mFirstFrame to mLastFrame;;
    IntervalList<int> mGroups{annotationGroups::NO_GROUP.id};

public:
    TrackPerson() = default;
    TrackPerson(int nr, int frame, const TrackPoint &p);

    TrackPerson(int nr, int frame, const TrackPoint &p, int markerID);

    bool insertAtFrame(int frame, const TrackPoint &p, int persNr, bool extrapolate);

    inline int  nrInBg() const { return mNrInBg; }
    inline void setNrInBg(int n) { mNrInBg = n; }

    inline double height() const { return mHeight; }
    inline void   setHeight(double h) { mHeight = h; }
    // beim setzen der Hoehe wird Mittelwert gebildet
    // z is z distance to camera
    // altitude is height of the camera over floor
    inline void setHeight(float z, float altitude) // , int frame
    {
        mHeight = (mHeight * mHeightCount + (altitude - z)) / (mHeightCount + 1);
        ++mHeightCount;
    }
    inline void resetHeight()
    {
        mHeight      = MIN_HEIGHT;
        mHeightCount = 0;
    }
    void   recalcHeight(float altitude);
    double getNearestZ(int i, int *extrapolated) const;

    inline int  getMarkerID() const { return mMarkerID; }
    inline void setMarkerID(const int markerID)
    {
        SPDLOG_INFO("Set person marker id to {}", markerID);
        mMarkerID = markerID;
    }
    inline const QColor &color() const { return mColor; }
    inline void          setColor(const QColor &col) { mColor = col; }
    inline bool          newReco() const { return mNewReco; }
    inline void          setNewReco(bool b) { mNewReco = b; }
    inline int           firstFrame() const { return mFirstFrame; }
    inline int           lastFrame() const { return mFirstFrame + mData.size() - 1; }
    inline int           nr() const { return mNr; }
    inline const QString comment() const { return mComment; }
    /**
     * @brief Get the comment without line breaks
     *
     * Get the comment as a one line QString where all linebreak are replaced with '<br>'
     * @return comment without line breaks
     */
    inline QString serializeComment() const { return QString{mComment}.replace(QRegularExpression("\n"), "<br>"); }

    inline void       setComment(QString s) { mComment = s; }
    inline int        colCount() const { return mColorCount; }
    inline void       setColCount(int c) { mColorCount = c; }
    void              addColor(const QColor &col);
    void              optimizeColor();
    bool              trackPointExist(int frame) const;
    const TrackPoint &trackPointAt(int frame) const;
    // gibt -1 zurueck, wenn frame oder naechster frame nicht existiert
    // entfernung ist absolut
    double distanceToNextFrame(int frame) const;
    void   syncTrackPersonMarkerID(int markerID);

    const TrackPoint &at(int i) const;

    int                               size() const;
    bool                              isEmpty() const;
    const TrackPoint                 &first() const;
    const TrackPoint                 &last() const;
    QList<TrackPoint>::const_iterator begin() const;
    QList<TrackPoint>::const_iterator end() const;
    QList<TrackPoint>::const_iterator cbegin() const;
    QList<TrackPoint>::const_iterator cend() const;

    void append(const TrackPoint &trackPoint);
    void clear();
    void replaceTrackPoint(int frame, TrackPoint trackPoint);
    void updateStereoPoint(int frame, Vec3F stereoPoint);
    void updateMarkerID(int frame, int markerID);

    void                            removeFramesBetween(int startFrame, int endFrame);
    inline IntervalList<int>       &getGroups() { return mGroups; }
    inline const IntervalList<int> &getGroups() const { return mGroups; }
};

// mHeightCount wird nicht e3xportiert und auch nicht wieder eingelesen -> nach import auf 0 obwohl auf height ein wert
// steht, daher immer mheight auf -1 testen!!!
// keine Konsistenzueberpruefung
QTextStream &operator<<(QTextStream &s, const TrackPerson &tp);

std::ostream &operator<<(std::ostream &s, const TrackPerson &tp);

ParseResult parseTrackPerson(
    const QStringList      &lines,
    int                    &currentLineIndex,
    TrackPerson            &trackPerson,
    reco::RecognitionMethod recoMethod);

//----------------------------------------------------------------------------

/**
 * @brief Class orchestrating tracking and related stuff
 *
 * using tracker:
 * 1. initial recognition
 * 2. next frame track existing track points
 * 3. new recognition and check if exist otherwise include new
 * (4. delete inner tracking point, which does not recognized over a longer time)
 * 5. backward tracking from firstFrame() on
 * 6. calculate color over tracking (accumulated over tracking while procedure above) path and set height
 * 7. recalc coord with real coord with known height
 */
class Tracker
{
private:
    enum class TrackStatus
    {
        Tracked,
        NotTracked,
        Merged
    };

    Petrack                 *mMainWindow;
    cv::Mat                  mGrey, mPrevGrey;
    std::vector<cv::Mat>     mPrevPyr, mCurrentPyr;
    std::vector<cv::Point2f> mPrevFeaturePoints, mFeaturePoints;
    std::vector<TrackStatus> mStatus;
    int                      mPrevFrame;
    std::vector<int>         mPrevFeaturePointsIdx;
    std::vector<float>       mTrackError;
    cv::TermCriteria         mTermCriteria;
    PersonStorage           &mPersonStorage;

public:
    Tracker(QWidget *wParent, PersonStorage &storage);

    // neben loeschen der liste muessen auch ...
    void init(cv::Size size);

    void reset();

    void resize(cv::Size size);

    size_t calcPrevFeaturePoints(
        int          prevFrame,
        cv::Rect    &rect,
        int          frame,
        bool         reTrack,
        int          reQual,
        int          borderSize,
        QSet<size_t> onlyVisible);

    int insertFeaturePoints(int frame, size_t count, cv::Mat &img, int borderSize, cv::Mat map1, float errorScale);

    // frame ist frame fuer naechsten prev frame
    int track(
        cv::Mat                &img,
        cv::Rect               &rect,
        cv::Mat                 map1,
        int                     frame,
        bool                    reTrack,
        int                     reQual,
        int                     borderSize,
        reco::RecognitionMethod recoMethod,
        int                     level              = 3,
        QSet<size_t>            onlyVisible        = QSet<size_t>(),
        int                     errorScaleExponent = 0);

    void checkPlausibility(
        QList<int> &pers,
        QList<int> &frame,
        bool        testEqual    = true,
        bool        testVelocity = true,
        bool        testInside   = true,
        bool        testLength   = true);

private:
    bool tryMergeTrajectories(const TrackPoint &v, size_t i, int frame);

    void trackFeaturePointsLK(int level);
    void trackFeaturePointsLK(int level, bool adaptive);
    void refineViaColorPointLK(int level, float errorScale);
    void useBackgroundFilter(QList<int> &trjToDel, BackgroundFilter *bgFilter);
    void refineViaNearDarkPoint();
    void preCalculateImagePyramids(int level);
};

#endif
