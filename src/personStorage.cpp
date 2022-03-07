/*
 * PeTrack - Software for tracking pedestrians movement in videos
 * Copyright (C) 2010-2022 Forschungszentrum Jülich GmbH,
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

#include "personStorage.h"

#include "animation.h"
#include "control.h"
#include "multiColorMarkerWidget.h"
#include "pMessageBox.h"
#include "petrack.h"
#include "roiItem.h"

/**
 * @brief split trajectorie pers before frame frame
 * @param pers index of person
 * @param frame frame to split at
 */
void PersonStorage::splitPerson(size_t pers, int frame)
{
    int j;

    if(mPersons.at(pers).firstFrame() < frame)
    {
        mPersons.push_back(mPersons.at(pers));

        // alte trj einkuerzen und ab aktuellem frame zukunft loeschen
        for(j = 0; j < mPersons.at(pers).lastFrame() - frame + 1; ++j)
        {
            mPersons[pers].removeLast();
        }
        mPersons[pers].setLastFrame(frame - 1);

        // neu angehaengte/gedoppelte trajektorie
        for(j = 0; j < frame - mPersons.back().firstFrame(); ++j)
        {
            mPersons.back().removeFirst();
        }
        mPersons.back().setFirstFrame(frame);
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
bool PersonStorage::splitPersonAt(const Vec2F &point, int frame, const QSet<int> &onlyVisible)
{
    for(size_t i = 0; i < mPersons.size(); ++i)
    { // ueber TrackPerson
        if(((onlyVisible.empty()) || (onlyVisible.contains(i))) &&
           (mPersons.at(i).trackPointExist(frame) && (mPersons.at(i).trackPointAt(frame).distanceToPoint(point) <
                                                      mMainWindow.getHeadSize(nullptr, i, frame) / 2.)))
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
 * @param direction notes if previous (-1), following(1) or whole(0) trajectory should be deleted
 * @param frame
 * @return true, if deletion occured
 */
bool PersonStorage::delPointOf(int pers, int direction, int frame)
{
    if(direction == -1)
    {
        for(int j = 0; j < frame - mPersons.at(pers).firstFrame(); ++j)
        {
            mPersons[pers].removeFirst();
        }
        mPersons[pers].setFirstFrame(frame);
    }
    else if(direction == 0)
    {
        mPersons.erase(mPersons.begin() + pers);
    }
    else if(direction == 1)
    {
        for(int j = 0; j < mPersons.at(pers).lastFrame() - frame; ++j)
        {
            mPersons[pers].removeLast();
        }
        mPersons[pers].setLastFrame(frame);
    }

    return true;
}


/**
 * @brief Deletes points of a SINGLE person in onlyVisible
 * @param point point which need to be on the person (helpful if onlyVisible is not properly set)
 * @param direction notes if previous (-1), following(1) or whole(0) trajectory should be deleted
 * @param frame
 * @param onlyVisible set of people whose points could be deleted; empty means everyone
 * @return true if deletion occured
 */
bool PersonStorage::delPoint(const Vec2F &point, int direction, int frame, const QSet<int> &onlyVisible)
{
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
void PersonStorage::delPointAll(Direction direction, int frame)
{
    for(size_t i = 0; i < mPersons.size(); ++i) // ueber TrackPerson
    {
        if(mPersons.at(i).trackPointExist(frame))
        {
            switch(direction)
            {
                case Direction::Previous:
                    for(int j = 0; j < frame - mPersons.at(i).firstFrame(); ++j)
                    {
                        mPersons[i].removeFirst();
                    }
                    mPersons[i].setFirstFrame(frame);
                    break;
                case Direction::Whole:
                    mPersons.erase(mPersons.begin() + i--); // nach Loeschen wird i um 1 erniedrigt
                    break;
                case Direction::Following:
                    for(int j = 0; j < mPersons.at(i).lastFrame() - frame; ++j)
                    {
                        mPersons[i].removeLast();
                    }
                    mPersons[i].setLastFrame(frame);
                    break;
            }
        }
        else if(
            ((direction == Direction::Previous) && (frame > mPersons.at(i).lastFrame())) ||
            (direction == Direction::Whole) ||
            ((direction == Direction::Following) && (frame < mPersons.at(i).firstFrame())))
        {
            mPersons.erase(mPersons.begin() + i);
            i--;
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
                    mPersons.erase(mPersons.begin() + i);
                    i--;
                    inside = !inside;
                }
                break;
            }
        }
        if(inside)
        {
            // rest loeschen
            mPersons.erase(mPersons.begin() + i);
            i--;
        }
    }
}


/**
 * @brief deletes trajectory, if it is partly inside ROI
 */
void PersonStorage::delPointROI()
{
    int    anz  = 0;
    QRectF rect = mMainWindow.getRecoRoiItem()->rect();

    for(size_t i = 0; i < mPersons.size(); ++i) // ueber TrackPerson
    {
        for(int j = 0; j < mPersons.at(i).size(); ++j)
        {
            if(rect.contains(mPersons.at(i).at(j).x(), mPersons.at(i).at(j).y()))
            {
                anz++;
                mPersons.erase(mPersons.begin() + i);
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
bool PersonStorage::editTrackPersonComment(const Vec2F &point, int frame, const QSet<int> &onlyVisible)
{
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
            QString comment = QInputDialog::getMultiLineText(
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
bool PersonStorage::setTrackPersonHeight(const Vec2F &point, int frame, const QSet<int> &onlyVisible)
{
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
                                 -mMainWindow.getControlWidget()->mapDefaultHeight->value();
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
                    debout << "Warning: you entered a negative height!" << std::endl; // is not supported!" << endl;
                    // return false;
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
                    debout << std::endl
                           << "No height change detected. Color-mapped height will remain set." << std::endl;
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
bool PersonStorage::resetTrackPersonHeight(const Vec2F &point, int frame, QSet<int> onlyVisible)
{
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

// used for calculation of 3D point for all points in frame
// returns number of found points or -1 if no stereoContext available (also points without disp found are counted)
int PersonStorage::calcPosition(int /*frame*/)
{
#ifndef STEREO_DISABLED
    int                 anz = 0, notFoundDisp = 0;
    pet::StereoContext *sc = mMainWindow.getStereoContext();
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
                    mPersons[i][frame - at(i).firstFrame()].setSp(x, y, z); // setZdistanceToCam(z);
                    mPersons[i].setHeight(z, mMainWindow.getControlWidget()->coordAltitude->value());
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
    const QSet<int>        &onlyVisible,
    reco::RecognitionMethod method,
    int                    *pers)
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
    if(mMainWindow.getStereoContext() && mMainWindow.getStereoWidget()->stereoUseForHeight->isChecked())
    {
        if(mMainWindow.getStereoContext()->getMedianXYZaround(
               (int) p.x(), (int) p.y(), &x, &y, &z)) // nicht myRound, da pixel 0 von 0..0.99 in double geht
        {
            // statt altitude koennte hier irgendwann die berechnete Bodenhoehe einfliessen
            p.setSp(x, y, z); // setZdistanceToCam(z);
        }
        // cout << " " << point.x()<< " " << point.y() << " " << x << " " << y << " " << z <<endl;
        // if (i == 10)
        //     debout << i << " " << mMainWindow.getControlWidget()->coordAltitude->value() - z << " " << z << " " <<
        //     mPersons[i].height() << endl;
    }
#endif
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
            if((dist < scaleHead * mMainWindow.getHeadSize(nullptr, i, frame) / 2.) ||
               // fuer multifarbmarker mit schwarzem punkt wird nur farbmarker zur Abstandbetrachtung herangezogen
               // at(i).trackPointAt(frame).colPoint() existiert nicht an dieser stelle, da bisher nur getrackt
               // wurde!!!!
               (multiColorWithDot && point.color().isValid() &&
                (mPersons.at(i).trackPointAt(frame).distanceToPoint(point.colPoint()) <
                 mMainWindow.getHeadSize(nullptr, i, frame) / 2.)))
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
               (mMainWindow.getControlWidget()->trackExtrapolation->checkState() ==
                Qt::Checked))) // wenn eingefuegt wurde (bessere qualitaet)
        //|| !at(i).trackPointAt(frame).color().isValid() moeglich, um auch bei schlechterer
        // qualitaet aber aktuell nicht
        // vorliegender farbe die ermittelte farbe einzutragen - kommt nicht vor!
        {
            // Synchronize TrackPerson.markerID with TrackPoint.markerID
            mPersons[iNearest].syncTrackPersonMarkerID(point.getMarkerID());

            // set/add color
            if(point.color().isValid()) // not valid for manual, than old color is used
            {
                // if (at(i).trackPointAt(frame).color().isValid()) man koennte alte farbe abziehen - aber nicht noetig,
                // kommt nicht vor
                mPersons[iNearest].addColor(point.color());
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

        if(point.qual() > 100) // manual add
        {
            point.setQual(100);
        }
        mPersons.push_back(TrackPerson(
            0, frame, point, point.getMarkerID())); // 0 is person number/markerID; newReco is set to true by default
    }
    if((z > 0) && ((onlyVisible.empty()) || found))
    {
        mPersons[iNearest].setHeight(z, mMainWindow.getControlWidget()->coordAltitude->value()); // , frame
    }
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
        addPoint(point, frame, QSet<int>(), method);
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
 * @brief Recalcs the height of all persons (used with stereo)
 * @param altitude altitude of the camera (assumes orthogonal view?)
 */
void PersonStorage::recalcHeight(float altitude)
{
    for(auto &person : mPersons)
    {
        person.recalcHeight(altitude);
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
void PersonStorage::checkPlausibility(
    QList<int> &pers,
    QList<int> &frame,
    bool        testEqual,
    bool        testVelocity,
    bool        testInside,
    bool        testLength)
{
    QProgressDialog progress("Check Plausibility", nullptr, 0, 400, mMainWindow.window());
    progress.setWindowTitle("Check plausibility");
    progress.setWindowModality(Qt::WindowModal);
    progress.setVisible(true);
    progress.setValue(0);
    progress.setLabelText("Check Plausibility...");
    static int margin = 30; // rand am bild, ab dem trajectorie verloren sein darf
    int        i, j;
    double     x, y;
    QRectF     rect      = mMainWindow.getRecoRoiItem()->rect();
    int        lastFrame = mMainWindow.getAnimation()->getNumFrames() - 1;

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
        for(i = 0; i < static_cast<int>(mPersons.size()); ++i) // ueber TrackPerson
        {
            progress.setValue(static_cast<int>(i * 100. / mPersons.size()));
            qApp->processEvents();
            if(mPersons.at(i).size() < 10)
            {
                debout << "Warning: Trajectory of person " << i + 1 << " has less than 10 trackpoints!" << std::endl;
                pers.append(i + 1);
                frame.append(mPersons[i].firstFrame());
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
        for(i = 0; i < static_cast<int>(mPersons.size()); ++i) // ueber TrackPerson
        {
            qApp->processEvents();
            progress.setValue(100 + i * 100. / mPersons.size());
            x = mPersons[i].first().x();
            y = mPersons[i].first().y();
            // mGrey hat gleiche groesse wie zuletzt getracktes bild
            if(mPersons[i].firstFrame() != 0 && x >= MAX(margin, rect.x()) &&
               y >= MAX(margin, rect.y())) //&&
                                           // x <= MIN(mGrey.cols - 1 - 2 * bS - margin, rect.x() + rect.width()) &&
                                           // y <= MIN(mGrey.rows - 1 - 2 * bS - margin, rect.y() + rect.height()))
            {
                debout << "Warning: Start of trajectory inside picture and recognition area of person " << i + 1 << "!"
                       << std::endl;
                pers.append(i + 1);
                frame.append(mPersons[i].firstFrame());
            }

            x = mPersons[i].last().x();
            y = mPersons[i].last().y();
            // mGrey hat gleiche groesse wie zuletzt getracktes bild
            if(mPersons[i].lastFrame() != lastFrame && x >= MAX(margin, rect.x()) &&
               y >= MAX(margin, rect.y())) //&&
                                           // x <= MIN(mGrey.cols - 1 - 2 * bS - margin, rect.x() + rect.width()) &&
                                           // y <= MIN(mGrey.rows - 1 - 2 * bS - margin, rect.y() + rect.height()))
            {
                debout << "Warning: End of trajectory inside picture and recognition area of person " << i + 1 << "!"
                       << std::endl;
                pers.append(i + 1);
                frame.append(mPersons[i].lastFrame());
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
        for(i = 0; i < static_cast<int>(mPersons.size()); ++i) // ueber TrackPerson
        {
            qApp->processEvents();
            progress.setValue(200 + i * 100. / mPersons.size());
            for(j = 1; j < mPersons.at(i).size() - 2; ++j) // ueber TrackPoint (ohne ersten und letzten beiden)
            {
                d01 = mPersons.at(i).at(j).distanceToPoint(mPersons.at(i).at(j - 1));
                d12 = mPersons.at(i).at(j + 1).distanceToPoint(mPersons.at(i).at(j));
                d23 = mPersons.at(i).at(j + 2).distanceToPoint(mPersons.at(i).at(j + 1));
                if(((1.8 * (d01 + d23) / 2.) < d12) &&
                   ((d12 > 6.) ||
                    ((d01 + d23) / 2. > 3.))) // geschwindigkeit 1,8-fach && mindestpixelbewegung im schnitt von 3
                {
                    debout << "Warning: Fast variation of velocity of person " << i + 1 << " between frame "
                           << j + mPersons.at(i).firstFrame() << " and " << j + 1 + mPersons.at(i).firstFrame() << "!"
                           << std::endl;
                    pers.append(i + 1);
                    frame.append(j + mPersons.at(i).firstFrame());
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

            for(i = 0; i < static_cast<int>(mPersons.size()); ++i)
            {
                // if (!pers.contains(i+1)) man koennte nur einmal eine Person aufnehmen, da aufeinanderfolgende frames
                // oft betroffen
                for(j = i + 1; j < static_cast<int>(mPersons.size()); ++j)
                {
                    if(mPersons.at(i).trackPointExist(f) && mPersons.at(j).trackPointExist(f))
                    {
                        if(mPersons.at(i).trackPointAt(f).distanceToPoint(mPersons.at(j).trackPointAt(f)) <
                           mMainWindow.getHeadSize(nullptr, i, f) / 2.)
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

/// optimize color for all persons
void PersonStorage::optimizeColor()
{
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
    for(auto &person : mPersons)
    {
        person.resetHeight();
    }
}

/// reset the pos of the tzrackpoints, but not the heights
void PersonStorage::resetPos()
{
    for(auto &person : mPersons)
    {
        for(auto &point : person)
        {
            point.setSp(-1., -1., -1.);
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
    debout << std::endl;
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

    debout << "number of persons with measured height                            : " << anz << std::endl;
    debout << "person without measured height (not included in calculated values): " << noHeight
           << " (using default height for export)" << std::endl;
    if(anz == 0)
    {
        return false;
    }

    for(j = dict.constBegin(); j != dict.constEnd(); ++j)
    {
        debout << "height " << std::fixed << std::setprecision(1) << std::setw(5) << j.key() << " - "
               << j.key() + heightStep << " : number " << std::setw(3) << j.value() << " (" << std::setw(4)
               << (100. * j.value()) / anz << "%)" << std::endl;
        average += (j.key() + heightStep / 2.) * j.value();
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
void PersonStorage::setMarkerHeights(const std::unordered_map<int, float> &heights)
{
    for(auto &person : mPersons) // over TrackPerson
    {
        for(auto &point : person) // over TrackPoints
        {
            // markerID of current person at current TrackPoint:
            int markerID = point.getMarkerID();

            if(markerID != -1) // when a real markerID is found (not -1)
            {
                // find index of mID within List of MarkerIDs that were read from txt-file:
                if(heights.find(markerID) != std::end(heights))
                {
                    person.setHeight(heights.at(markerID));
                }
                else
                {
                    debout << "Warning, the following markerID was not part of the height-file: " << markerID
                           << std::endl;
                    debout << "No height set for personNR: " << person.nr() << std::endl;
                }
            }
        }
    }
}


/**
 * Sets the marker IDs based on the internal used IDs (personID).
 * @param markerIDs Map between internal ID and marker ID
 */
void PersonStorage::setMarkerIDs(const std::unordered_map<int, int> &markerIDs)
{
    for(int i = 0; i < static_cast<int>(mPersons.size()); ++i) // over TrackPerson
    {
        // personID of current person
        int personID = i + 1;
        if(markerIDs.find(personID) != std::end(markerIDs))
        {
            int markerID = markerIDs.at(personID);
            mPersons[i].setMarkerID(markerID);
            for(int j = 0; j < mPersons[i].size(); ++j) // over TrackPoints
            {
                mPersons[i][j].setMarkerID(markerID);
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
void PersonStorage::purge(int frame)
{
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
                mPersons.erase(mPersons.begin() + i); // delete trj}
            }
        }
    }
}

/**
 * @brief Smoothes the height of a stereo point of a person
 * @param i index of person whose heights/z-coordinates to smooth
 * @param j index of the stereo point whose height to smooth
 */
void PersonStorage::smoothHeight(size_t i, int j)
{
    // ACHTUNG: Aenderungen in Originaltrajektorie, so dass aenderungen auf folgeuntersuchungen einfluss
    // haben: j auf j+1
    int  tsize      = mPersons[i].size();
    auto firstFrame = mPersons[i].firstFrame();
    if(mPersons[i][j].sp().z() != -1)
    {
        int nrFor = 1; // anzahl der ztrackpoint ohne hoeheninfo
        int nrRew = 1;

        // nach && wird nur ausgefuehrt, wenn erstes true == size() also nicht
        while((j + nrFor < tsize) && (mPersons[i].at(j + nrFor).sp().z() < 0))
        {
            nrFor++;
        }

        // nach && wird nur ausgefuehrt, wenn erstes true == size() also nicht
        while((j - nrRew >= 0) && (mPersons[i].at(j - nrRew).sp().z() < 0))
        {
            nrRew++;
        }

        // nur oder eher in Vergangenheit hoeheninfo gefunden
        if(((j - nrRew >= 0) && (j + nrFor == tsize)) || ((j - nrRew >= 0) && (nrRew < nrFor)))
        {
            if(fabs(mPersons[i].at(j - nrRew).sp().z() - mPersons[i].at(j).sp().z()) > nrRew * 40.) // 40cm
            {
                mPersons[i][j].setSp(
                    mPersons[i].at(j).sp().x(), mPersons[i].at(j).sp().y(), mPersons[i].at(j - nrRew).sp().z());
                debout << "Warning: Trackpoint smoothed height at the end or next to unknown height in "
                          "the future for trajectory "
                       << i + 1 << " in frame " << j + firstFrame << "." << std::endl;
            }
        }
        else if(
            ((j + nrFor != tsize) && (j - nrRew < 0)) ||
            ((j + nrFor != tsize) && (nrFor < nrRew))) // nur oder eher in der zukunft hoeheninfo gefunden
        {
            if(fabs(mPersons[i].at(j + nrFor).sp().z() - mPersons[i].at(j).sp().z()) > nrFor * 40.) // 40cm
            {
                mPersons[i][j].setSp(
                    mPersons[i].at(j).sp().x(), mPersons[i].at(j).sp().y(), mPersons[i].at(j + nrFor).sp().z());
                debout << "Warning: Trackpoint smoothed height at the beginning or next to unknown "
                          "height in the past for trajectory "
                       << i + 1 << " in frame " << j + firstFrame << "." << std::endl;
            }
        }
        else if((j + nrFor != tsize) && (j - nrRew >= 0)) // in beiden richtungen hoeheninfo gefunden
                                                          // und nrFor==nrRew
        {
            // median genommen um zwei fehlmessungen nebeneinander nicht dazu fuehren zu lassen, dass
            // bessere daten veraendert werden
            auto zMedian = getMedianOf3(
                mPersons[i].at(j).sp().z(), mPersons[i].at(j - nrRew).sp().z(), mPersons[i].at(j + nrFor).sp().z());
            // lineare interpolation
            if(fabs(zMedian - mPersons[i].at(j).sp().z()) > 20. * (nrFor + nrRew)) // 20cm
            {
                mPersons[i][j].setSp(mPersons[i].at(j).sp().x(), mPersons[i].at(j).sp().y(), zMedian);
                debout << "Warning: Trackpoint smoothed height inside for trajectory " << i + 1 << " in frame "
                       << j + firstFrame << "." << std::endl;
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
    bool              extrapolate,
    float             z,
    float             height)
{
    if(mPersons.at(person).insertAtFrame(frame, point, persNr, extrapolate) && z > -1)
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
    auto      &person      = mPersons.at(pers1);
    auto      &other       = mPersons.at(pers2);
    const bool extrapolate = mMainWindow.getControlWidget()->trackExtrapolation->checkState() == Qt::Checked;
    int        deleteIndex;
    if(other.firstFrame() < person.firstFrame() && other.lastFrame() > person.lastFrame())
    {
        for(int k = 0; k < person.size(); ++k)
        {
            // bei insertAtFrame wird qual beruecksichtigt, ob vorheriger besser
            other.insertAtFrame(person.firstFrame() + k, person[k], pers2, extrapolate);
        }
        deleteIndex = pers1;
    }
    else if(other.firstFrame() < person.firstFrame())
    {
        for(int k = other.size() - 1; k > -1; --k)
        {
            // bei insertAtFrame wird qual beruecksichtigt, ob vorheriger besser
            person.insertAtFrame(other.firstFrame() + k, other[k], pers1, extrapolate);
        }
        deleteIndex = pers2;
    }
    else
    {
        for(int k = 0; k < other.size(); ++k)
        {
            // bei insertAtFrame wird qual beruecksichtigt, ob vorheriger besser
            person.insertAtFrame(other.firstFrame() + k, other[k], pers1, extrapolate);
        }
        deleteIndex = pers2;
    }
    mPersons.erase(mPersons.begin() + deleteIndex);

    return deleteIndex;
}
