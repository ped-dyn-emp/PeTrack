/*
 * PeTrack - Software for tracking pedestrians movement in videos
 * Copyright (C) 2023 Forschungszentrum Jülich GmbH, IAS-7
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
#include "extrCalibration.h"
#include "petrack.h"

#include <QtWidgets>
#include <cmath>

// in x und y gleichermassen skaliertes koordinatensystem,
// da von einer vorherigen intrinsischen kamerakalibrierung ausgegenagen wird,
// so dass pixel quadratisch
CoordItem::CoordItem(QWidget *wParent, QGraphicsItem *parent) : QGraphicsItem(parent)
{
    mMainWindow    = (class Petrack *) wParent;
    extCalib       = mMainWindow->getExtrCalibration();
    mControlWidget = mMainWindow->getControlWidget();

    // Set Min and Max
    calibPointsMin.x = 50000;
    calibPointsMax.x = 0;
    calibPointsMin.y = 50000;
    calibPointsMax.y = 0;

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
    // bounding box wird in lokalen koordinaten angegeben!!! (+-10 wegen zahl "1")
    if(mControlWidget->getCalibCoordShow())
    {
        if(mControlWidget->getCalibCoordDimension() != 0) // 2D view
        {
            return QRectF(-110., -110., 220., 220.);
        }
        else // 3D view
        {
            double min_x = std::min(std::min(x.x, y.x), std::min(z.x, ursprung.x));
            double max_x = std::max(std::max(x.x, y.x), std::max(z.x, ursprung.x));

            double min_y = std::min(std::min(x.y, y.y), std::min(z.y, ursprung.y));
            double max_y = std::max(std::max(x.y, y.y), std::max(z.y, ursprung.y));

            if(mControlWidget->getCalibExtrCalibPointsShow())
            {
                min_x = std::min(float(min_x), calibPointsMin.x);
                max_x = std::max(float(max_x), calibPointsMax.x);

                min_y = std::min(float(min_y), calibPointsMin.y);
                max_y = std::max(float(max_y), calibPointsMax.y);
            }
            return QRectF(min_x - 25, min_y - 25, max_x - min_x + 50, max_y - min_y + 50);
        }
    }
    else
    {
        return QRectF(0., 0., 0., 0.);
    }
}

// event, of moving mouse while button is pressed
void CoordItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    // if coordinate system position is not fixed
    if(!mControlWidget->getCalibCoordFix())
    {
        setCursor(Qt::ClosedHandCursor);

        QPointF diff = event->scenePos() -
                       event->lastScenePos();   // screenPos()-buttonDownScreenPos(Qt::RightButton) also interesting
        if(event->buttons() == Qt::RightButton) // event->button() doesnt work
        {
            mControlWidget->setCalibCoordRotate(
                mControlWidget->getCalibCoordRotate() -
                (int) (3. * (diff.x() + diff.y()))); // 10* nicht noetig, da eh nur relativ
        }
        else if(event->buttons() == Qt::LeftButton)
        {
            if(mControlWidget->getCalibCoordDimension() == 0) // 3D
            {
                cv::Point3f p_cur = extCalib->get3DPoint(
                    cv::Point2f(event->scenePos().x(), event->scenePos().y()), mControlWidget->getCalibCoord3DTransZ());
                cv::Point3f p_last = extCalib->get3DPoint(
                    cv::Point2f(mouse_x /*event->lastScenePos().x()*/, mouse_y /*event->lastScenePos().y()*/),
                    mControlWidget->getCalibCoord3DTransZ());
                // ToDo:
                // Problem: Die Mouse Bewegungen, die erfasst werden sind zu gering, sodass viele Werte als 0 gewertet
                // werden und nicht als Bewegung bercksichtigt werden, obwohl man die Maus bewegt. D.h. die Maus bewegt
                // sich der Abstand zwischen dem Start und dem End-punkt ist aber sehr gering und wird als 0 gewertet
                // und das Kooridnatensystem bewegt sich nicht. Effekt wird noch verstrkt, da das stndig passiert
                // Besonders schnelle Mausbewegungen lindern den Effekt

                mControlWidget->setCalibCoord3DTransX(
                    coordTrans_x - (mControlWidget->getCalibCoord3DSwapX() ? -1 : 1) * round(p_last.x - p_cur.x));
                mControlWidget->setCalibCoord3DTransY(
                    coordTrans_y - (mControlWidget->getCalibCoord3DSwapY() ? -1 : 1) * round(p_last.y - p_cur.y));
            }
            else
            {
                mControlWidget->setCalibCoordTransX(mControlWidget->getCalibCoordTransX() + (int) (10. * diff.x()));
                mControlWidget->setCalibCoordTransY(mControlWidget->getCalibCoordTransY() + (int) (10. * diff.y()));
            }
        }
        else if(event->buttons() == Qt::MiddleButton)
        {
            if(mControlWidget->getCalibCoordDimension() == 0)
            {
                mControlWidget->setCalibCoord3DAxeLen(
                    mControlWidget->getCalibCoord3DAxeLen() + (int) (10. * (diff.x() - diff.y())));
            }
            else
            {
                mControlWidget->setCalibCoordScale(
                    mControlWidget->getCalibCoordScale() + (int) (10. * (diff.x() - diff.y())));
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
    if(!mControlWidget->getCalibCoordFix())
    {
        if(event->button() == Qt::LeftButton)
        {
            mouse_x = event->scenePos().x();
            mouse_y = event->scenePos().y();

            coordTrans_x = mControlWidget->getCalibCoord3DTransX();
            coordTrans_y = mControlWidget->getCalibCoord3DTransY();
        }
    }
    else
    {
        QGraphicsItem::mousePressEvent(event);
    }
}

void CoordItem::updateData()
{
    if(!mControlWidget->getCalibCoordFix())
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

    if(mControlWidget->getCalibCoordDimension() == 1) // 2D
    {
        double sc = mControlWidget->getCalibCoordScale() / 10.;
        double tX = mControlWidget->getCalibCoordTransX() / 10.;
        double tY = mControlWidget->getCalibCoordTransY() / 10.;
        double ro = mControlWidget->getCalibCoordRotate() / 10.;

        // aktualisierung der transformationsmatrix
        QTransform matrix;
        // matrix wird nur bei aenderungen neu bestimmt
        matrix.translate(tX, tY);
        matrix.rotate(ro);
        matrix.scale(sc / 100., sc / 100.);
        // matrix.shear(tX,tY);
        setTransform(matrix);
    }
    else // 3D
    {
        ////////////////////////////////////////
        //     3D World-Coordinate-System     //
        ////////////////////////////////////////
        if(mMainWindow->getImage())
        {
            // Reset Matrix - No Matrix Transformations for 3D Coordsystem
            // aktualisierung der transformationsmatrix
            QTransform matrix;
            // matrix wird nur bei aenderungen neu bestimmt
            matrix.translate(0, 0);
            matrix.rotate(0);
            matrix.scale(1, 1);
            setTransform(matrix);

            const double axeLen = mControlWidget->getCalibCoord3DAxeLen();
            const int    bS     = mMainWindow->getImageBorderSize();

            // Coordinate-system origin at (tX,tY,tZ)
            if(extCalib->isSetExtrCalib())
            {
                ursprung = extCalib->getImagePoint(cv::Point3f(0, 0, 0));

                x3D = cv::Point3f(axeLen, 0, 0);
                y3D = cv::Point3f(0, axeLen, 0);
                z3D = cv::Point3f(0, 0, axeLen);

                // Tests if the origin-point of the coordinate-system is outside the image
                if(extCalib->isOutsideImage(ursprung))
                {
                    return;
                }
                x3D.x++;
                y3D.y++;
                z3D.z++;

                // Kuerzt die Koordinaten-Achsen, falls sie aus dem angezeigten Bild raus laufen wuerden
                do
                {
                    x3D.x--;
                    x = extCalib->getImagePoint(x3D);
                    // tests if the coord system axis are inside the view or outside, if outside short them till they
                    // are inside the image
                } while(x.x < -bS || x.x > mMainWindow->getImage()->width() - bS || x.y < -bS ||
                        x.y > mMainWindow->getImage()->height() - bS);
                do
                {
                    y3D.y--;
                    y = extCalib->getImagePoint(y3D);
                } while(y.x < -bS || y.x > mMainWindow->getImage()->width() - bS || y.y < -bS ||
                        y.y > mMainWindow->getImage()->height() - bS);
                do
                {
                    z3D.z--;
                    z = extCalib->getImagePoint(z3D);
                } while(z.x < -bS || z.x > mMainWindow->getImage()->width() - bS || z.y < -bS ||
                        z.y > mMainWindow->getImage()->height() - bS);
            }
        }
        prepareGeometryChange();
    }
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
    if(mControlWidget->getCalibExtrCalibPointsShow() && mControlWidget->getCalibCoordDimension() == 0)
    {
        if(extCalib->isSetExtrCalib())
        {
            QFont font;
            font.setBold(mControlWidget->isTrackNumberBoldChecked());
            font.setPixelSize(mControlWidget->getTrackNumberSize());

            painter->setFont(font);

            for(size_t i = 0; i < extCalib->get2DList().size(); i++)
            {
                painter->setPen(Qt::red);
                painter->setBrush(Qt::NoBrush);
                // Original 2D-Pixel-Points
                cv::Point2f p2 = extCalib->get2DList().at(i);
                painter->drawEllipse(p2.x - 8, p2.y - 8, 16, 16);

                // general configuration
                painter->setPen(Qt::blue);
                painter->setBrush(Qt::blue);

                // Projected 3D-Points
                cv::Point3f p3d = extCalib->get3DList().at(i);
                p3d.x -= mControlWidget->getCalibCoord3DTransX();
                p3d.y -= mControlWidget->getCalibCoord3DTransY();
                p3d.z -= mControlWidget->getCalibCoord3DTransZ();

                p3d.x *= (mControlWidget->getCalibCoord3DSwapX() ? -1 : 1);
                p3d.y *= (mControlWidget->getCalibCoord3DSwapY() ? -1 : 1);
                p3d.z *= (mControlWidget->getCalibCoord3DSwapZ() ? -1 : 1);

                cv::Point2f p3 = extCalib->getImagePoint(p3d);

                painter->drawEllipse(p3.x - 4, p3.y - 4, 8, 8);

                // Connecting-line Pixel-3D-Points
                painter->drawLine(QPointF(p2.x, p2.y), QPointF(p3.x, p3.y));

                // Show point number
                painter->setPen(Qt::black);
                painter->setBrush(Qt::black);
                painter->drawText(QPointF(p2.x + 10, p2.y + font.pixelSize()), QObject::tr("%1").arg((i + 1)));

                calibPointsMin.x = std::min({calibPointsMin.x, p2.x, p3.x});
                calibPointsMin.y = std::min({calibPointsMin.y, p2.y, p3.y});
                calibPointsMax.x = std::max({calibPointsMax.x, p2.x, p3.x});
                calibPointsMax.y = std::max({calibPointsMax.y, p2.y, p3.y});
            }
        }
    }

    if(mControlWidget->getCalibCoordShow())
    {
        // general configuration
        painter->setPen(Qt::blue);
        painter->setBrush(QBrush(Qt::blue, Qt::SolidPattern));

        if(mControlWidget->getCalibCoordDimension() == 1) // 2D
        {
            //////////////////////////
            // 2D Coordinate-System //
            //////////////////////////

            painter->setPen(QPen(QBrush(Qt::blue), 0));

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
            double axeLen = mControlWidget->getCalibCoord3DAxeLen();

            qreal coordLineWidth = 2.0;

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

                painter->setPen(QPen(QBrush(Qt::blue), coordLineWidth));
                painter->drawLine(getImgPoint({0, 0, 0}), getImgPoint(x3D));
                painter->drawLine(getImgPoint({0, 0, 0}), getImgPoint(y3D));
                painter->drawLine(getImgPoint({0, 0, 0}), getImgPoint(z3D));

                /////////////////////////////////
                // Drawing the X,Y,Z - Symbols //
                /////////////////////////////////

                painter->setPen(QPen(QBrush(Qt::black), coordLineWidth));
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
                painter->setPen(QPen(QBrush(Qt::blue), coordLineWidth));

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
                painter->setPen(QPen(QBrush(Qt::blue), coordLineWidth));
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
