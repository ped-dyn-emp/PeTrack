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

#include "gridItem.h"

#include "control.h"
#include "coordinateSystemBox.h"
#include "logger.h"
#include "petrack.h"
#include "view.h"

#include <QtWidgets>
#include <cmath>

GridItem::GridItem(QWidget *wParent, QGraphicsItem *parent, CoordinateSystemBox *coordSys) :
    QGraphicsItem(parent), mCoordSys(coordSys)
{
    mMainWindow    = (class Petrack *) wParent;
    mExtrCalib     = mMainWindow->getExtrCalibration();
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
QRectF GridItem::boundingRect() const
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

// event, of moving mouse while pressing a mouse button
void GridItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if(!mControlWidget->getCalibGridFix() && mControlWidget->getCalibGridShow())
    {
        setCursor(Qt::SizeBDiagCursor);
        QPointF diff = event->scenePos() -
                       event->lastScenePos(); // screenPos()-buttonDownScreenPos(Qt::RightButton) also interesting
        if(event->buttons() == Qt::RightButton)
        {
            mControlWidget->setCalibGridRotate(
                mControlWidget->getCalibGridRotate() + (int) (3. * (diff.x() + diff.y())));
        }
        else if(event->buttons() == Qt::LeftButton)
        {
            if(mControlWidget->getCalibGridDimension() == 0)
            {
                const auto &swap = mCoordSys->getSwap3D();

                cv::Point3f p3d = mExtrCalib->get3DPoint(
                    cv::Point2f(event->scenePos().x(), event->scenePos().y()), mControlWidget->getCalibGrid3DTransZ());
                cv::Point3f p3d_last =
                    mExtrCalib->get3DPoint(cv::Point2f(mMouseX, mMouseY), mControlWidget->getCalibGrid3DTransZ());
                mControlWidget->setCalibGrid3DTransX(mGridTransX + (swap.x ? -1 : 1) * round(p3d.x - p3d_last.x));
                mControlWidget->setCalibGrid3DTransY(mGridTransY + (swap.y ? -1 : 1) * round(p3d.y - p3d_last.y));
            }
            else
            {
                SPDLOG_INFO("Grid Move 2D: {}, {}", diff.x(), diff.y());
                mControlWidget->setCalibGridTransX(mControlWidget->getCalibGridTransX() + (int) (10. * diff.x()));
                mControlWidget->setCalibGridTransY(mControlWidget->getCalibGridTransY() + (int) (10. * diff.y()));
            }
        }
        else if(event->buttons() == Qt::MiddleButton)
        {
            if(mControlWidget->getCalibGridDimension() == 0)
            {
                mControlWidget->setCalibGrid3DResolution(
                    mControlWidget->getCalibGrid3DResolution() + (int) (10. * (diff.x() - diff.y())));
            }
            else
            {
                mControlWidget->setCalibGridScale(
                    mControlWidget->getCalibGridScale() + (int) (10. * (diff.x() - diff.y())));
            }
        }
    }
    else
    {
        QGraphicsItem::mouseMoveEvent(event); // drag mach ich selber
    }
}

void GridItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if(!mControlWidget->getCalibGridFix() && mControlWidget->getCalibGridShow())
    {
        if(event->button() == Qt::LeftButton)
        {
            mMouseX = event->scenePos().x();
            mMouseY = event->scenePos().y();

            mGridTransX = mControlWidget->getCalibGrid3DTransX();
            mGridTransY = mControlWidget->getCalibGrid3DTransY();
        }
    }
    else
    {
        QGraphicsItem::mousePressEvent(event);
    }
}

void GridItem::drawLine(QPainter *painter, const std::array<cv::Point2f, 2> &p)
{
    QLineF line{p[0].x, p[0].y, p[1].x, p[1].y};
    line.setLength(1'000'000);
    painter->drawLine(line);
}

