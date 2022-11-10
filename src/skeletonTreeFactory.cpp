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
#include "skeletonTreeFactory.h"

#include "vector.h"

#include <opencv2/core/matx.hpp>

/**
 * @brief Static method to construct a SkeletonTree.
 *
 * This method constructs a @see SkeletonTree using data given from the XSense System.
 *
 * @param[in] points  The struct with the data from the xsense system.
 *
 * @return The constructed Skeleton.
 *
 */
SkeletonTree SkeletonTreeFactory::generateTree(const XSenseStruct &points)
{
    // Start from the root
    SkeletonNode  root(0, points.mRoot);
    SkeletonNode &neck = root.addChild(SkeletonNode(1, points.mNeck1));

    // continue with the neck
    SkeletonNode &neck2 = neck.addChild(SkeletonNode(19, points.mNeck2));
    neck2.addChild(SkeletonNode(2, points.mHeadTop));

    // add the left arm
    SkeletonNode &lShoulder = neck.addChild(SkeletonNode(3, points.mShldrL));
    SkeletonNode &lElbow    = lShoulder.addChild(SkeletonNode(4, points.mElbowL));
    SkeletonNode &lWrist    = lElbow.addChild(SkeletonNode(5, points.mWristL));
    lWrist.addChild(SkeletonNode(6, points.mHandL));

    // add the right arm
    SkeletonNode &rShoulder = neck.addChild(SkeletonNode(7, points.mShldrR));
    SkeletonNode &rElbow    = rShoulder.addChild(SkeletonNode(8, points.mElbowR));
    SkeletonNode &rWrist    = rElbow.addChild(SkeletonNode(9, points.mWristR));
    rWrist.addChild(SkeletonNode(10, points.mHandR));

    // add the left leg
    SkeletonNode &lHip   = root.addChild(SkeletonNode(11, points.mHipL));
    SkeletonNode &lKnee  = lHip.addChild(SkeletonNode(12, points.mKneeL));
    SkeletonNode &lAnkle = lKnee.addChild(SkeletonNode(20, points.mAnkleL));
    SkeletonNode &lHeel  = lAnkle.addChild(SkeletonNode(13, points.mHeelL));
    lHeel.addChild(SkeletonNode(14, points.mToeL));

    SkeletonNode &rHip   = root.addChild(SkeletonNode(15, points.mHipR));
    SkeletonNode &rKnee  = rHip.addChild(SkeletonNode(16, points.mKneeR));
    SkeletonNode &rAnkle = rKnee.addChild(SkeletonNode(21, points.mAnkleR));
    SkeletonNode &rHeel  = rAnkle.addChild(SkeletonNode(17, points.mHeelR));
    rHeel.addChild(SkeletonNode(18, points.mToeR));


    // calculate view direction of the head
    cv::Point3f headUp      = points.mHeadTop - points.mNeck2;
    cv::Point3f rightVector = points.mEarR - points.mEarL;
    // the direction is calculated using the cross product
    cv::Point3f dir = headUp.cross(rightVector);

    SkeletonTree skeleton(std::move(root), Vec3F(dir));

    return skeleton;
}
