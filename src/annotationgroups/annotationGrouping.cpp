/*
 * PeTrack - Software for tracking pedestrians movement in videos
 * Copyright (C) 2024 Forschungszentrum Jülich GmbH, IAS-7
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

#include "annotationGrouping.h"


using namespace annotationGroups;

Group annotationGroups::NO_GROUP = Group{-1, "no group", ""};

namespace annotationGroups
{
void GroupConfiguration::fillData(
    const std::vector<TopLevelGroup>         &tlgs,
    const std::vector<Group>                 &groups,
    const std::map<int, IntervalList<int> *> &intervalLists)
{
    mGroups         = groups;
    mTopLevelGroups = tlgs;

    for(const auto &entry : intervalLists)
    {
        int                               trackPersonId = entry.first;
        std::vector<std::tuple<int, int>> assignments;
        for(const auto &interval : entry.second->getEntries())
        {
            assignments.push_back({interval.start, interval.data});
        }

        mTrajectoryAssignments[trackPersonId] = assignments;
    }
}

int GroupConfiguration::getNextGroupId() const
{
    int max = 0;
    for(const auto &group : mGroups)
    {
        if(group.id > max)
        {
            max = group.id + 1;
        }
    }
    return max;
}
} // namespace annotationGroups