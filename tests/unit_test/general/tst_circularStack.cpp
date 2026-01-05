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

#include "circularStack.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Stack of ints")
{
    CircularStack<int, 5> stack;

    SECTION("Error on pop on empty one")
    {
        CHECK_THROWS(stack.pop());

        stack.push(42);
        stack.pop();
        CHECK_THROWS(stack.pop());
    }

    SECTION("Single Push'n'Pop")
    {
        stack.push(1337);
        CHECK(stack.pop() == 1337);
    }

    SECTION("LIFO")
    {
        stack.push(1337);
        stack.push(42);
        stack.push(7353);

        CHECK(stack.pop() == 7353);
        CHECK(stack.pop() == 42);
        CHECK(stack.pop() == 1337);
    }

    SECTION("Overwrite with unsafePush")
    {
        for(int i = 0; i < 5; ++i)
        {
            stack.push(i);
        }

        stack.push(42);
        stack.push(1337);

        CHECK(stack.pop() == 1337);
        CHECK(stack.pop() == 42);
        CHECK(stack.pop() == 4);
        CHECK(stack.pop() == 3);
        CHECK(stack.pop() == 2);
        CHECK_THROWS(stack.pop());
    }
}
