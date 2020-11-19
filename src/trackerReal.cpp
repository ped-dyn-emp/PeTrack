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

#include "trackerReal.h"
#include "helper.h"
#include "recognition.h"

// TrackPointReal::TrackPointReal()
//     : mQual(0)
// {
// }
TrackPointReal::TrackPointReal(const Vec3F &p, int frameNum)
    : Vec3F(p), mFrameNum(frameNum)
{
    mAngleOfView = -1;
    mMarkerID = -1;
}

TrackPointReal::TrackPointReal(const Vec3F &p, int frameNum, const Vec2F &d)
    : Vec3F(p), mFrameNum(frameNum), mViewDir(d)
{
    mAngleOfView = -1;
    mMarkerID = -1;
}

// const TrackPointReal& TrackPointReal::operator=(const Vec2F& v)
// {
//     Vec2F::operator=(v);
//     return *this;
// }

// const TrackPointReal& TrackPointReal::operator+=(const Vec2F& v)
// {
//     Vec2F::operator+=(v);
//     return *this;
// }
// const TrackPointReal TrackPointReal::operator+(const Vec2F& v) const
// {
//     return TrackPointReal(*this) += v; //Vec2F(mX + v.mX, mY + v.mY);
// }

//--------------------------------------------------------------------------

// the list index is the frame number plus mFirstFrame 0..mLastFrame-mFirstFrame
// no frame is left blank
TrackPersonReal::TrackPersonReal()
    : mHeight(-1.),
      mFirstFrame(-1),
      mLastFrame(0)
{
}
TrackPersonReal::TrackPersonReal(int frame, const TrackPointReal &p)
    : mHeight(-1.),
      mFirstFrame(frame),
      mLastFrame(frame)
{
    append(p);
}

bool TrackPersonReal::trackPointExist(int frame) const
{
    if (frame >= mFirstFrame && frame <= mLastFrame)
        return true;
    else
        return false;
}
const TrackPointReal& TrackPersonReal::trackPointAt(int frame) const // & macht bei else probleme, sonst mit [] zugreifbar
{
//     if (frame >= mFirstFrame && frame <= mLastFrame) ////!!!! muss vorher ueberprueft werden, ob es existiert!!!
        return at(frame-mFirstFrame);
//     else              
//         return TrackPointReal();
}

// gibt -1 zurueck, wenn frame oder naechster frame nicht existiert
// entfernung ist absolut
double TrackPersonReal::distanceToNextFrame(int frame) const
{
    if (frame >= mFirstFrame && frame+1 <= mLastFrame)
        return at(frame-mFirstFrame).distanceToPoint(at(frame-mFirstFrame+1));
    else
        return -1;
}
void TrackPersonReal::init(int firstFrame, double height, int markerID)
{
    clear();
    mFirstFrame = firstFrame;
    mLastFrame = firstFrame-1;
    mHeight = height;
    mMarkerID = markerID;
}

void TrackPersonReal::addEnd(const QPointF& pos, int frame)
{
    Vec3F p(pos.x(), pos.y(), -1.);
    addEnd(p, frame);
    //++mLastFrame;
    //append(TrackPointReal(p, frame));
}
void TrackPersonReal::addEnd(const Vec3F& pos, int frame)
{
    ++mLastFrame;
    append(TrackPointReal(pos, frame));
}

void TrackPersonReal::addEnd(const QPointF& pos, int frame, const QPointF& dir)
{
    Vec3F p(pos.x(), pos.y(), -1.);
    Vec2F d(dir.x(), dir.y());
    ++mLastFrame;
    d.normalize();
    append(TrackPointReal(p, frame, d));
}

//-------------------------------------------------------------------------------------------------------------------------------------

TrackerReal::TrackerReal(QWidget *wParent)
    : mXMin(100000.), mXMax(-100000.), mYMin(100000.), mYMax(-100000.)
{
        mMainWindow = (class Petrack*) wParent;
}


// TrackerReal::~TrackerReal()
// {
// }

