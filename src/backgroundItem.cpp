/*
 * PeTrack - Software for tracking pedestrians movement in videos
 * Copyright (C) 2010-2022 Forschungszentrum Jülich GmbH,
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

#include "backgroundItem.h"

#include "petrack.h"
#include "view.h"

#include <QtWidgets>

// in x und y gleichermassen skaliertes koordinatensystem,
// da von einer vorherigen intrinsischen kamerakalibrierung ausgegenagen wird,
// so dass pixel quadratisch
BackgroundItem::BackgroundItem(QWidget *wParent, QGraphicsItem *parent) : QGraphicsItem(parent)
{
    mMainWindow = (class Petrack *) wParent;
    mImage      = nullptr;
    //    setEnabled(false); // all mouse events cannot access this item, but it will be seen
    // einzig move koennte interessant sein, um grid zu verschieben?!
}

/**
 * @brief Bounding box of drawn to area.
 *
 * This bounding box is used to determine if this Item needs to be redrawn or not.
 * See the official Qt Docs for QGraphicsItem
 *
 * @return (updated) bounding rect of this item
 */
QRectF BackgroundItem::boundingRect() const
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

void BackgroundItem::paint(QPainter *painter, const QStyleOptionGraphicsItem * /*option*/, QWidget * /*widget*/)
{
    cv::Mat fg;

    if(mMainWindow->getBackgroundFilter())
    {
        fg = mMainWindow->getBackgroundFilter()->getForeground();
    }
    else
    {
        return;
    }

    if(!fg.empty())
    {
        if((mImage != nullptr) && ((mImage->width() != fg.cols) || (mImage->height() != fg.rows)))
        {
            delete mImage;
        }
        if(mImage == nullptr) // zu Beginn oder wenn sich die Groesse aendert
        {
            mImage = new QImage(fg.cols, fg.rows, QImage::Format_ARGB32);
        }

        int   x, y;
        auto *data  = fg.data;
        auto *yData = data;

        for(y = 0; y < fg.rows; y++)
        {
            // Pointer to the data information in the QImage for just one column
            // set pointer to value before, because ++p is faster than p++
            auto *p = mImage->scanLine(y) - 1;
            for(x = 0; x < fg.cols; x++)
            {
                *(++p) = 255;               // color.red();
                *(++p) = 255;               // color.green();
                *(++p) = 255;               // color.blue();
                *(++p) = (1 - *data) * 255; // color.alpha(); // 255;
                ++data;
            }
            data = (yData += fg.cols);
        }

        painter->setOpacity(0.7);
        painter->drawImage(-mMainWindow->getImageBorderSize(), -mMainWindow->getImageBorderSize(), *mImage);
    }
}
