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

#include <QGraphicsScene>
#include <QPainter>
#include <QGraphicsSceneHoverEvent>
#include <cmath>

#include "imageItem.h"
#include "petrack.h"
#include "control.h"

ImageItem::ImageItem(QWidget *wParent, QGraphicsItem * parent)
    : QGraphicsItem(parent)
{
    mMainWindow = (class Petrack*) wParent;
    mControlWidget = mMainWindow->getControlWidget();
    mImage = nullptr;
    mCoordItem = nullptr;
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
    if (mImage)
    {
        return QRectF(0, 0, mImage->width(), mImage->height());
    }else
        return QRectF(0, 0, 0, 0);
}
    
void ImageItem::paint(QPainter *painter, const QStyleOptionGraphicsItem */*option*/, QWidget */*widget*/)
{
    if (mImage)
        painter->drawImage(0,0,*mImage);
}

void ImageItem::setImage(QImage *img)
{
    mImage = img;

    QTransform matrix;
    matrix.translate(1, 1); // FEHLER IN QT ????? noetig, damit trotz recognitionroiitem auch image auch ohne border komplett neu gezeichnet wird // wird 2 zeilen weiter zurueckgesetzt, aber mit 0, 0 geht es nicht
    setTransform(matrix);
    matrix.translate(-mMainWindow->getImageBorderSize()-1, -mMainWindow->getImageBorderSize()-1);
    setTransform(matrix);

    // value nicht setzen, da mgl mehrere videos mit gleichem objektiv erzeugt
    mControlWidget->setCalibCxMin(0 /*MAX(0,mImage->width()/2.-50)*/);
    mControlWidget->setCalibCxMax(mImage->width() /*mImage->width()/2.+50*/);
    mControlWidget->setCalibCyMin(0 /*MAX(0,mImage->height()/2.-50)*/);
    mControlWidget->setCalibCyMax(mImage->height() /*mImage->height()/2.+50*/);

    // trans nicht setzen, da mgl mehrere videos mit gleicher scene und gleichem koord sinnvoll
    mControlWidget->setCalibCoordTransXMin(-10*mMainWindow->getImageBorderSize());
    mControlWidget->setCalibCoordTransYMin(-10*mMainWindow->getImageBorderSize());
    mControlWidget->setCalibCoordTransXMax(10*(mImage->width()-mMainWindow->getImageBorderSize()));
    mControlWidget->setCalibCoordTransYMax(10*(mImage->height()-mMainWindow->getImageBorderSize()));

    mMainWindow->updateSceneRect();
}

