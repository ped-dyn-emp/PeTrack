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

#ifndef TRACKER_H
#define TRACKER_H

#include <QList>
#include <QColor>
#include <QTextStream>

//#include <opencv2/core/core.hpp>


#include "vector.h"

#include "petrack.h"

#define EXTRAPOLATE_FACTOR 3. // war 1.5, aber bei bildauslassungen kann es ungewollt zuschlagen (bei 3 ist ein ausgelassener frame mgl, bei 2 wieder ein problem)
#define MAX_COUNT 1500 // maximale anzahl an gleichzeitig getrackten personen
//#define MIN_DISTANCE 30 // abstand zwischen kopfen !!!! wird nun ueber berechnung der kopfgroesse geregelt !!!!
#define MAX_STEP_TRACK 5 // maximale zahl an frames zwischen denen noch getrackt wird
#define MAX_TRACK_ERROR 200 // maximaler fehler beim tracken, so das noch punkt hinzugefuegt wird
                            // um am ende des tracking nicht am bildrand herumzukrakseln!
#define MIN_HEIGHT -100000. // minimale Personengroesse zur Identifizierung, dass Hoehe nicht gesetz, war -1, was aber an Treppen schlecht war

class TrackPoint : public Vec2F // Vec2F is pixel point in picture
{
private:
    Vec2F mColPoint; // center of color marker
    QColor mCol;     // color of corresponding marker
    int mQual;       // quality 0 (worst) .. 100 (best)
    int mMarkerID;   // ID of detected Marker
    Vec3F mSp;        // measured 3d point with stereo // mZdistanceToCam; // distance in z direction to camera - measured with stereo

public:
    TrackPoint();
    TrackPoint(const Vec2F &p);
    TrackPoint(const Vec2F &p, int qual);
    TrackPoint(const Vec2F &p, int qual, int markerID);
    TrackPoint(const Vec2F &p, int qual, const QColor &col);
    TrackPoint(const Vec2F &p, int qual, const Vec2F &colPoint, const QColor &col);

    inline const Vec2F& colPoint() const
    {
        return mColPoint;
    }
    inline void setColPoint(Vec2F &cp)
    {
        mColPoint = cp;
    }
//     inline Vec2F& colPoint()
//     {
//         return mColPoint;
//     }
    inline const QColor& color() const
    {
        return mCol;
    }
    inline void setColor(QColor& col)
    {
        mCol = col;
    }
//     inline QColor color()
//     {
//         return mCol;
//     }
    inline void setColPoint(const Vec2F &colPoint)
    {
        mColPoint = colPoint;
    }
    inline void setCol(const QColor &col)
    {
        mCol = col;
    }
    inline int qual() const
    {
        return mQual;
    }
    inline void setQual(int qual)
    {
        mQual = qual;
    }
    inline int getMarkerID() const
    {
        return mMarkerID;
    }
    inline void setMarkerID(int markerID)
    {
        mMarkerID = markerID;
    }

    inline const Vec3F& sp() const
    {
        return mSp;
    }
    inline void setSp(const Vec3F &sp)
    {
        mSp = sp;
    }
    inline void setSp(float x, float y, float z)
    {
        mSp.setX(x);
        mSp.setY(y);
        mSp.setZ(z);
    }

//    inline void setZdistanceToCam(float z)
//    {
//        mZdistanceToCam = z;
//    }
//    inline float zDistanceToCam() const
//    {
//        return mZdistanceToCam;
//    }

//     const TrackPoint& operator=(const TrackPoint& tp); // Zuweisungsoperator
    const TrackPoint& operator=(const Vec2F& v);
    const TrackPoint& operator+=(const Vec2F& v);
    const TrackPoint operator+(const Vec2F& v) const;
};

// inline ostream& operator<< (ostream& s, const TrackPoint& tp)
// {
//     s << "(" << tp.x() << ", " << tp.y() << ") " << tp.qual() << " - (" << tp.colPoint().x() << ", " << tp.colPoint().y() << ") (" << tp.color().red() << ", " << tp.color().green() << ", " << tp.color().blue() << ")";
//     return s;
// }

