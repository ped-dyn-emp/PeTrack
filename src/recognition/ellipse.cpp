/*
 * PeTrack - Software for tracking pedestrians movement in videos
 * Copyright (C) 2025 Forschungszentrum Jülich GmbH, IAS-7
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

#include "ellipse.h"

#include "helper.h"
#include "vector.h"

#include <QPointF>
#include <QSizeF>
#include <cmath>

// Name musste MyEllipse genommen werden, da anscheinend Ellipse schon vergeben ist

// QRegion besitzt auch elliptische regionen und hat sogar schnittbereiche und abfrage ob punkt innerhalb
// kann aber nur ortogonal ausgerichtete ellipsen

// QGraphicsEllipseItem ist sehr umfangreich fuer die repraesentation
// kann auch schief stehen, hat aber keine abfragen, ob punkt innerhalb

MyEllipse::MyEllipse() : mR1(0), mR2(0), mAngle(0) {}
MyEllipse::MyEllipse(double x, double y, double r1, double r2, double angle) :
    mC(x, y), mR1(r1 > r2 ? r1 : r2), mR2(r1 > r2 ? r2 : r1), mAngle(angle)
{
    double dist = sqrt(mR1 * mR1 - mR2 * mR2); // distance of focal points from centre
    mF1         = mC + Vec2F::fromAngle(angle) * dist;
    mF2         = mC - Vec2F::fromAngle(angle) * dist;
}
MyEllipse::MyEllipse(QPointF center, QSizeF size, double angle)
{
    MyEllipse(center.x(), center.y(), size.width(), size.height(), angle);
}

Vec2F MyEllipse::center() const
{
    return mC;
}
QSizeF MyEllipse::size() const
{
    return QSizeF(mR1, mR2);
}

double MyEllipse::area() const
{
    return PI * mR1 * mR2;
}

/// only estimation, because of complex elliptical integral
double MyEllipse::outline() const
{
    return PI * (1.5 * (mR1 + mR2) - sqrt(mR1 * mR2));
    // weitere Moeglichkeiten, siehe http://de.wikipedia.org/wiki/Ellipse#Formelsammlung_Fl.C3.A4cheninhalt_und_Umfang
    // return
    // PI*(mR1+mR2)*(1+(3*((mR1-mR2)/(mR1+mR2))*((mR1-mR2)/(mR1+mR2)))/(10+sqrt(4-(3*((mR1-mR2)/(mR1+mR2))*((mR1-mR2)/(mR1+mR2))))));
    // return PI*sqrt(2.*(mR1*mR1+mR2*mR2));
    // return PI*(mR1+mR2);
}

/// is point p inside or on the ellipse
bool MyEllipse::isInside(const Vec2F &p) const
{
    // for a point to be inside the ellipse the sum of the
    // distances from both focal points must be <= 2 * mR1
    return ((p - mF1).length() + (p - mF2).length()) <= 2. * mR1;
}
bool MyEllipse::isInside(double x, double y) const
{
    return isInside(Vec2F(x, y));
}
bool MyEllipse::isNearlyCircle() const
{
    return ratio() < 1.3;
}

void MyEllipse::draw(cv::Mat &img, int r, int g, int b) const
{
    cv::Size size;
    size.width          = myRound(mR1);
    size.height         = myRound(mR2);
    Vec2F     centerVec = center();
    cv::Point centerPoint(centerVec.x(), centerVec.y());
    cv::ellipse(img, centerPoint, size, 180 * mAngle / PI, 0, 360, cv::Scalar(r, g, b), 1, cv::LINE_AA, 0);
}
