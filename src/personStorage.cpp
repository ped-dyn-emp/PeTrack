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

#include "personStorage.h"

#include "animation.h"
#include "autosave.h"
#include "control.h"
#include "logger.h"
#include "multiColorMarkerWidget.h"
#include "pInputDialog.h"
#include "pMessageBox.h"
#include "petrack.h"
#include "roiItem.h"
#include "stereoWidget.h"

/**
 * @brief split trajectorie pers before frame frame
 * @param pers index of person
 * @param frame frame to split at
 */
void PersonStorage::splitPerson(size_t pers, int frame)
{
    onManualAction();

    {
        const QSignalBlocker blocker(this);
        if(mPersons.at(pers).firstFrame() < frame)
        {
            mPersons.push_back(mPersons.at(pers));

            // alte trj einkuerzen und ab aktuellem frame zukunft loeschen
            deletePersonFrameRange(pers, frame, mPersons[pers].lastFrame());

            // neu angehaengte/gedoppelte trajektorie
            deletePersonFrameRange(mPersons.size() - 1, mPersons[pers].firstFrame(), frame - 1);
        }
    }

    emit splitPersonAtFrame(pers, mPersons.size() - 1, frame);
    emit changedPerson(pers);
    emit changedPerson(mPersons.size() - 1);
}

/**
 * @brief Split trajectory at point point before given frame
 *
 * @param point point where to split trajectory (helpful if onlyVisible isn't set)
 * @param frame frame at which to split the trajectory
 * @param onlyVisible set of people for whom to do it (empty means everyone)
 * @return true if a trajectory was split
 */
bool PersonStorage::splitPersonAt(const Vec2F &point, int frame, const QSet<size_t> &onlyVisible)
{
    onManualAction();

    for(size_t i = 0; i < mPersons.size(); ++i)
    { // ueber TrackPerson
        if(((onlyVisible.empty()) || (onlyVisible.contains(i))) &&
           (mPersons.at(i).trackPointExist(frame) &&
            (mPersons.at(i).trackPointAt(frame).distanceToPoint(point) <
             mMainWindow.getHeadSize(nullptr, static_cast<int>(i), frame) / 2.)))
        {
            splitPerson(i, frame);

            return true;
        }
    }
    return false;
}


/**
 * @brief Deletes points of pers
 * @param pers TrackPerson whose points should be deleted
 * @param direction the direction in which the trajectory should be deleted
 * @param frame the frame (exclusive) from which the deletion will happen
 * @return true, if deletion occured
 */
bool PersonStorage::delPointOf(int pers, TrajectorySegment direction, int frame)
{
    onManualAction();
    mPersons[pers].resetKalmanFilter();

    if(direction == TrajectorySegment::Previous)
    {
        deletePersonFrameRange(pers, mPersons[pers].firstFrame(), frame - 1);
    }
    else if(direction == TrajectorySegment::Whole)
    {
        deletePerson(pers);
    }
    else if(direction == TrajectorySegment::Following)
    {
        deletePersonFrameRange(pers, frame + 1, mPersons[pers].lastFrame());
    }
    return true;
}


/**
 * @brief Deletes points of a SINGLE person in onlyVisible
 * @param point point which need to be on the person (helpful if onlyVisible is not properly set)
 * @param direction the direction in which the trajectory should be deleted
 * @param frame the frame (exclusive) from which the deletion will happen
 * @param onlyVisible set of people whose points could be deleted; empty means everyone
 * @return true if deletion occured
 */
bool PersonStorage::delPoint(
    const Vec2F        &point,
    TrajectorySegment   direction,
    int                 frame,
    const QSet<size_t> &onlyVisible)
{
    onManualAction();

    for(int i = 0; i < static_cast<int>(mPersons.size()); ++i)
    { // ueber TrackPerson
        if(((onlyVisible.empty()) || (onlyVisible.contains(i))) &&
           (mPersons.at(i).trackPointExist(frame) && (mPersons.at(i).trackPointAt(frame).distanceToPoint(point) <
                                                      mMainWindow.getHeadSize(nullptr, i, frame) / 2.)))
        {
            delPointOf(i, direction, frame);
            return true;
        }
    }
    return false;
}


/**
 * @brief Deletes trackpoints of all trajectories
 * @param direction notes if previous, following or whole trajectory should be deleted
 * @param frame
 */
