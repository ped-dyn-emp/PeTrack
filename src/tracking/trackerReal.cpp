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

#include "trackerReal.h"

#include "animation.h"
#include "calibFilter.h"
#include "control.h"
#include "helper.h"
#include "logger.h"
#include "personStorage.h"
#include "petrack.h"
#include "player.h"
#include "recognition.h"
#include "worldImageCorrespondence.h"

#include <H5Cpp.h>
#include <fstream>
#include <opencv2/highgui.hpp>

TrackPointReal::TrackPointReal(const Vec3F &p, int frameNum) : Vec3F(p), mFrameNum(frameNum)
{
    mAngleOfView = -1;
    mMarkerID    = -1;
}

TrackPointReal::TrackPointReal(const Vec3F &p, int frameNum, const Vec2F &d) :
    Vec3F(p), mFrameNum(frameNum), mViewDir(d)
{
    mAngleOfView = -1;
    mMarkerID    = -1;
}

//--------------------------------------------------------------------------

// the list index is the frame number plus mFirstFrame 0..mLastFrame-mFirstFrame
// no frame is left blank
TrackPersonReal::TrackPersonReal() : mHeight(-1.), mFirstFrame(-1), mLastFrame(0) {}
TrackPersonReal::TrackPersonReal(int frame, const TrackPointReal &p) :
    mHeight(-1.), mFirstFrame(frame), mLastFrame(frame)
{
    append(p);
}

bool TrackPersonReal::trackPointExist(int frame) const
{
    if(frame >= mFirstFrame && frame <= mLastFrame)
    {
        return true;
    }
    else
    {
        return false;
    }
}
const TrackPointReal &
TrackPersonReal::trackPointAt(int frame) const // & macht bei else probleme, sonst mit [] zugreifbar
{
    return at(frame - mFirstFrame);
}

// gibt -1 zurueck, wenn frame oder naechster frame nicht existiert
// entfernung ist absolut
double TrackPersonReal::distanceToNextFrame(int frame) const
{
    if(frame >= mFirstFrame && frame + 1 <= mLastFrame)
    {
        return at(frame - mFirstFrame).distanceToPoint(at(frame - mFirstFrame + 1));
    }
    else
    {
        return -1;
    }
}
void TrackPersonReal::init(int firstFrame, double height, int markerID, const QString &comment)
{
    clear();
    mFirstFrame = firstFrame;
    mLastFrame  = firstFrame - 1;
    mHeight     = height;
    mMarkerID   = markerID;
    mComment    = comment;
}

void TrackPersonReal::addEnd(const QPointF &pos, int frame)
{
    Vec3F point(pos.x(), pos.y(), -1.);
    addEnd(point, frame);
}
void TrackPersonReal::addEnd(const Vec3F &pos, int frame)
{
    ++mLastFrame;
    append(TrackPointReal(pos, frame));
}

void TrackPersonReal::addEnd(const QPointF &pos, int frame, const QPointF &dir)
{
    Vec3F point(pos.x(), pos.y(), -1.);
    Vec2F viewDirection(dir.x(), dir.y());
    ++mLastFrame;
    viewDirection.normalize();
    append(TrackPointReal(point, frame, viewDirection));
}

//-------------------------------------------------------------------------------------------------------------------------------------

TrackerReal::TrackerReal(QWidget *wParent, PersonStorage &storage) :
    mXMin(100000.), mXMax(-100000.), mYMin(100000.), mYMax(-100000.), mPersonStorage(storage)
{
    mMainWindow = (class Petrack *) wParent;
}

