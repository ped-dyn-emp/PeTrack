/*
 * PeTrack - Software for tracking pedestrians movement in videos
 * Copyright (C) 2026 Forschungszentrum Jülich GmbH, IAS-7
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
#include "pSlider.h"

#include <QKeyEvent>
#include <QStyleOptionSlider>

PSlider::PSlider(QWidget *parent) : QSlider(parent)
{
    mThrottleTimer.setSingleShot(true);
    mThrottleTimer.setInterval(0);
    connect(&mThrottleTimer, &QTimer::timeout, this, &PSlider::onThrottleTimerTimeout);

    connect(
        this,
        &QSlider::valueChanged,
        this,
        [this](int val)
        {
            if(!mThrottleTimer.isActive())
            {
                emit throttledValueChanged(val);
            }
        });
}

/**
 * Sets the throttling interval in ms
 * This determines how often the slider can emit throttledValueChanged during a mouse drag
 * @param interval int
 */
void PSlider::setThrottleInterval(int interval)
{
    if(interval > 0)
    {
        mThrottleTimer.setInterval(interval);
    }
}

void PSlider::throttle()
{
    setThrottleInterval(DEFAULT_THROTTLE_INTERVAL);
}

int PSlider::throttleInterval() const
{
    return mThrottleTimer.interval();
}

/**
 * Overwritten event that only scrolls if there is focus on this object
 * @param e QWheelEvent
 */
void PSlider::wheelEvent(QWheelEvent *event)
{
    if(hasFocus())
    {
        QSlider::wheelEvent(event);
    }
    else
    {
        event->ignore();
    }
}


void PSlider::mousePressEvent(QMouseEvent *event)
{
    // Handle clicking on the slider track
    if(event->button() == Qt::LeftButton)
    {
        if(mThrottleTimer.interval() > 0)
        {
            mThrottleTimer.start();
        }
        QStyleOptionSlider opt;
        initStyleOption(&opt);

        QRect controlRect = style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderHandle, this);

        if(!controlRect.contains(event->pos()))
        {
            // If click is outside the handle, calculate and set value directly
            int val = QStyle::sliderValueFromPosition(
                minimum(),
                maximum(),
                orientation() == Qt::Horizontal ? event->pos().x() : height() - event->pos().y(),
                orientation() == Qt::Horizontal ? width() : height());

            mLastMouseVal = val;

            // emit directly
            directEmit(val);
        }
    }

    QSlider::mousePressEvent(event);
}

void PSlider::mouseReleaseEvent(QMouseEvent *event)
{
    mThrottleTimer.stop();
    QSlider::mouseReleaseEvent(event);
    mLastMouseVal = value();

    // Ensure final value is emitted
    directEmit(mLastMouseVal);
}

void PSlider::mouseMoveEvent(QMouseEvent *event)
{
    if(mThrottleTimer.interval() > 0 && !mThrottleTimer.isActive())
    {
        mThrottleTimer.start();
    }
    QSlider::mouseMoveEvent(event);
    mLastMouseVal = value();
}


void PSlider::onThrottleTimerTimeout()
{
    if(!isSliderDown())
    {
        mThrottleTimer.stop();
        return;
    }

    emit throttledValueChanged(mLastMouseVal);
}


void PSlider::directEmit(int value)
{
    if(value != QSlider::value())
    {
        QSlider::setValue(value);
    }
    emit throttledValueChanged(value);
}
