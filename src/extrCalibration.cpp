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

#include <QtWidgets>
#include <QFileDialog>

#include "extrCalibration.h"
#include "pMessageBox.h"
#include "petrack.h"
#include "control.h"

#define MAX_AV_ERROR 20

ExtrCalibration::ExtrCalibration()
{
    mMainWindow = nullptr;
    mControlWidget = nullptr;
}

ExtrCalibration::~ExtrCalibration()
{
}

void ExtrCalibration::setMainWindow(Petrack *mw)
{
    mMainWindow = mw;
    mControlWidget = mw->getControlWidget();
    init();
}
void ExtrCalibration::init()
{
    rotation_matrix = new double[9];
    translation_vector = new double[3];
    translation_vector2 = new double[3];

    camValues = new double[9];
    distValues = new double[8];

    isExtCalib = false;
}
bool ExtrCalibration::isEmptyExtrCalibFile()
{
    return mExtrCalibFile.isEmpty();
}

void ExtrCalibration::setExtrCalibFile(const QString &f)
{
    mExtrCalibFile = f;
}

QString ExtrCalibration::getExtrCalibFile()
{
    if (!this->isEmptyExtrCalibFile())
        return mExtrCalibFile;
    else
        return QString();
}

bool ExtrCalibration::openExtrCalibFile(){

    if (mMainWindow)
    {
        static QString lastDir;

        if (!mExtrCalibFile.isEmpty())
            lastDir = QFileInfo(mExtrCalibFile).path();

        QString extrCalibFile = QFileDialog::getOpenFileName(mMainWindow, Petrack::tr("Open extrinisc calibration file with point correspondences"), lastDir, "3D-Calibration-File (*.3dc);;Text (*.txt);;All supported types (*.3dc *.txt);;All files (*.*)");
        if (!extrCalibFile.isEmpty())
        {
            mExtrCalibFile = extrCalibFile;
            return loadExtrCalibFile();
        }
    }
    return false;

}

/**
 * @brief Loads the extrinsic calibration from mExtrCalibFile
 *
 * This methods reads an extrinsic calibration in one of two formats:
 * First: 3D coordinates followed by corresponding 2D coordinates
 *
 *     x y z px py
 *
 * Second: Just 3D coordinates
 *
 *     x y z
 *
 * It is possible to optionally start the file with the number of lines:
 *
 *     2
 *     x1 y1 z1
 *     x2 y2 z2
 *
 * This is just going to be ignored. Comments start with "#".
 *
 * @return
 */
bool ExtrCalibration::loadExtrCalibFile(){

    bool all_ok = true;

    if( !mExtrCalibFile.isEmpty() )
    {
        if( mExtrCalibFile.right(4) == ".3dc" || mExtrCalibFile.right(4) == ".txt" )
        {
            QFile file(mExtrCalibFile);
            if( !file.open(QIODevice::ReadOnly | QIODevice::Text) )
            {
                PCritical(mMainWindow, QObject::tr("Petrack"), QObject::tr("Error: Cannot open %1:\n%2.").arg(mExtrCalibFile, file.errorString()));
                return false;
            }

            debout << "Reading 3D calibration data from " << mExtrCalibFile << "..." << std::endl;


            std::vector<cv::Point3f> points3D_tmp;
            std::vector<cv::Point2f> points2D_tmp;

            QTextStream in(&file);
            QString line;
            int line_counter = 0, counter;
            float x,y,z,px,py;
            float zahl;
            bool with_2D_data = false,
                 with_3D_data = false,
                 end_loop = false;

            // Exit loop when reaching the end of the file
            while( !in.atEnd() )
            {
                // Neue Zeile einlesen
                line = in.readLine();
                ++line_counter;

                // Kommentare ueberlesen
                if( line.startsWith("#",Qt::CaseInsensitive) ||
                    line.startsWith(";;",Qt::CaseInsensitive) ||
                    line.startsWith("//",Qt::CaseInsensitive) ||
                    line.startsWith("!",Qt::CaseInsensitive) )
                    continue;

                QTextStream stream(&line);
                counter = 0;
                end_loop = false;

                while( !stream.atEnd() && !end_loop )
                {
                    stream >> zahl;
                    ++counter;

                    switch( counter )
                    {
                    case 1:
                        x = zahl;
                        if( !with_3D_data )
                        {
                            points3D_tmp.clear();
                            with_3D_data = true;
                        }
                        break;
                    case 2:
                        y = zahl;
                        break;
                    case 3:
                        z = zahl;
                        break;
                    case 4:
                        px = zahl;
                        if( !with_2D_data )
                        {
                            points2D_tmp.clear();
                            with_2D_data = true;
                        }
                        break;
                    case 5:
                        py = zahl;
                        break;
                    default:
                        end_loop = true;
                    }

                }
                if( counter == 1 )
                {
                    debout << "Optional number of points in line " << line_counter << " ignored." << std::endl;
                }else if( counter != 3 && counter != 5 )
                    debout << "Something wrong in line " << line_counter << "( " << line << " )! Ignored. (counter=" << counter << ")" << std::endl;

                // 3D daten abspeichern
                if( with_3D_data && (counter == 3 || counter == 5) )
                {
                    //debout << "x: " << x << " y: " << y << " z: " << z << endl;
                    points3D_tmp.push_back( cv::Point3f( x, y, z ) );
                }
                // 2D daten abspeichern
                if( with_2D_data && counter == 5 )
                {
                    //debout << " px: " << px << " py: " << py << endl;
                    points2D_tmp.push_back( cv::Point2f( px, py ) );
                }



            }
            // Check if there are more than 4 points for calibration in the file
             if( points3D_tmp.size() < 4 )
            {
                PCritical(mMainWindow, QObject::tr("PeTrack"), QObject::tr("Error: Not enough points given: %1 (minimum 4 needed!). Please check your extrinsic calibration file!").arg(points3D_tmp.size()));
                all_ok = false;
            }

            // Check if 2D points delivered and if the number of 2D and 3D points agree
            else if( points2D_tmp.size() > 0 && points2D_tmp.size() != points3D_tmp.size() )
            {
                PCritical(mMainWindow, QObject::tr("PeTrack"), QObject::tr("Error: Unsupported File Format in: %1 (number of 3D (%2) and 2D (%3) points disagree!)").arg(mExtrCalibFile).arg(points3D_tmp.size()).arg(points2D_tmp.size()));
                all_ok = false;
            }
            // Check if number of loaded 3D points agree with stored 2D points
            else if( !with_2D_data && points2D.size()>0 && points3D_tmp.size() != points2D.size() )
            {
                // ask if stored 2D points should be deleted?
                int result = PWarning(mMainWindow,
                                                  QObject::tr("PeTrack"),
                                                  QObject::tr("Number of 3D points (%1) disagree with number of stored 2D points (%2)!<br />The 2D points will be deleted! You have to fetch new ones from the image!").arg(points3D_tmp.size()).arg(points2D.size()),
                                                  PMessageBox::StandardButton::Ok | PMessageBox::StandardButton::Abort);
                if (result != PMessageBox::StandardButton::Ok)
                    all_ok = false;
                else
                    points2D.clear();

            }
            if( all_ok )
            {
                if( with_3D_data ) points3D = points3D_tmp;
                if( with_2D_data ) points2D = points2D_tmp;
            }
        }else
        {
            debout << "unsupported file extension (supported: .3dc,.txt)" << std::endl;
        }
    }else
    {
        // no calib_file
        all_ok = false;
    }
    if (all_ok && !mMainWindow->isLoading())
        calibExtrParams();
    return all_ok;
}

