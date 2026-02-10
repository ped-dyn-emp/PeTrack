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

#include "coordItem.h"

#include "control.h"
#include "coordinateSystemBox.h"
#include "extrCalibration.h"
#include "penUtils.h"
#include "petrack.h"

#include <QtWidgets>
#include <cmath>

// in x und y gleichermassen skaliertes koordinatensystem,
// da von einer vorherigen intrinsischen kamerakalibrierung ausgegenagen wird,
// so dass pixel quadratisch
CoordItem::CoordItem(QWidget *wParent, QGraphicsItem *parent, CoordinateSystemBox *coordSys) :
    QGraphicsObject(parent), mCoordSys(coordSys)
{
    mMainWindow    = (class Petrack *) wParent;
    extCalib       = mMainWindow->getExtrCalibration();
    mControlWidget = mMainWindow->getControlWidget();

    QObject::connect(coordSys, &CoordinateSystemBox::coordDataChanged, this, &CoordItem::updateData);

    updateData(); // um zB setFlags(ItemIsMovable) je nach anzeige zu aendern
}

/**
 * @brief Bounding box of drawn to area.
 *
 * This bounding box is used to determine if this Item needs to be redrawn or not.
 * See the official Qt Docs for QGraphicsItem
 *
 * @return (updated) bounding rect of this item
 */
QRectF CoordItem::boundingRect() const
{
    if(mMainWindow->getImage() != nullptr)
    {
        return QRectF(
            -mMainWindow->getImageBorderSize(),
            -mMainWindow->getImageBorderSize(),
            mMainWindow->getImage()->width(),
            mMainWindow->getImage()->height());
    }
    else
    {
        return QRectF{0, 0, 0, 0};
    }
}

// event, of moving mouse while button is pressed
void CoordItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    // if coordinate system position is not fixed
    if(!mCoordSys->getCalibCoordFix())
    {
        setCursor(Qt::ClosedHandCursor);

        QPointF diff = event->scenePos() -
                       event->lastScenePos();   // screenPos()-buttonDownScreenPos(Qt::RightButton) also interesting
        if(event->buttons() == Qt::RightButton) // event->button() doesnt work
        {
            auto pose2D = mCoordSys->getCoordPose2D();
            pose2D.angle -= (int) (3. * (diff.x() + diff.y()));
            mCoordSys->setCoordPose2D(pose2D);
        }
        else if(event->buttons() == Qt::LeftButton)
        {
            if(mCoordSys->getCalibCoordDimension() == 0) // 3D
            {
                auto        trans = mCoordSys->getCoordTrans3D();
                cv::Point3f p_cur =
                    extCalib->get3DPoint(cv::Point2f(event->scenePos().x(), event->scenePos().y()), trans.z());
                cv::Point3f p_last = extCalib->get3DPoint(cv::Point2f(mouse_x, mouse_y), trans.z());
                // ToDo:
                // Problem: Die Mouse Bewegungen, die erfasst werden sind zu gering, sodass viele Werte als 0 gewertet
                // werden und nicht als Bewegung bercksichtigt werden, obwohl man die Maus bewegt. D.h. die Maus bewegt
                // sich der Abstand zwischen dem Start und dem End-punkt ist aber sehr gering und wird als 0 gewertet
                // und das Kooridnatensystem bewegt sich nicht. Effekt wird noch verstrkt, da das stndig passiert
                // Besonders schnelle Mausbewegungen lindern den Effekt

                auto swap = mCoordSys->getSwap3D();
                trans[0]  = coordTrans_x - (swap.x ? -1 : 1) * round(p_last.x - p_cur.x);
                trans[1]  = coordTrans_y - (swap.y ? -1 : 1) * round(p_last.y - p_cur.y);
                mCoordSys->setCoordTrans3D(trans);
            }
            else
            {
                auto pose = mCoordSys->getCoordPose2D();
                pose.position[0] += 10. * diff.x();
                pose.position[1] += 10. * diff.y();
                mCoordSys->setCoordPose2D(pose);
            }
        }
        else if(event->buttons() == Qt::MiddleButton)
        {
            if(mCoordSys->getCalibCoordDimension() == 0)
            {
                int axeLen = mCoordSys->getCoord3DAxeLen();
                axeLen += (int) (10. * (diff.x() - diff.y()));
                mCoordSys->setCoord3DAxeLen(axeLen);
            }
            else
            {
                auto pose2D = mCoordSys->getCoordPose2D();
                pose2D.scale += (int) (10. * (diff.x() - diff.y()));
                mCoordSys->setCoordPose2D(pose2D);
            }
        }
    }
    else
    {
        QGraphicsItem::mouseMoveEvent(event); // drag mach ich selber
    }
}

void CoordItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if(!mCoordSys->getCalibCoordFix())
    {
        if(event->button() == Qt::LeftButton)
        {
            mouse_x = event->scenePos().x();
            mouse_y = event->scenePos().y();

            auto trans   = mCoordSys->getCoordTrans3D();
            coordTrans_x = trans.x();
            coordTrans_y = trans.y();
        }
    }
    else
    {
        QGraphicsItem::mousePressEvent(event);
    }
}

void CoordItem::updateData()
{
    auto state = mCoordSys->getCoordItemState();
    if(state.isMovable)
    {
        setFlag(
            ItemIsMovable); // noetig, damit mouseEvent leftmousebutton weitergegeben wird, aber drag mach ich selber
    }
    else
    {
        setFlag(
            ItemIsMovable,
            false); // noetig, damit mouseEvent leftmousebutton weitergegeben wird, aber drag mach ich selber
    }

    setTransform(state.matrix);
    x3D                 = state.x3D;
    y3D                 = state.y3D;
    z3D                 = state.z3D;
    mCoordLineThickness = state.lineWidth;

    if(!mMainWindow->isLoading())
    {
        mMainWindow->updateImage();
    }
}

void CoordItem::paint(QPainter *painter, const QStyleOptionGraphicsItem * /*option*/, QWidget * /*widget*/)
{
    ////////////////////////////////
    // Drawing Calibration Points //
    ////////////////////////////////
    if(mCoordSys->getCalibExtrCalibPointsShow() && mCoordSys->getCalibCoordDimension() == 0)
    {
        if(extCalib->isSetExtrCalib())
        {
            QFont font;
            font.setBold(mControlWidget->isTrackNumberBoldChecked());
            font.setPixelSize(mControlWidget->getTrackNumberSize());
            painter->setFont(font);

            for(size_t i = 0; i < extCalib->get2DList().size(); i++)
            {
                QPen textPen = scaledPen(Qt::red, mCoordLineThickness);
                painter->setPen(textPen);
                painter->setBrush(Qt::NoBrush);
                // Original 2D-Pixel-Points
                cv::Point2f p2 = extCalib->get2DList().at(i);
                painter->drawEllipse(p2.x - 8, p2.y - 8, 16, 16);

                // Show point number
                painter->setPen(scaledPen(Qt::black, mCoordLineThickness));
                painter->setBrush(Qt::black);
                painter->drawText(QPointF(p2.x + 10, p2.y + font.pixelSize()), QObject::tr("%1").arg((i + 1)));

                if(i < extCalib->get3DList().size())
                {
                    // general configuration
                    painter->setPen(scaledPen(Qt::blue, mCoordLineThickness));
                    painter->setBrush(Qt::blue);
                    // Projected 3D-Points
                    cv::Point3f p3d   = extCalib->get3DList().at(i);
                    auto        trans = mCoordSys->getCoordTrans3D();
                    p3d -= trans.toCvPoint();

                    auto swap = mCoordSys->getSwap3D();
                    p3d.x *= (swap.x ? -1 : 1);
                    p3d.y *= (swap.y ? -1 : 1);
                    p3d.z *= (swap.z ? -1 : 1);

                    cv::Point2f p3 = extCalib->getImagePoint(p3d);

                    painter->drawEllipse(p3.x - 4, p3.y - 4, 8, 8);

                    // Connecting-line Pixel-3D-Points
                    painter->drawLine(QPointF(p2.x, p2.y), QPointF(p3.x, p3.y));
                }
            }
        }
    }

    if(mCoordSys->getCalibCoordShow())
    {
        // general configuration
        painter->setPen(scaledPen(Qt::blue, mCoordLineThickness));
        painter->setBrush(QBrush(Qt::blue, Qt::SolidPattern));

        if(mCoordSys->getCalibCoordDimension() == 1) // 2D
        {
            //////////////////////////
            // 2D Coordinate-System //
            //////////////////////////

            painter->setPen(scaledPen(Qt::blue, mCoordLineThickness));

            // Koordinatenachsen
            painter->drawLine(QPoint{-10, 0}, QPoint{100, 0});
            painter->drawLine(QPoint{0, 10}, QPoint{0, -100});

            // Ticks
            for(int i = 1; i < 11; i++) // i=10 zeichnen sieht ungewoehnlich aus, laeest sich aber besser mit messen
            {
                painter->drawLine(QPoint{2, -i * 10}, QPoint{-2, -i * 10});
                painter->drawLine(QPoint{i * 10, 2}, QPoint{i * 10, -2});
            }

            // Beschriftung
            painter->drawText(QPoint{97, 12}, QObject::tr("1"));
            painter->drawText(QPoint{-8, -97}, QObject::tr("1"));

            // Pfeilspitzen
            painter->setPen(Qt::NoPen);
            painter->setBrush(Qt::blue);

            QPolygon arrowTipX;
            arrowTipX << QPoint(100, 0) << QPoint(95, 2) << QPoint(95, -2);
            painter->drawPolygon(arrowTipX);

            QPolygon arrowTipY;
            arrowTipY << QPoint(0, -100) << QPoint(2, -95) << QPoint(-2, -95);
            painter->drawPolygon(arrowTipY);
        }
        else
        {
            double axeLen = mCoordSys->getCoord3DAxeLen();


            if(extCalib->isSetExtrCalib())
            {
                auto getImgPoint = [this](const cv::Point3f &realPoint)
                {
                    const auto imgPoint = extCalib->getImagePoint(realPoint);
                    return QPointF(imgPoint.x, imgPoint.y);
                };


                //////////////////////////////
                // Drawing the X,Y,Z - Axis //
                //////////////////////////////

                painter->setPen(scaledPen(Qt::blue, mCoordLineThickness));
                painter->drawLine(getImgPoint({0, 0, 0}), getImgPoint(x3D));
                painter->drawLine(getImgPoint({0, 0, 0}), getImgPoint(y3D));
                painter->drawLine(getImgPoint({0, 0, 0}), getImgPoint(z3D));

                /////////////////////////////////
                // Drawing the X,Y,Z - Symbols //
                /////////////////////////////////

                painter->setPen(scaledPen(Qt::black, mCoordLineThickness));
                painter->setFont(QFont("Arial", 15));

                const QPointF labelOffset{-5, 5};
                const QPointF xLabelPos = getImgPoint(x3D + cv::Point3f(10, 0, 0)) + labelOffset;
                const QPointF yLabelPos = getImgPoint(y3D + cv::Point3f(0, 10, 0)) + labelOffset;
                const QPointF zLabelPos = getImgPoint(z3D + cv::Point3f(0, 0, 10)) + labelOffset;
                painter->drawText(xLabelPos, QString("X"));
                painter->drawText(yLabelPos, QString("Y"));
                painter->drawText(zLabelPos, QString("Z"));

                //////////////////////////////
                // Drawing the tick-markers //
                //////////////////////////////

                int tickLength = AXIS_MARKERS_LENGTH;
                painter->setPen(scaledPen(Qt::blue, mCoordLineThickness));

                // Start bei 100cm bis Achsen-Laenge-Pfeilspitzenlaenge alle 100cm
                for(int i = 100; i < axeLen - tickLength; i += 100)
                {
                    // Solange Achsen-Ende noch nicht erreicht: Markierung zeichnen
                    if(i + tickLength < x3D.x)
                    {
                        painter->drawLine(
                            getImgPoint(cv::Point3f(i, -tickLength, 0)), getImgPoint(cv::Point3f(i, tickLength, 0)));
                        painter->drawLine(
                            getImgPoint(cv::Point3f(i, 0, -tickLength)), getImgPoint(cv::Point3f(i, 0, tickLength)));
                    }
                    if(i + tickLength < y3D.y)
                    {
                        painter->drawLine(
                            getImgPoint(cv::Point3f(-tickLength, i, 0)), getImgPoint(cv::Point3f(tickLength, i, 0)));
                        painter->drawLine(
                            getImgPoint(cv::Point3f(0, i, -tickLength)), getImgPoint(cv::Point3f(0, i, tickLength)));
                    }
                    if(i + tickLength < z3D.z)
                    {
                        painter->drawLine(
                            getImgPoint(cv::Point3f(-tickLength, 0, i)), getImgPoint(cv::Point3f(tickLength, 0, i)));
                        painter->drawLine(
                            getImgPoint(cv::Point3f(0, -tickLength, i)), getImgPoint(cv::Point3f(0, tickLength, i)));
                    }
                }

                QFont font("Arial", tickLength * 0.5);
                painter->setFont(font);

                //////////////////////////////////////////////
                // Drawing the peaks at the end of the axis //
                //////////////////////////////////////////////

                constexpr int peakSize = AXIS_MARKERS_LENGTH;
                painter->setPen(scaledPen(Qt::blue, mCoordLineThickness));
                painter->setBrush(Qt::NoBrush); // don't fill polygons

                ///////
                // X //
                ///////

                /* clang-format off */
                QPolygonF arrowTipX1;
                arrowTipX1 << getImgPoint({x3D.x, 0, 0})
                           << getImgPoint({x3D.x - peakSize, -peakSize, 0})
                           << getImgPoint({x3D.x - peakSize, peakSize, 0});
                QPolygonF arrowTipX2;
                arrowTipX2 << getImgPoint({x3D.x, 0, 0})
                           << getImgPoint({x3D.x - peakSize, 0, -peakSize})
                           << getImgPoint({x3D.x - peakSize, 0, peakSize});
                /* clang-format on */
                painter->drawPolygon(arrowTipX1);
                painter->drawPolygon(arrowTipX2);

                ///////
                // Y //
                ///////

                /* clang-format off */
                QPolygonF arrowTipY1;
                arrowTipY1 << getImgPoint({0, y3D.y, 0})
                           << getImgPoint({-peakSize, y3D.y - peakSize, 0})
                           << getImgPoint({peakSize, y3D.y - peakSize, 0});
                QPolygonF arrowTipY2;
                arrowTipY2 << getImgPoint({0, y3D.y, 0})
                           << getImgPoint({0, y3D.y - peakSize, -peakSize})
                           << getImgPoint({0, y3D.y - peakSize, peakSize});
                /* clang-format on */
                painter->drawPolygon(arrowTipY1);
                painter->drawPolygon(arrowTipY2);

                ///////
                // Z //
                ///////

                /* clang-format off */
                QPolygonF arrowTipZ1;
                arrowTipZ1 << getImgPoint({0, 0, z3D.z})
                           << getImgPoint({0, -peakSize, z3D.z - peakSize})
                           << getImgPoint({0, peakSize, z3D.z - peakSize});
                QPolygonF arrowTipZ2;
                arrowTipZ2 << getImgPoint({0, 0, z3D.z})
                           << getImgPoint({-peakSize, 0, z3D.z - peakSize})
                           << getImgPoint({peakSize, 0, z3D.z - peakSize});
                /* clang-format on */
                painter->drawPolygon(arrowTipZ1);
                painter->drawPolygon(arrowTipZ2);
            }
        }
    }
}