inline QTextStream& operator>>(QTextStream& s, TrackPoint& tp)
{
    double d;
    Vec2F p;
    Vec3F sp;
    QColor col;
    int qual;
    int markerID;

    s >> d; 
    tp.setX(d);
    s >> d;
    tp.setY(d);
    if (Petrack::trcVersion > 1)
    {
        s >> d;
        sp.setX(d);
        s >> d;
        sp.setY(d);
        s >> d;
        sp.setZ(d); //setZdistanceToCam(d);
        tp.setSp(sp);
    }
    s >> qual; 
    tp.setQual(qual);
    s >> d; 
    p.setX(d);
    s >> d; 
    p.setY(d);
    tp.setColPoint(p);
    s >> col; 
    tp.setColor(col);
    if (Petrack::trcVersion > 2)
    {
        s >> markerID;
        tp.setMarkerID(markerID);
    }
    return s;
}
inline QTextStream& operator<<(QTextStream& s, const TrackPoint& tp)
{
    if (Petrack::trcVersion > 2)
        s << tp.x() << " " << tp.y() << " " << tp.sp().x() << " " << tp.sp().y() << " " << tp.sp().z() << " " << tp.qual() << " " << tp.colPoint().x() << " " << tp.colPoint().y() << " " << tp.color() << " " << tp.getMarkerID();
    else if (Petrack::trcVersion == 2)
        s << tp.x() << " " << tp.y() << " " << tp.sp().x() << " " << tp.sp().y() << " " << tp.sp().z() << " " << tp.qual() << " " << tp.colPoint().x() << " " << tp.colPoint().y() << " " << tp.color();
    else
        s << tp.x() << " " << tp.y() << " " << tp.qual() << " " << tp.colPoint().x() << " " << tp.colPoint().y() << " " << tp.color();
    return s;
}
inline std::ostream& operator<<(std::ostream& s, const TrackPoint& tp)
{
    if (Petrack::trcVersion > 2)
        s << tp.x() << " " << tp.y() << " " << tp.sp().x() << " " << tp.sp().y() << " " << tp.sp().z() << " " << tp.qual() << " " << tp.colPoint().x() << " " << tp.colPoint().y() << " " << tp.color() << " " << tp.getMarkerID();
    else if (Petrack::trcVersion > 1)
        s << tp.x() << " " << tp.y() << " " << tp.sp().x() << " " << tp.sp().y() << " " << tp.sp().z() << " " << tp.qual() << " " << tp.colPoint().x() << " " << tp.colPoint().y() << " " << tp.color();
    else
        s << tp.x() << " " << tp.y() << " " << tp.qual() << " " << tp.colPoint().x() << " " << tp.colPoint().y() << " " << tp.color();
    return s;
}

//--------------------------------------------------------------------------

// the list index is the frame number plus mFirstFrame 0..mLastFrame-mFirstFrame
// no frame is left blank
class TrackPerson : public QList<TrackPoint>
{
private:
    int mNr; // person number
    int mMarkerID = -1; //markerID of Trackperson; -1 as positive values including 0 do exist as ArucoCodeNumbers-Values
    double mHeight; // height of the person
    int mHeightCount; // number of colors where mHeight is averaged
    int mFirstFrame; // 0.. frame where the person was tracked the first time
    int mLastFrame; // 0..
    //int qual; // quality
    bool mNewReco; // true if person was just recognized
    QColor mCol; // color of point
    QString mComment; // comment for person
    int mNrInBg; // number of successive frames in the background
    int mColCount; // number of colors where mCol is average from

public:
    TrackPerson();
//     TrackPerson(int nr, int frame, const Vec2F &p);
    TrackPerson(int nr, int frame, const TrackPoint &p);

    TrackPerson(int nr, int frame, const TrackPoint &p, int markerID);

    bool insertAtFrame(int frame, const TrackPoint &p, int persNr, bool extrapolate);

    inline int nrInBg() const
    {
        return mNrInBg;
    }
    inline void setNrInBg(int n)
    {
        mNrInBg = n;
    }

    inline double height() const
    {
        return mHeight;
    }
    // echte Personengroesse beim einlesen der trj datei
    inline void setHeight(double h)
    {
        mHeight = h;
    }
    // beim setzen der Hoehe wird Mittelwert gebildet
    // z is z distance to camera
    // altitude is height of the camera over floor
    inline void setHeight(float z, float altitude) // , int frame
    {
        //(*this)[frame-mFirstFrame].setZdistanceToCam(z);

        mHeight = (mHeight*mHeightCount+(altitude-z))/(mHeightCount+1);
        ++mHeightCount;
    }
    inline void resetHeight()
    {
        //(*this)[frame-mFirstFrame].setZdistanceToCam(z);

        mHeight = MIN_HEIGHT;
        mHeightCount = 0;
    }
    void recalcHeight(float altitude);
    double getNearestZ(int i, int *extrapolated);

