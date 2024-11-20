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


#include "intervalList.h"
#include "logger.h"

#include <catch2/catch.hpp>

TEST_CASE("src/groups/intervalList", "[groups]")
{
    int               undefValue = 0;
    IntervalList<int> list{undefValue};

    SECTION("Simple insert")
    {
        list.insert(0, 10);

        CHECK(10 == list.getValue(0));
        CHECK(10 == list.getValue(5));
        CHECK(10 == list.getValue(9));

        CHECK(0 == list.getMinimum());
        // outside region
        CHECK(0 == list.getValue(-1));
        CHECK(1 == list.size());
        CHECK(false == list.empty());
    }

    SECTION("Multiple inserts")
    {
        REQUIRE(0 == list.size());
        REQUIRE(true == list.empty());

        int a = 10, b = 20;
        int c = 30;
        SECTION("replacing")
        {
            list.insert(1, a);
            list.insert(10, b);

            SPDLOG_DEBUG("LIST: {}", list.toString());

            CHECK(0 == list.getValue(0));
            CHECK(a == list.getValue(1));
            CHECK(a == list.getValue(9));
            CHECK(b == list.getValue(10));
            CHECK(b == list.getValue(50));
            CHECK(1 == list.getMinimum());


            // replace value
            list.insert(10, c);
            REQUIRE(c == list.getValue(10));
            REQUIRE(c == list.getValue(50));
            REQUIRE(a == list.getValue(9));
        }
        SECTION("Compact")
        {
            list.insert(0, a);
            list.insert(5, a);

            SPDLOG_DEBUG("list: {}", list.toString());
            CHECK(1 == list.size());

            list.insert(10, b);
            list.insert(5, a);
            SPDLOG_DEBUG("list: {}", list.toString());
            CHECK(2 == list.size());

            // also after replacing first
            list.insert(0, c);
            SPDLOG_DEBUG("list: {}", list.toString());
            CHECK(2 == list.size());
            list.insert(5, a);
            SPDLOG_DEBUG("list: {}", list.toString());
            CHECK(3 == list.size());
        }
    }
    SECTION("center remove")
    {
        /*
         * Removing in the middle of an existing list should introduce a gap with the predefined "undefined value".
         */
        list.insert(0, 10);
        list.insert(5, 20);
        list.insert(10, 30);

        REQUIRE(3 == list.size());

        list.remove(6);

        CHECK(10 == list.getValue(4));
        CHECK(20 == list.getValue(5));
        CHECK(undefValue == list.getValue(6));
        CHECK(undefValue == list.getValue(7));
        CHECK(undefValue == list.getValue(8));
        CHECK(undefValue == list.getValue(9));
        CHECK(30 == list.getValue(10));

        // 4 segments
        REQUIRE(4 == list.size());

        list.remove(5);
        CHECK(undefValue == list.getValue(5));
        // both deleted segments combine to one
        REQUIRE(3 == list.size());
    }
    SECTION("remove in first entry")
    {
        /*
         * Removing in the middle of an existing list should introduce a gap with the predefined "undefined value".
         */
        list.insert(0, 10);
        list.insert(5, 20);
        list.insert(10, 30);

        REQUIRE(3 == list.size());

        list.remove(2);
        CHECK(4 == list.size());
    }
    SECTION("right remove")
    {
        /*
         * Removing elements to the right should result in an interval with the undefined value to the right.
         */

        list.insert(0, 10);
        list.insert(5, 20);
        list.insert(10, 30);

        REQUIRE(3 == list.size());

        list.remove(15);
        CHECK(undefValue == list.getValue(15));
        CHECK(undefValue == list.getValue(16));

        CHECK(30 == list.getValue(14));
        REQUIRE(4 == list.size());

        // removing further to the right should do nothing
        list.remove(20);
        REQUIRE(4 == list.size());

        // now removing similar sections will make the list smaller
        list.remove(10);
        REQUIRE(3 == list.size());

        list.remove(5);
        REQUIRE(2 == list.size());

        // removing the last element, will make the list become empty, although it had the size 2 before.
        list.remove(0);
        REQUIRE(0 == list.size());
    }
    SECTION("Left remove")
    {
        /*
         * Removing on the left of a list should delete the whole segment entry. There will be no gap inside the list
         */

        list.insert(0, 10);
        list.insert(5, 20);
        list.insert(10, 30);

        REQUIRE(3 == list.size());

        list.remove(0);

        CHECK(undefValue == list.getValue(3));
        CHECK(undefValue == list.getValue(4));
        CHECK(20 == list.getValue(5));
        REQUIRE(2 == list.size());

        list.remove(5);

        REQUIRE(1 == list.size());

        list.remove(10);
        REQUIRE(0 == list.size());
    }
    SECTION("index of")
    {
        list.insert(5, 10);
        list.insert(15, 20);
        list.insert(100, 30);
        list.insert(70, 40);

        REQUIRE(4 == list.size());

        CHECK(-1 == list.indexOf(0));
        CHECK(-1 == list.indexOf(1));
        CHECK(-1 == list.indexOf(4));
        CHECK(-1 == list.indexOf(-10));
        CHECK(0 == list.indexOf(5));
        CHECK(0 == list.indexOf(10));
        CHECK(0 == list.indexOf(14));
        CHECK(1 == list.indexOf(15));
        CHECK(1 == list.indexOf(30));
        CHECK(1 == list.indexOf(69));
        CHECK(2 == list.indexOf(70));
        CHECK(2 == list.indexOf(80));
        CHECK(2 == list.indexOf(99));
        CHECK(3 == list.indexOf(100));
        CHECK(3 == list.indexOf(101));
        CHECK(3 == list.indexOf(1000));
    }
    SECTION("Retriving Value")
    {
        list.insert(0, 10);
        list.insert(5, 20);
        list.insert(15, 30);
        list.remove(10);
        list.remove(40);
        list.remove(0);

        REQUIRE(4 == list.size());

        CHECK(undefValue == list.getValue(-1));
        CHECK(undefValue == list.getValue(0));
        CHECK(undefValue == list.getValue(4));
        CHECK(20 == list.getValue(5));
        CHECK(20 == list.getValue(9));
        CHECK(undefValue == list.getValue(10));
        CHECK(undefValue == list.getValue(14));
        CHECK(30 == list.getValue(15));
        CHECK(30 == list.getValue(30));
        CHECK(30 == list.getValue(39));
        CHECK(undefValue == list.getValue(40));
    }
    SECTION("Retrieving Entry")
    {
        list.insert(0, 10);
        list.insert(5, 20);
        list.insert(15, 30);
        list.remove(20);
        list.remove(40);
        list.remove(0);

        auto &entry = list.getEntry(5);

        CHECK(20 == entry.data);
        CHECK(5 == entry.start);

        // alternating entry should alternate list
        entry.data = 100;
        CHECK(100 == list.getValue(6));

        // accessing invalid position
        REQUIRE_THROWS(list.getEntry(0));
    }
}