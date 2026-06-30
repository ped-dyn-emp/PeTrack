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

#ifndef WALKAREAMANAGER_H
#define WALKAREAMANAGER_H

#include "polygonSelection.h"
#include "util/polygon.h"

#include <QObject>
#include <map>
#include <optional>
#include <set>

class QDomElement;
class WorldImageCorrespondence;
class WalkAreaWidget;

class WalkAreaManager : public QObject
{
    Q_OBJECT
public:
    WalkAreaManager(WalkAreaWidget *parent);

    void setVisible(bool v);
    void setWalkableVisible(bool v);
    void setObstacleVisible(bool v);
    bool isVisible() const { return mVisible; }
    bool isWalkableVisible() const { return mWalkableVisible; }
    bool isObstacleVisible() const { return mObstacleVisible; }

    void setEditMode(bool v);
    bool isEditMode() const { return mEdit; }

    void                  setDrawingType(walkarea::PolygonType t);
    walkarea::PolygonType getDrawingType() const { return mCurrentType; }

    void addPolygon(const QVector<QPointF> &points, walkarea::PolygonType type);
    void removePolygon(int id);
    void reset();
    void updatePolygon(const walkarea::Polygon &poly, bool validate = false);

    bool                                    hasWalkableArea() const { return mWalkableArea.has_value(); }
    std::optional<walkarea::Polygon>        getWalkableArea() const { return mWalkableArea; }
    const std::map<int, walkarea::Polygon> &getObstacles() const { return mObstacles; }

    PolygonSelection &selection() { return mSelection; }

    bool          isValid() const { return mIsValid; }
    std::set<int> getInvalidObstacleIds() const { return mInvalidObstacleIds; }

    void setXml(QDomElement &elem) const;
    void getXml(const QDomElement &elem);

signals:
    void visibilityChanged(bool visible, bool walkableVisible, bool obstacleVisible);
    void editModeChanged(bool);
    void geometryChanged();
    void imageUpdate();
    void drawingTypeChanged(walkarea::PolygonType);
    void validationChanged();

private:
    void ensureNextObstacleIdFree();
    void validatePolygons();

    WalkAreaWidget       *mParent;
    bool                  mVisible         = false;
    bool                  mWalkableVisible = true;
    bool                  mObstacleVisible = true;
    bool                  mEdit            = false;
    walkarea::PolygonType mCurrentType     = walkarea::PolygonType::Walkable;
    PolygonSelection      mSelection;

    std::optional<walkarea::Polygon> mWalkableArea;
    std::map<int, walkarea::Polygon> mObstacles;
    int                              mNextObstacleId = 1;

    std::set<int> mInvalidObstacleIds;
    bool          mIsValid = true;
};
#endif
