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

#include "tracker.h"

#include "animation.h"
#include "control.h"
#include "helper.h"
#include "logger.h"
#include "multiColorMarkerWidget.h"
#include "pMessageBox.h"
#include "personStorage.h"
#include "petrack.h"
#include "roiItem.h"
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


//--------------------------------------------------------------------------
TrackPerson::TrackPerson(int nr, int frame, const TrackPoint &p) :
    mNr(nr),
    mMarkerID(-1),
    mHeight(MIN_HEIGHT),
    mHeightCount(0),
    mFirstFrame(frame),
    mNewReco(true),
    mComment(),
    mColorCount(1)
{
    if(auto color = p.getColorForHeightMap())
    {
        mColor = *color;
    }
    mData.append(p);
}

TrackPerson::TrackPerson(int nr, int frame, const TrackPoint &p, int markerID) :
    mNr(nr),
    mMarkerID(markerID),
    mHeight(MIN_HEIGHT),
    mHeightCount(0),
    mFirstFrame(frame),
    mNewReco(true),
    mComment(),
    mColorCount(1)
{
    if(auto color = p.getColorForHeightMap())
    {
        mColor = *color;
    }
    mData.append(p);
}

/**
 * @brief Computes the average over all recognized TrackPoint to one color of the TrackPerson
 * @param col newly added color
 */
void TrackPerson::addColor(const QColor &col)
{
    mColor.setRed((mColorCount * mColor.red() + col.red()) / (mColorCount + 1));
    mColor.setBlue((mColorCount * mColor.blue() + col.blue()) / (mColorCount + 1));
    mColor.setGreen((mColorCount * mColor.green() + col.green()) / (mColorCount + 1));

    ++mColorCount;
}

/**
 * @brief optimize average color for casern marker
 */
void TrackPerson::optimizeColor()
{
    // ausreisser herausnehmen ueber die koordinate der farbe
    int   i, j;
    Vec2F v, vBefore;
    int   anz1 = 0, anz2 = 0;
    bool  swap = false;

    // den ersten farbpunkt suchen und vBefore initial setzen
    for(i = 0; i < mData.size(); ++i)
    {
        if(auto marker = mData.at(i).getCasernMarker())
        {
            vBefore = marker->mColorPoint - mData.at(i).pixelPoint();
            break;
        }
    }
    if(mData.at(i).getCasernMarker())
    {
        ++anz1;
    }
    // testen, auf welcher seit der farbmarker haeufiger gesehen wird
    for(j = i + 1; j < mData.size(); ++j)
    {
        if(auto marker = mData.at(j).getCasernMarker())
        {
            auto colorPoint = marker->mColorPoint;
            v               = colorPoint - mData.at(j).pixelPoint();
            if((v * vBefore) < 0)
            {
                swap = !swap;
            }

            if(swap)
            {
                ++anz2;
            }
            else
            {
                ++anz1;
            }
            vBefore = v;
        }
    }
    swap = false;
    if(auto colorPoint = mData.at(i).getColorPointForOrientation())
    {
        vBefore = *colorPoint - mData.at(i).pixelPoint();
    }
    // farben mit geringerer anzahl loeschen
    if(anz2 > anz1)
    {
        mData[i].deleteColorMarkers();
    }
    for(j = i + 1; j < mData.size(); ++j)
    {
        if(auto marker = mData.at(j).getCasernMarker())
        {
            auto colorPoint = marker->mColorPoint;
            v               = colorPoint - mData.at(j).pixelPoint();
            if((v * vBefore) < 0)
            {
                swap = !swap;
            }

            if(swap)
            {
                if(anz1 > anz2)
                {
                    mData[j].deleteColorMarkers();
                }
            }
            else
            {
                if(anz2 > anz1)
                {
                    mData[j].deleteColorMarkers();
                }
            }
            vBefore = v;
        }
    }

    // median statt mittelwert nehmen
    QList<int> r;
    QList<int> g;
    QList<int> b;
    for(i = 0; i < mData.size(); ++i)
    {
        if(auto marker = mData.at(i).getCasernMarker())
        {
            r.append(marker->mColor.red());
            g.append(marker->mColor.green());
            b.append(marker->mColor.blue());
        }
    }
    std::sort(r.begin(), r.end());
    std::sort(g.begin(), g.end());
    std::sort(b.begin(), b.end());
    if(r.size() > 0 && g.size() > 0 && b.size() > 0)
    {
        setColor(QColor(r[(r.size() - 1) / 2], g[(g.size() - 1) / 2], b[(b.size() - 1) / 2]));
    }
}

