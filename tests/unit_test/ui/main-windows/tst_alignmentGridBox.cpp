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

#include "alignmentGridBox.h"
#include "petrack.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("AlignmentGridBox: UI logic")
{
    Petrack          petrack{"Unkown"};
    AlignmentGridBox alignGridBox{&petrack};

    SECTION("Setting show to false should imply fixing the alignment grid")
    {
        // ensure show is checked and fix is unchecked
        alignGridBox.setShow(true);
        alignGridBox.setFix(false);
        alignGridBox.setShow(false);
        CHECK(alignGridBox.isFix());
    }
}