// default: int imageBorderSize = 0, bool missingFramesInserted = true, bool useTrackpoints = false
int TrackerReal::calculate(Tracker *tracker, ImageItem *imageItem, ColorPlot *colorPlot, int imageBorderSize, bool missingFramesInserted,
                           bool useTrackpoints, bool alternateHeight, double altitude, bool useCalibrationCenter,
                           bool exportElimTp, bool exportElimTrj, bool exportSmooth,
                           bool exportViewingDirection, bool exportAngleOfView, bool exportMarkerID, bool exportAutoCorrect)
{
    if (tracker || imageItem || colorPlot)
    {
        if (size() > 0)
        {
            clear();
//             debout << "Recalculating real tracking data" <<endl;
        }
        else
        {
//             debout << "Calculating real tracking data" <<endl;
        }

        int i, j, f;
        QList<int> missingList; // frame nr wo ausgelassen; passend dazu:
        QList<int> missingListAnz; // anzahl ausgelassener frames
        if (missingFramesInserted)
        {
            // finden von nicht aufgenommenen frames
            int largestLastFrame = tracker->largestLastFrame();
            QMap<int, double> distanceMap; // map der distanzen zu frame
            QMap<int, double> lastDistanceMap; // map der distanzen zu frame
            QMap<int, double>::const_iterator iter;
            int anz, skipAnz;
            double dist, sum, lastSum=0.;
            // (median neben durchschnittswert als bewertungszahl hinzugenommen,
            // da sonst bei wenigen personen und langsamen bewegungen nur ein kleiner ausreisser
            // einen frame hinzufuegt (insb linienexperiment mit vielen personen))
            for (f = tracker->smallestFirstFrame(); f <= largestLastFrame; ++f)
            {
                distanceMap.clear();
                skipAnz = anz = 0;
                sum = 0.;
                // erzeugen distanz-map im aktuellen frame
                for (i = 0; i < tracker->size(); ++i)
                    if ((dist = tracker->at(i).distanceToNextFrame(f)) > -1)
                        distanceMap[i] = dist;
                // distanzliste mit vorherigem frame vergleichen
                iter = distanceMap.constBegin();
                while (iter != distanceMap.constEnd()) 
                {
                    if (lastDistanceMap.contains(iter.key()) && 
                        lastDistanceMap[iter.key()] > 1.) // damit bei ganz kleinen bewegungen nicht angeschlagen wird 
                    {
                        ++anz;
                        if (distanceMap[iter.key()] > 1.5*lastDistanceMap[iter.key()])
                            ++skipAnz;
                    }
                    sum += distanceMap[iter.key()];
                    ++iter;
                }
                sum/=distanceMap.size();
                // bei sprung in mehreren trackingpfaden als sprung markieren
                if ((anz > 1) && (skipAnz/(double)anz > .5) && myRound(sum/lastSum - 1.) > 0) // 50% (war (anz > 2))
                {
                    missingList.append(f);
                    missingListAnz.append(myRound(sum/lastSum - 1.));
                    debout << "Warning: potentially missing " << myRound(sum/lastSum - 1.) << " frame(s) between " << f << " and " << f+1 << " will be inserted." << std::endl;
                }
                lastDistanceMap = distanceMap;
                lastSum = sum;
            }
        }

        // fps ist nicht aussagekraeftig, da sie mgl von ausgelassenen herruehren - besser immer 25,01 fps annehmen
        //                 out << tracker->size() << " " << mAnimation->getFPS() << endl << endl;

        double height; // groesse in cm
        int firstFrame, addFrames, anz;
        Vec2F br(imageBorderSize, imageBorderSize);
        QPointF pos, pos2;
        QList<int> tmpMissingList; // frame nr
        QList<int> tmpMissingListAnz; // anzahl frames
        TrackPersonReal trackPersonReal;
        QPointF center = imageItem->getPosReal(QPointF(imageItem->boundingRect().width()/2., imageItem->boundingRect().height()/2.), 0.);
        Vec3F sp;
        int nrFor;
        int nrRew;
        int tsize;
        double zMedian;
        int extrapolated;
        QPointF colPos;
        float angle;

        for (i = 0; i < tracker->size(); ++i) // ueber trajektorien
        {
            addFrames = 0;
            firstFrame = tracker->at(i).firstFrame();

            if ((*tracker)[i].height() < MIN_HEIGHT+1)
                height = colorPlot->map((*tracker)[i].color());
            else
                height = (*tracker)[i].height();
            //(*tracker)[i].setHeight(height);
            if (missingList.size() > 0)
            {
                tmpMissingList = missingList;
                tmpMissingListAnz = missingListAnz;
                // vorspulen
                while ((tmpMissingList.size() > 0) && (tmpMissingList.first() < firstFrame))
                {
                    tmpMissingList.removeFirst(); // frame
                    addFrames += tmpMissingListAnz.takeFirst(); // anzahl
                    //tmpMissingListAnz.removeFirst(); // anzahl
                }
            }

            int markerID = (*tracker)[i].getMarkerID(); // set markerID to TrackPerson.markerID

            trackPersonReal.init(firstFrame+addFrames, height, markerID);
            tsize = tracker->at(i).size();
            for (j = 0; (j < tsize); ++j) // ueber trackpoints
            {
                // ausreisser ausfindig machen (dies geschieht, bevor -1 elemente herausgenommen werden, um die glaettung beim eliminieren der -1 elemente hier nicht einfluss nehmen zu lassen):
                if (exportSmooth && (useTrackpoints || alternateHeight) && (tsize > 1)) // wenn direkt pointgrey hoehe oder eigene hoehenberechnung aber variierend ueber trj genommen werden soll
                {
                    // ACHTUNG: Aenderungen in Originaltrajektorie, so dass aenderungen auf folgeuntersuchungen einfluss haben: j auf j+1
                    if ((*tracker)[i][j].sp().z() != -1)
                    {
                        nrFor = 1; // anzahl der ztrackpoint ohne hoeheninfo
                        nrRew = 1;
                        while ((j+nrFor < tsize) && ((*tracker)[i].at(j+nrFor).sp().z() < 0)) // nach && wird nur ausgefuehrt, wenn erstes true == size() also nicht
                            nrFor++;
                        while ((j-nrRew >= 0) && ((*tracker)[i].at(j-nrRew).sp().z() < 0)) // nach && wird nur ausgefuehrt, wenn erstes true == size() also nicht
                            nrRew++;
                        //if ((j+nrFor == tsize) && (j-nrRew < 0)) // gar keine Hoeheninfo in trj gefunden

                        if (((j-nrRew >= 0) && (j+nrFor == tsize)) || ((j-nrRew >= 0) && (nrRew < nrFor))) // nur oder eher in Vergangenheit hoeheninfo gefunden
                        {
//if (i == 175) // N:\tagDerNeugier\14.19.21.trc
//{
//    debout << i+1 << endl;
//    debout << (*tracker)[i] << endl;
//    debout << (*tracker)[i].size() << endl;
//    debout << j << " " << nrRew << endl;
////    (*tracker)[i]
////    0 138.869 2927 2928 1 -1 -1 -1 2
////    952.885 550.538 -1 -1 -1 100 0 0 -1 -1 -1
////    965.469 564.862 104.295 28.3304 316.131 100 0 0 -1 -1 -1
////    i j nrRew:
////    175 1 2
//}
// in diesem if kam es zum Absturz, da j = 1 und nrRew = 2
                            if(fabs((*tracker)[i].at(j-nrRew).sp().z()-(*tracker)[i].at(j).sp().z()) > nrRew*40.) // 40cm
                            {
                                (*tracker)[i][j].setSp((*tracker)[i].at(j).sp().x(),(*tracker)[i].at(j).sp().y(),(*tracker)[i].at(j-nrRew).sp().z());
                                debout << "Warning: Trackpoint smoothed height at the end or next to unknown height in the future for trajectory " << i+1 << " in frame " << j+firstFrame << "." << std::endl;
                            }
                        }
                        else if (((j+nrFor != tsize) && (j-nrRew < 0)) || ((j+nrFor != tsize) && (nrFor < nrRew))) // nur oder eher in der zukunft hoeheninfo gefunden
                        {
                            //debout << nrRew << " " << nrFor << " " << (*tracker)[i].at(j+nrFor).sp().z() << " " << (*tracker)[i].at(j).sp().z() << " " << endl;
                            if(fabs((*tracker)[i].at(j+nrFor).sp().z()-(*tracker)[i].at(j).sp().z()) > nrFor*40.) // 40cm
                            {
                                (*tracker)[i][j].setSp((*tracker)[i].at(j).sp().x(),(*tracker)[i].at(j).sp().y(),(*tracker)[i].at(j+nrFor).sp().z());
                                debout << "Warning: Trackpoint smoothed height at the beginning or next to unknown height in the past for trajectory " << i+1 << " in frame " << j+firstFrame << "." << std::endl;
                            }
                        }
                        else if ((j+nrFor != tsize) && (j-nrRew >= 0))// in beiden richtungen hoeheninfo gefunden und nrFor==nrRew
                        {
                            // median genommen um zwei fehlmessungen nebeneinander nicht dazu fuehren zu lassen, dass bessere daten veraendert werden
                            zMedian = getMedianOf3((*tracker)[i].at(j).sp().z(), (*tracker)[i].at(j-nrRew).sp().z(), (*tracker)[i].at(j+nrFor).sp().z());
                            // lineare interpolation
                            //zInter=(*tracker)[i].at(j-nrRew).sp().z()+nrRew*((*tracker)[i].at(j+nrFor).sp().z()-(*tracker)[i].at(j-nrRew).sp().z())/(nrFor+nrRew);
                            if(fabs(zMedian-(*tracker)[i].at(j).sp().z()) > 20.*(nrFor+nrRew)) // 20cm
                            {
                                (*tracker)[i][j].setSp((*tracker)[i].at(j).sp().x(),(*tracker)[i].at(j).sp().y(),zMedian);
                                debout << "Warning: Trackpoint smoothed height inside for trajectory " << i+1 << " in frame " << j+firstFrame << "." << std::endl;
                            }
                        }
                    }
                }
                if (useTrackpoints)
                {
                    // border unberuecksichtigt
                    if (useCalibrationCenter)
                        trackPersonReal.addEnd(Vec3F((*tracker)[i][j].sp().x()+center.x(), center.y()-(*tracker)[i][j].sp().y(), altitude-(*tracker)[i][j].sp().z()), firstFrame+j);
                    else
                        trackPersonReal.addEnd((*tracker)[i][j].sp(), firstFrame+j);
                }
                else
                {
                    if (alternateHeight) // personenhoehe variiert ueber trajektorie (unebene versuche); berechnung durch mich und nicht pointgrey nutzen, Kamera altitude nutzen
                    {
                        extrapolated = 0; // 0 == false
                        double bestZ = (*tracker)[i].getNearestZ(j, &extrapolated); // gibt z wert zurueck bzw wenn -1, dann den neben diesem frame ersten z-wert ungleich -1
                        if (bestZ < 0) // == -1 // es liegt gar keine berechnete hoehe vor
                        {
                            if (exportElimTrj)
                            {
                                debout << "Warning: no calculated height for trackpoint " << j << " (frame " << tracker->at(i).firstFrame()+j<< ") of person " << i+1 << ", person is not exported!" << std::endl;
                                break; // TrackPerson ist angelegt, erhaelt aber keine Points und wird deshalb am ende nicht eingefuegt
                            }
                            else
                            {
                                bestZ = height; // wenn gar kein trackpoint ungleich -1 gefunden wird, dann wird zu allerletzt Hoehe genommen
                                debout << "Warning: no calculated height for trackpoint " << j << " (frame " << tracker->at(i).firstFrame()+j<< ") of person " << i+1 << ", default height is used!" << std::endl;
                            }
                        }
                        else
                            bestZ = altitude-bestZ;
                        // ab hier kann bestZ auch negativ sein, obwohl hoehe inhalt hatte

                        if (extrapolated && exportElimTp)
                        {
                            if (extrapolated == 1) // zu beginn verworfen
                                trackPersonReal.setFirstFrame(trackPersonReal.firstFrame()+1);
                            debout << "Warning: no calculated height for trackpoint " << j << " (frame " << tracker->at(i).firstFrame()+j<< ") of person " << i+1 << ", extrapolated height not used, trackpoint not inserted!" << std::endl;
                        }
                        else
                        {
                            if (extrapolated)
                                debout << "Warning: no calculated height for trackpoint " << j << " (frame " << tracker->at(i).firstFrame()+j<< ") of person " << i+1 << ", extrapolated height is used!" << std::endl;

                            Vec2F moveDir(0,0);
                            if (exportAutoCorrect)
                                moveDir += autoCorrectColorMarker((*tracker)[i][j], mMainWindow->getControlWidget());

                            pos = imageItem->getPosReal(((*tracker)[i][j]+moveDir+br).toQPointF(), bestZ);

                            if ((exportViewingDirection) && ((*tracker)[i][j].color().isValid())) // wenn blickrichtung mit ausgegeben werden soll
                            {
                                colPos = imageItem->getPosReal(((*tracker)[i][j].colPoint()+moveDir+br).toQPointF(), bestZ);
                                trackPersonReal.addEnd(pos, firstFrame+j, colPos-pos);
                            }
                            else
                                trackPersonReal.addEnd(Vec3F(pos.x(), pos.y(), bestZ), firstFrame+j);
                        }
                    }
                    else
                    {
                        Vec2F moveDir(0,0);
                        if (exportAutoCorrect)
                            moveDir += autoCorrectColorMarker((*tracker)[i][j], mMainWindow->getControlWidget());

                        pos = imageItem->getPosReal(((*tracker)[i][j]+moveDir+br).toQPointF(), height);
                        // die frame nummer der animation wird TrackPoint der PersonReal mitgegeben,
                        // da Index groesser sein kann, da vorher frames hinzugefuegt wurden duch trackPersonReal.init(firstFrame+addFrames, height)
                        // oder aber innerhalb des trackink path mit for schleife ueber f
                        if ((exportViewingDirection) && ((*tracker)[i][j].color().isValid())) // wenn blickrichtung mit ausgegeben werden soll
                        {
                            colPos = imageItem->getPosReal(((*tracker)[i][j].colPoint()+moveDir+br).toQPointF(), height);
                            trackPersonReal.addEnd(pos, firstFrame+j, colPos-pos);
                        }
                        else
                            trackPersonReal.addEnd(pos, firstFrame+j);
                        if (exportAngleOfView)
                        {
                            angle = (90.-imageItem->getAngleToGround(((*tracker)[i][j]+br).x(), ((*tracker)[i][j]+br).y(), height))*PI/180.;
                            trackPersonReal.last().setAngleOfView(angle);
                        }
                        if (exportMarkerID)
                        {
                            trackPersonReal.last().setMarkerID((*tracker)[i][j].getMarkerID());
                        }

                        // firstFrame+j+addFrames    //ist die kontinuierliche frame number
                    }
                }

                if (tmpMissingList.size() > 0)
                {
                    if ((tmpMissingList.first() == firstFrame+j) && (tracker->at(i).trackPointExist(firstFrame+j+1)))
                    {
                        tmpMissingList.removeFirst(); // frame
                        anz = tmpMissingListAnz.takeFirst(); // anzahl
                        if (useTrackpoints)
                        {
                            // border unberuecksichtigt
                            for (f = 1; f <= anz; ++f)
                            {
                                //trackPersonReal.addEnd((*tracker)[i][j].sp()+f*((*tracker)[i][j+1].sp()-(*tracker)[i][j].sp())/(anz+1), -1); // -1 zeigt an, dass nur interpoliert
                                sp = (*tracker)[i][j].sp()+f*((*tracker)[i][j+1].sp()-(*tracker)[i][j].sp())/(anz+1);
                                if (useCalibrationCenter)
                                    trackPersonReal.addEnd(Vec3F(sp.x()+center.x(), center.y()-sp.y(), altitude-sp.z()), firstFrame+j);
                                else
                                    trackPersonReal.addEnd(sp, -1); // -1 zeigt an, dass nur interpoliert
                            }
                        }
                        else
                        {
                            if (alternateHeight) // personenhoehe variiert ueber trajektorie (unebene versuche); berechnung durch mich und nicht pointgrey nutzen
                            {
                                debout << "Warning: No interpolation is done, because alternate height is enabled - has to be implemented!" << std::endl;
                            }
                            else
                            {
                                Vec2F moveDir(0,0);
                                if (exportAutoCorrect)
                                    moveDir += autoCorrectColorMarker((*tracker)[i][j], mMainWindow->getControlWidget());

                                pos2 = (imageItem->getPosReal(((*tracker)[i][j+1]+moveDir+br).toQPointF(), height) - pos)/(anz+1);
                                for (f = 1; f <= anz; ++f)
                                    trackPersonReal.addEnd(pos+f*pos2, -1); // -1 zeigt an, dass nur interpoliert
                                //addFrames += anz;
                            }
                        }
                    } 
                    else if (tmpMissingList.first() < firstFrame) // while, wenn nicht kontinuierlich waere
                    {
                        tmpMissingList.removeFirst(); // frame
                        //addFrames += tmpMissingListAnz.takeFirst(); // anzahl
                        tmpMissingListAnz.removeFirst(); // anzahl
                    }
                }
            }

            tsize = trackPersonReal.size();
            double maxHeightDiff = 30.;// 30cm
            int numBorderPoints = 50;
            int k;
            int delNumFront = 0;
            // die ersten und letzten numBorderPoints trackpoints untersuchen, ob die hoehe einen sprung groesser maxHeightDiff aufweist
            for (j = 1; j < tsize; ++j) // ueber trackpoints of personreal; ab 1, da vergleich zu vorherigem
            {
                if ((j < numBorderPoints) && ((trackPersonReal.at(j).z()-trackPersonReal.at(j-1).z()) > maxHeightDiff))
                {
                    delNumFront = j;
                    //debout << i << " " << j << " " <<trackPersonReal.at(j).z()-trackPersonReal.at(j-1).z() <<endl;
                }
                else if (((tsize - numBorderPoints) < j) && ((trackPersonReal.at(j).z()-trackPersonReal.at(j-1).z()) > maxHeightDiff))
                {
                    for (k = j; k < tsize; k++)
                    {
                        trackPersonReal.setLastFrame(trackPersonReal.lastFrame()-1);
                        trackPersonReal.removeLast();
                    }
                    debout << "Warning: delete last " << tsize-j << " frames of person " << i+1 << " because of height jump!" << std::endl;
                    break;
                }
            }
            tsize = trackPersonReal.size();
            if (delNumFront > 0)
            {
                debout << "Warning: delete first " << delNumFront << " frames of person " << i+1 << " because of height jump!" << std::endl;
                for (k = 0; (k < delNumFront) && (k < tsize); k++)
                {
                    trackPersonReal.setFirstFrame(trackPersonReal.firstFrame()+1);
                    trackPersonReal.removeFirst();
                }
            }
            if (trackPersonReal.size() < 20)
                debout << "Warning: Person " << i+1 << " has only " << trackPersonReal.size() << " trackpoints!" << std::endl;
            if (trackPersonReal.size() > 0)
                append(trackPersonReal);
            else // ggf weil keine calculated height vorlag (siehe exportElimTrj)
                debout << "Warning: Person " << i+1 << " is not inserted, because of no trackpoints!" << std::endl;

        }
        return size();
    }
    else
    {
        debout << "Warning: no real tracking data calculated, because of missing tracking data, image reference or color map!" << std::endl;
        return -1;
    }
}