/**
 * @brief Uses manually set TrackPoints as 2D points for extrinsic calibration
 *
 * @pre loaded at least 4 3D-points
 *
 * @return true if calibration did take place
 */
bool ExtrCalibration::fetch2DPoints()
{
    bool all_ok = true;
    if( !mMainWindow->getTracker() || mMainWindow->getTracker()->size() < 4 )
    {
       PCritical(mMainWindow, QObject::tr("Petrack"), QObject::tr("Error: At minimum four 3D calibration points needed for 3D calibration."));
       all_ok = false;
    }else
    {
        size_t sz_2d = mMainWindow->getTracker()->size();

        if( points3D.size()>0 && sz_2d != points3D.size() ){
            PCritical(mMainWindow, QObject::tr("Petrack"), QObject::tr("Count of 2D-Points (%1) and 3D-Points (%2) disagree").arg(sz_2d).arg(points3D.size()));
            all_ok = false;

        }
        //debout << "Marked 2D-Image-Points: " << endl;
        if( all_ok )
        {
            points2D.clear();

            for(int i = 0; i < static_cast<int>(sz_2d); i++)
            {
                //debout << "[" << i << "]: (" << mMainWindow->getTracker()->at(i).at(0).x() << ", " << mMainWindow->getTracker()->at(i).at(0).y() << ")" << endl;
                // Info: Tracker->TrackPerson->TrackPoint->Vec2F
                points2D.push_back(cv::Point2f(mMainWindow->getTracker()->at(i).at(0).x(),mMainWindow->getTracker()->at(i).at(0).y()));
            }
        }
    }
    if( all_ok )
    {
        mMainWindow->getTracker()->clear();
        calibExtrParams();
    }
    return all_ok;
}

/**
 * @brief Saves points used for extrinsic calibration
 *
 * Saves the points used for extrinsic calibration in the format:
 *
 *     n
 *     x y z px py
 *
 * With n as number of points, x,y,z as 3D coordianted and px,py as 2D coordinates.
 * @return
 */
bool ExtrCalibration::saveExtrCalibPoints()
{
    bool all_okay = false;

    QString out_str;
    QTextStream out(&out_str);


    for (size_t i = 0; i < points3D.size(); ++i)
    {
        out << "[" << QString::number(i+1,'i',0) << "]: "<< QString::number(points3D.at(i).x,'f',1) << " " << QString::number(points3D.at(i).y,'f',1) << " " << QString::number(points3D.at(i).z,'f',1) << " " << QString::number(points2D.at(i).x,'f',3) << " " << QString::number(points2D.at(i).y,'f',3) << Qt::endl;
    }

    QIcon icon = QApplication::style()->standardIcon(QStyle::SP_MessageBoxWarning);
    PMessageBox msgBox {nullptr, "PeTrack",
                       "The corresponding calibration points have been changed.\n"
                       "Do you want to save your changes?",
                       QIcon(),
                       out_str,
                       PMessageBox::StandardButton::Save | PMessageBox::StandardButton::Cancel,
                       PMessageBox::StandardButton::Save};
    int ret = msgBox.exec();
    switch (ret) {
        case PMessageBox::StandardButton::Save:
        {
           // Save was clicked
            QFile file(mExtrCalibFile);

            if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
            {
              PCritical(mMainWindow, QObject::tr("Petrack"), QObject::tr("Cannot open %1:\n%2.").arg(mExtrCalibFile).arg(file.errorString()));
              return false;
            }

            QTextStream file_out(&file);

            file_out << points3D.size() << Qt::endl;
            for (size_t i = 0; i < points3D.size(); ++i)
            {
                file_out << points3D.at(i).x << " " << points3D.at(i).y << " " << points3D.at(i).z << " " << points2D.at(i).x << " " << points2D.at(i).y << Qt::endl;
            }
            all_okay = file.flush();
            file.close();
           break;
        }
        case PMessageBox::StandardButton::Discard:
           // Don't Save was clicked
           break;
        case PMessageBox::StandardButton::Cancel:
           // Cancel was clicked
           break;
       default:
           // should never be reached
           break;
    }

    return all_okay;
}

bool ExtrCalibration::isSetExtrCalib(){
    return true;
}

/**
 * @brief Extrinsic calibration with help of cv::solvePnP
 */
