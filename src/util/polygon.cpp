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

#include "polygon.h"

namespace walkarea
{

Polygon::Polygon(int id, PolygonType type, const QVector<QPointF> &points) : QPolygonF(points), mId(id), mType(type) {}

bool Polygon::contains(const QPointF &p) const
{
    return containsPoint(p, Qt::OddEvenFill);
}

bool Polygon::isInside(const Polygon &other) const
{
    for(const QPointF &point : *this)
    {
        if(!other.contains(point))
        {
            return false;
        }
    }
    return true;
}

bool Polygon::overlaps(const Polygon &other) const
{
    return !intersected(other).isEmpty();
}

bool Polygon::isValidPolygon() const
{
    return size() >= 4 && isClosed();
}


} // namespace walkarea