// default: int imageBorderSize = 0, bool missingFramesInserted = true, bool useTrackpoints = false
int TrackerReal::calculate(
    Petrack                        *petrack,
    Tracker                        *tracker,
    const WorldImageCorrespondence *worldImageCorr,
    ColorPlot                      *colorPlot,
    MissingFrames                  &missingFrames,
    int                             imageBorderSize,
    bool                            missingFramesInserted,
    bool                            useTrackpoints,
    bool                            alternateHeight,
    double                          altitude,
    bool                            useCalibrationCenter,
    bool                            exportElimTp,
    bool                            exportElimTrj,
    bool                            exportSmooth,
    bool                            exportViewingDirection,
    bool                            exportAngleOfView,
    bool                            exportMarkerID,
    bool                            exportAutoCorrect)
{
    if(tracker || colorPlot)
    {
        if(size() > 0)
        {
            clear();
        }

        int        j, f;
        QList<int> missingList;    // frame nr wo ausgelassen; passend dazu:
        QList<int> missingListAnz; // anzahl ausgelassener frames
        if(missingFramesInserted)
        {
            if(!missingFrames.isExecuted())
            {
                missingFrames.setMissingFrames(computeDroppedFrames(petrack));
                missingFrames.setExecuted(true);
            }

            std::transform(
                missingFrames.getMissingFrames().begin(),
                missingFrames.getMissingFrames().end(),
                std::back_inserter(missingList),
                [](auto const &missingFrame) { return static_cast<int>(missingFrame.mNumber); });
            std::transform(
                missingFrames.getMissingFrames().begin(),
                missingFrames.getMissingFrames().end(),
                std::back_inserter(missingListAnz),
                [](auto const &missingFrame) { return missingFrame.mCount; });
        }

        // fps ist nicht aussagekraeftig, da sie mgl von ausgelassenen herruehren - besser immer 25,01 fps annehmen

        double          height; // groesse in cm
        int             firstFrame, addFrames, anz;
        Vec2F           br(imageBorderSize, imageBorderSize);
        QPointF         pos, pos2;
        QList<int>      tmpMissingList;    // frame nr
        QList<int>      tmpMissingListAnz; // anzahl frames
        TrackPersonReal trackPersonReal;
        auto            imgRect = mMainWindow->getImage()->size();
        QPointF         center  = worldImageCorr->getPosReal(QPointF(imgRect.width() / 2., imgRect.height() / 2.), 0.);
        Vec3F           sp;
        int             tsize;
        int             extrapolated;
        QPointF         colPos;
        float           angle;
        QString         comment;

        const auto &persons = mPersonStorage.getPersons();
        for(size_t i = 0; i < persons.size(); ++i) // ueber trajektorien
        {
            const auto &person = persons[i];
            addFrames          = 0;
            firstFrame         = person.firstFrame();
            comment            = person.comment();
            if(person.height() < MIN_HEIGHT + 1)
            {
                height = colorPlot->map(person.color());
            }
            else
            {
                height = person.height();
            }

            if(missingList.size() > 0)
            {
                tmpMissingList    = missingList;
                tmpMissingListAnz = missingListAnz;
                // vorspulen
                while((tmpMissingList.size() > 0) && (tmpMissingList.first() < firstFrame))
                {
                    tmpMissingList.removeFirst();               // frame
                    addFrames += tmpMissingListAnz.takeFirst(); // anzahl
                }
            }

            int markerID = person.getMarkerID();
            trackPersonReal.init(firstFrame + addFrames, height, markerID, comment);
            tsize = person.size();
            for(j = 0; (j < tsize); ++j) // ueber trackpoints
            {
                Vec2F moveDir(0, 0); // used for head direction
                // ausreisser ausfindig machen (dies geschieht, bevor -1 elemente herausgenommen werden, um die
                // glaettung beim eliminieren der -1 elemente hier nicht einfluss nehmen zu lassen):
                if(exportSmooth && (useTrackpoints || alternateHeight) &&
                   (tsize > 1)) // wenn direkt pointgrey hoehe oder eigene hoehenberechnung aber variierend ueber
                                // trj genommen werden soll
                {
                    // changes Trajectories!
                    mPersonStorage.smoothHeight(i, j + person.firstFrame());
                }
                if(useTrackpoints)
                {
                    // border unberuecksichtigt
                    if(useCalibrationCenter)
                    {
                        trackPersonReal.addEnd(
                            Vec3F(
                                person.at(j).sp().x() + center.x(),
                                center.y() - person.at(j).sp().y(),
                                altitude - person.at(j).sp().z()),
                            firstFrame + j);
                    }
                    else
                    {
                        trackPersonReal.addEnd(person.at(j).sp(), firstFrame + j);
                    }
                }
                else
                {
                    if(alternateHeight) // personenhoehe variiert ueber trajektorie (unebene versuche); berechnung
                                        // durch mich und nicht pointgrey nutzen, Kamera altitude nutzen
                    {
                        extrapolated = 0; // 0 == false
                        double bestZ =
                            person.getNearestZ(j, &extrapolated); // gibt z wert zurueck bzw wenn -1, dann den
                                                                  // neben diesem frame ersten z-wert ungleich -1
                        if(bestZ < 0)                             // == -1 // es liegt gar keine berechnete hoehe vor
                        {
                            if(exportElimTrj)
                            {
                                SPDLOG_WARN(
                                    "no calculated height for TrackPoint {} (frame {}) of person {}, person is not "
                                    "exported!",
                                    j,
                                    person.firstFrame() + j,
                                    i + 1);
                                break; // TrackPerson ist angelegt, erhaelt aber keine Points und wird deshalb am ende
                                       // nicht eingefuegt
                            }
                            else
                            {
                                bestZ = height; // wenn gar kein trackpoint ungleich -1 gefunden wird, dann wird zu
                                                // allerletzt Hoehe genommen
                                SPDLOG_WARN(
                                    "no calculated height for TrackPoint {} (frame {}) of person {}, default height is "
                                    "used!",
                                    j,
                                    person.firstFrame() + j,
                                    i + 1);
                            }
                        }
                        else
                        {
                            bestZ = altitude - bestZ;
                        }
                        // ab hier kann bestZ auch negativ sein, obwohl hoehe inhalt hatte

                        if(extrapolated && exportElimTp)
                        {
                            if(extrapolated == 1) // zu beginn verworfen
                            {
                                trackPersonReal.setFirstFrame(trackPersonReal.firstFrame() + 1);
                            }
                            SPDLOG_WARN(
                                "no calculated height for TrackPoint {} (frame {}) of person {}, extrapolated height "
                                "not used, TrackPoint not inserted!",
                                j,
                                person.firstFrame() + j,
                                i + 1);
                        }
                        else
                        {
                            if(extrapolated)
                            {
                                SPDLOG_WARN(
                                    "no calculated height for TrackPoint {} (frame {}) of person {}, extrapolated "
                                    "height is used!",
                                    j,
                                    person.firstFrame() + j,
                                    i + 1);
                            }
                            if(exportAutoCorrect)
                            {
                                moveDir += reco::autoCorrectColorMarker(person.at(j), mMainWindow->getControlWidget());
                            }

                            pos = worldImageCorr->getPosReal((person.at(j) + moveDir + br).toQPointF(), bestZ);
                            trackPersonReal.addEnd(Vec3F(pos.x(), pos.y(), bestZ), firstFrame + j);
                        }
                    }
                    else
                    {
                        if(exportAutoCorrect)
                        {
                            moveDir += reco::autoCorrectColorMarker(person.at(j), mMainWindow->getControlWidget());
                        }

                        pos = worldImageCorr->getPosReal((person.at(j) + moveDir + br).toQPointF(), height);
                        trackPersonReal.addEnd(pos, firstFrame + j);
                        if(exportAngleOfView)
                        {
                            angle = (90. - worldImageCorr->getAngleToGround(
                                               (person.at(j) + br).x(), (person.at(j) + br).y(), height)) *
                                    PI / 180.;
                            trackPersonReal.last().setAngleOfView(angle);
                        }
                        if(exportMarkerID)
                        {
                            trackPersonReal.last().setMarkerID(person.at(j).getMarkerID());
                        }
                    }
                }

                if(exportViewingDirection)
                {
                    auto method = petrack->getControlWidget()->getRecoMethod();

                    // multicolor markers can also be used together with code markers
                    if(method == reco::RecognitionMethod::Code || method == reco::RecognitionMethod::MultiColor)
                    {
                        auto orientation = person.at(j).getOrientation();
                        orientation      = petrack->getExtrCalibration()->camToWorldRotation(orientation);
                        trackPersonReal.last().setViewDirection(Vec2F(orientation[0], orientation[1]).unit());
                    }
                    else
                    {
                        // old implementation for expeortViewingDirection did not check for specific marker, so just use
                        // the else
                        // die frame nummer der animation wird TrackPoint der PersonReal mitgegeben,
                        // da Index groesser sein kann, da vorher frames hinzugefuegt wurden duch
                        // trackPersonReal.init(firstFrame+addFrames, height) oder aber innerhalb des trackink path
                        // mit for schleife ueber f
                        if((exportViewingDirection) &&
                           (person.at(j).color().isValid())) // wenn blickrichtung mit ausgegeben werden soll
                        {
                            colPos = worldImageCorr->getPosReal(
                                (person.at(j).colPoint() + moveDir + br).toQPointF(), height);
                            trackPersonReal.last().setViewDirection(colPos - pos);
                        }
                    }
                }

                if(tmpMissingList.size() > 0)
                {
                    if((tmpMissingList.first() == firstFrame + j) && (person.trackPointExist(firstFrame + j + 1)))
                    {
                        tmpMissingList.removeFirst();        // frame
                        anz = tmpMissingListAnz.takeFirst(); // anzahl
                        if(useTrackpoints)
                        {
                            // border unberuecksichtigt
                            for(f = 1; f <= anz; ++f)
                            {
                                sp = person.at(j).sp() + f * (person.at(j + 1).sp() - person.at(j).sp()) / (anz + 1);
                                if(useCalibrationCenter)
                                {
                                    trackPersonReal.addEnd(
                                        Vec3F(sp.x() + center.x(), center.y() - sp.y(), altitude - sp.z()),
                                        firstFrame + j);
                                }
                                else
                                {
                                    trackPersonReal.addEnd(sp, -1); // -1 zeigt an, dass nur interpoliert
                                }
                            }
                        }
                        else
                        {
                            if(alternateHeight) // personenhoehe variiert ueber trajektorie (unebene versuche);
                                                // berechnung durch mich und nicht pointgrey nutzen
                            {
                                SPDLOG_WARN("no interpolation is done, because alternate height is enabled - has to be "
                                            "implemented!");
                            }
                            else
                            {
                                // use local variable and just reset the value
                                moveDir.set(0, 0);
                                if(exportAutoCorrect)
                                {
                                    moveDir +=
                                        reco::autoCorrectColorMarker(person.at(j), mMainWindow->getControlWidget());
                                }

                                pos2 =
                                    (worldImageCorr->getPosReal((person.at(j + 1) + moveDir + br).toQPointF(), height) -
                                     pos) /
                                    (anz + 1);
                                for(f = 1; f <= anz; ++f)
                                {
                                    trackPersonReal.addEnd(pos + f * pos2, -1); // -1 zeigt an, dass nur interpoliert
                                }
                            }
                        }
                    }
                    else if(tmpMissingList.first() < firstFrame) // while, wenn nicht kontinuierlich waere
                    {
                        tmpMissingList.removeFirst();    // frame
                        tmpMissingListAnz.removeFirst(); // anzahl
                    }
                }
            }

            tsize                  = trackPersonReal.size();
            double maxHeightDiff   = 30.; // 30cm
            int    numBorderPoints = 50;
            int    k;
            int    delNumFront = 0;
            // die ersten und letzten numBorderPoints trackpoints untersuchen, ob die hoehe einen sprung groesser
            // maxHeightDiff aufweist
            for(j = 1; j < tsize; ++j) // ueber trackpoints of personreal; ab 1, da vergleich zu vorherigem
            {
                if((j < numBorderPoints) &&
                   ((trackPersonReal.at(j).z() - trackPersonReal.at(j - 1).z()) > maxHeightDiff))
                {
                    delNumFront = j;
                }
                else if(
                    ((tsize - numBorderPoints) < j) &&
                    ((trackPersonReal.at(j).z() - trackPersonReal.at(j - 1).z()) > maxHeightDiff))
                {
                    for(k = j; k < tsize; k++)
                    {
                        trackPersonReal.setLastFrame(trackPersonReal.lastFrame() - 1);
                        trackPersonReal.removeLast();
                    }
                    SPDLOG_WARN("delete last {} frames of person {} because of height jump!", tsize - j, i + 1);
                    break;
                }
            }
            tsize = trackPersonReal.size();
            if(delNumFront > 0)
            {
                SPDLOG_WARN("delete first {} frames of person {} because of height jump!", delNumFront, i + 1);

                for(k = 0; (k < delNumFront) && (k < tsize); k++)
                {
                    trackPersonReal.setFirstFrame(trackPersonReal.firstFrame() + 1);
                    trackPersonReal.removeFirst();
                }
            }
            if(trackPersonReal.size() < 20)
            {
                SPDLOG_WARN("person {} has only {} TrackPoints!", i + 1, trackPersonReal.size());
            }
            if(trackPersonReal.size() > 0)
            {
                append(trackPersonReal);
            }
            else // ggf weil keine calculated height vorlag (siehe exportElimTrj)
            {
                SPDLOG_WARN("person {} is not inserted, because of no TrackPoints!", i + 1);
            }
        }
        return size();
    }
    else
    {
        SPDLOG_WARN("no real tracking data calculated, because of missing tracking data, image reference or "
                    "color map!");
        return -1;
    }
}