void GridItem::draw2DGrid(QPainter *painter, int imageHeight, int imageWidth, int borderSize)
{
    {
        double scale = mControlWidget->getCalibGridScale() / 10.;
        double tX    = mControlWidget->getCalibGridTransX() / 10.;
        double tY    = mControlWidget->getCalibGridTransY() / 10.;
        double angle = mControlWidget->getCalibGridRotate() / 10.;

        painter->save(); // save current state (like matrix) on stack

        QTransform matrixPaint;
        matrixPaint.translate(tX, tY);
        matrixPaint.rotate(angle);
        painter->setWorldTransform(matrixPaint,
                                   true); // true means relative not absolute transform
        painter->setPen(Qt::red);

        int maxExp = imageHeight > imageWidth ? imageHeight : imageWidth;
        for(int i = (int) -((maxExp + 100) / scale); i < 2 * (maxExp / scale); i++)
        {
            painter->drawLine(QPointF(i * scale, -maxExp - 100), QPointF(i * scale, 2 * maxExp));
            painter->drawLine(QPointF(-maxExp - 100, i * scale), QPointF(2 * maxExp, i * scale));
        }

        painter->restore(); // restore from stack

        // white polygon, to overcast sticking out parts of grid
        QPointF points[] = {
            QPointF(-borderSize, -borderSize),
            QPointF(imageWidth - borderSize, -borderSize),
            QPointF(imageWidth - borderSize, imageHeight - borderSize),
            QPointF(imageWidth + 3 * (maxExp + 200) - borderSize, imageHeight - borderSize),
            QPointF(imageWidth + 3 * (maxExp + 200) - borderSize, -2 * maxExp - 100 - borderSize),
            QPointF(-2 * maxExp - 400 - borderSize, -2 * maxExp - 100 - borderSize),
            QPointF(-2 * maxExp - 400 - borderSize, imageHeight + 3 * (maxExp + 200) - borderSize),
            QPointF(imageWidth + 3 * (maxExp + 200) - borderSize, imageHeight + 3 * (maxExp + 200) - borderSize),
            QPointF(imageWidth + 3 * (maxExp + 200) - borderSize, imageHeight - borderSize),
            QPointF(-borderSize, imageHeight - borderSize)};
        painter->setPen(Qt::NoPen);
        painter->setBrush(Qt::white);

        painter->drawPolygon(points, 10);
    }
}

