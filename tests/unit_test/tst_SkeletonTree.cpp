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

#include "skeletonTree.h"
#include "skeletonTreeFactory.h"

#include <catch2/catch.hpp>

const XSenseStruct XSENSE_DUMMY_DATA = {
    {0, 0, 0},    {1, 1, 1},    {2, 2, 2},    {3, 3, 3},    {4, 4, 4},    {5, 5, 5},    {6, 6, 6},
    {7, 7, 7},    {8, 8, 8},    {9, 9, 9},    {10, 10, 10}, {11, 11, 11}, {12, 12, 12}, {13, 13, 13},
    {14, 14, 14}, {15, 15, 15}, {16, 16, 16}, {17, 17, 17}, {18, 18, 18}, {19, 19, 19}, {20, 20, 20},
};


TEST_CASE("SkeletonTree for XSenseData is built")
{
    SkeletonTree skel = SkeletonTreeFactory::generateTree(XSENSE_DUMMY_DATA);


    SECTION("Check all bone connections and positions")
    {
        auto &rootNode = skel.getRoot();
        REQUIRE(rootNode.getPos() == XSENSE_DUMMY_DATA.mRoot);
        REQUIRE(rootNode.getChildrenCount() == 3);
        // own id
        REQUIRE_THROWS(rootNode.getChildById(0));
        // does not exist
        REQUIRE_THROWS(rootNode.getChildById(100));
        // neck
        REQUIRE_NOTHROW(rootNode.getChildById(1));
        // left hip
        REQUIRE_NOTHROW(rootNode.getChildById(11));
        // right hip
        REQUIRE_NOTHROW(rootNode.getChildById(15));

        // check neck next
        auto &neckNode = rootNode.getChildById(1);
        REQUIRE(neckNode.getPos() == XSENSE_DUMMY_DATA.mNeck1);
        REQUIRE(neckNode.getChildrenCount() == 3);

        // head
        auto &upperNeckNode = neckNode.getChildById(19);
        REQUIRE_NOTHROW(upperNeckNode.getChildById(2));
        // left shoulder
        REQUIRE_NOTHROW(neckNode.getChildById(3));
        // right shoulder
        REQUIRE_NOTHROW(neckNode.getChildById(7));

        // head
        auto &headNode = upperNeckNode.getChildById(2);
        REQUIRE(headNode.getPos() == XSENSE_DUMMY_DATA.mHeadTop);
        REQUIRE(headNode.getChildrenCount() == 0);

        // left shoulder
        auto &lShldrNode = neckNode.getChildById(3);
        REQUIRE(lShldrNode.getPos() == XSENSE_DUMMY_DATA.mShldrL);
        REQUIRE(lShldrNode.getChildrenCount() == 1);

        REQUIRE_NOTHROW(lShldrNode.getChildById(4));

        // left shoulder
        auto &lElbowNode = lShldrNode.getChildById(4);
        REQUIRE(lElbowNode.getPos() == XSENSE_DUMMY_DATA.mElbowL);
        REQUIRE(lElbowNode.getChildrenCount() == 1);

        // left wrist
        REQUIRE_NOTHROW(lElbowNode.getChildById(5));

        // left wrist
        auto &lWristNode = lElbowNode.getChildById(5);
        REQUIRE(lWristNode.getPos() == XSENSE_DUMMY_DATA.mWristL);
        REQUIRE(lWristNode.getChildrenCount() == 1);

        // left hand
        REQUIRE_NOTHROW(lWristNode.getChildById(6));

        // left hand
        auto &lHandNode = lWristNode.getChildById(6);
        REQUIRE(lHandNode.getPos() == XSENSE_DUMMY_DATA.mHandL);
        REQUIRE(lHandNode.getChildrenCount() == 0);


        // right shoulder
        auto &rShldrNode = neckNode.getChildById(7);
        REQUIRE(rShldrNode.getPos() == XSENSE_DUMMY_DATA.mShldrR);
        REQUIRE(rShldrNode.getChildrenCount() == 1);

        REQUIRE_NOTHROW(rShldrNode.getChildById(8));

        // right shoulder
        auto &rElbowNode = rShldrNode.getChildById(8);
        REQUIRE(rElbowNode.getPos() == XSENSE_DUMMY_DATA.mElbowR);
        REQUIRE(rElbowNode.getChildrenCount() == 1);

        // right wrist
        REQUIRE_NOTHROW(rElbowNode.getChildById(9));

        // right wrist
        auto &rWristNode = rElbowNode.getChildById(9);
        REQUIRE(rWristNode.getPos() == XSENSE_DUMMY_DATA.mWristR);
        REQUIRE(rWristNode.getChildrenCount() == 1);

        // right hand
        REQUIRE_NOTHROW(rWristNode.getChildById(10));

        // right hand
        auto &rHandNode = rWristNode.getChildById(10);
        REQUIRE(rHandNode.getPos() == XSENSE_DUMMY_DATA.mHandR);
        REQUIRE(rHandNode.getChildrenCount() == 0);

        // left hip
        auto &lHipNode = rootNode.getChildById(11);
        REQUIRE(lHipNode.getPos() == XSENSE_DUMMY_DATA.mHipL);
        REQUIRE(lHipNode.getChildrenCount() == 1);

        REQUIRE_NOTHROW(lHipNode.getChildById(12));

        // left knee
        auto &lKneeNode = lHipNode.getChildById(12);
        REQUIRE(lKneeNode.getPos() == XSENSE_DUMMY_DATA.mKneeL);
        REQUIRE(lKneeNode.getChildrenCount() == 1);

        REQUIRE_NOTHROW(lKneeNode.getChildById(20));

        // left ankle
        auto &lAnkle = lKneeNode.getChildById(20);
        REQUIRE(lAnkle.getPos() == XSENSE_DUMMY_DATA.mAnkleL);
        REQUIRE(lAnkle.getChildrenCount() == 1);

        REQUIRE_NOTHROW(lAnkle.getChildById(13));

        // left heel
        auto &lHeel = lAnkle.getChildById(13);
        REQUIRE(lHeel.getPos() == XSENSE_DUMMY_DATA.mHeelL);
        REQUIRE(lHeel.getChildrenCount() == 1);

        REQUIRE_NOTHROW(lHeel.getChildById(14));

        // left Toe
        auto &lToe = lHeel.getChildById(14);
        REQUIRE(lToe.getPos() == XSENSE_DUMMY_DATA.mToeL);
        REQUIRE(lToe.getChildrenCount() == 0);

        // right hip
        auto &rHipNode = rootNode.getChildById(15);
        REQUIRE(rHipNode.getPos() == XSENSE_DUMMY_DATA.mHipR);
        REQUIRE(rHipNode.getChildrenCount() == 1);

        REQUIRE_NOTHROW(rHipNode.getChildById(16));

        // right knee
        auto &rKneeNode = rHipNode.getChildById(16);
        REQUIRE(rKneeNode.getPos() == XSENSE_DUMMY_DATA.mKneeR);
        REQUIRE(rKneeNode.getChildrenCount() == 1);

        REQUIRE_NOTHROW(rKneeNode.getChildById(21));

        // right ankle
        auto &rAnkle = rKneeNode.getChildById(21);
        REQUIRE(rAnkle.getPos() == XSENSE_DUMMY_DATA.mAnkleR);
        REQUIRE(rAnkle.getChildrenCount() == 1);

        REQUIRE_NOTHROW(rAnkle.getChildById(17));

        // right heel
        auto &rHeel = rAnkle.getChildById(17);
        REQUIRE(rHeel.getPos() == XSENSE_DUMMY_DATA.mHeelR);
        REQUIRE(rHeel.getChildrenCount() == 1);

        REQUIRE_NOTHROW(rHeel.getChildById(18));

        // right Toe
        auto &rToe = rHeel.getChildById(18);
        REQUIRE(rToe.getPos() == XSENSE_DUMMY_DATA.mToeR);
        REQUIRE(rToe.getChildrenCount() == 0);
    }

    SECTION("Test the line data")
    {
        auto lines = skel.getLines();
        REQUIRE(lines.size() == 21);
        for(SkeletonLine &line : lines)
        {
            REQUIRE(line.start_id != line.end_id);
            REQUIRE(line.start != line.end);
        }
    }
}
