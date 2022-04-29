/*
 * PeTrack - Software for tracking pedestrians movement in videos
 * Copyright (C) 2010-2021 Forschungszentrum Jülich GmbH,
 * Maik Boltes, Juliane Adrian, Ricardo Martin Brualla, Arne Graf, Paul Häger, Daniel Hillebrand,
 * Deniz Kilic, Paul Lieberenz, Daniel Salden, Tobias Schrödter, Ann Katrin Seemann
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

#include "helper.h"

#include <catch2/catch.hpp>


TEST_CASE("Petrack version strings are compared", "[lessThanVersion]")
{
    CHECK(lessThanVersion(QString("0.9.1"), QString("0.9.0")));
    CHECK(lessThanVersion(QString("0.9.0"), QString("0.8.0")));
    CHECK(lessThanVersion(QString("1.8.0"), QString("0.9.0")));
    CHECK(lessThanVersion(QString("0.9.1.1"), QString("0.9.0")));
    CHECK(lessThanVersion(QString("0.9.0.0"), QString("0.9.0")));

    CHECK_FALSE(lessThanVersion(QString("0.9.0"), QString("0.9.1")));
    CHECK_FALSE(lessThanVersion(QString("0.8.0"), QString("0.9.0")));
    CHECK_FALSE(lessThanVersion(QString("0.9.0"), QString("1.9.0")));
    CHECK_FALSE(lessThanVersion(QString("0.9.0"), QString("0.9.1.1")));
    CHECK_FALSE(lessThanVersion(QString("0.9.0.1"), QString("0.9.1")));
    CHECK_FALSE(lessThanVersion(QString("0.9.0"), QString("0.9.0.0")));
    CHECK_FALSE(lessThanVersion(QString("0.9.0"), QString("0.9.0")));

    CHECK_THROWS(lessThanVersion(QString("0.8.k"), QString("0.9.0")));
    CHECK_THROWS(lessThanVersion(QString("0.8.9"), QString("0.9.k")));
    CHECK_THROWS(lessThanVersion(QString("0.k.9"), QString("0.9.0")));
    CHECK_THROWS(lessThanVersion(QString("0.8.9"), QString("0.k.9")));
    CHECK_THROWS(lessThanVersion(QString("k.8.9"), QString("0.9.0")));
    CHECK_THROWS(lessThanVersion(QString("0.8.9"), QString("k.9.0")));
    CHECK_THROWS(lessThanVersion(QString("0.8.9"), QString("k.9.0")));
}
