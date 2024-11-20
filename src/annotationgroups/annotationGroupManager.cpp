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
#include "annotationGroupManager.h"

#include "animation.h"
#include "logger.h"
#include "pMessageBox.h"
#include "personStorage.h"
#include "petrack.h"

#include <utility>

using namespace std;
using namespace annotationGroups;


bool AnnotationGroupManager::addTrajectoryToGroup(size_t trajectory, int groupId, int frame)
{
    if(!isValidGroupId(groupId))
    {
        return false;
    }
    if(trajectory >= mPersonStorage.nbPersons())
    {
        return false;
    }
    auto &list  = mPersonStorage.getGroupList(trajectory);
    auto &group = mGroups.at(groupId);

    list.insert(frame, group.id);

    emit trajectoryAssignmentChanged();
    return true;
}

bool AnnotationGroupManager::addTrajectoryToGroup(size_t trajectory, int groupId)
{
    const int frame = mAnimation.getCurrentFrameNum();
    return addTrajectoryToGroup(trajectory, groupId, frame);
}

bool AnnotationGroupManager::removeTrajectoryAssignment(size_t trajectory)
{
    const int frame = mAnimation.getCurrentFrameNum();
    return removeTrajectoryAssignment(trajectory, frame);
}

bool AnnotationGroupManager::removeTrajectoryAssignment(size_t trajectory, int frame)
{
    if(trajectory >= mPersonStorage.nbPersons())
    {
        return false;
    }

    auto &list = mPersonStorage.getGroupList(trajectory);
    list.insert(frame, NO_GROUP.id);
    emit trajectoryAssignmentChanged();
    return true;
}

void AnnotationGroupManager::addGroupToTopLevelGroup(int groupId, int tlgId)
{
    if(isValidGroupId(groupId) && isValidTopLevelGroupId(tlgId))
    {
        auto &grp = mGroups.at(groupId);
        if(grp.tlgId != tlgId)
        {
            grp.tlgId = tlgId;
            emit groupsChanged();
        }
    }
}

void AnnotationGroupManager::addGroupToTopLevelGroup(Group &group, TopLevelGroup &tlg)
{
    addGroupToTopLevelGroup(group.id, tlg.id);
}

void AnnotationGroupManager::addGroupToTopLevelGroup(Group &group, int tlgId)
{
    addGroupToTopLevelGroup(group.id, tlgId);
}

bool AnnotationGroupManager::isValidTopLevelGroupId(int id) const
{
    return mTopLevelGroups.find(id) != mTopLevelGroups.end();
}

TopLevelGroup AnnotationGroupManager::getTopLevelGroupById(int id)
{
    return mTopLevelGroups.at(id);
}

bool AnnotationGroupManager::isValidGroupId(int id) const
{
    return mGroups.find(id) != mGroups.end();
}

annotationGroups::Group AnnotationGroupManager::getGroup(int id) const
{
    if(isValidGroupId(id))
    {
        return mGroups.at(id);
    }
    return NO_GROUP;
}

std::vector<annotationGroups::TopLevelGroup> AnnotationGroupManager::getTopLevelGroups()
{
    std::vector<annotationGroups::TopLevelGroup> groups;
    groups.reserve(mTopLevelGroups.size());
    for(auto &tlg : mTopLevelGroups)
    {
        groups.push_back(tlg.second);
    }
    return groups;
}


annotationGroups::GroupConfiguration AnnotationGroupManager::saveConfig() const
{
    annotationGroups::GroupConfiguration config;

    std::vector<annotationGroups::Group>         groups;
    std::vector<annotationGroups::TopLevelGroup> tlgs;

    groups.reserve(mGroups.size());
    for(const auto &entry : mGroups)
    {
        groups.push_back(entry.second);
    }

    tlgs.reserve(mTopLevelGroups.size());
    for(const auto &entry : mTopLevelGroups)
    {
        tlgs.push_back(entry.second);
    }

    std::map<int, IntervalList<int> *> intervals;
    for(size_t i = 0; i < mPersonStorage.nbPersons(); i++)
    {
        intervals[(int) i] = &mPersonStorage.getGroupList(i);
    }

    config.fillData(tlgs, groups, intervals);

    return config;
}

void AnnotationGroupManager::loadConfig(const annotationGroups::GroupConfiguration &config)
{
    blockSignals(true);
    mGroups.clear();
    mTopLevelGroups.clear();
    mNextGroupId = config.getNextGroupId();


    for(size_t i = 0; i < mPersonStorage.nbPersons(); i++)
    {
        mPersonStorage.getGroupList(i).clear();
    }

    for(const auto &entry : config.mTopLevelGroups)
    {
        mTopLevelGroups[entry.id] = entry;
    }

    for(const auto &entry : config.mGroups)
    {
        auto &group = createGroup(entry.id, entry.name, entry.type);
        group.color = entry.color;
        group.tlgId = entry.tlgId;
    }

    std::vector<int> trajectoryErrors;

    for(const auto &entry : config.mTrajectoryAssignments)
    {
        int trajectoryId = entry.first;
        if(trajectoryId >= (int) mPersonStorage.nbPersons())
        {
            SPDLOG_WARN(
                "Could not load group assignment for trajectory {}. Trajectory does not exist. Skipping", trajectoryId);
            trajectoryErrors.push_back(trajectoryId);
            continue;
        }
        std::vector<std::tuple<int, int>> intervals = entry.second;

        // sort ascending by startFrame to prevent accidental skipping entries due to IntervalList::insert.
        std::sort(intervals.begin(), intervals.end(), [](auto a, auto b) { return std::get<0>(a) < std::get<0>(b); });

        for(const auto &intervalEntry : intervals)
        {
            int groupId = std::get<1>(intervalEntry);
            int frame   = std::get<0>(intervalEntry);
            if(!addTrajectoryToGroup(trajectoryId, groupId, frame))
            {
                removeTrajectoryAssignment(trajectoryId, frame);
            }
        }
    }

    if(!trajectoryErrors.empty())
    {
        PWarning(
            mPetrack.parentWidget(),
            "Could not fully load group configuration",
            "Some trajectories were not able to be loaded since they do not exist. Import the .trc file before "
            "importing "
            "group configuration.");
    }

    blockSignals(false);
    emit groupsChanged();
}

