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

#ifndef POLYGON_H
#define POLYGON_H

#include <QPointF>
#include <QPolygonF>
#include <QVector>

namespace walkarea
{

enum class PolygonType
{
    None,
    Walkable,
    Obstacle
};

enum class ValidationError
{
    None,
    ObstacleOutside,         // Obstacle is outside walkable area
    ObstaclesOverlapping,    // Two obstacles overlap
    ObstacleContainsWalkable // Walkable area is inside an obstacle
};

class Polygon : public QPolygonF
{
public:
    Polygon() = default;
    Polygon(int id, PolygonType type, const QVector<QPointF> &points);

    int         id() const { return mId; }
    PolygonType type() const { return mType; }
    double      height() const { return mHeightMeters; }
    bool        disabled() const { return mDisabled; }

    void setId(int id) { mId = id; }
    void setType(PolygonType type) { mType = type; }
    void setPoints(const QVector<QPointF> &points) { QPolygonF::operator=(points); }
    void setHeight(double meters) { mHeightMeters = meters; }
    void setDisabled(bool disabled) { mDisabled = disabled; }
    void setValidationError(ValidationError error) { mValidationError = error; }

    // Validation methods
    bool contains(const QPointF &point) const;
    bool isInside(const Polygon &other) const;
    bool overlaps(const Polygon &other) const;
    bool isValidPolygon() const;

private:
    int             mId              = -1;
    PolygonType     mType            = PolygonType::None;
    double          mHeightMeters    = 0.0;
    bool            mDisabled        = false;
    ValidationError mValidationError = ValidationError::None;
};

} // namespace walkarea

#endif
