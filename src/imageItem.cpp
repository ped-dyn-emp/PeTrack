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

#include "imageItem.h"

#include "control.h"
#include "petrack.h"

#include <QGraphicsScene>
#include <QGraphicsSceneHoverEvent>
#include <QPainter>
#include <cmath>

ImageItem::ImageItem(QWidget *wParent, QGraphicsItem *parent) : QGraphicsItem(parent)
{
    mMainWindow    = (class Petrack *) wParent;
    mControlWidget = mMainWindow->getControlWidget();
    mImage         = nullptr;
    mCoordItem     = nullptr;
    setCursor(Qt::CrossCursor);
    setAcceptHoverEvents(true);
}

/**
 * @brief Bounding box of drawn to area.
 *
 * This bounding box is used to determine if this Item needs to be redrawn or not.
 * See the official Qt Docs for QGraphicsItem
 *
 * @return (updated) bounding rect of this item
 */
QRectF ImageItem::boundingRect() const
{
    if(mImage)
    {
        return QRectF(0, 0, mImage->width(), mImage->height());
    }
    else
    {
        return QRectF(0, 0, 0, 0);
    }
}

void ImageItem::paint(QPainter *painter, const QStyleOptionGraphicsItem * /*option*/, QWidget * /*widget*/)
{
    if(mImage)
    {
        painter->drawImage(0, 0, *mImage);
    }
}

void ImageItem::setImage(QImage *img)
{
    if(img->isNull())
    {
        mImage = nullptr;
        setTransform(QTransform());
        mMainWindow->updateSceneRect();
        return;
    }
    mImage = img;


    QTransform matrix;
    matrix.translate(
        1, 1); // FEHLER IN QT ????? noetig, damit trotz recognitionroiitem auch image auch ohne border komplett neu
               // gezeichnet wird // wird 2 zeilen weiter zurueckgesetzt, aber mit 0, 0 geht es nicht
    setTransform(matrix);
    matrix.translate(-mMainWindow->getImageBorderSize() - 1, -mMainWindow->getImageBorderSize() - 1);
    setTransform(matrix);

    mControlWidget->imageSizeChanged(mImage->width(), mImage->height(), 0);

    // trans nicht setzen, da mgl mehrere videos mit gleicher scene und gleichem koord sinnvoll
    mControlWidget->setCalibCoordTransXMin(-10 * mMainWindow->getImageBorderSize());
    mControlWidget->setCalibCoordTransYMin(-10 * mMainWindow->getImageBorderSize());
    mControlWidget->setCalibCoordTransXMax(10 * (mImage->width() - mMainWindow->getImageBorderSize()));
    mControlWidget->setCalibCoordTransYMax(10 * (mImage->height() - mMainWindow->getImageBorderSize()));

    mMainWindow->updateSceneRect();
}

void ImageItem::setCoordItem(QGraphicsItem *ci)
{
    mCoordItem = ci;
}
// in x und y richtung identisch, da vorher intrinsische kamerakalibrierung geschehen ist
double ImageItem::getCmPerPixel()
{
    if(mCoordItem)
    {
        // das sollte nur einmal berechne werden, wenn einfliessende daten sich aendern
        QPointF p1 = mapToItem(mCoordItem, QPointF(0., 0.));
        QPointF p2 = mapToItem(mCoordItem, QPointF(1., 0.));
        return mControlWidget->getCalibCoordUnit() * sqrt(pow(p1.x() - p2.x(), 2) + pow(p1.y() - p2.y(), 2)) / 100.;
        // durch 100., da coordsys so gezeichnet, dass 1 bei 100 liegt
    }
    else
    {
        return 0.;
    }
}
// Liefert zum Pixelpunkt (px,py) die Anzahl der Zentimeter in x- und y-Richtung
QPointF ImageItem::getCmPerPixel(float px, float py, float h)
{
    // ToDo:
    // 3D Punkte an (px-0.5, py) und (px+0.5, py) berechnen und Auswirkung in x-Richtung
    // und          (px, py-0.5) und (px, py+0.5) berechnen und Auswirkung in y-Richtung untersuchen
    //
    // Unterscheiden nach x- und y-Richtung?
    // Wie fliesst die Hoehe mit ein?

    cv::Point3f p3x1 = mMainWindow->getExtrCalibration()->get3DPoint(cv::Point2f(px - 0.5, py), h);
    cv::Point3f p3x2 = mMainWindow->getExtrCalibration()->get3DPoint(cv::Point2f(px + 0.5, py), h);

    cv::Point3f p3y1 = mMainWindow->getExtrCalibration()->get3DPoint(cv::Point2f(px, py - 0.5), h);
    cv::Point3f p3y2 = mMainWindow->getExtrCalibration()->get3DPoint(cv::Point2f(px, py + 0.5), h);

    double x_dir = norm(p3x1 - p3x2);
    double y_dir = norm(p3y1 - p3y2);

    QPointF res(x_dir, y_dir);

    return res;
}
///*
/// Liefert den Winkel zwischen der Geraden von der Kamera
/// zum uebergebenen Punkt mit der Hoehe height
/// zur Grundflaeche [0-90] 90 => senkrecht unter der Kamera
/// Punktkoordinaten beinhalten die Border
///*
double ImageItem::getAngleToGround(float px, float py, float height)
{
    cv::Point3f cam(
        -mControlWidget->getCalibCoord3DTransX() - mControlWidget->getCalibExtrTrans1(),
        -mControlWidget->getCalibCoord3DTransY() - mControlWidget->getCalibExtrTrans2(),
        -mControlWidget->getCalibCoord3DTransZ() - mControlWidget->getCalibExtrTrans3());

    cv::Point3f posInImage = mMainWindow->getExtrCalibration()->get3DPoint(
        cv::Point2f(px - mMainWindow->getImageBorderSize(), py - mMainWindow->getImageBorderSize()), height);

    cv::Point3f a(cam.x - posInImage.x, cam.y - posInImage.y, cam.z - posInImage.z), b(0, 0, 1);

    return asin(
               (a.x * b.x + a.y * b.y + a.z * b.z) / (abs(sqrt(pow(a.x, 2) + pow(a.y, 2) + pow(a.z, 2))) *
                                                      abs(sqrt(pow(b.x, 2) + pow(b.y, 2) + pow(b.z, 2))))) *
           180 / PI;
}

