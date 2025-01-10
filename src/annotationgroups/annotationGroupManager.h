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

#ifndef PETRACK_ANNOTATIONGROUPMANAGER_H
#define PETRACK_ANNOTATIONGROUPMANAGER_H

#include "annotationGrouping.h"
#include "colorList.h"

#include <QObject>
#include <map>
#include <set>
#include <string>
#include <vector>

class Petrack;
class Animation;
class PersonStorage;

/**
 * The GroupManager is the main class behind the Annotation group feature containing the business logic.
 *
 * It stores all existing groups and provides methods to add and remove trajectories to those groups.
 */
class AnnotationGroupManager : public QObject
{
    Q_OBJECT
private:
    Petrack         &mPetrack;
    const Animation &mAnimation;
    PersonStorage   &mPersonStorage;


    int                                            mNextGroupId = 1;
    std::map<int, annotationGroups::Group>         mGroups;
    std::map<int, annotationGroups::TopLevelGroup> mTopLevelGroups;
    std::map<int, ColorList>                       mDefaultColors;

    bool         mVisualization       = false;
    unsigned int mVisualizationRadius = 50;

public:
    AnnotationGroupManager(Petrack &petrack, Animation &animation, PersonStorage &personStorage) :
        mPetrack(petrack), mAnimation(animation), mPersonStorage(personStorage)
    {
        initDefaultGroups();
    }

    /**
     * Add a trajectory to a group at the current frame.
     *
     * @see addTrajectoryToGroup(size_t, grouping::Group&, int)
     */
    bool addTrajectoryToGroup(size_t trajectory, int groupId);

    /**
     * Add a trajectory to a group at a specific frame. It will keep this assignment, until a different one has been
     * set.
     * For example: Add trajectory 0 to Group A at frame 5, and to Group B at frame 15 results in:
     * No assignment for frames 0 to 4,
     * Group A for frames 5 to 14 and
     * Group B for frames 15 until the end.
     *
     * @param trajectory trajectory identified by the TrackPerson's position in the PersonStorage list.
     * @param group the group to add to
     * @param frame the starting frame, when the trajectory should be assigned with the group.
     * @return true, if adding was successfull
     */
    bool addTrajectoryToGroup(size_t trajectory, int groupId, int frame);

    bool removeTrajectoryAssignment(size_t trajectory);
    bool removeTrajectoryAssignment(size_t trajectory, int frame);


    void addGroupToTopLevelGroup(int groupId, int tlgId);
    void addGroupToTopLevelGroup(annotationGroups::Group &group, int tlgId);
    void addGroupToTopLevelGroup(annotationGroups::Group &group, annotationGroups::TopLevelGroup &tlg);

    /**
     * Get a List of entries of assignments of a group.
     * One entry consists of the TrackPersonID, start and end of the assigned interval.
     *
     */
    std::vector<annotationGroups::TrajectoryGroupEntry> getTrajectoriesOfGroup(int groupId) const;

    bool isValidGroupId(int id) const;

    /**
     * Get Group by id.
     * Return NO_GROUP for any invalid id.
     */
    annotationGroups::Group getGroup(int id) const;

    /**
     * Create new group based on a given group object.
     * The group will be checked for validity (e.g. no empty name).
     *
     * @param group the group to copy its data from. The Id field will be ignored
     * @return the id of the freshly created group. negative number if the group was not created.
     */
    int createGroup(annotationGroups::Group group);

    /**
     * Update a group. The Id of the given parameter will be used to determine the group to update
     * @param group The group that contains new data
     * @return true, if update was successfull, false else (e.g. invalid id)
     */
    bool updateGroup(const annotationGroups::Group &group);

    /**
     * Delete a group.
     * This will clean up all usages of the group first, by going through all TrackPersons in the person storage and
     * removing the groups from their assignment list.
     *
     * Removing the group of a TrackPerson is similar to inserting the NO_GROUP value in it i.e. it will create a gap.
     */
    void deleteGroup(int id);

    std::vector<annotationGroups::TopLevelGroup> getTopLevelGroups();
    std::vector<annotationGroups::Group>         getGroups();
    std::vector<annotationGroups::Group>         getGroupsOfTlg(int tlgId);

    bool isValidTopLevelGroupId(int id) const;

    /**
     * Get TopLevelGroup by id.
     * Throws error on invalid id
     */
    annotationGroups::TopLevelGroup getTopLevelGroupById(int id);

    /**
     * Save the current groups to a serializable GroupConfiguration object.
     */
    annotationGroups::GroupConfiguration saveConfig() const;

    /**
     * Load a GroupConfiguration object.
     *
     * This will overwrite all current data and delete it!
     */
    void loadConfig(const annotationGroups::GroupConfiguration &config);

    /**
     * Get a list of currently known types.
     * This is an accumulation over all distinct types that groups have.
     */
    std::set<std::string> getKnownTypes() const;

    void setVisualization(bool value);
    void setVisualizationRadius(unsigned int radius);

    bool         isVisualizationEnabled() const { return mVisualization; }
    unsigned int getVisualizationRadius() const { return mVisualizationRadius; }

    /**
     * Get the next color of a Top Level Group, if it has a specified color list.
     * @param id id of the top level group
     * @return next QColor in the correpsonding ColorList, fallback (default: Qt::transparent) if there is no such list.
     */
    QColor getNextTLGColor(int tlgId, QColor fallback = Qt::transparent) const;

signals:
    void groupsChanged();
    void trajectoryAssignmentChanged();
    void visualizationParameterChanged();

private:
    annotationGroups::Group &createGroup(int id, const std::string &name, const std::string &type);

    void initDefaultGroups();
};


#endif // PETRACK_ANNOTATIONGROUPMANAGER_H
