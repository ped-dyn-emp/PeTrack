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

#ifndef PETRACK_ANNOTATIONGROUPING_H
#define PETRACK_ANNOTATIONGROUPING_H

#include "intervalList.h"

#include <QColor>
#include <map>
#include <string>
#include <vector>


namespace annotationGroups
{

struct Group
{
    int         id;
    std::string name;
    std::string type;
    QColor      color = Qt::transparent;
    int         tlgId = -1;

    Group() = default;
    Group(const std::string &name, const std::string &type) : Group(-1, name, type, -1) {}
    Group(int id, const std::string &name, const std::string &type, int tlgId = -1) :
        id(id), name(name), type(type), tlgId(tlgId)
    {
    }
};

extern Group NO_GROUP;

struct TopLevelGroup
{
    int         id;
    std::string name;
};

struct TrajectoryGroupEntry
{
    int groupId;
    int trackPersonId;
    int frameBegin;
    int frameEnd;
};


static TopLevelGroup TLG1{0, "Large Action Groups"};
static TopLevelGroup TLG2{1, "Small Action Groups"};
static TopLevelGroup TLG3{2, "Individuals"};


/**
 * Struct used for serializing group information. Can be saved and loaded to/from a file.
 */
struct GroupConfiguration
{
    std::string version = "0.2";

    std::vector<Group>         mGroups;
    std::vector<TopLevelGroup> mTopLevelGroups;

    // mapping from TrackPersonId to list of (StartFrame, GroupId) assignments
    // map<TrackPersonId, vector<(StartFrame, GroupId)>>
    std::map<int, std::vector<std::tuple<int, int>>> mTrajectoryAssignments;

    /**
     * Fill the data structures with given lists
     * @param tlgs a list of used TopLevelGroups
     * @param groups a list of used Groups
     * @param intervalLists a map, mapping a TrackPersonId to its corresponding IntervalList pointer.
     */
    void fillData(
        const std::vector<TopLevelGroup>         &tlgs,
        const std::vector<Group>                 &groups,
        const std::map<int, IntervalList<int> *> &intervalLists);


    /**
     * Get the next groupId that can be used safely after loading this configuration, that is the highest used Id + 1.
     */
    int getNextGroupId() const;
};

} // namespace annotationGroups

#endif // PETRACK_ANNOTATIONGROUPING_H
