/*
 * PeTrack - Software for tracking pedestrians movement in videos
 * Copyright (C) 2024 Forschungszentrum Jülich GmbH, IAS-7
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
    IntrinsicCameraParams()
    {
        constexpr double defaultFocalLength = 881;
        constexpr double defaultCx          = 551.5;
        constexpr double defaultCy          = 383.5;
        setFx(defaultFocalLength);
        setFy(defaultFocalLength);
        setCx(defaultCx);
        setCy(defaultCy);

        // distortionCoeffs already 0-initalized
    }
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

    float getR2() const { return distortionCoeffs.at<float>(0); }
    void  setR2(float val) { distortionCoeffs.at<float>(0) = val; }

    float getR4() const { return distortionCoeffs.at<float>(1); }
    void  setR4(float val) { distortionCoeffs.at<float>(1) = val; }

    float getR6() const { return distortionCoeffs.at<float>(4); }
    void  setR6(float val) { distortionCoeffs.at<float>(4) = val; }

    float getTx() const { return distortionCoeffs.at<float>(2); }
    void  setTx(float val) { distortionCoeffs.at<float>(2) = val; }

    float getTy() const { return distortionCoeffs.at<float>(3); }
    void  setTy(float val) { distortionCoeffs.at<float>(3) = val; }

    float getK4() const { return distortionCoeffs.at<float>(5); }
    void  setK4(float val) { distortionCoeffs.at<float>(5) = val; }

    float getK5() const { return distortionCoeffs.at<float>(6); }
    void  setK5(float val) { distortionCoeffs.at<float>(6) = val; }

    float getK6() const { return distortionCoeffs.at<float>(7); }
    void  setK6(float val) { distortionCoeffs.at<float>(7) = val; }

    float getS1() const { return distortionCoeffs.at<float>(8); }
    void  setS1(float val) { distortionCoeffs.at<float>(8) = val; }

    float getS2() const { return distortionCoeffs.at<float>(9); }
    void  setS2(float val) { distortionCoeffs.at<float>(9) = val; }

    float getS3() const { return distortionCoeffs.at<float>(10); }
    void  setS3(float val) { distortionCoeffs.at<float>(10) = val; }

    float getS4() const { return distortionCoeffs.at<float>(11); }
    void  setS4(float val) { distortionCoeffs.at<float>(11) = val; }

    float getTaux() const { return distortionCoeffs.at<float>(12); }
    void  setTaux(float val) { distortionCoeffs.at<float>(12) = val; }

    float getTauy() const { return distortionCoeffs.at<float>(13); }
    void  setTauy(float val) { distortionCoeffs.at<float>(13) = val; }

    float getReprojectionError() const { return reprojectionError; }
    void  setReprojectionError(float val) { reprojectionError = val; }

    friend bool operator==(const IntrinsicCameraParams &lhs, const IntrinsicCameraParams &rhs)
    {
        const bool eqCamMat = cv::countNonZero(lhs.cameraMatrix != rhs.cameraMatrix) == 0;
        const bool eqDist   = cv::countNonZero(lhs.distortionCoeffs != rhs.distortionCoeffs) == 0;
        bool       eqReprError;
        if(qIsNaN(lhs.reprojectionError) && qIsNaN(rhs.reprojectionError))
        {
            eqReprError = true;
        }
        else if(!qIsNaN(lhs.reprojectionError) && !qIsNaN(rhs.reprojectionError))
        {
            eqReprError = lhs.reprojectionError == rhs.reprojectionError;
        }
        else
        {
            return false;
        }
        return eqCamMat && eqDist && eqReprError;
    }

    friend bool operator!=(const IntrinsicCameraParams &lhs, const IntrinsicCameraParams &rhs) { return !(lhs == rhs); }

    friend std::ostream &operator<<(std::ostream &os, IntrinsicCameraParams params)
    {
        os << "Camera params: " << std::endl;
        os << params.getFx() << " " << params.cameraMatrix.at<double>(0, 1) << " " << params.getCx() << std::endl;
        os << params.cameraMatrix.at<double>(1, 0) << " " << params.getFy() << " " << params.getCy() << std::endl;
        os << params.cameraMatrix.at<double>(2, 0) << " " << params.cameraMatrix.at<double>(2, 1) << " "
           << params.cameraMatrix.at<double>(2, 2) << std::endl;

        os << "Distortion params: " << std::endl;
        os << params.getR2() << " " << params.getR4() << " " << params.getTx() << " " << params.getTy() << " "
           << params.getR6() << " " << params.getK4() << " " << params.getK5() << " " << params.getK6() << " "
           << params.getS1() << " " << params.getS2() << " " << params.getS3() << " " << params.getS4() << " "
           << params.getTaux() << " " << params.getTauy() << std::endl;
        os << "Reprojection Error: " << params.reprojectionError << std::endl;
        return os;
    }
};

struct IntrinsicModelsParameters
{
    IntrinsicCameraParams oldModelParams;
    IntrinsicCameraParams extModelParams;

    friend bool operator==(const IntrinsicModelsParameters &lhs, const IntrinsicModelsParameters &rhs)
    {
        bool eqOldModel = lhs.oldModelParams == rhs.oldModelParams;
        bool eqExtModel = lhs.extModelParams == rhs.extModelParams;
        return eqOldModel && eqExtModel;
    }

    friend bool operator!=(const IntrinsicModelsParameters &lhs, const IntrinsicModelsParameters &rhs)
    {
        return !(lhs == rhs);
    }
};

Q_DECLARE_METATYPE(IntrinsicCameraParams)


#endif // INTRINSICCAMERAPARAMS_H