    inline int getMarkerID() const
    {
        return mMarkerID;
    }
    inline void setMarkerID(const int markerID)
    {
        mMarkerID = markerID;
    }
    inline const QColor& color() const
    {
        return mCol;
    }
    inline void setColor(const QColor& col)
    {
        mCol = col;
    }
    inline bool newReco() const
    {
        return mNewReco;
    }
    inline void setNewReco(bool b)
    {
        mNewReco = b;
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
    inline int nr() const
    {
        return mNr;
    }
    inline void setNr(int nr)
    {
        mNr = nr;
    }
    inline const QString comment() const
    {
        return mComment;
    }
    /**
     * @brief Get the comment without line breaks
     *
     * Get the comment as a one line QString where all linebreak are replaced with '<br>'
     * @return comment without line breaks
     */
    inline QString serializeComment() const
    {
        return QString{mComment}.replace(QRegularExpression("\n"), "<br>");
    }

    inline void setComment(QString s)
    {
        mComment = s;
    }
    inline int colCount() const
    {
        return mColCount;
    }
    inline void setColCount(int c)
    {
        mColCount = c;
    }
    void addColor(const QColor &col);
    void optimizeColor();
    bool trackPointExist(int frame) const;
    const TrackPoint& trackPointAt(int frame) const; // & macht bei else probleme, sonst mit [] zugreifbar
    // gibt -1 zurueck, wenn frame oder naechster frame nicht existiert
    // entfernung ist absolut
    double distanceToNextFrame(int frame) const;
    void syncTrackPersonMarkerID(int markerID);
};

//mHeightCount wird nicht e3xportiert und auch nicht wieder eingelesen -> nach import auf 0 obwohl auf height ein wert steht, daher immer mheight auf -1 testen!!!
// keine Konsistenzueberpruefung
inline QTextStream& operator>>(QTextStream& s, TrackPerson& tp)
{
    double d;
    QColor col;
    int n;
    TrackPoint p;
    int markerID;

    s.skipWhiteSpace();
    QString str = s.readLine();

    QTextStream trjInfoLine(&str);
    //debout << str << endl;

    trjInfoLine >> n;
    tp.setNr(n);
    //debout << "Nr: " << tp.nr();
    trjInfoLine >> d;
    tp.setHeight(d);
    //cout << " Height: " << tp.height();
    trjInfoLine >> n;
    tp.setFirstFrame(n);
    //cout << " 1th frame: " << tp.firstFrame();
    trjInfoLine >> n;
    tp.setLastFrame(n);
    //cout << " last frame: " << tp.lastFrame();
    trjInfoLine >> n;
    tp.setColCount(n);
    //cout << " ColCount: " << tp.colCount();
    trjInfoLine >> col;
    tp.setColor(col);
    //cout << " col: " << tp.color();
    if (Petrack::trcVersion > 3)
    {
        trjInfoLine >> markerID;
        tp.setMarkerID(markerID);
    }
    trjInfoLine >> n; // size of list
    if (Petrack::trcVersion > 2) // Reading the comment line
    {
        //s.skipWhiteSpace(); // skip white spaces for reading the comment line without this the reading makes some problems
        // Kommentarzeile lesen
        str = s.readLine();
        tp.setComment(str.replace(QRegularExpression("<br>"), "\n"));
        //cout << " comment: " << tp.comment() << endl;
        //if ( !comment.isEmpty())
        //    s.skipWhiteSpace();
    }


    for (int i = 0; i < n; ++i)
    {
        s >> p;
        tp.append(p);
    }
    return s;
}

inline QTextStream& operator<<(QTextStream& s, const TrackPerson& tp)
{
    s << tp.nr() << " " << tp.height() << " " << tp.firstFrame() << " " << tp.lastFrame() << " " << tp.colCount() << " " << tp.color();
    if (Petrack::trcVersion > 3)
    {
        s << " " <<tp.getMarkerID();
    }
    s << " " << tp.size();
    s << Qt::endl << tp.serializeComment() << Qt::endl;
    for (int i = 0; i < tp.size(); ++i)
        s << tp.at(i) << Qt::endl;
    return s;
}

inline std::ostream& operator<<(std::ostream& s, const TrackPerson& tp)
{
    s << tp.nr() << " " << tp.height() << " " << tp.firstFrame() << " " << tp.lastFrame() << " " << tp.colCount() << " " << tp.color();
    if(Petrack::trcVersion > 3)
    {
        s <<  " " << tp.getMarkerID();
    }
    s << " " << tp.size();
    s << std::endl << tp.serializeComment() << std::endl;
    for (int i = 0; i < tp.size(); ++i)
        s << tp.at(i) << std::endl;
    return s;
}

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
class Tracker : public QList<TrackPerson>
{
private:
    Petrack *mMainWindow;
    cv::Mat mGrey, mPrevGrey;
    std::vector<cv::Mat> mPrevPyr, mCurrentPyr;
    std::vector<cv::Point2f> mPrevFeaturePoints, mFeaturePoints;
    std::vector<uchar> mStatus;
    int mPrevFrame;
    std::vector<int> mPrevFeaturePointsIdx;
    std::vector<float> mTrackError;
    cv::TermCriteria mTermCriteria;

public:
    Tracker(QWidget *wParent);