void ExtrCalibration::calibExtrParams()
{

    if( !points3D.empty() && !points2D.empty() && points2D.size() == points3D.size() )
    {

        int bS = mMainWindow->getImageBorderSize();
        /* Create Camera-Matrix form Camera-Params in the Petrack-GUI */
        cv::Mat camMat = (cv::Mat_<double>(3,3) <<
                      mControlWidget->fx->value(), 0, mControlWidget->cx->value()-bS,
                      0, mControlWidget->fy->value(), mControlWidget->cy->value()-bS,
                      0, 0, 1);

        cv::Mat distMat = (cv::Mat_<double>(8,1) <<
                       0,//mControlWidget->r2->value(),
                       0,//mControlWidget->r4->value(),
                       0,//mControlWidget->tx->value(),
                       0,//mControlWidget->ty->value(),
                       // r^>4 not supported
                       0,//mControlWidget->r6->value(),
                       0,//mControlWidget->k4->value(),
                       0,//mControlWidget->k5->value(),
                       0//mControlWidget->k6->value()
                    );

        /* Create Mat-objects of point correspondences */
        cv::Mat op(points3D);
        cv::Mat ip(points2D);

        /* Mat-objects for result rotation and translation vectors */
        cv::Mat rvec(3,1,CV_64F),/*,0),*/ tvec(3,1,CV_64F);//,0);

        // Solve the PnP-Problem to calibrate the camera to its environment

        cv::solvePnP(op,ip,camMat,distMat,rvec,tvec,false,cv::SOLVEPNP_ITERATIVE);

        cv::Mat rot_mat(3,3,CV_64F);//, 0);
        // Transform the rotation vector into a rotation matrix with opencvs rodrigues method
        Rodrigues(rvec,rot_mat);

        rotation_matrix[0] = rot_mat.at<double>(0,0);
        rotation_matrix[1] = rot_mat.at<double>(0,1);
        rotation_matrix[2] = rot_mat.at<double>(0,2);
        rotation_matrix[3] = rot_mat.at<double>(1,0);
        rotation_matrix[4] = rot_mat.at<double>(1,1);
        rotation_matrix[5] = rot_mat.at<double>(1,2);
        rotation_matrix[6] = rot_mat.at<double>(2,0);
        rotation_matrix[7] = rot_mat.at<double>(2,1);
        rotation_matrix[8] = rot_mat.at<double>(2,2);

        translation_vector[0] = tvec.at<double>(0,0);
        translation_vector[1] = tvec.at<double>(0,1);
        translation_vector[2] = tvec.at<double>(0,2);

        translation_vector2[0] =
                rotation_matrix[0] * translation_vector[0] +
                rotation_matrix[3] * translation_vector[1] +
                rotation_matrix[6] * translation_vector[2];
        translation_vector2[1] =
                rotation_matrix[1] * translation_vector[0] +
                rotation_matrix[4] * translation_vector[1] +
                rotation_matrix[7] * translation_vector[2];
        translation_vector2[2] =
                rotation_matrix[2] * translation_vector[0] +
                rotation_matrix[5] * translation_vector[1] +
                rotation_matrix[8] * translation_vector[2];

        debout << "-.- ESTIMATED ROTATION -.-" << std::endl;
        for ( size_t p=0; p<3; p++ )
            debout << rotation_matrix[p*3] << " , " << rotation_matrix[p*3+1] << " , " << rotation_matrix[p*3+2] << std::endl;

        debout << "-.- ESTIMATED TRANSLATION -.-" << std::endl;
        debout << translation_vector[0] << " , " << translation_vector[1] << " , " << translation_vector[2] << std::endl;

        debout << "-.- Translation vector -.-" << std::endl;
        debout << translation_vector2[0] << " , " << translation_vector2[1] << " , " << translation_vector2[2] << std::endl;

        debout << "-.- Rotation vector -.-" << std::endl;
        debout << rvec.at<double>(0,0) << " , " << rvec.at<double>(1,0) << " , " << rvec.at<double>(2,0) << std::endl;

        camHeight = translation_vector2[2] < 0 ? -translation_vector2[2] : translation_vector2[2];

        mControlWidget->setCalibExtrRot1(rvec.at<double>(0,0));
        mControlWidget->setCalibExtrRot2(rvec.at<double>(1,0));
        mControlWidget->setCalibExtrRot3(rvec.at<double>(2,0));

        mControlWidget->setCalibExtrTrans1(translation_vector2[0]);
        mControlWidget->setCalibExtrTrans2(translation_vector2[1]);
        mControlWidget->setCalibExtrTrans3(translation_vector2[2]);

        if ( !calcReprojectionError() )
        {
            std::cout << "# Warning: extrinsic calibration not possible! Please select other 2D/3D points!" << std::endl;

            mControlWidget->setCalibExtrRot1(0);
            mControlWidget->setCalibExtrRot2(0);
            mControlWidget->setCalibExtrRot3(0);

            translation_vector2[0] = 0;
            translation_vector2[1] = 0;
            translation_vector2[2] = 0;

            rotation_matrix[0] = 0;
            rotation_matrix[1] = 0;
            rotation_matrix[2] = 0;

            mControlWidget->setCalibExtrTrans1(translation_vector2[0]);
            mControlWidget->setCalibExtrTrans2(translation_vector2[1]);
            mControlWidget->setCalibExtrTrans3(translation_vector2[2]);

            reprojectionError = ReprojectionError{};

            PCritical(mMainWindow, QObject::tr("Petrack"), QObject::tr("Error: Could not calculate extrinsic calibration. Please select other 2D/3D point correspondences for extrinsic calibration!"));

            isExtCalib = false;

            return;
        }

        isExtCalib = true;

        std::cout << "End of extern calibration!" << std::endl;
    }else
    {
        std::cerr << "# Warning: invalid point correspondences for camera calibration." << std::endl;
        std::cerr << "# 2D points:" << points2D.size() << ", 3D points: " << points3D.size() << std::endl;
    }
    mMainWindow->getScene()->update();

}

