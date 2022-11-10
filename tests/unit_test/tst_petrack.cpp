/*
 * PeTrack - Software for tracking pedestrians movement in videos
 * Copyright (C) 2022 Forschungszentrum Jülich GmbH, IAS-7
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

#include "petrack.h"

#include <catch2/catch.hpp>

TEST_CASE("Petrack version format")
{
    QString versionString{PETRACK_VERSION};
    // Major.Minor.Patch => 3 version parts
    constexpr int numberVersionParts = 3;
    CHECK(versionString.split(".").size() == numberVersionParts);
}

SCENARIO("Getting the IDs of the pedestrian from user input", "[petrack][util]")
{
    WHEN("Enter valid pedestrian ID filter")
    {
        AND_WHEN("single values")
        {
            QSet<int>         expectedIDs{1, 4, 6, 7};
            std::stringstream input;
            std::copy(expectedIDs.begin(), expectedIDs.end(), std::ostream_iterator<int>(input, ","));

            auto receivedIDs = util::splitStringToInt(QString(input.str().c_str()));

            THEN("the entered ids should be returned")
            {
                REQUIRE(receivedIDs.has_value());
                REQUIRE(expectedIDs == receivedIDs.value());
            }
        }
        AND_WHEN("range")
        {
            QSet<int> expectedIDs{1, 2, 3, 4};
            QString   input("1-4");

            auto receivedIDs = util::splitStringToInt(input);
            THEN("the entered ids should be returned")
            {
                REQUIRE(receivedIDs.has_value());
                REQUIRE(expectedIDs == receivedIDs.value());
            }
        }
        AND_WHEN("range (reverse order)")
        {
            QSet<int> expectedIDs{1, 2, 3, 4};
            QString   input("4-1");

            auto receivedIDs = util::splitStringToInt(input);
            THEN("the entered ids should be returned")
            {
                REQUIRE(receivedIDs.has_value());
                REQUIRE(expectedIDs == receivedIDs.value());
            }
        }
        AND_WHEN("range + single values")
        {
            QSet<int> expectedIDs{1, 2, 3, 4, 5, 8, 10};
            QString   input("1-5,8,10");

            auto receivedIDs = util::splitStringToInt(input);
            THEN("the entered ids should be returned")
            {
                REQUIRE(receivedIDs.has_value());
                REQUIRE(expectedIDs == receivedIDs.value());
            }
        }
        AND_WHEN("single values + range")
        {
            QSet<int> expectedIDs{1, 3, 5, 8, 9, 10, 11};
            QString   input("1,3,5,8-11");

            auto receivedIDs = util::splitStringToInt(input);
            THEN("the entered ids should be returned")
            {
                REQUIRE(receivedIDs.has_value());
                REQUIRE(expectedIDs == receivedIDs.value());
            }
        }
        AND_WHEN("single values + range (with spaces")
        {
            QSet<int> expectedIDs{1, 3, 5, 8, 9, 10, 11};
            QString   input("1,  3 ,5, 8 -11");

            auto receivedIDs = util::splitStringToInt(input);
            THEN("the entered ids should be returned")
            {
                REQUIRE(receivedIDs.has_value());
                REQUIRE(expectedIDs == receivedIDs.value());
            }
        }
        AND_WHEN("duplicate values")
        {
            QSet<int> expectedIDs{1, 2, 3, 4};
            QString   input("1-4,1");

            auto receivedIDs = util::splitStringToInt(input);

            THEN("the entered ids should be returned")
            {
                REQUIRE(receivedIDs.has_value());
                REQUIRE(expectedIDs == receivedIDs.value());
            }
        }
        AND_WHEN("range same start and end")
        {
            QSet<int> expectedIDs{1};
            QString   input("1-1");

            auto receivedIDs = util::splitStringToInt(input);

            THEN("the entered ids should be returned")
            {
                REQUIRE(receivedIDs.has_value());
                REQUIRE(expectedIDs == receivedIDs.value());
            }
        }
    }

    WHEN("Enter invalid pedestrian ID filter")
    {
        AND_WHEN("negative ID")
        {
            QString input("-1");
            auto    receivedIDs = util::splitStringToInt(input);

            THEN("std::nullopt should be returned")
            {
                REQUIRE_FALSE(receivedIDs.has_value());
            }
        }
        AND_WHEN("invalid range")
        {
            QString input("1-");
            auto    receivedIDs = util::splitStringToInt(input);

            THEN("std::nullopt should be returned")
            {
                REQUIRE_FALSE(receivedIDs.has_value());
            }
        }
        AND_WHEN("too many -'s")
        {
            QString input("1-2-");
            auto    receivedIDs = util::splitStringToInt(input);

            THEN("std::nullopt should be returned")
            {
                REQUIRE_FALSE(receivedIDs.has_value());
            }
        }
        AND_WHEN("not int values (single values)")
        {
            QString input("1, 5, a, b, 6");
            auto    receivedIDs = util::splitStringToInt(input);

            THEN("std::nullopt should be returned")
            {
                REQUIRE_FALSE(receivedIDs.has_value());
            }
        }
    }
}