void PersonStorage::delPointAll(TrajectorySegment direction, int frame)
{
    onManualAction();
    resetKalmanFilters();

    for(size_t i = 0; i < mPersons.size(); ++i) // ueber TrackPerson
    {
        if(mPersons.at(i).trackPointExist(frame))
        {
            switch(direction)
            {
                case TrajectorySegment::Previous:
                    deletePersonFrameRange(i, mPersons[i].firstFrame(), frame - 1);
                    break;
                case TrajectorySegment::Whole:
                    deletePerson(i--); // after deleting the person decrease i by one
                    break;
                case TrajectorySegment::Following:
                    deletePersonFrameRange(i, frame + 1, mPersons[i].lastFrame());
                    break;
            }
        }
        else if(
            ((direction == TrajectorySegment::Previous) && (frame > mPersons.at(i).lastFrame())) ||
            (direction == TrajectorySegment::Whole) ||
            ((direction == TrajectorySegment::Following) && (frame < mPersons.at(i).firstFrame())))
        {
            deletePerson(i--); // after deleting the person decrease i by one
        }
    }
}


/**
 * @brief deletes points of a trajectrory, which are inside ROI
 *
 * 1 trajectory can end in 0, 1 or multiple trajectories!!!!!!!!
 */
void PersonStorage::delPointInsideROI()
{
    onManualAction();

    QRectF rect = mMainWindow.getRecoRoiItem()->rect();
    bool   inside;

    for(size_t i = 0; i < mPersons.size(); ++i) // ueber TrackPerson
    {
        inside = ((!mPersons.empty()) && rect.contains(mPersons.at(i).at(0).x(), mPersons.at(i).at(0).y()));
        for(int j = 1; j < mPersons.at(i).size(); ++j)
        {
            if(inside != rect.contains(mPersons.at(i).at(j).x(), mPersons.at(i).at(j).y())) // aenderung von inside
            {
                splitPerson(i, mPersons.at(i).firstFrame() + j);
                if(inside)
                {
                    deletePerson(i--); // after deleting the person decrease i by one
                    inside = !inside;
                }
                break;
            }
        }
        if(inside)
        {
            // rest loeschen
            deletePerson(i--); // after deleting the person decrease i by one
        }
    }
}


/**
 * @brief deletes trajectory, if it is partly inside ROI
 */
void PersonStorage::delPointROI()
{
    onManualAction();

    int    anz  = 0;
    QRectF rect = mMainWindow.getRecoRoiItem()->rect();

    for(size_t i = 0; i < mPersons.size(); ++i) // ueber TrackPerson
    {
        for(int j = 0; j < mPersons.at(i).size(); ++j)
        {
            if(rect.contains(mPersons.at(i).at(j).x(), mPersons.at(i).at(j).y()))
            {
                anz++;
                deletePerson(i--); // after deleting the person decrease i by one
                break;
            }
        }
    }
    SPDLOG_INFO("deleted {} trajectories!", anz);
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
bool PersonStorage::editTrackPersonComment(const Vec2F &point, int frame, const QSet<size_t> &onlyVisible)
{
    onManualAction();

    for(int i = 0; i < static_cast<int>(mPersons.size()); ++i) // ueber TrackPerson
    {
        if(((onlyVisible.empty()) || (onlyVisible.contains(i))) &&
           (mPersons.at(i).trackPointExist(frame) &&
            (mPersons.at(i).trackPointAt(frame).distanceToPoint(point) <
             mMainWindow.getHeadSize(nullptr, i, frame) / 2.))) // war: MIN_DISTANCE)) // 30 ist abstand zwischen kopfen
        {
            QString displayedComment = mPersons.at(i).comment();
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
            QString comment = PInputDialog::getMultiLineText(
                &mMainWindow, QObject::tr("Add Comment"), QObject::tr("Comment:"), displayedComment, &ok);

            if(ok)
            {
                if(comment.isEmpty())
                {
                    int ret = PWarning(
                        &mMainWindow,
                        QObject::tr("Empty comment"),
                        QObject::tr("Are you sure you want to save an empty comment?"),
                        PMessageBox::StandardButton::Save | PMessageBox::StandardButton::Cancel);
                    if(ret == PMessageBox::StandardButton::Cancel)
                    {
                        return false;
                    }
                }
                mPersons[i].setComment(comment);
                return true;
            }
        }
    }
    return false;
}

/**
 * @brief Sets the height of the TrackPerson visible near the selected point at the selected frame
 * @param point point near the current TrackPoint of the person
 * @param frame current frame
 * @param onlyVisible Set of people which could be selected (empty means everyone can be selected)
 * @return whether the height of a TrackPerson was successfully changed
 */
bool PersonStorage::setTrackPersonHeight(const Vec2F &point, int frame, const QSet<size_t> &onlyVisible)
{
    onManualAction();

    for(int i = 0; i < static_cast<int>(mPersons.size()); ++i) // ueber TrackPerson
    {
        if(((onlyVisible.empty()) || (onlyVisible.contains(i))) &&
           (mPersons.at(i).trackPointExist(frame) &&
            (mPersons.at(i).trackPointAt(frame).distanceToPoint(point) <
             mMainWindow.getHeadSize(nullptr, i, frame) / 2.))) // war: MIN_DISTANCE)) // 30 ist abstand zwischen kopfen
        {
            bool ok;

            double col_height;
            // col_height is negative, if height is determined through color and not yet set manually
            if(mPersons.at(i).height() < MIN_HEIGHT + 1)
            {
                col_height = mPersons.at(i).color().isValid() ?
                                 -mMainWindow.getControlWidget()->getColorPlot()->map(mPersons.at(i).color()) :
                                 -mMainWindow.getControlWidget()->getDefaultHeight();
            }
            else
            {
                col_height = mPersons.at(i).height();
            }


            double height = QInputDialog::getDouble(
                &mMainWindow,
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
                    SPDLOG_WARN("you entered a negative height!");
                }
                // if previous value (col_height) is negative, height was determined thru color. If manually set value
                // is the color-map value, we do not change anything
                // @todo: @ar.graf: check if manually set values have side-effects (maybe do not show in statistics)
                if(!(std::abs(col_height + height) < 0.01))
                {
                    mPersons[i].setHeight(height);
                    return true;
                }
                else
                {
                    SPDLOG_INFO("No height change detected. Color-mapped height will remain set.");
                }
            }
        }
    }
    return false;
}

