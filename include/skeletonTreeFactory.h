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

#ifndef SKELETONTREE_FACTORY_H
#define SKELETONTREE_FACTORY_H

#include "skeletonTree.h"

#include <opencv2/core/types.hpp>

/**
 * @brief This struct defines all the points needed to construct a skeleton from a person.
 *
 * This struct contains all necessary points to construct an skeleton from it.
 * All points are used as node  points except the mEarL and mEarR elements.
 * These are used together with mNeck and mHeadTop to calculate the direction the head is facing to.
 * @see SkeletonTreeFactory
 * @see SkeletonTree
 **/
struct XSenseStruct
{
    cv::Point3f mRoot; /**< used as root. Id is 0. */

    cv::Point3f mNeck1; /**< used as atlas bone. Id is 1. */
    cv::Point3f mNeck2; /**< used as atlas bone. Id is 19. */

    cv::Point3f mHeadTop; /**< used as the top of the head. Id is 2. */

    cv::Point3f mShldrR; /**< used as right shoulder. Id is 7. */
    cv::Point3f mShldrL; /**< used as left shoulder. Id is 3. */

    cv::Point3f mElbowR; /**< used as right elbow. Id is 8. */
    cv::Point3f mElbowL; /**< used as left elbow. Id is 4. */

    cv::Point3f mWristR; /**< used as right wrist. Id is 9. */
    cv::Point3f mWristL; /**< used as left wrist. Id is 5. */

    cv::Point3f mHandR; /**< used as the right hand. Id is 10. */
    cv::Point3f mHandL; /**< used as the left hand. Id is 6.*/

    cv::Point3f mHipR; /**< used as the right hip. Id is 15. */
    cv::Point3f mHipL; /**< used as the left hip. Id is 11. */

    cv::Point3f mKneeR; /**< used as the right knee. Id is 16. */
    cv::Point3f mKneeL; /**< used as the left knee. Id is 12. */

    cv::Point3f mAnkleR; /**< used as the right ankle. Id is 20. */
    cv::Point3f mAnkleL; /**< used as the left ankle. Id is 21. */

    cv::Point3f mHeelR; /**< used as the right heel. Id is 17. */
    cv::Point3f mHeelL; /**< used as the left heel. Id is 13 .*/

    cv::Point3f mToeR; /**< used as the right of the right foot. Id is 18. */
    cv::Point3f mToeL; /**< used as the right of the left foot. Id is 14. */

    cv::Point3f mEarR; /**< used as the right ear. No id. */
    cv::Point3f mEarL; /**< used as the left ear. No id. */
};

/** @brief This class is used to construct a SkeletonTree.
 *
 *  This class contains static methods for constructing a SkeletonTree class.
 *  Since the given points can differ with different sources the appropiate structs should be defined for the source.
 *  Using that struct as a parameter the generateTree can be overloaded.
 *  @see SkeletonTree
 **/
class SkeletonTreeFactory
{
public:
    static SkeletonTree generateTree(const XSenseStruct &points);
};

#endif