    // neben loeschen der liste muessen auch ...
    void init(cv::Size size);

    void reset();

    void resize(cv::Size size);

    void splitPerson(int pers, int frame);
    bool splitPersonAt(const Vec2F& p, int frame, QSet<int> onlyVisible);

    bool delPointOf(int pers, int direction, int frame);
    bool delPoint(const Vec2F& p, int direction, int frame, QSet<int> onlyVisible);
    void delPointAll(int direction, int frame);
    void delPointROI();
    void delPointInsideROI();
    bool editTrackPersonComment(const Vec2F& p, int frame, const QSet<int>& onlyVisible);
    bool setTrackPersonHeight(const Vec2F& p, int frame, QSet<int> onlyVisible);
    bool resetTrackPersonHeight(const Vec2F& p, int frame, QSet<int> onlyVisible);

    // used for calculation of 3D point for all points in frame
    // returns number of found points or -1 if no stereoContext available (also points without disp found are counted)
    int calcPosition(int frame);

    // true, if new traj is inserted with point p and initial frame frame
    // p in pixel coord
    // pers wird gesetzt, wenn existierender trackpoint einer person verschoben wird
    bool addPoint(TrackPoint &p, int frame, QSet<int> onlyVisible, int *pers = NULL);

    // hier sollte direkt die farbe mit uebergeben werden
    void addPoints(QList<TrackPoint> &pL, int frame);

    // calculate height of person

    // convert all trajectorie coordinates in real coordinate (height needed)

    int visible(int frameNum);
    int largestFirstFrame();
    int largestLastFrame();
    int smallestFirstFrame();
    int smallestLastFrame();

    size_t calcPrevFeaturePoints(int prevFrame, cv::Rect &rect, int frame, bool reTrack, int reQual, int borderSize, QSet<int> onlyVisible);

    int insertFeaturePoints(int frame, size_t count, cv::Mat &img, int borderSize, float errorScale);

    // frame ist frame fuer naechsten prev frame
    int track(cv::Mat &img,cv::Rect &rect, int frame, bool reTrack, int reQual, int borderSize, int level=3, QSet<int> onlyVisible = QSet<int>(), int errorScaleExponent=0);


    void checkPlausibility(QList<int> &pers, QList<int> &frame,
                           bool testEqual = true,
                           bool testVelocity = true,
                           bool testInside = true,
                           bool testLength = true);
    void optimizeColor();

    // reset the height of all persons, but not the pos of the trackpoints
    void resetHeight();
    // reset the pos of the tzrackpoints, but not the heights
    void resetPos();
    void recalcHeight(float altitude);

    void setMarkerHeights(const std::unordered_map<int, float> &heights);

    void setMarkerIDs(const std::unordered_map<int, int> &markerIDs);

    // gibt groessenverteilung der personen auf stdout aus
    // rueckgabewert false wenn keine hoeheninformationen in tracker datensatz vorliegt
    bool printHeightDistribution();

    void purge(int frame);

private:
    bool tryMergeTrajectories(const TrackPoint& v, size_t i, int frame);

    void trackFeaturePointsLK(int level);
    void trackFeaturePointsLK(int level, bool adaptive);
    void refineViaColorPointLK(int level, float errorScale);
    void useBackgroundFilter(QList<int>& trjToDel, BackgroundFilter *bgFilter);
    void refineViaNearDarkPoint();
    void preCalculateImagePyramids(int level);
};

#endif
