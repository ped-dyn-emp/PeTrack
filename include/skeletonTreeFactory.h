/*
 * PeTrack - Software for tracking pedestrians movement in videos
 * Copyright (C) 2010-2021 Forschungszentrum Jülich GmbH,
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
    cv::Point3f mRoot; /***< pSacrum used as sacrum and root. Id is 0. */

    cv::Point3f mNeck; /**< pC7SpinalProcess used as atlas bone. Id is 1. */

    cv::Point3f mHeadTop; /**< pTopOfHead used as the top of the head. Id is 2. */

    cv::Point3f mShldrR; /**< pRightAcromion used as right shoulder. Id is 7. */
    cv::Point3f mShldrL; /**< pLeftAcromion used as left shoulder. Id is 3. */

    cv::Point3f mElbowR; /**< pRightOlecranon used as right elbow. Id is 8. */
    cv::Point3f mElbowL; /**< pLeftOlecranon used as left elbow. Id is 4. */

    cv::Point3f mWristR; /**< pRightStyloid used as right wrist. Id is 9. */
    cv::Point3f mWristL; /**< pLeftStyloid used as left wrist. Id is 5. */

    cv::Point3f mHandR; /**< pRightTopOfHand used as the right hand. Id is 10. */
    cv::Point3f mHandL; /**< pLeftTopOfHand used as the left hand. Id is 6.*/

    cv::Point3f mHipR; /**< pRightIschialTub used as the right hip. Id is 15. */
    cv::Point3f mHipL; /**< pLeftIschialTub used as the left hip. Id is 11. */

    cv::Point3f mKneeR; /**< pRightPatella used as the right knee. Id is 16. */
    cv::Point3f mKneeL; /**< pLeftPatella used as the left knee. Id is 12. */

    cv::Point3f mHeelR; /**< pRightHeelFoot used as the right heel. Id is 17. */
    cv::Point3f mHeelL; /**< pLeftHeelFoot used as the left heel. Id is 13 .*/

    cv::Point3f mToeR; /**< pRightHeel used as the right of the right foot. Id is 18. */
    cv::Point3f mToeL; /**< pLeftHeel used as the right of the left foot. Id is 14. */

    cv::Point3f mEarR; /**< pRightAuricularis used as the right ear. No id. */
    cv::Point3f mEarL; /**< pLeftAuricularis used as the left ear. No id. */
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