void TrackerReal::calcMinMax()
{
    Vec3F pos;
    for (int i = 0; i < size(); ++i)
        for (int j = 0; j < at(i).size(); ++j)
        {
            pos = at(i).at(j);
            if (mXMin > pos.x())
                mXMin = pos.x();
            else if (mXMax < pos.x())
                mXMax = pos.x();
            if (mYMin > pos.y())
                mYMin = pos.y();
            else if (mYMax < pos.y())
                mYMax = pos.y();
        }
}

int TrackerReal::largestFirstFrame()
{
    int max = -1, i;
    for (i = 0; i < size(); ++i)
    {
        if (at(i).firstFrame() > max)
            max = at(i).firstFrame();
    }
    return max;
}
int TrackerReal::largestLastFrame()
{
    int max = -1, i;
    for (i = 0; i < size(); ++i)
    {
        if (at(i).lastFrame() > max)
            max = at(i).lastFrame();
    }
    return max;
}
int TrackerReal::smallestFirstFrame()
{
    int i, min = ((size()>0) ? at(0).firstFrame() : -1);
    for (i = 1; i < size(); ++i)
    {
        if (at(i).firstFrame() < min)
            min = at(i).firstFrame();
    }
    return min;
}
int TrackerReal::smallestLastFrame()
{
    int i, min = ((size()>0) ? at(0).lastFrame() : -1);
    for (i = 1; i < size(); ++i)
    {
        if (at(i).lastFrame() < min)
            min = at(i).lastFrame();
    }
    return min;
}