void TrackerReal::calcMinMax()
{
    Vec3F pos;
    for(int i = 0; i < size(); ++i)
    {
        for(int j = 0; j < at(i).size(); ++j)
        {
            pos = at(i).at(j);
            if(mXMin > pos.x())
            {
                mXMin = pos.x();
            }
            else if(mXMax < pos.x())
            {
                mXMax = pos.x();
            }
            if(mYMin > pos.y())
            {
                mYMin = pos.y();
            }
            else if(mYMax < pos.y())
            {
                mYMax = pos.y();
            }
        }
    }
}

int TrackerReal::largestFirstFrame()
{
    int max = -1, i;
    for(i = 0; i < size(); ++i)
    {
        if(at(i).firstFrame() > max)
        {
            max = at(i).firstFrame();
        }
    }
    return max;
}
int TrackerReal::largestLastFrame()
{
    int max = -1, i;
    for(i = 0; i < size(); ++i)
    {
        if(at(i).lastFrame() > max)
        {
            max = at(i).lastFrame();
        }
    }
    return max;
}
int TrackerReal::smallestFirstFrame()
{
    int i, min = ((size() > 0) ? at(0).firstFrame() : -1);
    for(i = 1; i < size(); ++i)
    {
        if(at(i).firstFrame() < min)
        {
            min = at(i).firstFrame();
        }
    }
    return min;
}
int TrackerReal::smallestLastFrame()
{
    int i, min = ((size() > 0) ? at(0).lastFrame() : -1);
    for(i = 1; i < size(); ++i)
    {
        if(at(i).lastFrame() < min)
        {
            min = at(i).lastFrame();
        }
    }
    return min;
}

