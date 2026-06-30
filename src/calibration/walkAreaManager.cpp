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

#include "walkAreaManager.h"

#include "importHelper.h"
#include "logger.h"
#include "pGroupBox.h"
#include "polygon.h"
#include "util/wktParser.h"
#include "walkAreaWidget.h"

#include <QDomDocument>

WalkAreaManager::WalkAreaManager(WalkAreaWidget *parent) : mParent(parent)
{
    mSelection.setCompletionCallback(
        [this](const QVector<QPointF> &points)
        {
            addPolygon(points, mCurrentType);
            mSelection.reset();
        });

    mSelection.setShowCurMousePos(true);
    connect(this, &WalkAreaManager::visibilityChanged, this, &WalkAreaManager::imageUpdate);
    connect(this, &WalkAreaManager::geometryChanged, this, &WalkAreaManager::imageUpdate);
}

void WalkAreaManager::setVisible(bool val)
{
    if(mVisible == val)
    {
        return;
    }
    mVisible = val;
    emit visibilityChanged(mVisible, mWalkableVisible, mObstacleVisible);
}

void WalkAreaManager::setWalkableVisible(bool v)
{
    if(mWalkableVisible == v)
    {
        return;
    }
    mWalkableVisible = v;
    emit visibilityChanged(mVisible, mWalkableVisible, mObstacleVisible);
}

void WalkAreaManager::setObstacleVisible(bool v)
{
    if(mObstacleVisible == v)
    {
        return;
    }
    mObstacleVisible = v;
    emit visibilityChanged(mVisible, mWalkableVisible, mObstacleVisible);
}

void WalkAreaManager::setEditMode(bool val)
{
    if(mEdit == val)
    {
        return;
    }
    mEdit = val;
    if(!val)
    {
        mSelection.reset();
    }
    if(val && !mVisible)
    {
        setVisible(true);
    }

    SPDLOG_INFO("Walk area: in edit mode: {}", val);

    emit editModeChanged(val);
    emit imageUpdate();
}

void WalkAreaManager::setDrawingType(walkarea::PolygonType t)
{
    if(mCurrentType == t)
    {
        return;
    }
    mCurrentType = t;
    emit drawingTypeChanged(t);
}

void WalkAreaManager::ensureNextObstacleIdFree()
{
    // make sure nextId is not presend in map of obstacles
    while(mObstacles.find(mNextObstacleId) != mObstacles.end())
    {
        ++mNextObstacleId;
    }
}

void WalkAreaManager::addPolygon(const QVector<QPointF> &points, walkarea::PolygonType type)
{
    walkarea::Polygon poly;
    poly.setType(type);
    poly.setPoints(points);

    if(!poly.isValidPolygon())
    {
        SPDLOG_WARN("Walk area: invalid polygon (not closed or too few vertices)");
        return;
    }

    if(type == walkarea::PolygonType::Walkable && !mWalkableArea.has_value())
    {
        poly.setId(0);
        mWalkableArea = poly;
        setDrawingType(walkarea::PolygonType::Obstacle);
        SPDLOG_INFO("Walk area: set WALKABLE polygon with {} vertices", poly.size());
    }
    else if(type == walkarea::PolygonType::Obstacle)
    {
        ensureNextObstacleIdFree();
        poly.setId(mNextObstacleId);
        mObstacles.insert_or_assign(mNextObstacleId, poly);
        ++mNextObstacleId;
        SPDLOG_INFO("Walk area: new OBSTACLE polygon (id={}) with {} vertices", poly.id(), poly.size());
    }

    validatePolygons();
    emit geometryChanged();
}

void WalkAreaManager::removePolygon(int id)
{
    if(mWalkableArea.has_value() && mWalkableArea->id() == id)
    {
        mWalkableArea.reset();
        SPDLOG_INFO("Walk area: walkable area removed");
        setDrawingType(walkarea::PolygonType::Walkable);
    }
    else if(mObstacles.find(id) != mObstacles.end())
    {
        mObstacles.erase(id);
        SPDLOG_INFO("Walk area: obstacle {} removed", id);

        if(mObstacles.empty())
        {
            mNextObstacleId = 1;
        }
    }

    validatePolygons();
    emit geometryChanged();
}

void WalkAreaManager::reset()
{
    mWalkableArea.reset();
    mObstacles.clear();
    mNextObstacleId = 1;
    mEdit           = false;
    setDrawingType(walkarea::PolygonType::Walkable);
    validatePolygons();
    emit geometryChanged();
    emit imageUpdate();
    emit editModeChanged(false);
}

// i.e. height or status changes
void WalkAreaManager::updatePolygon(const walkarea::Polygon &poly, bool validate)
{
    if(mWalkableArea.has_value() && mWalkableArea->id() == poly.id())
    {
        mWalkableArea = poly;
        emit imageUpdate();
    }
    else if(mObstacles.find(poly.id()) != mObstacles.end())
    {
        mObstacles[poly.id()] = poly;
        emit imageUpdate();
    }

    if(validate)
    {
        validatePolygons();
    }
}