QPointF ImageItem::getPosImage(QPointF pos, float height)
{
    cv::Point2f p2d;
    if(mImage)
    {
        if(mControlWidget->getCalibCoordDimension() == 0) // Tab coordinate system is on 3D
        {
            p2d = mMainWindow->getExtrCalibration()->getImagePoint(cv::Point3f(pos.x(), pos.y(), height));
            pos.setX(p2d.x);
            pos.setY(p2d.y);
        }
        else
        {
            //////////////

            // Fehlerhaft funktioniert nicht wie gewollt

            //////////////
            // Old 2D mapping of Pixelpoints to RealPositions
            pos.setY(-pos.y());
            pos /= mControlWidget->getCalibCoordUnit() /
                   100.; // durch 100., da coordsys so gezeichnet, dass 1 bei 100 liegt

            pos = mapFromItem(mCoordItem, pos); // Einheit anpassen...
            if(mControlWidget->isCoordUseIntrinsicChecked())
            {
                const auto camMat = mControlWidget->getIntrinsicCameraParams();
                pos.rx() -= camMat.getCx();
                pos.ry() -= camMat.getCy();
            }
            else
            {
                pos.rx() -= mImage->width() / 2. - .5;  // Bildmitte
                pos.ry() -= mImage->height() / 2. - .5; // Bildmitte
            }
            pos = (mControlWidget->getCameraAltitude() / (mControlWidget->getCameraAltitude() - height)) *
                  pos; //((a-height)/a)*pos;

            if(mControlWidget->isCoordUseIntrinsicChecked())
            {
                const auto camMat = mControlWidget->getIntrinsicCameraParams();
                pos.rx() += camMat.getCx();
                pos.ry() += camMat.getCy();
            }
            else
            {
                pos.rx() += mImage->width() / 2. - .5;  // Bildmitte
                pos.ry() += mImage->height() / 2. - .5; // Bildmitte
            }
        }
    }
    return pos;
}

// eingabe pos als pixelkoordinate des bildes
// result in cm (mit y-Achse nach oben gerichtet)
// default height = 0. in Headerdatei (in cm)
// wenn kein Bild vorliegt, wird eingabeposition durchgereicht - kommt nicht vor, da kein mouseevent
QPointF ImageItem::getPosReal(QPointF pos, double height)
{
    if(mImage)
    {
        int bS = mMainWindow->getImageBorderSize();

        // Switch between 2D and 3D CameraCalibration/Position calculation
        if(mControlWidget->getCalibCoordDimension() == 0)
        {
            // New 3D mapping of Pixelpoints to RealPositions
            cv::Point3f p3d =
                mMainWindow->getExtrCalibration()->get3DPoint(cv::Point2f(pos.x() - bS, pos.y() - bS), height);

            // ToDo: Getting the floor point of the Person! (Only the x/y-coordinates?)
            pos = QPointF(p3d.x, p3d.y);
        }
        else
        {
            //  statt mControlWidget->getCalibFx() muesste spaeter wert stehen, der im verzerrten Bild fX=fY angibt
            //  a = mControlWidget->getCalibFx()*getMeterPerPixel();
            //  a = mControlWidget->getCameraAltitude();
            //  -.5 da pixel von 0..1023 (in skala bis 1024 anfaengt) laufen
            if(mControlWidget->isCoordUseIntrinsicChecked())
            {
                const auto camMat = mControlWidget->getIntrinsicCameraParams();
                pos.rx() -= camMat.getCx();
                pos.ry() -= camMat.getCy();
            }
            else
            {
                pos.rx() -= mImage->width() / 2. - .5;  // Bildmitte
                pos.ry() -= mImage->height() / 2. - .5; // Bildmitte
            }
            pos = ((mControlWidget->getCameraAltitude() - height) / mControlWidget->getCameraAltitude()) *
                  pos; //((a-height)/a)*pos;
            if(mControlWidget->isCoordUseIntrinsicChecked())
            {
                const auto camMat = mControlWidget->getIntrinsicCameraParams();
                pos.rx() += camMat.getCx();
                pos.ry() += camMat.getCy();
            }
            else
            {
                pos.rx() += mImage->width() / 2. - .5;  // Bildmitte
                pos.ry() += mImage->height() / 2. - .5; // Bildmitte
            }
            // Old 2D mapping of Pixelpoints to RealPositions
            pos = mapToItem(mCoordItem, pos); // Einheit anpassen...
            pos *= mControlWidget->getCalibCoordUnit() /
                   100.; // durch 100., da coordsys so gezeichnet, dass 1 bei 100 liegt
            pos.setY(-pos.y());
        }
    }
    return pos;
}

// event, of moving mouse when mouse button is pressed (grid is over image)
void ImageItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsItem::mouseMoveEvent(event);
}

/**
 * @brief Updates the mousePosOnImage in Petrack
 * @param event
 */
void ImageItem::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    // sets pixel coord on image for further use
    // QPointF pos = event->pos();
    mMainWindow->setMousePosOnImage(event->pos());

    QGraphicsItem::hoverMoveEvent(event);
}