/**
 * @brief Resets the height of the TrackPerson near the selected point at the selected frame
 * @param point point near the current TrackPoint of the person
 * @param frame current frame
 * @param onlyVisible Set of people which could be selected (empty means everyone can be selected)
 * @return true if height was successfully reset
 */
bool PersonStorage::resetTrackPersonHeight(const Vec2F &point, int frame, const QSet<size_t> &onlyVisible)
{
    onManualAction();

    for(int i = 0; i < static_cast<int>(mPersons.size()); ++i) // ueber TrackPerson
    {
        if(((onlyVisible.empty()) || (onlyVisible.contains(i))) &&
           (mPersons.at(i).trackPointExist(frame) &&
            (mPersons.at(i).trackPointAt(frame).distanceToPoint(point) <
             mMainWindow.getHeadSize(nullptr, i, frame) / 2.))) // war: MIN_DISTANCE)) // 30 ist abstand zwischen kopfen
        {
            mPersons[i].setHeight(MIN_HEIGHT);
            return true;
        }
    }
    return false;
}

void PersonStorage::moveTrackPoint(int personID, int frame, const Vec2F &newPosition)
{
    auto      &person = mPersons.at(personID);
    TrackPoint newPoint;
    newPoint = newPosition;
    newPoint.setQual(100);
    if(person.trackPointExist(frame))
    {
        person.replaceTrackPoint(frame, newPoint);
        emit changedPerson(personID);
    }
    else
    {
        // only logging since this is a software bug, not a user bug; precondition of function not fulfilled
        SPDLOG_WARN("Trying to move nonexistent TrackPoint of person {} at frame {}", personID, frame);
    }
}

void PersonStorage::resetKalmanFilters()
{
    for(auto &person : mPersons)
    {
        person.resetKalmanFilter();
    }
}


// used for calculation of 3D point for all points in frame
// returns number of found points or -1 if no stereoContext available (also points without disp found are counted)
int PersonStorage::calcPosition(int frame)
{
    int                 anz = 0;
    pet::StereoContext *sc  = mMainWindow.getStereoContext();
    float               x, y, z;

    if(sc)
    {
        // for every point of a person, which has already identified at this frame
        for(auto &person : mPersons) // ueber TrackPerson
        {
            if(person.trackPointExist(frame))
            {
                ++anz;

                // TrackPoint *point = &(at(i).trackPointAt(frame));
                //  ACHTUNG: BORDER NICHT BEACHTET bei p.x()...???
                //  calculate height with disparity map
                if(sc->getMedianXYZaround(
                       (int) person.trackPointAt(frame).x(),
                       (int) person.trackPointAt(frame).y(),
                       &x,
                       &y,
                       &z)) // nicht myRound, da pixel 0 von 0..0.99 in double geht
                {
                    // hier kommt man nur hinein, wenn x, y, z Wert berechnet werden konnten
                    // statt altitude koennte hier irgendwann die berechnete Bodenhoehe einfliessen
                    person.updateStereoPoint(frame, {x, y, z}); // setZdistanceToCam(z);
                    person.setHeight(z, mMainWindow.getControlWidget()->getCameraAltitude());
                }
            }
        }

        return anz;
    }
    else
        return -1;
}


