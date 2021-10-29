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

#ifndef EXTRCALIBRATION_H
#define EXTRCALIBRATION_H

#include <QDomElement>
#include <QString>
#include <array>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <vector>

class Petrack;
class Control;


class ReprojectionError
{
private:
    bool   mValid = false;
    double mPointHeightAvg;
    double mPointHeightStdDev;
    double mPointHeightVariance;
    double mPointHeightMax;

    double mDefaultHeightAvg;
    double mDefaultHeightStdDev;
    double mDefaultHeightVariance;
    double mDefaultHeightMax;

    double mPixelAvg;
    double mPixelStdDev;
    double mPixelVariance;
    double mPixelMax;

    double mUsedDefaultHeight;

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
    Petrack *mMainWindow;
    Control *mControlWidget;

    std::vector<cv::Point3f> points3D;
    std::vector<cv::Point2f> points2D;

    double *rotation_matrix;
    double *translation_vector;
    double *translation_vector2;

    double *camValues;
    double *distValues;
    bool    isExtCalib;
    float   camHeight;

    ReprojectionError reprojectionError;
    QString           mExtrCalibFile;
    void              init();

public:
    ExtrCalibration();
    ~ExtrCalibration();
    void                            setMainWindow(Petrack *mw);
    bool                            isEmptyExtrCalibFile();
    bool                            isSetExtrCalib();
    void                            setExtrCalibFile(const QString &f);
    QString                         getExtrCalibFile();
    bool                            openExtrCalibFile();
    bool                            loadExtrCalibFile();
    bool                            saveExtrCalibPoints();
    bool                            fetch2DPoints();
    void                            calibExtrParams();
    bool                            calcReprojectionError();
    virtual cv::Point2f             getImagePoint(cv::Point3f p3d);
    cv::Point3f                     get3DPoint(cv::Point2f p2d, double h);
    cv::Point3f                     transformRT(cv::Point3f p);
    bool                            isOutsideImage(cv::Point2f p2d);
    inline bool                     isOutsideImage(cv::Point3f p3d) { return isOutsideImage(getImagePoint(p3d)); }
    inline std::vector<cv::Point3f> get3DList() { return points3D; }
    inline void                     set3DList(std::vector<cv::Point3f> list3D) { this->points3D = list3D; }
    inline std::vector<cv::Point2f> get2DList() { return points2D; }
    inline void                     set2DList(std::vector<cv::Point2f> list2D) { this->points2D = list2D; }
    inline float                    getCamHeight() const { return camHeight; }
    inline void                     setCamHeight(float cHeight) { this->camHeight = cHeight; }
    inline ReprojectionError        getReprojectionError()
    {
        if(!reprojectionError.isValid())
        {
            calcReprojectionError();
        }
        return reprojectionError;
    }


    void setXml(QDomElement &elem);
    void getXml(QDomElement &elem);
};

#endif // EXTRCALIBRATION_H
