#ifndef ELLIPSE_H
#define ELLIPSE_H

#include <cxcore.h>

#include "vector.h"

class QPointF;
class QSizeF;

class MyEllipse {
private:
    Vec2F mC; // center 
    double mR1; // radii
    double mR2; // guarantees that mR1 >= mR2
    double mAngle; // 0..PI arc between pos x axis and ellipse axis with size mR1 (>mR2)
    Vec2F mF1; // focal points
    Vec2F mF2;

public:
    MyEllipse();
    MyEllipse(double x, double y, double r1, double r2, double angle);
    MyEllipse(QPointF center, QSizeF size, double angle);
    MyEllipse(const MyEllipse &ellipse);

    Vec2F center() const;
    QSizeF size() const;

    // inline fkt muessen im header definiert werden!!!
    inline double x() const
    {
        return mC.x();
    }
    inline double y() const
    {
        return mC.y();
    }
    inline double r1() const
    {
        return mR1;
    }
    inline double r2() const
    {
        return mR2;
    }
    inline double angle() const
    {
        return mAngle;
    }

    inline double ratio() const // >=1
    {
        return mR1/mR2;
    }
    double area() const;
    // only estimation, because of complex elliptical integral
    double outline() const;
    // is point p inside or on the ellipse
    bool isInside(const Vec2F& p) const;
    bool isInside(double x, double y) const;
    bool isNearlyCircle() const;

    void draw(cv::Mat &img, int r, int g, int b) const;
};

#endif