/**
 * @brief Adds the point to the PersonStorage, either to exising person or creating a new one.
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
bool PersonStorage::addPoint(
    TrackPoint             &point,
    int                     frame,
    const QSet<size_t>     &onlyVisible,
    reco::RecognitionMethod method,
    int                    *pers)
{
    if(point.qual() > TrackPoint::BEST_DETECTION_QUAL)
    {
        // manually added point
        onManualAction();
    }
    bool  found = false;
    int   i, iNearest = 0.;
    float scaleHead;
    float dist, minDist = 1000000.;
    float z = -1;
    float x = -1, y = -1;

    double trackPointSize = mMainWindow.getControlWidget()->getTrackCurrentPointSize(); // size for manual actions

    // ACHTUNG: BORDER NICHT BEACHTET bei point.x()...
    // hier wird farbe nur bei reco bestimmt gegebenfalls auch beim tracken interessant
    // calculate height with disparity map
    if(mMainWindow.getStereoContext() && mMainWindow.getStereoWidget()->stereoUseForHeight->isChecked())
    {
        if(mMainWindow.getStereoContext()->getMedianXYZaround(
               (int) point.x(), (int) point.y(), &x, &y, &z)) // nicht myRound, da pixel 0 von 0..0.99 in double geht
        {
            // statt altitude koennte hier irgendwann die berechnete Bodenhoehe einfliessen
            point.setStereoMarker({{x, y, z}}); // setZdistanceToCam(z);
        }
        // cout << " " << point.x()<< " " << point.y() << " " << x << " " << y << " " << z <<endl;
        // if (i == 10)
        //     debout << i << " " << mMainWindow.getControlWidget()->getCameraAltitude() - z << " " << z << " " <<
        //     mPersons[i].height() << endl;
    }
    // skalierungsfaktor fuer kopfgroesse
    // fuer multicolor marker groesser, da der schwarze punkt weit am rand liegen kann
    bool multiColorWithDot = false;
    if(method == reco::RecognitionMethod::MultiColor &&                // multicolor marker
       mMainWindow.getMultiColorMarkerWidget()->useDot->isChecked() && // nutzung von black dot
       !mMainWindow.getMultiColorMarkerWidget()
            ->ignoreWithoutDot->isChecked()) // muetzen ohne black dot werden auch akzeptiert
    {
        multiColorWithDot = true;
        scaleHead         = 1.3f;
    }
    else
    {
        scaleHead = 1.0f;
    }

    for(i = 0; i < static_cast<int>(mPersons.size()); ++i) // !found &&  // ueber TrackPerson
    {
        if(((onlyVisible.empty()) || (onlyVisible.contains(i))) && mPersons.at(i).trackPointExist(frame))
        {
            dist = mPersons.at(i).trackPointAt(frame).distanceToPoint(point);

            bool useTrackpointSize = point.qual() > TrackPoint::BEST_DETECTION_QUAL &&
                                     !mMainWindow.getControlWidget()->isTrackHeadSizedChecked();
            double headSize =
                useTrackpointSize ?
                    trackPointSize :
                    mMainWindow.getHeadSize(nullptr, i, frame); // manually added trackpoint affected by *visible* track
                                                                // point size; more intuitive and adaptable
            const auto multiColorMarker = point.getMultiColorMarker();
            if((dist < scaleHead * headSize / 2.) ||
               // fuer multifarbmarker mit schwarzem punkt wird nur farbmarker zur Abstandbetrachtung herangezogen
               // at(i).trackPointAt(frame).colPoint() existiert nicht an dieser stelle, da bisher nur getrackt
               // wurde!!!!
               (multiColorWithDot && multiColorMarker &&
                (mPersons.at(i).trackPointAt(frame).distanceToPoint(multiColorMarker->mColorPoint) < headSize / 2.)))
            {
                if(found)
                {
                    SPDLOG_WARN("multiple possible TrackPoints for point");
                    SPDLOG_WARN("         {} in frame {} with low distance:", point, frame);
                    SPDLOG_WARN("         person {} (distance: {})", i + 1, dist);
                    SPDLOG_WARN("         person {} (distance: {})", iNearest + 1, minDist);
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
                    found    = true;
                }
            }
        }
    }
    if(found) // den naechstgelegenen nehmen
    {
        // test, if recognition point or tracked point is better is made in at(i).insertAtFrame
        if(mPersons[iNearest].insertAtFrame(
               frame,
               point,
               iNearest,
               mMainWindow.getControlWidget()->isTrackUseKalmanChecked(),
               mMainWindow.getControlWidget()
                   ->isTrackExtrapolationChecked())) // wenn eingefuegt wurde (bessere qualitaet)
        //|| !at(i).trackPointAt(frame).color().isValid() moeglich, um auch bei schlechterer
        // qualitaet aber aktuell nicht
        // vorliegender farbe die ermittelte farbe einzutragen - kommt nicht vor!
        {
            // Synchronize TrackPerson.markerID with TrackPoints code marker markerID
            if(auto codeMarker = point.getCodeMarker())
            {
                mPersons[iNearest].syncTrackPersonMarkerID(codeMarker->mMarkerId);
            }

            // set/add color
            if(auto color = point.getColorForHeightMap()) // not valid for manual, than old color is used
            {
                // if (at(i).trackPointAt(frame).color().isValid()) man koennte alte farbe abziehen - aber nicht noetig,
                // kommt nicht vor
                mPersons[iNearest].addColor(*color);
            }
        }

        if(pers != nullptr)
        {
            *pers = iNearest;
        }

        mPersons[iNearest].setNewReco(true);
    }

    if((onlyVisible.empty()) && !found)
    {
        iNearest = static_cast<int>(mPersons.size());

        if(point.qual() > TrackPoint::BEST_DETECTION_QUAL) // manual add
        {
            point.setQual(TrackPoint::BEST_DETECTION_QUAL);
        }
        auto codeMarker = point.getCodeMarker();
        mPersons.emplace_back(
            0,
            frame,
            point,
            codeMarker ? codeMarker->mMarkerId : -1); // 0 is person number/markerID; newReco is set to true by default
    }
    if((z > 0) && ((onlyVisible.empty()) || found))
    {
        mPersons[iNearest].setHeight(z, mMainWindow.getControlWidget()->getCameraAltitude()); // , frame
    }
    if((!onlyVisible.empty()) && !found)
    {
        QMessageBox::warning(
            nullptr,
            "PeTrack",
            "Adding a manual TrackPoint is only possible, when \"show only people\" and \"show only people list\" are "
            "disabled!\n"
            "You would not see the newly created TrackPoint otherwise.");
        SPDLOG_WARN("No manual insertion, because not all trajectories are visible!");
        return false;
    }

    if(found)
    {
        emit changedPerson(iNearest);
    }
    return !found;
}


// used from recognition
/**
 * @brief Adds multiple points as new trajectories or to existing ones
 *
 * Is used by recognition!
 *
 * @param pL List of points to add
 * @param frame current frame/frame the points were detected in
 * @param method used recognition method/marker type
 */
