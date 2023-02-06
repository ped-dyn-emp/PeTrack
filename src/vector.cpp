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

#include "vector.h"

#include "helper.h"

#include <QPoint>
#include <QPointF>
#include <limits>
#include <opencv2/opencv.hpp>

Vec3F::Vec3F() : mX(0), mY(0), mZ(0) {}
Vec3F::Vec3F(double x, double y, double z) : mX(x), mY(y), mZ(z) {}

Vec3F::Vec3F(const cv::Point3f &v) : mX(v.x), mY(v.y), mZ(v.z) {}

double Vec3F::x() const
{
    return mX;
}
double Vec3F::y() const
{
    return mY;
}
double Vec3F::z() const
{
    return mZ;
}

void Vec3F::setX(double x)
{
    mX = x;
}
void Vec3F::setY(double y)
{
    mY = y;
}
void Vec3F::setZ(double z)
{
    mZ = z;
}

void Vec3F::set(double x, double y, double z)
{
    mX = x;
    mY = y;
    mZ = z;
}


Vec3F &Vec3F::operator=(const cv::Point3f &v)
{
    mX = v.x;
    mY = v.y;
    mZ = v.z;
    return *this;
}

Vec3F Vec3F::operator+(const Vec3F &v) const
{
    return Vec3F(*this) += v;
}

Vec3F Vec3F::operator-(const Vec3F &v) const
{
    return Vec3F(*this) -= v;
}

Vec3F &Vec3F::operator+=(const Vec3F &v)
{
    mX += v.mX;
    mY += v.mY;
    mZ += v.mZ;
    return *this;
}

Vec3F &Vec3F::operator-=(const Vec3F &v)
{
    mX -= v.mX;
    mY -= v.mY;
    mZ -= v.mZ;
    return *this;
}

Vec3F Vec3F::operator-() const
{
    return Vec3F(-mX, -mY, -mZ);
}

Vec3F Vec3F::operator*(double n) const
{
    return Vec3F(mX * n, mY * n, mZ * n);
}

Vec3F Vec3F::operator/(double n) const
{
    return Vec3F(mX / n, mY / n, mZ / n);
}

/**
 * @brief Calculates the dot product.
 * @return The dot product.
 */
double Vec3F::operator*(const Vec3F &v) const
{
    return mX * v.mX + mY * v.mY;
}

bool Vec3F::operator==(const Vec3F &v) const
{
    return mX == v.mX && mY == v.mY && mZ == v.mZ;
}

bool Vec3F::operator!=(const Vec3F &v) const
{
    return !(*this == v);
}

double Vec3F::length() const
{
    return sqrt(mX * mX + mY * mY + mZ * mZ);
}

/**
 * @brief Gets this vector in normalized form.
 *
 * @return A copy of this vector normalized to unit length.
 */
Vec3F Vec3F::unit() const
{
    Vec3F tmp(*this);
    tmp.normalize();
    return tmp;
}

/**
 * @brief Normalizes the components of the vector.
 *
 * This method normalizes the components of this vector to unit length.
 * If the length of the vector is 0 or very close to 0, then the vector will be a 0 vector.
 */
void Vec3F::normalize()
{
    double len = length();
    if(fabs(len) > std::numeric_limits<double>::epsilon())
    {
        mX /= len;
        mY /= len;
        mZ /= len;
    }
    else
    {
        // vector assumed to be 0 or very close to 0
        mX = 0.;
        mY = 0.;
        mZ = 0.;
    }
}

double Vec3F::distanceToPoint(const Vec3F &p) const
{
    return (*this - p).length();
}


//---------------------------------------------------------------------------

Vec2F::Vec2F() : mX(0), mY(0) {}
Vec2F::Vec2F(double x, double y) : mX(x), mY(y) {}
Vec2F::Vec2F(const QPointF &v) : mX(v.x()), mY(v.y()) {}

Vec2F::Vec2F(const cv::Point2f &p) : mX(p.x), mY(p.y) {}

Vec2F::Vec2F(const CvPoint *v) : mX(v->x), mY(v->y) {}
Vec2F::Vec2F(const CvPoint2D32f *v) : mX(v->x), mY(v->y) {}

