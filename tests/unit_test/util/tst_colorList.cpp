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

#include "colorList.h"

#include <catch2/catch.hpp>

TEST_CASE("test color list", "[util]")
{
    ColorList list{{Qt::red, Qt::green, Qt::blue, Qt::yellow}};

    SECTION("forward")
    {
        CHECK(list.peekNext() == Qt::red);
        CHECK(list.peekNext() == Qt::red);

        CHECK(list.next() == Qt::red);
        CHECK(list.peekNext() == Qt::green);

        CHECK(list.next() == Qt::green);
        CHECK(list.next() == Qt::blue);
        CHECK(list.next() == Qt::yellow);
        CHECK(list.peekNext() == Qt::red);
        CHECK(list.next() == Qt::red);
    }

    SECTION("backward")
    {
        CHECK(list.peekNext() == Qt::red);
        CHECK(list.previous() == Qt::yellow);
        CHECK(list.peekNext() == Qt::yellow);
        CHECK(list.previous() == Qt::blue);
        CHECK(list.peekNext() == Qt::blue);
    }
}

TEST_CASE("empty list for color list", "[util]")
{
    REQUIRE_THROWS(ColorList({}));
}