void TrackerReal::exportTxt(
    QTextStream &out,
    bool         alternateHeight,
    bool         useTrackpoints,
    bool         exportViewingDirection,
    bool         exportAngleOfView,
    bool         exportUseM,
    bool         exportMarkerID)
{
    float scale;

    out << "# z: can be 3d position or height of person (alternating or not)" << Qt::endl;
    if(exportViewingDirection)
    {
        out << "# viewDirX viewDirY: vector of direction of head of person" << Qt::endl;
    }
    if(exportAngleOfView)
    {
        out << "# viewAngle: angle of view of camera to person from perpendicular [0..Pi/2]" << Qt::endl;
    }
    if(exportUseM)
    {
        out << "# id frame x/m y/m z/m";
    }
    else
    {
        out << "# id frame x/cm y/cm z/cm";
    }
    if(exportViewingDirection)
    {
        out << " viewDirX viewDirY";
    }
    if(exportAngleOfView)
    {
        out << " viewAngle";
    }
    if(exportMarkerID)
    {
        out << " markerID";
    }

    out << Qt::endl;


    if(exportUseM)
    {
        scale = .01f;
    }
    else
    {
        scale = 1.f;
    }

    QProgressDialog progress("Export TXT-File", nullptr, 0, size(), mMainWindow->window());
    progress.setWindowTitle("Export .txt-File");
    progress.setWindowModality(Qt::WindowModal);
    progress.setVisible(true);
    progress.setValue(0);
    progress.setLabelText(QString("Export tracking data ..."));
    qApp->processEvents();

    for(int i = 0; i < size(); ++i)
    {
        qApp->processEvents();
        progress.setLabelText(QString("Export person %1 of %2 ...").arg(i + 1).arg(size()));
        progress.setValue(i + 1);

        for(int j = 0; j < at(i).size(); ++j)
        {
            out << i + 1 << " " << at(i).firstFrame() + j << " " << at(i).at(j).x() * scale << " "
                << at(i).at(j).y() * scale << " ";

            if(alternateHeight || useTrackpoints)
            {
                out << at(i).at(j).z() * scale;
            }
            else
            {
                out << at(i).height() * scale;
            }

            if(exportViewingDirection) // && (at(i).at(j).viewDir() != Vec2F(0,0)) zeigt an, dass keine richtung
                                       // berechnet werden konnte
            {
                out << " " << at(i).at(j).viewDir();
            }

            if(exportAngleOfView)
            {
                out << " " << at(i).at(j).angleOfView();
            }

            if(exportMarkerID)
            {
                out << " " << at(i).getMarkerID();
            }

            out << Qt::endl;
        }
    }
}

