/*
 * PeTrack - Software for tracking pedestrians movement in videos
 * Copyright (C) 2023 Forschungszentrum Jülich GmbH, IAS-7
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

#include "tracker.h"

#include <catch2/catch.hpp>

TEST_CASE("TrackPerson returns correct frame range", "[TrackPerson]")
{
    TrackPoint              startPoint{{-3., 3.}};
    std::vector<TrackPoint> trackPoints = {
        TrackPoint({-2., 2.}), TrackPoint({-1, 1.}), TrackPoint({0., 0.}), TrackPoint({1, -1.}), TrackPoint({2, 2.})};

    int startFrame = 10;

    TrackPerson trackPerson{0, startFrame, startPoint};
    for(auto const &trackPoint : trackPoints)
    {
        trackPerson.append(trackPoint);
    }
    int lastFrame = startFrame + trackPoints.size();

    REQUIRE(trackPerson.firstFrame() == startFrame);
    REQUIRE(trackPerson.lastFrame() == lastFrame);

    REQUIRE_FALSE(trackPerson.trackPointExist(startFrame - 1));
    REQUIRE(trackPerson.trackPointExist(startFrame));
    REQUIRE(trackPerson.trackPointExist(static_cast<int>((lastFrame + startFrame) / 2.)));
    REQUIRE(trackPerson.trackPointExist(lastFrame));
    REQUIRE_FALSE(trackPerson.trackPointExist(lastFrame + 1));
}

TEST_CASE("TrackPerson ranges can be removed", "[TrackPerson]")
{
    TrackPoint        startPoint{{-3., 3.}};
    QList<TrackPoint> trackPoints = {
        TrackPoint({-2., 2.}), TrackPoint({-1, 1.}), TrackPoint({0., 0.}), TrackPoint({1, -1.}), TrackPoint({2, 2.})};

    int startFrame = 10;

    TrackPerson trackPerson{0, startFrame, startPoint};
    for(auto const &trackPoint : trackPoints)
    {
        trackPerson.append(trackPoint);
    }

    // To make comparisons easier
    trackPoints.prepend(startPoint);
    int lastFrame = trackPerson.lastFrame();

    SECTION("ranges at beginning can be removed")
    {
        int startOffset   = 3;
        int newStartFrame = startFrame + startOffset;

        trackPerson.removeFramesBetween(startFrame, newStartFrame - 1);

        for(int i = 0; i < trackPerson.size(); ++i)
        {
            REQUIRE(trackPerson.trackPointAt(newStartFrame + i) == trackPoints[i + startOffset]);
        }

        REQUIRE(trackPerson.firstFrame() == newStartFrame);
        REQUIRE(trackPerson.lastFrame() == lastFrame);
    }

    SECTION("ranges at end can be removed")
    {
        int endOffset = 2;

        int newLastFrame = lastFrame - endOffset;

        trackPerson.removeFramesBetween(newLastFrame + 1, lastFrame);

        for(int i = 0; i < trackPerson.size(); ++i)
        {
            REQUIRE(trackPerson.trackPointAt(startFrame + i) == trackPoints[i]);
        }

        REQUIRE(trackPerson.firstFrame() == startFrame);
        REQUIRE(trackPerson.lastFrame() == newLastFrame);

        REQUIRE_FALSE(trackPerson.trackPointExist(startFrame - 1));
        REQUIRE(trackPerson.trackPointExist(startFrame));
        REQUIRE(trackPerson.trackPointExist(newLastFrame));
        REQUIRE_FALSE(trackPerson.trackPointExist(newLastFrame + 1));
    }

    SECTION("range covering non existing frames can not be removed")
    {
        REQUIRE_THROWS_AS(trackPerson.removeFramesBetween(startFrame - 1, startFrame + 1), std::out_of_range);
        REQUIRE_THROWS_AS(trackPerson.removeFramesBetween(lastFrame - 1, lastFrame + 1), std::out_of_range);
        REQUIRE_THROWS_AS(trackPerson.removeFramesBetween(startFrame - 1, lastFrame + 1), std::out_of_range);
    }

    SECTION("ranges leaving frames in the middle can not be removed")
    {
        REQUIRE_THROWS_AS(trackPerson.removeFramesBetween(startFrame + 1, lastFrame - 1), std::range_error);
    }
}