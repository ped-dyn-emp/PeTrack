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

#include "calibFilter.h"

#include <opencv2/calib3d.hpp>
#include <opencv2/imgproc.hpp>


Parameter<IntrinsicCameraParams> &CalibFilter::getCamParams()
{
    return mCamParams;
}

CalibFilter::CalibFilter() : Filter(), mCamParams(this)
{
    IntrinsicCameraParams params;

    constexpr double defaultFocalLength = 881;
    constexpr double defaultCx          = 551.5;
    constexpr double defaultCy          = 383.5;
    params.setFx(defaultFocalLength);
    params.setFy(defaultFocalLength);
    params.setCx(defaultCx);
    params.setCy(defaultCy);

    // distortionCoeffs already 0-initalized
    mCamParams.setValue(params);
}

/**
 * @brief Undistorts the image.
 *
 * This method calculates and caches the mapping for undistortion.
 * The mapping is applied to the input image.
 *
 * @param img[in]
 * @param res[out]
 * @return undistorted image
 */
cv::Mat CalibFilter::act(cv::Mat &img, cv::Mat &res)
{
    if(this->changed() || map1.size() != img.size())
    {
        cv::Mat camera;
        // conversion to CV_32F such that regression tests don't fail
        mCamParams.getValue().cameraMatrix.convertTo(camera, CV_32F);
        const cv::Mat dist = mCamParams.getValue().distortionCoeffs;

        cv::initUndistortRectifyMap(
            camera, dist, cv::Mat_<double>::eye(3, 3), camera, img.size(), CV_16SC2, map1, map2);
    }

    cv::remap(img, res, map1, map2, cv::INTER_LINEAR, cv::BORDER_CONSTANT);
    return res;
}
