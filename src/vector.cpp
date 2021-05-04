/*
 * PeTrack - Software for tracking pedestrians movement in videos
 * Copyright (C) 2010-2020 Forschungszentrum Jülich GmbH,
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

#include <opencv2/opencv.hpp>

#include <QPointF>

#include "vector.h"
#include "helper.h"

Vec3F::Vec3F()
    : mX(0),
      mY(0),
      mZ(0)
{
}
Vec3F::Vec3F(double x, double y, double z)
    : mX(x),
      mY(y),
      mZ(z)
{
}

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

Vec3F Vec3F::operator+(const Vec3F& v) const
{
    return Vec3F(mX + v.mX, mY + v.mY, mZ + v.mZ);
}

Vec3F Vec3F::operator-(const Vec3F& v) const
{
    return Vec3F(mX - v.mX, mY - v.mY, mZ - v.mZ);
}

const Vec3F Vec3F::operator+=(const Vec3F& v)
{
    mX += v.mX;
    mY += v.mY;
    mZ += v.mZ;
    return *this;
}

const Vec3F Vec3F::operator-=(const Vec3F& v)
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

double Vec3F::operator*(const Vec3F& v) const
{
    return mX * v.mX + mY * v.mY;
}

bool Vec3F::operator==(const Vec3F& v) const
{
    return mX == v.mX && mY == v.mY && mZ == v.mZ;
}

bool Vec3F::operator!=(const Vec3F& v) const
{
    return mX != v.mX || mY != v.mY || mZ != v.mZ;
}

double Vec3F::length() const
{
    return sqrt(mX*mX+mY*mY+mZ*mZ);
}

Vec3F Vec3F::unit() const
{
    if(mX == 0 && mY == 0)
        return Vec3F();

    double len = length();
    return Vec3F(mX / len, mY / len, mZ / len);
}

void Vec3F::normalize()
{
    double len = length();
    if (len != 0)
    {
        mX/=len;
        mY/=len;
        mZ/=len;
    }
}

double Vec3F::distanceToPoint(const Vec3F& p) const
{
    return ((operator-)(p)).length();
}


//---------------------------------------------------------------------------

Vec2F::Vec2F()
    : mX(0),
      mY(0)
{
}
Vec2F::Vec2F(double x, double y)
    : mX(x),
      mY(y)
{
}
Vec2F::Vec2F(const QPointF& v)
    : mX(v.x()),
      mY(v.y())
{
}

Vec2F::Vec2F(const cv::Point2f &p)
    : mX(p.x),
      mY(p.y)
{
}

Vec2F::Vec2F(const CvPoint* v)
    : mX(v->x),
      mY(v->y)
{
}
Vec2F::Vec2F(const CvPoint2D32f* v)
    : mX(v->x),
      mY(v->y)
{
}

CvPoint Vec2F::toCvPoint() const
{
    CvPoint p;
    p.x = myRound(mX);
    p.y = myRound(mY);
    return p;
}
CvPoint2D32f Vec2F::toCvPoint2D32f() const
{
    CvPoint2D32f p;
    p.x = mX;
    p.y = mY;
    return p;
}

QPoint Vec2F::toQPoint() const
{
    return QPoint(myRound(mX), myRound(mY));
}
QPointF Vec2F::toQPointF() const
{
    return QPointF(mX, mY);
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

// automatisch vom compiler (komponentenweise)
Vec2F Vec2F::operator=(const CvPoint *v)
{
    return Vec2F(v->x, v->y);
}

const Vec2F& Vec2F::operator+=(const Vec2F& v)
{
    mX += v.mX;
    mY += v.mY;
    return *this;
}
const Vec2F Vec2F::operator+(const Vec2F& v) const
{
    return Vec2F(*this) += v;
}

const Vec2F Vec2F::operator-=(const Vec2F& v)
{
    mX -= v.mX;
    mY -= v.mY;
    return *this;
}
const Vec2F Vec2F::operator-(const Vec2F& v) const
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

double Vec2F::operator*(const Vec2F& v) const
{
    return mX * v.mX + mY * v.mY;
}

bool Vec2F::operator==(const Vec2F& v) const
{
    return mX == v.mX && mY == v.mY;
}

bool Vec2F::operator!=(const Vec2F& v) const
{
    return mX != v.mX || mY != v.mY;
}

double Vec2F::length() const
{
    return hypot(mX, mY);
}

double Vec2F::angle() const
{
    return atan2(mY, mX);
}

Vec2F Vec2F::unit() const
{
    if(mX == 0 && mY == 0)
        return Vec2F();

    double len = hypot(mX, mY);
    return Vec2F(mX / len, mY / len);
}

Vec2F Vec2F::normal() const
{
    return Vec2F(-mY, mX);
}
void Vec2F::normalize()
{
    double len = length();
    if (len != 0)
    {
        mX/=len;
        mY/=len;
    }
}

double Vec2F::distanceToPoint(const Vec2F& p) const
{
    return ((operator-)(p)).length();
}

double Vec2F::distanceToLine(const Vec2F& p1, const Vec2F& p2) const
{
    Vec2F n = (p2-p1).normal(); // normal vector
    n.normalize(); // normalized normal vector
    return fabs((operator*)(n) - p1*n);
}
double Vec2F::angleBetweenVec(const Vec2F& v) const
{
    return acos((operator*)(v)/(length()*v.length()));
}

Vec2F Vec2F::fromAngle(double angle)
{
    return Vec2F(cos(angle), sin(angle));
}
