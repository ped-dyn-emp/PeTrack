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


#include "annotationGroupManager.h"
#include "annotationGrouping.h"
#include "logger.h"
#include "petrack.h"

#include <QColor>
#include <catch2/catch.hpp>

using namespace annotationGroups;

TEST_CASE("import GroupConfiguration", "[grouping]")
{
    Petrack                petrack{"grouping Test"};
    AnnotationGroupManager manager{petrack, *petrack.getAnimation(), petrack.getPersonStorage()};


    petrack.getPersonStorage().addPerson({0, 0, {{0, 0}}});
    petrack.getPersonStorage().addPerson({1, 0, {{0, 0}}});

    const std::vector<Group> groups = {
        {1, "one", "type", 0}, {2, "two", "type", 0}, {3, "three", "type", 0}, {5, "four", "type", 1}};
    const std::vector<TopLevelGroup>                       tlgs         = {{0, "LAG"}, {1, "SAG"}};
    const std::map<int, std::vector<std::tuple<int, int>>> trajectories = {
        {0, {{0, 1}, {5, 2}}}, {1, {{0, 3}, {3, -1}}}};

    const GroupConfiguration config{"0.2", groups, tlgs, trajectories};

    manager.loadConfig(config);

    CHECK(4 == manager.getGroups().size());
    CHECK(2 == manager.getTopLevelGroups().size());

    CHECK(manager.isValidGroupId(1));
    CHECK(manager.isValidGroupId(2));
    CHECK(manager.isValidGroupId(3));
    CHECK(!manager.isValidGroupId(4)); // skipped
    CHECK(manager.isValidGroupId(5));

    CHECK(manager.isValidTopLevelGroupId(0));
    CHECK(manager.isValidTopLevelGroupId(1));

    CHECK(3 == manager.getGroupsOfTlg(0).size());
    CHECK(1 == manager.getGroupsOfTlg(1).size());

    CHECK(1 == manager.getTrajectoriesOfGroup(1).size());
    CHECK(1 == manager.getTrajectoriesOfGroup(2).size());
    CHECK(1 == manager.getTrajectoriesOfGroup(3).size());
    CHECK(manager.getTrajectoriesOfGroup(5).empty());

    // check trajectory assignments
    auto &list0 = petrack.getPersonStorage().getGroupList(0);
    CHECK(1 == list0.getValue(0));
    CHECK(1 == list0.getValue(4));
    CHECK(2 == list0.getValue(5));
    CHECK(2 == list0.getValue(10));

    auto &list1 = petrack.getPersonStorage().getGroupList(1);
    CHECK(3 == list1.getValue(0));
    CHECK(3 == list1.getValue(2));
    CHECK(NO_GROUP.id == list1.getValue(3));
    CHECK(NO_GROUP.id == list1.getValue(10));

    // check equality with freshly exported config
    auto exported = manager.saveConfig();

    CHECK(4 == exported.mGroups.size());
    CHECK(2 == exported.mTopLevelGroups.size());
    REQUIRE(2 == exported.mTrajectoryAssignments.size());

    // assignments TrackPerson 0
    auto vector = exported.mTrajectoryAssignments[0];
    REQUIRE(2 == vector.size());
    REQUIRE(std::make_tuple(0, 1) == vector.at(0));
    REQUIRE(std::make_tuple(5, 2) == vector.at(1));


    // assignments TrackPerson 1
    vector = exported.mTrajectoryAssignments[1];
    REQUIRE(2 == exported.mTrajectoryAssignments[1].size());
    REQUIRE(std::make_tuple(0, 3) == vector.at(0));
    REQUIRE(std::make_tuple(3, -1) == vector.at(1));
}