void GridItem::draw3DGrid(
    QPainter   *painter,
    int         imageHeight,
    int         imageWidth,
    int         borderSize,
    bool        vanishPointYIsInsideImage,
    bool        vanishPointXIsInsideImage,
    cv::Point2f vanishPointY,
    cv::Point2f vanishPointX)
{
    {
        double minX = INT_MAX, minY = INT_MAX, maxX = INT_MIN, maxY = INT_MIN;
        int    yOffset = -borderSize;

        if(vanishPointXIsInsideImage)
        {
            yOffset = vanishPointX.y + 100;
        }
        if(vanishPointYIsInsideImage)
        {
            yOffset = vanishPointY.y + 100;
        }

        int   gridTransX = mControlWidget->getCalibGrid3DTransX();
        int   gridTransY = mControlWidget->getCalibGrid3DTransY();
        int   gridTransZ = mControlWidget->getCalibGrid3DTransZ();
        Vec3F gridTrans(gridTransX, gridTransY, gridTransZ);
        int   resolution = mControlWidget->getCalibGrid3DResolution();

        cv::Point3f corners[4];
        // top left corner
        corners[0] = mExtrCalib->get3DPoint(cv::Point2f(0 - borderSize, yOffset), gridTransZ);
        // top right corner
        corners[1] = mExtrCalib->get3DPoint(cv::Point2f(imageWidth, yOffset), gridTransZ);
        // bottom left corner
        corners[2] = mExtrCalib->get3DPoint(cv::Point2f(0 - borderSize, imageHeight), gridTransZ);
        // bottom right corner
        corners[3] = mExtrCalib->get3DPoint(cv::Point2f(imageWidth, imageHeight), gridTransZ);

        QPolygon poly;
        poly << QPoint(-borderSize, yOffset) << QPoint(imageWidth - borderSize, yOffset)
             << QPoint(imageWidth - borderSize, imageHeight - borderSize)
             << QPoint(-borderSize, imageHeight - borderSize);
        painter->setClipRegion(QRegion(poly));

        for(int i = 0; i < 4; i++)
        {
            double x = corners[i].x;
            double y = corners[i].y;

            maxX = x > maxX ? x : maxX;
            minX = x < minX ? x : minX;
            maxY = y > maxY ? y : maxY;
            minY = y < minY ? y : minY;
        }

        painter->setPen(Qt::red);

        const auto &swap       = mCoordSys->getSwap3D();
        const auto  coordTrans = mCoordSys->getCoordTrans3D();
        const auto  trans      = gridTrans - coordTrans;
        int         swapX      = swap.x ? -1 : 1;
        int         swapY      = swap.y ? -1 : 1;
        int         gridHeight = gridTransZ - coordTrans.z(); // Since ExtCalibration always uses origin
                                                              // but grid should be independent of that

        std::array<cv::Point2f, 2> linePoints;
        // horizontal lines from the left to the right on height tZ3D the lines start from origin point
        // (tY3D) until max_y or if tY3D < min_y it starts with min_y because otherwise it is outside the
        // image
        for(int i = trans.y(); i < (swapY > 0 ? maxY : -minY); i += resolution)
        {
            // Bildpunkte zu den Endpunkten der Linie holen
            linePoints[0] = mExtrCalib->getImagePoint(cv::Point3f(minX, swapY * i, gridHeight));
            linePoints[1] = mExtrCalib->getImagePoint(cv::Point3f(maxX, swapY * i, gridHeight));
            drawLine(painter, linePoints);
        }
        // see above but now the lines start from origin point (tY3D) until min_y
        // y-
        for(int i = trans.y() - resolution; i > (swapY > 0 ? minY : -maxY); i -= resolution)
        {
            // Bildpunkte zu den Endpunkten der Linie holen
            linePoints[0] = mExtrCalib->getImagePoint(cv::Point3f(minX, swapY * i, gridHeight));
            linePoints[1] = mExtrCalib->getImagePoint(cv::Point3f(maxX, swapY * i, gridHeight));
            drawLine(painter, linePoints);
        }
        // vertical lines from the top to the bottom on height tZ3D the lines start from origin point(tX3D)
        // until max_x of if tX3D < minx it starts with min_x because otherwise the lines are outside the
        // image x+
        for(int i = trans.x(); i < (swapX > 0 ? maxX : -minX); i += resolution)
        {
            // Bildpunkte zu den Endpunkten der Linie holen
            linePoints[0] = mExtrCalib->getImagePoint(cv::Point3f(swapX * i, minY, gridHeight));
            linePoints[1] = mExtrCalib->getImagePoint(cv::Point3f(swapX * i, maxY, gridHeight));
            drawLine(painter, linePoints);
        }
        // see above but now the lines start from origin point until min_x
        // x-
        for(int i = trans.x() - resolution; i > (swapX > 0 ? minX : -maxX); i -= resolution)
        {
            // Bildpunkte zu den Endpunkten der Linie holen
            linePoints[0] = mExtrCalib->getImagePoint(cv::Point3f(swapX * i, minY, gridHeight));
            linePoints[1] = mExtrCalib->getImagePoint(cv::Point3f(swapX * i, maxY, gridHeight));
            drawLine(painter, linePoints);
        }
    }
}

