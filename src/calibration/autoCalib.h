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

#ifndef AUTOCALIB_H
#define AUTOCALIB_H

#include "intrinsicCameraParams.h"

#include <QString>
#include <QStringList>
#include <opencv2/core/types.hpp>
#include <optional>

class Petrack;

namespace calib
{
struct Sample
{
    std::vector<cv::Point2f> corners;
    double                   area;
    double                   skew;
    std::vector<bool>        coverage;

    double getDifference(const Sample &other) const
    {
        double diff = 0;
        diff += cv::abs(area - other.area);
        diff += cv::abs(skew - other.skew);
        return diff;
    }
};
} // namespace calib


/**
 * @brief Class for intrinsic calibration
 *
 * If we know certain parameters of the camera, such as focal
 * length, radial and tangent distortion etc. we can calculate
 * where a point in the real world will be projected onto the
 * image plane and with additional information (e.g. person
 * height), the inverse is true as well. Determining these
 * parameters is called intrinsic calibration.
 *
 * This class runs the intrinsic calibration of the camera
 * based on a chessboard-pattern. The dimensions of the chessboard
 * are settable by the user, but no other pattern is currently supported.
 */
class AutoCalib
{
public:
    AutoCalib();
    ~AutoCalib();

    void        setMainWindow(Petrack *mw);
    bool        isEmptyCalibFiles();
    void        addCalibFile(const QString &f);
    QString     getCalibFile(int i);
    QStringList getCalibFiles();
    QString     getCalibVideo();
    void        setCalibVideo(const QString &v);
    bool        openCalibVideo();
    void        setCalibFiles(const QStringList &fl);
    bool        openCalibFiles(); // return true if at least one file is selected

    inline void setBoardSizeX(int i) // 6
    {
        mBoardSizeX = i;
    }
    inline int getBoardSizeX() const // 6
    {
        return mBoardSizeX;
    }
    inline void setBoardSizeY(int i) // 8 oder 9
    {
        mBoardSizeY = i;
    }
    inline int getBoardSizeY() const // 8 oder 9
    {
        return mBoardSizeY;
    }
    inline void setSquareSize(float d) // in cm
    {
        mSquareSize = d;
    }
    inline float getSquareSize() const // in cm
    {
        return mSquareSize;
    }
    inline void    setLastDir(QString path) { mLastDir = path; }
    inline QString getLastDir() const { return mLastDir; }

    virtual std::optional<IntrinsicModelsParameters> autoCalib(bool quadAspectRatio, bool fixCenter, bool tangDist);
    void                                             checkParamPlausibility(IntrinsicCameraParams &modelParams);

private:
    std::vector<cv::Point2f> getOuterChessboardCorners(const std::vector<cv::Point2f> &corners, cv::Size &boardSize);
    double                   calcInnerAreaOfChessboard(std::vector<cv::Point2f> &outerCorners);
    double                   calcSkewOfChessboard(std::vector<cv::Point2f> &outerCorners);
    int                      compareCoverage(std::vector<bool> &totalCovered, std::vector<bool> &covered);
    std::vector<bool>        mergeCoverages(std::vector<bool> &totalCovered, std::vector<bool> &covered);
    int                      getCovered(std::vector<bool> covered);
    std::vector<bool>        calcXYCoverage(std::vector<cv::Point2f> &corners, cv::Size imageSize, int gridSize);
    bool isGoodSample(std::vector<calib::Sample> &goodSamples, calib::Sample &sample, std::vector<bool> &totalCoverage);
    void findGoodCalibrationSamplesFromVideo(cv::Size boardSize);
    int  runCalibration(
         std::vector<std::vector<cv::Point2f>> corners,
         cv::Size                              img_size,
         cv::Size                              board_size,
         float                                 square_size,
         float                                 aspect_ratio,
         int                                   flags,
         cv::Mat                              &camera_matrix,
         cv::Mat                              &distortion_coeffs,
         double                               *reproj_errs);

    Petrack    *mMainWindow;
    QStringList mCalibFiles;
    QString     mCalibVideo;
    int         mBoardSizeX, mBoardSizeY;
    float       mSquareSize;
    QString     mLastDir;
};

#endif