void TrackerReal::createHdf5Attribute(H5::H5Object &obj, const std::string &name, const std::string &description)
{
    try
    {
        H5::StrType varLenStrType(H5::PredType::C_S1, H5T_VARIABLE);
        varLenStrType.setCset(H5T_CSET_UTF8);
        H5::DataSpace dataSpace(H5S_SCALAR);
        H5::Attribute attr               = obj.createAttribute(name, varLenStrType, dataSpace);
        const char   *descriptionPointer = description.c_str();
        attr.write(varLenStrType, static_cast<const void *>(&descriptionPointer));
    }
    catch(const H5::AttributeIException &e)
    {
        SPDLOG_ERROR("Failed to create/write attribute '{}': {}", name, e.getCDetailMsg());
    }
}

void TrackerReal::exportHdf5(
    const QString &filename,
    float          fps,
    bool           alternateHeight,
    bool           useTrackpoints,
    bool           exportViewingDirection,
    bool           exportAngleOfView,
    bool           exportMarkerID,
    bool           exportComment)
{
    try
    {
        float           scale = .01f; // should always export meters
        QProgressDialog progress("Export HDF5-File", nullptr, 0, size(), mMainWindow);
        progress.setWindowTitle("Export .hdf5-File");
        progress.setWindowModality(Qt::WindowModal);
        progress.setVisible(true);
        progress.setValue(0);
        progress.setLabelText(QString("Export tracking data ..."));
        qApp->processEvents();
        std::vector<TrackPointInfoHdf5>  data;
        std::vector<PersonalDetailsHdf5> personalDetailsData;

        for(int i = 0; i < size(); ++i)
        {
            qApp->processEvents();
            progress.setLabelText(QString("Export person %1 of %2 ...").arg(i + 1).arg(size()));
            progress.setValue(i + 1);
            const auto person = at(i);
            for(int j = 0; j < person.size(); ++j)
            {
                TrackPointInfoHdf5 point;
                point.id    = i + 1;
                point.frame = person.firstFrame() + j;
                point.x     = person.at(j).x() * scale;
                point.y     = person.at(j).y() * scale;
                if(alternateHeight || useTrackpoints)
                {
                    point.z = static_cast<float>(person.at(j).z() * scale);
                }
                else
                {
                    point.z = person.height() * scale;
                }
                if(exportViewingDirection)
                {
                    point.viewDirX = person.at(j).viewDir().x();
                    point.viewDirY = person.at(j).viewDir().y();
                }
                if(exportAngleOfView)
                {
                    point.viewAngle = person.at(j).angleOfView();
                }
                data.push_back(point);
            }

            // personal details
            PersonalDetailsHdf5 details;
            details.id       = i + 1;
            details.markerId = person.getMarkerID();
            details.height   = person.height() * scale;
            details.comment  = person.getComment().toStdString();
            personalDetailsData.push_back(details);
        }

        H5::Exception::dontPrint();
        H5::H5File   file(filename.toStdString(), H5F_ACC_TRUNC);
        H5::CompType datatype(sizeof(TrackPointInfoHdf5));
        datatype.insertMember("id", HOFFSET(TrackPointInfoHdf5, id), H5::PredType::NATIVE_INT);
        datatype.insertMember("frame", HOFFSET(TrackPointInfoHdf5, frame), H5::PredType::NATIVE_INT);
        datatype.insertMember("x", HOFFSET(TrackPointInfoHdf5, x), H5::PredType::NATIVE_FLOAT);
        datatype.insertMember("y", HOFFSET(TrackPointInfoHdf5, y), H5::PredType::NATIVE_FLOAT);
        datatype.insertMember("z", HOFFSET(TrackPointInfoHdf5, z), H5::PredType::NATIVE_FLOAT);
        if(exportViewingDirection)
        {
            datatype.insertMember("viewDirX", HOFFSET(TrackPointInfoHdf5, viewDirX), H5::PredType::NATIVE_FLOAT);
            datatype.insertMember("viewDirY", HOFFSET(TrackPointInfoHdf5, viewDirY), H5::PredType::NATIVE_FLOAT);
        }
        if(exportAngleOfView)
        {
            datatype.insertMember("viewAngle", HOFFSET(TrackPointInfoHdf5, viewAngle), H5::PredType::NATIVE_FLOAT);
        }

        hsize_t       dims[1] = {data.size()};
        H5::DataSpace dataspace(1, dims);

        H5::DataSet   dataset = file.createDataSet("trajectory", datatype, dataspace);
        H5::Attribute fpsAttribute =
            dataset.createAttribute("fps", H5::PredType::NATIVE_FLOAT, H5::DataSpace(H5S_SCALAR));
        fpsAttribute.write(H5::PredType::NATIVE_FLOAT, &fps);
        H5::StrType var_str_type(H5::PredType::C_S1, H5T_VARIABLE);
        var_str_type.setCset(H5T_CSET_UTF8);

        createHdf5Attribute(dataset, "id", "unique identifier for pedestrian");
        createHdf5Attribute(dataset, "frame", "frame number");
        createHdf5Attribute(dataset, "x", "pedestrian x-coordinate (meter [m])");
        createHdf5Attribute(dataset, "y", "pedestrian y-coordinate (meter [m])");
        createHdf5Attribute(dataset, "z", "pedestrian z-coordinate (meter [m])");

        dataset.write(data.data(), datatype);

        // personal details dataset
        H5::CompType personalDatatype(sizeof(PersonalDetailsHdf5));
        personalDatatype.insertMember("id", HOFFSET(PersonalDetailsHdf5, id), H5::PredType::NATIVE_INT);
        if(exportMarkerID)
        {
            personalDatatype.insertMember("markerId", HOFFSET(PersonalDetailsHdf5, markerId), H5::PredType::NATIVE_INT);
        }
        personalDatatype.insertMember("height", HOFFSET(PersonalDetailsHdf5, height), H5::PredType::NATIVE_FLOAT);
        if(exportComment)
        {
            personalDatatype.insertMember("comment", HOFFSET(PersonalDetailsHdf5, comment), var_str_type);
        }
        hsize_t       personalDims[1] = {personalDetailsData.size()};
        H5::DataSpace personalDataspace(1, personalDims);
        H5::DataSet   personalDataset = file.createDataSet("personal_details", personalDatatype, personalDataspace);

        createHdf5Attribute(personalDataset, "id", "unique identifier for pedestrian");
        createHdf5Attribute(personalDataset, "markerId", "marker identifier for pedestrian");
        createHdf5Attribute(personalDataset, "height", "pedestrian height (meter[m])");
        createHdf5Attribute(personalDataset, "comment", "comment about pedestrian");

        personalDataset.write(personalDetailsData.data(), personalDatatype);

        SPDLOG_INFO("Finished");
    }

    catch(const H5::FileIException &e)
    {
        throw std::runtime_error("HDF5 File error: " + std::string(e.getCDetailMsg()));
    }
    catch(const H5::DataSetIException &e)
    {
        throw std::runtime_error("HDF5 Dataset error: " + std::string(e.getCDetailMsg()));
    }
    catch(const H5::DataSpaceIException &e)
    {
        throw std::runtime_error("HDF5 Dataspace error: " + std::string(e.getCDetailMsg()));
    }
    catch(const H5::AttributeIException &e)
    {
        throw std::runtime_error("HDF5 Attribute error: " + std::string(e.getCDetailMsg()));
    }
    catch(const H5::Exception &e)
    {
        throw std::runtime_error("HDF5 Error: " + std::string(e.getCDetailMsg()));
    }
}


