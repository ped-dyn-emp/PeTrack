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
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <catch2/catch.hpp>

#include "skeletonTreeFactory.h"
#include "skeletonTree.h"

const XSenseStruct  XSENSE_DUMMY_DATA = {
    {0,0,0},
    {1,1,1},
    {2,2,2},
    {3,3,3},
    {4,4,4},
    {5,5,5},
    {6,6,6},
    {7,7,7},
    {8,8,8},
    {9,9,9},
    {10,10,10},
    {11,11,11},
    {12,12,12},
    {13,13,13},
    {14,14,14},
    {15,15,15},
    {16,16,16},
    {17,17,17},
    {18,18,18},
    {19,19,19},
    {20,20,20},
};


TEST_CASE("SkeletonTree for XSenseData is built") {
    SkeletonTree skel = SkeletonTreeFactory::generateTree(XSENSE_DUMMY_DATA);


    SECTION( "Check all bone connections and positions" ) {
        auto& rootNode = skel.getRoot();
        REQUIRE(rootNode.getPos() == XSENSE_DUMMY_DATA.mRoot);
        REQUIRE(rootNode.getChildrenCount() == 3);
        //own id
        REQUIRE_THROWS(rootNode.getChildById(0));
        //does not exist
        REQUIRE_THROWS(rootNode.getChildById(100));
        //neck
        REQUIRE_NOTHROW(rootNode.getChildById(1));
        //left hip
        REQUIRE_NOTHROW(rootNode.getChildById(11));
        //right hip
        REQUIRE_NOTHROW(rootNode.getChildById(15));

        //check neck next
        auto& neckNode = rootNode.getChildById(1);
        REQUIRE(neckNode.getPos() == XSENSE_DUMMY_DATA.mNeck);
        REQUIRE(neckNode.getChildrenCount() == 3);

        //head
        REQUIRE_NOTHROW(neckNode.getChildById(2));
        //left shoulder
        REQUIRE_NOTHROW(neckNode.getChildById(3));
        //right shoulder
        REQUIRE_NOTHROW(neckNode.getChildById(7));

        //head
        auto& headNode = neckNode.getChildById(2);
        REQUIRE(headNode.getPos() == XSENSE_DUMMY_DATA.mHeadTop);
        REQUIRE(headNode.getChildrenCount() == 0);

        //left shoulder
        auto& lShldrNode = neckNode.getChildById(3);
        REQUIRE(lShldrNode.getPos() == XSENSE_DUMMY_DATA.mShldrL);
        REQUIRE(lShldrNode.getChildrenCount() == 1);

        REQUIRE_NOTHROW(lShldrNode.getChildById(4));

        //left shoulder
        auto& lElbowNode = lShldrNode.getChildById(4);
        REQUIRE(lElbowNode.getPos() == XSENSE_DUMMY_DATA.mElbowL);
        REQUIRE(lElbowNode.getChildrenCount() == 1);

        //left wrist
        REQUIRE_NOTHROW(lElbowNode.getChildById(5));

        //left wrist
        auto& lWristNode = lElbowNode.getChildById(5);
        REQUIRE(lWristNode.getPos() == XSENSE_DUMMY_DATA.mWristL);
        REQUIRE(lWristNode.getChildrenCount() == 1);

        //left hand
        REQUIRE_NOTHROW(lWristNode.getChildById(6));

        //left hand
        auto& lHandNode = lWristNode.getChildById(6);
        REQUIRE(lHandNode.getPos() == XSENSE_DUMMY_DATA.mHandL);
        REQUIRE(lHandNode.getChildrenCount() == 0);


        //right shoulder
        auto& rShldrNode = neckNode.getChildById(7);
        REQUIRE(rShldrNode.getPos() == XSENSE_DUMMY_DATA.mShldrR);
        REQUIRE(rShldrNode.getChildrenCount() == 1);

        REQUIRE_NOTHROW(rShldrNode.getChildById(8));

        //right shoulder
        auto& rElbowNode = rShldrNode.getChildById(8);
        REQUIRE(rElbowNode.getPos() == XSENSE_DUMMY_DATA.mElbowR);
        REQUIRE(rElbowNode.getChildrenCount() == 1);

        //right wrist
        REQUIRE_NOTHROW(rElbowNode.getChildById(9));

        //right wrist
        auto& rWristNode = rElbowNode.getChildById(9);
        REQUIRE(rWristNode.getPos() == XSENSE_DUMMY_DATA.mWristR);
        REQUIRE(rWristNode.getChildrenCount() == 1);

        //right hand
        REQUIRE_NOTHROW(rWristNode.getChildById(10));

        //right hand
        auto& rHandNode = rWristNode.getChildById(10);
        REQUIRE(rHandNode.getPos() == XSENSE_DUMMY_DATA.mHandR);
        REQUIRE(rHandNode.getChildrenCount() == 0);

        //left hip
        auto& lHipNode = rootNode.getChildById(11);
        REQUIRE(lHipNode.getPos() == XSENSE_DUMMY_DATA.mHipL);
        REQUIRE(lHipNode.getChildrenCount() == 1 );

        REQUIRE_NOTHROW(lHipNode.getChildById(12));

        //left knee
        auto& lKneeNode = lHipNode.getChildById(12);
        REQUIRE(lKneeNode.getPos() == XSENSE_DUMMY_DATA.mKneeL);
        REQUIRE(lKneeNode.getChildrenCount() == 1 );

        REQUIRE_NOTHROW(lKneeNode.getChildById(13));

        //left heel
        auto& lHeelNode = lKneeNode.getChildById(13);
        REQUIRE(lKneeNode.getPos() == XSENSE_DUMMY_DATA.mKneeL);
        REQUIRE(lKneeNode.getChildrenCount() == 1 );

        REQUIRE_NOTHROW(lHeelNode.getChildById(14));

        //left toe
        auto& lToeNode = lHeelNode.getChildById(14);
        REQUIRE(lToeNode.getPos() == XSENSE_DUMMY_DATA.mToeL);
        REQUIRE(lToeNode.getChildrenCount() == 0 );

        //right hip
        auto& rHipNode = rootNode.getChildById(15);
        REQUIRE(rHipNode.getPos() == XSENSE_DUMMY_DATA.mHipR);
        REQUIRE(rHipNode.getChildrenCount() == 1 );

        REQUIRE_NOTHROW(rHipNode.getChildById(16));

        //right knee
        auto& rKneeNode = rHipNode.getChildById(16);
        REQUIRE(rKneeNode.getPos() == XSENSE_DUMMY_DATA.mKneeR);
        REQUIRE(rKneeNode.getChildrenCount() == 1 );

        REQUIRE_NOTHROW(rKneeNode.getChildById(17));

        //right heel
        auto& rHeelNode = rKneeNode.getChildById(17);
        REQUIRE(rKneeNode.getPos() == XSENSE_DUMMY_DATA.mKneeR);
        REQUIRE(rKneeNode.getChildrenCount() == 1 );

        REQUIRE_NOTHROW(rHeelNode.getChildById(18));

        //right toe
        auto& rToeNode = rHeelNode.getChildById(18);
        REQUIRE(rToeNode.getPos() == XSENSE_DUMMY_DATA.mToeR);
        REQUIRE(rToeNode.getChildrenCount() == 0 );
    }

    SECTION("Test the line data"){
        auto lines = skel.getLines();
        REQUIRE(lines.size() == 18);
        for(SkeletonLine& line : lines){
            REQUIRE(line.start_id != line.end_id);
            REQUIRE(line.start != line.end);
        }
    }
}