std::vector<annotationGroups::Group> AnnotationGroupManager::getGroups()
{
    std::vector<annotationGroups::Group> groups;
    groups.reserve(mGroups.size());
    for(auto &pair : mGroups)
    {
        groups.push_back(pair.second);
    }
    return groups;
}
annotationGroups::Group &AnnotationGroupManager::createGroup(int id, const string &name, const string &type)
{
    // check if id already exists
    if(mGroups.find(id) != mGroups.end())
    {
        throw std::invalid_argument("Group id already exists");
    }

    Group grp(id, name, type);

    mGroups[id] = grp;

    emit groupsChanged();
    return mGroups.at(id);
}

int AnnotationGroupManager::createGroup(annotationGroups::Group group)
{
    if(group.name.empty())
    {
        return -1;
    }

    // increase next Id as long as it is already in use (to be safe)
    while(mGroups.find(mNextGroupId) != mGroups.end())
    {
        mNextGroupId++;
    }

    if(group.tlgId >= 0)
    {
        if(isValidTopLevelGroupId(group.tlgId))
        {
            if(group.color == getNextTLGColor(group.tlgId))
            {
                mDefaultColors.at(group.tlgId).next();
            }
        }
        else
        {
            // NO tlg
            group.tlgId = -1;
        }
    }

    mGroups.insert({mNextGroupId, group});
    mGroups.at(mNextGroupId).id = mNextGroupId;
    mNextGroupId++;


    emit groupsChanged();

    return mNextGroupId - 1;
}

void AnnotationGroupManager::setVisualization(bool value)
{
    if(mVisualization != value)
    {
        mVisualization = value;
        emit visualizationParameterChanged();
    }
}

void AnnotationGroupManager::setVisualizationRadius(unsigned int radius)
{
    if(mVisualizationRadius != radius)
    {
        mVisualizationRadius = radius;
        emit visualizationParameterChanged();
    }
}

QColor AnnotationGroupManager::getNextTLGColor(int id, QColor fallback) const
{
    if(mDefaultColors.find(id) != mDefaultColors.end())
    {
        return mDefaultColors.at(id).peekNext();
    }
    return fallback;
}

void AnnotationGroupManager::initDefaultGroups()
{
    mTopLevelGroups = {{0, annotationGroups::TLG1}, {1, annotationGroups::TLG2}, {2, annotationGroups::TLG3}};
    mDefaultColors.insert({0, ColorList::red()});
    mDefaultColors.insert({1, ColorList::yellow()});
    mDefaultColors.insert({2, ColorList::green()});
}

std::vector<annotationGroups::TrajectoryGroupEntry> AnnotationGroupManager::getTrajectoriesOfGroup(int groupId) const
{
    std::vector<annotationGroups::TrajectoryGroupEntry> trajectories;

    for(size_t i = 0; i < mPersonStorage.nbPersons(); i++)
    {
        const auto &groupList = mPersonStorage.getGroupList(i);
        const auto &entries   = groupList.getEntries();

        for(size_t k = 0; k < entries.size() - 1; ++k)
        {
            const auto &entry = entries.at(k);

            if(entry.data == groupId)
            {
                const auto &next = entries.at(k + 1);
                trajectories.push_back({groupId, (int) i, entry.start, next.start - 1});
            }
        }
        if(entries.back().data == groupId)
        {
            trajectories.push_back({groupId, (int) i, entries.back().start, -1});
        }
    }

    return trajectories;
}
void AnnotationGroupManager::deleteGroup(int id)
{
    // early return when group is not present to prevent calculation and signal
    if(id < 0 || !isValidGroupId(id))
    {
        return;
    }

    auto             trajectories = getTrajectoriesOfGroup(id);
    std::set<size_t> people;
    for(auto &entry : trajectories)
    {
        auto      &personList = mPersonStorage.getGroupList(entry.trackPersonId);
        const auto frame      = entry.frameBegin;

        personList.remove(frame);
        people.insert(entry.trackPersonId);
    }

    mGroups.erase(id);

    for(const auto &p : people)
    {
        mPersonStorage.getGroupList(p).compact();
    }
    emit groupsChanged();
}
std::vector<annotationGroups::Group> AnnotationGroupManager::getGroupsOfTlg(int tlgId)
{
    std::vector<annotationGroups::Group> groups;

    for(auto &entry : mGroups)
    {
        if(entry.second.tlgId == tlgId)
        {
            groups.push_back(entry.second);
        }
    }

    return groups;
}
bool AnnotationGroupManager::updateGroup(const Group &group)
{
    if(isValidGroupId(group.id))
    {
        auto &grp = mGroups.at(group.id);

        grp.name  = group.name;
        grp.type  = group.type;
        grp.color = group.color;
        grp.tlgId = group.tlgId;

        emit groupsChanged();

        return true;
    }

    return false;
}
std::set<std::string> AnnotationGroupManager::getKnownTypes() const
{
    // No type should be inside by default
    std::set<std::string> types{"No Type"};

    for(const auto &groupEntry : mGroups)
    {
        const auto &group = groupEntry.second;
        types.insert(group.type);
    }

    return types;
}