void PersonStorage::addPoints(QList<TrackPoint> &pL, int frame, reco::RecognitionMethod method)
{
    // reset newReco
    for(auto &person : mPersons)
    {
        person.setNewReco(false);
    }

    // ueberprufen ob identisch mit einem Punkt in liste
    for(auto &point : pL) // ueber PointList
    {
        addPoint(point, frame, QSet<size_t>(), method);
    }
}

/// Number of visible (TrackPoint exists in current frame) people
int PersonStorage::visible(int frameNum) const
{
    return static_cast<int>(std::count_if(
        mPersons.begin(),
        mPersons.end(),
        [frameNum](const TrackPerson &pers) { return pers.trackPointExist(frameNum); }));
}

/// Returns the largest first frame of **all** TrackPersons
int PersonStorage::largestFirstFrame() const
{
    auto maxElement = std::max_element(
        mPersons.cbegin(),
        mPersons.cend(),
        [](const TrackPerson &lhs, const TrackPerson &rhs) { return lhs.firstFrame() < rhs.firstFrame(); });
    return maxElement != mPersons.cend() ? (*maxElement).firstFrame() : -1;
}

/// Returns the largest last frame of **all** TrackPersons
int PersonStorage::largestLastFrame() const
{
    auto maxElement = std::max_element(
        mPersons.cbegin(),
        mPersons.cend(),
        [](const TrackPerson &lhs, const TrackPerson &rhs) { return lhs.lastFrame() < rhs.lastFrame(); });
    return maxElement != mPersons.cend() ? (*maxElement).lastFrame() : -1;
}

/// Returns the smallest first frame of **all** TrackPersons
int PersonStorage::smallestFirstFrame() const
{
    auto minElement = std::min_element(
        mPersons.cbegin(),
        mPersons.cend(),
        [](const TrackPerson &lhs, const TrackPerson &rhs) { return lhs.firstFrame() < rhs.firstFrame(); });
    return minElement != mPersons.cend() ? (*minElement).firstFrame() : -1;
}

/**
 * @brief Get a list of all people near pos at a frame in the given interval
 *
 * This function returns a list of all people, which are near the
 * position pos at some frame f in the interval
 * frame - before <= f <= frame + after
 *
 * Near is, when the distance is less than half a headsize. When
 * multiple points for the same person are possible, the point
 * and frame at which the person is nearest to the pos is returned,
 * i.e. each persons only occurs once in the resulting list.
 *
 * @param pos position near the persons to select
 * @param frame current frame
 * @param selected set of ids of selected persons; empty means everyone is selected
 * @param before frames to include before the current frame
 * @param after frames to include after the current frame
 * @return list of the id of all proximal persons with the frame at which they are nearest to pos
 */
