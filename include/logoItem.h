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

#ifndef LOGOITEM_H
#define LOGOITEM_H

#include <QGraphicsObject>

class Petrack;
class LogoItem;
class QPropertyAnimation;

class LogoItem : public QGraphicsObject
{
public:
    LogoItem(QWidget *wParent, QGraphicsItem *parent = nullptr);

    QRectF boundingRect() const override;

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    void fadeOut();
    void fadeIn();

private:
    Petrack             *mMainWindow;
    QImage              *mImage;
    QPropertyAnimation  *mFadeAnimation;
    static constexpr int mFadeDuration = 2'000; // in ms
};

#endif
