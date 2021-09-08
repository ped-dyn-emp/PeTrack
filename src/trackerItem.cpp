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

#include "trackerItem.h"

#include "animation.h"
#include "control.h"
#include "petrack.h"
#include "recognitionRoiItem.h"
#include "tracker.h"
#include "trackingRoiItem.h"
#include "view.h"

#include <QInputDialog>
#include <QtWidgets>

// in x und y gleichermassen skaliertes koordinatensystem,
// da von einer vorherigen intrinsischen kamerakalibrierung ausgegenagen wird,
// so dass pixel quadratisch
TrackerItem::TrackerItem(QWidget *wParent, Tracker *tracker, QGraphicsItem *parent) : QGraphicsItem(parent)
{
    mMainWindow    = (class Petrack *) wParent;
    mControlWidget = mMainWindow->getControlWidget();
    mTracker       = tracker;
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
        return QRectF(
            -mMainWindow->getImageBorderSize(),
            -mMainWindow->getImageBorderSize(),
            mMainWindow->getImage()->width(),
            mMainWindow->getImage()->height());
    else
        return QRectF(0, 0, 0, 0);
}
void TrackerItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    if(!(event->modifiers() & Qt::ShiftModifier || event->modifiers() & Qt::ControlModifier ||
         event->modifiers() & Qt::AltModifier))
    {
        TrackPoint p(
            (Vec2F) event->pos(),
            110); // 110 ist ueber 100 (hoechste Qualitaetsstufe) und wird nach einfuegen auf 100 gesetzt
        bool  found = false;
        int   i, iNearest   = -1;
        float dist, minDist = 1000000.;

        QSet<int> onlyVisible = mMainWindow->getPedestrianUserSelection();
        int       frame       = mMainWindow->getAnimation()->getCurrentFrameNum();

        for(i = 0; i < mTracker->size(); ++i) // !found &&  // ueber TrackPerson
        {
            if(((onlyVisible.empty()) || (onlyVisible.contains(i))) && mTracker->at(i).trackPointExist(frame))
            {
                dist = mTracker->at(i).trackPointAt(frame).distanceToPoint(p);
                if((dist < mMainWindow->getHeadSize(nullptr, i, frame) / 2.) ||
                   ((mTracker->at(i).trackPointAt(frame).distanceToPoint(p.colPoint()) <
                     mMainWindow->getHeadSize(nullptr, i, frame) / 2.)))
                {
                    if(found)
                    {
                        debout << "Warning: more possible trackpoints for point" << std::endl;
                        debout << "         " << p << " in frame " << frame << " with low distance:" << std::endl;
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
        QMenu       menu;
        TrackPerson tp;
        float       height             = 0.f;
        bool        height_set_by_user = false;
        QAction *   delTrj = nullptr, *delFutureTrj = nullptr, *delPastTrj = nullptr, *creTrj = nullptr,
                *infoTrj = nullptr, *addComment = nullptr, *setHeight = nullptr, *resetHeight = nullptr;

        if(found)
        {
            i      = iNearest;
            tp     = mTracker->at(i);
            height = tp.height();

            if(height < MIN_HEIGHT + 1)
            {
                if(tp.color().isValid())
                    height = mControlWidget->getColorPlot()->map(tp.color());
            }
            else
            {
                height_set_by_user = true;
            }

            infoTrj      = menu.addAction(QString("PersonNr: %1 height: %2 frames: [%3..%4]")
                                         .arg(i + 1)
                                         .arg(height)
                                         .arg(tp.firstFrame())
                                         .arg(tp.lastFrame()));
            delTrj       = menu.addAction("Delete whole trajectory");
            delFutureTrj = menu.addAction("Delete past part of the trajectory");
            delPastTrj   = menu.addAction("Delete future part of the trajectory");
            setHeight    = menu.addAction("Set person height");
            if(height_set_by_user)
                resetHeight = menu.addAction("Reset height");
            addComment = menu.addAction("Edit comment");
        }
        else
        {
            creTrj = menu.addAction("Create new trajectory");
        }

        QAction *selectedAction = menu.exec(event->screenPos());
        if(selectedAction == creTrj)
        {
            mMainWindow->addOrMoveManualTrackPoint(event->scenePos());
        }
        else if(selectedAction == delTrj)
        {
            mMainWindow->deleteTrackPoint(event->scenePos(), 0);
        }
        else if(selectedAction == delFutureTrj)
        {
            mMainWindow->deleteTrackPoint(event->scenePos(), -1);
        }
        else if(selectedAction == delPastTrj)
        {
            mMainWindow->deleteTrackPoint(event->scenePos(), 1);
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
                                  "<tr><td>comment:</td><td>%5</td></tr>"
                                  "<tr><td></td><td></td></tr>");
                }

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
                                                  .arg(tp.at(tp.size() - 1).y()));
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
    double       pSP  = (double) mControlWidget->trackCurrentPointSize->value();
    double       pS   = (double) mControlWidget->trackPointSize->value();
    double       pSC  = (double) mControlWidget->trackColColorSize->value();
    double       pSM  = (double) mControlWidget->trackColorMarkerSize->value();
    double       pSN  = (double) mControlWidget->trackNumberSize->value();
    double       pSG  = (double) mControlWidget->trackGroundPositionSize->value();
    double       pSGP = (double) mControlWidget->trackGroundPathSize->value();

    QColor pGPC = mControlWidget->getTrackGroundPathColor();
    QColor pTPC = mControlWidget->getTrackPathColor();
    QFont  font, heightFont;
    float  x_offset = 0, y_offset = 0;
    float  y_switch = 0, x_switch = 0;
    double hS;

    painter->drawRect(boundingRect());

    linePen.setColor(pTPC);
    linePen.setWidth(mControlWidget->trackPathWidth->value());

    ellipsePen.setWidth(3);

    if(mControlWidget->trackNumberBold->checkState() == Qt::Checked)
        font.setBold(true);
    else
        font.setBold(false);
    font.setPixelSize(mControlWidget->trackNumberSize->value());
    heightFont.setPixelSize(mControlWidget->trackColColorSize->value());
    painter->setFont(font);
    numberPen.setColor(Qt::red);
    groundPositionPen.setColor(Qt::green);
    groundPositionPen.setWidth(pSG);
    groundPathPen.setColor(pGPC);
    groundPathPen.setWidth(pSGP);


    if(mControlWidget->showVoronoiCells->isChecked() && !mTracker->isEmpty())
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
        debout << "x_offset: " << x_offset << ", y_offset: " << y_offset << ", x_switch: " << x_switch
               << ", y_switch: " << y_switch << std::endl;

        cv::Rect delaunyROI(cv::Rect(
            leftTop.x + x_offset,
            leftTop.y + y_offset,
            x_switch > 0 ? x_switch : (rightBottom.x - leftTop.x),
            y_switch > 0 ? y_switch : (rightBottom.y - leftTop.y)));
        debout << "Rect size: P(" << delaunyROI.x << ", " << delaunyROI.y << "), width: " << delaunyROI.width
               << ", height: " << delaunyROI.height << std::endl;

        subdiv.initDelaunay(delaunyROI);
    }

    auto pedestrianToPaint = mMainWindow->getPedestrianUserSelection();
    for(int i = 0; i < mTracker->size(); ++i) // ueber TrackPerson
    {
        // show current frame
        if(pedestrianToPaint.contains(i) || pedestrianToPaint.empty())
        {
            if(mTracker->at(i).trackPointExist(curFrame))
            {
                if(mControlWidget->trackHeadSized->checkState() == Qt::Checked)
                    pSP = mMainWindow->getHeadSize(nullptr, i, curFrame); // headSize;
                const TrackPoint &tp = (*mTracker)[i][curFrame - mTracker->at(i).firstFrame()];
                if(mControlWidget->trackShowCurrentPoint->checkState() ==
                   Qt::Checked) //(mControlWidget->recoShowColor->checkState() == Qt::Checked)
                {
                    painter->setBrush(Qt::NoBrush);
                    if(mTracker->at(i).newReco())
                        painter->setPen(Qt::green);
                    else
                        painter->setPen(Qt::blue);
                    rect.setRect(tp.x() - pSP / 2., tp.y() - pSP / 2., pSP, pSP);
                    painter->drawEllipse(rect); // direkt waere nur int erlaubt tp.x()-5., tp.y()-5., 10., 10.
                }

                if(mControlWidget->trackShowSearchSize->checkState() == Qt::Checked)
                {
                    painter->setBrush(Qt::NoBrush);
                    painter->setPen(Qt::yellow);
                    hS = mMainWindow->winSize(nullptr, i, curFrame);
                    if(hS < 2)
                        hS = 2; // entspricht Vorgehen in tracker.cpp
                    for(int j = 0; j <= mControlWidget->trackRegionLevels->value(); ++j)
                    {
                        rect.setRect(tp.x() - hS / 2., tp.y() - hS / 2., hS, hS);
                        painter->drawRect(rect);
                        hS *= 2;
                    }
                }

                if(mControlWidget->trackShowColorMarker->checkState() == Qt::Checked)
                {
                    // farbe des trackpoints
                    if(tp.color().isValid())
                    {
                        painter->setBrush(Qt::NoBrush);
                        ellipsePen.setColor(tp.color());
                        painter->setPen(ellipsePen);
                        rect.setRect(tp.colPoint().x() - pSM / 2., tp.colPoint().y() - pSM / 2., pSM, pSM);
                        painter->drawEllipse(rect);
                    }
                }

                // berechnung der normalen, die zur positionierung der nummerieung und der gesamtfarbe dient
                if(((mControlWidget->trackShowColColor->checkState() == Qt::Checked) &&
                    (mTracker->at(i).color().isValid())) ||
                   (mControlWidget->trackShowNumber->checkState() == Qt::Checked) ||
                   ((mControlWidget->trackShowColColor->checkState() == Qt::Checked) &&
                    ((mTracker->at(i).height() > MIN_HEIGHT) ||
                     ((tp.sp().z() > 0.) && (mControlWidget->trackShowHeightIndividual->checkState() ==
                                             Qt::Checked))))) //  Hoehe kann auf Treppen auch negativ werden, wenn koord
                                                              //  weiter oben angesetzt wird
                {
                    if(tp.color().isValid())
                    {
                        normalVector = (tp - tp.colPoint()).normal();
                        normalVector.normalize();
                        if(normalVector.length() < .001) // wenn to und colpoint aufeinander liegen z bei colorMarker!
                            normalVector.set(1., 0.);
                    }
                    else
                    {
                        // man koennte auch lastNormal von anderem trackpath nehmen statt 1, 0
                        normalVector.set(1., 0.);
                        // den vorherigen trackpoint finden, wo reco farbe erzeugt hat und somit colpoint vorliegt
                        for(int j = curFrame - mTracker->at(i).firstFrame(); j > -1; --j)
                            if(mTracker->at(i).at(j).color().isValid())
                            {
                                normalVector = (mTracker->at(i).at(j) - mTracker->at(i).at(j).colPoint()).normal();
                                normalVector.normalize();
                                break;
                            }
                        // einen nachfolgenden trackpoint suchen, wenn vorher keiner mit farbe war
                        // zB wenn rueckwaerts abgespielt wird
                        if((normalVector.x() == 1.) && (normalVector.y() == 0.))
                        {
                            for(int j = curFrame - mTracker->at(i).firstFrame() + 1; j < mTracker->at(i).size(); ++j)
                                if(mTracker->at(i).at(j).color().isValid())
                                {
                                    normalVector = (mTracker->at(i).at(j) - mTracker->at(i).at(j).colPoint()).normal();
                                    normalVector.normalize();
                                    break;
                                }
                        }
                    }
                }

                // farbe der gesamten trackperson
                double height = mTracker->at(i).height();
                if(mControlWidget->trackShowColColor->checkState() == Qt::Checked)
                {
                    painter->setPen(numberPen);
                    painter->setBrush(Qt::NoBrush);
                    rect.setRect(tp.x() + 10, tp.y() + 10, 15 * pSC, 10 * pSC);
                    painter->drawText(rect, mTracker->at(i).comment());
                    rect.setRect(tp.x() - pSC, tp.y() - pSC, 50, 50);
                    if(tp.getMarkerID() > 0)
                    {
                        painter->drawText(rect, QString("id=%1").arg(tp.getMarkerID()));
                    }
                }

                if((mControlWidget->trackShowColColor->checkState() == Qt::Checked) &&
                   (mTracker->at(i).color().isValid()))
                {
                    painter->setPen(Qt::NoPen);
                    painter->setBrush(QBrush(mTracker->at(i).color()));
                    rect.setRect(
                        tp.x() + (pSP + pSC) * 0.6 * normalVector.x() - pSC / 2.,
                        tp.y() + (pSP + pSC) * 0.6 * normalVector.y() - pSC / 2.,
                        pSC,
                        pSC); // 11
                    painter->drawEllipse(rect);
                }
                else if(
                    (mControlWidget->trackShowColColor->checkState() == Qt::Checked) &&
                    ((height > MIN_HEIGHT) ||
                     ((tp.sp().z() > 0.) &&
                      (mControlWidget->trackShowHeightIndividual->checkState() ==
                       Qt::Checked)))) // Hoehe  && (mTracker->at(i).height() > 0.) Hoehe kann auf Treppen auch negativ
                                       // werden, wenn koord weiter oben angesetzt wird
                {
                    painter->setFont(heightFont);
                    if((mControlWidget->trackShowHeightIndividual->checkState() == Qt::Checked) &&
                       (tp.sp().z() > 0.)) // Hoehe incl individual fuer jeden trackpoint
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
                                painter->drawText(
                                    rect,
                                    Qt::AlignHCenter,
                                    QString("-\n%2").arg(
                                        -mControlWidget->getCalibExtrTrans3() - tp.sp().z(), 6, 'f', 1));
                            else
                                painter->drawText(
                                    rect,
                                    Qt::AlignHCenter,
                                    QString("-\n%2").arg(
                                        mControlWidget->coordAltitude->value() - tp.sp().z(), 6, 'f', 1));
                        }
                        else
                        {
                            if(mControlWidget->getCalibCoordDimension() == 0) // 3D
                                painter->drawText(
                                    rect,
                                    Qt::AlignHCenter,
                                    QString("%1\n%2")
                                        .arg(height, 6, 'f', 1)
                                        .arg(-mControlWidget->getCalibExtrTrans3() - tp.sp().z(), 6, 'f', 1));
                            else
                                painter->drawText(
                                    rect,
                                    Qt::AlignHCenter,
                                    QString("%1\n%2")
                                        .arg(height, 6, 'f', 1)
                                        .arg(mControlWidget->coordAltitude->value() - tp.sp().z(), 6, 'f', 1));
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
                if(mControlWidget->trackShowNumber->checkState() == Qt::Checked)
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
                if(mControlWidget->trackShowGroundPosition->checkState() == Qt::Checked)
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
                                cv::Point2f(tp.x(), tp.y()),
                                mControlWidget->getColorPlot()->map(mTracker->at(i).color()));
                        }
                        else
                        {
                            if(tp.sp().z() > 0)
                                p3d_height = mMainWindow->getExtrCalibration()->get3DPoint(
                                    cv::Point2f(tp.x(), tp.y()), -mControlWidget->getCalibExtrTrans3() - tp.sp().z());
                            else
                                p3d_height = mMainWindow->getExtrCalibration()->get3DPoint(
                                    cv::Point2f(tp.x(), tp.y()), height /*mControlWidget->mapDefaultHeight->value()*/);
                        }
                        p3d_height.z           = 0;
                        cv::Point2f p2d_ground = mMainWindow->getExtrCalibration()->getImagePoint(p3d_height);
                        QPointF     axis = mMainWindow->getImageItem()->getCmPerPixel(p2d_ground.x, p2d_ground.y, 0);
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
                if(mControlWidget->showVoronoiCells->checkState() == Qt::Checked)
                {
                    if(mControlWidget->getCalibCoordDimension() == 0) // 3D
                    {
                        cv::Point3f p3d_height;
                        if(height < MIN_HEIGHT + 1)
                        {
                            p3d_height = mMainWindow->getExtrCalibration()->get3DPoint(
                                cv::Point2f(tp.x(), tp.y()),
                                mControlWidget->getColorPlot()->map(mTracker->at(i).color()));
                        }
                        else
                        {
                            if(tp.sp().z() > 0)
                                p3d_height = mMainWindow->getExtrCalibration()->get3DPoint(
                                    cv::Point2f(tp.x(), tp.y()), -mControlWidget->getCalibExtrTrans3() - tp.sp().z());
                            else
                                p3d_height = mMainWindow->getExtrCalibration()->get3DPoint(
                                    cv::Point2f(tp.x(), tp.y()), height /*mControlWidget->mapDefaultHeight->value()*/);
                        }

                        debout << "insert P(" << p3d_height.x + x_offset << ", " << p3d_height.y + y_offset
                               << ") to subdiv" << std::endl;

                        subdiv.insert(cv::Point2f(
                            x_switch > 0 ? x_switch - p3d_height.x + x_offset : p3d_height.x + x_offset,
                            y_switch > 0 ? y_switch - p3d_height.y + y_offset :
                                           p3d_height.y + y_offset)); // p2d_ground);
                    }
                }
            }

            if(((mControlWidget->trackShowPoints->checkState() == Qt::Checked) ||
                (mControlWidget->trackShowPath->checkState() == Qt::Checked) ||
                (mControlWidget->trackShowGroundPath->checkState() == Qt::Checked)) &&
               ((mTracker->at(i).trackPointExist(curFrame)) ||
                (mControlWidget->trackShowOnlyVisible->checkState() == Qt::Unchecked)))
            {
                if(mControlWidget->trackShowBefore->value() == -1)
                    from = 0;
                else
                {
                    from = curFrame - mTracker->at(i).firstFrame() - mControlWidget->trackShowBefore->value();
                    if(from < 0)
                        from = 0;
                }
                if(mControlWidget->trackShowAfter->value() == -1)
                    to = mTracker->at(i).size();
                else
                {
                    to = curFrame - mTracker->at(i).firstFrame() + mControlWidget->trackShowAfter->value() + 1;
                    if(to > mTracker->at(i).size())
                        to = mTracker->at(i).size();
                }
                for(int j = from; j < to; ++j) // ueber TrackPoint
                {
                    // path
                    if(mControlWidget->trackShowPath->checkState() == Qt::Checked)
                    {
                        if(j != from) // autom. > 0
                        {
                            painter->setPen(linePen);
                            painter->setBrush(Qt::NoBrush);

                            // nur Linie zeichnen, wenn x oder y sich unterscheidet, sonst Punkt
                            // die Unterscheidung ist noetig, da Qt sonst grosses quadrat beim ranzoomen zeichnet
                            if((mTracker->at(i).at(j - 1).toQPointF().x() != mTracker->at(i).at(j).toQPointF().x()) ||
                               (mTracker->at(i).at(j - 1).toQPointF().y() != mTracker->at(i).at(j).toQPointF().y()))
                                painter->drawLine(
                                    mTracker->at(i).at(j - 1).toQPointF(), mTracker->at(i).at(j).toQPointF());
                            else
                                painter->drawPoint(mTracker->at(i).at(j - 1).toQPointF());
                        }
                    }
                    // path on ground
                    if(mControlWidget->trackShowGroundPath->checkState() == Qt::Checked)
                    {
                        if(j != from)
                        {
                            // ground position
                            painter->setPen(groundPathPen);
                            painter->setBrush(Qt::NoBrush);
                            if(mControlWidget->getCalibCoordDimension() == 0) // 3D
                            {
                                cv::Point3f p3d_height_p1, p3d_height_p2;
                                if(mTracker->at(i).height() < MIN_HEIGHT + 1)
                                {
                                    p3d_height_p1 = mMainWindow->getExtrCalibration()->get3DPoint(
                                        cv::Point2f(mTracker->at(i).at(j - 1).x(), mTracker->at(i).at(j - 1).y()),
                                        mControlWidget->getColorPlot()->map(mTracker->at(i).color()));
                                    p3d_height_p2 = mMainWindow->getExtrCalibration()->get3DPoint(
                                        cv::Point2f(mTracker->at(i).at(j).x(), mTracker->at(i).at(j).y()),
                                        mControlWidget->getColorPlot()->map(mTracker->at(i).color()));
                                }
                                else
                                {
                                    if(mTracker->at(i).at(j - 1).sp().z() > 0 && mTracker->at(i).at(j).sp().z() > 0)
                                    {
                                        p3d_height_p1 = mMainWindow->getExtrCalibration()->get3DPoint(
                                            cv::Point2f(mTracker->at(i).at(j - 1).x(), mTracker->at(i).at(j - 1).y()),
                                            -mControlWidget->getCalibExtrTrans3() - mTracker->at(i).at(j - 1).sp().z());
                                        p3d_height_p2 = mMainWindow->getExtrCalibration()->get3DPoint(
                                            cv::Point2f(mTracker->at(i).at(j).x(), mTracker->at(i).at(j).y()),
                                            -mControlWidget->getCalibExtrTrans3() - mTracker->at(i).at(j).sp().z());
                                    }
                                    else
                                    {
                                        p3d_height_p1 = mMainWindow->getExtrCalibration()->get3DPoint(
                                            cv::Point2f(mTracker->at(i).at(j - 1).x(), mTracker->at(i).at(j - 1).y()),
                                            mTracker->at(i).height());
                                        p3d_height_p2 = mMainWindow->getExtrCalibration()->get3DPoint(
                                            cv::Point2f(mTracker->at(i).at(j).x(), mTracker->at(i).at(j).y()),
                                            mTracker->at(i).height());
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
                                    painter->drawLine(
                                        QLineF(p2d_ground_p1.x, p2d_ground_p1.y, p2d_ground_p2.x, p2d_ground_p2.y));
                                else
                                    painter->drawPoint(p2d_ground_p1.x, p2d_ground_p1.y);
                            }
                            else // 2D
                            {
                            }
                        }
                    }

                    // points before and after
                    if(mControlWidget->trackShowPoints->checkState() == Qt::Checked)
                    {
                        if(mTracker->at(i).firstFrame() + j != curFrame)
                        {
                            if((mControlWidget->trackShowPointsColored->checkState() == Qt::Checked) &&
                               (mTracker->at(i).at(j).color().isValid()))
                            {
                                painter->setPen(Qt::NoPen);
                                painter->setBrush(QBrush(mTracker->at(i).at(j).color()));
                                rect.setRect(
                                    mTracker->at(i).at(j).x() - pS / 2.,
                                    mTracker->at(i).at(j).y() - pS / 2.,
                                    pS,
                                    pS); // 7
                            }
                            else
                            {
                                painter->setPen(Qt::red);
                                painter->setBrush(Qt::NoBrush);
                                // war noetig fuer alte qt-version: rect.setRect(mTracker->at(i).at(j).x()-(pS-1.)/2.,
                                // mTracker->at(i).at(j).y()-(pS-1.)/2., pS-1., pS-1.); // 6
                                rect.setRect(
                                    mTracker->at(i).at(j).x() - pS / 2., mTracker->at(i).at(j).y() - pS / 2., pS, pS);
                            }
                            painter->drawEllipse(rect);
                        }
                    }
                }
            }
        }
    }

    // Mat& img, Subdiv2D& subdiv )
    if(mControlWidget->showVoronoiCells->checkState() == Qt::Checked && !mTracker->isEmpty())
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

                debout << "facets3D.at(" << i << ").at(" << j << ").x = " << facets3D.at(i).at(j).x
                       << ", .y = " << facets3D.at(i).at(j).y << std::endl;
                debout << "point2D.x = " << point2D.x << " , .y = " << point2D.y << std::endl;

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

                        debout << "End point: (" << s1_x << ", " << s1_y << ")" << std::endl;
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

                        debout << "x=" << s1_x << " G(x)=" << (m * s1_x + n)
                               << " K(x)=" << pow(s1_x - centers3D.at(i).x, 2) + pow(s1_y - centers3D.at(i).y, 2)
                               << " = " << pow(r, 2) << std::endl;
                        debout << "x=" << s2_x << " G(x)=" << (m * s2_x + n)
                               << " K(x)=" << pow(s2_x - centers3D.at(i).x, 2) + pow(s2_y - centers3D.at(i).y, 2)
                               << " = " << pow(r, 2) << std::endl;

                        facets3D[i][j] = cv::Point2f(s1_x, s1_y);

                        point2D = mMainWindow->getExtrCalibration()->getImagePoint(cv::Point3f(s1_x, s1_y, 0));
                        ifacet2D.push_back(QPointF(point2D.x, point2D.y));
                        circleStart   = QPointF(point2D.x, point2D.y);
                        circleStarted = true;

                        debout << "Start point: (" << s1_x << ", " << s1_y << ")" << std::endl;
                    }
                }
                else
                {
                    ifacet2D.push_back(QPointF(point2D.x, point2D.y));
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
                painter->drawEllipse(QPointF(center2D.x, center2D.y), 100, 100);
            painter->drawConvexPolygon(QPolygonF(ifacet_vec));

            // voronoi cell point
            painter->setBrush(Qt::black);
            painter->setPen(Qt::red);
            painter->drawEllipse(QPointF(center2D.x, center2D.y), 5, 5);
        }
    }
}