std::vector<PersonFrame>
PersonStorage::getProximalPersons(const QPointF &pos, QSet<size_t> selected, const FrameRange &frameRange) const
{
    std::vector<PersonFrame> result;
    for(int i = 0; i < static_cast<int>(mPersons.size()); ++i)
    {
        if(!selected.empty() && !selected.contains(i))
        {
            continue;
        }

        double minDist  = std::numeric_limits<double>::max();
        int    minFrame = -1;
        for(int f = frameRange.current - frameRange.before; f <= frameRange.current + frameRange.after; ++f)
        {
            if(!mPersons[i].trackPointExist(f))
            {
                continue;
            }
            auto dist = mPersons[i].trackPointAt(f).distanceToPoint(pos);
            if(dist < minDist && dist < (mMainWindow.getHeadSize(nullptr, i, frameRange.current) / 2.))
            {
                minDist  = dist;
                minFrame = f;
            }
        }
        if(minFrame == -1)
        {
            continue;
        }
        result.push_back({i, minFrame});
    }

    return result;
}


/**
 * @brief Recalcs the height of all persons (used with stereo)
 * @param altitude altitude of the camera (assumes orthogonal view?)
 */
void PersonStorage::recalcHeight(float altitude)
{
    onManualAction();

    for(auto &person : mPersons)
    {
        person.recalcHeight(altitude);
    }
}

/// optimize color for all persons
void PersonStorage::optimizeColor()
{
    onManualAction();

    for(auto &person : mPersons)
    {
        if(person.color().isValid())
        {
            person.optimizeColor();
        }
    }
}

/// reset the height of all persons, but not the pos of the trackpoints
void PersonStorage::resetHeight()
{
    onManualAction();

    for(auto &person : mPersons)
    {
        person.resetHeight();
    }
}

/// reset the pos of the tzrackpoints, but not the heights
void PersonStorage::resetPos()
{
    onManualAction();

    for(auto &person : mPersons)
    {
        for(int frame = person.firstFrame(); frame <= person.lastFrame(); ++frame)
        {
            person.updateStereoPoint(frame, {-1, -1, -1});
        }
    }
}

/**
 * @brief Prints height distribution to stdout
 *
 * @return false if no height information is available, else true
 */
bool PersonStorage::printHeightDistribution()
{
    QMap<double, int>                 dict;
    QMap<double, int>::const_iterator j;
    int                               anz        = 0;
    int                               heightStep = 5;
    double                            average = 0., avg = 0.;
    int                               noHeight = 0;

    for(const auto &person : mPersons)
    {
        if(person.height() >
           MIN_HEIGHT) // !=-1// insbesondere von hand eingefuegte trackpoint/persons haben keine farbe
        {
            ++dict[(static_cast<int>(person.height()) / heightStep) * heightStep];
            avg += person.height();
        }
        else
        {
            ++noHeight;
        }
    }

    anz = std::accumulate(dict.cbegin(), dict.cend(), 0);

    SPDLOG_INFO("number of persons with measured height                            : {}", anz);
    SPDLOG_INFO(
        "person without measured height (not included in calculated values): {} (using default height for export)",
        noHeight);
    if(anz == 0)
    {
        return false;
    }

    for(j = dict.constBegin(); j != dict.constEnd(); ++j)
    {
        SPDLOG_INFO(
            "height {:1.5f} - {:1.5f} : number {:.3f} ({:4.f}%)",
            j.key(),
            j.key() + heightStep,
            j.value(),
            (100. * j.value()) / anz);
        average += (j.key() + heightStep / 2.) * j.value();
    }

    SPDLOG_INFO("average height (bucket): {:1.5f}", average / anz);
    SPDLOG_INFO("average height         : {:1.5f}", avg / anz);

    return true;
}

/**
 * Sets the heights based on the values contained in \p heights.
 * @param heights Map between marker ID and corresponding height
 */
void PersonStorage::setMarkerHeights(const std::unordered_map<int, float> &heights)
{
    onManualAction();

    for(auto &person : mPersons) // over TrackPerson
    {
        for(const auto &point : person) // over TrackPoints
        {
            if(auto codeMarker = point.getCodeMarker())
            {
                // markerID of current person at current TrackPoint:
                int markerID = codeMarker->mMarkerId;
                // find index of mID within List of MarkerIDs that were read from txt-file:
                if(heights.find(markerID) != std::end(heights))
                {
                    person.setHeight(heights.at(markerID));
                }
                else
                {
                    SPDLOG_WARN("The following markerID was not part of the height-file: {}", markerID);
                    SPDLOG_WARN("No height set for personNR: {}", person.nr());
                }
            }
        }
    }
}

/**
 * Sets/Overwrites the markerID for a specific person and all trackpoints belonging to that person
 * @param personIndex internal id of persons (0 based)
 * @param markerIDs new marker ID
 */
void PersonStorage::setMarkerID(size_t personIndex, int markerID, bool manual)
{
    if(manual)
    {
        onManualAction();
    }
    auto &person = mPersons.at(personIndex);
    person.setMarkerID(markerID);
    person.syncTrackPersonMarkerID(markerID);
    for(int frame = person.firstFrame(); frame <= person.lastFrame(); ++frame)
    {
        person.updateMarkerID(frame, markerID);
    }
}

