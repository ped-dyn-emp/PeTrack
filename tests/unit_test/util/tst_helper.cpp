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

#include "helper.h"
#include "logger.h"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_vector.hpp>

TEST_CASE("Test qRectToCvRect", "[helper]")
{
    // create image mock. The datatype has no effect, so take a small one
    auto image = cv::Mat(1080, 1920, CV_8U);

    // variables for reuse
    cv::Rect rect;
    QRect    roi;

    SECTION("ROI somewhere inside image")
    {
        GIVEN("uneven width ROI")
        {
            roi  = QRect(10, 10, 201, 201);
            rect = qRectToCvRect(roi, image);

            THEN("Rect is shortened to even length")
            {
                REQUIRE(roi.x() == rect.x);
                REQUIRE(roi.y() == rect.y);
                REQUIRE(roi.width() - 1 == rect.width);
                REQUIRE(roi.height() - 1 == rect.height);
            }
        }
        GIVEN("uneven width ROI and explicit evenPixel=false")
        {
            rect = qRectToCvRect(roi, image, false);

            THEN("Rect is equal to roi")
            {
                REQUIRE(roi.x() == rect.x);
                REQUIRE(roi.y() == rect.y);
                REQUIRE(roi.width() == rect.width);
                REQUIRE(roi.height() == rect.height);
            }
        }

        GIVEN("even width ROI")
        {
            roi = QRect(20, 20, 400, 400);

            rect = qRectToCvRect(roi, image);

            THEN("Rect is equal to roi")
            {
                REQUIRE(roi.x() == rect.x);
                REQUIRE(roi.y() == rect.y);
                REQUIRE(roi.width() == rect.width);
                REQUIRE(roi.height() == rect.height);
            }

            rect = qRectToCvRect(roi, image, false);

            THEN("Rect is equal to roi")
            {
                REQUIRE(roi.x() == rect.x);
                REQUIRE(roi.y() == rect.y);
                REQUIRE(roi.width() == rect.width);
                REQUIRE(roi.height() == rect.height);
            }
        }
    }

    SECTION("ROI partly outside image")
    {
        GIVEN("roi top left outside")
        {
            roi  = QRect(-10, -10, 200, 200);
            rect = qRectToCvRect(roi, image);

            THEN("rect is trimmed to be inside")
            {
                REQUIRE(0 == rect.x);
                REQUIRE(0 == rect.y);
                REQUIRE(190 == rect.width);
                REQUIRE(190 == rect.height);
            }
        }

        GIVEN("ROI bottom right outside")
        {
            roi  = QRect(1900, 1050, 100, 100);
            rect = qRectToCvRect(roi, image);

            THEN("ROI is trimmed to image")
            {
                REQUIRE(roi.x() == rect.x);
                REQUIRE(roi.y() == rect.y);
                REQUIRE(20 == rect.width);
                REQUIRE(30 == rect.height);
            }
        }

        GIVEN("ROI is outside left and right")
        {
            roi  = QRect(-10, 10, 2000, 100);
            rect = qRectToCvRect(roi, image);

            THEN("ROI is trimmed left and right")
            {
                REQUIRE(0 == rect.x);
                REQUIRE(roi.y() == rect.y);
                REQUIRE(1920 == rect.width);
                REQUIRE(roi.height() == rect.height);
            }
        }

        GIVEN("ROI is outside top and bottom")
        {
            roi  = QRect(10, -10, 100, 2000);
            rect = qRectToCvRect(roi, image);
            THEN("ROI is trimmed top and bottom")
            {
                REQUIRE(roi.x() == rect.x);
                REQUIRE(0 == rect.y);
                REQUIRE(roi.width() == rect.width);
                REQUIRE(1080 == rect.height);
            }
        }

        GIVEN("ROI is completely bigger than image")
        {
            roi  = QRect(-10, -10, 2000, 2000);
            rect = qRectToCvRect(roi, image);
            THEN("ROI is equal to image size")
            {
                REQUIRE(0 == rect.x);
                REQUIRE(0 == rect.y);
                REQUIRE(1920 == rect.width);
                REQUIRE(1080 == rect.height);
            }
        }


        SECTION("Trimming should not break evenPixel size")
        {
            GIVEN("ROI trimming would break evenness")
            {
                roi  = QRect(-11, 10, 50, 10);
                rect = qRectToCvRect(roi, image);

                // with plain trimming the rect would be (0, 10, 39, 10)
                // with even sides, width should be 38

                THEN("Trimmed Rect has still even dimension")
                {
                    REQUIRE(0 == rect.x);
                    REQUIRE(38 == rect.width);
                }
            }
        }
    }

    SECTION("ROI completely outside image")
    {
        roi  = QRect(-20, -20, 10, 10);
        rect = qRectToCvRect(roi, image);

        REQUIRE(0 == rect.x);
        REQUIRE(0 == rect.y);
        REQUIRE(0 == rect.width);
        REQUIRE(0 == rect.height);
    }

} // END TESTCASE qRectToCvRect

