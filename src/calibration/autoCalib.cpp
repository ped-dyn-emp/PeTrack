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
        if(!mLastDir.isEmpty())
        {
            QStringList calibFiles = QFileDialog::getOpenFileNames(
                mMainWindow,
                Petrack::tr("Open calibration sequence"),
                mLastDir,
                "All supported types (*.bmp *.dib *.jpeg *.jpg *.jpe *.png *.pbm *.pgm *.ppm *.sr *.ras *.tiff *.tif "
                "*.exr "
                "*.jp2);;All files (*.*)");

            if(!calibFiles.isEmpty())
            {
                mCalibFiles = calibFiles;
                return true;
            }
        }
    }
    return false;
}

QString AutoCalib::getCalibVideo()
{
    return mCalibVideo;
}

void AutoCalib::setCalibVideo(const QString &v)
{
    mCalibVideo = v;
}

/**
 * @brief Opens a file dialog to select a calibration video and processes it
 *
 * Prompts the user to select a video file, then analyzes it to extract suitable calibration frames.
 *
 * @return true if a video was selected and good samples were found, false otherwise
 */
bool AutoCalib::openCalibVideo()
{
    if(mMainWindow && !mLastDir.isEmpty())
    {
        QString calibVideo = QFileDialog::getOpenFileName(
            mMainWindow,
            Petrack::tr("Open calibration video"),
            QFileInfo(mLastDir).dir().path(),
            "All supported types (*.avi *.mpg *.mts *.m2ts "
            "*.m2t *.wmv *.mov *.mp4 *.mxf);;All files (*.*)");
        if(calibVideo.isEmpty())
        {
            return false;
        }
        mCalibVideo = calibVideo;
        findGoodCalibrationSamplesFromVideo(cv::Size(mBoardSizeX, mBoardSizeY));
        return !mCalibFiles.isEmpty();
    }
    return false;
}

/**
 * @brief Extracts the four outer corners of a chessboard from a set of corners
 *
 * @param corners[in] All detected chessboard corners
 * @param boardSize[in] Size of the chessboard (width x height in squares)
 * @return Vector containing the four outer corners (top-left, top-right, bottom-right, bottom-left)
 */
std::vector<cv::Point2f>
AutoCalib::getOuterChessboardCorners(const std::vector<cv::Point2f> &corners, cv::Size &boardSize)
{
    std::vector<cv::Point2f> outerCorners;
    auto                     upLeft    = corners[0];
    auto                     upRight   = corners[boardSize.width - 1];
    auto                     downRight = corners[corners.size() - 1];
    auto                     downLeft  = corners[corners.size() - boardSize.width];
    outerCorners.push_back(upLeft);
    outerCorners.push_back(upRight);
    outerCorners.push_back(downRight);
    outerCorners.push_back(downLeft);
    return outerCorners;
}

/**
 * @brief Calculates the inner area enclosed by the outer chessboard corners
 *
 * Uses the shoelace formula to compute the area based on the four outer corners.
 *
 * @param outerCorners[in] Four outer corners of the chessboard
 * @return Area enclosed by the outer corners
 */
double AutoCalib::calcInnerAreaOfChessboard(std::vector<cv::Point2f> &outerCorners)
{
    // https://mathworld.wolfram.com/Quadrilateral.html
    cv::Point2f a = outerCorners[1] - outerCorners[0]; // upRight - upLeft
    cv::Point2f b = outerCorners[2] - outerCorners[1]; // downRight - upRight
    cv::Point2f c = outerCorners[3] - outerCorners[2]; // downLeft - downRight
    cv::Point2f p = b + c;
    cv::Point2f q = a + b;
    return cv::abs(p.cross(q)) / 2;
}

/**
 * @brief Calculates the skew of the chessboard based on outer corners
 *
 * Measures the deviation from a 90-degree angle at one corner to assess skewness.
 *
 * @param outerCorners[in] Four outer corners of the chessboard
 * @return Absolute difference between the calculated angle and 90 degrees (in radians)
 */
double AutoCalib::calcSkewOfChessboard(std::vector<cv::Point2f> &outerCorners)
{
    auto upLeft    = outerCorners[0];
    auto upRight   = outerCorners[1];
    auto downRight = outerCorners[2];

    // calculate the angle between two lines and compare to 90°
    cv::Point2f ab    = upLeft - upRight;
    cv::Point2f cb    = downRight - upRight;
    double      angle = std::acos(ab.dot(cb) / (cv::norm(ab) * cv::norm(cb)));
    return cv::abs((PI / 2.) - angle);
}