/**
 * @brief Calculates the reprojection Error
 *
 * This method calculates following errors and their variance:
 * <ul>
 * <li>2D Point to 3D Point against 3D Point - using calibration points</li>
 * <li>3D to 2D to 3D against 2D to 3D - using default height for calib. points</li>
 * <li>3D to 2D against 2D - using calib. points</li>
 * </ul>
 * @return
 */
bool ExtrCalibration::calcReprojectionError()
{
    //////
    /// \brief error measurements
    ///
    double val, max_px = -1.0, max_pH = -1.0, max_dH = -1.0,
          var_px = 0, sd_px = 0, var_pH = 0, sd_pH = 0, var_dH = 0, sd_dH = 0,
          sum_px = 0, sum_pH = 0, sum_dH = 0;

    size_t num_points = get2DList().size();
    if(num_points == 0 || num_points != get3DList().size()){
        reprojectionError = ReprojectionError{};
        return false;
    }

    bool debug = false;
    for(size_t i=0; i< num_points; i++)
    {
        cv::Point2f p2d = get2DList().at(i);
        cv::Point3f p3d = get3DList().at(i);
        p3d.x -= mControlWidget->getCalibCoord3DTransX();
        p3d.y -= mControlWidget->getCalibCoord3DTransY();
        p3d.z -= mControlWidget->getCalibCoord3DTransZ();
        cv::Point2f p3dTo2d = getImagePoint(p3d);

        // Error measurements metric (cm)
        cv::Point3f p2dTo3d = get3DPoint(p2d,p3d.z);

        cv::Point3f p2dTo3dMapDefaultHeight = get3DPoint(p2d,mControlWidget->mapDefaultHeight->value());

        cv::Point3f p3dTo2dTo3dMapDefaultHeight = get3DPoint(p3dTo2d,mControlWidget->mapDefaultHeight->value());

        val = sqrt(pow(p3d.x - p2dTo3d.x,2) + pow(p3d.y - p2dTo3d.y,2));
        if ( val > max_pH ) max_pH = val;
            sum_pH += val;
        if( debug ) debout << "Error point[" << i << "]: " << val << std::endl;

        val = sqrt(pow(p3dTo2dTo3dMapDefaultHeight.x - p2dTo3dMapDefaultHeight.x,2) + pow(p3dTo2dTo3dMapDefaultHeight.y-p2dTo3dMapDefaultHeight.y,2));
        if ( val > max_dH ) max_dH = val;
            sum_dH += val;
        if( debug ) debout << "Error point[" << i << "]: " << val << std::endl;

        // Error measurements pixel
        val = sqrt(pow(p3dTo2d.x - p2d.x,2) + pow(p3dTo2d.y - p2d.y,2));
        // Maximum
        if ( val > max_px ) max_px = val;
            sum_px += val;
        if( debug ) debout << "Error point[" << i << "]: " << val << std::endl;

    }
    for(size_t i=0; i< num_points; i++)
    {
        cv::Point2f p2d = get2DList().at(i);
        cv::Point3f p3d = get3DList().at(i);
        p3d.x -= mControlWidget->getCalibCoord3DTransX();
        p3d.y -= mControlWidget->getCalibCoord3DTransY();
        p3d.z -= mControlWidget->getCalibCoord3DTransZ();
        cv::Point2f p3d_to_2d = getImagePoint(p3d);

        // Error measurements metric (cm)
        cv::Point3f p2d_to_3d = get3DPoint(p2d,p3d.z);

        cv::Point3f p2d_to_3d_mapDefaultHeight = get3DPoint(p2d,mControlWidget->mapDefaultHeight->value()); // mStatusPosRealHeight->value()); ?

        cv::Point3f p3d_to2d_to3d_mapDefaultHeight = get3DPoint(p3d_to_2d,mControlWidget->mapDefaultHeight->value());

        val = pow(sqrt(pow(p3d.x-p2d_to_3d.x,2) + pow(p3d.y-p2d_to_3d.y,2))-(sum_pH/num_points),2);
        var_pH += val;

        val = pow(sqrt(pow(p3d_to2d_to3d_mapDefaultHeight.x-p2d_to_3d_mapDefaultHeight.x,2) + pow(p3d_to2d_to3d_mapDefaultHeight.y-p2d_to_3d_mapDefaultHeight.y,2))-(sum_dH/num_points),2);
        var_dH += val;

        val = pow(sqrt(pow(p3d_to_2d.x-p2d.x,2) + pow(p3d_to_2d.y-p2d.y,2))-(sum_px/num_points),2);
        var_px += val;

    }

    // average
    sum_pH /= num_points;
    var_pH /= num_points;
    sd_pH = sqrt(var_pH);
    debout << "Reprojection error (pointHeight) average: " << sum_pH << "cm (standard deviation: " << sd_pH << " variance: " << var_pH << " Max error: " << max_pH << "cm)" << std::endl;

    // average
    sum_dH /= num_points;
    var_dH /= num_points;
    sd_dH = sqrt(var_dH);
    debout << "Reprojection error (defaultHeight=" << mControlWidget->mapDefaultHeight->value() << ") average: " << sum_dH << "cm (standard deviation: " << sd_dH << " variance: " << var_dH << " Max error: " << max_dH << "cm)" << std::endl;

    // average
    sum_px /= num_points;
    var_px /= num_points;
    sd_px = sqrt(var_px);
    debout << "Reprojection error (Pixel) average: " << sum_px << "px (standard deviation: " << sd_px << " variance: " << var_px << " Max error: " << max_px << "px)" << std::endl;

    reprojectionError = ReprojectionError{
        sum_pH, sd_pH, var_pH, max_pH,
        sum_dH, sd_dH, var_dH, max_dH,
        sum_px, sd_px, var_px, max_px,
        mControlWidget->mapDefaultHeight->value()};

    return reprojectionError.pointHeightAvg() > MAX_AV_ERROR ? false : true; // Falls pixel fehler im schnitt > 20 ist das Ergebnis nicht akzeptabel
}

