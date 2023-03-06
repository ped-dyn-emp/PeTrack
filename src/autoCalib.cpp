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

#include "autoCalib.h"

#include "logger.h"
#include "pMessageBox.h"
#include "petrack.h"

#include <QApplication>
#include <QFileDialog>
#include <QFileInfo>
#include <QProgressDialog>
#include <opencv2/highgui.hpp>

/// definieren, wenn das Schachbrett im Mainwindow und nicht separat angezeigt werden soll:
/// fuehrt nach Calibration dazu dass play des originalvideos abstuerzt, insb wenn intr apply nicht ausgewaehlt war
#define SHOW_CALIB_MAINWINDOW

AutoCalib::AutoCalib()
{
    mMainWindow = nullptr;

    // 6x8 und 4.6cm passen zu dem Schachbrettmuster auf der Eisenplatte mit Griff
    mBoardSizeX = 6;
    mBoardSizeY = 8;    //{6, 9};  //{6, 8}; // passt zu meinem Schachbrett, was ich ausgedruckt habe
    mSquareSize = 4.6f; // 3.f; //5.25f;
}

AutoCalib::~AutoCalib() {}

void AutoCalib::setMainWindow(Petrack *mw)
{
    mMainWindow = mw;
}

bool AutoCalib::isEmptyCalibFiles()
{
    return mCalibFiles.isEmpty();
}

void AutoCalib::addCalibFile(const QString &f)
{
    mCalibFiles += f;
}

QString AutoCalib::getCalibFile(int i)
{
    if(i < mCalibFiles.size())
    {
        return mCalibFiles.at(i);
    }
    else
    {
        return QString();
    }
}

QStringList AutoCalib::getCalibFiles()
{
    return mCalibFiles;
}

void AutoCalib::setCalibFiles(const QStringList &fl)
{
    mCalibFiles = fl;
}

bool AutoCalib::openCalibFiles()
{
    if(mMainWindow)
    {
        static QString lastDir;

        if(!mCalibFiles.isEmpty())
        {
            lastDir = QFileInfo(mCalibFiles.first()).path();
        }
        QStringList calibFiles = QFileDialog::getOpenFileNames(
            mMainWindow,
            Petrack::tr("Open calibration sequence"),
            lastDir,
            "All supported types (*.bmp *.dib *.jpeg *.jpg *.jpe *.png *.pbm *.pgm *.ppm *.sr *.ras *.tiff *.tif *.exr "
            "*.jp2);;All files (*.*)");
        if(!calibFiles.isEmpty())
        {
            mCalibFiles = calibFiles;
            return true;
        }
    }
    return false;
}

/**
 * @brief Loads CalibFiles, detects Chessboard corners and calibrates with these
 *
 * This function loads the calib files specified by the user. In these images
 * chessboard corners are detected and refined (cv::sornerSubPix). If this
 * detection is successful at least in one image, the detected points are used
 * for the intrinsic calibration of the camera.
 *
 * @param quadAspectRatio whether to fix the aspect ratio
 * @param fixCenter whether to fix the center/focal point
 * @param tangDist whether to use non-zero tangential distortion
 * @param extModel whether to use "extended" calibration model
 *
 * @return new parameters or std::nullopt, if no calibration could be done
 */
