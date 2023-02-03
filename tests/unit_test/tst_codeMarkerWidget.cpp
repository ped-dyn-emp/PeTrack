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

#include "codeMarkerWidget.h"
#include "recognition.h"
#include "ui_codeMarker.h"

#include <catch2/catch.hpp>

SCENARIO("The user opens the CodeMarkerWidget", "[ui]")
{
    // create the dialog
    Petrack                 petrack{"Unknown"};
    reco::CodeMarkerOptions options;
    Ui::CodeMarker         *view = new Ui::CodeMarker();
    CodeMarkerWidget        widget{&petrack, options, view};

    THEN("The view shows the correct values")
    {
        const auto &params          = options.getDetectorParams();
        const auto  displayedParams = packDetectorParams(view);
        REQUIRE(params == displayedParams);
    }

    GIVEN("The user changes an AruCo Parameter")
    {
        // since the params are handled the same, not all are
        // explicitly tested here
        constexpr double newMinMarkerDistance = 0;
        view->minMarkerDistance->setValue(newMinMarkerDistance); // <- user input

        const int newListIndex = view->dictList->currentIndex() + 1;
        view->dictList->setCurrentIndex(newListIndex); // <- user input

        THEN("The options-object is updated")
        {
            REQUIRE(options.getDetectorParams().getMinMarkerDistance() == Approx(newMinMarkerDistance));
            REQUIRE(options.getIndexOfMarkerDict() == newListIndex);
        }
    }

    GIVEN("The AruCo Parameters changed via an other way")
    {
        reco::ArucoCodeParams newData;
        newData.setMinMarkerDistance(10);
        options.setDetectorParams(newData);
        constexpr int newIndex = 2;
        options.setIndexOfMarkerDict(newIndex);

        THEN("The view is updated")
        {
            REQUIRE(packDetectorParams(view) == newData);
            REQUIRE(view->dictList->currentIndex() == newIndex);
        }
    }

    GIVEN("Minimum larger than maximum")
    {
        auto oldValues = packDetectorParams(view);
        view->adaptiveThreshWinSizeMin->setValue(view->adaptiveThreshWinSizeMax->value() + 1);
        view->minMarkerPerimeter->setValue(view->maxMarkerPerimeter->value() + 1);
        THEN("It is not changed")
        {
            REQUIRE(view->adaptiveThreshWinSizeMin->value() == oldValues.getAdaptiveThreshWinSizeMin());
            REQUIRE(view->minMarkerPerimeter->value() == Approx(oldValues.getMinMarkerPerimeter()));
        }
    }

    GIVEN("Maximum lower than minimum")
    {
        auto oldValues = packDetectorParams(view);
        // Since min already has minimal value this isn't properly tested with the default min
        view->adaptiveThreshWinSizeMin->setValue(10);
        view->adaptiveThreshWinSizeMax->setValue(view->adaptiveThreshWinSizeMin->value() - 1);
        // view->minMarkerPerimeter->
        view->maxMarkerPerimeter->setValue(view->minMarkerPerimeter->value() - 1);
        THEN("It is not changed")
        {
            REQUIRE(view->adaptiveThreshWinSizeMax->value() == oldValues.getAdaptiveThreshWinSizeMax());
            REQUIRE(view->maxMarkerPerimeter->value() == Approx(oldValues.getMaxMarkerPerimeter()));
        }
    }

    GIVEN("A cornerRefinementWinSize less than 1")
    {
        view->cornerRefinementWinSize->setValue(-2);
        THEN("The value gets set to 1")
        {
            REQUIRE(view->cornerRefinementWinSize->value() == 1);
        }
    }

    GIVEN("A cornerRefinementMaxIterations less than 1")
    {
        view->cornerRefinementMaxIterations->setValue(0);
        THEN("The value gets set to 1")
        {
            REQUIRE(view->cornerRefinementMaxIterations->value() == 1);
        }
    }

    GIVEN("A cornerRefinementMinAccuracy less than or equal to 0")
    {
        view->cornerRefinementMinAccuracy->setValue(-3);
        THEN("It is set to 0.01")
        {
            REQUIRE(view->cornerRefinementMinAccuracy->value() == Approx(0.01));
        }
    }

    GIVEN("markerBorderBits less than 1")
    {
        view->markerBorderBits->setValue(0);
        THEN("The value is set to 1")
        {
            REQUIRE(view->markerBorderBits->value() == 1);
        }
    }

    GIVEN("minStdDevOtsu less than or equal to 0")
    {
        view->minOtsuStdDev->setValue(-2);
        THEN("The value is set to 0.01")
        {
            REQUIRE(view->minOtsuStdDev->value() == Approx(0.01));
        }
    }

    GIVEN("adaptiveThreshSizeMin less than 3")
    {
        view->adaptiveThreshWinSizeMin->setValue(1);
        THEN("The value is set to 3")
        {
            REQUIRE(view->adaptiveThreshWinSizeMin->value() == 3);
        }
    }

    GIVEN("adaptiveThreshSizeMax less than 3")
    {
        view->adaptiveThreshWinSizeMax->setValue(1);
        THEN("The value is set to 3")
        {
            REQUIRE(view->adaptiveThreshWinSizeMax->value() == 3);
        }
    }

    GIVEN("adaptiveThreshWinSizeStep less than or equal to 0")
    {
        view->adaptiveThreshWinSizeStep->setValue(0);
        THEN("The value is set to 1")
        {
            REQUIRE(view->adaptiveThreshWinSizeStep->value() == 1);
        }
    }

    GIVEN("minMarkerPerimeter less than or equal to 0")
    {
        view->minMarkerPerimeter->setValue(0);
        THEN("The value is set to 0.1")
        {
            REQUIRE(view->minMarkerPerimeter->value() == Approx(0.1));
        }
    }

    GIVEN("maxMarkerPerimeter less than or equal to 0")
    {
        auto oldVal = view->maxMarkerPerimeter->value();
        view->minMarkerPerimeter->setValue(0); // since max cannot be lower than min
        view->maxMarkerPerimeter->setValue(0);
        THEN("The value is not changed")
        {
            REQUIRE(view->maxMarkerPerimeter->value() == Approx(oldVal));
        }
    }

    GIVEN("cornerRefinementMinAccuracy less than or equal to 0")
    {
        view->cornerRefinementMinAccuracy->setValue(0);
        THEN("The value is set to 0.01")
        {
            REQUIRE(view->cornerRefinementMinAccuracy->value() == Approx(0.01));
        }
    }

    GIVEN("minCornerDistance less than 0")
    {
        view->minCornerDistance->setValue(-1);
        THEN("The value is set to 1")
        {
            REQUIRE(view->minCornerDistance->value() == Approx(0));
        }
    }

    GIVEN("minDistanceToBorder less than 0")
    {
        view->minDistanceToBorder->setValue(-2);
        THEN("The value is set to 0")
        {
            REQUIRE(view->minDistanceToBorder->value() == 0);
        }
    }
}
