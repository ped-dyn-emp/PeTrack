/*
 * PeTrack - Software for tracking pedestrians movement in videos
 * Copyright (C) 2023 Forschungszentrum Jülich GmbH, IAS-7
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

#include "moCapController.h"
#include "moCapPerson.h"
#include "personStorage.h"
#include "petrack.h"

#include <catch2/catch.hpp>
#include <catch2/trompeloeil.hpp>

using namespace Catch::Matchers;

class ExtrCalibMock : public trompeloeil::mock_interface<ExtrCalibration>
{
public:
    ExtrCalibMock(PersonStorage &storage) : trompeloeil::mock_interface<ExtrCalibration>(storage) {}
    MAKE_MOCK1(getImagePoint, cv::Point2f(cv::Point3f), override);
};

SCENARIO("I want to get the render data with one person loaded", "[ui]")
{
    MoCapStorage storage;
    // TODO without any persons in the storage?? (Other Scenario)
    // NOTE SkeletonTree needs nodes with id 1 and 2 which are connected in the right order, but we do not enforce that
    // in any way

    /*
     * Resulting Lines should be: root to child; child to grandchild;
     */
    SkeletonNode  rootA{0, cv::Point3f{100, 100, 0}};
    SkeletonNode &childA      = rootA.addChild({19, cv::Point3f{200, 100, 69}});
    SkeletonNode &grandchildA = childA.addChild({19, cv::Point3f{50, 50, 42}});
    grandchildA.addChild({2, cv::Point3f{150, 150, 1337}});

    SkeletonNode  rootB{0, cv::Point3f{50, 50, 0}};
    SkeletonNode &childB      = rootB.addChild({19, cv::Point3f{100, 50, 69}});
    SkeletonNode &grandchildB = childB.addChild({19, cv::Point3f{25, 25, 42}});
    grandchildB.addChild({2, cv::Point3f{75, 75, 1337}});

    MoCapPerson person;
    person.setSamplerate(1);
    // default time offset 0

    Petrack       pet{"Unknown"};
    Autosave      save{pet};
    PersonStorage st{pet, save};
    ExtrCalibMock extrCalib{st};

    /*
     * We Mock extrCalib and just return the x and y coordinate
     * Actually getting swapping, image border and so on right
     * should be tested in getImagePoint. We just assume it works
     * for this test and ignore different projections from
     * 3D to 2D due to different settings
     */
    ALLOW_CALL(extrCalib, getImagePoint(ANY(cv::Point3f))).RETURN(cv::Point2f(_1.x, _1.y));

    MoCapController moCapController{storage, extrCalib};

    GIVEN("a skeleton with a head direction")
    {
        person.addSkeleton({rootA, cv::Vec3f{1, 0, 0}, {0, 0, 0}});
        // TODO Person entsprechend anpassen
        // NOTE Also am besten Person in der arrow/narrow GIVEN aufbauen
        AND_GIVEN("no interpolation")
        {
            storage.addPerson(person);

            THEN("we get the correct render data")
            {
                // Get renderData for Second 0, which would be sample 1 (samplerate set to 1)
                std::vector<SegmentRenderData> renderData = moCapController.getRenderData(0, 25);
                SegmentRenderData              schablone;
                schablone.mThickness = 2;
                schablone.mColor     = QColor(255, 255, 55);
                schablone.mDirected  = false;
                std::vector<SegmentRenderData> correctData;
                schablone.mLine = QLine(100, 100, 200, 100);
                correctData.push_back(schablone);
                schablone.mLine = QLine(200, 100, 50, 50);
                correctData.push_back(schablone);
                schablone.mLine = QLine(50, 50, 150, 150);
                correctData.push_back(schablone);
                schablone.mDirected = true;
                schablone.mLine     = QLine(125, 125, 1427, 125);
                correctData.push_back(schablone);

                REQUIRE_THAT(renderData, UnorderedEquals(correctData));
            }
        }
        AND_GIVEN("interpolation")
        {
            person.addSkeleton({rootB, cv::Vec3f{0, 1, 0}, cv::Vec3f{0, 0, 0}});
            storage.addPerson(person);

            THEN("we get the correct render data")
            {
                // Get renderData for Second 1.5, which would be sample 1.5 (samplerate set to 1)
                auto              renderData = moCapController.getRenderData(25, 50);
                SegmentRenderData schablone;
                schablone.mThickness = 2;
                schablone.mColor     = QColor(255, 255, 55);
                schablone.mDirected  = false;
                std::vector<SegmentRenderData> correctData;
                schablone.mLine = QLine(75, 75, 150, 75);
                correctData.push_back(schablone);
                schablone.mLine = QLine(150, 75, 37, 37);
                correctData.push_back(schablone);
                schablone.mLine = QLine(37, 37, 112, 112);
                correctData.push_back(schablone);
                schablone.mDirected = true;
                schablone.mLine     = QLine(93, 93, 1012, 1012);
                correctData.push_back(schablone);

                REQUIRE_THAT(renderData, UnorderedEquals(correctData));
            }
        }

        // other tests work with the simple skeleton
        storage.addPerson(person);

        AND_GIVEN("blue as the selected color")
        {
            THEN("the lines are blue")
            {
                QColor blue{0, 0, 255};
                moCapController.setColor(blue);
                auto renderData = moCapController.getRenderData(0, 25);
                for(const auto &line : renderData)
                {
                    REQUIRE(line.mColor == blue);
                }
            }
        }

        AND_GIVEN("4 as the selected line thickness")
        {
            THEN("the lines have thickness of 4")
            {
                int thickness = 4;
                moCapController.setThickness(thickness);
                auto renderData = moCapController.getRenderData(0, 25);
                for(const auto &line : renderData)
                {
                    REQUIRE(line.mThickness == thickness);
                }
            }
        }

        AND_GIVEN("no pre-sample")
        {
            // NOTE if current frame is 0, then pre-sample is just 0; possible to have no pre-sample?
            int currentFrame = -10;
            int framerate    = 25;
            THEN("no render data is generated")
            {
                auto renderData = moCapController.getRenderData(currentFrame, framerate);
                REQUIRE(renderData.empty());
            }
        }
        AND_GIVEN("no post-sample")
        {
            int currentFrame = 10;
            int framerate    = 25;
            THEN("no render data is generated")
            {
                auto renderData = moCapController.getRenderData(currentFrame, framerate);
                REQUIRE(renderData.empty());
            }
        }
        AND_GIVEN("neither pre- or post-sample")
        {
            int currentFrame = 100;
            int framerate    = 25;
            THEN("no render data is generated")
            {
                auto renderData = moCapController.getRenderData(currentFrame, framerate);
                REQUIRE(renderData.empty());
            }
        }
    }
}