std::optional<IntrinsicCameraParams>
AutoCalib::autoCalib(bool quadAspectRatio, bool fixCenter, bool tangDist, bool extModel)
{
    if(mMainWindow)
    {
        // no files are selected for calibration
        if(mCalibFiles.isEmpty())
        {
            PInformation(mMainWindow, Petrack::tr("Petrack"), Petrack::tr("At first you have to select files."));
            return std::nullopt;
        }

        cv::Size board_size(
            mBoardSizeX, mBoardSizeY);    //{6, 9};  //{6, 8}; // passt zu meinem Schachbrett, was ich ausgedruckt habe
        float square_size  = mSquareSize; // 5.25f; // 3.f;   // da 3x3cm hat Schachbrett, was ich ausgedruckt habe
        float aspect_ratio = 1.f;
        int   flags        = 0;
        std::vector<cv::Point2f>              corners;
        std::vector<std::vector<cv::Point2f>> image_points;
        cv::Mat                               camera_matrix = cv::Mat::eye(3, 3, CV_64F);
        cv::Mat                               dist_coeffs   = cv::Mat::zeros(1, 14, CV_64F);
        cv::Mat                               extr_params;
        double                                reproj_errs;
        cv::Mat                               view, view_gray;
        bool                                  found = false;
        cv::Mat                               origImg;
        cv::Size                              imgSize;

#ifdef SHOW_CALIB_MAINWINDOW
        if(!mMainWindow->getImg().empty())
        {
            origImg = mMainWindow->getImg().clone(); // must be cloned, because mIplImg will be deleted in updateImage
        }
#endif

        QProgressDialog progress(
            "Calculating intrinsic camera parameters...", "Abort calculation", 0, mCalibFiles.size(), mMainWindow);
        progress.setWindowModality(Qt::WindowModal); // blocks main window

        SPDLOG_INFO(
            "Search for chessboard pattern ({} x {}) with square size: {} cm...",
            board_size.width,
            board_size.height,
            square_size);
        bool min_one_pattern_found = false;
        // search for chessbord corners in every image
        for(int i = 0; i < mCalibFiles.size(); ++i)
        {
            progress.setValue(i);
            qApp->processEvents();
            if(progress.wasCanceled())
            {
                break;
            }


            // cannot load image
            view = cv::imread(mCalibFiles.at(i).toStdString(), cv::IMREAD_COLOR);
            if(view.empty())
            {
                progress.setValue(mCalibFiles.size());
                PCritical(
                    mMainWindow,
                    Petrack::tr("Petrack"),
                    Petrack::tr("Cannot load %1.\nTerminate Calibration.").arg(mCalibFiles.at(i)));
#ifdef SHOW_CALIB_MAINWINDOW
                // reset view to animation image
                if(!origImg.empty())
                {
                    mMainWindow->updateImage(origImg); // now the last view will be deleted
                }
#endif
                return std::nullopt;
            }

            // muss nur bei einem bild gemacht werden
            if(i == 0)
            {
                imgSize = cv::Size(view.rows, view.cols);
            }
            // search for chessboard corners
            found = findChessboardCorners(view, board_size, corners, cv::CALIB_CB_ADAPTIVE_THRESH);

            if(found)
            {
                // improve the found corners' coordinate accuracy
                cv::cvtColor(view, view_gray, cv::COLOR_BGR2GRAY);
                cv::cornerSubPix(
                    view_gray,
                    corners,
                    cv::Size(11, 11),
                    cv::Size(-1, -1),
                    cv::TermCriteria(CV_TERMCRIT_EPS + CV_TERMCRIT_ITER, 30, 0.1));

                image_points.push_back(corners);
                drawChessboardCorners(view, board_size, corners, found);

#ifndef SHOW_CALIB_MAINWINDOW
                namedWindow("img", CV_WINDOW_AUTOSIZE); // 0 wenn skalierbar sein soll
                imShow("img", view);
                // cvWaitKey( 0 ); // zahl statt null, wenn nach bestimmter zeit weitergegangen werden soll
#endif
#ifdef SHOW_CALIB_MAINWINDOW
                // show image in view to show calculation
                mMainWindow->updateImage(view);
#endif
                qApp->processEvents(); // to allow events and update sceen for viewing new image
                min_one_pattern_found = true;
            }
            else
            {
                SPDLOG_WARN("Calibration pattern not found in: {}", mCalibFiles.at(i));
            }
        }

        if(!min_one_pattern_found)
        {
            PWarning(
                mMainWindow,
                QString("Calibration failed"),
                QString("Chessboard pattern (%1x%2) not found in calibration files.")
                    .arg(board_size.width)
                    .arg(board_size.height));
            return std::nullopt;
        }
        // set flags for calibration
        if(quadAspectRatio)
        {
            flags |= CV_CALIB_FIX_ASPECT_RATIO; // durch setzen von aspect_ratio kann fix ascpect anders als 1:1
                                                // eingestellt werden
        }
        if(fixCenter)
        {
            flags |= CV_CALIB_FIX_PRINCIPAL_POINT;
        }
        if(!tangDist)
        {
            flags |= CV_CALIB_ZERO_TANGENT_DIST;
        }
        if(extModel)
        {
            flags |= CV_CALIB_RATIONAL_MODEL + CV_CALIB_THIN_PRISM_MODEL + CV_CALIB_TILTED_MODEL;
        }

        bool ok = runCalibration(
            image_points,
            view.size(),
            board_size,
            square_size,
            aspect_ratio,
            flags,
            camera_matrix,
            dist_coeffs,
            &reproj_errs);


        SPDLOG_INFO("{}", ok ? "Calibration succeeded." : "Calibration failed.");
        SPDLOG_INFO("Intrinsic reprojection error is: {:f}", reproj_errs);

        progress.setValue(mCalibFiles.size());

        SPDLOG_INFO("camera matrix:\n{}", camera_matrix);

        SPDLOG_INFO("distortion coefficients:");
        SPDLOG_INFO(
            "r2: {} r4: {} r6: {}",
            dist_coeffs.at<double>(0, 0),
            dist_coeffs.at<double>(0, 1),
            dist_coeffs.at<double>(0, 4));
        SPDLOG_INFO("tx: {} ty: {}", dist_coeffs.at<double>(0, 2), dist_coeffs.at<double>(0, 3));
        SPDLOG_INFO(
            "k4: {} k5: {} k6: {}",
            dist_coeffs.at<double>(0, 5),
            dist_coeffs.at<double>(0, 6),
            dist_coeffs.at<double>(0, 7));

        SPDLOG_INFO(
            "s1: {} s2: {} s3: {} s4: {}",
            dist_coeffs.at<double>(0, 8),
            dist_coeffs.at<double>(0, 9),
            dist_coeffs.at<double>(0, 10),
            dist_coeffs.at<double>(0, 11));
        SPDLOG_INFO("taux: {} tauy: {}", dist_coeffs.at<double>(0, 12), dist_coeffs.at<double>(0, 13));

#ifdef SHOW_CALIB_MAINWINDOW
        // reset view to animation image
        if(!origImg.empty())
        {
            mMainWindow->updateImage(origImg); // now the last view will be deleted
        }
#endif

        // set calibration values
        IntrinsicCameraParams params;
        params.cameraMatrix = camera_matrix;
        params.cameraMatrix.at<double>(0, 2) += mMainWindow->getImageBorderSize();
        params.cameraMatrix.at<double>(1, 2) += mMainWindow->getImageBorderSize();
        dist_coeffs.convertTo(params.distortionCoeffs, CV_32F);
        params.reprojectionError = reproj_errs;

        return params;
    }
    return std::nullopt;
}