// old - not all export options supported!!!!
void TrackerReal::exportDat(QTextStream &out, bool alternateHeight, bool useTrackpoints) // fuer gnuplot
{
    QProgressDialog progress("Export DAT-File", nullptr, 0, size(), mMainWindow->window());
    progress.setWindowTitle("Export .dat-File");
    progress.setWindowModality(Qt::WindowModal);
    progress.setVisible(true);
    progress.setValue(0);
    progress.setLabelText(QString("Export tracking data ..."));
    SPDLOG_INFO("size: {}", size());
    if(useTrackpoints)
    {
        for(int i = 0; i < size(); ++i)
        {
            progress.setLabelText(QString("Export person %1 of %2 ...").arg(i + 1).arg(size()));
            qApp->processEvents();
            progress.setValue(i + 1);

            for(int j = 0; j < at(i).size(); ++j)
            {
                // Umrechnung in coordSystem fehlt, auch camera altitude unberuecksichtigt!!!
                out << at(i).firstFrame() + j << " " << at(i).at(j) << " " << at(i).at(j).z()
                    << Qt::endl; // z Koordinate ist Kopf
            }
            out << Qt::endl;
        }
    }
    else
    {
        for(int i = 0; i < size(); ++i)
        {
            progress.setLabelText(QString("Export person %1 of %2 ...").arg(i + 1).arg(size()));
            qApp->processEvents();
            progress.setValue(i + 1);
            SPDLOG_INFO("person: {}", i);

            for(int j = 0; j < at(i).size(); ++j)
            {
                if(alternateHeight)
                {
                    out << at(i).firstFrame() + j << " " << at(i).at(j) << " " << at(i).at(j).z()
                        << Qt::endl; // z Koordinate ist Kopf
                }
                else
                {
                    out << at(i).firstFrame() + j << " " << at(i).at(j) << " " << at(i).height()
                        << Qt::endl; // z Koordinate ist Kopf
                }
            }
            out << Qt::endl;
        }
    }
}

