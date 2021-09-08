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

#include "tracker.h"

#include "animation.h"
#include "control.h"
#include "helper.h"
#include "multiColorMarkerWidget.h"
#include "pMessageBox.h"
#include "petrack.h"
#include "recognitionRoiItem.h"
#include "stereoWidget.h"

#include <ctime>
#include <iomanip>
#include <opencv2/opencv.hpp>

#define MIN_WIN_SIZE 3.

/**
 * @brief Transforms OpenCV error in tracking (L1-Norm) to quality
 * @param error error reported by cv::calcOpticalFlowPyrLK
 * @return quality according to error
 */
inline float errorToQual(float error)
{
    return 80.F - error / 20.F;
}

TrackPoint::TrackPoint() : mQual(0), mMarkerID(-1), mSp(-1., -1., -1.) {}
TrackPoint::TrackPoint(const Vec2F &p) : Vec2F(p), mQual(0), mMarkerID(-1), mSp(-1., -1., -1.) {}
TrackPoint::TrackPoint(const Vec2F &p, int qual) : Vec2F(p), mQual(qual), mMarkerID(-1), mSp(-1., -1., -1.) {}
TrackPoint::TrackPoint(const Vec2F &p, int qual, int markerID) :
    Vec2F(p), mQual(qual), mMarkerID(markerID), mSp(-1., -1., -1.)
{
}
TrackPoint::TrackPoint(const Vec2F &p, int qual, const QColor &col) :
    Vec2F(p), mCol(col), mQual(qual), mMarkerID(-1), mSp(-1., -1., -1.)
{
}

TrackPoint::TrackPoint(const Vec2F &p, int qual, const Vec2F &colPoint, const QColor &col) :
    Vec2F(p), mColPoint(colPoint), mCol(col), mQual(qual), mMarkerID(-1), mSp(-1., -1., -1.)
{
}

const TrackPoint &TrackPoint::operator=(const Vec2F &v)
{
    Vec2F::operator=(v);
    return *this;
}

const TrackPoint &TrackPoint::operator+=(const Vec2F &v)
{
    Vec2F::operator+=(v);
    return *this;
}
const TrackPoint TrackPoint::operator+(const Vec2F &v) const
{
    return TrackPoint(*this) += v; // Vec2F(mX + v.mX, mY + v.mY);
}

//--------------------------------------------------------------------------

// the list index is the frame number plus mFirstFrame 0..mLastFrame-mFirstFrame
// no frame is left blank
TrackPerson::TrackPerson() :
    mNr(0),
    mHeight(MIN_HEIGHT),
    mHeightCount(0),
    mFirstFrame(-1),
    mLastFrame(0),
    mNewReco(false),
    mComment(),
    mNrInBg(0),
    mColCount(0)
{
}
TrackPerson::TrackPerson(int nr, int frame, const TrackPoint &p) :
    mNr(nr),
    mMarkerID(-1),
    mHeight(MIN_HEIGHT),
    mHeightCount(0),
    mFirstFrame(frame),
    mLastFrame(frame),
    mNewReco(true),
    mCol(p.color()),
    mComment(),
    mColCount(1)
{
    append(p);
}

TrackPerson::TrackPerson(int nr, int frame, const TrackPoint &p, int markerID) :
    mNr(nr),
    mMarkerID(markerID),
    mHeight(MIN_HEIGHT),
    mHeightCount(0),
    mFirstFrame(frame),
    mLastFrame(frame),
    mNewReco(true),
    mCol(p.color()),
    mComment(),
    mColCount(1)
{
    append(p);
}

// mittelt alle erkannten farben der trackpoints zu einer farbe der trackperson
void TrackPerson::addColor(const QColor &col)
{
    mCol.setRed((mColCount * mCol.red() + col.red()) / (mColCount + 1));
    mCol.setBlue((mColCount * mCol.blue() + col.blue()) / (mColCount + 1));
    mCol.setGreen((mColCount * mCol.green() + col.green()) / (mColCount + 1));

    ++mColCount;
}

// die gemittelte farb wird hier verbessert
void TrackPerson::optimizeColor()
{
    // ausreisser herausnehmen ueber die koordinate der farbe
    int   i, j;
    Vec2F v, vBefore;
    int   anz1 = 0, anz2 = 0;
    bool  swap = false;

    // den ersten farbpunkt suchen und vBefore initial setzen
    for(i = 0; i < size(); ++i)
    {
        if(at(i).color().isValid())
        {
            vBefore = at(i).colPoint() - at(i);
            break;
        }
    }
    if(at(i).color().isValid())
        ++anz1;
    // testen, auf welcher seit der farbmarker haeufiger gesehen wird
    for(j = i + 1; j < size(); ++j)
    {
        if(at(j).color().isValid())
        {
            v = at(j).colPoint() - at(j);
            if((v * vBefore) < 0)
                swap = !swap;

            if(swap)
                ++anz2;
            else
                ++anz1;
            vBefore = v;
        }
    }
    swap = false;
    if(at(i).color().isValid())
        vBefore = at(i).colPoint() - at(i);
    // farben mit geringerer anzahl loeschen
    QColor colInvalid;
    if(anz2 > anz1)
        (*this)[i].setColor(colInvalid);
    for(j = i + 1; j < size(); ++j)
    {
        if(at(j).color().isValid())
        {
            v = at(j).colPoint() - at(j);
            if((v * vBefore) < 0)
                swap = !swap;

            if(swap)
            {
                if(anz1 > anz2)
                    (*this)[j].setColor(colInvalid);
            }
            else
            {
                if(anz2 > anz1)
                    (*this)[j].setColor(colInvalid);
            }
            vBefore = v;
        }
    }

    // median statt mittelwert nehmen
    QList<int> r;
    QList<int> g;
    QList<int> b;
    for(i = 0; i < size(); ++i)
    {
        if(at(i).color().isValid())
        {
            r.append(at(i).color().red());
            g.append(at(i).color().green());
            b.append(at(i).color().blue());
        }
    }
    std::sort(r.begin(), r.end());
    std::sort(g.begin(), g.end());
    std::sort(b.begin(), b.end());
    if(r.size() > 0 && g.size() > 0 && b.size() > 0) // kann eigentlich nicht vorkommen
        setColor(QColor(r[(r.size() - 1) / 2], g[(g.size() - 1) / 2], b[(b.size() - 1) / 2]));
}

void TrackPerson::recalcHeight(float altitude)
{
    double z = 0;
    // median statt mittelwert nehmen (bei gerader anzahl an werten den kleiner als mitte)
    QList<double> zList;

    resetHeight();

    for(int i = 0; i < size(); ++i)
    {
        z = at(i).sp().z();
        if(z >= 0)
        {
            ++mHeightCount;
            // h += z;
            zList.append(z);
        }
    }
    if(mHeightCount > 0)
    {
        std::sort(zList.begin(), zList.end());
        mHeight = zList[mHeightCount / 2];
        mHeight = altitude - mHeight;
    }
}

// gibt den ersten z-wert um index i heraus der ungleich -1 ist
// zudem interpolation zwischen Werten!
double TrackPerson::getNearestZ(int i, int *extrapolated)
{
    *extrapolated = 0;
    if((i < 0) || (i >= size())) // indexueberpruefung
        return -1.;
    if(at(i).sp().z() >= 0)
        return at(i).sp().z();
    else // -1 an aktueller hoehe
    {
        int nrFor = 1;
        int nrRew = 1;
        while((i + nrFor < size()) &&
              (at(i + nrFor).sp().z() < 0)) // nach && wird nur ausgefuehrt, wenn erstes true == size() also nicht
            nrFor++;
        while((i - nrRew >= 0) &&
              (at(i - nrRew).sp().z() < 0)) // nach && wird nur ausgefuehrt, wenn erstes true == size() also nicht
            nrRew++;
        if((i + nrFor == size()) && (i - nrRew < 0)) // gar keine Hoeheninfo in trj gefunden
            return -1.;
        else if(i + nrFor == size()) // nur in Vergangenheit hoeheninfo gefunden
        {
            *extrapolated = 2;
            return at(i - nrRew).sp().z();
        }
        else if(i - nrRew < 0) // nur in der zukunft hoeheninfo gefunden
        {
            *extrapolated = 1;
            return at(i + nrFor).sp().z();
        }
        else // in beiden richtungen hoeheninfo gefunden - INTERPOLATION, NICHT EXTRAPOLATION
            return at(i - nrRew).sp().z() +
                   nrRew * (at(i + nrFor).sp().z() - at(i - nrRew).sp().z()) / (nrFor + nrRew); // lineare interpolation
    }
}

// rueckgabe zeigt an, ob neuer point eingefuegt wurde oder nicht, falls qualitaet schlechter
// persNr ist index in uebergeordneter liste zur sinnvollen warnungs-ausgabe
/**
 * @brief Inserts point
 *
 * If the point would be appended or prepended, it is inserted
 * and the points at the frames between the current one and the
 * last one with a frame get linearly interpolated. If the point
 * being appended/prepended is further away than the speed
 * (between the last two points) would reasonably allow, the point
 * is either not inserted or extrapolated.
 *
 * If the new point would replace an old point, it is only added
 * if it has a better quality.
 *
 * Multicolor-Blackdot and Multicolor-Aruco markers which get
 * recognized by color are shifted by the difference between
 * dot/code-point and color-point to avoid stuttering of the
 * trajectory.
 *
 * @param frame
 * @param point
 * @param persNr
 * @param extrapolate
 * @return true if point was added
 */
