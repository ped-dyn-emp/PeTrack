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

#ifndef EXTRCALIBRATION_H
#define EXTRCALIBRATION_H

#include "extrinsicParameters.h"

#include <QDomElement>
#include <QString>
#include <array>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <optional>
#include <vector>

class Petrack;
class Control;
class PersonStorage;

class ReprojectionError
{
private:
    bool   mValid               = false;
    double mPointHeightAvg      = 0;
    double mPointHeightStdDev   = 0;
    double mPointHeightVariance = 0;
    double mPointHeightMax      = 0;

    double mDefaultHeightAvg      = 0;
    double mDefaultHeightStdDev   = 0;
    double mDefaultHeightVariance = 0;
    double mDefaultHeightMax      = 0;

    double mPixelAvg      = 0;
    double mPixelStdDev   = 0;
    double mPixelVariance = 0;
    double mPixelMax      = 0;

    double mUsedDefaultHeight = 0;

public:
    ReprojectionError() = default;

    ReprojectionError(
        double pointHeightAvg,
        double pointHeightStdDev,
        double pointHeightVariance,
        double pointHeightMax,
        double defaultHeightAvg,
        double defaultHeightStdDev,
        double defaultHeightVariance,
        double defaultHeightMax,
        double pixelAvg,
        double pixelStdDev,
        double pixelVariance,
        double pixelMax,
        double defaultHeight) :
        mPointHeightAvg(pointHeightAvg),
        mPointHeightStdDev(pointHeightStdDev),
        mPointHeightVariance(pointHeightVariance),
        mPointHeightMax(pointHeightMax),
        mDefaultHeightAvg(defaultHeightAvg),
        mDefaultHeightStdDev(defaultHeightStdDev),
        mDefaultHeightVariance(defaultHeightVariance),
        mDefaultHeightMax(defaultHeightMax),
        mPixelAvg(pixelAvg),
        mPixelStdDev(pixelStdDev),
        mPixelVariance(pixelVariance),
        mPixelMax(pixelMax),
        mUsedDefaultHeight(defaultHeight)
    {
        auto data = getData();
        mValid    = !std::any_of(data.begin(), data.end(), [](double a) { return !std::isfinite(a) || a < 0; });
    }

    void getXml(QDomElement &elem);
    void setXml(QDomElement &elem) const;

    double pointHeightAvg() const;
    double pointHeightStdDev() const;
    double pointHeightVariance() const;
    double pointHeightMax() const;

    double defaultHeightAvg() const;
    double defaultHeightStdDev() const;
    double defaultHeightVariance() const;
    double defaultHeightMax() const;

    double pixelAvg() const;
    double pixelStdDev() const;
    double pixelVariance() const;
    double pixelMax() const;

    double usedDefaultHeight() const;

    std::array<double, 13> getData() const
    {
        return {
            mPointHeightAvg,
            mPointHeightStdDev,
            mPointHeightVariance,
            mPointHeightMax,
            mDefaultHeightAvg,
            mDefaultHeightStdDev,
            mDefaultHeightVariance,
            mDefaultHeightMax,
            mPixelAvg,
            mPixelStdDev,
            mPixelVariance,
            mPixelMax,
            mUsedDefaultHeight};
    }

    bool isValid() const { return mValid; }
};


/**
 * @brief The ExtrCalibration class manages the extrinsic calibration
 *
 * If the aperture of our camera is not the origin of our world coordinate
 * system, we need to know the position of the camera in space to properly
 * reason about the position of points on the image plane in the real world.
 * To estimate the translation and rotation of the camera with respect to a
 * chosen world coordinate system with the help of a few specified points is
 * called the Perspective-n-Point problem. This class loads such a set of points
 * and solves PnP with the help of OpenCV.
 */
class ExtrCalibration
{
private:
    Petrack       *mMainWindow;
    Control       *mControlWidget;
    PersonStorage &mPersonStorage;

    std::vector<cv::Point3f> points3D;
    std::vector<cv::Point2f> points2D;

    float camHeight;

    ReprojectionError reprojectionError;
    QString           mExtrCalibFile;

public:
    ExtrCalibration(PersonStorage &storage);
    ~ExtrCalibration();
    void                               setMainWindow(Petrack *mw);
    bool                               isEmptyExtrCalibFile();
    bool                               isSetExtrCalib() const { return true; }
    void                               setExtrCalibFile(const QString &f);
    QString                            getExtrCalibFile();
    std::optional<ExtrinsicParameters> openExtrCalibFile();
    std::optional<ExtrinsicParameters> loadExtrCalibFile();
    bool                               saveExtrCalibPoints();
    std::optional<ExtrinsicParameters> fetch2DPoints();
    std::optional<ExtrinsicParameters> calibExtrParams();
    bool                               calcReprojectionError(const ExtrinsicParameters &extrParams);
    virtual cv::Point2f                getImagePoint(cv::Point3f p3d) const;
    virtual cv::Point2f                getImagePoint(cv::Point3f p3d, const ExtrinsicParameters &extrParams) const;

    cv::Point3f get3DPoint(const cv::Point2f &p2d, double h) const;
    cv::Point3f get3DPoint(const cv::Point2f &p2d, double h, const ExtrinsicParameters &extrParams) const;
    cv::Point3f transformRT(cv::Point3f p);
    cv::Vec3d   camToWorldRotation(const cv::Vec3d &vec) const;
    bool        isOutsideImage(cv::Point2f p2d) const;
    inline bool isOutsideImage(cv::Point3f p3d) { return isOutsideImage(getImagePoint(p3d)); }
    inline std::vector<cv::Point3f>  get3DList() const { return points3D; }
    inline std::vector<cv::Point3f> &get3DList() { return points3D; }
    inline void                      set3DList(std::vector<cv::Point3f> list3D) { this->points3D = list3D; }
    inline std::vector<cv::Point2f>  get2DList() const { return points2D; }
    inline std::vector<cv::Point2f> &get2DList() { return points2D; }
    inline void                      set2DList(std::vector<cv::Point2f> list2D) { this->points2D = list2D; }
    inline float                     getCamHeight() const { return camHeight; }
    inline void                      setCamHeight(float cHeight) { this->camHeight = cHeight; }
    ReprojectionError                getReprojectionError();


    void setXml(QDomElement &elem);
    void getXml(QDomElement &elem);
};

#endif // EXTRCALIBRATION_H
