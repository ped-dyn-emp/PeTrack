/*
 * PeTrack - Software for tracking pedestrians movement in videos
 * Copyright (C) 2026 Forschungszentrum Jülich GmbH, IAS-7
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

#include "trackPerson.h"

#include "petrack.h"


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
        std::optional<StereoMarker> nrForStereoMarker = mData.at(i + nrFor).getStereoMarker();

        while((i - nrRew >= 0) &&
              (!mData.at(i - nrRew)
                    .getStereoMarker())) // nach && wird nur ausgefuehrt, wenn erstes true == size() also nicht
        {
            ++nrRew;
        }

        std::optional<StereoMarker> nrRewStereoMarker = mData.at(i - nrRew).getStereoMarker();

        if((i + nrFor == mData.size()) && (i - nrRew < 0)) // gar keine Hoeheninfo in trj gefunden
        {
            return -1.;
        }
        else if(i + nrFor == mData.size()) // nur in Vergangenheit hoeheninfo gefunden
        {
            *extrapolated = 2;
            return nrRewStereoMarker->mStereoPoint.z();
        }
        else if(i - nrRew < 0) // nur in der zukunft hoeheninfo gefunden
        {
            *extrapolated = 1;
            return nrForStereoMarker->mStereoPoint.z();
        }
        else // in beiden richtungen hoeheninfo gefunden - INTERPOLATION, NICHT EXTRAPOLATION
        {
            return nrRewStereoMarker->mStereoPoint.z() +
                   (nrRew * (nrForStereoMarker->mStereoPoint.z() - nrRewStereoMarker->mStereoPoint.z()) /
                    (nrFor + nrRew)); // lineare interpolation
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
                auto orientationPoint = trackPointAt(frame - 1).getColorPointForOrientation();
                if(orientationPoint)
                {
                    tp.setQual(95);
                    tmp = point.pixelPoint() + (trackPointAt(frame - 1).pixelPoint() - *orientationPoint);
                    tp.setX(tmp.x());
                    tp.setY(tmp.y());
                    SPDLOG_WARN(
                        "move TrackPoint according to last distance of structure marker and color marker of person {}:",
                        persNr + 1);
                    SPDLOG_WARN("         {} -> {}", point, tp);
                }
            }
            else if(trackPointExist(frame + 1) && trackPointAt(frame + 1).qual() > 90)
            {
                auto orientationPoint = trackPointAt(frame + 1).getColorPointForOrientation();
                if(orientationPoint)
                {
                    tp.setQual(95);
                    tmp = point.pixelPoint() + (trackPointAt(frame + 1).pixelPoint() - *orientationPoint);
                    tp.setX(tmp.x());
                    tp.setY(tmp.y());
                    SPDLOG_WARN(
                        "move TrackPoint according to last distance of structure marker and color marker of person {}:",
                        persNr + 1);
                    SPDLOG_WARN("         {} -> {}", point, tp);
                }
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
    StereoMarker newStereo{stereoPoint};
    mData[frame - mFirstFrame].setStereoMarker(newStereo);
}

void TrackPerson::updateMarkerID(int frame, int markerID)
{
    auto marker = mData[frame - mFirstFrame].getCodeMarker();
    if(marker)
    {
        marker->mMarkerId = markerID;
        mData[frame - mFirstFrame].setCodeMarker(*marker);
    }
    else
    {
        CodeMarker newCode{markerID};
        mData[frame - mFirstFrame].setCodeMarker(newCode);
    }
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
        throw std::out_of_range(
            fmt::format(
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
