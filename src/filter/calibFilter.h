/*
 * PeTrack - Software for tracking pedestrians movement in videos
 * Copyright (C) 2025 Forschungszentrum Jülich GmbH, IAS-7
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

#ifndef CALIBFILTER_H
#define CALIBFILTER_H

#include "filter.h"
#include "intrinsicCameraParams.h"

/**
 * @brief Undistortion filter
 *
 * This class is a filter which undistorts the image using the camera matrix from intrinsic calibration.
 * It caches the mapping from distorted to undistorted image.
 */
class CalibFilter : public Filter
{
private:
    Parameter<IntrinsicCameraParams> mCamParams;

    cv::Mat map1;
    cv::Mat map2;

public:
    CalibFilter();

    cv::Mat act(cv::Mat &img, cv::Mat &res);

    Parameter<IntrinsicCameraParams> &getCamParams();
    cv::Mat                           getMap1();
    cv::Mat                           getMap2();
};

#endif