TEST_CASE("group basic creation and deletion", "[grouping]")
{
    Petrack                petrack{"grouping Test"};
    AnnotationGroupManager manager{petrack, *petrack.getAnimation(), petrack.getPersonStorage()};

    // Initialization
    // The manager should be initialized with:
    // Top level groups: 3 predefined ones
    // Groups: NO_GROUP with id -1 and no assigned top level
    const auto topLevel = manager.getTopLevelGroups();
    REQUIRE(3 == (topLevel.size()));

    const auto groups = manager.getGroups();
    REQUIRE(0 == (groups.size()));

    const auto types = manager.getKnownTypes();
    REQUIRE(1 == (types.size()));

    SECTION("create group")
    {
        Group group(-1, "one", "type");
        int   id = manager.createGroup(group);

        CHECK(1 == (manager.getGroups().size()));
    }

    SECTION("assign group to Top Level Group")
    {
        auto groupId = manager.createGroup({"one", "type"});
        auto group   = manager.getGroup(groupId);
        manager.addGroupToTopLevelGroup(groupId, 0);
        group = manager.getGroup(groupId);

        CHECK(0 == (group.tlgId));
        CHECK(1 == (manager.getGroupsOfTlg(0).size()));
    }

    SECTION("multiple groups")
    {
        // create some groups
        auto groupOneId   = manager.createGroup({"one", "type 1"});
        auto groupOne     = manager.getGroup(groupOneId);
        auto groupTwoId   = manager.createGroup({"two", "type 2"});
        auto groupTwo     = manager.getGroup(groupTwoId);
        auto groupThreeId = manager.createGroup({"three", "type 1"}); // same type as 1
        auto groupThree   = manager.getGroup(groupThreeId);
        {
            const auto groups = manager.getGroups();
            REQUIRE(3 == (groups.size()));

            const auto types = manager.getKnownTypes();
            REQUIRE(3 == (types.size()));
        }

        // add to top level groups
        manager.addGroupToTopLevelGroup(groupOne, 0);
        manager.addGroupToTopLevelGroup(groupTwo, 0);
        manager.addGroupToTopLevelGroup(groupThree, 1);
        // update values
        groupOne   = manager.getGroup(groupOneId);
        groupTwo   = manager.getGroup(groupTwoId);
        groupThree = manager.getGroup(groupThreeId);

        {
            REQUIRE(0 == (groupOne.tlgId));
            REQUIRE(0 == (groupTwo.tlgId));
            REQUIRE(1 == (groupThree.tlgId));

            const auto groupsOfTlg0 = manager.getGroupsOfTlg(0);
            REQUIRE(2 == (groupsOfTlg0.size()));
        }

        // removal
        // dangling reference after deletion?
        int id = groupOne.id;
        manager.deleteGroup(id);
        {
            REQUIRE(!manager.isValidGroupId(id));
            REQUIRE(1 == (manager.getGroupsOfTlg(0).size()));
        }
    }
}

TEST_CASE("creating new types are registered", "[grouping]")
{
    Petrack                petrack{"grouping Test"};
    AnnotationGroupManager manager{petrack, *petrack.getAnimation(), petrack.getPersonStorage()};

    REQUIRE(1 == manager.getKnownTypes().size());

    manager.createGroup({"no change", "No Type"});
    manager.createGroup({"plus one", "one"});
    manager.createGroup({"second", "two"});

    REQUIRE(3 == manager.getKnownTypes().size());
}

