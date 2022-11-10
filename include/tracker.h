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

#ifndef TRACKER_H
#define TRACKER_H

#include "recognition.h"
#include "vector.h"

#include <QColor>
#include <QList>
#include <QRegularExpression>
#include <QTextStream>

class PersonStorage;
class Petrack;

// war 1.5, aber bei bildauslassungen kann es ungewollt zuschlagen (bei 3 ist ein ausgelassener frame mgl, bei 2 wieder
// ein problem)
inline constexpr double EXTRAPOLATE_FACTOR = 3.;

// maximale anzahl an gleichzeitig getrackten personen
inline constexpr int MAX_COUNT = 1500;

// maximale zahl an frames zwischen denen noch getrackt wird
inline constexpr int MAX_STEP_TRACK = 5;

// maximaler fehler beim tracken, so das noch punkt hinzugefuegt wird
// um am ende des tracking nicht am bildrand herumzukrakseln!
inline constexpr float MAX_TRACK_ERROR = 200.F;

// minimale Personengroesse zur Identifizierung, dass Hoehe nicht gesetz, war -1, was aber an Treppen schlecht war
inline constexpr double MIN_HEIGHT = -100'000.;


class TrackPoint : public Vec2F // Vec2F is pixel point in picture
{
private:
    Vec2F  mColPoint; // center of color marker
    QColor mCol;      // color of corresponding marker
    int    mQual;     // quality 0 (worst) .. 100 (best)
    int    mMarkerID; // ID of detected Marker
    Vec3F mSp; // measured 3d point with stereo // mZdistanceToCam; // distance in z direction to camera - measured with
               // stereo
    cv::Vec3d
        mOrientation; // orientation of marker i. e. head direction if the marker is placed facing that way (aruco)

public:
    TrackPoint();
    TrackPoint(const Vec2F &p);
    TrackPoint(const Vec2F &p, int qual);
    TrackPoint(const Vec2F &p, int qual, int markerID);
    TrackPoint(const Vec2F &p, int qual, const QColor &col);
    TrackPoint(const Vec2F &p, int qual, const Vec2F &colPoint, const QColor &col);

    inline const Vec2F     &colPoint() const { return mColPoint; }
    inline void             setColPoint(Vec2F &cp) { mColPoint = cp; }
    inline const QColor    &color() const { return mCol; }
    inline void             setColor(QColor &col) { mCol = col; }
    inline void             setColPoint(const Vec2F &colPoint) { mColPoint = colPoint; }
    inline void             setCol(const QColor &col) { mCol = col; }
    inline int              qual() const { return mQual; }
    inline void             setQual(int qual) { mQual = qual; }
    inline int              getMarkerID() const { return mMarkerID; }
    inline void             setMarkerID(int markerID) { mMarkerID = markerID; }
    inline const cv::Vec3d &getOrientation() const { return mOrientation; }
    inline void             setOrientation(const cv::Vec3d &orientation) { mOrientation = orientation; }

    inline const Vec3F &sp() const { return mSp; }
    inline void         setSp(const Vec3F &sp) { mSp = sp; }
    inline void         setSp(float x, float y, float z)
    {
        mSp.setX(x);
        mSp.setY(y);
        mSp.setZ(z);
    }

    const TrackPoint &operator=(const Vec2F &v);
    const TrackPoint &operator+=(const Vec2F &v);
    const TrackPoint  operator+(const Vec2F &v) const;

    static constexpr int minDetectionQual  = 80;
    static constexpr int bestDetectionQual = 100;
    [[nodiscard]] bool   isDetection() const;
};


QTextStream  &operator>>(QTextStream &s, TrackPoint &tp);
QTextStream  &operator<<(QTextStream &s, const TrackPoint &tp);
std::ostream &operator<<(std::ostream &s, const TrackPoint &tp);

//--------------------------------------------------------------------------