CvPoint2D32f Vec2F::toCvPoint2D32f() const
{
    CvPoint2D32f p;
    p.x = mX;
    p.y = mY;
    return p;
}

cv::Point Vec2F::toCvPoint() const
{
    return cv::Point(myRound(mX), myRound(mY));
}

QPoint Vec2F::toQPoint() const
{
    return QPoint(myRound(mX), myRound(mY));
}
QPointF Vec2F::toQPointF() const
{
    return QPointF(mX, mY);
}

cv::Point2f Vec2F::toPoint2f() const
{
    return cv::Point2f(static_cast<float>(mX), static_cast<float>(mY));
}

double Vec2F::x() const
{
    return mX;
}
double Vec2F::y() const
{
    return mY;
}

void Vec2F::setX(double x)
{
    mX = x;
}
void Vec2F::setY(double y)
{
    mY = y;
}
void Vec2F::set(double x, double y)
{
    mX = x;
    mY = y;
}

Vec2F &Vec2F::operator+=(const Vec2F &v)
{
    mX += v.mX;
    mY += v.mY;
    return *this;
}
Vec2F Vec2F::operator+(const Vec2F &v) const
{
    return Vec2F(*this) += v;
}

Vec2F &Vec2F::operator-=(const Vec2F &v)
{
    mX -= v.mX;
    mY -= v.mY;
    return *this;
}
Vec2F Vec2F::operator-(const Vec2F &v) const
{
    return Vec2F(*this) -= v;
}

Vec2F Vec2F::operator-() const
{
    return Vec2F(-mX, -mY);
}

Vec2F Vec2F::operator*(double n) const
{
    return Vec2F(mX * n, mY * n);
}

Vec2F Vec2F::operator/(double n) const
{
    return Vec2F(mX / n, mY / n);
}

/**
 * @brief Calculates the dot product.
 *
 * @return The dot product.
 */
double Vec2F::operator*(const Vec2F &v) const
{
    return mX * v.mX + mY * v.mY;
}

bool Vec2F::operator==(const Vec2F &v) const
{
    return mX == v.mX && mY == v.mY;
}

bool Vec2F::operator!=(const Vec2F &v) const
{
    return !(*this == v);
}

double Vec2F::length() const
{
    return hypot(mX, mY);
}

/**
 * @brief Calculates the angle of the vector with atan2.
 *
 * This method calculates the angle of the vector using atan2.
 * That means (1,0) has an angle of 0 and (-1, 0) has an angle of pi.
 * The angles are mirrored on the x axis, however for negative y the angle will be negative.
 * To calculate the shortest angle to (1,0) the absolute value of this method can be used.
 * To calculate the angle in proper radians 2pi minus the return value of this method needs
 * to be used for all vectors with a negative y value. The angle is counted in
 * counter-clockwise direction then.
 *
 * @return The angle calculated with atan2.
 */
double Vec2F::angle() const
{
    return atan2(mY, mX);
}

/**
 * @brief Gets this vector in normalized form.
 *
 * @return A copy of this vector normalized to unit length.
 */
Vec2F Vec2F::unit() const
{
    Vec2F tmp(*this);
    tmp.normalize();
    return tmp;
}

/**
 * @brief Normalizes the components of the vector.
 *
 * This method normalizes the components of this vector to unit length.
 * If the length of the vector is 0 or very close to 0, then the vector will be a 0 vector.
 */
Vec2F Vec2F::normal() const
{
    return Vec2F(-mY, mX);
}

void Vec2F::normalize()
{
    double len = length();
    if(len > std::numeric_limits<double>::epsilon())
    {
        mX /= len;
        mY /= len;
    }
    else
    {
        mX = 0.;
        mY = 0.;
    }
}

double Vec2F::distanceToPoint(const Vec2F &p) const
{
    return (*this - p).length();
}

double Vec2F::distanceToLine(const Vec2F &p1, const Vec2F &p2) const
{
    Vec2F n = (p2 - p1).normal(); // normal vector
    n.normalize();                // normalized normal vector
    return fabs(*this * n - p1 * n);
}

double Vec2F::angleBetweenVec(const Vec2F &v) const
{
    return acos((*this * v) / (length() * v.length()));
}

Vec2F Vec2F::fromAngle(double angle)
{
    return Vec2F(cos(angle), sin(angle));
}
