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

#include "wktParser.h"

#include "calibration/extrCalibration.h"
#include "calibration/worldImageCorrespondence.h"
#include "util/polygon.h"

#include <QPoint>
#include <QRegularExpression>
#include <QStringList>

namespace walkarea
{

QStringList WKTParser::extractPolygonStrings(const QString &wktString)
{
    // matches either WKT String with POLYGON or POLYGON Z and minimum amount of brackets
    // Doesn't check if polygon points are correct
    static const QRegularExpression reTop(
        R"(^\s*POLYGON\s*(?:Z\s*)?\(\(\s*(.*)\s*\)\)\s*$)",
        QRegularExpression::CaseInsensitiveOption | QRegularExpression::DotMatchesEverythingOption);

    const auto match = reTop.match(wktString);
    if(!match.hasMatch())
    {
        return QStringList();
    }

    // Split on "),(" allowing arbitrary whitespace around comma
    const QString content = match.captured(1);
    QStringList   rings   = content.split(QRegularExpression(R"(\)\s*,\s*\()"), Qt::SkipEmptyParts);
    for(QString &r : rings)
    {
        r = r.trimmed();
    }

    return rings;
}

Polygon WKTParser::parsePolygon(
    const QString                  &polygonString,
    const WorldImageCorrespondence *corr,
    walkarea::CoordinateUnit        unit)
{
    // matches Block of space-sepreated Coords XYZ or XY: regexr.com/8jbps
    static const QRegularExpression re(R"((-?\d+\.?\d*)\s+(-?\d+\.?\d*)(?:\s+(-?\d+\.?\d*))?)");
    QRegularExpressionMatchIterator it = re.globalMatch(polygonString);

    Polygon poly;
    bool    firstPoint = true;
    while(it.hasNext())
    {
        const auto    match = it.next();
        const double  x     = match.captured(1).toDouble();
        const double  y     = match.captured(2).toDouble();
        const QString zStr  = match.captured(3);

        // get first vertex height as polygon height if available
        if(firstPoint && !zStr.isEmpty())
        {
            poly.setHeight(zStr.toDouble());
            firstPoint = false;
        }

        QPointF pixelPos;
        if(unit == walkarea::CoordinateUnit::Meters)
        {
            // Currently PeTrack uses cm for world coordinates: conversions since walkarea/WKT use meters
            const QPointF worldPos = QPointF(x * 100.0, y * 100.0);
            pixelPos               = corr->getPosImage(worldPos, poly.height() * 100.0);
        }
        else
        {
            pixelPos = QPointF(x, y);
        }
        poly.append(pixelPos);
    }

    if(poly.first() != poly.last())
    {
        poly.append(poly.first());
    }
    return poly;
}

QVector<walkarea::Polygon> WKTParser::parseWorldWKT(const QString &wktString, const WorldImageCorrespondence *corr)
{
    QStringList strings = extractPolygonStrings(wktString);

    QVector<walkarea::Polygon> walkarea;
    walkarea.reserve(strings.size());
    for(const QString &str : strings)
    {
        Polygon poly = parsePolygon(str, corr, CoordinateUnit::Meters);
        walkarea.append(poly);
    }

    return walkarea;
}

QVector<walkarea::Polygon> WKTParser::parsePixelWKT(const QString &wktString)
{
    QStringList strings = extractPolygonStrings(wktString);

    QVector<walkarea::Polygon> walkarea;
    walkarea.reserve(strings.size());
    for(const QString &str : strings)
    {
        Polygon poly = parsePolygon(str, nullptr, CoordinateUnit::Pixels);
        walkarea.append(poly);
    }

    return walkarea;
}

QString WKTParser::polygonToString(const Polygon &polygon, const ExtrCalibration *calib, CoordinateUnit unit)
{
    QStringList coords;
    coords.reserve(polygon.size());
    for(const auto &point : polygon)
    {
        double x, y;
        if(unit == CoordinateUnit::Meters)
        {
            const cv::Point2f pixelPos = cv::Point2f(point.x(), point.y());
            // Currently PeTrack uses cm for world coordinates: conversions since walkarea/WKT use meters
            const cv::Point3f worldPos = calib->get3DPoint(pixelPos, polygon.height() * 100.0);
            x                          = worldPos.x / 100.0;
            y                          = worldPos.y / 100.0;
            coords.append(QString::number(x, 'g', 12) + " " + QString::number(y, 'g', 12));
        }
        else
        {
            x = point.x();
            y = point.y();
            coords.append(
                QString::number(x, 'g', 12) + " " + QString::number(y, 'g', 12) + " " +
                QString::number(polygon.height(), 'g', 12));
        }
    }
    return coords.join(", ");
}

QString WKTParser::walkareaToString(
    const QVector<walkarea::Polygon> &polygons,
    const ExtrCalibration            *calib,
    CoordinateUnit                    outputUnit)
{
    QStringList polygonStrings;
    polygonStrings.reserve(polygons.size());
    for(const auto &poly : polygons)
    {
        // only convert properly closed polygons
        if(poly.size() >= 4)
        {
            polygonStrings.append("(" + polygonToString(poly, calib, outputUnit) + ")");
        }
    }
    return polygonStrings.join(", ");
}

QString WKTParser::generatePixelWKT(const QVector<Polygon> &polygons)
{
    const QString header  = "POLYGON Z";
    const QString content = walkareaToString(polygons, nullptr, CoordinateUnit::Pixels);
    return QString("%1 (%2)").arg(header).arg(content);
}

QString WKTParser::generateWorldWKT(const QVector<Polygon> &polygons, const ExtrCalibration *calib)
{
    const QString header  = "POLYGON";
    const QString content = walkareaToString(polygons, calib, CoordinateUnit::Meters);
    return QString("%1 (%2)").arg(header).arg(content);
}

} // namespace walkarea