/**
 * @brief Runs intrinsic calibration with given parameters
 *
 *
 * @param image_points[in] detected and refined chessboard-corners
 * @param img_size
 * @param board_size[in] size of chessboard in fields (e.g. 6x8)
 * @param square_size[in] size of single square on chessboard pattern
 * @param aspect_ratio
 * @param flags[in] Flags for calibration; user input assembled in autoCalib()
 * @param camera_matrix[out]
 * @param dist_coeffs[out] distortion coefficients
 * @param reproj_errs[out]
 * @return
 */
int AutoCalib::runCalibration(
    std::vector<std::vector<cv::Point2f>> image_points,
    cv::Size                              img_size,
    cv::Size                              board_size,
    float                                 square_size,
    float                                 aspect_ratio,
    int                                   flags,
    cv::Mat                              &camera_matrix,
    cv::Mat                              &dist_coeffs,
    double                               *reproj_errs)
{
    int code;

    std::vector<std::vector<cv::Point3f>> object_points;

    // initialize arrays of points
    for(size_t j = 0; j < image_points.size(); j++)
    {
        std::vector<cv::Point3f> points_3d;
        for(int i = 0; i < board_size.width * board_size.height; i++)
        {
            points_3d.push_back(cv::Point3f(
                float((i / board_size.width) * square_size), float((i % board_size.width) * square_size), 0));
        }
        object_points.push_back(points_3d);
    }

    std::vector<cv::Mat> rot_vects, trans_vects;

    if(flags & CV_CALIB_FIX_ASPECT_RATIO)
    {
        camera_matrix.ptr<double>(0)[0] = aspect_ratio;
        camera_matrix.ptr<double>(1)[1] = 1.;
    }

    cv::Mat dist_coeffs_out;
    *reproj_errs = calibrateCamera(
        object_points, image_points, img_size, camera_matrix, dist_coeffs_out, rot_vects, trans_vects, flags);

    // guarantee the same size by only changing new values; input is a zero-vector
    // (some methods generate less coefficients than others/ normal vs. extended model)
    for(int i = 0; i < dist_coeffs_out.cols; ++i)
    {
        dist_coeffs.at<double>(i) = dist_coeffs_out.at<double>(i);
    }

    code = 1;
    return code;
}