bool TrackPerson::insertAtFrame(int frame, const TrackPoint &point, int persNr, bool extrapolate)
{
    int        i;
    Vec2F      tmp; // ua. zur linearen Interpolation
    TrackPoint tp;  // default: 0 = ist schlechteste qualitaet
    double     distance;

    if(frame > mLastFrame)
    {
        // lineare interpolation, wenn frames uebersprungen wurden
        if(frame - mLastFrame - 1 > 0)
        {
            tmp.setX((point.x() - last().x()) / (frame - mLastFrame));
            tmp.setY((point.y() - last().y()) / (frame - mLastFrame));
            tp = last();
            tp.setQual(0);
            for(i = 0; i < frame - mLastFrame - 1; ++i)
            {
                tp += tmp;
                append(tp);
            }
            append(point);
        }
        else if(extrapolate && ((mLastFrame - mFirstFrame) > 0)) // mind. 2 trackpoints sind in liste!
        {
            tmp = last() - at(size() - 2); // vektor zw letztem und vorletztem pkt
            // der Abstand zum extrapoliertem Ziel darf nicht groesser als 2x so gross sein, wie die entfernung
            // vorheriger trackpoints eine mindestbewegung wird vorausgesetzt, da sonst bewegungen aus dem "stillstand"
            // immer als fehler angesehen werden
            //   am besten waere eine fehlerbetrachtung in abhaengigkeit von der geschwindigkeit - nicht gemacht!
            if(((distance = ((last() + tmp).distanceToPoint(point))) > EXTRAPOLATE_FACTOR * tmp.length()) &&
               (distance > 3))
            {
                if(!((last().qual() == 0) &&
                     (at(size() - 2).qual() == 0))) // das vorherige einfuegen ist 2x nicht auch schon schlecht gewesen
                {
                    tp = point;
                    debout << "Warning: Extrapolation instaed of tracking because of big difference from tracked point "
                              "in speed and direction of person "
                           << persNr + 1 << " between frame " << mLastFrame << " and " << mLastFrame + 1 << "!"
                           << std::endl;
                    tp = last() + tmp; // nur vektor wird hier durch + geaendert
                    tp.setQual(0);
                    // im anschluss koennte noch dunkelster pkt in umgebung gesucht werden!!!
                    // keine Extrapolation der Groesse
                    append(tp);
                }
                else
                {
                    debout << "Warning: Because of three big differences from tracked point in speed and direction "
                              "between last three frames the track point of person "
                           << persNr + 1 << " at " << mLastFrame + 1 << " was NOT inserted!" << std::endl;
                    return false;
                }
            }
            else
                append(point);
        }
        else
            append(point);
        mLastFrame = frame;
    }
    else if(frame < mFirstFrame)
    {
        if(mFirstFrame - frame - 1 > 0)
        {
            tmp.setX((point.x() - first().x()) / (mFirstFrame - frame));
            tmp.setY((point.y() - first().y()) / (mFirstFrame - frame));
            tp = first();
            tp.setQual(0);
            for(i = 0; i < mFirstFrame - frame - 1; ++i)
            {
                tp += tmp;
                prepend(tp);
            }
            prepend(point);
        }
        else if(extrapolate && ((mLastFrame - mFirstFrame) > 0)) // mind. 2 trackpoints sind in liste!
        {
            tmp = at(0) - at(1); // vektor zw letztem und vorletztem pkt
            // der Abstand zum extrapoliertem Ziel darf nicht groesser als 2x so gross sein, wie die entfernung
            // vorheriger trackpoints eine mindestbewegung wird vorausgesetzt, da sonst bewegungen aus dem "stillstand"
            // immer als fehler angesehen werden
            //   am besten waere eine fehlerbetrachtung in abhaengigkeit von der geschwindigkeit - nicht gemacht!
            if(((distance = (at(0) + tmp).distanceToPoint(point)) > EXTRAPOLATE_FACTOR * tmp.length()) &&
               (distance > 3))
            {
                if(!((at(0).qual() == 0) &&
                     (at(1).qual() == 0))) // das vorherige einfuegen ist 2x nicht auch schon schlecht gewesen
                {
                    tp = point;
                    debout << "Warning: Extrapolation instaed of tracking because of big difference from tracked point "
                              "in speed and direction of person "
                           << persNr + 1 << " between frame " << mFirstFrame << " and " << mFirstFrame - 1 << "!"
                           << std::endl;
                    tp = at(0) + tmp; // nur vektor wird hier durch + geaendert
                    tp.setQual(0);
                    prepend(tp);
                }
                else
                {
                    debout << "Warning: Because of three big differences from tracked point in speed and direction "
                              "between last three frames the track point of person "
                           << persNr + 1 << " at " << mLastFrame - 1 << " was NOT inserted!" << std::endl;
                    return false;
                }
            }
            else
                prepend(point);
        }
        else
            prepend(point);
        mFirstFrame = frame;
    }
    else
    {
        // dieser Zweig wird insbesondere von reco durchlaufen, da vorher immer auch noch getrackt wird und reco
        // draufgesetzt wird!!!

        tp = point;
        if(point.qual() < 100 && point.qual() > 80) // erkannte Person aber ohne strukturmarker
        {
            // wenn in angrenzenden Frames qual groesse 90 (100 oder durch vorheriges verschieben entstanden), dann
            // verschieben
            if(trackPointExist(frame - 1) && trackPointAt(frame - 1).qual() > 90)
            {
                tp.setQual(95);
                tmp = point + (trackPointAt(frame - 1) - trackPointAt(frame - 1).colPoint());
                tp.set(tmp.x(), tmp.y());
                debout << "Warning: move trackpoint according to last distance of structur marker and color marker of "
                          "person "
                       << persNr + 1 << ":" << std::endl;
                debout << "         " << point << " -> " << tp << std::endl;
            }
            else if(trackPointExist(frame + 1) && trackPointAt(frame + 1).qual() > 90)
            {
                tp.setQual(95);
                tmp = point + (trackPointAt(frame + 1) - trackPointAt(frame + 1).colPoint());
                tp.set(tmp.x(), tmp.y());
                debout << "Warning: move trackpoint according to last distance of structur marker and color marker of "
                          "person "
                       << persNr + 1 << ":" << std::endl;
                debout << "         " << point << " -> " << tp << std::endl;
            }
        }

        // ueberprueft, welcher punkt besser
        if(tp.qual() > at(frame - mFirstFrame).qual())
        {
            // warnung ausgeben, wenn replacement (fuer gewoehnlich von reco) den pfadverlauf abrupt aendert
            if(trackPointExist(frame - 1))
                tmp = trackPointAt(frame) - trackPointAt(frame - 1);
            else if(trackPointExist(frame + 1))
                tmp = trackPointAt(frame + 1) - trackPointAt(frame);
            if((trackPointExist(frame - 1) || trackPointExist(frame + 1)) &&
               ((distance = (trackPointAt(frame).distanceToPoint(tp))) > 1.5 * tmp.length()) && (distance > 3))
            {
                int anz;
                debout << "Warning: Big difference in location between existing and replacing track point of person "
                       << persNr + 1 << " in frame " << frame << "!" << std::endl;
                // qualitaet anpassen, da der weg zum pkt nicht der richtige gewesen sein kann
                // zurueck
                for(anz = 1; trackPointExist(frame - anz) && (trackPointAt(frame - anz).qual() < 100); ++anz)
                    ;
                for(i = 1; i < (anz - 1);
                    ++i) // anz ist einer zu viel; zudem nur boie anz-1 , da sonst eh nur mit 1 multipliziert wuerde
                    (*this)[frame - mFirstFrame - i].setQual((i * trackPointAt(frame - i).qual()) / anz);
                // vor
                for(anz = 1; trackPointExist(frame + anz) && (trackPointAt(frame + anz).qual() < 100); ++anz)
                    ;
                for(i = 1; i < (anz - 1);
                    ++i) // anz ist einer zu viel; zudem nur boie anz-1 , da sonst eh nur mit 1 multipliziert wuerde
                    (*this)[frame - mFirstFrame + i].setQual((i * trackPointAt(frame + i).qual()) / anz);
            }

            replace(frame - mFirstFrame, tp);

            if(tp.qual() > 100)                            // manual add // after inserting, because point ist const
                (*this)[frame - mFirstFrame].setQual(100); // so moving of a point is possible
        }
        else
            return false;
    }
    return true;
}

bool TrackPerson::trackPointExist(int frame) const
{
    if(frame >= mFirstFrame && frame <= mLastFrame)
        return true;
    else
        return false;
}
const TrackPoint &TrackPerson::trackPointAt(int frame) const // & macht bei else probleme, sonst mit [] zugreifbar
{
    return at(frame - mFirstFrame);
}

/**
 * @brief Absolute distance to next frame
 *
 * @param frame
 * @return absolute distance or -1, if frame doen't exist
 */
