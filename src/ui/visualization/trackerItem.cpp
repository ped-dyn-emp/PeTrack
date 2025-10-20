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

#include "trackerItem.h"

#include "animation.h"
#include "control.h"
#include "logger.h"
#include "personStorage.h"
#include "petrack.h"
#include "roiItem.h"
#include "tracker.h"
#include "view.h"
#include "worldImageCorrespondence.h"

#include <QInputDialog>
#include <QtWidgets>

// in x und y gleichermassen skaliertes koordinatensystem,
// da von einer vorherigen intrinsischen kamerakalibrierung ausgegenagen wird,
// so dass pixel quadratisch
TrackerItem::TrackerItem(QWidget *wParent, PersonStorage &storage, QGraphicsItem *parent) :
    QGraphicsItem(parent), mPersonStorage(storage)
{
    mMainWindow    = (class Petrack *) wParent;
    mControlWidget = mMainWindow->getControlWidget();
}

/**
 * @brief Bounding box of drawn to area.
 *
 * This bounding box is used to determine if this Item needs to be redrawn or not.
 * See the official Qt Docs for QGraphicsItem
 *
 * @return (updated) bounding rect of this item
 */
QRectF TrackerItem::boundingRect() const
{
    if(mMainWindow->getImage())
    {
        return QRectF(
            -mMainWindow->getImageBorderSize(),
            -mMainWindow->getImageBorderSize(),
            mMainWindow->getImage()->width(),
            mMainWindow->getImage()->height());
    }
    else
    {
        return QRectF(0, 0, 0, 0);
    }
}
void TrackerItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    if(!(event->modifiers() & Qt::ShiftModifier || event->modifiers() & Qt::ControlModifier ||
         event->modifiers() & Qt::AltModifier))
    {
        TrackPoint p(
            (Vec2F) event->pos(),
            110); // 110 ist ueber 100 (hoechste Qualitaetsstufe) und wird nach einfuegen auf 100 gesetzt
        bool  found    = false;
        int   iNearest = -1;
        float dist, minDist = 1000000.;

        QSet<size_t> onlyVisible = mMainWindow->getPedestrianUserSelection();
        int          frame       = mMainWindow->getAnimation()->getCurrentFrameNum();

        const auto &persons = mPersonStorage.getPersons();
        size_t      i;
        for(i = 0; i < persons.size(); ++i)
        {
            const auto &person = persons[i];
            if(((onlyVisible.empty()) || (onlyVisible.contains(i))) && person.trackPointExist(frame))
            {
                dist                       = person.trackPointAt(frame).distanceToPoint(p);
                auto orientationColorPoint = p.getColorPointForOrientation();
                if((dist < mMainWindow->getHeadSize(nullptr, static_cast<int>(i), frame) / 2.) ||
                   (orientationColorPoint && person.trackPointAt(frame).distanceToPoint(*orientationColorPoint) <
                                                 mMainWindow->getHeadSize(nullptr, static_cast<int>(i), frame) / 2.))
                {
                    if(found)
                    {
                        SPDLOG_WARN("more possible TrackPoints for point");
                        SPDLOG_WARN("         {} in frame {} with low distance:", p, frame);
                        SPDLOG_WARN("         person {} (distance {}),", i + 1, dist);
                        SPDLOG_WARN("         person {} (distance {})", iNearest + 1, minDist);
                        if(minDist > dist)
                        {
                            minDist  = dist;
                            iNearest = static_cast<int>(i);
                        }
                    }
                    else
                    {
                        minDist  = dist;
                        iNearest = static_cast<int>(i);
                        // WAR: break inner loop
                        found = true;
                    }
                }
            }
        }
        QMenu    menu;
        float    height             = 0.f;
        bool     height_set_by_user = false;
        QAction *delTrj = nullptr, *delFutureTrj = nullptr, *delPastTrj = nullptr, *creTrj = nullptr,
                *infoTrj = nullptr, *addComment = nullptr, *setHeight = nullptr, *resetHeight = nullptr,
                *setMarkerID = nullptr, *splitTrj = nullptr;


        QString groupInfo{""};
        QString groupName{""};

        if(found)
        {
            i              = iNearest;
            TrackPerson tp = persons[i];
            height         = tp.height();

            if(height < MIN_HEIGHT + 1)
            {
                if(tp.color().isValid())
                {
                    height = mControlWidget->getColorPlot()->map(tp.color());
                }
            }
            else
            {
                height_set_by_user = true;
            }


            if(tp.getGroups().getEntry(frame).data >= 0)
            {
                const auto groupId = tp.getGroups().getEntry(frame).data;
                const auto group   = mMainWindow->getGroupManager().getGroup(groupId);
                groupName          = QString::fromStdString(group.name);
                groupInfo          = QString("Group: %1").arg(groupName);
            }

            infoTrj      = menu.addAction(QString("PersonNr: %1 %5 height: %2 frames: [%3..%4]")
                                         .arg(i + 1)
                                         .arg(height)
                                         .arg(tp.firstFrame())
                                         .arg(tp.lastFrame())
                                         .arg(groupInfo));
            delTrj       = menu.addAction("Delete whole trajectory");
            delFutureTrj = menu.addAction("Delete past part of the trajectory");
            delPastTrj   = menu.addAction("Delete future part of the trajectory");
            setHeight    = menu.addAction("Set person height");
            setMarkerID  = menu.addAction("Set marker ID");
            splitTrj     = menu.addAction("Split trajectory");
            if(height_set_by_user)
            {
                resetHeight = menu.addAction("Reset height");
            }
            addComment = menu.addAction("Edit comment");
        }
        else
        {
            creTrj = menu.addAction("Create new trajectory");
        }

        QAction *selectedAction = menu.exec(event->screenPos());
        if(selectedAction == nullptr)
        {
            return;
        }
        if(selectedAction == creTrj)
        {
            mMainWindow->addOrMoveManualTrackPoint(event->scenePos());
        }
        else if(selectedAction == delTrj)
        {
            mMainWindow->deleteTrackPoint(event->scenePos(), PersonStorage::TrajectorySegment::Whole);
        }
        else if(selectedAction == delFutureTrj)
        {
            mMainWindow->deleteTrackPoint(event->scenePos(), PersonStorage::TrajectorySegment::Previous);
        }
        else if(selectedAction == delPastTrj)
        {
            mMainWindow->deleteTrackPoint(event->scenePos(), PersonStorage::TrajectorySegment::Following);
        }
        else if(selectedAction == addComment)
        {
            mMainWindow->editTrackPersonComment(event->scenePos());
        }
        else if(selectedAction == setHeight)
        {
            mMainWindow->setTrackPersonHeight(event->scenePos());
        }
        else if(selectedAction == resetHeight)
        {
            mMainWindow->resetTrackPersonHeight(event->scenePos());
        }
        else if(selectedAction == setMarkerID)
        {
            int  currentID = mPersonStorage.getPersons()[i].getMarkerID();
            bool changeID  = true;

            if(currentID != -1)
            {
                QMessageBox::StandardButton reply = QMessageBox::question(
                    mMainWindow,
                    "Overwrite marker ID",
                    QString("The person (%1) already has a markerID (%2)- are you sure you want to assign a new one?")
                        .arg(i + 1)
                        .arg(currentID),
                    QMessageBox::Yes | QMessageBox::No);

                changeID = reply == QMessageBox::Yes;
            }

            if(changeID)
            {
                int markerID = QInputDialog::getInt(mMainWindow, "Enter Marker ID", "Marker ID:");
                mPersonStorage.setMarkerID(i, markerID, true);
            }
        }
        else if(selectedAction == infoTrj)
        {
            if(found)
            {
                QString     out;
                QMessageBox msgBox;
                msgBox.setText(QString("Info for trajectory number %1:").arg(i + 1));

                if(height_set_by_user)
                {
                    out = QString("<table>"
                                  "<tr><td>height:</td><td>%0 cm (edited by user)</td></tr>"
                                  "<tr><td>frames:</td><td>[%1...%2]</td></tr>"
                                  "<tr><td>color:</td><td><font "
                                  "style='display:inline;background:%3;color:#fff;'>%4</font></td></tr>"
                                  "<tr><td>group:</td><td>%18</td></tr>"
                                  "<tr><td>comment:</td><td>%5</td></tr>"
                                  "<tr><td></td><td></td></tr>");
                }
                else
                {
                    out = QString("<table>"
                                  "<tr><td>height:</td><td>%0 cm</td></tr>"
                                  "<tr><td>frames:</td><td>[%1...%2]</td></tr>"
                                  "<tr><td>color:</td><td><font "
                                  "style='display:inline;background:%3;color:#fff;'>%4</font></td></tr>"
                                  "<tr><td>group:</td><td>%18</td></tr>"
                                  "<tr><td>comment:</td><td>%5</td></tr>"
                                  "<tr><td></td><td></td></tr>");
                }
                TrackPerson tp = persons[i];
                if(tp.lastFrame() - tp.firstFrame() > 5)
                {
                    out.append(QString("<tr><td>frame [%6]:</td><td>[%7, %8]</td></tr>"
                                       "<tr><td>frame [%9]:</td><td>[%10, %11]</td></tr>"
                                       "<tr><td colspan='2'>...</td></tr>"
                                       "<tr><td colspan='2'>...</td></tr>"
                                       "<tr><td>frame [%12]:</td><td>[%13, %14]</td></tr>"
                                       "<tr><td>frame [%15]:</td><td>[%16, %17]]</td></tr>"
                                       "</table>")
                                   .toLatin1());
                    msgBox.setInformativeText(out.arg(height)
                                                  .arg(tp.firstFrame())
                                                  .arg(tp.lastFrame())
                                                  .arg(tp.color().name())
                                                  .arg(tp.color().name())
                                                  .arg(tp.comment())
                                                  .arg(tp.firstFrame())
                                                  .arg(tp.at(0).x())
                                                  .arg(tp.at(0).y())
                                                  .arg(tp.firstFrame() + 1)
                                                  .arg(tp.at(1).x())
                                                  .arg(tp.at(1).y())
                                                  .arg(tp.lastFrame() - 1)
                                                  .arg(tp.at(tp.size() - 2).x())
                                                  .arg(tp.at(tp.size() - 2).y())
                                                  .arg(tp.lastFrame())
                                                  .arg(tp.at(tp.size() - 1).x())
                                                  .arg(tp.at(tp.size() - 1).y())
                                                  .arg(groupName));
                }
                else
                {
                    out.append(QString("</table>"));
                    msgBox.setInformativeText(out.arg(height)
                                                  .arg(tp.firstFrame())
                                                  .arg(tp.lastFrame())
                                                  .arg(tp.color().name())
                                                  .arg(tp.color().name())
                                                  .arg(tp.comment()));
                }
                out = QString();

                for(int frameTrackperson = tp.firstFrame(); frameTrackperson <= tp.lastFrame(); frameTrackperson++)
                {
                    out.append(QString("frame [%0]: [%1, %2]\n")
                                   .arg(frameTrackperson)
                                   .arg(tp.at(frameTrackperson - tp.firstFrame()).x())
                                   .arg(tp.at(frameTrackperson - tp.firstFrame()).y()));
                }

                msgBox.setDetailedText(out);

                msgBox.setWindowTitle("PeTrack");
                msgBox.setIcon(QMessageBox::Information);
                msgBox.setStandardButtons(QMessageBox::Ok);
                msgBox.setDefaultButton(QMessageBox::Ok);
                msgBox.setEscapeButton(QMessageBox::Ok);
                msgBox.exec();
            }
        }
        else if(selectedAction == splitTrj)
        {
            mMainWindow->splitTrackPerson(event->scenePos());
        }
    }
    mMainWindow->getScene()->update();
}

