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

#include "YOLOMarkerWidget.h"
#include "control.h"
#include "petrack.h"
#include "recognition.h"
#include "ui_YOLOMarkerWidget.h"
#include "ui_control.h"

#include <catch2/catch_test_macros.hpp>

SCENARIO("The user opens the YOLOMarkerWidget", "[ui]")
{
    Petrack               petrack{"Unknown"};
    Ui::YOLOMarkerWidget *view = new Ui::YOLOMarkerWidget();
    YOLOMarkerWidget      widget{&petrack, view};

    THEN("The values in the UI and internal marker options should match")
    {
        REQUIRE(view->confThreshold->value() == widget.getYOLOMarkerOptions().confThreshold);
        REQUIRE(view->nmsThreshold->value() == widget.getYOLOMarkerOptions().nmsThreshold);
        REQUIRE(view->scoreThreshold->value() == widget.getYOLOMarkerOptions().scoreThreshold);
        REQUIRE(view->imageSize->value() == widget.getYOLOMarkerOptions().imageSize);
    }
    GIVEN("The user changes the values in the UI")
    {
        view->confThreshold->setValue(0.75);
        view->nmsThreshold->setValue(0.25);
        view->scoreThreshold->setValue(0.9);
        view->imageSize->setValue(100);

        THEN("The marker options are updated accordingly")
        {
            REQUIRE(view->confThreshold->value() == widget.getYOLOMarkerOptions().confThreshold);
            REQUIRE(view->nmsThreshold->value() == widget.getYOLOMarkerOptions().nmsThreshold);
            REQUIRE(view->scoreThreshold->value() == widget.getYOLOMarkerOptions().scoreThreshold);
            REQUIRE(view->imageSize->value() == widget.getYOLOMarkerOptions().imageSize);
        }
    }
    GIVEN("The user tries to perform recognition without giving a model")
    {
        THEN("An exception is thrown")
        {
            CHECK_THROWS_AS(widget.initialize(), std::invalid_argument);
        }
    }
    GIVEN("The user provides a model file that doesn't exist")
    {
        THEN("An exception is thrown")
        {
            CHECK_THROWS_AS(widget.setModelFile("nonExistingFile.abc"), std::invalid_argument);
        }
    }
    GIVEN("The provides an invalid model file")
    {
        THEN("An exception is thrown")
        {
            CHECK_THROWS_AS(widget.setModelFile("CMakeLists.txt"), std::invalid_argument);
        }
    }
}