void TrackerReal::exportXml(QTextStream &outXml, bool alternateHeight, bool useTrackpoints)
{
    int    i, j, largestLastFr = largestLastFrame();
    double z;
    int    defaultPersonHeight = 176;

    QProgressDialog progress("Export XML-File", nullptr, 0, largestLastFr, mMainWindow->window());
    progress.setWindowTitle("Export .xml-File");
    progress.setWindowModality(Qt::WindowModal);
    progress.setVisible(true);
    progress.setValue(0);
    progress.setLabelText(QString("Export tracking data ..."));
    qApp->processEvents();

    outXml << "    <shape>" << Qt::endl;
    for(j = 0; j < size(); ++j)
    {
        if(alternateHeight) // bei variierender groesse wird einfach durchschnittsgroesse genommen, da an treppen
                            // gar keine vernuempftige Groesse vorliegt
        {
            outXml << "        <agentInfo ID=\"" << j + 1 << "\" color=\"100\" height=\"" << defaultPersonHeight
                   << "\"/>" << Qt::endl;
        }
        else
        {
            outXml << "        <agentInfo ID=\"" << j + 1 << "\" color=\"100\" height=\"" << at(j).height() << "\"/>"
                   << Qt::endl;
        }
    }
    outXml << "    </shape>" << Qt::endl << Qt::endl;

    // i = frame; j = person
    for(i = smallestFirstFrame(); i <= largestLastFr; ++i)
    {
        progress.setLabelText(QString("Export data ..."));
        qApp->processEvents();
        progress.setValue(i);

        outXml << "    <frame ID=\"" << i << "\">" << Qt::endl;
        for(j = 0; j < size(); ++j)
        {
            if(at(j).trackPointExist(i))
            {
                // z-wert ist hier ausnahmsweise nicht der kopf, sondern der boden, die prsonengroesse wird dem
                // obigem person-datenentnommen personID, Frame ID(?) , X , Y , Z
                if(useTrackpoints)
                {
                    z = at(j).trackPointAt(i).z() + defaultPersonHeight;
                }
                else // war, wenn ebene versuche sinnvoll:z = at(j).trackPointAt(i).z()+at(j).height();
                {
                    if(alternateHeight)
                    {
                        z = at(j).trackPointAt(i).z() - defaultPersonHeight;
                    }
                    else // war, wenn ebene versuche sinnvoll: z = at(j).trackPointAt(i).z()-at(j).height();
                    {
                        z = 0; // at(j).height();
                    }
                }
                outXml << "        <agent ID=\"" << j + 1 << "\" xPos=\"" << at(j).trackPointAt(i).x() << "\" yPos=\""
                       << at(j).trackPointAt(i).y() << "\" zPos=\"" << z
                       << "\" xVel=\"0\" yVel=\"0\" zVel=\"0\" radiusA=\"18\" radiusB=\"15\" "
                          "ellipseOrientation=\"130\" ellipseColor=\"0\"/>"
                       << Qt::endl; // z Koordinate ist Boden
            }
        }
        outXml << "    </frame>" << Qt::endl;
    }
}

/**
 * @brief Compute the dropped frames
 *
 * @note This function needs to "play" the video again to track each pedestrian, hence it is quite expensive!
 *
 * @param petrack handler for managing the player and getting the frame
 * @return vector of all missing frame (frame number and number of frames missing)
 */
std::vector<MissingFrame> TrackerReal::computeDroppedFrames(Petrack *petrack)
{
    if(petrack->getImageFiltered().empty())
    {
        throw std::runtime_error(
            "Can not compute missing frames with an empty video. Please load a video, and try again.");
    }

    // Save current state
    auto recognitionState = petrack->getControlWidget()->isPerformRecognitionChecked();
    petrack->getControlWidget()->setPerformRecognitionChecked(false);

    auto trackingState = petrack->getControlWidget()->isOnlineTrackingChecked();
    petrack->getControlWidget()->setOnlineTrackingChecked(false);

    int currentFrameNum = petrack->getPlayer()->getPos();

    auto minFrame = std::max(0, petrack->getPlayer()->getFrameInNum());
    auto maxFrame = std::min(petrack->getAnimation()->getNumFrames(), petrack->getPlayer()->getFrameOutNum());


    std::vector<std::vector<cv::Point2f>> personsInFrame(maxFrame + 1);
    std::vector<std::vector<int>>         idsInFrame(maxFrame + 1);
    auto                                  persons = mPersonStorage.getPersons();

    for(size_t i = 0; i < persons.size(); ++i)
    {
        auto const &person = persons[i];
        for(int frame = person.firstFrame(); frame <= std::min(person.lastFrame(), maxFrame); ++frame)
        {
            personsInFrame[frame].push_back(person.trackPointAt(frame).toPoint2f());
            idsInFrame[frame].push_back(static_cast<int>(i));
        }
    }

    auto displacementsPerFrame = utils::computeDisplacement(minFrame, maxFrame, petrack, personsInFrame, idsInFrame);

    // Detect missing frames
    std::vector<MissingFrame> missingFrames = utils::detectMissingFrames(displacementsPerFrame);


    petrack->getPlayer()->skipToFrame(currentFrameNum);
    petrack->getControlWidget()->setPerformRecognitionChecked(recognitionState);
    petrack->getControlWidget()->setOnlineTrackingChecked(trackingState);

    return missingFrames;
}

/**
 * @brief Computes the displacement for each detected pedestrian in each frame
 *
 * @param minFrameNum frame to start the computation
 * @param maxFrameNum frame to end the computation
 * @param petrack handler for managing the player and getting the frame
 * @param personsInFrame pixel coordinates where each pedestrian is located in a frame (same order as idsInFrame)
 * @param idsInFrame ids of pedestrians in a frame (same order as personsInFrame)
 * @return displacement for each pedestrian in each frame
 */
