/*
 * PeTrack - Software for tracking pedestrians movement in videos
 * Copyright (C) 2024 Forschungszentrum Jülich GmbH, IAS-7
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

#ifndef PSLIDER_H
#define PSLIDER_H


#include <QScrollEvent>
#include <QSlider>

/**
 * Custom Slider-class that changes scrolling behaviour.
 *
 * To prevent an unwanted change in value when scrolling on the UI, this custom
 * implementation of the Slider only scrolls, if it has focus.
 */
class PSlider : public QSlider
{
    Q_OBJECT
public:
    PSlider(QWidget *parent = nullptr) : QSlider(parent){};

protected:
    void wheelEvent(QWheelEvent *event) override;
};


#endif // PSLIDER_H
