#include <QFileInfo>
#include <QFileDialog>
#include <QMessageBox>
#include <QProgressDialog>
#include <QApplication>

#include "opencv2/highgui.hpp"

#include "autoCalib.h"
#include "petrack.h"
#include "control.h"

using namespace::cv;
using namespace std;

#define SHOW_CALIB_MAINWINDOW   // definieren, wenn das Schachbrett im Mainwindow und nicht separat angezeigt werden soll:
                                // fuehrt nach Calibration dazu dass play des originalvideos abstuerzt, insb wenn intr apply nicht ausgewaehlt war

AutoCalib::AutoCalib()
{
    mMainWindow = NULL;
    mControlWidget = NULL;

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

void AutoCalib::autoCalib()
{
    if (mMainWindow)
    {
        // no files are selected for calibration
        if (mCalibFiles.isEmpty()) 
        {
            QMessageBox::information(mMainWindow, Petrack::tr("Petrack"), Petrack::tr("At first you have to select files."));
            return;
        }

        Size board_size(mBoardSizeX, mBoardSizeY); //{6, 9};  //{6, 8}; // passt zu meinem Schachbrett, was ich ausgedruckt habe
        float square_size = mSquareSize; //5.25f; // 3.f;   // da 3x3cm hat Schachbrett, was ich ausgedruckt habe
        float aspect_ratio = 1.f;
        int flags = 0;
        vector<Point2f> corners;
        vector<vector<Point2f> > image_points;
//        Mat corners;
//        CvSeq* image_points_seq = NULL;
//        CvPoint2D32f* image_points_buf = NULL;
//        double _camera_matrix[9], _dist_coeffs[8];
        Mat camera_matrix = Mat::eye(3, 3, CV_64F);//, _camera_matrix);
        Mat dist_coeffs = Mat::zeros(1, 8, CV_64F);//, _dist_coeffs);
        Mat extr_params;
        double reproj_errs;
        double avg_reproj_err = 0;
        Mat view, view_gray;
        int count = 0;
        bool found = false;
//        CvMemStorage* storage = NULL;
//        int elem_size;
        Mat origImg;
        Size imgSize;

#ifdef SHOW_CALIB_MAINWINDOW
        if (!mMainWindow->getImg().empty())
            origImg = mMainWindow->getImg().clone(); // must be cloned, because mIplImg will be deleted in updateImage
#endif

//        elem_size = board_size.width*board_size.height*sizeof(image_points_buf[0]);
//        storage = cvCreateMemStorage(MAX( elem_size*4, 1 << 16));
//        image_points_buf = (CvPoint2D32f*)cvAlloc(elem_size);
//        image_points_seq = cvCreateSeq(0, sizeof(CvSeq), elem_size, storage);

        QProgressDialog progress("Calculating intrinsic camera parameters...", "Abort calculation", 0, mCalibFiles.size(), mMainWindow);
        progress.setWindowModality(Qt::WindowModal); // blocks main window

        debout << "Search for cheesboard pattern (" << board_size.width << "x" << board_size.height << ") with square size: " << square_size << "cm..." << endl;
        bool min_one_pattern_found = false;
        // search for chessbord corners in every image
        for (int i = 0; i < mCalibFiles.size(); ++i)
        {
            progress.setValue(i);
            qApp->processEvents();
            if (progress.wasCanceled())
                break;
            //cout << mCalibFiles.at(i).toStdString() << endl; //toAscii() .data() Local8Bit().constData() << endl;

            // cannot load image
            //if (!(view = cvLoadImage(mCalibFiles.at(i).toAscii(), 1)))
            //            if (!(view = cvLoadImage(mCalibFiles.at(i).toLatin1(), CV_LOAD_IMAGE_COLOR)))
            view = imread(mCalibFiles.at(i).toStdString(),IMREAD_COLOR);
            if (view.empty())
            {
                progress.setValue(mCalibFiles.size());
                QMessageBox::critical(mMainWindow, Petrack::tr("Petrack"), Petrack::tr("Cannot load %1.\nTerminate Calibration.").arg(mCalibFiles.at(i)));
#ifdef SHOW_CALIB_MAINWINDOW
                // reset view to animation image 
                if (!origImg.empty())
                {
//                    Mat ii = mMainWindow->getImg();
//                    cvReleaseImage(&ii); // neue zeile, da in updateimage nicht mehr geloescht wird
                    mMainWindow->updateImage(origImg); // now the last view will be deleted
                }
#endif
                return;
            }
//            debout << "Size: " << view.rows << "x" << view.cols << endl;
            // muss nur bei einem bild gemacht werden
            if (i==0)
                imgSize = Size(view.rows,view.cols);
//            debout << "Size: " << imgSize.width << "x" << imgSize.height << endl;
            // search for chessboard corners
            found = findChessboardCorners(view,board_size,corners,CALIB_CB_ADAPTIVE_THRESH);
//            found = cvFindChessboardCorners(view, board_size,
//                image_points_buf, &count, CV_CALIB_CB_ADAPTIVE_THRESH);

            // if not found try other board_size
//            if( !found )
//            {
//                debout << "Try other chessboard size..." << endl;
//                if( board_size.width == 9 )
//                {
//                    board_size.width = 8;
//                }else
//                {
//                    board_size.width = 9;
//                }
//                found = cvFindChessboardCorners(view, board_size,
//                             image_points_buf, &count, CV_CALIB_CB_ADAPTIVE_THRESH);
//            }
//            debout << "found: " << found << endl;
            if (found)
            {
                // improve the found corners' coordinate accuracy
//                view_gray = cvCreateImage(imgSize, 8, 1);
                cvtColor(view,view_gray,COLOR_BGR2GRAY);
//                cvCvtColor(view, view_gray, CV_BGR2GRAY);
                cornerSubPix(view_gray,corners,Size(11,11),
                             Size(-1,-1),TermCriteria(CV_TERMCRIT_EPS+CV_TERMCRIT_ITER,30,0.1));
//                cvFindCornerSubPix(view_gray, image_points_buf, count, cvSize(11,11),
//                    cvSize(-1,-1), cvTermCriteria(CV_TERMCRIT_EPS+CV_TERMCRIT_ITER, 30, 0.1));
//                cvReleaseImage(&view_gray);
            
//                cvSeqPush(image_points_seq, image_points_buf);

                image_points.push_back(corners);
                drawChessboardCorners(view,board_size,corners,found);
//                cvDrawChessboardCorners(view, board_size, image_points_buf, count, found);

#ifndef SHOW_CALIB_MAINWINDOW
                namedWindow("img", CV_WINDOW_AUTOSIZE ); // 0 wenn skalierbar sein soll
                imShow("img", view);
                //cvWaitKey( 0 ); // zahl statt null, wenn nach bestimmter zeit weitergegangen werden soll
#endif
#ifdef SHOW_CALIB_MAINWINDOW
                // show image in view to show calculation
//                IplImage* ii = mMainWindow->getIplImage();
//                cvReleaseImage(&ii); // neue zeile, da in updateimage nicht mehr geloescht wird
                mMainWindow->updateImage(view);
#endif
                qApp->processEvents(); // to allow events and update sceen for viewing new image
                min_one_pattern_found = true;
            }
            else
                debout << "Calibration pattern not found in: "<< mCalibFiles.at(i).toStdString() << endl;

            //cvReleaseImage(&view); // not allowed, because view will be deleted in updateImage
        }

        if( !min_one_pattern_found )
        {
            debout << "Calibration failed. No patterns found!" << endl;
            QMessageBox::warning(mMainWindow,
                                 QString("Calibration failed"),
                                 QString("Chessboard pattern (%1x%2) not found in calibration files.").arg(board_size.width).arg(board_size.height),
                                 QMessageBox::Ok);
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
        debout << (ok ? "Calibration succeeded." : "Calibration failed.") << endl; //  "Avgage reprojection error is "  << avg_reproj_err << endl;
        debout << "Intrinsic reprojection error is: " << reproj_errs << endl;

        progress.setValue(mCalibFiles.size());

        debout << "Cameramatrix: " << endl;
        debout << "( " << camera_matrix.at<double>(0,0) << " " << camera_matrix.at<double>(0,1) << " " << camera_matrix.at<double>(0,2) << ")" << endl;
        debout << "( " << camera_matrix.at<double>(1,0) << " " << camera_matrix.at<double>(1,1) << " " << camera_matrix.at<double>(1,2) << ")" << endl;
        debout << "( " << camera_matrix.at<double>(2,0) << " " << camera_matrix.at<double>(2,1) << " " << camera_matrix.at<double>(2,2) << ")" << endl;
        //        debout << camera_matrix << endl;

        debout << "Distortioncoefficients: " << endl;
        debout << "r2: " << dist_coeffs.at<double>(0,0) << " r4: " << dist_coeffs.at<double>(0,1) << " r6: " << dist_coeffs.at<double>(0,4) << endl;
        debout << "tx: " << dist_coeffs.at<double>(0,2) << " ty: " << dist_coeffs.at<double>(0,3) << endl;
        debout << "k4: " << dist_coeffs.at<double>(0,5) << " k5: " << dist_coeffs.at<double>(0,6) << " k6: " << dist_coeffs.at<double>(0,7) << endl;
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
//            IplImage* ii = mMainWindow->getIplImage();
//            cvReleaseImage(&ii); // neue zeile, da in updateimage nicht mehr geloescht wird
            mMainWindow->updateImage(origImg); // now the last view will be deleted
        }
#endif
    }
}
 
int AutoCalib::runCalibration(vector<vector<Point2f> > image_points, Size img_size, Size board_size,
    float square_size, float aspect_ratio, int flags,
    Mat &camera_matrix, Mat &dist_coeffs, double *reproj_errs)//, Mat &extr_params)//,
    //double *reproj_errs)//, double* avg_reproj_err)
{
    int code;

    vector<vector<Point3f> > object_points;

    // initialize arrays of points
    for(int j=0; j<image_points.size();j++)
    {
        vector<Point3f> points_3d;
        for(int i=0; i<board_size.width*board_size.height;i++)
        {
           points_3d.push_back(Point3f(float((i/board_size.width)*square_size),float((i%board_size.width)*square_size),0));

        }
        object_points.push_back(points_3d);
    }

    vector<Mat> rot_vects, trans_vects;

//    for(i = 0; i < image_count; i++)
//    {
//        CvPoint2D32f* src_img_pt = (CvPoint2D32f*)reader.ptr;
//        CvPoint2D32f* dst_img_pt = ((CvPoint2D32f*)image_points->data.fl) + i*point_count;
//        CvPoint3D32f* obj_pt = ((CvPoint3D32f*)object_points->data.fl) + i*point_count;

//        for(j = 0; j < board_size.height; j++)
//            for(k = 0; k < board_size.width; k++)
//            {
//                *obj_pt++ = cvPoint3D32f(j*square_size, k*square_size, 0);
//                *dst_img_pt++ = *src_img_pt++;
//            }
//        CV_NEXT_SEQ_ELEM(image_points_seq->elem_size, reader);
//    }

//    cvSet(point_counts, cvScalar(point_count));

//    *extr_params = cvCreateMat(image_count, 6, CV_32FC1);
//    cvGetCols(*extr_params, &rot_vects, 0, 3);
//    cvGetCols(*extr_params, &trans_vects, 3, 6);

//    cvZero(camera_matrix);
//    cvZero(dist_coeffs);

    if(flags & CV_CALIB_FIX_ASPECT_RATIO)
    {
        camera_matrix.ptr<double>(0)[0] = aspect_ratio;
        camera_matrix.ptr<double>(1)[1] = 1.;
    }

    *reproj_errs = calibrateCamera(object_points,image_points,img_size,camera_matrix,dist_coeffs,rot_vects,trans_vects,flags);
//    cvCalibrateCamera2(object_points, image_points, point_counts,
//        img_size, camera_matrix, dist_coeffs,
//        &rot_vects, &trans_vects, flags);

    code = 1;//cvCheckArr(camera_matrix, CV_CHECK_QUIET) &&
//    cvCheckArr(dist_coeffs, CV_CHECK_QUIET) &&
//    cvCheckArr(*extr_params, CV_CHECK_QUIET);

//    *reproj_errs = cvCreateMat(1, image_count, CV_64FC1);
    //*avg_reproj_err =
    //         compute_reprojection_error( object_points, &rot_vects, &trans_vects,
    //             camera_matrix, dist_coeffs, image_points, point_counts, *reproj_errs );

//    cvReleaseMat(&object_points);
//    cvReleaseMat(&image_points);
//    cvReleaseMat(&point_counts);

    return code;
}