/**
 * @brief Projects the 3D point to the image plane
 *
 * Projection is done by multiplying with the external camera matrix
 * composed out of rotation and translation aquired in ExtrCalibration::calibExtrParams().
 * After that, the internal camera matrix is applied.
 *
 * @param p3d 3D point to transform
 * @return calculated 2D projection of p3d
 */
cv::Point2f ExtrCalibration::getImagePoint(cv::Point3f p3d)
{
    bool debug = false;

    p3d.x *= mControlWidget->getCalibCoord3DSwapX() ? -1 : 1;
    p3d.y *= mControlWidget->getCalibCoord3DSwapY() ? -1 : 1;
    p3d.z *= mControlWidget->getCalibCoord3DSwapZ() ? -1 : 1;

    // Adding the coordsystem translation from petrack window
    p3d.x += mControlWidget->getCalibCoord3DTransX();
    p3d.y += mControlWidget->getCalibCoord3DTransY();
    p3d.z += mControlWidget->getCalibCoord3DTransZ();

    if( debug ) std::cout << "getImagePoint: Start Point3D: (" << p3d.x << ", " << p3d.y << ", " << p3d.z << ")" << std::endl;
    // ToDo: use projectPoints();
    int bS = mMainWindow->getImage() ? mMainWindow->getImageBorderSize() : 0;

    double rvec_array[3], translation_vector[3];

    rvec_array[0] = mControlWidget->getCalibExtrRot1();
    rvec_array[1] = mControlWidget->getCalibExtrRot2();
    rvec_array[2] = mControlWidget->getCalibExtrRot3();

    cv::Mat rvec(3,1,CV_64F, rvec_array), rot_inv;
    cv::Mat rot_mat(3,3,CV_64F), e(3,3,CV_64F);
    // Transform the rotation vector into a rotation matrix with opencvs rodrigues method
    Rodrigues(rvec,rot_mat);

    // use inverse Matrix to get translation_vector?
    rot_inv = rot_mat.inv(cv::DECOMP_SVD);

    e = rot_inv*rot_mat;

    translation_vector[0] =
            rot_mat.at<double>(0,0)*mControlWidget->getCalibExtrTrans1()+
            rot_mat.at<double>(0,1)*mControlWidget->getCalibExtrTrans2()+
            rot_mat.at<double>(0,2)*mControlWidget->getCalibExtrTrans3();
    translation_vector[1] =
            rot_mat.at<double>(1,0)*mControlWidget->getCalibExtrTrans1()+
            rot_mat.at<double>(1,1)*mControlWidget->getCalibExtrTrans2()+
            rot_mat.at<double>(1,2)*mControlWidget->getCalibExtrTrans3();
    translation_vector[2] =
            rot_mat.at<double>(2,0)*mControlWidget->getCalibExtrTrans1()+
            rot_mat.at<double>(2,1)*mControlWidget->getCalibExtrTrans2()+
            rot_mat.at<double>(2,2)*mControlWidget->getCalibExtrTrans3();

    if( debug )
    {
        std::cout << "\n-.- ESTIMATED ROTATION\n";
        for ( int p=0; p<3; p++ )
            printf("%20.18f, %20.18f, %20.18f\n",rot_mat.at<double>(p,0),rot_mat.at<double>(p,1),rot_mat.at<double>(p,2));

        std::cout << "\n-.- ESTIMATED ROTATION^-1\n";
        for ( int p=0; p<3; p++ )
            printf("%20.18f, %20.18f, %20.18f\n",rot_inv.at<double>(p,0),rot_inv.at<double>(p,1),rot_inv.at<double>(p,2));

        std::cout << "\n-.- ESTIMATED R^-1*R\n";
        for ( int p=0; p<3; p++ )
            printf("%20.18f, %20.18f, %20.18f\n",e.at<double>(p,0),e.at<double>(p,1),e.at<double>(p,2));

        std::cout << "\n-.- ESTIMATED TRANSLATION\n";
        printf("%20.15f, %20.15f, %20.15f\n",translation_vector[0],translation_vector[1],translation_vector[2]);
    }
    cv::Point3f point3D;

    point3D.x =
            rot_mat.at<double>(0,0) * p3d.x +
            rot_mat.at<double>(0,1) * p3d.y +
            rot_mat.at<double>(0,2) * p3d.z +
            translation_vector[0];
    point3D.y =
            rot_mat.at<double>(1,0) * p3d.x +
            rot_mat.at<double>(1,1) * p3d.y +
            rot_mat.at<double>(1,2) * p3d.z +
            translation_vector[1];
    point3D.z =
            rot_mat.at<double>(2,0) * p3d.x +
            rot_mat.at<double>(2,1) * p3d.y +
            rot_mat.at<double>(2,2) * p3d.z +
            translation_vector[2];

    if( debug ) std::cout << "###### After extern calibration: (" << point3D.x << ", " << point3D.y << ", " << point3D.z << ")" << std::endl;

    cv::Point2f point2D = cv::Point2f( 0.0, 0.0 );
    if ( point3D.z != 0 )
    {
        point2D.x = (mControlWidget->fx->value() * point3D.x) / point3D.z + (mControlWidget->cx->value()-bS);
        point2D.y = (mControlWidget->fy->value() * point3D.y) / point3D.z + (mControlWidget->cy->value()-bS);
    }
    if (false && bS > 0)
    {
        point2D.x += bS;
        point2D.y += bS;
    }
    return point2D;
}


/**
 * @brief Tranforms a 2D point into a 3D point with given height.
 *
 * @param p2d 2D pixel point (without border)
 * @param h height i.e. distance to xy-plane
 * @return calculated 3D point
 */