void TrackPerson::recalcHeight(float altitude)
{
    // median statt mittelwert nehmen (bei gerader anzahl an werten den kleiner als mitte)
    QList<double> zList;

    resetHeight();

    for(const auto &trackPoint : mData)
    {
        if(auto stereoMarker = trackPoint.getStereoMarker())
        {
            ++mHeightCount;
            // h += z;
            zList.append(stereoMarker->mStereoPoint.z());
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
double TrackPerson::getNearestZ(int i, int *extrapolated) const
{
    *extrapolated = 0;
    if((i < 0) || (i >= mData.size())) // indexueberpruefung
    {
        return -1.;
    }
    if(auto stereoMarker = mData.at(i).getStereoMarker())
    {
        return stereoMarker->mStereoPoint.z();
    }
    else // -1 an aktueller hoehe
    {
        int nrFor = 1;
        int nrRew = 1;
        while((i + nrFor < mData.size()) &&
              (!mData.at(i + nrFor)
                    .getStereoMarker())) // nach && wird nur ausgefuehrt, wenn erstes true == size() also nicht
        {
            ++nrFor;
        }
        while((i - nrRew >= 0) &&
              (!mData.at(i - nrRew)
                    .getStereoMarker())) // nach && wird nur ausgefuehrt, wenn erstes true == size() also nicht
        {
            ++nrRew;
        }
        if((i + nrFor == mData.size()) && (i - nrRew < 0)) // gar keine Hoeheninfo in trj gefunden
        {
            return -1.;
        }
        else if(i + nrFor == mData.size()) // nur in Vergangenheit hoeheninfo gefunden
        {
            *extrapolated = 2;
            return mData.at(i - nrRew).stereoGetStereoPoint().z();
        }
        else if(i - nrRew < 0) // nur in der zukunft hoeheninfo gefunden
        {
            *extrapolated = 1;
            return mData.at(i + nrFor).stereoGetStereoPoint().z();
        }
        else // in beiden richtungen hoeheninfo gefunden - INTERPOLATION, NICHT EXTRAPOLATION
        {
            return mData.at(i - nrRew).stereoGetStereoPoint().z() +
                   nrRew *
                       (mData.at(i + nrFor).stereoGetStereoPoint().z() -
                        mData.at(i - nrRew).stereoGetStereoPoint().z()) /
                       (nrFor + nrRew); // lineare interpolation
        }
    }
}

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
 * @param frame frame to insert the point
 * @param point TrackPoint to add
 * @param persNr number shown to the user, for more meaningful warnings
 * @param extrapolate extrapolate with huge differences
 * @return true if point was added
 */
bool TrackPerson::insertAtFrame(int frame, const TrackPoint &point, int persNr, bool extrapolate)
{
    Vec2F      tmp; // ua. zur linearen Interpolation
    TrackPoint tp;  // default: 0 = ist schlechteste qualitaet
    double     distance;

    if(frame > lastFrame())
    {
        // lineare interpolation, wenn frames uebersprungen wurden
        if(frame - lastFrame() - 1 > 0)
        {
            tmp.setX((point.x() - mData.last().x()) / (frame - lastFrame()));
            tmp.setY((point.y() - mData.last().y()) / (frame - lastFrame()));
            tp = mData.last();
            tp.setQual(0);
            for(int i = lastFrame() + 1; i <= frame; ++i)
            {
                tp += tmp;
                mData.append(tp);
            }
        }
        else if(extrapolate && ((lastFrame() - mFirstFrame) > 0)) // mind. 2 trackpoints sind in liste!
        {
            tmp = mData.last().pixelPoint() -
                  mData.at(mData.size() - 2).pixelPoint(); // vektor zw letztem und vorletztem pkt
            // der Abstand zum extrapoliertem Ziel darf nicht groesser als 2x so gross sein, wie die entfernung
            // vorheriger trackpoints eine mindestbewegung wird vorausgesetzt, da sonst bewegungen aus dem "stillstand"
            // immer als fehler angesehen werden
            //   am besten waere eine fehlerbetrachtung in abhaengigkeit von der geschwindigkeit - nicht gemacht!
            if(((distance = ((mData.last() + tmp).distanceToPoint(point))) > EXTRAPOLATE_FACTOR * tmp.length()) &&
               (distance > 3))
            {
                if(!((mData.last().qual() == 0) &&
                     (mData.at(mData.size() - 2).qual() ==
                      0))) // das vorherige einfuegen ist 2x nicht auch schon schlecht gewesen
                {
                    tp = point;
                    SPDLOG_WARN(
                        "Extrapolation instead of tracking because of big difference from tracked point in speed and "
                        "direction of person {} between frame {} and {}!",
                        persNr + 1,
                        lastFrame(),
                        lastFrame() + 1);
                    tp = mData.last() + tmp; // nur vektor wird hier durch + geaendert
                    tp.setQual(0);
                    // im anschluss koennte noch dunkelster pkt in umgebung gesucht werden!!!
                    // keine Extrapolation der Groesse
                    mData.append(tp);
                }

                else
                {
                    SPDLOG_WARN(
                        "Because of three big differences from tracked point in speed and direction between last three "
                        "frames the track point of person {} at {} was NOT inserted!",
                        persNr + 1,
                        lastFrame() + 1);
                    return false;
                }
            }

            else
            {
                mData.append(point);
            }
        }
        else
        {
            mData.append(point);
        }
    }
    else if(frame < mFirstFrame)
    {
        if(mFirstFrame - frame - 1 > 0)
        {
            tmp.setX((point.x() - mData.first().x()) / (mFirstFrame - frame));
            tmp.setY((point.y() - mData.first().y()) / (mFirstFrame - frame));
            tp = mData.first();
            tp.setQual(0);
            for(int i = firstFrame() - 1; i >= frame; --i)
            {
                tp += tmp;
                mData.prepend(tp);
            }
        }
        else if(extrapolate && ((lastFrame() - mFirstFrame) > 0)) // mind. 2 trackpoints sind in liste!
        {
            tmp = mData.at(0).pixelPoint() - mData.at(1).pixelPoint(); // vektor zw letztem und vorletztem pkt
            // der Abstand zum extrapoliertem Ziel darf nicht groesser als 2x so gross sein, wie die entfernung
            // vorheriger trackpoints eine mindestbewegung wird vorausgesetzt, da sonst bewegungen aus dem "stillstand"
            // immer als fehler angesehen werden
            //   am besten waere eine fehlerbetrachtung in abhaengigkeit von der geschwindigkeit - nicht gemacht!
            if(((distance = (mData.at(0).pixelPoint() + tmp).distanceToPoint(point.pixelPoint())) >
                EXTRAPOLATE_FACTOR * tmp.length()) &&
               (distance > 3))
            {
                if(!((mData.at(0).qual() == 0) &&
                     (mData.at(1).qual() == 0))) // das vorherige einfuegen ist 2x nicht auch schon schlecht gewesen
                {
                    tp = point;
                    SPDLOG_WARN(
                        "Extrapolation instead of tracking because of big difference from tracked point in speed and "
                        "direction of person {} between frame {} and {}!",
                        persNr + 1,
                        lastFrame(),
                        lastFrame() - 1);
                    tp = mData.at(0) + tmp; // nur vektor wird hier durch + geaendert
                    tp.setQual(0);
                    mData.prepend(tp);
                }
                else
                {
                    SPDLOG_WARN(
                        "Because of three big differences from tracked point in speed and direction between last three "
                        "frames the track point of person {} at {} was NOT inserted!",
                        persNr + 1,
                        lastFrame() - 1);
                    return false;
                }
            }
            else
            {
                mData.prepend(point);
            }
        }
        else
        {
            mData.prepend(point);
        }
        mFirstFrame = frame;
    }
    else
    {
        // dieser Zweig wird insbesondere von reco durchlaufen, da vorher immer auch noch getrackt wird und reco
        // draufgesetzt wird!!!

        tp = point;
        if(point.qual() < TrackPoint::BEST_DETECTION_QUAL &&
           point.isDetection()) // erkannte Person aber ohne strukturmarker
        {
            // wenn in angrenzenden Frames qual groesse 90 (100 oder durch vorheriges verschieben entstanden), dann
            // verschieben
            if(trackPointExist(frame - 1) && trackPointAt(frame - 1).qual() > 90)
            {
                tp.setQual(95);
                tmp = point.pixelPoint() +
                      (trackPointAt(frame - 1).pixelPoint() - *trackPointAt(frame - 1).getColorPointForOrientation());
                tp.setX(tmp.x());
                tp.setY(tmp.y());
                SPDLOG_WARN(
                    "move TrackPoint according to last distance of structure marker and color marker of person {}:",
                    persNr + 1);
                SPDLOG_WARN("         {} -> {}", point, tp);
            }
            else if(trackPointExist(frame + 1) && trackPointAt(frame + 1).qual() > 90)
            {
                tp.setQual(95);
                tmp = point.pixelPoint() +
                      (trackPointAt(frame + 1).pixelPoint() - *trackPointAt(frame + 1).getColorPointForOrientation());
                tp.setX(tmp.x());
                tp.setY(tmp.y());
                SPDLOG_WARN(
                    "move TrackPoint according to last distance of structure marker and color marker of person {}:",
                    persNr + 1);
                SPDLOG_WARN("         {} -> {}", point, tp);
            }
        }

        // ueberprueft, welcher punkt besser
        if(tp.qual() > mData.at(frame - mFirstFrame).qual())
        {
            // warnung ausgeben, wenn replacement (fuer gewoehnlich von reco) den pfadverlauf abrupt aendert
            if(trackPointExist(frame - 1))
            {
                tmp = trackPointAt(frame).pixelPoint() - trackPointAt(frame - 1).pixelPoint();
            }
            else if(trackPointExist(frame + 1))
            {
                tmp = trackPointAt(frame + 1).pixelPoint() - trackPointAt(frame).pixelPoint();
            }
            if((trackPointExist(frame - 1) || trackPointExist(frame + 1)) &&
               ((distance = (trackPointAt(frame).distanceToPoint(tp.pixelPoint()))) > 1.5 * tmp.length()) &&
               (distance > 3))
            {
                int anz;
                SPDLOG_WARN(
                    "Big difference in location between existing and replacing track point of person {} in frame {}!",
                    persNr + 1,
                    frame);
                // qualitaet anpassen, da der weg zum pkt nicht der richtige gewesen sein kann
                // zurueck
                anz = 1;
                while(trackPointExist(frame - anz) && !trackPointAt(frame - anz).isDetection())
                {
                    ++anz;
                }
                for(int i = 1; i < (anz - 1);
                    ++i) // anz ist einer zu viel; zudem nur boie anz-1 , da sonst eh nur mit 1 multipliziert wuerde
                {
                    mData[frame - mFirstFrame - i].setQual((i * trackPointAt(frame - i).qual()) / anz);
                }
                // vor
                anz = 1;
                while(trackPointExist(frame + anz) && !trackPointAt(frame + anz).isDetection())
                {
                    ++anz;
                }
                for(int i = 1; i < (anz - 1);
                    ++i) // anz ist einer zu viel; zudem nur boie anz-1 , da sonst eh nur mit 1 multipliziert wuerde
                {
                    mData[frame - mFirstFrame + i].setQual((i * trackPointAt(frame + i).qual()) / anz);
                }
            }

            mData.replace(frame - mFirstFrame, tp);

            if(tp.qual() > TrackPoint::BEST_DETECTION_QUAL) // manual add // after inserting, because point ist const
            {
                mData[frame - mFirstFrame].setQual(TrackPoint::BEST_DETECTION_QUAL); // so moving of a point is possible
            }
        }
        else
        {
            return false;
        }
    }
    return true;
}

/**
 * Checks, if a TrackPoint for the frame exist
 * @param frame frame to check
 * @return TrackPoint exists at that frame
 */
bool TrackPerson::trackPointExist(int frame) const
{
    return (frame >= mFirstFrame && frame <= lastFrame());
}

/**
 * Get the TrackPoint at a specific frame
 * @param frame frame to get TrackPoint
 * @return TrackPoint at frame
 */
const TrackPoint &TrackPerson::trackPointAt(int frame) const
{
    return mData.at(frame - mFirstFrame);
}

/**
 * @brief Absolute distance to next frame
 *
 * @param frame
 * @return absolute distance or -1, if frame doesn't exist
 */
double TrackPerson::distanceToNextFrame(int frame) const
{
    if(trackPointExist(frame))
    {
        return trackPointAt(frame).distanceToPoint(at(frame - mFirstFrame + 1));
    }
    else
    {
        return -1;
    }
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

Tracker::Tracker(QWidget *wParent, PersonStorage &storage) : mPersonStorage(storage)
{
    mMainWindow = (class Petrack *) wParent;
    mTermCriteria =
        cv::TermCriteria(cv::TermCriteria::COUNT | cv::TermCriteria::EPS, 20, 0.03); ///< maxIter=20 and epsilon=0.03

    reset();
}


// neben loeschen der liste muessen auch ...
void Tracker::init(cv::Size size)
{
    mPersonStorage.clear(); // loescht liste aller getrackten personen
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
            {
                cv::copyMakeBorder(mPrevGrey, mPrevGrey, s, s, s, s, cv::BORDER_CONSTANT, cv::Scalar(0));
            }
            else
            {
                mPrevGrey = mPrevGrey(cv::Rect(-s, -s, mPrevGrey.cols + 2 * s, mPrevGrey.rows + 2 * s));
            }
        }
    }
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
    int          prevFrame,
    cv::Rect    &rect,
    int          frame,
    bool         reTrack,
    int          reQual,
    int          borderSize,
    QSet<size_t> onlyVisible)
{
    int j = -1;

    mPrevFeaturePoints.clear();
    mPrevFeaturePointsIdx.clear();

    if(prevFrame != -1)
    {
        const auto &persons = mPersonStorage.getPersons();
        for(int i = 0; i < static_cast<int>(mPersonStorage.nbPersons()); ++i)
        {
            const auto &person = persons[i];
            if(!((onlyVisible.empty()) || (onlyVisible.contains(i))))
            {
                continue;
            }
            if(!person.trackPointExist(prevFrame))
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
                for(int j = 0; person.trackPointExist(frame + j * dir) &&
                               person.trackPointAt(frame + j * dir).qual() < minQualReco;
                    ++j)
                {
                    if(person.trackPointAt(frame + j * dir).qual() < reQual)
                    {
                        applyReTrack = false;
                        break;
                    }
                }
            }
            if(person.trackPointExist(frame) && !applyReTrack)
            {
                continue;
            }


            Vec2F prevPoint = person.at(prevFrame - person.firstFrame()).pixelPoint();
            prevPoint += Vec2F(borderSize, borderSize);
            cv::Point2f p2f = prevPoint.toPoint2f();
            if(rect.contains(p2f))
            {
                mPrevFeaturePoints.push_back(p2f);
                ++j;

                mPrevFeaturePointsIdx.push_back(i);
                if(j > MAX_COUNT - 2)
                {
                    SPDLOG_WARN("reached maximal number of tracking point: {}", MAX_COUNT);
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
int Tracker::insertFeaturePoints(int frame, size_t count, cv::Mat &img, int borderSize, cv::Mat map1, float errorScale)
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
        if(mStatus[i] == TrackStatus::Tracked)
        {
            v = Vec2F(mFeaturePoints.at(i).x, mFeaturePoints.at(i).y); // umwandlung nach TrackPoint bei "="

            cv::Rect    rect(cv::Point2f(), map1.size());
            cv::Point2f p(v.x(), v.y());

            /*
             * Check if the map1 contains the current track point, because otherwise the point is outside of the
             * original image and it is not necessary to go ahead with this point.
             */
            if(rect.contains(p))
            {
                cv::Vec2s vOriginal        = map1.at<cv::Vec2s>(v.y(), v.x());
                short     sx               = vOriginal[0];
                short     sy               = vOriginal[1];
                int       headsize         = mMainWindow->getHeadSize();
                float     distanceToBorder = headsize / 5.;

                /*
                 * To avoid problems at the image border, persons should only be tracked up to a certain distance to the
                 * image border. So it is checked, if the current track point is to close to a border of the original
                 * distorted image.
                 */
                if(sy - borderSize >= distanceToBorder && sx - borderSize >= distanceToBorder &&
                   sy + borderSize <= img.rows - distanceToBorder && sx + borderSize <= img.cols - distanceToBorder)
                {
                    // ausserhalb der groesse des originalbildes
                    if((v.x() >= borderSize && v.y() >= borderSize && v.x() <= img.cols - 1 - borderSize &&
                        v.y() <= img.rows - 1 - borderSize) ||
                       (mTrackError[i] <
                        errorScale *
                            MAX_TRACK_ERROR)) // nur bei kleinem Fehler darf auch im Randbereich getrackt werden
                    {
                        // das Beschraenken auf die Bildgroesse ist reine sicherheitsmassnahme,
                        // oft sind tracking paths auch ausserhalb des bildes noch gut,
                        // aber beim tracken in die andere richtung kann es bei petrack probleme machen
                        if(v.x() >= 0 && v.y() >= 0 && v.x() <= img.cols - 1 && v.y() <= img.rows - 1)
                        {
                            // borderSize abziehen, da Trackerdaten am Rand des Originalbildes 0/0 ist
                            // set position relative to original image size
                            v += borderSize2F;
                            float x = -1, y = -1;
                            // ACHTUNG: BORDER NICHT BEACHTET bei point.x()...
                            // calculate height with disparity map
                            if(mMainWindow->getStereoContext() &&
                               mMainWindow->getStereoWidget()->stereoUseForHeight->isChecked())
                            {
                                mMainWindow->getStereoContext()->getMedianXYZaround(
                                    (int) v.x(), (int) v.y(), &x, &y, &z);
                                {
                                    v.setStereoMarker({{x, y, z}});
                                }
                            }
                            // wenn bei punkten, die nicht am rand liegen, der fehler gross ist,
                            // wird geguckt, ob der sprung sich zur vorherigen richtung stark veraendert hat
                            // wenn sprung sehr unterschiedlich, wird lieber interpoliert oder stehen geblieben
                            // ist richtung ok, dann wird dunkelstes pixel gesucht
                            // (subpixel aufgrund von nachbarpixel)
                            // oder einfach bei schlechtem fehler mit groesserem winSize=30 den Problempunkt nochmal
                            // machen
                            // TODO Wird gerade eben nicht gemacht. Sollten wir???

                            // ueberpruefen, ob tracking ziel auf anderem tracking path landet, dann beide trackpaths
                            // verschmelzen lassen
                            found = false;
                            if(mMainWindow->getControlWidget()
                                   ->isTrackMergeChecked()) // wenn zusammengefuehrt=merge=verschmolzen werden soll
                            {
                                found = tryMergeTrajectories(v, i, frame);
                            }

                            // wenn keine verschmelzung erfolgte, versuchen trackpoint einzufuegen
                            if(!found)
                            {
                                qual = static_cast<int>(errorToQual(mTrackError[i]));
                                if(qual < 20)
                                {
                                    qual = 20;
                                }
                                v.setQual(qual); // qual um 50, damit nur reco-kopf-ellipsen points nicht herauskegeln
                                // bei insertAtFrame wird qual beruecksichtigt, ob vorheiger besser

                                mPersonStorage.insertFeaturePoint(
                                    mPrevFeaturePointsIdx[i],
                                    frame,
                                    v,
                                    mPrevFeaturePointsIdx[i],
                                    (mMainWindow->getControlWidget()->isTrackExtrapolationChecked()),
                                    z,
                                    mMainWindow->getControlWidget()->getCameraAltitude());
                            }

                            ++inserted;
                        }
                    }
                }
            }
        }

        else
        {
            if(mStatus[i] == TrackStatus::NotTracked && v.x() >= dist && v.y() >= dist &&
               v.x() <= img.cols - 1 - dist && v.y() <= img.rows - 1 - dist)
            {
                SPDLOG_WARN(
                    "lost trajectory inside picture of person {} a frame {}!", mPrevFeaturePointsIdx[i] + 1, frame);
            }
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
    bool        found   = false;
    const auto &persons = mPersonStorage.getPersons();
    const auto &person  = persons[mPrevFeaturePointsIdx[i]];
    // nach trajektorie suchen, mit der eine verschmelzung erfolgen koennte
    for(int j = 0; !found && j < static_cast<int>(mPersonStorage.nbPersons()); ++j) // ueber TrackPerson
    {
        const auto &other = persons[j];
        if(j != mPrevFeaturePointsIdx[i] && other.trackPointExist(frame) &&
           (other.trackPointAt(frame).distanceToPoint(v) < mMainWindow->getHeadSize(nullptr, j, frame) / 2.))
        {
            // um ein fehltracking hin zu einer anderen Trajektorie nicht zum Verschmelzen dieser fuehren zu
            // lassen (die fehlerbehandlung durch interpolation wird in insertAtFrame durchgefuehrt)
            if(!((person.trackPointExist(frame - 1) &&
                  (person.trackPointAt(frame - 1).distanceToPoint(v) >
                   mMainWindow->getHeadSize(nullptr, mPrevFeaturePointsIdx[i], frame - 1) / 2.)) ||
                 (person.trackPointExist(frame + 1) &&
                  (person.trackPointAt(frame + 1).distanceToPoint(v) >
                   mMainWindow->getHeadSize(nullptr, mPrevFeaturePointsIdx[i], frame + 1) / 2.))))
            {
                int deleteIndex = mPersonStorage.merge(mPrevFeaturePointsIdx[i], j);

                int idxOtherMerged = -1;
                // shift index of feature points
                for(size_t k = 0; k < mPrevFeaturePointsIdx.size(); ++k)
                {
                    if(mPrevFeaturePointsIdx[k] > deleteIndex)
                    {
                        --mPrevFeaturePointsIdx[k];
                    }
                    else if(mPrevFeaturePointsIdx[k] == deleteIndex)
                    {
                        idxOtherMerged = static_cast<int>(k);
                    }
                }
                if(idxOtherMerged != -1)
                {
                    // set status to 2, so the already merged person is ignored/skipped
                    mStatus[idxOtherMerged] = TrackStatus::Merged;
                }
                found = true;
            }
        }
    }

    return found;
}

// default: int winSize=10, int level=3
// winSize=3 ist genauer, aber kann auch leichter abgelenkt werden; winSize=30 ist robuster aber ungenauer
// level kann groesser gewaehlt werden, wenn winSize klein, macht aber keinen grossen unterschied; (0) waere
// ohne pyramide war , int winSize=10
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
    cv::Mat                &img,
    cv::Rect               &rect,
    cv::Mat                 map1,
    int                     frame,
    bool                    reTrack,
    int                     reQual,
    int                     borderSize,
    reco::RecognitionMethod recoMethod,
    int                     level,
    QSet<size_t>            onlyVisible,
    int                     errorScaleExponent)
{
    QList<int> trjToDel;
    float      errorScale = pow(1.5, errorScaleExponent); // 0 waere neutral

    if(mGrey.empty())
    {
        SPDLOG_ERROR("you have to initialize tracking before using tracker!");
        return -1;
    }

    if(img.empty())
    {
        SPDLOG_ERROR("no NULL image allowed for tracking!!");
        return -1;
    }

    if((mPrevFrame != -1) && (abs(frame - mPrevFrame) > MAX_STEP_TRACK))
    {
        reset();
    }

    if(abs(frame - mPrevFrame) == 0)
    {
        SPDLOG_ERROR("Frame has not changed. There is nothing to track!");
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
        SPDLOG_ERROR("wrong number of channels: {}", img.channels());
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
            {
                SPDLOG_WARN("no tracking because of too many skipped frames ({} to {})!", mPrevFrame, frame);
            }
            else if(abs(frame - mPrevFrame) > 1)
            {
                SPDLOG_WARN(
                    "linear interpolation of skipped frames which are not already tracked({} to {}).",
                    mPrevFrame,
                    frame);
            }
        }

        trackFeaturePointsLK(level, mMainWindow->getControlWidget()->getAdaptiveLevel());

        // TODO Split up refineViaColorPointLK as well...
        refineViaColorPointLK(level, errorScale);

        BackgroundFilter *bgFilter = mMainWindow->getBackgroundFilter();
        // testen, ob Punkt im Vordergrund liegt, ansonsten, wenn nicht gerade zuvor detektiert, ganze
        // trajektorie loeschen (maximnale laenge ausserhalb ist somit 2 frames)
        if(bgFilter && bgFilter->getEnabled() && (mPrevFrame != -1)) // nur fuer den fall von bgSubtraction durchfuehren
        {
            useBackgroundFilter(trjToDel, bgFilter);
        }

        // (bei schlechten, aber noch ertraeglichem fehler in der naehe dunkelsten punkt suchen)
        // dieser ansatz kann dazu fuehren, dass bei starken helligkeitsunterschieden auf pappe zum schatten
        // gewandert wird!!!
        if(!mMainWindow->getStereoWidget()->stereoUseForReco->isChecked() &&
           ((recoMethod == reco::RecognitionMethod::Casern) ||
            (recoMethod == reco::RecognitionMethod::Hermes))) // nicht benutzen, wenn ueber disparity der kopf
                                                              // gesucht wird und somit kein marker vorhanden
                                                              // oder zumindest nicht am punkt lewigen muss
        {
            refineViaNearDarkPoint();
        }

        insertFeaturePoints(frame, numOfPeopleToTrack, img, borderSize, map1, errorScale);
    }

    cv::swap(mPrevGrey, mGrey);

    mPrevFrame = frame;

    // delete vorher ausgewaehlte trj
    // ACHTUNG: einzige stelle in tracker, wo eine trj geloescht wird
    // trackNumberAll, trackShowOnlyNr werden nicht angepasst, dies wird aber am ende von petrack::updateimage
    // gemacht
    for(int i = 0; i < trjToDel.size(); ++i) // ueber TrackPerson
    {
        mPersonStorage.delPointOf(trjToDel[i], PersonStorage::TrajectorySegment::Whole, -1);
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
                SPDLOG_WARN("try tracking person {} with pyramid level {}", mPrevFeaturePointsIdx[i], l);
            }

            winSize = mMainWindow->winSize(nullptr, mPrevFeaturePointsIdx[i], mPrevFrame, l);
            if(winSize < MIN_WIN_SIZE)
            {
                winSize = MIN_WIN_SIZE;
                SPDLOG_WARN(
                    "set search region to the minimum size of {} for person {}!",
                    MIN_WIN_SIZE,
                    mPrevFeaturePointsIdx[i] + 1);
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
        // status from OpenCV: 0 -> not tracked, 1 -> tracked
        mStatus[i] = localStatus[0] ? TrackStatus::Tracked : TrackStatus::NotTracked;
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
        const auto &person = mPersonStorage.at(mPrevFeaturePointsIdx[i]);
        // wenn fehler zu gross, dann Farbmarkerelement nehmen // fuer multicolor marker / farbiger hut mit
        // schwarzem punkt
        if(useColor && mTrackError[i] > errorScale * 150.F)
        {
            auto colorPoint = person.at(mPrevFrame - person.firstFrame()).getColorPointForOrientation();
            if(!colorPoint)
            {
                continue;
            }
            auto prevPointX = static_cast<float>(colorPoint->x());
            auto prevPointY = static_cast<float>(colorPoint->y());
            prevColorFeaturePoint.emplace_back(prevPointX, prevPointY);
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
                SPDLOG_WARN(
                    "tracking color marker instead of structural marker of person {} at {} x {} / error: {} / color "
                    "error: {}",
                    mPrevFeaturePointsIdx[i] + 1,
                    mFeaturePoints[i].x,
                    mFeaturePoints[i].y,
                    mTrackError[i],
                    colorTrackError[i]);


                mFeaturePoints[i] = cv::Point2f(
                    mPrevFeaturePoints[i].x + (colorFeaturePoint[i].x - prevColorFeaturePoint[i].x),
                    mPrevFeaturePoints[i].x + (colorFeaturePoint[i].x - prevColorFeaturePoint[i].x));
                SPDLOG_INFO("\tresulting point: {} x {}", mFeaturePoints[i].x, mFeaturePoints[i].y);
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
        const auto &person = mPersonStorage.at(mPrevFeaturePointsIdx[i]);
        x                  = myRound(mFeaturePoints[i].x - .5);
        y                  = myRound(mFeaturePoints[i].y - .5);

        // Rahmen, in dem nicht vordergrund pflicht, insbesondere am rechten rand!!!! es wird gruenes von hand
        // angelegtes bounding rect roi genutzt
        if((mStatus[i] == TrackStatus::Tracked) && x >= MAX(margin, rect.x()) &&
           x <= MIN(mGrey.cols - 1 - 2 * bS - margin - 50, rect.x() + rect.width()) && y >= MAX(margin, rect.y()) &&
           y <= MIN(mGrey.rows - 1 - 2 * bS - margin, rect.y() + rect.height()))
        {
            if(!bgFilter->isForeground(x, y) && person.trackPointAt(mPrevFrame).qual() < 100)
            {
                if((mMainWindow->getControlWidget()->isFilterBgDeleteTrjChecked()) &&
                   (person.nrInBg() >= mMainWindow->getControlWidget()->getFilterBgDeleteNumber()))
                {
                    // nur zum loeschen vormerken und am ende der fkt loeschen, da sonst Seiteneffekte komplex
                    trjToDel += mPrevFeaturePointsIdx[i];
                    SPDLOG_WARN(
                        "delete trajectory {} inside region of interest, because it laid outside foreground for {} "
                        "successive frames!",
                        mPrevFeaturePointsIdx[i] + 1,
                        mMainWindow->getControlWidget()->getFilterBgDeleteNumber());
                }
                else
                {
                    mPersonStorage.setNrInBg(mPrevFeaturePointsIdx[i], person.nrInBg() + 1);
                }
            }
            else // zaehler zuruecksetzen, der anzahl von getrackten Punkten im hintergrund zaehlt
            {
                mPersonStorage.setNrInBg(mPrevFeaturePointsIdx[i], 0);
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
        if((mTrackError[i] > MAX_TRACK_ERROR) && (mStatus[i] == TrackStatus::Tracked) && x >= 0 && x < mGrey.cols &&
           y >= 0 && y < mGrey.rows)
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
            {
                for(int j = xMin2 + 1; j < xMax2 - 1; ++j)
                {
                    if((getValue(mGrey, j, yMin2).value() <= darkest) ||
                       (getValue(mGrey, j, yMax2 - 1).value() <= darkest))
                    {
                        markerInsideWhite = false;
                        break;
                    }
                }
            }

            if(markerInsideWhite)
            {
                mFeaturePoints[i].x = xDark;
                mFeaturePoints[i].y = yDark;
                SPDLOG_INFO("move TrackPoint to darker pixel for {}!", i + 1);
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
            SPDLOG_ERROR(
                "Two MarkerIDs were found for one trajectory. (trackpoint: {}, person: {})", tpMarkerID, mMarkerID);
        }
    }
}

/**
 * @brief Get the i-th TrackPoint of the TrackPerson
 *
 * The i-th TrackPoint is the TrackPoint at Frame: mFirstFrame + i
 *
 * @param i index of TrackPoint
 * @return i-th TrackPoint of the TrackPerson
 */
const TrackPoint &TrackPerson::at(int i) const
{
    return mData.at(i);
}

int TrackPerson::size() const
{
    return mData.size();
}

bool TrackPerson::isEmpty() const
{
    return mData.isEmpty();
}

const TrackPoint &TrackPerson::first() const
{
    return mData.first();
}

const TrackPoint &TrackPerson::last() const
{
    return mData.last();
}

QList<TrackPoint>::const_iterator TrackPerson::cbegin() const
{
    return mData.cbegin();
}

QList<TrackPoint>::const_iterator TrackPerson::cend() const
{
    return mData.cend();
}

QList<TrackPoint>::const_iterator TrackPerson::begin() const
{
    return mData.cbegin();
}

QList<TrackPoint>::const_iterator TrackPerson::end() const
{
    return mData.cend();
}

void TrackPerson::append(const TrackPoint &trackPoint)
{
    mData.append(trackPoint);
}

void TrackPerson::clear()
{
    mData.clear();
}

void TrackPerson::replaceTrackPoint(int frame, TrackPoint trackPoint)
{
    mData[frame - mFirstFrame] = trackPoint;
}

void TrackPerson::updateStereoPoint(int frame, Vec3F stereoPoint)
{
    auto marker          = mData[frame - mFirstFrame].getStereoMarker();
    marker->mStereoPoint = stereoPoint;
    mData[frame - mFirstFrame].setStereoMarker(*marker);
}

void TrackPerson::updateMarkerID(int frame, int markerID)
{
    auto marker       = mData[frame - mFirstFrame].getCodeMarker();
    marker->mMarkerId = markerID;
    mData[frame - mFirstFrame].setCodeMarker(*marker);
}

/**
 * @brief Removes all trackPoints in range [startFrame, endFrame]
 *
 * Removes all trackPoints in the range [startFrame, endFrame].
 *
 * @param startFrame
 * @param endFrame
 */
void TrackPerson::removeFramesBetween(int startFrame, int endFrame)
{
    if((startFrame < mFirstFrame) || (endFrame > lastFrame()))
    {
        throw std::out_of_range(fmt::format(
            "The supplied range of frames [{},{}] to delete includes frames which this person (firstFrame: {}, "
            "lastFrame: {}) does not have.",
            startFrame,
            endFrame,
            firstFrame(),
            lastFrame()));
    }

    if((startFrame != mFirstFrame) && (endFrame != lastFrame()))
    {
        throw std::range_error(
            "Points of a TrackPerson need to be continuous in time. Cannot remove middle part of trajectory.");
    }

    auto startIndex = startFrame - mFirstFrame;
    auto endIndex   = endFrame - mFirstFrame + 1; // +1 to also remove endFrame

    auto startIt = mData.begin() + startIndex;
    auto endIt   = mData.begin() + endIndex;
    mData.erase(startIt, endIt);

    if(startFrame == mFirstFrame)
    {
        mFirstFrame = endFrame + 1;
    }
}


ParseResult parseTrackPerson(
    const QStringList      &lines,
    int                    &currentLineIndex,
    TrackPerson            &trackPerson,
    reco::RecognitionMethod recoMethod)
{
    if(currentLineIndex >= lines.size())
    {
        return {"Unexpected end of file while reading track person header"};
    }
    // parse header
    QStringView   headerLine = lines[currentLineIndex];
    TrcLineParser headerParser(headerLine, currentLineIndex + 1);
    int           expectedHeaderTokens = 9; // nr, height, firstFrame, lastFrame, colorCount, color(r,g,b), frameAmount
    if(Petrack::trcVersion > 3)
    {
        ++expectedHeaderTokens; // markerID
    }
    auto result = headerParser.validateTokenCount(expectedHeaderTokens);
    if(!result.success)
    {
        return result;
    }
    // parse header fields

    int    nr, firstFrame, lastFrame, colorCount, n, markerID = -1;
    double height;
    QColor col;

    result = headerParser.parseInt(nr);
    if(!result.success)
    {
        return result;
    }

    result = headerParser.parseDouble(height);
    if(!result.success)
    {
        return result;
    }

    result = headerParser.parseInt(firstFrame);
    if(!result.success)
    {
        return result;
    }

    result = headerParser.parseInt(lastFrame);
    if(!result.success)
    {
        return result;
    }

    result = headerParser.parseInt(colorCount);
    if(!result.success)
    {
        return result;
    }

    result = headerParser.parseColor(col);
    if(!result.success)
    {
        return result;
    }

    if(Petrack::trcVersion > 3)
    {
        result = headerParser.parseInt(markerID);
        if(!result.success)
        {
            return result;
        }
    }

    result = headerParser.parseInt(n);
    if(!result.success)
    {
        return result;
    }

    if(n <= 0)
    {
        return {QString("Invalid trackPoint count: %1").arg(n), currentLineIndex + 1};
    }

    ++currentLineIndex;

    // Parse comment line if version > 2
    QString comment;
    if(Petrack::trcVersion > 2)
    {
        if(currentLineIndex >= lines.size())
        {
            return {"Expected comment line but reached end of file"};
        }
        comment = lines[currentLineIndex];
        comment.replace(QRegularExpression("<br>"), "\n");
        ++currentLineIndex;
    }

    // Parse first track point
    if(currentLineIndex >= lines.size())
    {
        return {"Expected first track point but reached end of file"};
    }

    TrackPoint firstPoint;
    result = parseTrackPoint(lines[currentLineIndex], currentLineIndex + 1, firstPoint, recoMethod);
    if(!result.success)
    {
        return result;
    }
    ++currentLineIndex;

    // Create TrackPerson
    trackPerson = TrackPerson(nr, firstFrame, firstPoint, markerID);
    trackPerson.setHeight(height);
    trackPerson.setColCount(colorCount);
    trackPerson.setColor(col);
    if(!comment.isEmpty())
    {
        trackPerson.setComment(comment);
    }

    // Parse remaining track points
    for(int i = 1; i < n; ++i)
    {
        if(currentLineIndex >= lines.size())
        {
            return {QString("Expected track point %1 of %2 but reached end of file").arg(i + 1).arg(n)};
        }

        TrackPoint trackPoint;
        result = parseTrackPoint(lines[currentLineIndex], currentLineIndex + 1, trackPoint, recoMethod);
        if(!result.success)
        {
            return result;
        }

        trackPerson.append(trackPoint);
        ++currentLineIndex;
    }

    return {};
}

QTextStream &operator<<(QTextStream &s, const TrackPerson &tp)
{
    s << tp.nr() << " " << tp.height() << " " << tp.firstFrame() << " " << tp.lastFrame() << " " << tp.colCount() << " "
      << tp.color();
    if(Petrack::trcVersion > 3)
    {
        s << " " << tp.getMarkerID();
    }
    s << " " << tp.size();
    s << Qt::endl << tp.serializeComment() << Qt::endl;
    for(int i = 0; i < tp.size(); ++i)
    {
        s << tp.at(i) << Qt::endl;
    }
    return s;
}

std::ostream &operator<<(std::ostream &s, const TrackPerson &tp)
{
    s << tp.nr() << " " << tp.height() << " " << tp.firstFrame() << " " << tp.lastFrame() << " " << tp.colCount() << " "
      << tp.color();
    if(Petrack::trcVersion > 3)
    {
        s << " " << tp.getMarkerID();
    }
    s << " " << tp.size();
    s << std::endl << tp.serializeComment() << std::endl;
    for(int i = 0; i < tp.size(); ++i)
    {
        s << tp.at(i) << std::endl;
    }
    return s;
}