/**
 * @brief Compares coverage of grid blocks between two coverage vectors
 *
 * Determines how many new grid blocks are covered in the `covered` vector compared to `totalCovered`,
 * useful for assessing the uniqueness of a sample's spatial distribution.
 *
 * @param totalCovered[in] Vector representing previously covered grid blocks
 * @param covered[in] Vector representing grid blocks covered by the current sample
 * @return Number of newly covered blocks in `covered` that were not in `totalCovered`
 */
int AutoCalib::compareCoverage(std::vector<bool> &totalCovered, std::vector<bool> &covered)
{
    // calculate how many different blocks are covered compared to the previous samples
    int amountDiff = 0;
    for(size_t i = 0; i < totalCovered.size(); i++)
    {
        if(covered[i] && !totalCovered[i])
        {
            amountDiff++;
        }
    }
    return amountDiff;
}

/**
 * @brief Merges two coverage vectors into one
 *
 * Combines the coverage information from `covered` into `totalCovered` by performing a logical OR
 * operation on corresponding elements, updating `totalCovered` in place.
 *
 * @param totalCovered[in,out] Vector of previously covered grid blocks, updated with new coverage
 * @param covered[in] Vector of grid blocks covered by the current sample
 * @return Updated `totalCovered` vector reflecting the merged coverage
 */
std::vector<bool> AutoCalib::mergeCoverages(std::vector<bool> &totalCovered, std::vector<bool> &covered)
{
    for(size_t i = 0; i < totalCovered.size(); i++)
    {
        totalCovered[i] = totalCovered[i] || covered[i];
    }
    return totalCovered;
}

/**
 * @brief Counts the number of covered blocks in a coverage vector
 *
 * Calculates the total number of grid blocks marked as covered (true) in the provided vector.
 *
 * @param covered[in] Vector representing covered grid blocks
 * @return Number of blocks marked as covered
 */
int AutoCalib::getCovered(std::vector<bool> covered)
{
    int totalCovered = 0;
    for(size_t i = 0; i < covered.size(); i++)
    {
        if(covered[i])
        {
            totalCovered++;
        }
    }
    return totalCovered;
}

/**
 * @brief Calculates coverage of chessboard corners across an image grid
 *
 * Divides the image into a grid and determines which grid cells contain chessboard corners,
 * returning a boolean vector indicating coverage per cell.
 *
 * @param corners[in] All detected chessboard corners
 * @param imageSize[in] Size of the image (width x height in pixels)
 * @param gridSize[in] Number of grid divisions along each axis (e.g., 10 for a 10x10 grid)
 * @return Vector of booleans indicating which grid cells are covered by corners
 */
std::vector<bool> AutoCalib::calcXYCoverage(std::vector<cv::Point2f> &corners, cv::Size imageSize, int gridSize)
{
    int               rectWidth  = imageSize.width / gridSize;
    int               rectHeight = imageSize.height / gridSize;
    std::vector<bool> covered(gridSize * gridSize);


    for(size_t i = 0; i < corners.size(); i++)
    {
        cv::Point2f p = corners[i];
        // calculate according block
        int colIndex = p.x / rectWidth;
        int rowIndex = p.y / rectHeight;
        int idx      = rowIndex * gridSize + colIndex;
        covered[idx] = true;
    }
    return covered;
}

/**
 * @brief Evaluates if a sample is sufficiently different from existing good samples
 *
 * Assesses whether a new sample is unique enough to be added to the set of good samples by comparing
 * its area, skew, and grid coverage against existing samples. A sample is good if it differs significantly
 * in parameters or covers new grid areas.
 *
 * @param goodSamples[in] Vector of previously accepted calibration samples
 * @param sample[in] New sample to evaluate
 * @param totalCoverage[in,out] Vector tracking total grid coverage across all good samples
 * @return true if the sample is sufficiently different and should be added, false otherwise
 */
bool AutoCalib::isGoodSample(
    std::vector<calib::Sample> &goodSamples,
    calib::Sample              &sample,
    std::vector<bool>          &totalCoverage)
{
    if(goodSamples.size() == 0)
    {
        return true;
    }
    // a sample is considered good if the parameters differ greatly enough from the existing samples
    int    totalCovered     = getCovered(totalCoverage);
    int    notCovered       = totalCoverage.size() - totalCovered;
    int    differentCovered = compareCoverage(totalCoverage, sample.coverage);
    double diffCoverage     = static_cast<double>(differentCovered) / notCovered;

    double minDifference = std::numeric_limits<double>::max();
    for(calib::Sample goodSample : goodSamples)
    {
        double difference = goodSample.getDifference(sample);
        if(difference < minDifference)
        {
            minDifference = difference;
        }
    }
    return minDifference > 0.1 || diffCoverage > 0.1;
}