std::vector<std::unordered_map<int, double>> utils::computeDisplacement(
    int                                          minFrameNum,
    int                                          maxFrameNum,
    Petrack                                     *petrack,
    const std::vector<std::vector<cv::Point2f>> &personsInFrame,
    const std::vector<std::vector<int>>         &idsInFrame)
{
    auto fps = petrack->getAnimation()->getSequenceFPS();

    petrack->getPlayer()->skipToFrame(minFrameNum);

    cv::Mat currentFrame;
    cv::Mat prevFrame;

    cv::Mat prevFrameColor = petrack->getImageFiltered();
    cv::cvtColor(prevFrameColor, prevFrame, cv::COLOR_BGR2GRAY);

    std::vector<std::unordered_map<int, double>> displacementsPerFrame(maxFrameNum + 1);

    // compute window size
    auto cmPerPixelXYMiddle = petrack->getWorldImageCorrespondence().getCmPerPixel(
        static_cast<float>(prevFrame.cols / 2),
        static_cast<float>(prevFrame.rows / 2),
        static_cast<float>(petrack->getControlWidget()->getDefaultHeight()));
    auto             cmPerPixelMiddle = (cmPerPixelXYMiddle.x() + cmPerPixelXYMiddle.y()) / 2.;
    constexpr double headFactor       = 1.25; //< factor around head size to ensure complete head is in window
    int              winsize          = static_cast<int>(headFactor * HEAD_SIZE / cmPerPixelMiddle);
    cv::Size         window{winsize, winsize};

    for(int frame = minFrameNum + 1; frame <= maxFrameNum; frame++)
    {
        // get the current frame and convert to gray scale image
        petrack->getPlayer()->frameForward();
        cv::Mat currentFrameColor = petrack->getImageFiltered();
        cv::cvtColor(currentFrameColor, currentFrame, cv::COLOR_BGR2GRAY);

        // track the current points in the image
        std::vector<cv::Point2f> prevFeaturePoint{personsInFrame[frame - 1]};
        if(prevFeaturePoint.empty())
        {
            cv::swap(prevFrame, currentFrame);
            continue;
        }

        std::vector<cv::Point2f> nextFeaturePoint{};
        std::vector<uchar>       localStatus;
        std::vector<float>       localTrackError;

        int maxLevel = petrack->getControlWidget()->getTrackRegionLevels();

        cv::calcOpticalFlowPyrLK(
            prevFrame,
            currentFrame,
            prevFeaturePoint,
            nextFeaturePoint,
            localStatus,
            localTrackError,
            window,
            maxLevel);

        // compute the displacement for each pedestrian
        std::unordered_map<int, double> displacementsInFrame(prevFeaturePoint.size());
        for(size_t i = 0; i < prevFeaturePoint.size(); ++i)
        {
            auto displacement = prevFeaturePoint[i] - nextFeaturePoint[i];
            if(!idsInFrame[frame].empty() && localStatus[i] == 1)
            {
                auto id = idsInFrame[frame - 1][i];

                auto cmPerPixelXY = petrack->getWorldImageCorrespondence().getCmPerPixel(
                    nextFeaturePoint[i].x, nextFeaturePoint[i].y, petrack->getControlWidget()->getDefaultHeight());

                auto mPerPixel = (cmPerPixelXY.x() + cmPerPixelXY.y()) / 2. / 100.;

                auto movement = cv::norm(displacement);

                auto speed = movement * mPerPixel * fps;

                constexpr auto minSpeed = 0.25;

                // if small movement, assume the same movement as before
                if(speed < minSpeed)
                {
                    if(displacementsPerFrame[frame - 1].find(id) != displacementsPerFrame[frame - 1].end())
                    {
                        movement = displacementsPerFrame[frame - 1][id];
                    }
                    else
                    {
                        continue;
                    }
                }
                displacementsInFrame.emplace(id, movement);
            }
        }

        cv::swap(prevFrame, currentFrame);
        displacementsPerFrame[frame] = displacementsInFrame;
    }

    return displacementsPerFrame;
}

/**
 * @brief Detects the missing frames based on the displacements
 *
 * Main idea: The displacement between a small range of frames should be roughly the same. Meaning no large jumps
 * between two frames.
 *
 * @param displacementsPerFrame displacement for each pedestrian per frame
 * @return vector of all missing frame (frame number and number of frames missing)
 */
std::vector<MissingFrame>
utils::detectMissingFrames(const std::vector<std::unordered_map<int, double>> &displacementsPerFrame)
{
    std::vector<MissingFrame> missingFrames;

    constexpr long averageWindow = 3;

    for(size_t frame = 2; frame < displacementsPerFrame.size(); ++frame)
    {
        auto begin = displacementsPerFrame.begin() + std::max(static_cast<long>(frame) - averageWindow, 0l);
        auto end   = displacementsPerFrame.begin() + frame;

        auto referenceDisplacement = std::vector<std::unordered_map<int, double>>(begin, end);

        const auto &displacement = displacementsPerFrame[frame];
        if(referenceDisplacement.empty() || displacement.empty())
        {
            continue;
        }

        std::vector<double> relationToPrevFrames;
        for(auto const &[id, norm] : displacement)
        {
            std::vector<double> norms;
            for(auto const &disp : referenceDisplacement)
            {
                if(disp.find(id) != disp.end())
                {
                    norms.push_back(disp.at(id));
                }
            }
            if(norms.empty())
            {
                continue;
            }

            double averageNorm = std::accumulate(norms.begin(), norms.end(), 0.0) / norms.size();

            double relation = norm / averageNorm;

            relationToPrevFrames.push_back(relation);
        }

        if(relationToPrevFrames.empty())
        {
            continue;
        }
        double median = computeMedian(relationToPrevFrames);

        auto medianRounded = std::round(median);

        if(medianRounded > 1.)
        {
            int numMissingFrames = static_cast<int>(medianRounded) - 1;

            SPDLOG_WARN(
                "potentially missing {} frame(s) between {} and {} will be inserted",
                numMissingFrames,
                frame - 1,
                frame);

            missingFrames.push_back({frame - 1, numMissingFrames});
        }
    }

    return missingFrames;
}