void TrackerReal::exportTxt(QTextStream &out, bool alternateHeight, bool useTrackpoints, bool exportViewingDirection, bool exportAngleOfView, bool exportUseM, bool exportMarkerID)
{
    float scale;

    out << "# z: can be 3d position or height of person (alternating or not)" << Qt::endl;
    if (exportViewingDirection)
        out << "# viewDirX viewDirY: vector of direction of head of person" << Qt::endl;
    if (exportAngleOfView)
        out << "# viewAngle: angle of view of camera to person from perpendicular [0..Pi/2]" << Qt::endl;
    if (exportUseM)
        out << "# id frame x/m y/m z/m";
    else
        out << "# id frame x/cm y/cm z/cm";
    if (exportViewingDirection)
        out << " viewDirX viewDirY";
    if (exportAngleOfView)
        out << " viewAngle" << Qt::endl;
    if (exportMarkerID)
        out << " markerID" << Qt::endl;
    else
        out << Qt::endl;

    if (exportUseM)
        scale = .01;
    else
        scale = 1.;

//    if (useTrackpoints)
//    {
//        for (int i = 0; i < size(); ++i)
//            for (int j = 0; j < at(i).size(); ++j)
//                // Umrechnung in coordSystem fehlt, auch camera altitude unberuecksichtigt!!!
//                out << i+1 << " " << at(i).firstFrame()+j << " " << at(i).at(j).x() << " " << at(i).at(j).y() << " " << at(i).at(j).z()*scale <<endl;
//   war:         out << i+1 << " " << at(i).firstFrame()+j << " " << at(i).at(j) << " " << at(i).at(j).z()*scale <<endl;
//    }  //<< " " << tp.qual();
//    else
//    {
    //   out << *this;
    QProgressDialog progress("Export TXT-File",NULL,0,size(),mMainWindow->window());
    progress.setWindowTitle("Export .txt-File");
    progress.setWindowModality(Qt::WindowModal);
    progress.setVisible(true);
    progress.setValue(0);
    progress.setLabelText(QString("Export tracking data ..."));
    qApp->processEvents();

    for (int i = 0; i < size(); ++i){

        qApp->processEvents();
        progress.setLabelText(QString("Export person %1 of %2 ...").arg(i+1).arg(size()));
        progress.setValue(i+1);

        for (int j = 0; j < at(i).size(); ++j)
        {
            out << i+1 << " " << at(i).firstFrame()+j << " " << at(i).at(j).x()*scale << " " << at(i).at(j).y()*scale << " ";
            //out << i+1 << " " << at(i).firstFrame()+j << " " << at(i).at(j).x()*scale << " " << at(i).at(j).y()*scale << " ";

            if (alternateHeight || useTrackpoints)
                out << at(i).at(j).z()*scale;
            else
                out << at(i).height()*scale;

            if (exportViewingDirection) // && (at(i).at(j).viewDir() != Vec2F(0,0)) zeigt an, dass keine richtung berechnet werden konnte
                out << " " << at(i).at(j).viewDir();

            if (exportAngleOfView)
                out << " " << at(i).at(j).angleOfView() << Qt::endl;

            if (exportMarkerID)
                out << " " << at(i).getMarkerID() << Qt::endl;
            else
                out << Qt::endl;
        }
    }
}