cv::Point3f ExtrCalibration::get3DPoint(cv::Point2f p2d, double h)
{
        bool debug = false;

        if( debug ) std::cout << "get3DPoint: Start Point2D: (" << p2d.x << ", " << p2d.y << ") h: " << h << std::endl;

        int bS = mMainWindow->getImage() ? mMainWindow->getImageBorderSize() : 0;

        if (false && bS > 0)
        {
            p2d.x += bS;
            p2d.y += bS;
        }

        // Ergebnis 3D-Punkt
        cv::Point3f resultPoint, tmpPoint;


        bool newMethod = true;
        /////////////// Start new method
        if( newMethod )
        {
        double rvec_array[3], translation_vector[3];
        rvec_array[0] = mControlWidget->getCalibExtrRot1();
        rvec_array[1] = mControlWidget->getCalibExtrRot2();
        rvec_array[2] = mControlWidget->getCalibExtrRot3();

        cv::Mat rvec(3,1,CV_64F, rvec_array), rot_inv;
        cv::Mat rot_mat(3,3,CV_64F), e(3,3,CV_64F);
        // Transform the rotation vector into a rotation matrix with opencvs rodrigues method
        Rodrigues(rvec,rot_mat);

        translation_vector[0] =
                rot_mat.at<double>(0,0)*mControlWidget->getCalibExtrTrans1()+
                rot_mat.at<double>(0,1)*mControlWidget->getCalibExtrTrans2()+
                rot_mat.at<double>(0,2)*mControlWidget->getCalibExtrTrans3();
        translation_vector[1] =
                rot_mat.at<double>(1,0)*mControlWidget->getCalibExtrTrans1()+
                rot_mat.at<double>(1,1)*mControlWidget->getCalibExtrTrans2()+
                rot_mat.at<double>(1,2)*mControlWidget->getCalibExtrTrans3();
        translation_vector[2] =
                rot_mat.at<double>(2,0)*mControlWidget->getCalibExtrTrans1()+
                rot_mat.at<double>(2,1)*mControlWidget->getCalibExtrTrans2()+
                rot_mat.at<double>(2,2)*mControlWidget->getCalibExtrTrans3();

        // use inverse Matrix
        rot_inv = rot_mat.inv(cv::DECOMP_LU);
        e = rot_inv*rot_mat;

        if( debug )
        {
        debout << "\n-.- ESTIMATED ROTATION\n";
        for ( int p=0; p<3; p++ )
            printf("%20.18f, %20.18f, %20.18f\n",rot_mat.at<double>(p,0),rot_mat.at<double>(p,1),rot_mat.at<double>(p,2));

        debout << "\n-.- ESTIMATED ROTATION^-1\n";
        for ( int p=0; p<3; p++ )
            printf("%20.18f, %20.18f, %20.18f\n",rot_inv.at<double>(p,0),rot_inv.at<double>(p,1),rot_inv.at<double>(p,2));

        debout << "\n-.- ESTIMATED R^-1*R\n";
        for ( int p=0; p<3; p++ )
            printf("%20.18f, %20.18f, %20.18f\n",e.at<double>(p,0),e.at<double>(p,1),e.at<double>(p,2));

        debout << "\n-.- ESTIMATED TRANSLATION\n";
        debout << mControlWidget->getCalibExtrTrans1() << " , " << mControlWidget->getCalibExtrTrans2() << " , " << mControlWidget->getCalibExtrTrans3() << "\n";
        debout << translation_vector[0] << " , " << translation_vector[1] << " , " << translation_vector[2] << "\n";

        debout << "Det(rot_mat): "<< determinant(rot_mat) << std::endl;
        debout << "Det(rot_inv): "<< determinant(rot_inv) << std::endl;
        }
        double z = h + rot_inv.at<double>(2,0)*translation_vector[0] +
                       rot_inv.at<double>(2,1)*translation_vector[1] +
                       rot_inv.at<double>(2,2)*translation_vector[2];
        if( debug )
        {
        debout << "##### z: " << h << " + " << rot_inv.at<double>(2,0) << "*" << translation_vector[0] << " + "
                                          << rot_inv.at<double>(2,1) << "*" << translation_vector[1] << " + "
                                          << rot_inv.at<double>(2,2) << "*" << translation_vector[2] << " = " << z << std::endl;
        }
        z /= (rot_inv.at<double>(2,0)*(p2d.x-(mControlWidget->getCalibCxValue()-bS))/mControlWidget->getCalibFxValue() +
                 rot_inv.at<double>(2,1)*(p2d.y-(mControlWidget->getCalibCyValue()-bS))/mControlWidget->getCalibFyValue() +
                 rot_inv.at<double>(2,2));
        if( debug ) std::cout << "###### z: "<< z << std::endl;

        resultPoint.x = (p2d.x-(mControlWidget->getCalibCxValue()-bS));
        resultPoint.y = (p2d.y-(mControlWidget->getCalibCyValue()-bS));
        resultPoint.z = z;

        if( debug ) std::cout << "###### (" << resultPoint.x << ", " << resultPoint.y << ", " << resultPoint.z << ")" << std::endl;

        resultPoint.x = resultPoint.x * z/mControlWidget->getCalibFxValue();
        resultPoint.y = resultPoint.y * z/mControlWidget->getCalibFyValue();

        if( debug ) std::cout << "###### After intern re-calibration: (" << resultPoint.x << ", " << resultPoint.y << ", " << resultPoint.z << ")" << std::endl;

        tmpPoint.x = resultPoint.x - translation_vector[0];
        tmpPoint.y = resultPoint.y - translation_vector[1];
        tmpPoint.z = resultPoint.z - translation_vector[2];

        if( debug ) std::cout << "###### After translation: (" << tmpPoint.x << ", " << tmpPoint.y << ", " << tmpPoint.z << ")" << std::endl;

        resultPoint.x = rot_inv.at<double>(0,0)*(tmpPoint.x)+
                        rot_inv.at<double>(0,1)*(tmpPoint.y)+
                        rot_inv.at<double>(0,2)*(tmpPoint.z);
        resultPoint.y = rot_inv.at<double>(1,0)*(tmpPoint.x)+
                        rot_inv.at<double>(1,1)*(tmpPoint.y)+
                        rot_inv.at<double>(1,2)*(tmpPoint.z);
        resultPoint.z = rot_inv.at<double>(2,0)*(tmpPoint.x)+
                        rot_inv.at<double>(2,1)*(tmpPoint.y)+
                        rot_inv.at<double>(2,2)*(tmpPoint.z);

        if( debug ) std::cout << "#resultPoint: (" << resultPoint.x << ", " << resultPoint.y << ", " << resultPoint.z << ")" << std::endl;
        if( debug ) std::cout << "Coord Translation: x: " << mControlWidget->getCalibCoord3DTransX() << ", y: " << mControlWidget->getCalibCoord3DTransY() << ", z: " << mControlWidget->getCalibCoord3DTransZ() << std::endl;


        // Coordinate Transformations

        resultPoint.x -= mControlWidget->getCalibCoord3DTransX();
        resultPoint.y -= mControlWidget->getCalibCoord3DTransY();
        resultPoint.z -= mControlWidget->getCalibCoord3DTransZ();

        resultPoint.x *= mControlWidget->getCalibCoord3DSwapX() ? -1 : 1;
        resultPoint.y *= mControlWidget->getCalibCoord3DSwapY() ? -1 : 1;
        resultPoint.z *= mControlWidget->getCalibCoord3DSwapZ() ? -1 : 1;


        }else//////////////// End new method
        {
        //////////////// Start old method

        cv::Point3f camInWorld = transformRT(cv::Point3f(0,0,0));

        // 3D-Punkt vor der Kamera mit Tiefe 5
        CvPoint3D32f pointBeforeCam;
        pointBeforeCam.x = (p2d.x - mControlWidget->cx->value()) / mControlWidget->fx->value() * 50;
        pointBeforeCam.y = (p2d.y - mControlWidget->cy->value()) / mControlWidget->fy->value() * 50;
        pointBeforeCam.z = 50;
        if( debug ) std::cout << "Point before Camera: [" << pointBeforeCam.x << ", " << pointBeforeCam.y << ", " << pointBeforeCam.z << "]" << std::endl;
        // 3D-Punkt vor Kamera in Weltkoordinaten
        cv::Point3f pBCInWorld = transformRT(pointBeforeCam);
        if( debug ) std::cout << "Point before Camera in World-Coordinatesystem: [" << pBCInWorld.x << ", " << pBCInWorld.y << ", " << pBCInWorld.z << "]" << std::endl;
        if( debug ) std::cout << "Camera in World-Coordinatesystem: [" << camInWorld.x << ", " << camInWorld.y << ", " << camInWorld.z << "]" << std::endl;
        // Berechnung des Richtungsvektors der Gerade von der Kamera durch den Pixel
        // Als Sttzvektor der Geraden wird die Position der Kamera gewhlt
        pBCInWorld.x -= camInWorld.x;
        pBCInWorld.y -= camInWorld.y;
        pBCInWorld.z -= camInWorld.z;
        if( debug ) std::cout << "G:x = (" << camInWorld.x << " / " << camInWorld.y << " / " << camInWorld.z << ") + lambda (" << pBCInWorld.x << " / " << pBCInWorld.y << " / " << pBCInWorld.z << ")" << std::endl;

        // Berechnung des Schnittpunktes: Hier lambda von der Geraden
        double lambda = (h -camInWorld.z) / (pBCInWorld.z);
        if( debug ) std::cout << "Lambda: " << lambda << std::endl;

        // Lambda in Gerade einsetzen
        resultPoint.x = (mControlWidget->getCalibCoord3DSwapX() ? -1 : 1) * (camInWorld.x + lambda * pBCInWorld.x);
        resultPoint.y = (mControlWidget->getCalibCoord3DSwapY() ? -1 : 1) * (camInWorld.y + lambda * pBCInWorld.y);
        resultPoint.z = (mControlWidget->getCalibCoord3DSwapZ() ? -1 : 1) * (camInWorld.z + lambda * pBCInWorld.z);

        }//////////////// End old method

        return resultPoint;
}
/**
  * Transformiert den angegebenen 3D-Punkt mit der Rotation und Translation
  * um Umrechnungen zwischen verschiedenen Koordinatensystemen zu ermglichen
  */
