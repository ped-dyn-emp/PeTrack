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

#include <QtWidgets>

#include "petrack.h"
#include "view.h"
#include "multiColorMarkerItem.h"
#include "multiColorMarkerWidget.h"
#include "tracker.h"
#include "animation.h"

using namespace::cv;

// in x und y gleichermassen skaliertes koordinatensystem,
// da von einer vorherigen intrinsischen kamerakalibrierung ausgegenagen wird,
// so dass pixel quadratisch 
MultiColorMarkerItem::MultiColorMarkerItem(QWidget *wParent, QGraphicsItem * parent)
    : QGraphicsItem(parent)
{
    mMainWindow = (class Petrack*) wParent;
    mImage = NULL;
//    mMask = NULL;
    //setAcceptsHoverEvents(true);

    //    setEnabled(false); // all mouse events connot access this item, but it will be seen
    // einzig move koennte interessant sein, um grid zu verschieben?!
//     setAcceptsHoverEvents(true);
}
// // bounding box wird durch linke obere ecke und breite/hoehe angegeben
// // wenn an den rand gescrollt wurde im view, dann wird durch das dynamische anpassen
// // bei trans und scale zwar zuerst alles neu gezeichnet durch update, 
// // aber beim verkleinern des scrollbereichs nur der teil von tracker neu gezeichnet
QRectF MultiColorMarkerItem::boundingRect() const
{
    if (mMainWindow->getImage())
        return QRectF(-mMainWindow->getImageBorderSize(), -mMainWindow->getImageBorderSize(), mMainWindow->getImage()->width(), mMainWindow->getImage()->height());
//         return QRectF(-mMainWindow->getImageBorderSize(), -mMainWindow->getImageBorderSize(), mImage->width(), mImage->height());
    else
        return QRectF(0, 0, 0, 0);
//     // bounding box wird in lokalen koordinaten angegeben!!! (+-10 wegen zahl "1")
//     if (mControlWidget->getCalibCoordShow())
//         return QRectF(-110., -110., 220., 220.);
//     else                    ;

//         return QRectF(0., 0., 0., 0.);

//     // sicher ware diese boundingbox, da alles
//     //     return QRectF(xMin, yMin, xMax-xMin, yMax-yMin);
//     // eigentlich muesste folgende Zeile reichen, aber beim ranzoomen verschwindet dann koord.sys.
//     //     return QRectF(mControlWidget->getCalibCoordTransX()/10.-scale, mControlWidget->getCalibCoordTransY()/10.-scale, 2*scale, 2*scale);
}

void MultiColorMarkerItem::setRect(Vec2F& v)
{
    mUlc = v; // upper left corner to draw
}

void MultiColorMarkerItem::paint(QPainter *painter, const QStyleOptionGraphicsItem */*option*/, QWidget */*widget*/)
{
    if (!mMask.empty())
    {
        if ((mImage != nullptr) && ((mImage->width() != mMask.cols) || (mImage->height() != mMask.rows)))
        {
            delete mImage; // delete null pointer is ok
            mImage = nullptr; // is not been done by delete
        }
        if (mImage == nullptr) // zu Beginn oder wenn sich die Groesse aendert
            mImage = new QImage(mMask.cols, mMask.rows, QImage::Format_ARGB32);

        int x,y;
        auto* data = mMask.data;
        auto* yData = data;
        int notMaskMask = ((int) !mMainWindow->getMultiColorMarkerWidget()->maskMask->isChecked())*255; // 255 oder 0

        for (y = 0; y < mMask.rows; y++)
        {
            // Pointer to the data information in the QImage for just one column
            // set pointer to value before, because ++p is faster than p++
            auto* p = mImage->scanLine(y)-1;
            for (x = 0; x < mMask.cols; x++)
            {
                *(++p) = *data; // color.blue();
                *(++p) = *data; // color.green();
                *(++p) = *data; // color.red();
                *(++p) = *data ? notMaskMask : 255; // color.alpha(); // 255;
                ++data;
            }
            data = (yData += mMask.cols/sizeof(char)); // because sometimes widthStep != width
        }
        painter->setOpacity(mMainWindow->getMultiColorMarkerWidget()->opacity->value()/100.);
        //painter->drawImage(mMainWindow->getRecoRoiItem()->rect().x(),mMainWindow->getRecoRoiItem()->rect().y(), *mImage); // during hoverMoveEvent of recognitionRect the painting moves with rect
        painter->drawImage(mUlc.x(),mUlc.y(), *mImage);

    }
}

// only pointer is set, no copy of data
void MultiColorMarkerItem::setMask(Mat &mask)
{
    mMask = mask;
}

// original width w and height h must be given
Mat MultiColorMarkerItem::createMask(int w, int h)
{
    if (w>0 && h>0 && (mMask.empty() || (!mMask.empty() && (w != mMask.cols || h != mMask.rows))))
    {
        mMask.create(h,w,CV_8UC1);
//        cvReleaseImage(&mMask);
//        mMask = cvCreateImage(cvSize(w, h), 8, 1);
    }
    return mMask;
}
