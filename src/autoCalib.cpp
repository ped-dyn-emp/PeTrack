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

#include <QFileInfo>
#include <QFileDialog>
#include <QProgressDialog>
#include <QApplication>

#include <opencv2/highgui.hpp>

#include "autoCalib.h"
#include "petrack.h"
#include "control.h"
#include "pMessageBox.h"

#define SHOW_CALIB_MAINWINDOW   ///< definieren, wenn das Schachbrett im Mainwindow und nicht separat angezeigt werden soll:
                                // fuehrt nach Calibration dazu dass play des originalvideos abstuerzt, insb wenn intr apply nicht ausgewaehlt war

AutoCalib::AutoCalib()
{
    mMainWindow = nullptr;
    mControlWidget = nullptr;

    // 6x8 und 4.6cm passen zu dem Schachbrettmuster auf der Eisenplatte mit Griff
    mBoardSizeX = 6;
    mBoardSizeY = 8;  //{6, 9};  //{6, 8}; // passt zu meinem Schachbrett, was ich ausgedruckt habe
    mSquareSize = 4.6f; //3.f; //5.25f;
}

AutoCalib::~AutoCalib()
{
}

void AutoCalib::setMainWindow(Petrack *mw)
{
    mMainWindow = mw;
    mControlWidget = mw->getControlWidget();
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
    if (i < mCalibFiles.size())
        return mCalibFiles.at(i);
    else
        return QString();
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
    if (mMainWindow)
    {
        static QString lastDir;
        
        if (!mCalibFiles.isEmpty()) 
            lastDir = QFileInfo(mCalibFiles.first()).path();
        
        QStringList calibFiles = QFileDialog::getOpenFileNames(mMainWindow, Petrack::tr("Open calibration sequence"), lastDir, "All supported types (*.bmp *.dib *.jpeg *.jpg *.jpe *.png *.pbm *.pgm *.ppm *.sr *.ras *.tiff *.tif *.exr *.jp2);;All files (*.*)");
        if (!calibFiles.isEmpty())
        { 
            mCalibFiles = calibFiles;
            return true;
        }
        //cout << mCalibFiles.first().toStdString() << endl; //toAscii() .data() Local8Bit().constData() << endl;
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
 * Also sets the GUI elements to the calculated value.
 */
void AutoCalib::autoCalib()
{
    if (mMainWindow)
    {
        // no files are selected for calibration
        if (mCalibFiles.isEmpty()) 
        {
            PInformation(mMainWindow, Petrack::tr("Petrack"), Petrack::tr("At first you have to select files."));
            return;
        }

        cv::Size board_size(mBoardSizeX, mBoardSizeY); //{6, 9};  //{6, 8}; // passt zu meinem Schachbrett, was ich ausgedruckt habe
        float square_size = mSquareSize; //5.25f; // 3.f;   // da 3x3cm hat Schachbrett, was ich ausgedruckt habe
        float aspect_ratio = 1.f;
        int flags = 0;
        std::vector<cv::Point2f> corners;
        std::vector<std::vector<cv::Point2f> > image_points;
        cv::Mat camera_matrix = cv::Mat::eye(3, 3, CV_64F);
        cv::Mat dist_coeffs = cv::Mat::zeros(1, 8, CV_64F);
        cv::Mat extr_params;
        double reproj_errs;
        cv::Mat view, view_gray;
        bool found = false;
        cv::Mat origImg;
        cv::Size imgSize;

#ifdef SHOW_CALIB_MAINWINDOW
        if (!mMainWindow->getImg().empty())
            origImg = mMainWindow->getImg().clone(); // must be cloned, because mIplImg will be deleted in updateImage
#endif

        QProgressDialog progress("Calculating intrinsic camera parameters...", "Abort calculation", 0, mCalibFiles.size(), mMainWindow);
        progress.setWindowModality(Qt::WindowModal); // blocks main window

        debout << "Search for cheesboard pattern (" << board_size.width << "x" << board_size.height << ") with square size: " << square_size << "cm..." << std::endl;
        bool min_one_pattern_found = false;
        // search for chessbord corners in every image
        for (int i = 0; i < mCalibFiles.size(); ++i)
        {
            progress.setValue(i);
            qApp->processEvents();
            if (progress.wasCanceled())
                break;

            // cannot load image
            view = cv::imread(mCalibFiles.at(i).toStdString(),cv::IMREAD_COLOR);
            if (view.empty())
            {
                progress.setValue(mCalibFiles.size());
                PCritical(mMainWindow, Petrack::tr("Petrack"), Petrack::tr("Cannot load %1.\nTerminate Calibration.").arg(mCalibFiles.at(i)));
#ifdef SHOW_CALIB_MAINWINDOW
                // reset view to animation image 
                if (!origImg.empty())
                {
                    mMainWindow->updateImage(origImg); // now the last view will be deleted
                }
#endif
                return;
            }

            // muss nur bei einem bild gemacht werden
            if (i==0)
                imgSize = cv::Size(view.rows,view.cols);

            // search for chessboard corners
            found = findChessboardCorners(view,board_size,corners,cv::CALIB_CB_ADAPTIVE_THRESH);

            if (found)
            {
                // improve the found corners' coordinate accuracy
                cv::cvtColor(view,view_gray,cv::COLOR_BGR2GRAY);
                cv::cornerSubPix(view_gray,corners,cv::Size(11,11),
                             cv::Size(-1,-1),cv::TermCriteria(CV_TERMCRIT_EPS+CV_TERMCRIT_ITER,30,0.1));
            
                image_points.push_back(corners);
                drawChessboardCorners(view,board_size,corners,found);

#ifndef SHOW_CALIB_MAINWINDOW
                namedWindow("img", CV_WINDOW_AUTOSIZE ); // 0 wenn skalierbar sein soll
                imShow("img", view);
                //cvWaitKey( 0 ); // zahl statt null, wenn nach bestimmter zeit weitergegangen werden soll
#endif
#ifdef SHOW_CALIB_MAINWINDOW
                // show image in view to show calculation
                mMainWindow->updateImage(view);
#endif
                qApp->processEvents(); // to allow events and update sceen for viewing new image
                min_one_pattern_found = true;
            }
            else
                debout << "Calibration pattern not found in: "<< mCalibFiles.at(i).toStdString() << std::endl;
        }

        if( !min_one_pattern_found )
        {
            debout << "Calibration failed. No patterns found!" << std::endl;
            PWarning(mMainWindow,
                                 QString("Calibration failed"),
                                 QString("Chessboard pattern (%1x%2) not found in calibration files.").arg(board_size.width).arg(board_size.height));
            return;
        }

        // set flags for calibration
        if (mControlWidget->quadAspectRatio->isChecked())
            flags |= CV_CALIB_FIX_ASPECT_RATIO; //durch setzen von aspect_ratio kann fix ascpect anders als 1:1 eingestellt werden
        if (mControlWidget->fixCenter->isChecked())
            flags |= CV_CALIB_FIX_PRINCIPAL_POINT;
        if (!mControlWidget->tangDist->isChecked())
            flags |= CV_CALIB_ZERO_TANGENT_DIST;
        // run calibration

        bool ok = runCalibration( image_points, view.size(), board_size,
            square_size, aspect_ratio, flags, camera_matrix, dist_coeffs, &reproj_errs);//, extr_params);//,
//            &reproj_errs);//, &avg_reproj_err );
        debout << (ok ? "Calibration succeeded." : "Calibration failed.") << std::endl; //  "Avgage reprojection error is "  << avg_reproj_err << endl;
        debout << "Intrinsic reprojection error is: " << reproj_errs << std::endl;

        progress.setValue(mCalibFiles.size());

        debout << "Cameramatrix: " << std::endl;
        debout << "( " << camera_matrix.at<double>(0,0) << " " << camera_matrix.at<double>(0,1) << " " << camera_matrix.at<double>(0,2) << ")" << std::endl;
        debout << "( " << camera_matrix.at<double>(1,0) << " " << camera_matrix.at<double>(1,1) << " " << camera_matrix.at<double>(1,2) << ")" << std::endl;
        debout << "( " << camera_matrix.at<double>(2,0) << " " << camera_matrix.at<double>(2,1) << " " << camera_matrix.at<double>(2,2) << ")" << std::endl;
        //        debout << camera_matrix << endl;

        debout << "Distortioncoefficients: " << std::endl;
        debout << "r2: " << dist_coeffs.at<double>(0,0) << " r4: " << dist_coeffs.at<double>(0,1) << " r6: " << dist_coeffs.at<double>(0,4) << std::endl;
        debout << "tx: " << dist_coeffs.at<double>(0,2) << " ty: " << dist_coeffs.at<double>(0,3) << std::endl;
        debout << "k4: " << dist_coeffs.at<double>(0,5) << " k5: " << dist_coeffs.at<double>(0,6) << " k6: " << dist_coeffs.at<double>(0,7) << std::endl;
        //        debout << dist_coeffs << endl;

        // set calibration values
        mControlWidget->setCalibFxValue(camera_matrix.at<double>(0,0));
        mControlWidget->setCalibFyValue(camera_matrix.at<double>(1,1));
        mControlWidget->setCalibCxValue(camera_matrix.at<double>(0,2) + mMainWindow->getImageBorderSize());
        mControlWidget->setCalibCyValue(camera_matrix.at<double>(1,2) + mMainWindow->getImageBorderSize());
        mControlWidget->setCalibR2Value(dist_coeffs.at<double>(0,0));
        mControlWidget->setCalibR4Value(dist_coeffs.at<double>(0,1));
        mControlWidget->setCalibTxValue(dist_coeffs.at<double>(0,2));
        mControlWidget->setCalibTyValue(dist_coeffs.at<double>(0,3));
        mControlWidget->setCalibR6Value(dist_coeffs.at<double>(0,4));
        mControlWidget->setCalibK4Value(dist_coeffs.at<double>(0,5));
        mControlWidget->setCalibK5Value(dist_coeffs.at<double>(0,6));
        mControlWidget->setCalibK6Value(dist_coeffs.at<double>(0,7));


#ifdef SHOW_CALIB_MAINWINDOW
        // reset view to animation image 
        if (!origImg.empty())
        {
            mMainWindow->updateImage(origImg); // now the last view will be deleted
        }
#endif
    }
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
int AutoCalib::runCalibration(std::vector<std::vector<cv::Point2f> > image_points, cv::Size img_size, cv::Size board_size,
    float square_size, float aspect_ratio, int flags,
    cv::Mat &camera_matrix, cv::Mat &dist_coeffs, double *reproj_errs)
{
    int code;

    std::vector<std::vector<cv::Point3f> > object_points;

    // initialize arrays of points
    for(size_t j=0; j<image_points.size();j++)
    {
        std::vector<cv::Point3f> points_3d;
        for(int i=0; i<board_size.width*board_size.height;i++)
        {
           points_3d.push_back(cv::Point3f(float((i/board_size.width)*square_size),float((i%board_size.width)*square_size),0));

        }
        object_points.push_back(points_3d);
    }

    std::vector<cv::Mat> rot_vects, trans_vects;

    if(flags & CV_CALIB_FIX_ASPECT_RATIO)
    {
        camera_matrix.ptr<double>(0)[0] = aspect_ratio;
        camera_matrix.ptr<double>(1)[1] = 1.;
    }

    *reproj_errs = calibrateCamera(object_points,image_points,img_size,camera_matrix,dist_coeffs,rot_vects,trans_vects,flags);

    code = 1;
    return code;
}