cv::Point3f ExtrCalibration::transformRT(cv::Point3f p)
{
    // ToDo: use projectPoints();

    double rvec_array[3], rotation_matrix[9];
    rvec_array[0] = mControlWidget->getCalibExtrRot1();
    rvec_array[1] = mControlWidget->getCalibExtrRot2();
    rvec_array[2] = mControlWidget->getCalibExtrRot3();

    cv::Mat rvec(3,1,CV_64F, rvec_array);
    cv::Mat rot_mat(3,3,CV_64F);
    // Transform the rotation vector into a rotation matrix with opencvs rodrigues method
    Rodrigues(rvec,rot_mat);

    rotation_matrix[0] = rot_mat.at<double>(0,0);
    rotation_matrix[1] = rot_mat.at<double>(0,1);
    rotation_matrix[2] = rot_mat.at<double>(0,2);
    rotation_matrix[3] = rot_mat.at<double>(1,0);
    rotation_matrix[4] = rot_mat.at<double>(1,1);
    rotation_matrix[5] = rot_mat.at<double>(1,2);
    rotation_matrix[6] = rot_mat.at<double>(2,0);
    rotation_matrix[7] = rot_mat.at<double>(2,1);
    rotation_matrix[8] = rot_mat.at<double>(2,2);

    cv::Point3f point3D;

    point3D.x = rotation_matrix[0] * p.x +
            rotation_matrix[3] * p.y +
            rotation_matrix[6] * p.z -
            mControlWidget->trans1->value();
    point3D.y = rotation_matrix[1] * p.x +
            rotation_matrix[4] * p.y +
            rotation_matrix[7] * p.z -
            mControlWidget->trans2->value();
    point3D.z = rotation_matrix[2] * p.x +
            rotation_matrix[5] * p.y +
            rotation_matrix[8] * p.z -
            mControlWidget->trans3->value();
    return point3D;
}
bool ExtrCalibration::isOutsideImage(cv::Point2f p2d)
{
    int bS = mMainWindow->getImage() ? mMainWindow->getImageBorderSize() : 0;
    if( mMainWindow->getImage())
    {
        if( !isnormal(p2d.x) || !isnormal(p2d.y) || !isnormal(p2d.x) || !isnormal(p2d.y) )
            return true;
        if (isnan(p2d.x) || isnan(p2d.y) || isinf(p2d.x) || isinf(p2d.y))
            return true;
        return p2d.x < -bS || p2d.x > mMainWindow->getImage()->width()-bS || p2d.y < -bS || p2d.y > mMainWindow->getImage()->height()-bS;
    }else
    {
        return false;
    }
}