void ImageItem::setCoordItem(QGraphicsItem *ci)
{ 
    mCoordItem = ci;
}
// in x und y richtung identisch, da vorher intrinsische kamerakalibrierung geschehen ist
double ImageItem::getCmPerPixel()
{
    if (mCoordItem)
    {
            // das sollte nur einmal berechne werden, wenn einfliessende daten sich aendern
            QPointF p1 = mapToItem(mCoordItem, QPointF(0.,0.));
            QPointF p2 = mapToItem(mCoordItem, QPointF(1.,0.));
            return mControlWidget->getCalibCoordUnit() * sqrt(pow(p1.x()-p2.x(),2) + pow(p1.y()-p2.y(),2)) / 100.;
            // durch 100., da coordsys so gezeichnet, dass 1 bei 100 liegt
    }
    else
        return 0.;
}
// Liefert zum Pixelpunkt (px,py) die Anzahl der Zentimeter in x- und y-Richtung
QPointF ImageItem::getCmPerPixel(float px, float py, float h){
    // ToDo:    
    // 3D Punkte an (px-0.5, py) und (px+0.5, py) berechnen und Auswirkung in x-Richtung
    // und          (px, py-0.5) und (px, py+0.5) berechnen und Auswirkung in y-Richtung untersuchen
    //
    // Unterscheiden nach x- und y-Richtung?
    // Wie fliesst die Hoehe mit ein?

    bool debug = false;

    cv::Point3f p3x1 = mMainWindow->getExtrCalibration()->get3DPoint(cv::Point2f(px-0.5,py),h);
    cv::Point3f p3x2 = mMainWindow->getExtrCalibration()->get3DPoint(cv::Point2f(px+0.5,py),h);

    cv::Point3f p3y1 = mMainWindow->getExtrCalibration()->get3DPoint(cv::Point2f(px,py-0.5),h);
    cv::Point3f p3y2 = mMainWindow->getExtrCalibration()->get3DPoint(cv::Point2f(px,py+0.5),h);

    if( debug ) debout << "Punkte: " << p3x1.x << ", " << p3x1.y << ", " << p3x1.z << std::endl;
    if( debug ) debout << "Punkte: " << p3x2.x << ", " << p3x2.y << ", " << p3x2.z << std::endl;
    if( debug ) debout << "Punkte: " << p3y1.x << ", " << p3y1.y << ", " << p3y1.z << std::endl;
    if( debug ) debout << "Punkte: " << p3y2.x << ", " << p3y2.y << ", " << p3y2.z << std::endl;

    double x_dir = norm(p3x1-p3x2);
    double y_dir = norm(p3y1-p3y2);

    if( debug ) debout << "x_dir: " << x_dir << ", y_dir: " << y_dir << " Durchschnitt: " << (0.5*(x_dir+y_dir)) << std::endl;

    QPointF res(x_dir,y_dir);

    if( debug ) debout << "CmPerPixel (x,y): " << res << std::endl;

    return res;
}
///*
/// Liefert den Winkel zwischen der Geraden von der Kamera
/// zum uebergebenen Punkt mit der Hoehe height
/// zur Grundflaeche [0-90] 90 => senkrecht unter der Kamera
/// Punktkoordinaten beinhalten die Border
///*
double ImageItem::getAngleToGround(float px, float py, float height){

    bool debug = false;

    cv::Point3f cam( - mControlWidget->getCalibCoord3DTransX() - mControlWidget->getCalibExtrTrans1(),
                 - mControlWidget->getCalibCoord3DTransY() - mControlWidget->getCalibExtrTrans2(),
                 - mControlWidget->getCalibCoord3DTransZ() - mControlWidget->getCalibExtrTrans3() );

    cv::Point3f posInImage = mMainWindow->getExtrCalibration()->get3DPoint(cv::Point2f(px-mMainWindow->getImageBorderSize(),py-mMainWindow->getImageBorderSize()),height);

    if( debug ) debout << "Camera:          " << cam.x << ", " << cam.y << ", " << cam.z << std::endl;
    if( debug ) debout << "posInImage:      " << posInImage.x << ", " << posInImage.y << ", " << posInImage.z << std::endl;

    cv::Point3f a(cam.x-posInImage.x,cam.y-posInImage.y,cam.z-posInImage.z),
            b(0,0,1);

    if( debug ) debout << "a: (" << a.x << ", " << a.y << ", " << a.z << ")" << std::endl;
    if( debug ) debout << "b: (" << b.x << ", " << b.y << ", " << b.z << ")" << std::endl;

    return asin( (a.x*b.x+a.y*b.y+a.z*b.z) / (abs(sqrt(pow(a.x,2)+pow(a.y,2)+pow(a.z,2)))*abs(sqrt(pow(b.x,2)+pow(b.y,2)+pow(b.z,2))))) * 180 / PI;
}