/**
 * @brief Analyzes a video to find and save frames with good chessboard samples
 *
 * Processes the video frame by frame, detects chessboard corners, evaluates their quality,
 * and saves good samples as images.
 *
 * @param boardSize[in] Size of the chessboard (width x height in squares)
 */
void AutoCalib::findGoodCalibrationSamplesFromVideo(cv::Size boardSize)
{
    cv::Mat                    view, viewGray;
    bool                       found = false;
    cv::Mat                    origImg;
    cv::Size                   imgSize;
    std::vector<cv::Point2f>   corners;
    std::vector<calib::Sample> goodSamples;
    int                        stepSize = 10;
    cv::VideoCapture           video(mCalibVideo.toStdString());
    QStringList                calibFiles;
    QString                    outputDir;
    if(!mLastDir.isEmpty())
    {
        outputDir = QFileDialog::getExistingDirectory(
            mMainWindow,
            Petrack::tr("Choose an output directory"),
            QFileInfo(mLastDir).dir().path(),
            QFileDialog::ShowDirsOnly);
    }
    else
    {
        outputDir = QFileDialog::getExistingDirectory(
            mMainWindow,
            Petrack::tr("Choose an output directory"),
            QFileInfo(mCalibVideo).dir().path(),
            QFileDialog::ShowDirsOnly);
    }

    qApp->processEvents();

    if(outputDir.isEmpty())
    {
        return;
    }
    if(!mMainWindow->getImg().empty())
    {
        origImg = mMainWindow->getImg().clone();
    }

    QProgressDialog progress(
        "Searching for good samples...",
        "Abort search",
        0,
        video.get(cv::CAP_PROP_FRAME_COUNT) / stepSize,
        mMainWindow);
    progress.setWindowModality(Qt::WindowModal); // blocks main window
    int               gridSize = 10;
    std::vector<bool> totalCovered(gridSize * gridSize);
    // search for chessbord corners in every image
    for(int i = 0; i < video.get(cv::CAP_PROP_FRAME_COUNT); i += stepSize)
    {
        progress.setValue(i / stepSize);
        qApp->processEvents();
        if(progress.wasCanceled())
        {
            break;
        }

        video.set(cv::CAP_PROP_POS_FRAMES, i);
        video >> view;
        // cannot load image
        if(view.empty())
        {
            progress.setValue(video.get(cv::CAP_PROP_FRAME_COUNT) / stepSize);
            // reset view to animation image
            if(!origImg.empty())
            {
                mMainWindow->updateImage(origImg); // now the last view will be deleted
            }
            break;
        }
        imgSize = cv::Size(video.get(cv::CAP_PROP_FRAME_WIDTH), video.get(cv::CAP_PROP_FRAME_HEIGHT));

        found = cv::findChessboardCorners(view, boardSize, corners, cv::CALIB_CB_ADAPTIVE_THRESH);
        if(found)
        {
            calib::Sample sample;
            sample.corners = corners;
            // evaluate how good the frame is (balance between x and y distribution, varying coverage, skew)
            auto outerCorners = getOuterChessboardCorners(corners, boardSize);
            sample.area       = calcInnerAreaOfChessboard(outerCorners) / imgSize.area();
            sample.skew       = calcSkewOfChessboard(outerCorners);
            sample.coverage   = calcXYCoverage(corners, imgSize, gridSize);

            if(isGoodSample(goodSamples, sample, totalCovered))
            {
                cv::cvtColor(view, viewGray, cv::COLOR_BGR2GRAY);
                cv::cornerSubPix(
                    viewGray,
                    sample.corners,
                    cv::Size(11, 11),
                    cv::Size(-1, -1),
                    cv::TermCriteria(CV_TERMCRIT_EPS + CV_TERMCRIT_ITER, 30, 0.1));
                goodSamples.push_back(sample);
                totalCovered = mergeCoverages(totalCovered, sample.coverage);

                // handle Output
                QString outputPath = QString("%1/%2.png").arg(outputDir).arg(goodSamples.size());
                cv::imwrite(outputPath.toStdString(), view);
                calibFiles.push_back(outputPath);

                cv::drawChessboardCorners(view, boardSize, sample.corners, found);
                progress.setLabelText(
                    QString("Searching for good samples... \n Samples found: %1").arg(goodSamples.size()));
                mMainWindow->updateImage(view);
                qApp->processEvents();
            }
        }
    }
    if(calibFiles.empty())
    {
        PCritical(
            mMainWindow,
            Petrack::tr("Petrack"),
            Petrack::tr("No good samples found in the video! Try again with a better video."));
    }
    else
    {
        mCalibFiles = calibFiles;
    }
    // reset view to animation image
    if(!origImg.empty())
    {
        mMainWindow->updateImage(origImg); // now the last view will be deleted
    }
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
 *
 * @return new parameters or std::nullopt, if no calibration could be done
 */
std::optional<IntrinsicModelsParameters> AutoCalib::autoCalib(bool quadAspectRatio, bool fixCenter, bool tangDist)
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
        cv::Mat                               camera_matrix         = cv::Mat::eye(3, 3, CV_64F);
        cv::Mat                               distortion_coeffs     = cv::Mat::zeros(1, 14, CV_64F);
        cv::Mat                               camera_matrix_ext     = cv::Mat::eye(3, 3, CV_64F);
        cv::Mat                               distortion_coeffs_ext = cv::Mat::zeros(1, 14, CV_64F);
        cv::Mat                               extr_params;
        double                                reproj_errs;
        double                                reproj_errs_ext;
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

        bool ok = runCalibration(
            image_points,
            view.size(),
            board_size,
            square_size,
            aspect_ratio,
            flags,
            camera_matrix,
            distortion_coeffs,
            &reproj_errs);

        SPDLOG_INFO("OLD MODEL CALIBRATION");
        SPDLOG_INFO("{}", ok ? "Calibration succeeded." : "Calibration failed.");
        SPDLOG_INFO("Intrinsic reprojection error is: {:f}", reproj_errs);

        progress.setValue(mCalibFiles.size());

        SPDLOG_INFO("camera matrix:\n{}", camera_matrix);

        SPDLOG_INFO("distortion coefficients:");
        SPDLOG_INFO(
            "r2: {} r4: {} r6: {}",
            distortion_coeffs.at<double>(0, 0),
            distortion_coeffs.at<double>(0, 1),
            distortion_coeffs.at<double>(0, 4));
        SPDLOG_INFO("tx: {} ty: {}", distortion_coeffs.at<double>(0, 2), distortion_coeffs.at<double>(0, 3));
        SPDLOG_INFO(
            "k4: {} k5: {} k6: {}",
            distortion_coeffs.at<double>(0, 5),
            distortion_coeffs.at<double>(0, 6),
            distortion_coeffs.at<double>(0, 7));

        SPDLOG_INFO(
            "s1: {} s2: {} s3: {} s4: {}",
            distortion_coeffs.at<double>(0, 8),
            distortion_coeffs.at<double>(0, 9),
            distortion_coeffs.at<double>(0, 10),
            distortion_coeffs.at<double>(0, 11));
        SPDLOG_INFO("taux: {} tauy: {}", distortion_coeffs.at<double>(0, 12), distortion_coeffs.at<double>(0, 13));


        flags |= CV_CALIB_RATIONAL_MODEL + CV_CALIB_THIN_PRISM_MODEL + CV_CALIB_TILTED_MODEL;

        bool ok_ext = runCalibration(
            image_points,
            view.size(),
            board_size,
            square_size,
            aspect_ratio,
            flags,
            camera_matrix_ext,
            distortion_coeffs_ext,
            &reproj_errs_ext);

        SPDLOG_INFO("NEW MODEL CALIBRATION");
        SPDLOG_INFO("{}", ok_ext ? "Calibration succeeded." : "Calibration failed.");
        SPDLOG_INFO("Intrinsic reprojection error is: {:f}", reproj_errs_ext);

        progress.setValue(mCalibFiles.size());

        SPDLOG_INFO("camera matrix:\n{}", camera_matrix_ext);

        SPDLOG_INFO("distortion coefficients:");
        SPDLOG_INFO(
            "r2: {} r4: {} r6: {}",
            distortion_coeffs_ext.at<double>(0, 0),
            distortion_coeffs_ext.at<double>(0, 1),
            distortion_coeffs_ext.at<double>(0, 4));
        SPDLOG_INFO("tx: {} ty: {}", distortion_coeffs_ext.at<double>(0, 2), distortion_coeffs_ext.at<double>(0, 3));
        SPDLOG_INFO(
            "k4: {} k5: {} k6: {}",
            distortion_coeffs_ext.at<double>(0, 5),
            distortion_coeffs_ext.at<double>(0, 6),
            distortion_coeffs_ext.at<double>(0, 7));

        SPDLOG_INFO(
            "s1: {} s2: {} s3: {} s4: {}",
            distortion_coeffs_ext.at<double>(0, 8),
            distortion_coeffs_ext.at<double>(0, 9),
            distortion_coeffs_ext.at<double>(0, 10),
            distortion_coeffs_ext.at<double>(0, 11));
        SPDLOG_INFO(
            "taux: {} tauy: {}", distortion_coeffs_ext.at<double>(0, 12), distortion_coeffs_ext.at<double>(0, 13));


#ifdef SHOW_CALIB_MAINWINDOW
        // reset view to animation image
        if(!origImg.empty())
        {
            mMainWindow->updateImage(origImg); // now the last view will be deleted
        }
#endif

        // set calibration values
        IntrinsicModelsParameters params;

        params.oldModelParams.cameraMatrix = camera_matrix;
        params.oldModelParams.cameraMatrix.at<double>(0, 2) += mMainWindow->getImageBorderSize();
        params.oldModelParams.cameraMatrix.at<double>(1, 2) += mMainWindow->getImageBorderSize();
        distortion_coeffs.convertTo(params.oldModelParams.distortionCoeffs, CV_32F);
        params.oldModelParams.reprojectionError = reproj_errs;
        checkParamPlausibility(params.oldModelParams);

        params.extModelParams.cameraMatrix = camera_matrix_ext;
        params.extModelParams.cameraMatrix.at<double>(0, 2) += mMainWindow->getImageBorderSize();
        params.extModelParams.cameraMatrix.at<double>(1, 2) += mMainWindow->getImageBorderSize();
        distortion_coeffs_ext.convertTo(params.extModelParams.distortionCoeffs, CV_32F);
        params.extModelParams.reprojectionError = reproj_errs_ext;
        checkParamPlausibility(params.extModelParams);

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
 * @param distortion_coeffs[out] distortion coefficients
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
    cv::Mat                              &distortion_coeffs,
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
        distortion_coeffs.at<double>(i) = dist_coeffs_out.at<double>(i);
    }

    code = 1;
    return code;
}

