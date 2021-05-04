/*
 * PeTrack - Software for tracking pedestrians movement in videos
 * Copyright (C) 2010-2020 Forschungszentrum Jülich GmbH,
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
 * along with this program.  If not, see <https://cdwww.gnu.org/licenses/>.
 */

#include <catch2/catch.hpp>

#include "recognition.h"
#include "petrack.h"

TEST_CASE("src/recognition", "[recognition]")
{
    SECTION("RecognitionMarker to int")
    {
        // Note: be aware that changing the explicitly assigned integer values will break backwards compatibility!
        REQUIRE(static_cast<int>(reco::RecognitionMethod::Casern) == 0);
        REQUIRE(static_cast<int>(reco::RecognitionMethod::Hermes) == 1);
        REQUIRE(static_cast<int>(reco::RecognitionMethod::Stereo) == 2);
        REQUIRE(static_cast<int>(reco::RecognitionMethod::Color) == 3);
        REQUIRE(static_cast<int>(reco::RecognitionMethod::Japan) == 4);
        REQUIRE(static_cast<int>(reco::RecognitionMethod::MultiColor) == 5);
        REQUIRE(static_cast<int>(reco::RecognitionMethod::Code) == 6);
    }
}
