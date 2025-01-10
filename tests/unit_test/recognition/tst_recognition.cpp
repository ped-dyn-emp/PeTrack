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

#include "petrack.h"
#include "recognition.h"

#include <QSignalSpy>
#include <catch2/catch.hpp>

using namespace reco;

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
        REQUIRE(static_cast<int>(reco::RecognitionMethod::MachineLearning) == 7);
    }
}

SCENARIO("I change Aruco parameters (via UI)")
{
    CodeMarkerOptions options;

    GIVEN("I change the detector params")
    {
        QSignalSpy      spy{&options, &CodeMarkerOptions::detectorParamsChanged};
        ArucoCodeParams params;
        params.setAdaptiveThreshConstant(20);
        options.setDetectorParams(params);
        THEN("A corresponding change signal was emitted")
        {
            REQUIRE(spy.count() == 1);
        }
    }

    GIVEN("I change the index of the marker dict")
    {
        QSignalSpy spy{&options, &CodeMarkerOptions::indexOfMarkerDictChanged};
        const int  newIndex = options.getIndexOfMarkerDict() + 1;
        options.setIndexOfMarkerDict(newIndex);
        THEN("A corresponding change signal was emitted")
        {
            REQUIRE(spy.count() == 1);
        }
    }
}

SCENARIO("I use the setter/getter of ArucoCodeParams")
{
    ArucoCodeParams params;
    GIVEN("I set a minimum larger than the maximum")
    {
        const int    newAdaptiveThreshWinSizeMin = params.getAdaptiveThreshWinSizeMax() + 1;
        const double newMinMarkerPerimeter       = params.getMaxMarkerPerimeter() + 1;
        const auto   oldAdaptiveThreshWinSizeMin = params.getAdaptiveThreshWinSizeMin();
        const auto   oldMinMarkerPerimeter       = params.getMinMarkerPerimeter();
        THEN("An exception is thrown and values aren't changed")
        {
            REQUIRE_THROWS(params.setAdaptiveThreshWinSizeMin(newAdaptiveThreshWinSizeMin));
            REQUIRE(params.getAdaptiveThreshWinSizeMin() == oldAdaptiveThreshWinSizeMin);
            REQUIRE_THROWS(params.setMinMarkerPerimeter(newMinMarkerPerimeter));
            REQUIRE(params.getMinMarkerPerimeter() == Approx(oldMinMarkerPerimeter));
        }
    }

    GIVEN("I set a maximum lower than the minimum")
    {
        const int    newAdaptiveThreshSizeMax    = params.getAdaptiveThreshWinSizeMin() - 1;
        const double newMaxMarkerPerimeter       = params.getMinMarkerPerimeter() - 1;
        const auto   oldAdaptiveThreshWinSizeMax = params.getAdaptiveThreshWinSizeMax();
        const auto   oldMaxMarkerPerimeter       = params.getMaxMarkerPerimeter();
        THEN("An exception is thrown and values aren't changed")
        {
            REQUIRE_THROWS(params.setAdaptiveThreshWinSizeMax(newAdaptiveThreshSizeMax));
            REQUIRE(params.getAdaptiveThreshWinSizeMax() == oldAdaptiveThreshWinSizeMax);
            REQUIRE_THROWS(params.setMaxMarkerPerimeter(newMaxMarkerPerimeter));
            REQUIRE(params.getMaxMarkerPerimeter() == Approx(oldMaxMarkerPerimeter));
        }
    }

    GIVEN("A cornerRefinementWinSize less than 1")
    {
        constexpr int newCornerRefinementWinSize = -2;
        const int     oldCornerRefinementWinSize = params.getCornerRefinementWinSize();
        THEN("An Exception is thrown and value isn't changed")
        {
            REQUIRE_THROWS(params.setCornerRefinementWinSize(newCornerRefinementWinSize));
            REQUIRE(params.getCornerRefinementWinSize() == oldCornerRefinementWinSize);
        }
    }

    GIVEN("A cornerRefinementMaxIterations less than 1")
    {
        constexpr int newCornerRefinementMaxIterations = 0;
        const int     oldCornerRefinementMaxIterations = params.getCornerRefinementMaxIterations();
        THEN("An exception is thrown and value isn't changed")
        {
            REQUIRE_THROWS(params.setCornerRefinementMaxIterations(newCornerRefinementMaxIterations));
            REQUIRE(params.getCornerRefinementMaxIterations() == oldCornerRefinementMaxIterations);
        }
    }

    GIVEN("A cornerRefinementMinAccuracy less than or equal to 0")
    {
        constexpr double newCornerRefinementMinAccuracy = -3;
        const auto       oldCornerRefinementMinAccuracy = params.getCornerRefinementMinAccuracy();
        THEN("An exception is thrown and value isn't changed")
        {
            REQUIRE_THROWS(params.setCornerRefinementMinAccuracy(newCornerRefinementMinAccuracy));
            REQUIRE(params.getCornerRefinementMinAccuracy() == Approx(oldCornerRefinementMinAccuracy));
        }
    }

    GIVEN("markerBorderBits less than 1")
    {
        constexpr int newBorderBits = 0;
        const int     oldBorderBits = params.getMarkerBorderBits();
        THEN("An exception is thrown and value isn't changed")
        {
            REQUIRE_THROWS(params.setMarkerBorderBits(newBorderBits));
            REQUIRE(params.getMarkerBorderBits() == oldBorderBits);
        }
    }

    GIVEN("minStdDevOtsu less than or equal to 0")
    {
        constexpr double newMinOtsuStdDev = -2;
        const auto       oldMinOtsuStdDev = params.getMinOtsuStdDev();
        THEN("An exception is thrown and value isn't changed")
        {
            REQUIRE_THROWS(params.setMinOtsuStdDev(newMinOtsuStdDev));
            REQUIRE(params.getMinOtsuStdDev() == Approx(oldMinOtsuStdDev));
        }
    }

    GIVEN("adaptiveThreshSizeMin less than 3")
    {
        constexpr int newThreshSizeMin = 2;
        const int     oldThreshSizeMin = params.getAdaptiveThreshWinSizeMin();
        THEN("An exception is thrown and value isn't changed")
        {
            REQUIRE_THROWS(params.setAdaptiveThreshWinSizeMin(newThreshSizeMin));
            REQUIRE(params.getAdaptiveThreshWinSizeMin() == oldThreshSizeMin);
        }
    }

    GIVEN("adaptiveThreshSizeMax less than 3")
    {
        constexpr int newThreshWinSizeMax = 2;
        const int     oldThreshWinSizeMax = params.getAdaptiveThreshWinSizeMax();
        THEN("An exception is thrown and value isn't changed")
        {
            REQUIRE_THROWS(params.setAdaptiveThreshWinSizeMax(newThreshWinSizeMax));
            REQUIRE(params.getAdaptiveThreshWinSizeMax() == oldThreshWinSizeMax);
        }
    }

    GIVEN("adaptiveThreshWinSizeStep less than or equal to 0")
    {
        constexpr int newWinSizeStep = 0;
        const int     oldWinSizeStep = params.getAdaptiveThreshWinSizeStep();
        THEN("An exception is thrown and value isn't changed")
        {
            REQUIRE_THROWS(params.setAdaptiveThreshWinSizeStep(newWinSizeStep));
            REQUIRE(params.getAdaptiveThreshWinSizeStep() == oldWinSizeStep);
        }
    }

    GIVEN("minMarkerPerimeter less than or equal to 0")
    {
        constexpr double newMinMarkerPerimeter = 0;
        const auto       oldMinMarkerPerimeter = params.getMinMarkerPerimeter();
        THEN("An exception is thrown and value isn't changed")
        {
            REQUIRE_THROWS(params.setMinMarkerPerimeter(newMinMarkerPerimeter));
            REQUIRE(params.getMinMarkerPerimeter() == Approx(oldMinMarkerPerimeter));
        }
    }

    GIVEN("maxMarkerPerimeter less than or equal to 0")
    {
        constexpr double newMaxMarkerPerimeter = 0;
        const auto       oldMaxMarkerPerimeter = params.getMaxMarkerPerimeter();
        THEN("An exception is thrown and value isn't changed")
        {
            REQUIRE_THROWS(params.setMaxMarkerPerimeter(newMaxMarkerPerimeter));
            REQUIRE(params.getMaxMarkerPerimeter() == Approx(oldMaxMarkerPerimeter));
        }
    }

    GIVEN("cornerRefinementMinAccuracy less than or equal to 0")
    {
        constexpr double newMinAccuracy = 0;
        const auto       oldMinAccuracy = params.getCornerRefinementMinAccuracy();
        THEN("An exception is thrown and value isn't changed")
        {
            REQUIRE_THROWS(params.setCornerRefinementMinAccuracy(newMinAccuracy));
            REQUIRE(params.getCornerRefinementMinAccuracy() == Approx(oldMinAccuracy));
        }
    }

    GIVEN("minCornerDistance less than 0")
    {
        constexpr double newMinCornerDistance = -1;
        const auto       oldMinCornerDistance = params.getMinMarkerDistance();
        THEN("An exception is thrown and value isn't changed")
        {
            REQUIRE_THROWS(params.setMinCornerDistance(newMinCornerDistance));
            REQUIRE(params.getMinCornerDistance() == Approx(oldMinCornerDistance));
        }
    }

    GIVEN("minDistanceToBorder less than 0")
    {
        constexpr int newMinDistToBorder = -2;
        const int     oldMinDistToBorder = params.getMinDistanceToBorder();
        THEN("An exception is thown and value isn't changed")
        {
            REQUIRE_THROWS(params.setMinDistanceToBorder(newMinDistToBorder));
            REQUIRE(params.getMinDistanceToBorder() == oldMinDistToBorder);
        }
    }
}