double TrackPerson::distanceToNextFrame(int frame) const
{
    if(frame >= mFirstFrame && frame + 1 <= mLastFrame)
        return at(frame - mFirstFrame).distanceToPoint(at(frame - mFirstFrame + 1));
    else
        return -1;
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

Tracker::Tracker(QWidget *wParent)
{
    mMainWindow = (class Petrack *) wParent;
    mTermCriteria =
        cv::TermCriteria(cv::TermCriteria::COUNT | cv::TermCriteria::EPS, 20, 0.03); ///< maxIter=20 and epsilon=0.03

    reset();
}


// neben loeschen der liste muessen auch ...
void Tracker::init(cv::Size size)
{
    clear(); // loescht liste aller getrackten personen
    // nicht mehr noetig, da nicht mehr in track selber // jetzt start, war prevImg == NULL && prevFrame == -1 zeigt an,
    // dass ein neuer Trackingprozess beginnt / neue Bildfolge

    mGrey.create(size, CV_8UC1);
    mPrevGrey.create(size, CV_8UC1);

    reset();
}

// damit neu aufgesetzt werden kann
void Tracker::reset()
{
    mPrevFrame = -1; // das vorherige Bild ist zu ignorieren oder existiert nicht
}

void Tracker::resize(cv::Size size)
{
    if(!mGrey.empty() && ((size.width != mGrey.cols) || (size.height != mGrey.rows)))
    {
        mGrey.create(size, CV_8UC1);

        // umkopieren des alten Graubildes in groesseres oder auch kleineres bild (wg border)
        // aus borderFilter kopiert
        if(!mPrevGrey.empty())
        {
            int s = (size.width - mPrevGrey.cols) / 2;
            if(s >= 0)
                cv::copyMakeBorder(mPrevGrey, mPrevGrey, s, s, s, s, cv::BORDER_CONSTANT, cv::Scalar(0));
            else
                mPrevGrey = mPrevGrey(cv::Rect(-s, -s, mPrevGrey.cols + 2 * s, mPrevGrey.rows + 2 * s));
        }
    }
}

/// split trajectorie pers before frame frame
void Tracker::splitPerson(int pers, int frame)
{
    int j;

    if(at(pers).firstFrame() < frame)
    {
        append(at(pers));

        // alte trj einkuerzen und ab aktuellem frame zukunft loeschen
        for(j = 0; j < at(pers).lastFrame() - frame + 1; ++j)
            (*this)[pers].removeLast();
        (*this)[pers].setLastFrame(frame - 1);

        // neu angehaengte/gedoppelte trajektorie
        for(j = 0; j < frame - last().firstFrame(); ++j)
            last().removeFirst();
        last().setFirstFrame(frame);
    }
}

/**
 * @brief Split trajectory at point point before given frame
 *
 * @param point point where to split trajectory (helpful if onlyVisible isn't set)
 * @param frame frame at which to split the trajectory
 * @param onlyVisible set of people for whom to do it (empty means everyone)
 * @return true if a trajectory was split
 */
bool Tracker::splitPersonAt(const Vec2F &point, int frame, QSet<int> onlyVisible)
{
    int i;

    for(i = 0; i < size(); ++i) // ueber TrackPerson
        if(((onlyVisible.empty()) || (onlyVisible.contains(i))) &&
           (at(i).trackPointExist(frame) &&
            (at(i).trackPointAt(frame).distanceToPoint(point) < mMainWindow->getHeadSize(nullptr, i, frame) / 2.)))
        {
            splitPerson(i, frame);

            return true;
        }
    return false;
}

/**
 * @brief Deletes points of pers
 * @param pers TrackPerson whose points should be deleted
 * @param direction notes if previous (-1), following(1) or whole(0) trajectory should be deleted
 * @param frame
 * @return true, if deletion occured
 */
bool Tracker::delPointOf(int pers, int direction, int frame)
{
    int j;

    if(direction == -1)
    {
        for(j = 0; j < frame - at(pers).firstFrame(); ++j)
            (*this)[pers].removeFirst();
        (*this)[pers].setFirstFrame(frame);
    }
    else if(direction == 0)
        removeAt(pers);
    else if(direction == 1)
    {
        for(j = 0; j < at(pers).lastFrame() - frame; ++j)
            (*this)[pers].removeLast();
        (*this)[pers].setLastFrame(frame);
    }

    return true;
}

// gibt true zurueck, wenn punkt geloescht werden konnte
// direction zeigt an, ob bis zum aktuellen (-1), ab dem aktuellen (1) oder ganzer trackpath (0)
// onlyVisible == -1 : immer alles betrachten, ansonsten nur person onlyVisible
// loescht trackpoint nur einer trajektorie
/**
 * @brief Deletes points of a SINGLE person in onlyVisible
 * @param point point which need to be on the person (helpful if onlyVisible is not properly set)
 * @param direction notes if previous (-1), following(1) or whole(0) trajectory should be deleted
 * @param frame
 * @param onlyVisible set of people whose points could be deleted; empty means everyone
 * @return true if deletion occured
 */
bool Tracker::delPoint(const Vec2F &point, int direction, int frame, QSet<int> onlyVisible)
{
    int i;

    for(i = 0; i < size(); ++i) // ueber TrackPerson
        if(((onlyVisible.empty()) || (onlyVisible.contains(i))) &&
           (at(i).trackPointExist(frame) &&
            (at(i).trackPointAt(frame).distanceToPoint(point) < mMainWindow->getHeadSize(nullptr, i, frame) / 2.)))
        {
            delPointOf(i, direction, frame);
            return true;
        }
    return false;
}

/**
 * @brief Deletes trackpoints of all trajectories
 * @param direction notes if previous (-1), following(1) or whole(0) trajectory should be deleted
 * @param frame
 */
void Tracker::delPointAll(int direction, int frame)
{
    int i, j;

    for(i = 0; i < size(); ++i) // ueber TrackPerson
    {
        if(at(i).trackPointExist(frame)) //
        {
            if(direction == -1)
            {
                for(j = 0; j < frame - at(i).firstFrame(); ++j)
                    (*this)[i].removeFirst();
                (*this)[i].setFirstFrame(frame);
            }
            else if(direction == 0)
                removeAt(i--); // nach Loeschen wird i um 1 erniedrigt
            else if(direction == 1)
            {
                for(j = 0; j < at(i).lastFrame() - frame; ++j)
                    (*this)[i].removeLast();
                (*this)[i].setLastFrame(frame);
            }
        }
        else if(
            ((direction == -1) && (frame > at(i).lastFrame())) || (direction == 0) ||
            ((direction == 1) && (frame < at(i).firstFrame())))
        {
            removeAt(i);
            i--;
        }
    }
}


// deletes points of a trajectrory, which are inside ROI
// 1 trajectory can end in 0, 1 or multiple trajectories!!!!!!!!
// man koennte noch unterscheiden, ob trajektorie aktuell in petrack zu sehen sein soll
void Tracker::delPointInsideROI()
{
    int    i, j;
    QRectF rect = mMainWindow->getRecoRoiItem()->rect();
    bool   inside;

    for(i = 0; i < size(); ++i) // ueber TrackPerson
    {
        inside = ((at(i).size() > 0) && rect.contains(at(i).at(0).x(), at(i).at(0).y()));
        for(j = 1; j < at(i).size(); ++j)
        {
            if(inside != rect.contains(at(i).at(j).x(), at(i).at(j).y())) // aenderung von inside
            {
                splitPerson(i, at(i).firstFrame() + j);
                if(inside)
                {
                    removeAt(i);
                    i--;
                    inside = !inside;
                }
                break;
            }
        }
        if(inside)
        {
            // rest loeschen
            removeAt(i);
            i--;
        }
    }
}

// deletes trajectory, if it is partly inside ROI
// man koennte noch unterscheiden, ob trajektorie aktuell in petrack zu sehen sein soll
void Tracker::delPointROI()
{
    int    i, j, anz = 0;
    QRectF rect = mMainWindow->getRecoRoiItem()->rect();

    for(i = 0; i < size(); ++i) // ueber TrackPerson
    {
        for(j = 0; j < at(i).size(); ++j)
        {
            if(rect.contains(at(i).at(j).x(), at(i).at(j).y()))
            {
                anz++;
                removeAt(i);
                i--;
                break;
            }
        }
    }
    debout << "deleted " << anz << " trajectories!" << std::endl;
}

/**
 * @brief Editing the comment of a TrackPerson
 *
 * Allows editing the comment of a TrackPerson in a new Dialog. When a new dialog gets opened, it automatically
 * appends 'Frame {\point frame}: ' to the dialog, if no comment for the frame exists.
 *
 * @param point position the user clicked
 * @param frame current frame number
 * @param onlyVisible list of visible persons
 * @return if a comment has been saved
 */
bool Tracker::editTrackPersonComment(const Vec2F &point, int frame, const QSet<int> &onlyVisible)
{
    for(int i = 0; i < size(); ++i) // ueber TrackPerson
    {
        if(((onlyVisible.empty()) || (onlyVisible.contains(i))) &&
           (at(i).trackPointExist(frame) && (at(i).trackPointAt(frame).distanceToPoint(point) <
                                             mMainWindow->getHeadSize(nullptr, i, frame) /
                                                 2.))) // war: MIN_DISTANCE)) // 30 ist abstand zwischen kopfen
        {
            QString displayedComment = at(i).comment();
            QString framePrefix      = "Frame " + QString::number(frame, 'g', 5) + ": ";

            if(displayedComment.isEmpty())
            {
                displayedComment.append(framePrefix);
            }
            else if(!displayedComment.contains(framePrefix))
            {
                displayedComment.append("\n" + framePrefix);
            }

            bool    ok      = false;
            QString comment = QInputDialog::getMultiLineText(
                mMainWindow, QObject::tr("Add Comment"), QObject::tr("Comment:"), displayedComment, &ok);

            if(ok)
            {
                if(comment.isEmpty())
                {
                    int ret = PWarning(
                        mMainWindow,
                        QObject::tr("Empty comment"),
                        QObject::tr("Are you sure you want to save an empty comment?"),
                        PMessageBox::StandardButton::Save | PMessageBox::StandardButton::Cancel);
                    if(ret == PMessageBox::StandardButton::Cancel)
                    {
                        return false;
                    }
                }
                (*this)[i].setComment(comment);
                return true;
            }
        }
    }
    return false;
}

bool Tracker::setTrackPersonHeight(const Vec2F &point, int frame, QSet<int> onlyVisible)
{
    int i;

    for(i = 0; i < size(); ++i) // ueber TrackPerson
    {
        if(((onlyVisible.empty()) || (onlyVisible.contains(i))) &&
           (at(i).trackPointExist(frame) && (at(i).trackPointAt(frame).distanceToPoint(point) <
                                             mMainWindow->getHeadSize(nullptr, i, frame) /
                                                 2.))) // war: MIN_DISTANCE)) // 30 ist abstand zwischen kopfen
        {
            bool ok;

            double col_height;
            // col_height is negative, if height is determined through color and not yet set manually
            if(at(i).height() < MIN_HEIGHT + 1)
                col_height = at(i).color().isValid() ?
                                 -mMainWindow->getControlWidget()->getColorPlot()->map(at(i).color()) :
                                 -mMainWindow->getControlWidget()->mapDefaultHeight->value();
            else
                col_height = at(i).height();


            double height = QInputDialog::getDouble(
                mMainWindow,
                QObject::tr("Set person height"),
                QObject::tr("Person height[cm]:"),
                fabs(col_height),
                -500,
                500,
                1,
                &ok);
            if(ok)
            {
                if(height < 0)
                {
                    debout << "Warning: you entered a negative height!" << std::endl; // is not supported!" << endl;
                    // return false;
                }
                // if previous value (col_height) is negative, height was determined thru color. If manually set value
                // is the color-map value, we do not change anything
                // @todo: @ar.graf: check if manually set values have side-effects (maybe do not show in statistics)
                if(!(std::abs(col_height + height) < 0.01))
                {
                    (*this)[i].setHeight(height);
                    return true;
                }
                else
                {
                    debout << std::endl
                           << "No height change detected. Color-mapped height will remain set." << std::endl;
                }
            }
        }
    }
    return false;
}
bool Tracker::resetTrackPersonHeight(const Vec2F &point, int frame, QSet<int> onlyVisible)
{
    int i;

    for(i = 0; i < size(); ++i) // ueber TrackPerson
    {
        if(((onlyVisible.empty()) || (onlyVisible.contains(i))) &&
           (at(i).trackPointExist(frame) && (at(i).trackPointAt(frame).distanceToPoint(point) <
                                             mMainWindow->getHeadSize(nullptr, i, frame) /
                                                 2.))) // war: MIN_DISTANCE)) // 30 ist abstand zwischen kopfen
        {
            (*this)[i].setHeight(MIN_HEIGHT);
            return true;
        }
    }
    return false;
}


// used for calculation of 3D point for all points in frame
// returns number of found points or -1 if no stereoContext available (also points without disp found are counted)
int Tracker::calcPosition(int /*frame*/)
{
#ifndef STEREO_DISABLED
    int                 anz = 0, notFoundDisp = 0;
    pet::StereoContext *sc = mMainWindow->getStereoContext();
    float               x, y, z;

    if(sc)
    {
        // for every point of a person, which has already identified at this frame
        for(int i = 0; i < size(); ++i) // ueber TrackPerson
        {
            if(at(i).trackPointExist(frame))
            {
                ++anz;

                // TrackPoint *point = &(at(i).trackPointAt(frame));
                //  ACHTUNG: BORDER NICHT BEACHTET bei p.x()...???
                //  calculate height with disparity map
                if(sc->getMedianXYZaround(
                       (int) at(i).trackPointAt(frame).x(),
                       (int) at(i).trackPointAt(frame).y(),
                       &x,
                       &y,
                       &z)) // nicht myRound, da pixel 0 von 0..0.99 in double geht
                {
                    // hier kommt man nur hinein, wenn x, y, z Wert berechnet werden konnten
                    // statt altitude koennte hier irgendwann die berechnete Bodenhoehe einfliessen
                    (*this)[i][frame - at(i).firstFrame()].setSp(x, y, z); // setZdistanceToCam(z);
                    (*this)[i].setHeight(z, mMainWindow->getControlWidget()->coordAltitude->value());
                }
                else
                    ++notFoundDisp;
                // else // Meldung zu haeufig
                //     debout << "Warning: No disparity information for person " << i+1 << "." << endl;
            }
        }
        // if (notFoundDisp>0) // Meldung zu haeufig
        //     debout << "Warning: No disparity information found for " << (100.*notFoundDisp)/anz << " percent of
        //     points." << endl;
        return anz;
    }
    else
        return -1;
#endif
    return -1;
}

/**
 * @brief Adds the point to the Tracker, either to exising person or creating a new one.
 *
 * This function find the nearest person to the given point and if the distance between point
 * and trajectory is small enough, it gets added to this trajectory. If the point is not fitting to
 * any trajectory, a new TrackPerson is created.
 *
 * It is possible for point to replace existing ones, if the quality is better. (Manual insertion,
 * reverse tracking,...)
 *
 * For multicolor, the color gets added as well. For Aruco, the code of TrackPoint and TrackPerson
 * gets synchronized.
 *
 * This function is used form manual insertions and from recognition.
 *
 * @param[in] point TrackPoint to add
 * @param[in] frame current frame (frame in which point was detected)
 * @param[in] onlyVisible set of selected persons, see Petrack::getPedestriansToTrack()
 * @param[out] pers person the point was added to; undefined when new trajectory was created
 * @return true if new trajectory was created; false otherwise
 */
bool Tracker::addPoint(
    TrackPoint &            point,
    int                     frame,
    const QSet<int> &       onlyVisible,
    reco::RecognitionMethod method,
    int *                   pers)
{
    bool  found = false;
    int   i, iNearest = 0.;
    float scaleHead;
    float dist, minDist = 1000000.;
    float z = -1;
#ifndef STEREO_DISABLED
    float x = -1, y = -1;

    // ACHTUNG: BORDER NICHT BEACHTET bei point.x()...
    // hier wird farbe nur bei reco bestimmt gegebenfalls auch beim tracken interessant
    // calculate height with disparity map
    if(mMainWindow->getStereoContext() && mMainWindow->getStereoWidget()->stereoUseForHeight->isChecked())
    {
        if(mMainWindow->getStereoContext()->getMedianXYZaround(
               (int) p.x(), (int) p.y(), &x, &y, &z)) // nicht myRound, da pixel 0 von 0..0.99 in double geht
        {
            // statt altitude koennte hier irgendwann die berechnete Bodenhoehe einfliessen
            p.setSp(x, y, z); // setZdistanceToCam(z);
        }
        // cout << " " << point.x()<< " " << point.y() << " " << x << " " << y << " " << z <<endl;
        // if (i == 10)
        //     debout << i << " " << mMainWindow->getControlWidget()->coordAltitude->value() - z << " " << z << " " <<
        //     (*this)[i].height() << endl;
    }
#endif
    // skalierungsfaktor fuer kopfgroesse
    // fuer multicolor marker groesser, da der schwarze punkt weit am rand liegen kann
    bool multiColorWithDot = false;
    if(method == reco::RecognitionMethod::MultiColor &&                 // multicolor marker
       mMainWindow->getMultiColorMarkerWidget()->useDot->isChecked() && // nutzung von black dot
       !mMainWindow->getMultiColorMarkerWidget()
            ->ignoreWithoutDot->isChecked()) // muetzen ohne black dot werden auch akzeptiert
    {
        multiColorWithDot = true;
        scaleHead         = 1.3f;
    }
    else
        scaleHead = 1.0f;

    for(i = 0; i < size(); ++i) // !found &&  // ueber TrackPerson
    {
        if(((onlyVisible.empty()) || (onlyVisible.contains(i))) && at(i).trackPointExist(frame))
        {
            dist = at(i).trackPointAt(frame).distanceToPoint(point);
            if((dist < scaleHead * mMainWindow->getHeadSize(nullptr, i, frame) / 2.) ||
               // fuer multifarbmarker mit schwarzem punkt wird nur farbmarker zur Abstandbetrachtung herangezogen
               // at(i).trackPointAt(frame).colPoint() existiert nicht an dieser stelle, da bisher nur getrackt
               // wurde!!!!
               (multiColorWithDot && point.color().isValid() &&
                (at(i).trackPointAt(frame).distanceToPoint(point.colPoint()) <
                 mMainWindow->getHeadSize(nullptr, i, frame) / 2.)))
            {
                if(found)
                {
                    debout << "Warning: more possible trackpoints for point" << std::endl;
                    debout << "         " << point << " in frame " << frame << " with low distance:" << std::endl;
                    debout << "         person " << i + 1 << " (distance: " << dist << "), " << std::endl;
                    debout << "         person " << iNearest + 1 << " (distance: " << minDist << "), " << std::endl;
                    if(minDist > dist)
                    {
                        minDist  = dist;
                        iNearest = i;
                    }
                }
                else
                {
                    minDist  = dist;
                    iNearest = i;
                    // WAR: break inner loop
                    found = true;
                }
            }
        }
    }
    if(found) // den naechstgelegenen nehmen
    {
        // test, if recognition point or tracked point is better is made in at(i).insertAtFrame
        if((*this)[iNearest].insertAtFrame(
               frame,
               point,
               iNearest,
               (mMainWindow->getControlWidget()->trackExtrapolation->checkState() ==
                Qt::Checked))) // wenn eingefuegt wurde (bessere qualitaet)
                               //|| !at(i).trackPointAt(frame).color().isValid() moeglich, um auch bei schlechterer
                               // qualitaet aber aktuell nicht
                               // vorliegender farbe die ermittelte farbe einzutragen - kommt nicht vor!
        {
            // Synchronize TrackPerson.markerID with TrackPoint.markerID
            (*this)[iNearest].syncTrackPersonMarkerID(point.getMarkerID());

            // set/add color
            if(point.color().isValid()) // not valid for manual, than old color is used
            {
                // if (at(i).trackPointAt(frame).color().isValid()) man koennte alte farbe abziehen - aber nicht noetig,
                // kommt nicht vor
                (*this)[iNearest].addColor(point.color());
            }
        }

        if(pers != nullptr)
            *pers = iNearest;

        (*this)[iNearest].setNewReco(true);
    }

    if((onlyVisible.empty()) && !found)
    {
        iNearest = size();

        if(point.qual() > 100) // manual add
            point.setQual(100);
        append(TrackPerson(
            0, frame, point, point.getMarkerID())); // 0 is person number/markerID; newReco is set to true by default
    }
    if((z > 0) && ((onlyVisible.empty()) || found))
        (*this)[iNearest].setHeight(z, mMainWindow->getControlWidget()->coordAltitude->value()); // , frame
    if((!onlyVisible.empty()) && !found)
    {
        QMessageBox::warning(
            nullptr,
            "PeTrack",
            "Adding a manual TrackPoint is only possible, when \"show only people\" and \"show only people list\" are "
            "disabled!\n"
            "You would not see the newly created TrackPoint otherwise.");
        debout << "Warning: No manual insertion, because not all trajectories are visible!" << std::endl;
        return false;
    }

    return !found;
}

// used from recognition
void Tracker::addPoints(QList<TrackPoint> &pL, int frame, reco::RecognitionMethod method)
{
    int i;

    // reset newReco
    for(i = 0; i < size(); ++i) // ueber TrackPerson
        (*this)[i].setNewReco(false);

    // ueberprufen ob identisch mit einem Punkt in liste
    for(i = 0; i < pL.size(); ++i) // ueber PointList
    {
        addPoint(pL[i], frame, QSet<int>(), method);
    }
}

int Tracker::visible(int frameNum)
{
    int i, anz = 0;
    for(i = 0; i < size(); ++i)
    {
        if(at(i).trackPointExist(frameNum))
            anz++;
    }
    return anz;
}

int Tracker::largestFirstFrame()
{
    int max = -1, i;
    for(i = 0; i < size(); ++i)
    {
        if(at(i).firstFrame() > max)
            max = at(i).firstFrame();
    }
    return max;
}
int Tracker::largestLastFrame()
{
    int max = -1, i;
    for(i = 0; i < size(); ++i)
    {
        if(at(i).lastFrame() > max)
            max = at(i).lastFrame();
    }
    return max;
}
int Tracker::smallestFirstFrame()
{
    int i, min = ((size() > 0) ? at(0).firstFrame() : -1);
    for(i = 1; i < size(); ++i)
    {
        if(at(i).firstFrame() < min)
            min = at(i).firstFrame();
    }
    return min;
}
int Tracker::smallestLastFrame()
{
    int i, min = ((size() > 0) ? at(0).lastFrame() : -1);
    for(i = 1; i < size(); ++i)
    {
        if(at(i).lastFrame() < min)
            min = at(i).lastFrame();
    }
    return min;
}

/**
 * @brief Tracker::calcPrevFeaturePoints calculates all featurePoints(Persons) from the "previous" frame
 *
 * @param prevFrame Number of previous frame (can be both, larger or smaller; forward or backwards)
 * @param rect ROI
 * @param frame current frame number
 * @param reTrack boolean saying if people should be retracked, when tracking was of low quality
 * @param reQual threshold for low quality in case of reTrack = true
 * @param borderSize
 * @param onlyVisible
 * @return number of feature points
 */
size_t Tracker::calcPrevFeaturePoints(
    int       prevFrame,
    cv::Rect &rect,
    int       frame,
    bool      reTrack,
    int       reQual,
    int       borderSize,
    QSet<int> onlyVisible)
{
    int j = -1;

    mPrevFeaturePoints.clear();
    mPrevFeaturePointsIdx.clear();

    if(prevFrame != -1)
    {
        for(int i = 0; i < size(); ++i)
        {
            if(!((onlyVisible.empty()) || (onlyVisible.contains(i))))
            {
                continue;
            }
            if(!at(i).trackPointExist(prevFrame))
            {
                continue;
            }

            /*
             * For retracking to occur, every point in the path from the last
             * recognized point to this point should have been tracked with higher qual.
             * This should eliminate cases in which wrong points are tracked confidently
             * and replace better ones, under the assumption we only get to a wrong point
             * due to a low-quality tracking earlier.
             */
            bool applyReTrack = reTrack;
            if(reTrack)
            {
                int dir = (frame - prevFrame); // direction of tracking - forward/backwards
                dir /= std::abs(dir);          // theoretically possible to omit MAX_STEP_TRACK frames
                constexpr int minQualReco = 90;
                for(int j = 0;
                    at(i).trackPointExist(frame + j * dir) && at(i).trackPointAt(frame + j * dir).qual() < minQualReco;
                    ++j)
                {
                    if(at(i).trackPointAt(frame + j * dir).qual() < reQual)
                    {
                        applyReTrack = false;
                        break;
                    }
                }
            }
            if(at(i).trackPointExist(frame) && !applyReTrack)
            {
                continue;
            }


            Vec2F prevPoint = at(i).at(prevFrame - at(i).firstFrame());
            prevPoint += Vec2F(borderSize, borderSize);
            cv::Point2f p2f = prevPoint.toPoint2f();
            if(rect.contains(p2f))
            {
                mPrevFeaturePoints.push_back(p2f);
                ++j;

                mPrevFeaturePointsIdx.push_back(i);
                if(j > MAX_COUNT - 2)
                {
                    debout << "Warning: reached maximal number of tracking point: " << MAX_COUNT << std::endl;
                    break; // for loop
                }
            }
        }
    }

    return mPrevFeaturePointsIdx.size();
}

// rueckgabewert gibt anzahl an, wieviele punkte akzeptiert wurden,
//   um listen verschmelzen zu lassen, neu hinzugefuegt wurden, existierende mgl verbessert haben
/**
 * @brief Tries to insert the featurepoints into the trajectories. Might merge.
 *
 * If the point is in the original image (without border) and/or has a small tracking error
 * and is in the picture, it will be added to a tajectory, by using insertAtFrame, which will
 * only accept the point, if the quality increases. If merging is activated, we search for
 * another point, less then one head size away from the current one and merge these, if they werent
 * more distant to each other in neighbouring frames.
 *
 * @see Tracker::tryMergeTrajectories
 * @see TrackPerson::insertAtFrame
 *
 * @param frame Current frame
 * @param count number of tracked people
 * @param img current image
 * @param borderSize
 * @param errorScale
 * @return
 */
int Tracker::insertFeaturePoints(int frame, size_t count, cv::Mat &img, int borderSize, float errorScale)
{
    int        inserted = 0;
    TrackPoint v;
    int        qual;
    bool       found;
    Vec2F      borderSize2F(-borderSize, -borderSize);
    int        dist = (borderSize > 9) ? borderSize :
                                         10; // abstand zum bildrand, ab wann warnung ueber trj verlust herausgeschrieben wird
    float      z    = -1;
    //    int borderColorGray = qGray(mMainWindow->getBorderFilter()->getBorderColR()->getValue(),
    //                                mMainWindow->getBorderFilter()->getBorderColG()->getValue(),
    //                                mMainWindow->getBorderFilter()->getBorderColB()->getValue());

    for(size_t i = 0; i < count; ++i)
    {
        if(mStatus[i])
        {
            v = Vec2F(mFeaturePoints.at(i).x, mFeaturePoints.at(i).y); // umwandlung nach TrackPoint bei "="

            // ausserhalb der groesse des originalbildes
            if((v.x() >= borderSize && v.y() >= borderSize && v.x() <= img.cols - 1 - borderSize &&
                v.y() <= img.rows - 1 - borderSize) ||
               (mTrackError[i] <
                errorScale * MAX_TRACK_ERROR)) // nur bei kleinem Fehler darf auch im Randbereich getrackt werden
            {
                // das Beschraenken auf die Bildgroesse ist reine sicherheitsmassnahme,
                // oft sind tracking paths auch ausserhalb des bildes noch gut,
                // aber beim tracken in die andere richtung kann es bei petrack probleme machen
                if(v.x() >= 0 && v.y() >= 0 && v.x() <= img.cols - 1 && v.y() <= img.rows - 1)
                {
                    // borderSize abziehen, da Trackerdaten am Rand des Originalbildes 0/0 ist
                    // set position relative to original image size
                    v += borderSize2F;

#ifndef STEREO_DISABLED
                    float x = -1, y = -1;
                    // ACHTUNG: BORDER NICHT BEACHTET bei point.x()...
                    // calculate height with disparity map
                    if(mMainWindow->getStereoContext() &&
                       mMainWindow->getStereoWidget()->stereoUseForHeight->isChecked())
                    {
                        mMainWindow->getStereoContext()->getMedianXYZaround((int) v.x(), (int) v.y(), &x, &y, &z);
                        {
                            v.setSp(x, y, z); // v.setZdistanceToCam(z);
                        }
                        //(*this)[i].setHeight(z, mMainWindow->getControlWidget()->coordAltitude->value(), frame);
                    }
#endif

                    // wenn bei punkten, die nicht am rand liegen, der fehler gross ist,
                    // wird geguckt, ob der sprung sich zur vorherigen richtung stark veraendert hat
                    // wenn sprung sehr unterschiedlich, wird lieber interpoliert oder stehen geblieben
                    // ist richtung ok, dann wird dunkelstes pixel gesucht
                    // (subpixel aufgrund von nachbarpixel)
                    // oder einfach bei schlechtem fehler mit groesserem winSize=30 den Problempunkt nochmal machen
                    // TODO Wird gerade eben nicht gemacht. Sollten wir???


                    // ueberpruefen, ob tracking ziel auf anderem tracking path landet, dann beide trackpaths
                    // verschmelzen lassen
                    found = false;
                    if(mMainWindow->getControlWidget()->trackMerge->checkState() ==
                       Qt::Checked) // wenn zusammengefuehrt=merge=verschmolzen werden soll
                    {
                        found = tryMergeTrajectories(v, i, frame);
                    }

                    // wenn keine verschmelzung erfolgte, versuchen trackpoint einzufuegen
                    if(!found)
                    {
                        qual = static_cast<int>(errorToQual(mTrackError[i]));
                        if(qual < 20)
                            qual = 20;
                        v.setQual(qual); // qual um 50, damit nur reco-kopf-ellipsen points nicht herauskegeln
                        // bei insertAtFrame wird qual beruecksichtigt, ob vorheiger besser
                        if((*this)[mPrevFeaturePointsIdx[i]].insertAtFrame(
                               frame,
                               v,
                               mPrevFeaturePointsIdx[i],
                               (mMainWindow->getControlWidget()->trackExtrapolation->checkState() == Qt::Checked)) &&
                           (z > 0))
                            (*this)[mPrevFeaturePointsIdx[i]].setHeight(
                                z, mMainWindow->getControlWidget()->coordAltitude->value()); // , frame
                    }

                    ++inserted;
                }
            }
        }
        else
        {
            if(v.x() >= dist && v.y() >= dist && v.x() <= img.cols - 1 - dist && v.y() <= img.rows - 1 - dist)
                debout << "Warning: Lost trajectory inside picture of person " << mPrevFeaturePointsIdx[i] + 1
                       << " at frame " << frame << "!" << std::endl;
        }
    }

    return inserted;
}

/**
 * @brief Tries to find a suitable other trajectory and merge
 *
 * @param v TrackPoint to be inserted
 * @param i Index in mFeaturePointsIdx and rest of point/person to be inserted
 * @param frame frame in which the point v was tracked
 * @return true if a suitable trajectory to merge with was found
 */
bool Tracker::tryMergeTrajectories(const TrackPoint &v, size_t i, int frame)
{
    int  deleteIndex;
    bool found = false;
    int  j;
    // nach trajektorie suchen, mit der eine verschmelzung erfolgen koennte
    for(j = 0; !found && j < size(); ++j) // ueber TrackPerson
    {
        if(j != mPrevFeaturePointsIdx[i] && at(j).trackPointExist(frame) &&
           (at(j).trackPointAt(frame).distanceToPoint(v) < mMainWindow->getHeadSize(nullptr, j, frame) / 2.))
        {
            // um ein fehltracking hin zu einer anderen Trajektorie nicht zum Verschmelzen dieser fuehren zu lassen
            // (die fehlerbehandlung durch interpolation wird in insertAtFrame durchgefuehrt)
            if(!((at(mPrevFeaturePointsIdx[i]).trackPointExist(frame - 1) &&
                  (at(mPrevFeaturePointsIdx[i]).trackPointAt(frame - 1).distanceToPoint(v) >
                   mMainWindow->getHeadSize(nullptr, mPrevFeaturePointsIdx[i], frame - 1) / 2.)) ||
                 (at(mPrevFeaturePointsIdx[i]).trackPointExist(frame + 1) &&
                  (at(mPrevFeaturePointsIdx[i]).trackPointAt(frame + 1).distanceToPoint(v) >
                   mMainWindow->getHeadSize(nullptr, mPrevFeaturePointsIdx[i], frame + 1) / 2.))))
            {
                if(at(j).firstFrame() < (*this)[mPrevFeaturePointsIdx[i]].firstFrame() &&
                   at(j).lastFrame() > (*this)[mPrevFeaturePointsIdx[i]].lastFrame())
                {
                    for(int k = 0; k < at(mPrevFeaturePointsIdx[i]).size(); ++k)
                    {
                        // bei insertAtFrame wird qual beruecksichtigt, ob vorheriger besser
                        (*this)[j].insertAtFrame(
                            at(mPrevFeaturePointsIdx[i]).firstFrame() + k,
                            at(mPrevFeaturePointsIdx[i]).at(k),
                            j,
                            (mMainWindow->getControlWidget()->trackExtrapolation->checkState() == Qt::Checked));
                    }
                    deleteIndex = mPrevFeaturePointsIdx[i];
                }
                else if(at(j).firstFrame() < (*this)[mPrevFeaturePointsIdx[i]].firstFrame())
                {
                    for(int k = at(j).size() - 1; k > -1; --k)
                    {
                        // bei insertAtFrame wird qual beruecksichtigt, ob vorheriger besser
                        (*this)[mPrevFeaturePointsIdx[i]].insertAtFrame(
                            at(j).firstFrame() + k,
                            at(j).at(k),
                            mPrevFeaturePointsIdx[i],
                            (mMainWindow->getControlWidget()->trackExtrapolation->checkState() == Qt::Checked));
                    }
                    deleteIndex = j;
                }
                else
                {
                    for(int k = 0; k < at(j).size(); ++k)
                    {
                        // bei insertAtFrame wird qual beruecksichtigt, ob vorheriger besser
                        (*this)[mPrevFeaturePointsIdx[i]].insertAtFrame(
                            at(j).firstFrame() + k,
                            at(j).at(k),
                            mPrevFeaturePointsIdx[i],
                            (mMainWindow->getControlWidget()->trackExtrapolation->checkState() == Qt::Checked));
                    }
                    deleteIndex = j;
                }
                removeAt(deleteIndex);

                // shift index of feature points
                for(size_t k = 0; k < mPrevFeaturePointsIdx.size(); ++k)
                    if(mPrevFeaturePointsIdx[k] > deleteIndex)
                        --mPrevFeaturePointsIdx[k];
                found = true;
            }
        }
    }

    return found;
}

// default: int winSize=10, int level=3
// winSize=3 ist genauer, aber kann auch leichter abgelenkt werden; winSize=30 ist robuster aber ungenauer
// level kann groesser gewaehlt werden, wenn winSize klein, macht aber keinen grossen unterschied; (0) waere ohne
// pyramide war , int winSize=10
/**
 * @brief Tracks points from the last frame in this (current) frame
 *
 * @param img Image of current frame
 * @param rect ROI in which tracking is executed
 * @param frame frame-number of the current frame
 * @param reTrack boolean saying if people should be retracked, when tracking was of low quality
 * @param reQual threshold for low quality in case of reTrack = true
 * @param borderSize
 * @param level level of Gauss-Pyramid that is used with Lucas-Kanade
 * @param onlyVisible Set of trajectories which should be evaluated; @see Petrack::getPedestriansToTrack
 * @param errorScaleExponent errorScale is 1.5^errorScaleExponent
 * @return Number of tracked points
 */
int Tracker::track(
    cv::Mat &               img,
    cv::Rect &              rect,
    int                     frame,
    bool                    reTrack,
    int                     reQual,
    int                     borderSize,
    reco::RecognitionMethod recoMethod,
    int                     level,
    QSet<int>               onlyVisible,
    int                     errorScaleExponent)
{
    QList<int> trjToDel;
    float      errorScale = pow(1.5, errorScaleExponent); // 0 waere neutral

    if(mGrey.empty())
    {
        debout << "ERROR: you have to initialize tracking before using tracker!" << std::endl;
        return -1;
    }

    if(img.empty())
    {
        debout << "ERROR: no NULL image allowed for tracking!" << std::endl;
        return -1;
    }

    if((mPrevFrame != -1) && (abs(frame - mPrevFrame) > MAX_STEP_TRACK))
        reset();

    if(abs(frame - mPrevFrame) == 0)
    {
        debout << "ERROR: Frame has not changed. There is nothing to track!" << std::endl;
        return -1;
    }

    if(img.channels() == 3)
    {
        cv::cvtColor(img, mGrey, cv::COLOR_BGR2GRAY);
    }
    else if(img.channels() == 1)
    {
        img.copyTo(mGrey);
    }
    else
    {
        debout << "Error: Wrong number of channels: " << img.channels() << std::endl;
        return -1;
    }

    size_t numOfPeopleToTrack =
        calcPrevFeaturePoints(mPrevFrame, rect, frame, reTrack, reQual, borderSize, onlyVisible);

    if(numOfPeopleToTrack > 0)
    {
        preCalculateImagePyramids(level);

        if(mPrevFrame != -1)
        {
            if(abs(frame - mPrevFrame) > MAX_STEP_TRACK)
                debout << "Warning: no tracking because of too many skipped frames (" << mPrevFrame << " to " << frame
                       << ")!" << std::endl;
            else if(abs(frame - mPrevFrame) > 1)
                debout << "Warning: linear interpolation of skipped frames which are not already tracked ("
                       << mPrevFrame << " to " << frame << ")." << std::endl; // will be done in insertFeaturePoints
        }

        trackFeaturePointsLK(level, mMainWindow->getControlWidget()->getAdaptiveLevel());

        // TODO Split up refineViaColorPointLK as well...
        refineViaColorPointLK(level, errorScale);

        BackgroundFilter *bgFilter = mMainWindow->getBackgroundFilter();
        // testen, ob Punkt im Vordergrund liegt, ansonsten, wenn nicht gerade zuvor detektiert, ganze trajektorie
        // loeschen (maximnale laenge ausserhalb ist somit 2 frames)
        if(bgFilter && bgFilter->getEnabled() && (mPrevFrame != -1)) // nur fuer den fall von bgSubtraction durchfuehren
        {
            useBackgroundFilter(trjToDel, bgFilter);
        }

        // (bei schlechten, aber noch ertraeglichem fehler in der naehe dunkelsten punkt suchen)
        // dieser ansatz kann dazu fuehren, dass bei starken helligkeitsunterschieden auf pappe zum schatten gewandert
        // wird!!!
        if(!mMainWindow->getStereoWidget()->stereoUseForReco->isChecked() &&
           ((recoMethod == reco::RecognitionMethod::Casern) ||
            (recoMethod ==
             reco::RecognitionMethod::Hermes))) // nicht benutzen, wenn ueber disparity der kopf gesucht wird und somit
                                                // kein marker vorhanden oder zumindest nicht am punkt lewigen muss
        {
            refineViaNearDarkPoint();
        }

        insertFeaturePoints(frame, numOfPeopleToTrack, img, borderSize, errorScale);
    }

    cv::swap(mPrevGrey, mGrey);

    mPrevFrame = frame;

    // delete vorher ausgewaehlte trj
    // ACHTUNG: einzige stelle in tracker, wo eine trj geloescht wird
    // trackNumberAll, trackShowOnlyNr werden nicht angepasst, dies wird aber am ende von petrack::updateimage gemacht
    for(int i = 0; i < trjToDel.size(); ++i) // ueber TrackPerson
    {
        removeAt(trjToDel[i]);
    }

    // numOfPeopleToTrack kann trotz nicht retrack > 0 sein auch bei alten pfaden
    // da am bildrand pfade keinen nachfolger haben und somit dort immer neu bestimmt werden!
    return static_cast<int>(numOfPeopleToTrack);
}

/**
 * @brief Calculates the image pyramids for Lucas-Kanade
 *
 * This functions calculates image pyramids together with the gradients for the
 * consumption by Lucas-Kanade. They are precomputed with the biggest winsize
 * so they have enough padding calcOpticalFlowPyrLK can use them for all used
 * winsizes
 *
 * @param level Maximum used level for Lucas-Kanade
 * @param numOfPeopleToTrack Number of people who are going to be tracked
 */
void Tracker::preCalculateImagePyramids(int level)
{
    int maxWinSize = 3;
    for(size_t i = 0; i < mPrevFeaturePointsIdx.size(); ++i)
    {
        int winSize = mMainWindow->winSize(nullptr, mPrevFeaturePointsIdx[i], mPrevFrame, 0);
        if(winSize > maxWinSize)
        {
            maxWinSize = winSize;
        }
    }

    cv::buildOpticalFlowPyramid(mPrevGrey, mPrevPyr, cv::Size(maxWinSize, maxWinSize), level);
    cv::buildOpticalFlowPyramid(mGrey, mCurrentPyr, cv::Size(maxWinSize, maxWinSize), level);
}


/**
 * @brief Tracks the mPrevFeaturePoints with Lucas-Kanade (optional adaptive pyramid level)
 *
 * This function tracks all points in mPrevFeaturePoints via Lucas-Kanade. Each person is
 * tracked with a different winSize according to the size of the head. Optionally the
 * pyramid level can lowered, if the person could not be properly tracked and adaptive is
 * set to true. This was added because from OpenCV 3 onwards the tracking failed in cases,
 * where it should work. A suspicion: Happens when there is no unique point in the following
 * frame, because of every pixel having the exact same grey level in the smalles pyramid scale.
 *
 * @param level Maximum pyramid level to track with
 * @param adaptive indicates if pyramid level should be lowered after unsuccessful tracking attempt
 */
void Tracker::trackFeaturePointsLK(int level, bool adaptive)
{
    const size_t numOfPeople = mPrevFeaturePointsIdx.size();
    mFeaturePoints.resize(numOfPeople);
    mStatus.resize(numOfPeople);
    mTrackError.resize(numOfPeople);
    std::vector<uchar> localStatus;
    std::vector<float> localTrackError;

    for(size_t i = 0; i < numOfPeople; ++i)
    {
        int l       = level;
        int winSize = 0;

        do
        {
            if(l < level)
            {
                debout << "Warning: try tracking person " << mPrevFeaturePointsIdx[i] << " with pyramid level " << l
                       << "!" << std::endl;
            }

            winSize = mMainWindow->winSize(nullptr, mPrevFeaturePointsIdx[i], mPrevFrame, l);
            if(winSize < MIN_WIN_SIZE)
            {
                winSize = MIN_WIN_SIZE;
                debout << "Warning: set search region to the minimum size of " << MIN_WIN_SIZE << " for person "
                       << mPrevFeaturePointsIdx[i] << "!" << std::endl;
            }

            std::vector<cv::Point2f> prevFeaturePoint{mPrevFeaturePoints[i]};
            std::vector<cv::Point2f> nextFeaturePoint{};
            localStatus.clear();
            localTrackError.clear();

            cv::calcOpticalFlowPyrLK(
                mPrevPyr,
                mCurrentPyr,
                prevFeaturePoint,
                nextFeaturePoint,
                localStatus,
                localTrackError,
                cv::Size(winSize, winSize),
                l,
                mTermCriteria);

            mFeaturePoints[i] = nextFeaturePoint[0];
            mTrackError[i]    = localTrackError[0] * 10.F / winSize;

        } while(adaptive && localStatus[0] == 0 && (l--) > 0);
        mStatus[i] = localStatus[0];
    }
}

/**
 * @brief Tries to track colorPoint when featurePoint has high error
 *
 * @param level Pyramidlevel to track with
 * @param numOfPeopleToTrack
 * @param errorScale Factor for highest tolerable tracking error
 */
void Tracker::refineViaColorPointLK(int level, float errorScale)
{
    int                      winSize;
    bool                     useColor = mMainWindow->getMultiColorMarkerWidget()->useColor->isChecked();
    std::vector<cv::Point2f> prevColorFeaturePoint, colorFeaturePoint;
    std::vector<uchar>       colorStatus;
    std::vector<float>       colorTrackError;

    for(size_t i = 0; i < mPrevFeaturePointsIdx.size(); ++i)
    {
        // wenn fehler zu gross, dann Farbmarkerelement nehmen // fuer multicolor marker / farbiger hut mit schwarzem
        // punkt
        if(useColor && mTrackError[i] > errorScale * 150.F &&
           at(mPrevFeaturePointsIdx[i]).at(mPrevFrame - at(mPrevFeaturePointsIdx[i]).firstFrame()).color().isValid())
        {
            float prevPointX = static_cast<float>(
                at(mPrevFeaturePointsIdx[i]).at(mPrevFrame - at(mPrevFeaturePointsIdx[i]).firstFrame()).colPoint().x());
            float prevPointY = static_cast<float>(
                at(mPrevFeaturePointsIdx[i]).at(mPrevFrame - at(mPrevFeaturePointsIdx[i]).firstFrame()).colPoint().y());
            prevColorFeaturePoint.push_back(cv::Point2f(prevPointX, prevPointY));
            winSize = mMainWindow->winSize(nullptr, mPrevFeaturePointsIdx[i], mPrevFrame, level);

            cv::calcOpticalFlowPyrLK(
                mPrevPyr,
                mCurrentPyr,
                prevColorFeaturePoint,
                colorFeaturePoint,
                colorStatus,
                colorTrackError,
                cv::Size(winSize, winSize),
                level,
                mTermCriteria);

            colorTrackError[i] = colorTrackError[i] * 10.F / winSize;

            if((colorStatus[i] == 1) && (colorTrackError[i] < errorScale * 50.F))
            {
                debout << "Warning: tracking color marker instead of structural marker of person "
                       << mPrevFeaturePointsIdx[i] + 1 << " at " << mFeaturePoints[i].x << " x " << mFeaturePoints[i].y
                       << " / error: " << mTrackError[i] << " / color error: " << colorTrackError[i] << std::endl;


                mFeaturePoints[i] = cv::Point2f(
                    mPrevFeaturePoints[i].x + (colorFeaturePoint[i].x - prevColorFeaturePoint[i].x),
                    mPrevFeaturePoints[i].x + (colorFeaturePoint[i].x - prevColorFeaturePoint[i].x));
                debout << "         resulting point: " << mFeaturePoints[i].x << " x " << mFeaturePoints[i].y
                       << std::endl;
                mTrackError[i] = colorTrackError[i];
            }
        }
    }
}

/**
 * @brief Counts consecutive TrackPoints in background and deletes trajectories with too many
 *
 * If a TrackPoint is inside the recognition ROI and in the background
 * a counter is incremented. Once this counter hits a user defined value
 * and if the user has enabled deletion of trajectories, the trajectory is
 * marked for deletion.
 *
 * @param trjToDel[out] trajectories marked for deletion
 * @param bgFilter[in] backgroundFilter, which determines if a point is in the bg
 */
void Tracker::useBackgroundFilter(QList<int> &trjToDel, BackgroundFilter *bgFilter)
{
    int        x, y;
    static int margin = 10; // rand am bild, ab dem trajectorie in den hintergrund laufen darf
    int        bS     = mMainWindow->getImageBorderSize();
    QRectF     rect   = mMainWindow->getRecoRoiItem()->rect();
    for(size_t i = 0; i < mPrevFeaturePointsIdx.size(); ++i)
    {
        x = myRound(mFeaturePoints[i].x - .5);
        y = myRound(mFeaturePoints[i].y - .5);

        // Rahmen, in dem nicht vordergrund pflicht, insbesondere am rechten rand!!!! es wird gruenes von hand
        // angelegtes bounding rect roi genutzt
        if((mStatus[i] == 1) && x >= MAX(margin, rect.x()) &&
           x <= MIN(mGrey.cols - 1 - 2 * bS - margin - 50, rect.x() + rect.width()) && y >= MAX(margin, rect.y()) &&
           y <= MIN(mGrey.rows - 1 - 2 * bS - margin, rect.y() + rect.height()))
        {
            if(!bgFilter->isForeground(x, y) && at(mPrevFeaturePointsIdx[i]).trackPointAt(mPrevFrame).qual() < 100)
            {
                if((mMainWindow->getControlWidget()->filterBgDeleteTrj->checkState() == Qt::Checked) &&
                   (at(mPrevFeaturePointsIdx[i]).nrInBg() >=
                    mMainWindow->getControlWidget()->filterBgDeleteNumber->value()))
                {
                    // nur zum loeschen vormerken und am ende der fkt loeschen, da sonst Seiteneffekte komplex
                    trjToDel += mPrevFeaturePointsIdx[i];
                    debout << "Warning: Delete trajectory " << mPrevFeaturePointsIdx[i] + 1
                           << " inside region of interest, because it laid outside foreground for "
                           << mMainWindow->getControlWidget()->filterBgDeleteNumber->value() << " successive frames!"
                           << std::endl;
                }
                else
                {
                    (*this)[mPrevFeaturePointsIdx[i]].setNrInBg(at(mPrevFeaturePointsIdx[i]).nrInBg() + 1);
                }
            }
            else // zaehler zuruecksetzen, der anzahl von getrackten Punkten im hintergrund zaehlt
            {
                (*this)[mPrevFeaturePointsIdx[i]].setNrInBg(0);
            }
        }
    }
}

/**
 * @brief Tracks a near dark point if error is (still) high
 *
 * This method is only used with the Casern and Hermes Markers, which both have
 * an black middle. The target of this method is to find this dark point in the
 * middle of the marker and track that instead of the feature point.
 *
 * @param numOfPeopleTracked Number of people who have been tracked
 */
void Tracker::refineViaNearDarkPoint()
{
    int x, y;
    for(size_t i = 0; i < mPrevFeaturePointsIdx.size(); ++i)
    {
        x = myRound(mFeaturePoints[i].x - .5);
        y = myRound(mFeaturePoints[i].y - .5);
        // der reine fehler ist leider kein alleinig gutes mass,
        // da in kontrastarmen regionen der angegebene fehler gering, aber das resultat haeufiger fehlerhaft ist
        // es waere daher schoen, wenn der fehler in abhaengigkeit von kontrast in umgebung skaliert wuerde
        // zb (max 0..255): normal 10..150 -> *1; klein 15..50 -> *3; gross 0..255 -> *.5
        if((mTrackError[i] > MAX_TRACK_ERROR) && (mStatus[i] == 1) && x >= 0 && x < mGrey.cols && y >= 0 &&
           y < mGrey.rows)
        {
            int regionSize = myRound(
                mMainWindow->getHeadSize(nullptr, mPrevFeaturePointsIdx[i], mPrevFrame) /
                10.); ///< size of searched region around point: -regionSize to regionSize
            int  xMin, xMax, yMin, yMax, xMin2, xMax2, yMin2, yMax2, darkest;
            bool markerInsideWhite = true;
            int  xDark = x, yDark = y;

            // trotz grau (img)->nChannels=3
            xMin = ((0 > (x - regionSize)) ? 0 : (x - regionSize));
            yMin = ((0 > (y - regionSize)) ? 0 : (y - regionSize));
            xMax = ((mGrey.cols < (x + regionSize + 1)) ? mGrey.cols : (x + regionSize + 1));
            yMax = ((mGrey.rows < (y + regionSize + 1)) ? mGrey.rows : (y + regionSize + 1));

            darkest = 255;
            for(int k = yMin; k < yMax; ++k)
            {
                for(int j = xMin; j < xMax; ++j)
                {
                    if(getValue(mGrey, j, k).value() < darkest)
                    {
                        darkest = getValue(mGrey, j, k).value();
                        xDark   = j;
                        yDark   = k;
                    }
                }
            }

            xMin2 = ((0 > (xDark - regionSize)) ? 0 : (xDark - regionSize));
            yMin2 = ((0 > (yDark - regionSize)) ? 0 : (yDark - regionSize));
            xMax2 = ((mGrey.cols < (xDark + regionSize + 1)) ? mGrey.cols : (xDark + regionSize + 1));
            yMax2 = ((mGrey.rows < (yDark + regionSize + 1)) ? mGrey.rows : (yDark + regionSize + 1));

            // suchbereich:
            //  ###
            // #   #
            // #   #
            // #   #
            //  ###
            for(int k = yMin2 + 1; k < yMax2 - 1; ++k)
            {
                if((getValue(mGrey, xMin2, k).value() <= darkest) || (getValue(mGrey, xMax2 - 1, k).value() <= darkest))
                {
                    markerInsideWhite = false;
                    break;
                }
            }
            if(markerInsideWhite)
                for(int j = xMin2 + 1; j < xMax2 - 1; ++j)
                {
                    if((getValue(mGrey, j, yMin2).value() <= darkest) ||
                       (getValue(mGrey, j, yMax2 - 1).value() <= darkest))
                    {
                        markerInsideWhite = false;
                        break;
                    }
                }

            if(markerInsideWhite)
            {
                mFeaturePoints[i].x = xDark;
                mFeaturePoints[i].y = yDark;
                debout << "Move trackpoint to darker pixel for" << i + 1 << "!" << std::endl;
            }

            // interpolation wg nachbargrauwerten:
            x = myRound(mFeaturePoints[i].x);
            y = myRound(mFeaturePoints[i].y);
            if((x > 0) && (x < (mGrey.cols - 1)) && (y > 0) && (y < (mGrey.rows - 1)) && (darkest < 255))
            {
                mFeaturePoints[i].x +=
                    .5 * ((double) (getValue(mGrey, x + 1, y).value() - getValue(mGrey, x - 1, y).value())) /
                    ((double) (255 - darkest));
                mFeaturePoints[i].y +=
                    .5 * ((double) (getValue(mGrey, x, y + 1).value() - getValue(mGrey, x, y - 1).value())) /
                    ((double) (255 - darkest));
            }

            mFeaturePoints[i].x += .5F;
            mFeaturePoints[i].y += .5F; // da 1. pixel von 0..1, das 2. pixel von 1..2 etc geht
        }
    }
}

void Tracker::recalcHeight(float altitude)
{
    // in TrackPerson: resetHeight();
    for(int i = 0; i < size(); ++i) // ueber TrackPerson
    {
        (*this)[i].recalcHeight(altitude);
    }
}

/**
 * @brief Performs different tests to check the plausibility of trajectories.
 *
 * This method can check for
 * <ul><li>shortness (less than 10 points)</li>
 * <li>start and endpoint (both should be outside the reco ROI
 * with exceptions for the beginning and end of the video)</li>
 * <li>Fast variations of speed (4 frame interval)</li>
 * <li>TrackPoints are too close together</li></ul>
 *
 * @param pers[in] list of persons (ID) to check
 * @param frame[out] list of frames at which "problems" occured
 * @param testEqual[in] true if warning for very close points are wished
 * @param testVelocity[in] true if warning for fast speed variations is whished
 * @param testInside[in] true if warning for start and endpoint in reco ROI is wished
 * @param testLength[in] true if warning for very short trajectories is wished
 */
void Tracker::checkPlausibility(
    QList<int> &pers,
    QList<int> &frame,
    bool        testEqual,
    bool        testVelocity,
    bool        testInside,
    bool        testLength)
{
    QProgressDialog progress("Check Plausibility", nullptr, 0, 400, mMainWindow->window());
    progress.setWindowTitle("Check plausibility");
    progress.setWindowModality(Qt::WindowModal);
    progress.setVisible(true);
    progress.setValue(0);
    progress.setLabelText("Check Plausibility...");
    static int margin = 30; // rand am bild, ab dem trajectorie verloren sein darf
    int        i, j;
    double     x, y;
    int        bS        = mMainWindow->getImageBorderSize();
    QRectF     rect      = mMainWindow->getRecoRoiItem()->rect();
    int        lastFrame = mMainWindow->getAnimation()->getNumFrames() - 1;

#ifdef TIME_MEASUREMENT
    double time1, tstart;
#endif
    // test, if the trajectory is very short (less than 10 Trackpoints)
    if(testLength)
    {
        progress.setValue(0);
        progress.setLabelText("Check trajectories lengths...");
        qApp->processEvents();
#ifdef TIME_MEASUREMENT
        time1  = 0.0;
        tstart = clock();
#endif
        for(i = 0; i < size(); ++i) // ueber TrackPerson
        {
            progress.setValue(i * 100. / size());
            qApp->processEvents();
            if(at(i).size() < 10)
            {
                debout << "Warning: Trajectory of person " << i + 1 << " has less than 10 trackpoints!" << std::endl;
                pers.append(i + 1);
                frame.append((*this)[i].firstFrame());
            }
        }
#ifdef TIME_MEASUREMENT
        time1 += clock() - tstart;
        time1 = time1 / CLOCKS_PER_SEC;
        cout << "  time(testLength) = " << time1 << " sec." << endl;
#endif
    }

    // check, if trajectory starts and ends outside the recognition area
    if(testInside)
    {
        progress.setValue(100);
        progress.setLabelText("Check if trajectories are inside image...");
        qApp->processEvents();
#ifdef TIME_MEASUREMENT
        time1  = 0.0;
        tstart = clock();
#endif
        for(i = 0; i < size(); ++i) // ueber TrackPerson
        {
            qApp->processEvents();
            progress.setValue(100 + i * 100. / size());
            x = (*this)[i].first().x();
            y = (*this)[i].first().y();
            // mGrey hat gleiche groesse wie zuletzt getracktes bild
            if((*this)[i].firstFrame() != 0 && x >= MAX(margin, rect.x()) && y >= MAX(margin, rect.y()) &&
               x <= MIN(mGrey.cols - 1 - 2 * bS - margin, rect.x() + rect.width()) &&
               y <= MIN(mGrey.rows - 1 - 2 * bS - margin, rect.y() + rect.height()))
            {
                debout << "Warning: Start of trajectory inside picture and recognition area of person " << i + 1 << "!"
                       << std::endl;
                pers.append(i + 1);
                frame.append((*this)[i].firstFrame());
            }

            x = (*this)[i].last().x();
            y = (*this)[i].last().y();
            // mGrey hat gleiche groesse wie zuletzt getracktes bild
            if((*this)[i].lastFrame() != lastFrame && x >= MAX(margin, rect.x()) && y >= MAX(margin, rect.y()) &&
               x <= MIN(mGrey.cols - 1 - 2 * bS - margin, rect.x() + rect.width()) &&
               y <= MIN(mGrey.rows - 1 - 2 * bS - margin, rect.y() + rect.height()))
            {
                debout << "Warning: End of trajectory inside picture and recognition area of person " << i + 1 << "!"
                       << std::endl;
                pers.append(i + 1);
                frame.append((*this)[i].lastFrame());
            }
        }
#ifdef TIME_MEASUREMENT
        time1 += clock() - tstart;
        time1 = time1 / CLOCKS_PER_SEC;
        cout << "  time(testInside) = " << time1 << " sec." << endl;
#endif
    }

    // testen, ob grosse Geschwindigkeitsaenderungen
    // statt distanz koennte man auch noch vektoren vergleichen, was genauere analyse waer!!!!
    if(testVelocity)
    {
        qApp->processEvents();
        progress.setValue(200);
        progress.setLabelText("Check velocity...");
#ifdef TIME_MEASUREMENT
        time1  = 0.0;
        tstart = clock();
#endif
        double d01, d12, d23;
        for(i = 0; i < size(); ++i) // ueber TrackPerson
        {
            qApp->processEvents();
            progress.setValue(200 + i * 100. / size());
            for(j = 1; j < at(i).size() - 2; ++j) // ueber TrackPoint (ohne ersten und letzten beiden)
            {
                d01 = at(i).at(j).distanceToPoint(at(i).at(j - 1));
                d12 = at(i).at(j + 1).distanceToPoint(at(i).at(j));
                d23 = at(i).at(j + 2).distanceToPoint(at(i).at(j + 1));
                if(((1.8 * (d01 + d23) / 2.) < d12) &&
                   ((d12 > 6.) ||
                    ((d01 + d23) / 2. > 3.))) // geschwindigkeit 1,8-fach && mindestpixelbewegung im schnitt von 3
                {
                    debout << "Warning: Fast variation of velocity of person " << i + 1 << " between frame "
                           << j + at(i).firstFrame() << " and " << j + 1 + at(i).firstFrame() << "!" << std::endl;
                    pers.append(i + 1);
                    frame.append(j + at(i).firstFrame());
                }
            }
        }
#ifdef TIME_MEASUREMENT
        time1 += clock() - tstart;
        time1 = time1 / CLOCKS_PER_SEC;
        cout << "  time(testVelocity) = " << time1 << " sec." << endl;
#endif
    }

    // testen, ob zwei trackpoint sehr nah beieinanderliegen (es gibt trajektorien, die uebereinander liegen, wenn nicht
    // genmergt wird)
    if(testEqual)
    {
        progress.setValue(300);
        progress.setLabelText("Check if trajectories are equal...");
        qApp->processEvents();
#ifdef TIME_MEASUREMENT
        time1  = 0.0;
        tstart = clock();
#endif
        int lLF = largestLastFrame();
        int f;
        for(f = smallestFirstFrame(); f <= lLF; ++f)
        {
            progress.setValue(300 + f * 100. / lLF);
            qApp->processEvents();

            for(i = 0; i < size(); ++i)
            {
                // if (!pers.contains(i+1)) man koennte nur einmal eine Person aufnehmen, da aufeinanderfolgende frames
                // oft betroffen
                for(j = i + 1; j < size(); ++j)
                {
                    if(at(i).trackPointExist(f) && at(j).trackPointExist(f))
                    {
                        if(at(i).trackPointAt(f).distanceToPoint(at(j).trackPointAt(f)) <
                           mMainWindow->getHeadSize(nullptr, i, f) / 2.)
                        {
                            debout << "Warning: Person " << i + 1 << " and " << j + 1
                                   << " are very close to each other at frame " << f << "!" << std::endl;
                            pers.append(i + 1);
                            frame.append(f);
                        }
                    }
                }
            }
        }
#ifdef TIME_MEASUREMENT
        time1 += clock() - tstart;
        time1 = time1 / CLOCKS_PER_SEC;
        cout << "  time(testEqual) = " << time1 << " sec." << endl;
#endif
    }
}

// color optimieren fuer alle personen
void Tracker::optimizeColor()
{
    int i;
    for(i = 0; i < size(); ++i) // ueber TrackPerson
    {
        if((*this)[i].color().isValid())
            (*this)[i].optimizeColor();
    }
}

// reset the height of all persons, but not the pos of the trackpoints
void Tracker::resetHeight()
{
    for(int i = 0; i < size(); ++i) // ueber TrackPerson
    {
        (*this)[i].resetHeight();
    }
}

// reset the pos of the tzrackpoints, but not the heights
void Tracker::resetPos()
{
    for(int i = 0; i < size(); ++i) // ueber TrackPerson
    {
        for(int j = 0; j < (*this)[i].size(); ++j) // ueber TrackPoints
        {
            (*this)[i][j].setSp(-1., -1., -1.);
        }
    }
}

/**
 * @brief Prints height distribution to stdout
 *
 * @return false if no height information is available, else true
 */
bool Tracker::printHeightDistribution()
{
    debout << std::endl;
    QMap<double, int>                 dict;
    QMap<double, int>::const_iterator j;
    int                               i, anz = 0;
    int                               heightStep = 5;
    double                            average = 0., avg = 0.;
    int                               noHeight = 0;

    for(i = 0; i < size(); ++i)
    {
        if((*this)[i].height() >
           MIN_HEIGHT) // !=-1// insbesondere von hand eingefuegte trackpoint/persons haben keine farbe
        {
            ++dict[(((int) (*this)[i].height()) / heightStep) * heightStep];
            avg += (*this)[i].height();
        }
        else
            ++noHeight;
    }
    j = dict.constBegin();
    while(j != dict.constEnd())
    {
        anz += j.value();
        ++j;
    }
    debout << "number of persons with measured height                            : " << anz << std::endl;
    debout << "person without measured height (not included in calculated values): " << noHeight
           << " (using default height for export)" << std::endl;
    if(anz == 0)
        return false;
    j = dict.constBegin();
    while(j != dict.constEnd())
    {
        debout << "height " << std::fixed << std::setprecision(1) << std::setw(5) << j.key() << " - "
               << j.key() + heightStep << " : number " << std::setw(3) << j.value() << " (" << std::setw(4)
               << (100. * j.value()) / anz << "%)" << std::endl;
        average += (j.key() + heightStep / 2.) * j.value();
        ++j;
    }
    debout << "average height (bucket): " << std::fixed << std::setprecision(1) << std::setw(5) << average / anz
           << std::endl;
    debout << "average height         : " << std::fixed << std::setprecision(1) << std::setw(5) << avg / anz
           << std::endl;

    return true;
}

/**
 * Sets the heights based on the values contained in \p heights.
 * @param heights Map between marker ID and corresponding height
 */
void Tracker::setMarkerHeights(const std::unordered_map<int, float> &heights)
{
    for(int i = 0; i < size(); ++i) // over TrackPerson
    {
        for(int j = 0; j < (*this)[i].size(); ++j) // over TrackPoints
        {
            // markerID of current person at current TrackPoint:
            int markerID = (*this)[i][j].getMarkerID();

            if(markerID != -1) // when a real markerID is found (not -1)
            {
                // find index of mID within List of MarkerIDs that were read from txt-file:
                if(heights.find(markerID) != std::end(heights))
                {
                    (*this)[i].setHeight(heights.at(markerID));
                }
                else
                {
                    debout << "Warning, the following markerID was not part of the height-file: " << markerID
                           << std::endl;
                    debout << "No height set for personNR: " << (*this)[i].nr() << std::endl;
                }
            }
        }
    }
}

/**
 * Sets the marker IDs based on the internal used IDs (personID).
 * @param markerIDs Map between internal ID and marker ID
 */
void Tracker::setMarkerIDs(const std::unordered_map<int, int> &markerIDs)
{
    for(int i = 0; i < size(); ++i) // over TrackPerson
    {
        // personID of current person
        int personID = i + 1;
        if(markerIDs.find(personID) != std::end(markerIDs))
        {
            int markerID = markerIDs.at(personID);
            (*this)[i].setMarkerID(markerID);
            for(int j = 0; j < (*this)[i].size(); ++j) // over TrackPoints
            {
                (*this)[i][j].setMarkerID(markerID);
            }
        }
        else
        {
            debout << "Warning, the following personID was not part of the markerID-file: " << personID << std::endl;
        }
    }
}

/**
 * @brief Deletes TrackPersons with over 80% solely tracked points
 *
 * DOESN'T WORK WITH COLOR MARKERS because they have a quality under
 * 100 (quality threshold of this method could be changed)
 *
 * Only trajectories having a point at the given frame are purged.
 * Trajectories with less than 10 points are not purged.
 *
 * @param frame frame at which all trajectories should be purged
 */
void Tracker::purge(int frame)
{
    int   i, j;
    float count; ///< number of trackpoints without recognition

    for(i = 0; i < size(); ++i)
    {
        if(at(i).size() > 10 && at(i).firstFrame() <= frame && at(i).lastFrame() >= frame)
        {
            count = 0;
            for(j = 0; j < at(i).size(); ++j)
            {
                if(at(i).at(j).qual() < 100.)
                    ++count;
            }
            if(count / at(i).size() > 0.8) // Achtung, wenn roi klein, dann viele tp nur getrackt
                removeAt(i);               // delete trj
        }
    }
}

/**
 * @brief TrackPerson::syncTrackPersonMarkerID Synchronize TrackPoint.mMarkerID with TrackPerson.mMarkerID
 *
 * 1. Function sets PersonMarkerID from TrackPointMarkerID if MarkerID == -1
 * 2. checks if not other ID was detected and triggers a warning otherwise
 *
 * @param markerID integer representing ArucoCodeMarker for currently handled TrackPoint
 */
void TrackPerson::syncTrackPersonMarkerID(int markerID)
{
    int tpMarkerID = markerID; // MarkerID of currently handled trackpoint

    if(tpMarkerID != -1) // CodeMarker was recognized
    {
        if(mMarkerID == -1) // first time a Person is found TrackPerson.mMarkerID is -1 by initialisation
        {
            setMarkerID(tpMarkerID); // set TrackPerson MarkerID equal to TrackPoint MarkerID
        }
        if(mMarkerID != tpMarkerID)
        {
            std::cout << "ERROR: Two MarkerIDs were found for one trajectory." << std::endl;
        }
    }
}
