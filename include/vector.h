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

#ifndef VECTOR_H
#define VECTOR_H

#include "helper.h"

#include <QVector3D>
#include <opencv2/core/types_c.h>

struct CvPoint;
struct CvPoint2D32f;
class QPointF;
class QPoint;


class Vec3F
{
protected:
    double mX;
    double mY;
    double mZ;

public:
    Vec3F();
    Vec3F(double x, double y, double z);
    ~Vec3F() = default;

    Vec3F(const Vec3F &v) = default;
    Vec3F(Vec3F &&v)      = default;

    Vec3F(const cv::Point3f &v);

    cv::Point3f toCvPoint();

    double  x() const;
    double  y() const;
    double  z() const;
    void    setX(double x);
    void    setY(double y);
    void    setZ(double z);
    void    set(double x, double y, double z);
    double &operator[](size_t i)
    {
        if(i == 0)
        {
            return mX;
        }
        else if(i == 1)
        {
            return mY;
        }
        else if(i == 2)
        {
            return mZ;
        }
        else
        {
            throw std::runtime_error("Out-of-range index for Vec3F");
        }
    }

    Vec3F &operator=(const Vec3F &v) = default;
    Vec3F &operator=(Vec3F &&v)      = default;

    Vec3F &operator=(const cv::Point3f &v);


    Vec3F  operator+(const Vec3F &v) const;
    Vec3F  operator-(const Vec3F &v) const;
    Vec3F &operator+=(const Vec3F &v);
    Vec3F &operator-=(const Vec3F &v);
    Vec3F  operator-() const;

    Vec3F operator*(double n) const; // multiply with a scalar value
    Vec3F operator/(double n) const; // divide by a scalar value

    double operator*(const Vec3F &v) const; // dot product

    bool operator==(const Vec3F &v) const;
    bool operator!=(const Vec3F &v) const;

    double length() const;
    Vec3F  unit() const;
    void   normalize();

    double distanceToPoint(const Vec3F &p) const;
};

inline Vec3F operator*(double f, const Vec3F &v)
{
    return v * f;
}

inline std::ostream &operator<<(std::ostream &s, const Vec3F &v)
{
    s << "(" << v.x() << ", " << v.y() << ", " << v.z() << ")";
    return s;
}

//-------------------------------------------------------------------------------

class Vec2F
{
protected:
    // private: dann waeren mX... nicht direkt aus abgeleitet klassen zugreifbar
    double mX;
    double mY;

public:
    Vec2F();
    Vec2F(double x, double y);
    Vec2F(const QPointF &v);
    Vec2F(const cv::Point2f &p);
    Vec2F(const CvPoint *v);
    Vec2F(const CvPoint2D32f *v);
    Vec2F(const Vec2F &c)            = default;
    Vec2F(Vec2F &&c)                 = default;
    Vec2F &operator=(const Vec2F &c) = default;
    Vec2F &operator=(Vec2F &&c)      = default;
    ~Vec2F()                         = default;

    cv::Point    toCvPoint() const;
    CvPoint2D32f toCvPoint2D32f() const;
    QPoint       toQPoint() const;
    QPointF      toQPointF() const;
    cv::Point2f  toPoint2f() const;

    double  x() const;
    double  y() const;
    void    setX(double x);
    void    setY(double y);
    void    set(double x, double y);
    double &operator[](size_t i)
    {
        if(i == 0)
        {
            return mX;
        }
        else if(i == 1)
        {
            return mY;
        }
        else
        {
            throw std::runtime_error("Out-of-range index for Vec2F");
        }
    }

    Vec2F &operator=(const CvPoint *v);

    Vec2F &operator+=(const Vec2F &v);
    Vec2F  operator+(const Vec2F &v) const;
    Vec2F &operator-=(const Vec2F &v);
    Vec2F  operator-(const Vec2F &v) const;
    Vec2F  operator-() const;

    Vec2F operator*(double n) const; // multiply with a scalar value
    Vec2F operator/(double n) const; // divide by a scalar value

    double operator*(const Vec2F &v) const; // dot product

    bool operator==(const Vec2F &v) const;
    bool operator!=(const Vec2F &v) const;

    double length() const;

    double angle() const;

    Vec2F unit() const;

    Vec2F normal() const;

    void normalize();

    double distanceToPoint(const Vec2F &p) const;
    // return distance from vec to line between p1 and p2
    double distanceToLine(const Vec2F &p1, const Vec2F &p2) const;
    double angleBetweenVec(const Vec2F &v) const;

    static Vec2F fromAngle(double angle);
};


inline Vec2F operator*(double f, const Vec2F &v)
{
    return v * f;
}

inline std::ostream &operator<<(std::ostream &s, const Vec2F &v)
{
    s << "(" << v.x() << ", " << v.y() << ")";
    return s;
}

// only for combine programm, not for petrack
inline QTextStream &operator<<(QTextStream &s, const Vec2F &v)
{
    s << v.x() << " " << v.y(); // ohne (,) !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    return s;
}
inline std::istream &operator>>(std::istream &s, Vec2F &v)
{
    double d;
    s >> d;
    v.setX(d);
    s >> d;
    v.setY(d);
    return s;
}
inline QTextStream &operator>>(QTextStream &s, Vec2F &v)
{
    double d;
    s >> d;
    v.setX(d);
    s >> d;
    v.setY(d);
    return s;
}

#endif