// the list index is the frame number plus mFirstFrame 0..mLastFrame-mFirstFrame
// no frame is left blank
class TrackPerson : public QList<TrackPoint>
{
private:
    int mNr;             // person number
    int mMarkerID = -1;  // markerID of Trackperson; -1 as positive values including 0 do exist as
                         // ArucoCodeNumbers-Values
    double mHeight;      // height of the person
    int    mHeightCount; // number of colors where mHeight is averaged
    int    mFirstFrame;  // 0.. frame where the person was tracked the first time
    int    mLastFrame;   // 0..
    // int qual; // quality
    bool    mNewReco;  // true if person was just recognized
    QColor  mCol;      // color of point
    QString mComment;  // comment for person
    int     mNrInBg;   // number of successive frames in the background
    int     mColCount; // number of colors where mCol is average from

public:
    TrackPerson();
    TrackPerson(int nr, int frame, const TrackPoint &p);

    TrackPerson(int nr, int frame, const TrackPoint &p, int markerID);

    bool insertAtFrame(int frame, const TrackPoint &p, int persNr, bool extrapolate);

    inline int  nrInBg() const { return mNrInBg; }
    inline void setNrInBg(int n) { mNrInBg = n; }

    inline double height() const { return mHeight; }
    // echte Personengroesse beim einlesen der trj datei
    inline void setHeight(double h) { mHeight = h; }
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

    inline int           getMarkerID() const { return mMarkerID; }
    inline void          setMarkerID(const int markerID) { mMarkerID = markerID; }
    inline const QColor &color() const { return mCol; }
    inline void          setColor(const QColor &col) { mCol = col; }
    inline bool          newReco() const { return mNewReco; }
    inline void          setNewReco(bool b) { mNewReco = b; }
    inline int           firstFrame() const { return mFirstFrame; }
    inline void          setFirstFrame(int f) { mFirstFrame = f; }
    inline int           lastFrame() const { return mLastFrame; }
    inline void          setLastFrame(int f) { mLastFrame = f; }
    inline int           nr() const { return mNr; }
    inline void          setNr(int nr) { mNr = nr; }
    inline const QString comment() const { return mComment; }
    /**
     * @brief Get the comment without line breaks
     *
     * Get the comment as a one line QString where all linebreak are replaced with '<br>'
     * @return comment without line breaks
     */
    inline QString serializeComment() const { return QString{mComment}.replace(QRegularExpression("\n"), "<br>"); }

    inline void       setComment(QString s) { mComment = s; }
    inline int        colCount() const { return mColCount; }
    inline void       setColCount(int c) { mColCount = c; }
    void              addColor(const QColor &col);
    void              optimizeColor();
    bool              trackPointExist(int frame) const;
    const TrackPoint &trackPointAt(int frame) const; // & macht bei else probleme, sonst mit [] zugreifbar
    // gibt -1 zurueck, wenn frame oder naechster frame nicht existiert
    // entfernung ist absolut
    double distanceToNextFrame(int frame) const;
    void   syncTrackPersonMarkerID(int markerID);
};

// mHeightCount wird nicht e3xportiert und auch nicht wieder eingelesen -> nach import auf 0 obwohl auf height ein wert
// steht, daher immer mheight auf -1 testen!!!
// keine Konsistenzueberpruefung
QTextStream &operator>>(QTextStream &s, TrackPerson &tp);

QTextStream &operator<<(QTextStream &s, const TrackPerson &tp);

std::ostream &operator<<(std::ostream &s, const TrackPerson &tp);

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

    int insertFeaturePoints(int frame, size_t count, cv::Mat &img, int borderSize, float errorScale);

    // frame ist frame fuer naechsten prev frame
    int track(
        cv::Mat                &img,
        cv::Rect               &rect,
        int                     frame,
        bool                    reTrack,
        int                     reQual,
        int                     borderSize,
        reco::RecognitionMethod recoMethod,
        int                     level              = 3,
        QSet<size_t>            onlyVisible        = QSet<size_t>(),
        int                     errorScaleExponent = 0);

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
