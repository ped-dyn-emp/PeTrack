#include <QtWidgets>
#include <QFileDialog>
#include <QMessageBox>

#include "extrCalibration.h"

#include "petrack.h"
#include "control.h"

using namespace std;
using namespace cv;

#define MAX_AV_ERROR 20

ExtrCalibration::ExtrCalibration()
{
    mMainWindow = NULL;
    mControlWidget = NULL;
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
        //cout << mCalibFiles.first().toStdString() << endl; //toAscii() .data() Local8Bit().constData() << endl;
    }
    return false;

}
bool ExtrCalibration::loadExtrCalibFile(){

    bool all_ok = true;

    if( !mExtrCalibFile.isEmpty() )
    {
        if( mExtrCalibFile.right(4) == ".3dc" || mExtrCalibFile.right(4) == ".txt" )
        {
            QFile file(mExtrCalibFile);
            if( !file.open(QIODevice::ReadOnly | QIODevice::Text) )
            {
                QMessageBox::critical(mMainWindow, QObject::tr("Petrack"), QObject::tr("Error: Cannot open %1:\n%2.").arg(mExtrCalibFile).arg(file.errorString()));
                return false;
            }

            debout << "Reading 3D calibration data from " << mExtrCalibFile << "..." << endl;


            vector<Point3f> points3D_tmp;
            vector<Point2f> points2D_tmp;

            QTextStream in(&file);
            QString line;
            int line_counter = 0, counter;
            float x,y,z,px,py;
            float zahl;
            bool with_2D_data = false,
                 with_3D_data = false,
                 end_loop = false;

            while( 1 )
            {
                // Falls Datei am Ende Schleife beenden
                if( in.atEnd() )
                {
                    break;
                }

                // Neue Zeile einlesen
                line = in.readLine();
                ++line_counter;

                // Kommentare ueberlesen
                if( line.startsWith("#",Qt::CaseInsensitive) ||
                    line.startsWith(";;",Qt::CaseInsensitive) ||
                    line.startsWith("//",Qt::CaseInsensitive) ||
                    line.startsWith("!",Qt::CaseInsensitive) )
                    continue;

                // Test-Ausgabe
//                debout << "line: " << line << endl;

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
                        //debout << "### Error: counter=" << counter << endl;
                        end_loop = true;
                    }

                }
                if( counter == 1 )
                {
                    debout << "Optional number of points in line " << line_counter << " ignored." << endl;
                }else if( counter != 3 && counter != 5 )
                    debout << "Something wrong in line " << line_counter << "( " << line << " )! Ignored. (counter=" << counter << ")" << endl;

                // 3D daten abspeichern
                if( with_3D_data && (counter == 3 || counter == 5) )
                {
                    //debout << "x: " << x << " y: " << y << " z: " << z << endl;
                    points3D_tmp.push_back( Point3f( x, y, z ) );
                }
                // 2D daten abspeichern
                if( with_2D_data && counter == 5 )
                {
                    //debout << " px: " << px << " py: " << py << endl;
                    points2D_tmp.push_back( Point2f( px, py ) );
                }



            }
            // Check if there are more than 4 points for calibration in the file
             if( points3D_tmp.size() < 4 )
            {
                QMessageBox::critical(mMainWindow, QObject::tr("PeTrack"), QObject::tr("Error: Not enough points given: %1 (minimum 4 needed!). Please check your extrinsic calibration file!").arg(points3D_tmp.size()));
                all_ok = false;
            }

            // Check if 2D points delivered and if the number of 2D and 3D points agree
            else if( points2D_tmp.size() > 0 && points2D_tmp.size() != points3D_tmp.size() )
            {
                QMessageBox::critical(mMainWindow, QObject::tr("PeTrack"), QObject::tr("Error: Unsupported File Format in: %1 (number of 3D (%2) and 2D (%3) points disagree!)").arg(mExtrCalibFile).arg(points3D_tmp.size()).arg(points2D_tmp.size()));
                all_ok = false;
            }
            // Check if number of loaded 3D points agree with stored 2D points
            else if( !with_2D_data && points2D.size()>0 && points3D_tmp.size() != points2D.size() )
            {
                // ask if stored 2D points should be deleted?
                int result = QMessageBox::warning(mMainWindow, QObject::tr("PeTrack"), QObject::tr("Number of 3D points (%1) disagree with number of stored 2D points (%2)!<br />The 2D points will be deleted! You have to fetch new ones from the image!").arg(points3D_tmp.size()).arg(points2D.size()),QMessageBox::Ok, QMessageBox::Abort);
                if (result != QMessageBox::Ok)
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
            debout << "unsupported file extension (supported: .3dc,.txt)" << endl;
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
bool ExtrCalibration::fetch2DPoints()
{
    bool all_ok = true;
    int sz_2d = 0;
    if( !mMainWindow->getTracker() || mMainWindow->getTracker()->size() < 4 )
    {
       QMessageBox::critical(mMainWindow, QObject::tr("Petrack"), QObject::tr("Error: At minimum four 3D calibration points needed for 3D calibration."));
       all_ok = false;
    }else
    {
        sz_2d = mMainWindow->getTracker()->size();

        if( points3D.size()>0 && sz_2d != points3D.size() ){
            QMessageBox::critical(mMainWindow, QObject::tr("Petrack"), QObject::tr("Count of 2D-Points (%1) and 3D-Points (%2) disagree").arg(sz_2d).arg(points3D.size()));
            all_ok = false;

        }
        //debout << "Marked 2D-Image-Points: " << endl;
        if( all_ok )
        {
            points2D.clear();

            for(int i = 0; i < sz_2d; i++)
            {
                //debout << "[" << i << "]: (" << mMainWindow->getTracker()->at(i).at(0).x() << ", " << mMainWindow->getTracker()->at(i).at(0).y() << ")" << endl;
                // Info: Tracker->TrackPerson->TrackPoint->Vec2F
                points2D.push_back(Point2f(mMainWindow->getTracker()->at(i).at(0).x(),mMainWindow->getTracker()->at(i).at(0).y()));
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
bool ExtrCalibration::saveExtrCalibPoints()
{
    bool all_okay = false;

    QString out_str;
    QTextStream out(&out_str);

    int i;

    for (i = 0; i < points3D.size(); ++i)
    {
        out << "[" << QString::number(i+1,'i',0) << "]: "<< QString::number(points3D.at(i).x,'f',1) << " " << QString::number(points3D.at(i).y,'f',1) << " " << QString::number(points3D.at(i).z,'f',1) << " " << QString::number(points2D.at(i).x,'f',3) << " " << QString::number(points2D.at(i).y,'f',3) << endl;
    }
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setText("The corresponding calibration points have been changed.");
    msgBox.setInformativeText("Do you want to save your changes?");
    msgBox.setDetailedText(out_str);
    msgBox.setStandardButtons(QMessageBox::QMessageBox::Save | QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::Save);
    int ret = msgBox.exec();
    switch (ret) {
       case QMessageBox::Save:
        {
           // Save was clicked
            QFile file(mExtrCalibFile);

            if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
            {
              QMessageBox::critical(mMainWindow, QObject::tr("Petrack"), QObject::tr("Cannot open %1:\n%2.").arg(mExtrCalibFile).arg(file.errorString()));
              return false;
            }

            QTextStream file_out(&file);

            file_out << points3D.size() << endl;
            for (i = 0; i < points3D.size(); ++i)
            {
                file_out << points3D.at(i).x << " " << points3D.at(i).y << " " << points3D.at(i).z << " " << points2D.at(i).x << " " << points2D.at(i).y << endl;
            }
            all_okay = file.flush();
            file.close();
           break;
        }
       case QMessageBox::Discard:
           // Don't Save was clicked
           break;
       case QMessageBox::Cancel:
           // Cancel was clicked
           break;
       default:
           // should never be reached
           break;
    }

    return all_okay;
}

bool ExtrCalibration::isSetExtrCalib(){

    bool isSetExtrCalib = false;

    if( mControlWidget->getCalibExtrRot1() != 0.00 &&
        mControlWidget->getCalibExtrRot2() != 0.00 &&
        mControlWidget->getCalibExtrRot3() != 0.00 &&
        mControlWidget->getCalibExtrTrans1() != 0.00 &&
        mControlWidget->getCalibExtrTrans2() != 0.00 &&
        mControlWidget->getCalibExtrTrans3() != 0.00 )
    {
        isSetExtrCalib = true;
    }
    return true;//isSetExtrCalib;
}

void ExtrCalibration::calibExtrParams()
{

    if( !points3D.empty() && !points2D.empty() && points2D.size() == points3D.size() )
    {

        bool debug = false;
        int bS = mMainWindow->getImageBorderSize();
        /* Create Camera-Matrix form Camera-Params in the Petrack-GUI */
        Mat camMat = (Mat_<double>(3,3) <<
                      mControlWidget->fx->value(), 0, mControlWidget->cx->value()-bS,
                      0, mControlWidget->fy->value(), mControlWidget->cy->value()-bS,
                      0, 0, 1);

        /* Distortion Params must be 0 because only undistorted images are supported */
//        if( mControlWidget->r2->value() != 0 || mControlWidget->r4->value() != 0
//         || mControlWidget->tx->value() != 0 || mControlWidget->ty->value() != 0 )
//        {
//            QMessageBox::warning(mMainWindow, "Petrack",
//                                            "The distortion Parameters are set!\n"
//                                               "The 3D Camera-Calibration is implemented only for images without distortion.\n"
//                                               "There is no waranty of correct results!",
//                                            QMessageBox::Ok,QMessageBox::Ok);
//        }
        Mat distMat = (Mat_<double>(8,1) <<
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
        Mat op(points3D);
        Mat ip(points2D);

        //cout << "3D Punkte: " << endl << "  " << format(points3D,"csv") << endl;
        //cout << "2D Punkte: " << endl << "  " << format(points2D,"csv") << endl;

        /* Mat-objects for result rotation and translation vectors */
        Mat rvec(3,1,CV_64F),/*,0),*/ tvec(3,1,CV_64F);//,0);

        // Solve the PnP-Problem to calibrate the camera to its environment
#if ((CV_MAJOR_VERSION < 2) || ((CV_MAJOR_VERSION == 2) && (CV_MINOR_VERSION < 3)))
        bool solvePNPsuccess = true;
        solvePnP(op,ip,camMat,distMat,rvec,tvec,false);
#else
        bool solvePNPsuccess = solvePnP(op,ip,camMat,distMat,rvec,tvec,false,SOLVEPNP_ITERATIVE);
        //bool solvePNPsuccess = solvePnP(op,ip,camMat,distMat,rvec,tvec,false,SOLVEPNP_P3P); // Requires exactly 4 points
        //bool solvePNPsuccess = solvePnP(op,ip,camMat,distMat,rvec,tvec,false,SOLVEPNP_EPNP);
        //bool solvePNPsuccess = solvePnP(op,ip,camMat,distMat,rvec,tvec,false,SOLVEPNP_DLS);
        //bool solvePNPsuccess = solvePnP(op,ip,camMat,distMat,rvec,tvec,false,SOLVEPNP_UPNP);
        //bool solvePNPsuccess = true;
        //solvePnPRansac(op,ip,camMat,distMat,rvec,tvec);
#endif


//        debout << "The solvePNP-Method " << (solvePNPsuccess ? "" : "doesn't ") << "worked. " << solvePNPsuccess << endl;

//        void decomposeProjectionMatrix(InputArray projMatrix,
//                                       OutputArray cameraMatrix,
//                                       OutputArray rotMatrix,
//                                       OutputArray transVect,
//                                       OutputArray rotMatrixX=noArray(),
//                                       OutputArray rotMatrixY=noArray(),
//                                       OutputArray rotMatrixZ=noArray(),
//                                       OutputArray eulerAngles=noArray() );

//        debout << "Rotation:" << endl;
//        debout << rvec.at<double>(0,0) << ", " << rvec.at<double>(1,0) << ", " << rvec.at<double>(2,0) << endl;
//        debout << rvec.at<double>(0,0) << ", " << rvec.at<double>(0,1) << ", " << rvec.at<double>(0,2) << endl;
//        debout << "Translation:" << endl;
//        debout << tvec.at<double>(0,0) << ", " << tvec.at<double>(1,0) << ", " << tvec.at<double>(2,0) << endl;
//        debout << tvec.at<double>(0,0) << ", " << tvec.at<double>(0,1) << ", " << tvec.at<double>(0,2) << endl;

        Mat rot_mat(3,3,CV_64F);//, 0);
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

        debout << "-.- ESTIMATED ROTATION -.-" << endl;
        for ( size_t p=0; p<3; p++ )
            debout << rotation_matrix[p*3] << " , " << rotation_matrix[p*3+1] << " , " << rotation_matrix[p*3+2] << endl;

        debout << "-.- ESTIMATED TRANSLATION -.-" << endl;
        debout << translation_vector[0] << " , " << translation_vector[1] << " , " << translation_vector[2] << endl;

        debout << "-.- Translation vector -.-" << endl;
        debout << translation_vector2[0] << " , " << translation_vector2[1] << " , " << translation_vector2[2] << endl;

        debout << "-.- Rotation vector -.-" << endl;
        debout << rvec.at<double>(0,0) << " , " << rvec.at<double>(1,0) << " , " << rvec.at<double>(2,0) << endl;

        camHeight = translation_vector2[2] < 0 ? -translation_vector2[2] : translation_vector2[2];

        mControlWidget->setCalibExtrRot1(rvec.at<double>(0,0));
        mControlWidget->setCalibExtrRot2(rvec.at<double>(1,0));
        mControlWidget->setCalibExtrRot3(rvec.at<double>(2,0));

        mControlWidget->setCalibExtrTrans1(translation_vector2[0]);
        mControlWidget->setCalibExtrTrans2(translation_vector2[1]);
        mControlWidget->setCalibExtrTrans3(translation_vector2[2]);

//        mControlWidget->setCalibCoord3DTransX(0);
//        mControlWidget->setCalibCoord3DTransY(0);
//        mControlWidget->setCalibCoord3DTransZ(0);

        if ( !calcReprojectionError() )
        {
            cout << "# Warning: extrinsic calibration not possible! Please select other 2D/3D points!" << endl;

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

            reprojectionError.clear();

            QMessageBox::critical(mMainWindow, QObject::tr("Petrack"), QObject::tr("Error: Could not calculate extrinsic calibration. Please select other 2D/3D point correspondences for extrinsic calibration!"));

            isExtCalib = false;

            return;
        }

        isExtCalib = true;

        cout << "End of extern calibration!" << endl;
    }else
    {
        cerr << "# Warning: invalid point correspondences for camera calibration." << endl;
        cerr << "# 2D points:" << points2D.size() << ", 3D points: " << points3D.size() << endl;
    }
    mMainWindow->getScene()->update();

}

bool ExtrCalibration::calcReprojectionError()
{
    //////
    /// \brief error measurements
    ///
    float val, max_px = -1.0, max_pH = -1.0, max_dH = -1.0,
          var_px = 0, sd_px = 0, var_pH = 0, sd_pH = 0, var_dH = 0, sd_dH = 0,
          sum_px = 0, sum_pH = 0, sum_dH = 0;

    //int bS = mMainWindow->getImageBorderSize();
    int num_points = get2DList().size();

    bool debug = false;
    for(int i=0; i< num_points; i++)
    {
        Point2f p2 = get2DList().at(i);
        Point3f p3d = get3DList().at(i);
        p3d.x -= mControlWidget->getCalibCoord3DTransX();
        p3d.y -= mControlWidget->getCalibCoord3DTransY();
        p3d.z -= mControlWidget->getCalibCoord3DTransZ();
        Point2f p3 = getImagePoint(p3d);

        // Error measurements metric (cm)
        //debout << "Point-Height: " << endl;
        Point3f p2d = get3DPoint(Point2f(p2.x/*+bS*/,p2.y/*+bS*/),p3d.z);
        //debout << p2d.x << " " << p2d.y << " " << p2d.z << endl;
        //debout << p3d.x << " " <<p3d.y << " " <<p3d.z  << endl;

        //debout << "Default-Height: " << endl;
        Point3f p2d_mapDefaultHeight = get3DPoint(p2,mControlWidget->mapDefaultHeight->value()); // mStatusPosRealHeight->value()); ?
        //debout << p2d_mapDefaultHeight.x << " " << p2d_mapDefaultHeight.y << " " << p2d_mapDefaultHeight.z << endl;

        Point3f p3d_mapDefaultHeight = get3DPoint(Point2f(p3.x,p3.y)/*getImagePoint(p2d_mapDefaultHeight)*/,mControlWidget->mapDefaultHeight->value());
        //debout << p3d_mapDefaultHeight.x << " " << p3d_mapDefaultHeight.y << " " << p3d_mapDefaultHeight.z << endl;

        val = sqrt(pow(p3d.x-p2d.x,2) + pow(p3d.y-p2d.y,2));
        if ( val > max_pH ) max_pH = val;
            sum_pH += val;
        if( debug ) debout << "Error point[" << i << "]: " << val << endl;

        val = sqrt(pow(p3d_mapDefaultHeight.x-p2d_mapDefaultHeight.x,2) + pow(p3d_mapDefaultHeight.y-p2d_mapDefaultHeight.y,2));
        if ( val > max_dH ) max_dH = val;
            sum_dH += val;
        if( debug ) debout << "Error point[" << i << "]: " << val << endl;

        // Error measurements pixel
        val = sqrt(pow(p3.x-p2.x,2) + pow(p3.y-p2.y,2));
        // Maximum
        if ( val > max_px ) max_px = val;
            sum_px += val;
        if( debug ) debout << "Error point[" << i << "]: " << val << endl;

    }
    for(int i=0; i< num_points; i++)
    {
        Point2f p2 = get2DList().at(i);
        Point3f p3d = get3DList().at(i);
        p3d.x -= mControlWidget->getCalibCoord3DTransX();
        p3d.y -= mControlWidget->getCalibCoord3DTransY();
        p3d.z -= mControlWidget->getCalibCoord3DTransZ();
        Point2f p3 = getImagePoint(p3d);

        // Error measurements metric (cm)
        //debout << "Point-Height: " << endl;
        Point3f p2d = get3DPoint(Point2f(p2.x/*+bS*/,p2.y/*+bS*/),p3d.z);
        //debout << p2d.x << " " << p2d.y << " " << p2d.z << endl;
        //debout << p3d.x << " " <<p3d.y << " " <<p3d.z  << endl;

        //debout << "Default-Height: " << endl;
        Point3f p2d_mapDefaultHeight = get3DPoint(p2,mControlWidget->mapDefaultHeight->value()); // mStatusPosRealHeight->value()); ?
        //debout << p2d_mapDefaultHeight.x << " " << p2d_mapDefaultHeight.y << " " << p2d_mapDefaultHeight.z << endl;

        Point3f p3d_mapDefaultHeight = get3DPoint(Point2f(p3.x,p3.y)/*getImagePoint(p2d_mapDefaultHeight)*/,mControlWidget->mapDefaultHeight->value());
        //debout << p3d_mapDefaultHeight.x << " " << p3d_mapDefaultHeight.y << " " << p3d_mapDefaultHeight.z << endl;

        val = pow(sqrt(pow(p3d.x-p2d.x,2) + pow(p3d.y-p2d.y,2))-(sum_pH/num_points),2);
        var_pH += val;

        val = pow(sqrt(pow(p3d_mapDefaultHeight.x-p2d_mapDefaultHeight.x,2) + pow(p3d_mapDefaultHeight.y-p2d_mapDefaultHeight.y,2))-(sum_dH/num_points),2);
        var_dH += val;

        val = pow(sqrt(pow(p3.x-p2.x,2) + pow(p3.y-p2.y,2))-(sum_px/num_points),2);
        var_px += val;

    }

    if( reprojectionError.isEmpty() )
        reprojectionError = QVector<double>(13);

    // average
    sum_pH /= num_points;
    var_pH /= num_points;
    sd_pH = sqrt(var_pH);
    debout << "Reprojection error (pointHeight) average: " << sum_pH << "cm (standard deviation: " << sd_pH << " variance: " << var_pH << " Max error: " << max_pH << "cm)" << endl;
    reprojectionError[0] = sum_pH;
    reprojectionError[1] = sd_pH;
    reprojectionError[2] = var_pH;
    reprojectionError[3] = max_pH;

    // average
    sum_dH /= num_points;
    var_dH /= num_points;
    sd_dH = sqrt(var_dH);
    debout << "Reprojection error (defaultHeight=" << mControlWidget->mapDefaultHeight->value() << ") average: " << sum_dH << "cm (standard deviation: " << sd_dH << " variance: " << var_dH << " Max error: " << max_dH << "cm)" << endl;
    reprojectionError[4] = sum_dH;
    reprojectionError[5] = sd_dH;
    reprojectionError[6] = var_dH;
    reprojectionError[7] = max_dH;

    // average
    sum_px /= num_points;
    var_px /= num_points;
    sd_px = sqrt(var_px);
    debout << "Reprojection error (Pixel) average: " << sum_px << "px (standard deviation: " << sd_px << " variance: " << var_px << " Max error: " << max_px << "px)" << endl;
    reprojectionError[8] = sum_px;
    reprojectionError[9] = sd_px;
    reprojectionError[10] = var_px;
    reprojectionError[11] = max_px;

    // default height
    reprojectionError[12] = mControlWidget->mapDefaultHeight->value();

    return reprojectionError[0] > MAX_AV_ERROR ? false : true; // Falls pixel fehler im schnitt > 20 ist das Ergebnis nicht akzeptabel
}


Point2f ExtrCalibration::getImagePoint(Point3f p3d)
{
    bool debug = false;

    p3d.x *= mControlWidget->getCalibCoord3DSwapX() ? -1 : 1;
    p3d.y *= mControlWidget->getCalibCoord3DSwapY() ? -1 : 1;
    p3d.z *= mControlWidget->getCalibCoord3DSwapZ() ? -1 : 1;

    // Adding the coordsystem translation from petrack window
    p3d.x += mControlWidget->getCalibCoord3DTransX();
    p3d.y += mControlWidget->getCalibCoord3DTransY();
    p3d.z += mControlWidget->getCalibCoord3DTransZ();

    if( debug ) cout << "getImagePoint: Start Point3D: (" << p3d.x << ", " << p3d.y << ", " << p3d.z << ")" << endl;
    // ToDo: use projectPoints();
    int bS = mMainWindow->getImage() ? mMainWindow->getImageBorderSize() : 0;

    double rvec_array[3], translation_vector[3];

    rvec_array[0] = mControlWidget->getCalibExtrRot1();
    rvec_array[1] = mControlWidget->getCalibExtrRot2();
    rvec_array[2] = mControlWidget->getCalibExtrRot3();

    Mat rvec(3,1,CV_64F, rvec_array), rot_inv;
    Mat rot_mat(3,3,CV_64F), e(3,3,CV_64F);
    // Transform the rotation vector into a rotation matrix with opencvs rodrigues method
    Rodrigues(rvec,rot_mat);

    // use inverse Matrix to get translation_vector?
    rot_inv = rot_mat.inv(DECOMP_SVD);

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
    cout << "\n-.- ESTIMATED ROTATION\n";
    for ( size_t p=0; p<3; p++ )
        printf("%20.18f, %20.18f, %20.18f\n",rot_mat.at<double>(p,0),rot_mat.at<double>(p,1),rot_mat.at<double>(p,2));
        //cout << rot_mat.at<double>(p,0) << " , " << rot_mat.at<double>(p,1) << " , " << rot_mat.at<double>(p,2) << "\n";

    cout << "\n-.- ESTIMATED ROTATION^-1\n";
    for ( size_t p=0; p<3; p++ )
        printf("%20.18f, %20.18f, %20.18f\n",rot_inv.at<double>(p,0),rot_inv.at<double>(p,1),rot_inv.at<double>(p,2));
        //cout << rot_inv.at<double>(p,0) << " , " << rot_inv.at<double>(p,1) << " , " << rot_inv.at<double>(p,2) << "\n";

    cout << "\n-.- ESTIMATED R^-1*R\n";
    for ( size_t p=0; p<3; p++ )
        printf("%20.18f, %20.18f, %20.18f\n",e.at<double>(p,0),e.at<double>(p,1),e.at<double>(p,2));
        //cout << e.at<double>(p,0) << " , " << e.at<double>(p,1) << " , " << e.at<double>(p,2) << "\n";

    cout << "\n-.- ESTIMATED TRANSLATION\n";
    printf("%20.15f, %20.15f, %20.15f\n",translation_vector[0],translation_vector[1],translation_vector[2]);
    //cout << translation_vector[0] << " , " << translation_vector[1] << " , " << translation_vector[2] << "\n";

    //cout << this->translation_vector[0] << " = " << translation_vector[0] << endl;
    //cout << this->translation_vector[1] << " = " << translation_vector[1] << endl;
    //cout << this->translation_vector[2] << " = " << translation_vector[2] << endl;
    }
    Point3f point3D;

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

    if( debug ) cout << "###### After extern calibration: (" << point3D.x << ", " << point3D.y << ", " << point3D.z << ")" << endl;

    Point2f point2D = Point2f( 0.0, 0.0 );
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
  * Methode get3DPoint
  * Wandelt einen 2D-Punkt in einen 3D Punkt um
  * Dazu muss der 2D-Pixelpunkt sowie der Abstand
  * des 2D-Punktes zur xy-Ebene angegeben werden (i.d.R. die Hoehe/Personengroesse)
  *
  * Input: p2d = 2D Pixelpunkt (Uebergabe ohne Border)
  *        h = Abstand des Punktes zur xy-Ebene
  *
  * Output: Point3f resultPoint gibt den berechneten 3D-Punkt zurueck
  *
  */

Point3f ExtrCalibration::get3DPoint(Point2f p2d, double h)
{
        bool debug = false;
//        bool debug = true;

        if( debug ) cout << "get3DPoint: Start Point2D: (" << p2d.x << ", " << p2d.y << ") h: " << h << endl;

        int bS = mMainWindow->getImage() ? mMainWindow->getImageBorderSize() : 0;

        if (false && bS > 0)
        {
            p2d.x += bS;
            p2d.y += bS;
        }

        // Ergebnis 3D-Punkt
        Point3f resultPoint, tmpPoint;

        bool newMethod = true;
        /////////////// Start new method
        if( newMethod )
        {
        double rvec_array[3], translation_vector[3];
        rvec_array[0] = mControlWidget->getCalibExtrRot1();
        rvec_array[1] = mControlWidget->getCalibExtrRot2();
        rvec_array[2] = mControlWidget->getCalibExtrRot3();

        Mat rvec(3,1,CV_64F, rvec_array), rot_inv;
        Mat rot_mat(3,3,CV_64F), e(3,3,CV_64F);
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
        rot_inv = rot_mat.inv(DECOMP_LU);
        e = rot_inv*rot_mat;

        if( debug )
        {
        debout << "\n-.- ESTIMATED ROTATION\n";
        for ( size_t p=0; p<3; p++ )
            printf("%20.18f, %20.18f, %20.18f\n",rot_mat.at<double>(p,0),rot_mat.at<double>(p,1),rot_mat.at<double>(p,2));
            //cout << rot_mat.at<double>(p,0) << " , " << rot_mat.at<double>(p,1) << " , " << rot_mat.at<double>(p,2) << "\n";

        debout << "\n-.- ESTIMATED ROTATION^-1\n";
        for ( size_t p=0; p<3; p++ )
            printf("%20.18f, %20.18f, %20.18f\n",rot_inv.at<double>(p,0),rot_inv.at<double>(p,1),rot_inv.at<double>(p,2));
            //cout << rot_inv.at<double>(p,0) << " , " << rot_inv.at<double>(p,1) << " , " << rot_inv.at<double>(p,2) << "\n";

        debout << "\n-.- ESTIMATED R^-1*R\n";
        for ( size_t p=0; p<3; p++ )
            printf("%20.18f, %20.18f, %20.18f\n",e.at<double>(p,0),e.at<double>(p,1),e.at<double>(p,2));
            //cout << e.at<double>(p,0) << " , " << e.at<double>(p,1) << " , " << e.at<double>(p,2) << "\n";

        debout << "\n-.- ESTIMATED TRANSLATION\n";
        debout << mControlWidget->getCalibExtrTrans1() << " , " << mControlWidget->getCalibExtrTrans2() << " , " << mControlWidget->getCalibExtrTrans3() << "\n";
        debout << translation_vector[0] << " , " << translation_vector[1] << " , " << translation_vector[2] << "\n";

        debout << "Det(rot_mat): "<< determinant(rot_mat) << endl;
        debout << "Det(rot_inv): "<< determinant(rot_inv) << endl;
        }
        double z = h + rot_inv.at<double>(2,0)*translation_vector[0] +
                       rot_inv.at<double>(2,1)*translation_vector[1] +
                       rot_inv.at<double>(2,2)*translation_vector[2];
        if( debug )
        {
        debout << "##### z: " << h << " + " << rot_inv.at<double>(2,0) << "*" << translation_vector[0] << " + "
                                          << rot_inv.at<double>(2,1) << "*" << translation_vector[1] << " + "
                                          << rot_inv.at<double>(2,2) << "*" << translation_vector[2] << " = " << z << endl;
        }
        z /= (rot_inv.at<double>(2,0)*(p2d.x-/*bS)-(*/(mControlWidget->getCalibCxValue()-bS)/*-bS*/)/mControlWidget->getCalibFxValue() +
                 rot_inv.at<double>(2,1)*(p2d.y-/*bS)-(*/(mControlWidget->getCalibCyValue()-bS)/*-bS*/)/mControlWidget->getCalibFyValue() +
                 rot_inv.at<double>(2,2));
        if( debug ) cout << "###### z: "<< z << endl;

        resultPoint.x = (p2d.x-/*bS)-(*/(mControlWidget->getCalibCxValue()-bS)/*-bS*/);
        resultPoint.y = (p2d.y-/*bS)-(*/(mControlWidget->getCalibCyValue()-bS)/*-bS*/);
        resultPoint.z = z;

        if( debug ) cout << "###### (" << resultPoint.x << ", " << resultPoint.y << ", " << resultPoint.z << ")" << endl;

        resultPoint.x = resultPoint.x * z/mControlWidget->getCalibFxValue();
        resultPoint.y = resultPoint.y * z/mControlWidget->getCalibFyValue();

        if( debug ) cout << "###### After intern re-calibration: (" << resultPoint.x << ", " << resultPoint.y << ", " << resultPoint.z << ")" << endl;

        tmpPoint.x = resultPoint.x - translation_vector[0];
        tmpPoint.y = resultPoint.y - translation_vector[1];
        tmpPoint.z = resultPoint.z - translation_vector[2];

        if( debug ) cout << "###### After translation: (" << tmpPoint.x << ", " << tmpPoint.y << ", " << tmpPoint.z << ")" << endl;

        resultPoint.x = rot_inv.at<double>(0,0)*(tmpPoint.x)+
                        rot_inv.at<double>(0,1)*(tmpPoint.y)+
                        rot_inv.at<double>(0,2)*(tmpPoint.z);
        resultPoint.y = rot_inv.at<double>(1,0)*(tmpPoint.x)+
                        rot_inv.at<double>(1,1)*(tmpPoint.y)+
                        rot_inv.at<double>(1,2)*(tmpPoint.z);
        resultPoint.z = rot_inv.at<double>(2,0)*(tmpPoint.x)+
                        rot_inv.at<double>(2,1)*(tmpPoint.y)+
                        rot_inv.at<double>(2,2)*(tmpPoint.z);

        if( debug ) cout << "#resultPoint: (" << resultPoint.x << ", " << resultPoint.y << ", " << resultPoint.z << ")" << endl;
        if( debug ) cout << "Coord Translation: x: " << mControlWidget->getCalibCoord3DTransX() << ", y: " << mControlWidget->getCalibCoord3DTransY() << ", z: " << mControlWidget->getCalibCoord3DTransZ() << endl;


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

        Point3f camInWorld = transformRT(Point3f(0,0,0));

        // 3D-Punkt vor der Kamera mit Tiefe 5
        CvPoint3D32f pointBeforeCam;
        pointBeforeCam.x = (p2d.x - mControlWidget->cx->value()) / mControlWidget->fx->value() * 50;
        pointBeforeCam.y = (p2d.y - mControlWidget->cy->value()) / mControlWidget->fy->value() * 50;
        pointBeforeCam.z = 50;
        if( debug ) cout << "Point before Camera: [" << pointBeforeCam.x << ", " << pointBeforeCam.y << ", " << pointBeforeCam.z << "]" << endl;
        // 3D-Punkt vor Kamera in Weltkoordinaten
        Point3f pBCInWorld = transformRT(pointBeforeCam);
        if( debug ) cout << "Point before Camera in World-Coordinatesystem: [" << pBCInWorld.x << ", " << pBCInWorld.y << ", " << pBCInWorld.z << "]" << endl;
        if( debug ) cout << "Camera in World-Coordinatesystem: [" << camInWorld.x << ", " << camInWorld.y << ", " << camInWorld.z << "]" << endl;
        // Berechnung des Richtungsvektors der Gerade von der Kamera durch den Pixel
        // Als Sttzvektor der Geraden wird die Position der Kamera gewhlt
        pBCInWorld.x -= camInWorld.x;
        pBCInWorld.y -= camInWorld.y;
        pBCInWorld.z -= camInWorld.z;
        if( debug ) cout << "G:x = (" << camInWorld.x << " / " << camInWorld.y << " / " << camInWorld.z << ") + lambda (" << pBCInWorld.x << " / " << pBCInWorld.y << " / " << pBCInWorld.z << ")" << endl;

        // Berechnung des Schnittpunktes: Hier lambda von der Geraden
        double lambda = (h -camInWorld.z) / (pBCInWorld.z);
        if( debug ) cout << "Lambda: " << lambda << endl;

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
Point3f ExtrCalibration::transformRT(Point3f p)
{
    // ToDo: use projectPoints();

    double rvec_array[3], rotation_matrix[9];
    rvec_array[0] = mControlWidget->getCalibExtrRot1();
    rvec_array[1] = mControlWidget->getCalibExtrRot2();
    rvec_array[2] = mControlWidget->getCalibExtrRot3();

    Mat rvec(3,1,CV_64F, rvec_array);
    Mat rot_mat(3,3,CV_64F);
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

    Point3f point3D;

    point3D.x = rotation_matrix[0] * p.x +
            rotation_matrix[3] * p.y +
            rotation_matrix[6] * p.z -
            mControlWidget->trans1->value();//translation_vector2[0];
    point3D.y = rotation_matrix[1] * p.x +
            rotation_matrix[4] * p.y +
            rotation_matrix[7] * p.z -
            mControlWidget->trans2->value();//translation_vector2[1];
    point3D.z = rotation_matrix[2] * p.x +
            rotation_matrix[5] * p.y +
            rotation_matrix[8] * p.z -
            mControlWidget->trans3->value();//translation_vector2[2];
    return point3D;
}
bool ExtrCalibration::isOutsideImage(Point2f p2d)
{
    int bS = mMainWindow->getImage() ? mMainWindow->getImageBorderSize() : 0;
    if( mMainWindow->getImage())
    {
        if( !isnormal(p2d.x) || !isnormal(p2d.y) || !isnormal(p2d.x) || !isnormal(p2d.y) )
            return true;
        if (isnan(p2d.x) || isnan(p2d.y) || isinf(p2d.x) || isinf(p2d.y))
            return true;
        //return p2d.x < 0 || p2d.x > mMainWindow->getImage()->width() || p2d.y < 0 || p2d.y > mMainWindow->getImage()->height();
        return p2d.x < -bS || p2d.x > mMainWindow->getImage()->width()-bS || p2d.y < -bS || p2d.y > mMainWindow->getImage()->height()-bS;
    }else
    {
        return false;
    }
}