void ExtrCalibration::setXml(QDomElement &elem)
{
    reprojectionError.setXml(elem);
}

void ExtrCalibration::getXml(QDomElement &elem)
{
    QDomElement subElem;
    QString styleString;

    for(subElem = elem.firstChildElement(); !subElem.isNull(); subElem = subElem.nextSiblingElement())
    {
        if (subElem.tagName() == "REPROJECTION_ERROR")
        {
            reprojectionError.getXml(subElem);
        }
    }
}

void ReprojectionError::getXml(QDomElement &subElem)
{
    if (subElem.hasAttribute("SUM_PH"))
    {
        mPointHeightAvg = subElem.attribute("AVG_PH").toDouble();
    }
    if (subElem.hasAttribute("SD_PH"))
    {
        mPointHeightStdDev = subElem.attribute("SD_PH").toDouble();
        if(mPointHeightStdDev < 0)
        {
            mPointHeightVariance = -1;
        }else
        {
            mPointHeightVariance = pow(mPointHeightStdDev, 2);
        }
    }
    if (subElem.hasAttribute("MAX_PH"))
    {
        mPointHeightMax = subElem.attribute("MAX_PH").toDouble();
    }
    if (subElem.hasAttribute("SUM_DH"))
    {
        mDefaultHeightAvg = subElem.attribute("AVG_DH").toDouble();
    }
    if (subElem.hasAttribute("SD_DH"))
    {
        mDefaultHeightStdDev = subElem.attribute("SD_DH").toDouble();
        if(mDefaultHeightStdDev < 0){
            mDefaultHeightVariance = -1;
        }else{
            mDefaultHeightVariance = pow(mDefaultHeightStdDev, 2);
        }
    }
    if (subElem.hasAttribute("MAX_DH"))
    {
        mDefaultHeightMax = subElem.attribute("MAX_DH").toDouble();
    }
    if (subElem.hasAttribute("SUM_PX"))
    {
        mPixelAvg = subElem.attribute("AVG_PX").toDouble();
    }
    if (subElem.hasAttribute("SD_PX"))
    {
        mPixelStdDev = subElem.attribute("SD_PX").toDouble();
        if(mPixelStdDev < 0)
        {
            mPixelVariance = -1;
        }else
        {
            mPixelVariance = pow(mPixelStdDev, 2);
        }
    }
    if (subElem.hasAttribute("MAX_PX"))
    {
        mPixelMax = subElem.attribute("MAX_PX").toDouble();
    }
    if (subElem.hasAttribute("USED_HEIGHT"))
    {
        mUsedDefaultHeight = subElem.attribute("USED_HEIGHT").toDouble();
    }

    auto data = getData();
    mValid = !std::any_of(data.begin(), data.end(), [](double a){return !std::isfinite(a) || a < 0;});
}

void ReprojectionError::setXml(QDomElement &elem) const
{
    QDomElement subElem = elem.ownerDocument().createElement("REPROJECTION_ERROR");

    subElem.setAttribute("AVG_PH", mPointHeightAvg);
    subElem.setAttribute("SD_PH", mPointHeightStdDev);
    subElem.setAttribute("MAX_PH", mPointHeightMax);
    subElem.setAttribute("AVG_DH", mDefaultHeightAvg);
    subElem.setAttribute("SD_DH", mDefaultHeightStdDev);
    subElem.setAttribute("MAX_DH", mDefaultHeightMax);
    subElem.setAttribute("AVG_PX", mPixelAvg);
    subElem.setAttribute("SD_PX", mPixelStdDev);
    subElem.setAttribute("MAX_PX", mPixelMax);
    subElem.setAttribute("USED_HEIGHT", mUsedDefaultHeight);

    elem.appendChild(subElem);
}

double ReprojectionError::pointHeightAvg() const{
    return mPointHeightAvg;
}

double ReprojectionError::pointHeightStdDev() const{
    return mPointHeightStdDev;
}

double ReprojectionError::pointHeightVariance() const{
    return mPointHeightVariance;
}

double ReprojectionError::pointHeightMax() const{
    return mPointHeightMax;
}

double ReprojectionError::defaultHeightAvg() const{
    return mDefaultHeightAvg;
}

double ReprojectionError::defaultHeightStdDev() const{
    return mDefaultHeightStdDev;
}

double ReprojectionError::defaultHeightVariance() const{
    return mDefaultHeightVariance;
}

double ReprojectionError::defaultHeightMax() const{
    return mDefaultHeightMax;
}

double ReprojectionError::pixelAvg() const{
    return mPixelAvg;
}

double ReprojectionError::pixelStdDev() const{
    return mPixelStdDev;
}

double ReprojectionError::pixelVariance() const{
    return mPixelVariance;
}

double ReprojectionError::pixelMax() const{
    return mPixelMax;
}

double ReprojectionError::usedDefaultHeight() const{
    return mUsedDefaultHeight;
}