TEST_CASE("group trajectory assignment operations", "[grouping]")
{
    Petrack                petrack{"grouping Test"};
    AnnotationGroupManager manager{petrack, *petrack.getAnimation(), petrack.getPersonStorage()};

    // create some groups
    auto groupOneId = manager.createGroup({"one", "type 1"});
    auto groupOne   = manager.getGroup(groupOneId);
    auto groupTwoId = manager.createGroup({"two", "type 2"});
    auto groupTwo   = manager.getGroup(groupTwoId);


    manager.addGroupToTopLevelGroup(groupOne, 0);
    manager.addGroupToTopLevelGroup(groupTwo, 0);

    REQUIRE(2 == (manager.getGroups().size()));
    REQUIRE(3 == (manager.getTopLevelGroups().size()));
    REQUIRE(2 == (manager.getGroupsOfTlg(0).size()));


    auto &personStorage = petrack.getPersonStorage();
    // create sample trajectories
    TrackPerson tp1(1, 0, {{0., 0.}});
    REQUIRE(tp1.insertAtFrame(10, {{10., 10.}}, 0, true));

    TrackPerson tp2(2, 0, {{10, 10}});
    REQUIRE(tp2.insertAtFrame(10, {{15, 15}}, 2, true));

    personStorage.addPerson(tp1);
    personStorage.addPerson(tp2);

    for(size_t i = 0; i < personStorage.nbPersons(); ++i)
    {
        REQUIRE(personStorage.getGroupList(i).empty());
    }

    // actual insertion
    REQUIRE(manager.addTrajectoryToGroup(0, groupOneId, 2));
    REQUIRE(manager.addTrajectoryToGroup(1, groupTwoId, 0));
    REQUIRE(manager.addTrajectoryToGroup(1, groupOneId, 5));
    manager.removeTrajectoryAssignment(1, 10);
    REQUIRE(manager.addTrajectoryToGroup(1, groupOneId, 16));

    {
        auto &list = personStorage.getGroupList(0);
        CHECK(1 == (list.size()));
        CHECK((NO_GROUP.id) == (list.getValue(0)));
        CHECK((NO_GROUP.id) == (list.getValue(1)));
        CHECK((groupOneId) == (list.getValue(2)));
        CHECK((groupOneId) == (list.getValue(5)));
        CHECK((groupOneId) == (list.getValue(10)));
        CHECK((groupOneId) == (list.getValue(20)));
    }
    {
        auto &list = personStorage.getGroupList(1);
        CHECK(4 == (list.size()));
        CHECK((groupTwoId) == (list.getValue(0)));
        CHECK((groupTwoId) == (list.getValue(4)));
        CHECK((groupOneId) == (list.getValue(5)));
        CHECK((groupOneId) == (list.getValue(9)));
        CHECK((NO_GROUP.id) == (list.getValue(10)));
        CHECK((NO_GROUP.id) == (list.getValue(15)));
        CHECK((groupOneId) == (list.getValue(16)));
        CHECK((groupOneId) == (list.getValue(20)));
    }
    {
        const auto &trajectories = manager.getTrajectoriesOfGroup(groupOne.id);
        REQUIRE(3 == (trajectories.size()));
    }
}

TEST_CASE("automatic group color working properly", "[grouping]")
{
    Petrack                petrack{"grouping Test"};
    AnnotationGroupManager manager{petrack, *petrack.getAnimation(), petrack.getPersonStorage()};

    REQUIRE(3 == manager.getTopLevelGroups().size());

    const auto  tlg      = manager.getTopLevelGroups().at(0);
    const auto &oldColor = manager.getNextTLGColor(tlg.id);

    Group group{"group", "type"};
    group.color = oldColor;
    group.tlgId = tlg.id;

    manager.createGroup(group);

    const auto &newColor = manager.getNextTLGColor(tlg.id);

    CHECK(newColor != oldColor);

    // creating again with a different to NextTLGColor should not change the color
    manager.createGroup(group);
    const auto &newnewColor = manager.getNextTLGColor(tlg.id);
    CHECK(newnewColor == newColor);
}

TEST_CASE("deleting group cleans up assignments", "[grouping]")
{
    Petrack                petrack{"grouping Test"};
    AnnotationGroupManager manager{petrack, *petrack.getAnimation(), petrack.getPersonStorage()};

    auto groupOneId = manager.createGroup({"one", "type 1"});
    auto groupOne   = manager.getGroup(groupOneId);
    manager.addGroupToTopLevelGroup(groupOne, 0);

    TrackPerson tp1(1, 0, {{0, 0}});
    tp1.insertAtFrame(10, {{10, 10}}, 1, true);
    petrack.getPersonStorage().addPerson(tp1);

    manager.addTrajectoryToGroup(0, groupOneId, 0);

    REQUIRE(1 == manager.getGroupsOfTlg(0).size());
    REQUIRE(groupOneId == petrack.getPersonStorage().getGroupList(0).getValue(0));
    REQUIRE(groupOneId == petrack.getPersonStorage().getGroupList(0).getValue(10));


    // deleting group cleans up assignments
    manager.deleteGroup(groupOne.id);
    CHECK(0 == manager.getGroupsOfTlg(0).size());
    CHECK(NO_GROUP.id == petrack.getPersonStorage().getGroupList(0).getValue(0));
}