/**
 * Sets the marker IDs based on the internal used IDs (personID).
 * @param markerIDs Map between internal ID and marker ID
 */
void PersonStorage::setMarkerIDs(const std::unordered_map<int, int> &markerIDs)
{
    onManualAction();

    for(int i = 0; i < static_cast<int>(mPersons.size()); ++i) // over TrackPerson
    {
        // personID of current person
        int personID = i + 1;
        if(markerIDs.find(personID) != std::end(markerIDs))
        {
            int markerID = markerIDs.at(personID);
            setMarkerID(i, markerID);
        }
        else
        {
            SPDLOG_WARN("The following personID was not part of the markerID-file: {}", personID);
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
void PersonStorage::purge(int frame)
{
    onManualAction();

    int   i, j;
    float count; ///< number of trackpoints without recognition

    for(i = 0; i < static_cast<int>(mPersons.size()); ++i)
    {
        if(mPersons.at(i).size() > 10 && mPersons.at(i).firstFrame() <= frame && mPersons.at(i).lastFrame() >= frame)
        {
            count = 0;
            for(j = 0; j < mPersons.at(i).size(); ++j)
            {
                if(mPersons.at(i).at(j).qual() < 100.)
                {
                    ++count;
                }
            }
            if(count / mPersons.at(i).size() > 0.8) // Achtung, wenn roi klein, dann viele tp nur getrackt
            {
                deletePerson(i--); // after deleting the person decrease i by one
            }
        }
    }
}

void PersonStorage::undo()
{
    if(!mUndo.empty())
    {
        mRedo.push(std::move(mPersons));
        mPersons = mUndo.pop();
    }
}

void PersonStorage::redo()
{
    if(!mRedo.empty())
    {
        mUndo.push(std::move(mPersons));
        mPersons = mRedo.pop();
    }
}

/**
 * @brief This function is called on manual actions, i.e. non-tracking changes of trajectories
 */
void PersonStorage::onManualAction()
{
    mAutosave.trackPersonModified();
    mUndo.push(mPersons);
    mRedo.clear();
}

/**
 * @brief Smooths the height of a stereo point of a person
 * @param i index of person whose heights/z-coordinates to smooth
 * @param j index of the stereo point whose height to smooth
 */
void PersonStorage::smoothHeight(size_t i, int j)
{
    // ACHTUNG: Aenderungen in Originaltrajektorie, so dass aenderungen auf folgeuntersuchungen einfluss
    // haben: j auf j+1
    int  tsize      = mPersons[i].size();
    auto firstFrame = mPersons[i].firstFrame();

    if(auto stereoMarker = mPersons[i].at(j).getStereoMarker())
    {
        int nrFor = 1; // anzahl der ztrackpoint ohne hoeheninfo
        int nrRew = 1;

        // nach && wird nur ausgefuehrt, wenn erstes true == size() also nicht
        while((j + nrFor < tsize) && (!mPersons[i].at(j + nrFor).getStereoMarker()))
        {
            ++nrFor;
        }
        std::optional<StereoMarker> nrForStereoMarker = mPersons[i].at(j + nrFor).getStereoMarker();

        // nach && wird nur ausgefuehrt, wenn erstes true == size() also nicht
        while((j - nrRew >= 0) && (!mPersons[i].at(j - nrRew).getStereoMarker()))
        {
            ++nrRew;
        }
        std::optional<StereoMarker> nrRewStereoMarker = mPersons[i].at(j - nrRew).getStereoMarker();

        // nur oder eher in Vergangenheit hoeheninfo gefunden
        if(((j - nrRew >= 0) && (j + nrFor == tsize)) || ((j - nrRew >= 0) && (nrRew < nrFor)))
        {
            if(fabs(nrRewStereoMarker->mStereoPoint.z() - stereoMarker->mStereoPoint.z()) > nrRew * 40.) // 40cm
            {
                mPersons[i].updateStereoPoint(
                    j + mPersons[i].firstFrame(),
                    {stereoMarker->mStereoPoint.x(),
                     stereoMarker->mStereoPoint.y(),
                     nrRewStereoMarker->mStereoPoint.z()});
                SPDLOG_WARN(
                    "Trackpoint smoothed height at the end or next to unknown height in the future for trajectory {} "
                    "in frame {}.",
                    i + 1,
                    j + firstFrame);
            }
        }
        else if(
            ((j + nrFor != tsize) && (j - nrRew < 0)) ||
            ((j + nrFor != tsize) && (nrFor < nrRew))) // nur oder eher in der zukunft hoeheninfo gefunden
        {
            if(fabs(nrForStereoMarker->mStereoPoint.z() - stereoMarker->mStereoPoint.z()) > nrFor * 40.) // 40cm
            {
                mPersons[i].updateStereoPoint(
                    j + mPersons[i].firstFrame(),
                    {stereoMarker->mStereoPoint.x(),
                     stereoMarker->mStereoPoint.y(),
                     nrForStereoMarker->mStereoPoint.z()});
                SPDLOG_WARN(
                    "TrackPoint smoothed height at the beginning or next to unknown height in the past for trajectory "
                    "{} in frame {}.",
                    i + 1,
                    j + firstFrame);
            }
        }
        else if((j + nrFor != tsize) && (j - nrRew >= 0)) // in beiden richtungen hoeheninfo gefunden
                                                          // und nrFor==nrRew
        {
            // median genommen um zwei fehlmessungen nebeneinander nicht dazu fuehren zu lassen, dass
            // bessere daten veraendert werden
            auto zMedian = getMedianOf3(
                stereoMarker->mStereoPoint.z(),
                nrRewStereoMarker->mStereoPoint.z(),
                nrForStereoMarker->mStereoPoint.z());
            // lineare interpolation
            if(fabs(zMedian - stereoMarker->mStereoPoint.z()) > 20. * (nrFor + nrRew)) // 20cm
            {
                mPersons[i].updateStereoPoint(
                    j + mPersons[i].firstFrame(),
                    {stereoMarker->mStereoPoint.x(), stereoMarker->mStereoPoint.y(), zMedian});
                SPDLOG_WARN("Trackpoint smoothed height inside for trajectory {} in frame {}.", i + 1, j + firstFrame);
            }
        }
    }
}

/**
 * @brief Inserts the points to the corresponding person
 * @param person index of person to which to add a point
 * @param frame frame the TrackPoint is from
 * @param point the point to add
 * @param persNr persNr (same as person)
 * @param extrapolate whether far away point should be extrapolated
 * @param z z-coordiante of stereo point/distance to camera
 * @param height altitude of the camera
 */
void PersonStorage::insertFeaturePoint(
    size_t            person,
    int               frame,
    const TrackPoint &point,
    int               persNr,
    bool              useKalmanFilter,
    bool              extrapolate,
    float             z,
    float             height)
{
    if(mPersons.at(person).insertAtFrame(frame, point, persNr, useKalmanFilter, extrapolate) && z > -1)
    {
        mPersons[person].setHeight(z, height);
    }
}

/**
 * @brief Merge two trajectories into one
 * @param pers1 index of first trajectory
 * @param pers2 index of second trajectory
 * @return index of trajectory which was deleted in the process
 */
int PersonStorage::merge(int pers1, int pers2)
{
    onManualAction();

    auto      &person          = mPersons.at(pers1);
    auto      &other           = mPersons.at(pers2);
    const bool extrapolate     = mMainWindow.getControlWidget()->isTrackExtrapolationChecked();
    const bool useKalmanFilter = mMainWindow.getControlWidget()->isTrackUseKalmanChecked();
    int        deleteIndex;
    int        keepIndex;
    if(other.firstFrame() < person.firstFrame() && other.lastFrame() > person.lastFrame())
    {
        for(int k = 0; k < person.size(); ++k)
        {
            // bei insertAtFrame wird qual beruecksichtigt, ob vorheriger besser
            other.insertAtFrame(person.firstFrame() + k, person.at(k), pers2, useKalmanFilter, extrapolate);
        }
        deleteIndex = pers1;
        keepIndex   = pers2;
    }
    else if(other.firstFrame() < person.firstFrame())
    {
        for(int k = other.size() - 1; k > -1; --k)
        {
            // bei insertAtFrame wird qual beruecksichtigt, ob vorheriger besser
            person.insertAtFrame(other.firstFrame() + k, other.at(k), pers1, useKalmanFilter, extrapolate);
        }
        deleteIndex = pers2;
        keepIndex   = pers1;
    }
    else
    {
        for(int k = 0; k < other.size(); ++k)
        {
            // bei insertAtFrame wird qual beruecksichtigt, ob vorheriger besser
            person.insertAtFrame(other.firstFrame() + k, other.at(k), pers1, useKalmanFilter, extrapolate);
        }
        deleteIndex = pers2;
        keepIndex   = pers1;
    }
    deletePerson(deleteIndex);
    emit changedPerson(keepIndex);

    return deleteIndex;
}

std::vector<TrackPerson>::iterator PersonStorage::deletePerson(size_t index)
{
    auto retIt = mPersons.erase(mPersons.begin() + index);
    emit deletedPerson(index);
    return retIt;
}

void PersonStorage::deletePersonFrameRange(size_t index, int startFrame, int endFrame)
{
    mPersons[index].removeFramesBetween(startFrame, endFrame);
    emit deletedPersonFrameRange(index, startFrame, endFrame);
}