void AutoCalib::checkParamPlausibility(IntrinsicCameraParams &modelParams)
{
    // check camera params
    auto camParams = modelParams.cameraMatrix;
    if(camParams.at<double>(0, 0) < 0 || camParams.at<double>(0, 0) > 5000)
    {
        PWarning(
            nullptr,
            "Warning!",
            QString("Value %1 for fx might indicate poor calibration results.").arg(camParams.at<double>(0, 0)));
    }
    if(camParams.at<double>(1, 1) < 0 || camParams.at<double>(1, 1) > 5000)
    {
        PWarning(
            nullptr,
            "Warning!",
            QString("Value %1 for fy might indicate poor calibration results.").arg(camParams.at<double>(1, 1)));
    }
    if(camParams.at<double>(0, 2) < 0 || camParams.at<double>(0, 2) > 5000)
    {
        PWarning(
            nullptr,
            "Warning!",
            QString("Value %1 for cx might indicate poor calibration results.").arg(camParams.at<double>(0, 2)));
    }
    if(camParams.at<double>(1, 2) < 0 || camParams.at<double>(1, 2) > 5000)
    {
        PWarning(
            nullptr,
            "Warning!",
            QString("Value %1 for cy might indicate poor calibration results.").arg(camParams.at<double>(1, 2)));
    }
    // check distortion params
    std::vector<std::string> distParamsNames{
        "r2", "r4", "tx", "ty", "r6", "k4", "k5", "k6", "s1", "s2", "s3", "s4", "taux", "tauy"};
    auto distParams = modelParams.distortionCoeffs;
    for(int i = 0; i < distParams.cols; ++i)
    {
        if(distParams.at<float>(i) < -5 || distParams.at<float>(i) > 5)
        {
            PWarning(
                nullptr,
                "Warning!",
                QString("Value %1 for %2 might indicate poor calibration results.")
                    .arg(distParams.at<float>(i))
                    .arg(QString::fromStdString(distParamsNames[i])));
        }
    }
}
