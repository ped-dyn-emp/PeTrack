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

#include "logoItem.h"

#include "petrack.h"

#include <QApplication>
#include <QPainter>


LogoItem::LogoItem(QWidget *wParent, QGraphicsItem *parent) : QGraphicsObject(parent)
{
    mMainWindow    = dynamic_cast<Petrack *>(wParent);
    mImage         = new QImage(":/logo"); // in icons.qrc definiert
    mFadeAnimation = new QPropertyAnimation(this, "opacity", this);
}

QRectF LogoItem::boundingRect() const
{
    return QRectF(0, 0, mImage->width(), mImage->height());
}

void LogoItem::paint(QPainter *painter, const QStyleOptionGraphicsItem * /*option*/, QWidget * /*widget*/)
{
    int w;
    int h;
    if(mMainWindow->getImage() && !mMainWindow->getImage()->isNull())
    {
        w = (mMainWindow->getImage()->width()) / 2 - (mImage->width()) / 2 - mMainWindow->getImageBorderSize();
        h = (mMainWindow->getImage()->height()) / 2 - (mImage->height()) / 2 - mMainWindow->getImageBorderSize();
    }
    else
    {
        w = 0;
        h = 0;
    }
    painter->drawImage(w, h, *mImage);
}

void LogoItem::fadeOut()
{
    mFadeAnimation->setStartValue(1);
    mFadeAnimation->setEndValue(0);
    mFadeAnimation->setDuration(mFadeDuration);
    mFadeAnimation->start();
}

void LogoItem::fadeIn()
{
    mFadeAnimation->setStartValue(0);
    mFadeAnimation->setEndValue(1);
    mFadeAnimation->setDuration(mFadeDuration);
    mFadeAnimation->start();
}