//    inline QTextStream& operator<<(QTextStream& s, const TrackerReal& trackerReal)
//    {
//        for (int i = 0; i < trackerReal.size(); ++i)
//            for (int j = 0; j < trackerReal.at(i).size(); ++j)
//                s << i+1 << " " << trackerReal.at(i).firstFrame()+j << " " << trackerReal.at(i).at(j) << " " << trackerReal.at(i).height() <<endl; //<< " " << tp.qual();
//        return s;
//    }

// old - not all export options supported!!!!
void TrackerReal::exportDat(QTextStream &out, bool alternateHeight, bool useTrackpoints) // fuer gnuplot
{
    QProgressDialog progress("Export DAT-File",NULL,0,size(),mMainWindow->window());
    progress.setWindowTitle("Export .dat-File");
    progress.setWindowModality(Qt::WindowModal);
    progress.setVisible(true);
    progress.setValue(0);
    progress.setLabelText(QString("Export tracking data ..."));
    debout << "size:" << size() << std::endl;
    if (useTrackpoints)
    {
        for (int i = 0; i < size(); ++i)
        {
            progress.setLabelText(QString("Export person %1 of %2 ...").arg(i+1).arg(size()));
            qApp->processEvents();
            progress.setValue(i+1);

            for (int j = 0; j < at(i).size(); ++j)
                // Umrechnung in coordSystem fehlt, auch camera altitude unberuecksichtigt!!!
                out << at(i).firstFrame()+j << " " << at(i).at(j) << " " << at(i).at(j).z() << Qt::endl; // z Koordinate ist Kopf
            out << Qt::endl;
        }
    }
    else
    {
        for (int i = 0; i < size(); ++i)
        {
            progress.setLabelText(QString("Export person %1 of %2 ...").arg(i+1).arg(size()));
            qApp->processEvents();
            progress.setValue(i+1);
            debout << "person: " << i << std::endl;

            for (int j = 0; j < at(i).size(); ++j)
            {
                if (alternateHeight)
                    out << at(i).firstFrame()+j << " " << at(i).at(j) << " " << at(i).at(j).z() << Qt::endl; // z Koordinate ist Kopf
                else
                    out << at(i).firstFrame()+j << " " << at(i).at(j) << " " << at(i).height() << Qt::endl; // z Koordinate ist Kopf
        }
            out << Qt::endl;
        }
    }
}

