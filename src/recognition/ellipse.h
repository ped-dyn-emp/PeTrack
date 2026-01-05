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

#ifndef ELLIPSE_H
#define ELLIPSE_H

#include "vector.h"

#include <opencv2/opencv.hpp>

class QPointF;
class QSizeF;

class MyEllipse
{
private:
    Vec2F  mC;     // center
    double mR1;    // radii
    double mR2;    // guarantees that mR1 >= mR2
    double mAngle; // 0..PI arc between pos x axis and ellipse axis with size mR1 (>mR2)
    Vec2F  mF1;    // focal points
    Vec2F  mF2;

public:
    MyEllipse();
    MyEllipse(double x, double y, double r1, double r2, double angle);
    MyEllipse(QPointF center, QSizeF size, double angle);
    MyEllipse(const MyEllipse &c)            = default;
    MyEllipse(MyEllipse &&c)                 = default;
    MyEllipse &operator=(const MyEllipse &c) = default;
    MyEllipse &operator=(MyEllipse &&c)      = default;
    ~MyEllipse()                             = default;

    Vec2F  center() const;
    QSizeF size() const;

    inline double x() const { return mC.x(); }
    inline double y() const { return mC.y(); }
    inline double r1() const { return mR1; }
    inline double r2() const { return mR2; }
    inline double angle() const { return mAngle; }

    inline double ratio() const // >=1
    {
        return mR1 / mR2;
    }
    double area() const;
    // only estimation, because of complex elliptical integral
    double outline() const;
    // is point p inside or on the ellipse
    bool isInside(const Vec2F &p) const;
    bool isInside(double x, double y) const;
    bool isNearlyCircle() const;

    void draw(cv::Mat &img, int r, int g, int b) const;
};

#endif