void GridItem::paint(QPainter *painter, const QStyleOptionGraphicsItem * /*option*/, QWidget * /*widget*/)
{
    if(!mControlWidget->getCalibGridFix() && mControlWidget->getCalibGridShow())
    {
        // such that mouseEvent leftmousebutton is forwarded; drag is done ourself
        setFlag(ItemIsMovable);
    }
    else
    {
        setFlag(ItemIsMovable, false);
    }

    // confirmation prompt if the vanish points are inside the image
    bool        vanishPointYIsInsideImage = false;
    bool        vanishPointXIsInsideImage = false;
    cv::Point2f vanishPointY, vanishPointX;
    double      x, y;
    if(mMainWindow->getImage())
    {
        // create 2 parallel lines in x-direction
        cv::Point3f a3d = cv::Point3f(-500, -500, 0), b3d = cv::Point3f(500, -500, 0), c3d = cv::Point3f(-500, 500, 0),
                    d3d = cv::Point3f(500, 500, 0);
        cv::Point2f a2d = mExtrCalib->getImagePoint(a3d), b2d = mExtrCalib->getImagePoint(b3d),
                    c2d = mExtrCalib->getImagePoint(c3d), d2d = mExtrCalib->getImagePoint(d3d);

        // y = m*x+n
        float m1 = (b2d.y - a2d.y) / (b2d.x - a2d.x), m2 = (d2d.y - c2d.y) / (d2d.x - c2d.x), n1 = a2d.y - m1 * a2d.x,
              n2 = c2d.y - m2 * c2d.x;

        x = (n2 - n1) / (m1 - m2);
        y = ((m1 * x + n1) + (m2 * x + n2)) / 2.0;

        vanishPointY = cv::Point2f(x, y);

        // create 2 parallel lines in y-direction
        a3d = cv::Point3f(-500, -500, 0), b3d = cv::Point3f(-500, 500, 0), c3d = cv::Point3f(500, -500, 0),
        d3d = cv::Point3f(500, 500, 0);
        a2d = mExtrCalib->getImagePoint(a3d), b2d = mExtrCalib->getImagePoint(b3d),
        c2d = mExtrCalib->getImagePoint(c3d), d2d = mExtrCalib->getImagePoint(d3d);

        // y = m*x+n
        m1 = (b2d.y - a2d.y) / (b2d.x - a2d.x), m2 = (d2d.y - c2d.y) / (d2d.x - c2d.x), n1 = a2d.y - m1 * a2d.x,
        n2 = c2d.y - m2 * c2d.x;


        x            = (n2 - n1) / (m1 - m2);
        y            = ((m1 * x + n1) + (m2 * x + n2)) / 2.0;
        vanishPointX = cv::Point2f(x, y);

        vanishPointYIsInsideImage = !mExtrCalib->isOutsideImage(vanishPointY);
        vanishPointXIsInsideImage = !mExtrCalib->isOutsideImage(vanishPointX);

        ////////////////////////////////
        // Drawing Vanish Points      //
        ////////////////////////////////
        if(mCoordSys->getCalibExtrVanishPointsShow())
        {
            painter->setPen(Qt::yellow);
            painter->drawLine(
                QPointF(vanishPointY.x - 10, vanishPointY.y - 10), QPointF(vanishPointY.x + 10, vanishPointY.y + 10));
            painter->drawLine(
                QPointF(vanishPointY.x - 10, vanishPointY.y + 10), QPointF(vanishPointY.x + 10, vanishPointY.y - 10));
            painter->setPen(Qt::red);
            painter->drawPoint(vanishPointY.x, vanishPointY.y);
            painter->drawText(vanishPointY.x - 20, vanishPointY.y - 10, "Vanishing Point (y)");

            painter->setPen(Qt::yellow);
            painter->drawLine(
                QPointF(vanishPointX.x - 10, vanishPointX.y - 10), QPointF(vanishPointX.x + 10, vanishPointX.y + 10));
            painter->drawLine(
                QPointF(vanishPointX.x - 10, vanishPointX.y + 10), QPointF(vanishPointX.x + 10, vanishPointX.y - 10));

            painter->setPen(Qt::red);
            painter->drawPoint(vanishPointX.x, vanishPointX.y);
            painter->drawText(vanishPointX.x - 20, vanishPointX.y - 10, "Vanishing Point (x)");
        }
    }

    if(!mControlWidget->getCalibGridShow())
    {
        return;
    }

    QImage *img = mMainWindow->getImage();
    int     bS  = mMainWindow->getImageBorderSize();
    int     iW, iH;

    if(img)
    {
        iW = img->width();
        iH = img->height();
    }
    else
    {
        iW = (int) mMainWindow->getScene()->width();
        iH = (int) mMainWindow->getScene()->height();
    }

    if(mControlWidget->getCalibGridDimension() == 1)
    {
        draw2DGrid(painter, iH, iW, bS);
    }
    else
    {
        draw3DGrid(
            painter, iH, iW, bS, vanishPointYIsInsideImage, vanishPointXIsInsideImage, vanishPointY, vanishPointX);
    }
}
