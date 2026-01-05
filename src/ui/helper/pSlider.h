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

#ifndef PSLIDER_H
#define PSLIDER_H

#include <QKeyEvent>
#include <QMouseEvent>
#include <QPoint>
#include <QScrollEvent>
#include <QSlider>
#include <QTimer>


/**
 * Custom Slider-class that changes scrolling behaviour and adds a throttling option.
 *
 * To prevent an unwanted change in value when scrolling on the UI, this custom
 * implementation of the Slider only scrolls, if it has focus.
 * To throttle the amount of updates during a mouse event this slider has a custom throttling system
 * that only sends signals when a timer has run out.
 * This implementation keeps proper handling of keyboard navigation and fixes track clicks */
class PSlider : public QSlider
{
    Q_OBJECT
signals:
    void throttledValueChanged(int value);

public:
    PSlider(QWidget *parent = nullptr);
    void setThrottleInterval(int interval);
    void throttle();
    int  throttleInterval() const;
    void directEmit(int value);

protected:
    void wheelEvent(QWheelEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
private slots:
    void onThrottleTimerTimeout();

private:
    QTimer mThrottleTimer;
    QPoint mLastMousePos;

    static constexpr int DEFAULT_THROTTLE_INTERVAL = 100;
};
#endif // PSLIDER_H