TEST_CASE("Test split compact string", "[helper][grouping]")
{
    std::string input;

    // this test case translates the split result to a vector,
    // because catch2 offers matchers for quick vector content checking (but not for sets)

    input                = "1, 1";
    auto             res = splitCompactString(input);
    std::vector<int> vec(res.begin(), res.end());
    CHECK_THAT(vec, Catch::Matchers::UnorderedEquals(std::vector<int>{1}));

    input = "1";
    res   = splitCompactString(input);
    vec   = std::vector(res.begin(), res.end());
    CHECK_THAT(vec, Catch::Matchers::UnorderedEquals(std::vector<int>{1}));

    input = "1, 2,   3,                     4,5";
    res   = splitCompactString(input);
    vec   = std::vector(res.begin(), res.end());
    CHECK_THAT(vec, Catch::Matchers::UnorderedEquals(std::vector<int>{1, 2, 3, 4, 5}));

    input = "1-5";
    res   = splitCompactString(input);
    vec   = std::vector(res.begin(), res.end());
    CHECK_THAT(vec, Catch::Matchers::UnorderedEquals(std::vector<int>{1, 2, 3, 4, 5}));

    // leading and trailing whitespace
    input = "  1-5  ";
    res   = splitCompactString(input);
    vec   = std::vector(res.begin(), res.end());
    CHECK_THAT(vec, Catch::Matchers::UnorderedEquals(std::vector<int>{1, 2, 3, 4, 5}));

    input = "1 - 3, 4, 5";
    res   = splitCompactString(input);
    vec   = std::vector(res.begin(), res.end());
    CHECK_THAT(vec, Catch::Matchers::UnorderedEquals(std::vector<int>{1, 2, 3, 4, 5}));

    input = "1 - 3, 4, 5, 5";
    res   = splitCompactString(input);
    vec   = std::vector(res.begin(), res.end());
    CHECK_THAT(vec, Catch::Matchers::UnorderedEquals(std::vector<int>{1, 2, 3, 4, 5}));

    input = "1 - 1,2-3,2-5, 4, 5, 5";
    res   = splitCompactString(input);
    vec   = std::vector(res.begin(), res.end());
    CHECK_THAT(vec, Catch::Matchers::UnorderedEquals(std::vector<int>{1, 2, 3, 4, 5}));


    input = "5 - 1";
    res   = splitCompactString(input);
    vec   = std::vector(res.begin(), res.end());
    CHECK_THAT(vec, Catch::Matchers::UnorderedEquals(std::vector<int>{1, 2, 3, 4, 5}));

    input = "-1 - 3, 4, 5, 5";
    CHECK_THROWS_AS(splitCompactString(input), std::invalid_argument);

    input = "-1";
    CHECK_THROWS_AS(splitCompactString(input), std::invalid_argument);

    input = "1 - 3,, 4, 5, 5";
    CHECK_THROWS_AS(splitCompactString(input), std::invalid_argument);

    input = "1 - 3,e, 4, 5, 5";
    CHECK_THROWS_AS(splitCompactString(input), std::invalid_argument);

    input = "1. - 3, 4";
    CHECK_THROWS_AS(splitCompactString(input), std::invalid_argument);

    input = "1.5 - 3, 4";
    CHECK_THROWS_AS(splitCompactString(input), std::invalid_argument);

    input = "1 --3, 4";
    CHECK_THROWS_AS(splitCompactString(input), std::invalid_argument);

    input = "1 - 3, 4,";
    CHECK_THROWS_AS(splitCompactString(input), std::invalid_argument);
}