void TrackerReal::exportXml(QTextStream &outXml, bool alternateHeight, bool useTrackpoints)
{
    int i, j, largestLastFr = largestLastFrame();
    double z;
    //TrackPointReal tp;
    int defaultPersonHeight = 176;

    QProgressDialog progress("Export XML-File",NULL,0,largestLastFr,mMainWindow->window());
    progress.setWindowTitle("Export .xml-File");
    progress.setWindowModality(Qt::WindowModal);
    progress.setVisible(true);
    progress.setValue(0);
    progress.setLabelText(QString("Export tracking data ..."));
    qApp->processEvents();

    // j = person
    outXml << "    <shape>" << Qt::endl;
    for (j = 0; j < size(); ++j)
    {
        if (alternateHeight) // bei variierender groesse wird einfach durchschnittsgroesse genommen, da an treppen gar keine vernuempftige Groesse vorliegt
            outXml << "        <agentInfo ID=\"" << j+1 << "\" color=\"100\" height=\"" << defaultPersonHeight << "\"/>" << Qt::endl;
        else
            outXml << "        <agentInfo ID=\"" << j+1 << "\" color=\"100\" height=\"" << at(j).height() << "\"/>" << Qt::endl;
    }
    outXml << "    </shape>" << Qt::endl << Qt::endl;

    // i = frame; j = person
    for (i = smallestFirstFrame(); i <= largestLastFr; ++i)
    {
        progress.setLabelText(QString("Export data ..."));
        qApp->processEvents();
        progress.setValue(i);

        outXml << "    <frame ID=\""<< i <<"\">" << Qt::endl;
        for (j = 0; j < size(); ++j)
        {
            if (at(j).trackPointExist(i))
            {
                // z-wert ist hier ausnahmsweise nicht der kopf, sondern der boden, die prsonengroesse wird dem obigem person-datenentnommen
                //personID, Frame ID(?) , X , Y , Z
                if (useTrackpoints)
                    z = at(j).trackPointAt(i).z()+defaultPersonHeight;
                    //war, wenn ebene versuche sinnvoll:z = at(j).trackPointAt(i).z()+at(j).height();
                else
                {
                    if (alternateHeight)
                        z = at(j).trackPointAt(i).z()-defaultPersonHeight;
                        //war, wenn ebene versuche sinnvoll: z = at(j).trackPointAt(i).z()-at(j).height();
                    else
                        z = 0; //at(j).height();
                }
                outXml << "        <agent ID=\"" << j+1 << "\" xPos=\"" << at(j).trackPointAt(i).x() << "\" yPos=\"" << at(j).trackPointAt(i).y() << "\" zPos=\"" << z << "\" xVel=\"0\" yVel=\"0\" zVel=\"0\" radiusA=\"18\" radiusB=\"15\" ellipseOrientation=\"130\" ellipseColor=\"0\"/>" << Qt::endl; // z Koordinate ist Boden
            }
        }
        outXml << "    </frame>" << Qt::endl;
    }

    // alte Version der trav datei
//    // j = person
//    outXml << "    <persons>" << endl;
//    for (j = 0; j < size(); ++j)
//    {
//        outXml << "    " << j+1 << " " << at(j).height() << endl;
//    }
//    outXml << "    </persons>" << endl <<endl;

//    // i = frame; j = person
//    for (i = smallestFirstFrame(); i <= largestLastFr; ++i)
//    {
//        outXml << "    <data>" << endl;
//        for (j = 0; j < size(); ++j)
//        {
//            if (at(j).trackPointExist(i))
//            {
//                // z-wert ist hier ausnahmsweise nicht der kopf, sondern der boden, die prsonengroesse wird dem obigem person-datenentnommen
//                //personID, Frame ID(?) , X , Y , Z
//                if (useTrackpoints)
//                    z = at(j).trackPointAt(i).z()+at(j).height();
//                else
//                {
//                    if (alternateHeight)
//                        z = at(j).trackPointAt(i).z()-at(j).height();
//                    else
//                        z = 0; //at(j).height();
//                }
//                outXml << "    " << j+1 << " " << i << " " << at(j).trackPointAt(i) << " " << z << " 1" << endl; // z Koordinate ist Boden
//            }
//        }
//        outXml << "    </data>" << endl;
//    }
}