QPointF ImageItem::getPosImage(QPointF pos, float height)
{
    bool debug = false;
    cv::Point2f p2d;
    if( mImage )
    {
        if( mControlWidget->getCalibCoordDimension() == 0 ) // Tab coordinate system is on 3D
        {
            p2d = mMainWindow->getExtrCalibration()->getImagePoint(cv::Point3f(pos.x(),pos.y(),height));
            pos.setX(p2d.x);
            pos.setY(p2d.y);
        }else
        {
            //////////////

            //Fehlerhaft funktioniert nicht wie gewollt

            //////////////
            // Old 2D mapping of Pixelpoints to RealPositions
            if( debug ) debout << "x: " << pos.x() << " y: " << pos.y() << std::endl;
            pos.setY(-pos.y());
            pos /= mControlWidget->getCalibCoordUnit()/100.; // durch 100., da coordsys so gezeichnet, dass 1 bei 100 liegt
            if( debug ) debout << "x: " << pos.x() << " y: " << pos.y() << std::endl;

            pos = mapFromItem(mCoordItem, pos);// Einheit anpassen...
            if (mControlWidget->coordUseIntrinsic->checkState() == Qt::Checked)
            {
                pos.rx() -= mControlWidget->getCalibCxValue();
                pos.ry() -= mControlWidget->getCalibCyValue();
            }
            else
            {
                pos.rx() -= mImage->width()/2.-.5; // Bildmitte
                pos.ry() -= mImage->height()/2.-.5; // Bildmitte
            }
            if( debug ) debout << "x: " << pos.x() << " y: " << pos.y() << std::endl;
            pos = (mControlWidget->coordAltitude->value()/(mControlWidget->coordAltitude->value()-height))*pos; //((a-height)/a)*pos;
            if( debug ) debout << "x: " << pos.x() << " y: " << pos.y() << std::endl;
            if (mControlWidget->coordUseIntrinsic->checkState() == Qt::Checked)
            {
                pos.rx() += mControlWidget->getCalibCxValue();
                pos.ry() += mControlWidget->getCalibCyValue();
            }
            else
            {
                pos.rx() += mImage->width()/2.-.5; // Bildmitte
                pos.ry() += mImage->height()/2.-.5; // Bildmitte
            }
            if( debug ) debout << "x: " << pos.x() << " y: " << pos.y() << std::endl;

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
    if (mImage)
    {
        bool debug = false;
        int bS = mMainWindow->getImageBorderSize();

        if( debug ) debout << "Point pos.x: " << pos.x() << " pos.y: " << pos.y() << " height: " << height << std::endl;

        // Switch between 2D and 3D CameraCalibration/Position calculation
        if( mControlWidget->getCalibCoordDimension() == 0 )
        {
            // New 3D mapping of Pixelpoints to RealPositions
            cv::Point3f p3d = mMainWindow->getExtrCalibration()->get3DPoint(cv::Point2f(pos.x()-bS,pos.y()-bS),height);

            cv::Point2f p2d = debug ? mMainWindow->getExtrCalibration()->getImagePoint(p3d) : cv::Point2f(0,0);

            if( debug ) debout << "########## INFO ###############" << std::endl;
            if( debug ) debout << "Org. 2D Point: (" << pos.x() << ", " << pos.y() << ") Hoehe: "<< height << std::endl;
            if( debug ) debout << "Est. 3D Point: (" << p3d.x << ", " << p3d.y << ", " << p3d.z << ")" << std::endl;
            if( debug ) debout << "Est. 2D Point: (" << p2d.x << ", " << p2d.y << ")" << std::endl;
            if( debug ) debout << "######## END INFO #############" << std::endl;

            // ToDo: Getting the floor point of the Person! (Only the x/y-coordinates?)
            pos = QPointF(p3d.x,p3d.y);

        }
        else
        {
            if( debug ) debout << "########## INFO ###############" << std::endl;
            if( debug ) debout << "Org. 2D Point: (" << pos.x() << ", " << pos.y() << ") Hoehe: "<< height << std::endl;
            // statt mControlWidget->getCalibFx() muesste spaeter wert stehen, der im verzerrten Bild fX=fY angibt
            //a = mControlWidget->getCalibFxValue()*getMeterPerPixel();
            //a = mControlWidget->coordAltitude->value();
            // -.5 da pixel von 0..1023 (in skala bis 1024 anfaengt) laufen
            if (mControlWidget->coordUseIntrinsic->checkState() == Qt::Checked)
            {
                pos.rx() -= mControlWidget->getCalibCxValue();
                pos.ry() -= mControlWidget->getCalibCyValue();
            }
            else
            {
                pos.rx() -= mImage->width()/2.-.5; // Bildmitte
                pos.ry() -= mImage->height()/2.-.5; // Bildmitte
            }
            if( debug ) debout << "CoordAltitude: " << mControlWidget->coordAltitude->value() << std::endl;
            if( debug ) debout << "x: " << pos.x() << " y: " << pos.y() << std::endl;
            pos = ((mControlWidget->coordAltitude->value()-height)/mControlWidget->coordAltitude->value())*pos; //((a-height)/a)*pos;
            if( debug ) debout << "x: " << pos.x() << " y: " << pos.y() << std::endl;
            if (mControlWidget->coordUseIntrinsic->checkState() == Qt::Checked)
            {
                pos.rx() += mControlWidget->getCalibCxValue();
                pos.ry() += mControlWidget->getCalibCyValue();
            }
            else
            {
                pos.rx() += mImage->width()/2.-.5; // Bildmitte
                pos.ry() += mImage->height()/2.-.5; // Bildmitte
            }
            // Old 2D mapping of Pixelpoints to RealPositions
            if( debug ) debout << "x: " << pos.x() << " y: " << pos.y() << std::endl;
            pos = mapToItem(mCoordItem, pos);// Einheit anpassen...
            if( debug ) debout << "x: " << pos.x() << " y: " << pos.y() << std::endl;
            pos *= mControlWidget->getCalibCoordUnit()/100.; // durch 100., da coordsys so gezeichnet, dass 1 bei 100 liegt
            if( debug ) debout << "x: " << pos.x() << " y: " << pos.y() << std::endl;
            pos.setY(-pos.y());
            if( debug ) debout << "Est. 3D Point: (" << pos.x() << ", " << pos.y() << ", " << height << ")" << std::endl;
            if( debug ) debout << "######## END INFO #############" << std::endl;

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

    bool debug = false;
    if( debug )
    {
        QPointF cmPerPixel = getCmPerPixel(event->pos().x(),event->pos().y(),mMainWindow->getStatusPosRealHeight());
        double angleToGround = getAngleToGround(event->pos().x(),event->pos().y(), mMainWindow->getStatusPosRealHeight());
        debout << "At position: " << event->pos().x() << ", " << event->pos().y() << " cm per pixel: x: " << cmPerPixel.x() << ", y: " << cmPerPixel.y() << ", angle: " << angleToGround << std::endl;

        QPointF posReal = getPosReal(QPointF(event->pos().x(),event->pos().y()), mMainWindow->getStatusPosRealHeight());
        QPointF posImage = getPosImage(posReal,mMainWindow->getStatusPosRealHeight());
        debout << "Pos(real): " << posReal << " Pos(image): " << posImage << std::endl;

        // Pixel-Koordinaten unter der Kamera bestimmen
        QPointF pixUnderCam = getPosImage(QPointF(
                                              -mControlWidget->getCalibCoord3DTransX()-mControlWidget->getCalibExtrTrans1(),
                                              -mControlWidget->getCalibCoord3DTransY()-mControlWidget->getCalibExtrTrans2()),
                                          0);

        pixUnderCam.setX(pixUnderCam.x() + mMainWindow->getImageBorderSize());
        pixUnderCam.setY(pixUnderCam.y() + mMainWindow->getImageBorderSize());

        debout << "Pixel unter der Camera: " << pixUnderCam.x() << ", " << pixUnderCam.y() << std::endl;

        cv::Point2f pixUnderCam2f = mMainWindow->getExtrCalibration()->getImagePoint(cv::Point3f(
                                                                                -mControlWidget->getCalibCoord3DTransX()-mControlWidget->getCalibExtrTrans1(),
                                                                                -mControlWidget->getCalibCoord3DTransY()-mControlWidget->getCalibExtrTrans2(),
                                                                                0));

        pixUnderCam2f.x += mMainWindow->getImageBorderSize();
        pixUnderCam2f.y += mMainWindow->getImageBorderSize();

        debout << "Pixel unter der Camera: " << pixUnderCam2f.x << ", " << pixUnderCam2f.y << std::endl;

    }



    //QPointF pos = event->pos();
    mMainWindow->setMousePosOnImage(event->pos());

    QGraphicsItem::hoverMoveEvent(event);
}
