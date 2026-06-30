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

#ifndef WKTPARSER_H
#define WKTPARSER_H

#include <QString>
#include <QVector>

class WorldImageCorrespondence;
class ExtrCalibration;

namespace walkarea
{

class Polygon;

/**
 * @brief Coordinate unit for WKT import/export
 */
enum class CoordinateUnit
{
    Pixels,
    Meters
};

/**
 * @brief WKT parser/generator for POLYGON with holes
 * @details
 * Converts between WKT strings and pixel-based polygon representations.
 * Coordinates are stored internally as image pixels.
 */
class WKTParser
{
public:
    /**
     * @brief Parse WKT string into polygon list (pixel coordinates)
     * @param wktString Input WKT format: POLYGON ((x y, ...), ...)
     * @param corr WorldImageCorrespondence for coordinate transformation
     * @return Vector of polygons (each polygon is closed: first == last), empty on parse error
     */
    static QVector<Polygon> parseWorldWKT(const QString &wktString, const WorldImageCorrespondence *corr);

    static QVector<Polygon> parsePixelWKT(const QString &wktString);


    /**
     * @brief Generate WKT string from polygon list (pixel coordinates)
     * @param polygons Vector of polygons in image coordinates (must be closed)
     * @param corr WorldImageCorrespondence for coordinate transformation
     * @return WKT format string, empty if input is empty or invalid
     */
    static QString generateWorldWKT(const QVector<Polygon> &polygons, const ExtrCalibration *calib);

    static QString generatePixelWKT(const QVector<Polygon> &polygons);

private:
    static QStringList extractPolygonStrings(const QString &wktString);
    static Polygon
    parsePolygon(const QString &polygonString, const WorldImageCorrespondence *corr, CoordinateUnit inputUnit);

    static QString polygonToString(const Polygon &polygon, const ExtrCalibration *calib, CoordinateUnit outputUnit);
    static QString walkareaToString(
        const QVector<walkarea::Polygon> &polygons,
        const ExtrCalibration            *calib,
        CoordinateUnit                    outputUnit);
};

} // namespace walkarea

#endif // WKTPARSER_H