void TrackerItem::paint(QPainter *painter, const QStyleOptionGraphicsItem * /*option*/, QWidget * /*widget*/)
{
    int          from, to;
    int          curFrame = mMainWindow->getAnimation()->getCurrentFrameNum();
    QPen         ellipsePen;
    QRectF       rect;
    Vec2F        normalVector;
    cv::Subdiv2D subdiv;
    QPen         linePen;
    QPen         numberPen;
    QPen         groundPositionPen;
    QPen         groundPathPen;
    QPen         currentPointLineWidthPen;
    QPen         trackPointLineWidthPen;
    double       pSP  = (double) mControlWidget->getTrackCurrentPointSize();
    double       pS   = (double) mControlWidget->getTrackPointSize();
    double       pSC  = (double) mControlWidget->getTrackColColorSize();
    double       pSM  = (double) mControlWidget->getTrackColorMarkerSize();
    double       pSN  = (double) mControlWidget->getTrackNumberSize();
    double       pSG  = (double) mControlWidget->getTrackGroundPositionSize();
    double       pSGP = (double) mControlWidget->getTrackGroundPathSize();

    QColor pGPC = mControlWidget->getTrackGroundPathColor();
    QColor pTPC = mControlWidget->getTrackPathColor();
    QFont  font, heightFont;
    float  x_offset = 0, y_offset = 0;
    float  y_switch = 0, x_switch = 0;
    double hS;

    painter->drawRect(boundingRect());

    linePen.setColor(pTPC);
    linePen.setWidth(mControlWidget->getTrackPathWidth());

    ellipsePen.setWidth(3);
    currentPointLineWidthPen.setWidth(mControlWidget->getTrackCurrentPointLineWidth());
    if(mControlWidget->isTrackNumberBoldChecked())
    {
        font.setBold(true);
    }
    else
    {
        font.setBold(false);
    }
    font.setPixelSize(mControlWidget->getTrackNumberSize());
    heightFont.setPixelSize(mControlWidget->getTrackColColorSize());
    painter->setFont(font);
    numberPen.setColor(Qt::red);
    groundPositionPen.setColor(Qt::green);
    groundPositionPen.setWidth(pSG);
    groundPathPen.setColor(pGPC);
    groundPathPen.setWidth(pSGP);


    if(mControlWidget->isShowVoronoiCellsChecked() && mPersonStorage.nbPersons() != 0)
    {
        // ToDo: adjust subdiv rect to correct area
        QRectF qrect = mMainWindow->getRecoRoiItem()->rect();

        cv::Point3f leftTop = mMainWindow->getExtrCalibration()->get3DPoint(cv::Point2f(qrect.left(), qrect.top()), 0);
        cv::Point3f rightBottom =
            mMainWindow->getExtrCalibration()->get3DPoint(cv::Point2f(qrect.right(), qrect.bottom()), 0);

        x_offset = -std::min(leftTop.x, rightBottom.x);
        y_offset = -std::min(leftTop.y, rightBottom.y);
        x_switch = rightBottom.x < leftTop.x ? abs(rightBottom.x - leftTop.x) : 0;
        y_switch = rightBottom.y < leftTop.y ? abs(leftTop.y - rightBottom.y) : 0;
        SPDLOG_INFO("x_offset: {}, y_offset: {}, x_switch: {}, y_switch: {}", x_offset, y_offset, x_switch, y_switch);

        cv::Rect delaunyROI(
            leftTop.x + x_offset,
            leftTop.y + y_offset,
            x_switch > 0 ? x_switch : (rightBottom.x - leftTop.x),
            y_switch > 0 ? y_switch : (rightBottom.y - leftTop.y));
        SPDLOG_INFO(
            "Rect size: P({}, {}), width: {}, height: {}",
            delaunyROI.x,
            delaunyROI.y,
            delaunyROI.width,
            delaunyROI.height);

        subdiv.initDelaunay(delaunyROI);
    }

    auto        pedestrianToPaint = mMainWindow->getPedestrianUserSelection();
    const auto &persons           = mPersonStorage.getPersons();
    for(size_t i = 0; i < persons.size(); ++i) // ueber TrackPerson
    {
        const auto &person = persons[i];
        // show current frame
        if(pedestrianToPaint.contains(i) || pedestrianToPaint.empty())
        {
            if(person.trackPointExist(curFrame))
            {
                if(mControlWidget->isTrackHeadSizedChecked())
                {
                    pSP = mMainWindow->getHeadSize(nullptr, static_cast<int>(i), curFrame);
                }
                const TrackPoint &tp = person.trackPointAt(curFrame);
                if(mControlWidget->isTrackShowCurrentPointChecked())
                {
                    painter->setBrush(Qt::NoBrush);
                    if(person.newReco())
                    {
                        currentPointLineWidthPen.setColor(Qt::green);
                    }
                    else
                    {
                        currentPointLineWidthPen.setColor(Qt::blue);
                    }
                    painter->setPen(currentPointLineWidthPen);
                    rect.setRect(tp.x() - pSP / 2., tp.y() - pSP / 2., pSP, pSP);
                    painter->drawEllipse(rect); // direkt waere nur int erlaubt tp.x()-5., tp.y()-5., 10., 10.
                }

                if(mControlWidget->isTrackShowSearchSizeChecked())
                {
                    painter->setBrush(Qt::NoBrush);
                    painter->setPen(Qt::yellow);
                    hS = mMainWindow->winSize(nullptr, static_cast<int>(i), curFrame);
                    if(hS < 2)
                    {
                        hS = 2; // entspricht Vorgehen in tracker.cpp
                    }
                    for(int j = 0; j <= mControlWidget->getTrackRegionLevels(); ++j)
                    {
                        rect.setRect(tp.x() - hS / 2., tp.y() - hS / 2., hS, hS);
                        painter->drawRect(rect);
                        hS *= 2;
                    }
                }

                if(mControlWidget->isTrackShowColorMarkerChecked())
                {
                    // farbe des trackpoints
                    auto color      = tp.getColorForHeightMap();
                    auto colorPoint = tp.getColorPointForOrientation();
                    if(!colorPoint && color) // its a multicolor point instead
                    {
                        colorPoint = tp.getMultiColorMarker()->mColorPoint;
                    }
                    if(colorPoint)
                    {
                        painter->setBrush(Qt::NoBrush);
                        // we don't save a specific color for japan marker since they all have the same color, however
                        // we still want to be able to see the colorpoint
                        ellipsePen.setColor(color ? *color : JapanMarker::COLOR);
                        ellipsePen.setWidth(mControlWidget->getTrackColorMarkerLineWidth());
                        painter->setPen(ellipsePen);
                        rect.setRect(colorPoint->x() - pSM / 2., colorPoint->y() - pSM / 2., pSM, pSM);
                        painter->drawEllipse(rect);
                    }
                }

                // berechnung der normalen, die zur positionierung der nummerieung und der gesamtfarbe dient
                if(((mControlWidget->isTrackShowColColorChecked()) && (person.color().isValid())) ||
                   (mControlWidget->isTrackShowNumberChecked()) ||
                   ((mControlWidget->isTrackShowColColorChecked()) &&
                    ((person.height() > MIN_HEIGHT) ||
                     (tp.getStereoMarker() &&
                      (mControlWidget
                           ->isTrackShowHeightIndividualChecked()))))) //  Hoehe kann auf Treppen auch negativ werden,
                                                                       //  wenn koord weiter oben angesetzt wird
                {
                    if(auto colorPoint = tp.getColorPointForOrientation())
                    {
                        normalVector = (tp.pixelPoint() - *colorPoint).normal();
                        normalVector.normalize();
                        if(normalVector.length() <
                           .001) // if orientation color point is very close to pixelpoint of trackpoint
                        {
                            normalVector.set(1., 0.);
                        }
                    }
                    else
                    {
                        // man koennte auch lastNormal von anderem trackpath nehmen statt 1, 0
                        normalVector.set(1., 0.);
                        // den vorherigen trackpoint finden, wo reco farbe erzeugt hat und somit colpoint vorliegt
                        for(int j = curFrame - person.firstFrame(); j > -1; --j)
                        {
                            if(auto colorPoint = person.at(j).getColorPointForOrientation())
                            {
                                normalVector = (person.at(j).pixelPoint() - *colorPoint).normal();
                                normalVector.normalize();
                                break;
                            }
                        }
                        // einen nachfolgenden trackpoint suchen, wenn vorher keiner mit farbe war
                        // zB wenn rueckwaerts abgespielt wird
                        if((normalVector.x() == 1.) && (normalVector.y() == 0.))
                        {
                            for(int j = curFrame - person.firstFrame() + 1; j < person.size(); ++j)
                            {
                                if(auto colorPoint = person.at(j).getColorPointForOrientation())
                                {
                                    normalVector = (person.at(j).pixelPoint() - *colorPoint).normal();
                                    normalVector.normalize();
                                    break;
                                }
                            }
                        }
                    }
                }

                // farbe der gesamten trackperson
                double height = person.height();
                if(mControlWidget->isTrackShowColColorChecked())
                {
                    painter->setPen(numberPen);
                    painter->setBrush(Qt::NoBrush);
                    rect.setRect(tp.x() + 10, tp.y() + 10, 15 * pSC, 10 * pSC);
                    painter->drawText(rect, person.comment());

                    if(person.getMarkerID() >= 0)
                    {
                        QPen markerIDPen;
                        auto codeMarker = tp.getCodeMarker();
                        markerIDPen.setColor(codeMarker && codeMarker->mMarkerId >= 0 ? Qt::green : Qt::blue);
                        painter->setPen(markerIDPen);
                        painter->drawText(QPointF{tp.x(), tp.y()}, QString("id=%1").arg(person.getMarkerID()));
                    }
                }

                if((mControlWidget->isTrackShowColColorChecked()) && (person.color().isValid()))
                {
                    painter->setPen(Qt::NoPen);
                    painter->setBrush(QBrush(person.color()));
                    rect.setRect(
                        tp.x() + (pSP + pSC) * 0.6 * normalVector.x() - pSC / 2.,
                        tp.y() + (pSP + pSC) * 0.6 * normalVector.y() - pSC / 2.,
                        pSC,
                        pSC); // 11
                    painter->drawEllipse(rect);
                }
                else if(
                    (mControlWidget->isTrackShowColColorChecked()) &&
                    ((height > MIN_HEIGHT) ||
                     ((tp.getStereoMarker()) &&
                      (mControlWidget
                           ->isTrackShowHeightIndividualChecked())))) // Hoehe  && (person.height() > 0.) Hoehe
                                                                      // kann auf Treppen auch negativ werden,
                                                                      // wenn koord weiter oben angesetzt wird
                {
                    painter->setFont(heightFont);
                    if(mControlWidget->isTrackShowHeightIndividualChecked() &&
                       tp.getStereoMarker()) // Hoehe incl individual fuer jeden trackpoint
                    {
                        painter->setPen(numberPen);
                        painter->setBrush(Qt::NoBrush);
                        rect.setRect(
                            tp.x() + (pSP + pSC) * 0.6 * normalVector.x() - pSC / 2.,
                            tp.y() + (pSP + pSC) * 0.6 * normalVector.y() - pSC / 2.,
                            3 * pSC,
                            2.5 * pSC); // 11
                        if(height < MIN_HEIGHT + 1)
                        {
                            if(mControlWidget->getCalibCoordDimension() == 0) // 3D
                            {
                                painter->drawText(
                                    rect,
                                    Qt::AlignHCenter,
                                    QString("-\n%2").arg(
                                        -mControlWidget->getExtrinsicParameters().trans3 -
                                            tp.stereoGetStereoPoint().z(),
                                        6,
                                        'f',
                                        1));
                            }
                            else
                            {
                                painter->drawText(
                                    rect,
                                    Qt::AlignHCenter,
                                    QString("-\n%2").arg(
                                        mControlWidget->getCameraAltitude() - tp.stereoGetStereoPoint().z(),
                                        6,
                                        'f',
                                        1));
                            }
                        }
                        else
                        {
                            if(mControlWidget->getCalibCoordDimension() == 0) // 3D
                            {
                                painter->drawText(
                                    rect,
                                    Qt::AlignHCenter,
                                    QString("%1\n%2")
                                        .arg(height, 6, 'f', 1)
                                        .arg(
                                            -mControlWidget->getExtrinsicParameters().trans3 -
                                                tp.stereoGetStereoPoint().z(),
                                            6,
                                            'f',
                                            1));
                            }
                            else
                            {
                                painter->drawText(
                                    rect,
                                    Qt::AlignHCenter,
                                    QString("%1\n%2")
                                        .arg(height, 6, 'f', 1)
                                        .arg(
                                            mControlWidget->getCameraAltitude() - tp.stereoGetStereoPoint().z(),
                                            6,
                                            'f',
                                            1));
                            }
                        }
                    }
                    else
                    {
                        painter->setPen(numberPen);
                        painter->setBrush(Qt::NoBrush);
                        rect.setRect(
                            tp.x() + (pSP + pSC) * 0.6 * normalVector.x() - pSC / 2.,
                            tp.y() + (pSP + pSC) * 0.6 * normalVector.y() - pSC / 2.,
                            3 * pSC,
                            2 * pSC); // 11
                        painter->drawText(rect, Qt::AlignHCenter, QString("%1").arg(height, 6, 'f', 1));
                    }
                    painter->setFont(font);
                }
                if(mControlWidget->isTrackShowNumberChecked())
                {
                    // listennummer
                    painter->setPen(numberPen);
                    painter->setBrush(Qt::NoBrush);
                    rect.setRect(
                        tp.x() - (pSP + pSN) * 0.6 * normalVector.x() - pSN,
                        tp.y() - (pSP + pSN) * 0.6 * normalVector.y() - pSN / 2.,
                        2. * pSN,
                        pSN); // 11
                    painter->drawText(rect, Qt::AlignHCenter, QString("%1").arg(i + 1));
                }
                if(mControlWidget->isTrackShowGroundPositionChecked())
                {
                    // ground position
                    painter->setPen(groundPositionPen);
                    painter->setBrush(Qt::NoBrush);
                    if(mControlWidget->getCalibCoordDimension() == 0) // 3D
                    {
                        double      cross_size = 15 + pSG * 0.25;
                        cv::Point3f p3d_height;
                        if(height < MIN_HEIGHT + 1)
                        {
                            p3d_height = mMainWindow->getExtrCalibration()->get3DPoint(
                                cv::Point2f(tp.x(), tp.y()), mControlWidget->getColorPlot()->map(person.color()));
                        }
                        else
                        {
                            if(tp.getStereoMarker())
                            {
                                p3d_height = mMainWindow->getExtrCalibration()->get3DPoint(
                                    cv::Point2f(tp.x(), tp.y()),
                                    -mControlWidget->getExtrinsicParameters().trans3 - tp.stereoGetStereoPoint().z());
                            }
                            else
                            {
                                p3d_height = mMainWindow->getExtrCalibration()->get3DPoint(
                                    cv::Point2f(tp.x(), tp.y()), height /*mControlWidget->mapDefaultHeight->value()*/);
                            }
                        }
                        p3d_height.z           = 0;
                        cv::Point2f p2d_ground = mMainWindow->getExtrCalibration()->getImagePoint(p3d_height);
                        QPointF     axis =
                            mMainWindow->getWorldImageCorrespondence().getCmPerPixel(p2d_ground.x, p2d_ground.y, 0);
                        painter->drawLine(QLineF(
                            p2d_ground.x - cross_size * 0.5 * pow(axis.x(), -1),
                            p2d_ground.y - cross_size * 0.5 * pow(axis.y(), -1),
                            p2d_ground.x + cross_size * 0.5 * pow(axis.x(), -1),
                            p2d_ground.y + cross_size * 0.5 * pow(axis.y(), -1)));
                        painter->drawLine(QLineF(
                            p2d_ground.x - cross_size * 0.5 * pow(axis.x(), -1),
                            p2d_ground.y + cross_size * 0.5 * pow(axis.y(), -1),
                            p2d_ground.x + cross_size * 0.5 * pow(axis.x(), -1),
                            p2d_ground.y - cross_size * 0.5 * pow(axis.y(), -1)));
                        painter->drawLine(QLineF(p2d_ground.x, p2d_ground.y, tp.x(), tp.y()));
                    }
                    else // 2D
                    {
                    }
                }
                if(mControlWidget->isShowVoronoiCellsChecked())
                {
                    if(mControlWidget->getCalibCoordDimension() == 0) // 3D
                    {
                        cv::Point3f p3d_height;
                        if(height < MIN_HEIGHT + 1)
                        {
                            p3d_height = mMainWindow->getExtrCalibration()->get3DPoint(
                                cv::Point2f(tp.x(), tp.y()), mControlWidget->getColorPlot()->map(person.color()));
                        }
                        else
                        {
                            if(tp.getStereoMarker())
                            {
                                p3d_height = mMainWindow->getExtrCalibration()->get3DPoint(
                                    cv::Point2f(tp.x(), tp.y()),
                                    -mControlWidget->getExtrinsicParameters().trans3 - tp.stereoGetStereoPoint().z());
                            }
                            else
                            {
                                p3d_height = mMainWindow->getExtrCalibration()->get3DPoint(
                                    cv::Point2f(tp.x(), tp.y()), height /*mControlWidget->mapDefaultHeight->value()*/);
                            }
                        }

                        SPDLOG_INFO("insert P({}, {}) to subdiv", p3d_height.x + x_offset, p3d_height.y + y_offset);

                        subdiv.insert(cv::Point2f(
                            x_switch > 0 ? x_switch - p3d_height.x + x_offset : p3d_height.x + x_offset,
                            y_switch > 0 ? y_switch - p3d_height.y + y_offset :
                                           p3d_height.y + y_offset)); // p2d_ground);
                    }
                }
            }

            const bool showPathLike = mControlWidget->isTrackShowPointsChecked() ||
                                      mControlWidget->isTrackShowPathChecked() ||
                                      mControlWidget->isTrackShowGroundPathChecked();
            const bool isVisible       = person.trackPointExist(curFrame);
            const bool drawOnlyVisible = mControlWidget->isTrackShowOnlyVisibleChecked();
            const bool personToDraw    = !drawOnlyVisible || isVisible;
            if(showPathLike && personToDraw)
            {
                const bool hasActiveSelection = !pedestrianToPaint.empty();
                if(mControlWidget->getTrackShowBefore() == -1 ||
                   (mControlWidget->isTrackShowComplPathChecked() && hasActiveSelection))
                {
                    from = 0;
                }
                else
                {
                    from = curFrame - person.firstFrame() - mControlWidget->getTrackShowBefore();
                    if(from < 0)
                    {
                        from = 0;
                    }
                }
                if(mControlWidget->getTrackShowAfter() == -1 ||
                   (mControlWidget->isTrackShowComplPathChecked() && hasActiveSelection))
                {
                    to = person.size();
                }
                else
                {
                    to = curFrame - person.firstFrame() + mControlWidget->getTrackShowAfter() + 1;
                    if(to > person.size())
                    {
                        to = person.size();
                    }
                }
                for(int j = from; j < to; ++j) // ueber TrackPoint
                {
                    // path
                    if(mControlWidget->isTrackShowPathChecked())
                    {
                        if(j != from) // autom. > 0
                        {
                            painter->setPen(linePen);
                            painter->setBrush(Qt::NoBrush);

                            // nur Linie zeichnen, wenn x oder y sich unterscheidet, sonst Punkt
                            // die Unterscheidung ist noetig, da Qt sonst grosses quadrat beim ranzoomen zeichnet
                            if((person.at(j - 1).pixelPoint().toQPointF().x() !=
                                person.at(j).pixelPoint().toQPointF().x()) ||
                               (person.at(j - 1).pixelPoint().toQPointF().y() !=
                                person.at(j).pixelPoint().toQPointF().y()))
                            {
                                painter->drawLine(
                                    person.at(j - 1).pixelPoint().toQPointF(), person.at(j).pixelPoint().toQPointF());
                            }
                            else
                            {
                                painter->drawPoint(person.at(j - 1).pixelPoint().toQPointF());
                            }
                        }
                    }
                    // path on ground
                    if(mControlWidget->isTrackShowGroundPathChecked())
                    {
                        if(j != from)
                        {
                            // ground position
                            painter->setPen(groundPathPen);
                            painter->setBrush(Qt::NoBrush);
                            if(mControlWidget->getCalibCoordDimension() == 0) // 3D
                            {
                                cv::Point3f p3d_height_p1, p3d_height_p2;
                                if(person.height() < MIN_HEIGHT + 1)
                                {
                                    p3d_height_p1 = mMainWindow->getExtrCalibration()->get3DPoint(
                                        cv::Point2f(person.at(j - 1).x(), person.at(j - 1).y()),
                                        mControlWidget->getColorPlot()->map(person.color()));
                                    p3d_height_p2 = mMainWindow->getExtrCalibration()->get3DPoint(
                                        cv::Point2f(person.at(j).x(), person.at(j).y()),
                                        mControlWidget->getColorPlot()->map(person.color()));
                                }
                                else
                                {
                                    if(person.at(j - 1).getStereoMarker() && person.at(j).getStereoMarker())
                                    {
                                        p3d_height_p1 = mMainWindow->getExtrCalibration()->get3DPoint(
                                            cv::Point2f(person.at(j - 1).x(), person.at(j - 1).y()),
                                            -mControlWidget->getExtrinsicParameters().trans3 -
                                                person.at(j - 1).stereoGetStereoPoint().z());
                                        p3d_height_p2 = mMainWindow->getExtrCalibration()->get3DPoint(
                                            cv::Point2f(person.at(j).x(), person.at(j).y()),
                                            -mControlWidget->getExtrinsicParameters().trans3 -
                                                person.at(j).stereoGetStereoPoint().z());
                                    }
                                    else
                                    {
                                        p3d_height_p1 = mMainWindow->getExtrCalibration()->get3DPoint(
                                            cv::Point2f(person.at(j - 1).x(), person.at(j - 1).y()), person.height());
                                        p3d_height_p2 = mMainWindow->getExtrCalibration()->get3DPoint(
                                            cv::Point2f(person.at(j).x(), person.at(j).y()), person.height());
                                    }
                                }
                                p3d_height_p1.z = 0;
                                p3d_height_p2.z = 0;
                                cv::Point2f p2d_ground_p1 =
                                    mMainWindow->getExtrCalibration()->getImagePoint(p3d_height_p1);
                                cv::Point2f p2d_ground_p2 =
                                    mMainWindow->getExtrCalibration()->getImagePoint(p3d_height_p2);
                                // nur Linie zeichnen, wenn x oder y sich unterscheidet, sonst Punkt
                                // die Unterscheidung ist noetig, da Qt sonst grosses quadrat beim ranzoomen zeichnet
                                if(p2d_ground_p1.x != p2d_ground_p2.x || p2d_ground_p1.y != p2d_ground_p2.y)
                                {
                                    painter->drawLine(
                                        QLineF(p2d_ground_p1.x, p2d_ground_p1.y, p2d_ground_p2.x, p2d_ground_p2.y));
                                }
                                else
                                {
                                    painter->drawPoint(p2d_ground_p1.x, p2d_ground_p1.y);
                                }
                            }
                            else // 2D
                            {
                            }
                        }
                    }

                    // points before and after
                    if(mControlWidget->isTrackShowPointsChecked())
                    {
                        if(person.firstFrame() + j != curFrame)
                        {
                            if(mControlWidget->isTrackShowPointsColoredChecked())
                            {
                                if(auto color = person.at(j).getColorForHeightMap())
                                {
                                    painter->setPen(Qt::NoPen);
                                    painter->setBrush(QBrush(*color));
                                    rect.setRect(person.at(j).x() - pS / 2., person.at(j).y() - pS / 2., pS,
                                                 pS); // 7
                                }
                            }
                            else
                            {
                                trackPointLineWidthPen.setColor(Qt::red);
                                trackPointLineWidthPen.setWidth(mControlWidget->getTrackShowPointsLineWidth());
                                painter->setPen(trackPointLineWidthPen);

                                painter->setBrush(Qt::NoBrush);
                                rect.setRect(person.at(j).x() - pS / 2., person.at(j).y() - pS / 2., pS, pS);
                            }
                            painter->drawEllipse(rect);
                        }
                    }
                }
            }
        }
    }

    // Mat& img, Subdiv2D& subdiv )
    if(mControlWidget->isShowVoronoiCellsChecked() && !mPersonStorage.getPersons().empty())
    {
        std::vector<std::vector<cv::Point2f>> facets3D;
        std::vector<cv::Point2f>              centers3D;

        // get Voronoi cell info from subDiv in 3D coordinates on ground (z=0)
        subdiv.getVoronoiFacetList(std::vector<int>(), facets3D, centers3D);

        painter->setClipRect(mMainWindow->getRecoRoiItem()
                                 ->rect()); // 0,0,mMainWindow->getImage()->width(),mMainWindow->getImage()->height());

        // cell by cell
        for(size_t i = 0; i < facets3D.size(); i++)
        {
            centers3D.at(i).x = x_switch > 0 ? x_switch - centers3D.at(i).x - x_offset : centers3D.at(i).x - x_offset;
            centers3D.at(i).y = y_switch > 0 ? y_switch - centers3D.at(i).y - y_offset : centers3D.at(i).y - y_offset;
            // voronoi cell center in 2D
            cv::Point2f center2D =
                mMainWindow->getExtrCalibration()->getImagePoint(cv::Point3f(centers3D.at(i).x, centers3D.at(i).y, 0));

            std::vector<QPointF> ifacet2D;
            QPointF              circleStart, circleEnd;
            float                area = 0;
            float                r = 50, m = 0, n = 0, s1_x = 0, s2_x = 0, s1_y = 0, s2_y = 0;
            bool                 circleStarted = false;
            for(size_t j = 0; j < facets3D[i].size(); j++)
            {
                facets3D.at(i).at(j).x =
                    x_switch > 0 ? x_switch - facets3D.at(i).at(j).x - x_offset : facets3D.at(i).at(j).x - x_offset;
                facets3D.at(i).at(j).y =
                    y_switch > 0 ? y_switch - facets3D.at(i).at(j).y - y_offset : facets3D.at(i).at(j).y - y_offset;

                cv::Point2f point2D = mMainWindow->getExtrCalibration()->getImagePoint(
                    cv::Point3f(facets3D.at(i).at(j).x, facets3D.at(i).at(j).y, 0));

                SPDLOG_INFO(
                    "facets3D.at({}).at({}).x = {}, .y = {}", i, j, facets3D.at(i).at(j).x, facets3D.at(i).at(j).y);
                SPDLOG_INFO("point2D.x = {}, .y = {}", point2D.x, point2D.y);

                if constexpr(
                    false && sqrt(
                                 pow((facets3D.at(i).at(j).x - centers3D.at(i).x), 2) +
                                 pow((facets3D.at(i).at(j).y - centers3D.at(i).y), 2)) > r)
                {
                    if(circleStarted)
                    {
                        m = (facets3D.at(i).at(j).y - facets3D.at(i).at((j - 1) % facets3D.at(i).size()).y) /
                            (facets3D.at(i).at(j).x - facets3D.at(i).at((j - 1) % facets3D.at(i).size()).x);

                        // End punkt berechnen (Schnittpunkt Gerade-Kreis)
                        // Steigung der Geraden
                        m = (facets3D.at(i).at(j).y - facets3D.at(i).at((j + 1) % facets3D.at(i).size()).y) /
                            (facets3D.at(i).at(j).x - facets3D.at(i).at((j + 1) % facets3D.at(i).size()).x);
                        // Achsenabschnitt der Geraden
                        n = facets3D.at(i).at(j).y - m * facets3D.at(i).at(j).x;

                        float p = -((m * n - m * centers3D.at(i).y - centers3D.at(i).x) / (1 + pow(m, 2)));
                        float q = sqrt(
                            (pow(r, 2) - pow(centers3D.at(i).x, 2) - pow(centers3D.at(i).y, 2) - pow(n, 2) -
                             2 * n * centers3D.at(i).y) /
                                (1 + pow(m, 2)) +
                            pow((m * n - m * centers3D.at(i).y - centers3D.at(i).x) / (1 + pow(m, 2)), 2));
                        // Schnittpunkte mit Kreis
                        s1_x = p + q;
                        s1_y = m * s1_x + n;

                        s2_x = p - q;
                        s2_y = m * s2_x + n;

                        facets3D[i][j] = cv::Point2f(s1_x, s1_y);

                        point2D   = mMainWindow->getExtrCalibration()->getImagePoint(cv::Point3f(s1_x, s1_y, 0));
                        circleEnd = QPointF(point2D.x, point2D.y);

                        ifacet2D.push_back(QPointF(center2D.x, center2D.y));
                        ifacet2D.push_back(QPointF(point2D.x, point2D.y));

                        SPDLOG_INFO("End point: ({}, {})", s1_x, s1_y);
                    }
                    else
                    {
                        // start punkt berechnen

                        m = (facets3D.at(i).at(j).y - facets3D.at(i).at((j - 1) % facets3D.at(i).size()).y) /
                            (facets3D.at(i).at(j).x - facets3D.at(i).at((j - 1) % facets3D.at(i).size()).x);

                        n = facets3D.at(i).at(j).y - m * facets3D.at(i).at(j).x;

                        float p = -((m * n - m * centers3D.at(i).y - centers3D.at(i).x) / (1 + pow(m, 2)));
                        float q = sqrt(
                            (pow(r, 2) - pow(centers3D.at(i).x, 2) - pow(centers3D.at(i).y, 2) - pow(n, 2) -
                             2 * n * centers3D.at(i).y) /
                                (1 + pow(m, 2)) +
                            pow((m * n - m * centers3D.at(i).y - centers3D.at(i).x) / (1 + pow(m, 2)), 2));

                        // Schnittpunkte mit Kreis
                        s1_x = p + q;
                        s1_y = m * s1_x + n;

                        s2_x = p - q;
                        s2_y = m * s2_x + n;

                        SPDLOG_INFO(
                            "x={} G(x)={} K(x)={} = {}",
                            s1_x,
                            (m * s1_x + n),
                            pow(s1_x - centers3D.at(i).x, 2) + pow(s1_y - centers3D.at(i).y, 2),
                            pow(r, 2));
                        SPDLOG_INFO(
                            "x={} G(x)={} K(x)={} = {}",
                            s2_x,
                            (m * s2_x + n),
                            pow(s2_x - centers3D.at(i).x, 2) + pow(s2_y - centers3D.at(i).y, 2),
                            pow(r, 2));

                        facets3D[i][j] = cv::Point2f(s1_x, s1_y);

                        point2D = mMainWindow->getExtrCalibration()->getImagePoint(cv::Point3f(s1_x, s1_y, 0));
                        ifacet2D.push_back(QPointF(point2D.x, point2D.y));
                        circleStart   = QPointF(point2D.x, point2D.y);
                        circleStarted = true;

                        SPDLOG_INFO("Start point: ({}, {})", s1_x, s1_y);
                    }
                }
                else
                {
                    ifacet2D.emplace_back(point2D.x, point2D.y);
                    area += (facets3D.at(i).at(j).x * facets3D.at(i).at((j + 1) % facets3D[i].size()).y);
                    area -= (facets3D.at(i).at((j + 1) % facets3D[i].size()).x * facets3D.at(i).at(j).y);
                }
            }
            area *= 0.5;
            area = 1.0 / area;
            area *= 10000;

            QColor color;

            color.setHsv((255 - area * 25.5) < 0 ? 0 : (255 - area * 25.5), 255, 255, 128);

            QVector<QPointF> ifacet_vec;
            for(size_t i = 0; i < ifacet2D.size(); i++)
            {
                ifacet_vec.append(ifacet2D.at(i));
            }

            painter->setBrush(color);
            painter->setPen(Qt::black);
            if(ifacet2D.size() == 0)
            {
                painter->drawEllipse(QPointF(center2D.x, center2D.y), 100, 100);
            }
            painter->drawConvexPolygon(QPolygonF(ifacet_vec));

            // voronoi cell point
            painter->setBrush(Qt::black);
            painter->setPen(Qt::red);
            painter->drawEllipse(QPointF(center2D.x, center2D.y), 5, 5);
        }
    }
}