void WalkAreaManager::validatePolygons()
{
    mInvalidObstacleIds.clear();
    mIsValid = true;

    // Reset validation errors
    if(mWalkableArea.has_value())
    {
        mWalkableArea->setValidationError(walkarea::ValidationError::None);
    }
    for(auto &kv : mObstacles)
    {
        kv.second.setValidationError(walkarea::ValidationError::None);
    }

    std::vector<walkarea::Polygon *> enabledObstacles;
    for(auto &[key, obstacle] : mObstacles)
    {
        if(!obstacle.disabled())
        {
            enabledObstacles.emplace_back(&obstacle);
        }
    }

    // Collect enabled obstacle IDs and check if they're inside walkable area
    for(auto *obstacle : enabledObstacles)
    {
        // catches both obstacle being outside the walkarea and obstacle contains the walkarea
        // e.g. all vertices of the polygon have to be inside the walkarea
        if(!mWalkableArea.has_value() || mWalkableArea->disabled() || !obstacle->isInside(*mWalkableArea))
        {
            obstacle->setValidationError(walkarea::ValidationError::ObstacleOutside);
            mInvalidObstacleIds.insert(obstacle->id());
            SPDLOG_WARN("Invalid experiment geometry: obstacle {} is outside the walkable area.", obstacle->id());
        }
    }

    for(size_t i = 0; i < enabledObstacles.size(); ++i)
    {
        for(size_t j = i + 1; j < enabledObstacles.size(); ++j)
        {
            auto *obstacle1 = enabledObstacles[i];
            auto *obstacle2 = enabledObstacles[j];

            if(obstacle1->overlaps(*obstacle2))
            {
                obstacle1->setValidationError(walkarea::ValidationError::ObstaclesOverlapping);
                obstacle2->setValidationError(walkarea::ValidationError::ObstaclesOverlapping);
                mInvalidObstacleIds.insert(obstacle1->id());
                mInvalidObstacleIds.insert(obstacle2->id());
                SPDLOG_WARN(
                    "Invalid experiment geometry: obstacle {} and {} are overlapping.",
                    obstacle1->id(),
                    obstacle2->id());
            }
        }
    }

    mIsValid = mInvalidObstacleIds.empty();
    emit validationChanged();
}

void WalkAreaManager::setXml(QDomElement &elem) const
{
    QDomDocument doc = elem.ownerDocument();

    elem.setAttribute("VISIBLE", mVisible);
    elem.setAttribute("WALKABLE_VISIBLE", mWalkableVisible);
    elem.setAttribute("OBSTACLE_VISIBLE", mObstacleVisible);
    if(mParent->parent())
    {
        auto *parent = dynamic_cast<PGroupBox *>(mParent->parent()->parent());
        if(parent)
        {
            elem.setAttribute("IMMUTABLE", parent->isImmutable());
        }
    }

    if(mWalkableArea.has_value())
    {
        const auto &poly = *mWalkableArea;

        QVector<walkarea::Polygon> polygons;
        polygons.append(poly);

        QString wkt = walkarea::WKTParser::generatePixelWKT(polygons);

        QDomElement walkableElem = doc.createElement("WALKABLE");
        walkableElem.setAttribute("WKT", wkt);
        walkableElem.setAttribute("DISABLED", poly.disabled());
        elem.appendChild(walkableElem);
    }

    for(const auto &[id, poly] : mObstacles)
    {
        QVector<walkarea::Polygon> polygons;
        polygons.append(poly);

        QString wkt = walkarea::WKTParser::generatePixelWKT(polygons);

        QDomElement obstacleElem = doc.createElement("OBSTACLE");
        obstacleElem.setAttribute("ID", id);
        obstacleElem.setAttribute("WKT", wkt);
        obstacleElem.setAttribute("DISABLED", poly.disabled());
        elem.appendChild(obstacleElem);
    }
}

void WalkAreaManager::getXml(const QDomElement &elem)
{
    reset();

    setVisible(readBool(elem, "VISIBLE", false));
    if(mParent->parent())
    {
        auto *parent = dynamic_cast<PGroupBox *>(mParent->parent()->parent());
        if(parent)
        {
            parent->setImmutable(readBool(elem, "IMMUTABLE", false));
        }
    }

    setWalkableVisible(readBool(elem, "WALKABLE_VISIBLE", true));
    setObstacleVisible(readBool(elem, "WALKABLE_VISIBLE", true));
    for(QDomElement child = elem.firstChildElement(); !child.isNull(); child = child.nextSiblingElement())
    {
        if(child.tagName() == "WALKABLE" && child.hasAttribute("WKT"))
        {
            QString                    wkt      = readQString(child, "WKT");
            QVector<walkarea::Polygon> polygons = walkarea::WKTParser::parsePixelWKT(wkt);

            if(!polygons.isEmpty())
            {
                walkarea::Polygon poly = polygons.first();
                poly.setId(0);
                poly.setType(walkarea::PolygonType::Walkable);
                poly.setDisabled(readBool(child, "DISABLED", false));
                mWalkableArea = poly;
                SPDLOG_INFO("Walk area: loaded WALKABLE polygon with {} vertices", poly.size());
            }
        }
        else if(child.tagName() == "OBSTACLE" && child.hasAttribute("WKT") && child.hasAttribute("ID"))
        {
            QString wkt = readQString(child, "WKT");
            int     id  = readInt(child, "ID");

            QVector<walkarea::Polygon> polygons = walkarea::WKTParser::parsePixelWKT(wkt);

            if(!polygons.isEmpty())
            {
                walkarea::Polygon poly = polygons.first();
                poly.setId(id);
                poly.setType(walkarea::PolygonType::Obstacle);
                poly.setDisabled(readBool(child, "DISABLED", false));

                mObstacles.insert_or_assign(id, poly);
                SPDLOG_INFO("Walk area: loaded OBSTACLE polygon (id={}) with {} vertices", poly.id(), poly.size());

                // Update next obstacle ID to be larger than any loaded ID
                if(id >= mNextObstacleId)
                {
                    mNextObstacleId = id + 1;
                }
            }
        }
    }

    // Update drawing type based on loaded data
    if(mWalkableArea.has_value())
    {
        setDrawingType(walkarea::PolygonType::Obstacle);
    }
    else
    {
        setDrawingType(walkarea::PolygonType::Walkable);
    }

    // Validate loaded polygons
    validatePolygons();
    emit geometryChanged();
}
