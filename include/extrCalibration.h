/*
 * PeTrack - Software for tracking pedestrians movement in videos
 * Copyright (C) 2010-2020 Forschungszentrum Jülich GmbH,
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

#include <iostream>
#include <vector>

#include <QString>
#include <QVector>

//#include <opencv.hpp>

//#include "opencv/cxcore.h"
//#include "opencv/cvaux.h"
#include "opencv2/opencv.hpp"




class Petrack;
class Control;

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

//    Mat mRotation;
//    Mat mTranslation;
//    Mat mCamera;
//    Mat mDistCoeffs;

    double *rotation_matrix;
    double *translation_vector;
    double *translation_vector2;

    double *camValues;
    double *distValues;
    bool isExtCalib;
    float camHeight;

    QVector<double> reprojectionError;
    QString mExtrCalibFile;
    void init();

public:

    ExtrCalibration();
    ~ExtrCalibration();
    void setMainWindow(Petrack *mw);
    bool isEmptyExtrCalibFile();
    bool isSetExtrCalib();
    void setExtrCalibFile(const QString &f);
    QString getExtrCalibFile();
    bool openExtrCalibFile();
    bool loadExtrCalibFile();
    bool saveExtrCalibPoints();
    bool fetch2DPoints();
    void calibExtrParams();
    bool calcReprojectionError();
    cv::Point2f getImagePoint(cv::Point3f p3d);
    cv::Point3f get3DPoint(cv::Point2f p2d, double h);
    cv::Point3f transformRT(cv::Point3f p);
    bool isOutsideImage(cv::Point2f p2d);
    inline bool isOutsideImage(cv::Point3f p3d)
    {
        return isOutsideImage(getImagePoint(p3d));
    }
    inline std::vector<cv::Point3f> get3DList()
    {
        return points3D;
    }
    inline void set3DList(std::vector<cv::Point3f> list3D)
    {
        this->points3D = list3D;
    }
    inline std::vector<cv::Point2f> get2DList()
    {
        return points2D;
    }
    inline void set2DList(std::vector<cv::Point2f> list2D)
    {
        this->points2D = list2D;
    }
    inline float getCamHeight()
    {
        return camHeight;
    }
    inline void setCamHeight(float cHeight)
    {
        this->camHeight = cHeight;
    }
    inline QVector<double> getReprojectionError()
    {
        if( reprojectionError.size() < 12 )
            calcReprojectionError();
        return this->reprojectionError;
    }

};

#endif // EXTRCALIBRATION_H
