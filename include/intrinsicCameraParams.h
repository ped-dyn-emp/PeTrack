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
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef INTRINSICCAMERAPARAMS_H
#define INTRINSICCAMERAPARAMS_H

#include <QMetaType>
#include <opencv2/core.hpp>

/**
 * @brief struct used for storing intrinsic camera params.
 *
 * Central place to store this information and easier passing in methods.
 * Using value semantics.
 *
 * This struct includes the camera matrix from the pinhole camera model, which looks as follows
 *     [fx 0 cx
 *      0 fy cy
 *      0  0  1]
 * Note the 0 in the middle of the first row is an assumption made in OpenCV.
 * The skewness is always set to zero.
 *
 * It also includes the distortion parameters, which are regressed parameters of a distortion model
 * used to undistort the image into an image on which the pinhole camera is a good model.
 *
 * @warning cv::Mat only does shallow copies. If you set the member of this class, use cv::Mat::clone
 */
struct IntrinsicCameraParams
{
    IntrinsicCameraParams() {}
    IntrinsicCameraParams(const IntrinsicCameraParams &other);
    IntrinsicCameraParams(IntrinsicCameraParams &&other) = default;
    IntrinsicCameraParams &operator=(const IntrinsicCameraParams &other);
    IntrinsicCameraParams &operator=(IntrinsicCameraParams &&other) = default;
    ~IntrinsicCameraParams()                                        = default;

    cv::Mat cameraMatrix      = cv::Mat::eye(cv::Size(3, 3), CV_64F);
    cv::Mat distortionCoeffs  = cv::Mat::zeros(1, 14, CV_32F);
    double  reprojectionError = std::numeric_limits<double>::quiet_NaN();

    double getCx() const { return cameraMatrix.at<double>(0, 2); }
    void   setCx(double val) { cameraMatrix.at<double>(0, 2) = val; }

    double getCy() const { return cameraMatrix.at<double>(1, 2); }
    void   setCy(double val) { cameraMatrix.at<double>(1, 2) = val; }

    double getFx() const { return cameraMatrix.at<double>(0, 0); }
    void   setFx(double val) { cameraMatrix.at<double>(0, 0) = val; }

    double getFy() const { return cameraMatrix.at<double>(1, 1); }
    void   setFy(double val) { cameraMatrix.at<double>(1, 1) = val; }


    friend bool operator==(const IntrinsicCameraParams &lhs, const IntrinsicCameraParams &rhs)
    {
        const bool eqCamMat = cv::countNonZero(lhs.cameraMatrix != rhs.cameraMatrix) == 0;
        const bool eqDist   = cv::countNonZero(lhs.distortionCoeffs != rhs.distortionCoeffs) == 0;

        return eqCamMat && eqDist && (lhs.reprojectionError == rhs.reprojectionError);
    }

    friend bool operator!=(const IntrinsicCameraParams &lhs, const IntrinsicCameraParams &rhs) { return !(lhs == rhs); }
};

Q_DECLARE_METATYPE(IntrinsicCameraParams)


#endif // INTRINSICCAMERAPARAMS_H
