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

#include "colorMarkerItem.h"

#include "animation.h"
#include "colorMarkerWidget.h"
#include "petrack.h"
#include "tracker.h"
#include "view.h"

#include <QtWidgets>


// in x und y gleichermassen skaliertes koordinatensystem,
// da von einer vorherigen intrinsischen kamerakalibrierung ausgegenagen wird,
// so dass pixel quadratisch
ColorMarkerItem::ColorMarkerItem(QWidget *wParent, QGraphicsItem *parent) : QGraphicsItem(parent)
{
    mMainWindow = (class Petrack *) wParent;
    mImage      = nullptr;
}

/**
 * @brief Bounding box of drawn to area.
 *
 * This bounding box is used to determine if this Item needs to be redrawn or not.
 * See the official Qt Docs for QGraphicsItem
 *
 * @return (updated) bounding rect of this item
 */
QRectF ColorMarkerItem::boundingRect() const
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

void ColorMarkerItem::setRect(Vec2F &v)
{
    mUlc = v; // upper left corner to draw
}

/**
 * @brief Paints color thresholding mask
 *
 * Paints a mask over the image which imitates the thresholding
 * via thresholdHSV(). The mask is set by using it in the thresholding.
 * It is accessed via createMask.
 *
 * @param painter
 */
void ColorMarkerItem::paint(QPainter *painter, const QStyleOptionGraphicsItem * /*option*/, QWidget * /*widget*/)
{
    if(!mMask.empty())
    {
        if((mImage != nullptr) && ((mImage->width() != mMask.cols) || (mImage->height() != mMask.rows)))
        {
            delete mImage;    // delete null pointer is ok
            mImage = nullptr; // is not been done by delete
        }
        if(mImage == nullptr) // zu Beginn oder wenn sich die Groesse aendert
            mImage = new QImage(mMask.cols, mMask.rows, QImage::Format_ARGB32);

        int   x, y;
        auto *data        = mMask.data; //->imageData);
        auto *yData       = data;
        int   notMaskMask = ((int) !mMainWindow->getColorMarkerWidget()->maskMask->isChecked()) * 255; // 255 oder 0

        for(y = 0; y < mMask.rows; y++)
        {
            // Pointer to the data information in the QImage for just one column
            // set pointer to value before, because ++p is faster than p++
            auto *p = mImage->scanLine(y) - 1;
            for(x = 0; x < mMask.cols; x++)
            {
                *(++p) = *data;                     // color.blue();
                *(++p) = *data;                     // color.green();
                *(++p) = *data;                     // color.red();
                *(++p) = *data ? notMaskMask : 255; // color.alpha(); // 255;
                ++data;
            }
            data = (yData += mMask.cols / sizeof(char)); // because sometimes widthStep != width
        }
        painter->setOpacity(mMainWindow->getColorMarkerWidget()->opacity->value() / 100.);
        painter->drawImage(mUlc.x(), mUlc.y(), *mImage);
    }
}

/// be aware of data sharing between cv::Mat's
void ColorMarkerItem::setMask(cv::Mat &mask)
{
    mMask = mask;
}

/// original width w and height h must be given
cv::Mat ColorMarkerItem::createMask(int w, int h)
{
    if(w > 0 && h > 0 && (mMask.empty() || (!mMask.empty() && (w != mMask.cols || h != mMask.rows))))
    {
        mMask.create(h, w, CV_8UC1);
    }
    return mMask;
}
