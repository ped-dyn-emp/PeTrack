/*
 * PeTrack - Software for tracking pedestrians movement in videos
 * Copyright (C) 2024 Forschungszentrum Jülich GmbH, IAS-7
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

#include "extrCalibration.h"

#include "control.h"
#include "logger.h"
#include "pMessageBox.h"
#include "petrack.h"

#include <QFileDialog>
#include <QtWidgets>

#define MAX_AV_ERROR 20

ExtrCalibration::ExtrCalibration(PersonStorage &storage) : mPersonStorage(storage)
{
    mMainWindow    = nullptr;
    mControlWidget = nullptr;
}

ExtrCalibration::~ExtrCalibration() {}

void ExtrCalibration::setMainWindow(Petrack *mw)
{
    mMainWindow    = mw;
    mControlWidget = mw->getControlWidget();
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
    if(!this->isEmptyExtrCalibFile())
    {
        return mExtrCalibFile;
    }
    else
    {
        return QString();
    }
}

std::optional<ExtrinsicParameters> ExtrCalibration::openExtrCalibFile()
{
    if(mMainWindow)
    {
        static QString lastDir;

        if(!mExtrCalibFile.isEmpty())
        {
            lastDir = QFileInfo(mExtrCalibFile).path();
        }

        QString extrCalibFile = QFileDialog::getOpenFileName(
            mMainWindow,
            Petrack::tr("Open extrinisc calibration file with point correspondences"),
            lastDir,
            "3D-Calibration-File (*.3dc);;Text (*.txt);;All supported types (*.3dc *.txt);;All files (*.*)");
        if(!extrCalibFile.isEmpty())
        {
            mExtrCalibFile = extrCalibFile;
            return loadExtrCalibFile();
        }
    }
    return std::nullopt;
}

// following function copied from OpenCV
static bool isPlanarObjectPoints(cv::InputArray _objectPoints, double threshold = 1e-3)
{
    CV_CheckType(
        _objectPoints.type(),
        _objectPoints.type() == CV_32FC3 || _objectPoints.type() == CV_64FC3,
        "Type of _objectPoints must be CV_32FC3 or CV_64FC3");
    cv::Mat objectPoints;
    if(_objectPoints.type() == CV_32FC3)
    {
        _objectPoints.getMat().convertTo(objectPoints, CV_64F);
    }
    else
    {
        objectPoints = _objectPoints.getMat();
    }

    cv::Scalar meanValues          = mean(objectPoints);
    int        nbPts               = objectPoints.checkVector(3, CV_64F);
    cv::Mat    objectPointsCentred = objectPoints - meanValues;
    objectPointsCentred            = objectPointsCentred.reshape(1, nbPts);

    cv::Mat w, u, vt;
    cv::Mat MM = objectPointsCentred.t() * objectPointsCentred;
    SVDecomp(MM, w, u, vt);

    return (w.at<double>(2) < w.at<double>(1) * threshold);
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
std::optional<ExtrinsicParameters> ExtrCalibration::loadExtrCalibFile()
{
    if(mExtrCalibFile.isEmpty())
    {
        return std::nullopt;
    }

    if(!mExtrCalibFile.endsWith(".3dc", Qt::CaseInsensitive) && !mExtrCalibFile.endsWith(".txt", Qt::CaseInsensitive))
    {
        PWarning(nullptr, "Unsupported File Type", "Unsupported file extension (supported: .3dc, .txt)");
        return std::nullopt;
    }

    QFile file(mExtrCalibFile);
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        PCritical(
            mMainWindow,
            QObject::tr("Petrack"),
            QObject::tr("Error: Cannot open %1:\n%2.").arg(mExtrCalibFile, file.errorString()));
        return std::nullopt;
    }

    SPDLOG_INFO("Reading 3D calibration data from {} ...", mExtrCalibFile);

    std::vector<cv::Point3f> points3D_tmp;
    std::vector<cv::Point2f> points2D_tmp;

    QTextStream in(&file);
    QString     line;
    int         line_counter = 0, counter;
    float       x, y, z, px, py;
    float       zahl;
    bool        with_2D_data = false, with_3D_data = false, end_loop = false;

    // Exit loop when reaching the end of the file
    while(!in.atEnd())
    {
        // Neue Zeile einlesen
        line = in.readLine();
        ++line_counter;

        // Kommentare ueberlesen
        if(line.startsWith("#", Qt::CaseInsensitive) || line.startsWith(";;", Qt::CaseInsensitive) ||
           line.startsWith("//", Qt::CaseInsensitive) || line.startsWith("!", Qt::CaseInsensitive))
        {
            continue;
        }

        QTextStream stream(&line);
        counter  = 0;
        end_loop = false;

        while(!stream.atEnd() && !end_loop)
        {
            stream >> zahl;
            ++counter;

            switch(counter)
            {
                case 1:
                    x = zahl;
                    if(!with_3D_data)
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
                    if(!with_2D_data)
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
        if(counter == 1)
        {
            SPDLOG_INFO("Optional number of points in line {} ignored.", line_counter);
        }
        else if(counter != 3 && counter != 5)
        {
            SPDLOG_INFO("Something wrong in line {} ({})! Ignored. (counter={})", line_counter, line, counter);
        }

        // 3D daten abspeichern
        if(with_3D_data && (counter == 3 || counter == 5))
        {
            points3D_tmp.push_back(cv::Point3f(x, y, z));
        }
        // 2D daten abspeichern
        if(with_2D_data && counter == 5)
        {
            points2D_tmp.push_back(cv::Point2f(px, py));
        }
    }
    // Check if there are more than 4 points for calibration in the file
    if(points3D_tmp.size() < 4)
    {
        PCritical(
            mMainWindow,
            QObject::tr("PeTrack"),
            QObject::tr("Error: Not enough points given: %1 (minimum 4 (coplanar) or 6 (not coplanar) "
                        "needed!). Please check your extrinsic "
                        "calibration file!")
                .arg(points3D_tmp.size()));
        return std::nullopt;
    }

    // Non-planar points use DLT - we need at least 6 points; not only 4
    if(!isPlanarObjectPoints(points3D_tmp) && points3D_tmp.size() < 6)
    {
        PCritical(
            mMainWindow,
            QObject::tr("PeTrack"),
            QObject::tr("Error: Not enough points given: %1 (minimum 4 (coplanar) or 6 (not coplanar) "
                        "needed!). Please check your extrinsic "
                        "calibration file!")
                .arg(points3D_tmp.size()));
        return std::nullopt;
    }

    // Check if 2D points delivered and if the number of 2D and 3D points agree
    if(points2D_tmp.size() > 0 && points2D_tmp.size() != points3D_tmp.size())
    {
        PCritical(
            mMainWindow,
            QObject::tr("PeTrack"),
            QObject::tr("Error: Unsupported File Format in: %1 (number of 3D (%2) and 2D (%3) points disagree!)")
                .arg(mExtrCalibFile)
                .arg(points3D_tmp.size())
                .arg(points2D_tmp.size()));
        return std::nullopt;
    }

    // Check if number of loaded 3D points agree with stored 2D points
    if(!with_2D_data && points2D.size() > 0 && points3D_tmp.size() != points2D.size())
    {
        // ask if stored 2D points should be deleted?
        int result = PWarning(
            mMainWindow,
            QObject::tr("PeTrack"),
            QObject::tr("Number of 3D points (%1) disagree with number of stored 2D points (%2)!<br />The 2D "
                        "points will be deleted! You have to fetch new ones from the image!")
                .arg(points3D_tmp.size())
                .arg(points2D.size()),
            PMessageBox::StandardButton::Ok | PMessageBox::StandardButton::Abort);
        if(result != PMessageBox::StandardButton::Ok)
        {
            return std::nullopt;
        }
        else
        {
            points2D.clear();
        }
    }

    if(with_3D_data)
    {
        points3D = points3D_tmp;
    }
    if(with_2D_data)
    {
        points2D = points2D_tmp;
    }

    if(!mMainWindow->isLoading())
    {
        return calibExtrParams();
    }
    return std::nullopt;
}

/**
 * @brief Uses manually set TrackPoints as 2D points for extrinsic calibration
 *
 * @pre loaded at least 4 3D-points
 *
 * @return true if calibration did take place
 */
std::optional<ExtrinsicParameters> ExtrCalibration::fetch2DPoints()
{
    if(!mMainWindow->getTracker() || mPersonStorage.nbPersons() < 4)
    {
        PCritical(
            mMainWindow,
            QObject::tr("Petrack"),
            QObject::tr("Error: At minimum four 3D calibration points needed for 3D calibration."));
        return std::nullopt;
    }

    size_t sz_2d = mPersonStorage.nbPersons();

    if(points3D.size() > 0 && sz_2d != points3D.size())
    {
        PCritical(
            mMainWindow,
            QObject::tr("Petrack"),
            QObject::tr("Count of 2D-Points (%1) and 3D-Points (%2) disagree").arg(sz_2d).arg(points3D.size()));
        return std::nullopt;
    }

    points2D.clear();
    for(int i = 0; i < static_cast<int>(sz_2d); i++)
    {
        //  Info: Tracker->TrackPerson->TrackPoint->Vec2F
        points2D.push_back(cv::Point2f(mPersonStorage.at(i).at(0).x(), mPersonStorage.at(i).at(0).y()));
    }

    mPersonStorage.clear();
    return calibExtrParams();
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

    QString     out_str;
    QTextStream out(&out_str);


    for(size_t i = 0; i < points3D.size(); ++i)
    {
        out << "[" << QString::number(i + 1, 'i', 0) << "]: " << QString::number(points3D.at(i).x, 'f', 1) << " "
            << QString::number(points3D.at(i).y, 'f', 1) << " " << QString::number(points3D.at(i).z, 'f', 1) << " "
            << QString::number(points2D.at(i).x, 'f', 3) << " " << QString::number(points2D.at(i).y, 'f', 3)
            << Qt::endl;
    }

    QIcon       icon = QApplication::style()->standardIcon(QStyle::SP_MessageBoxWarning);
    PMessageBox msgBox{
        nullptr,
        "PeTrack",
        "The corresponding calibration points have been changed.\n"
        "Do you want to save your changes?",
        QIcon(),
        out_str,
        PMessageBox::StandardButton::Save | PMessageBox::StandardButton::Cancel,
        PMessageBox::StandardButton::Save};
    int ret = msgBox.exec();
    switch(ret)
    {
        case PMessageBox::StandardButton::Save:
        {
            // Save was clicked
            QFile file(mExtrCalibFile);

            if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
            {
                PCritical(
                    mMainWindow,
                    QObject::tr("Petrack"),
                    QObject::tr("Cannot open %1:\n%2.").arg(mExtrCalibFile).arg(file.errorString()));
                return false;
            }

            QTextStream file_out(&file);

            file_out << points3D.size() << Qt::endl;
            for(size_t i = 0; i < points3D.size(); ++i)
            {
                file_out << points3D.at(i).x << " " << points3D.at(i).y << " " << points3D.at(i).z << " "
                         << points2D.at(i).x << " " << points2D.at(i).y << Qt::endl;
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


/**
 * @brief Extrinsic calibration with help of cv::solvePnP
 */
std::optional<ExtrinsicParameters> ExtrCalibration::calibExtrParams()
{
    if(points3D.empty() || points2D.empty() || points2D.size() != points3D.size())
    {
        QString msg = QString{"Invalid point correspondences for camera calibration\n"
                              "2D points: %1, 3D points %2"}
                          .arg(points2D.size())
                          .arg(points3D.size());
        PWarning(nullptr, "Invalid point correspondences", msg);
        return std::nullopt;
    }

    cv::Vec3d translation_vector2{0., 0., 0.};

    int bS = mMainWindow->getImageBorderSize();
    /* Create Camera-Matrix form Camera-Params in the Petrack-GUI */
    cv::Mat camMat = mControlWidget->getIntrinsicCameraParams().cameraMatrix;
    camMat.at<double>(0, 2) -= bS;
    camMat.at<double>(1, 2) -= bS;

    cv::Mat distMat = cv::Mat::zeros(cv::Size(8, 1), CV_64F);

    /* Create Mat-objects of point correspondences */
    cv::Mat op(points3D);
    cv::Mat ip(points2D);

    /* Mat-objects for result rotation and translation vectors */
    cv::Mat rvec(3, 1, CV_64F), /*,0),*/ tvec(3, 1, CV_64F); //,0);

    // Solve the PnP-Problem to calibrate the camera to its environment
    cv::solvePnP(op, ip, camMat, distMat, rvec, tvec, false, cv::SOLVEPNP_ITERATIVE);

    cv::Mat rot_mat(3, 3, CV_64F); //, 0);
    // Transform the rotation vector into a rotation matrix with opencvs rodrigues method
    Rodrigues(rvec, rot_mat);

    //(inverse of rot_mat is its transposed) we want rot_inverse times tvec, which looks like
    translation_vector2[0] = rot_mat.at<double>(0, 0) * tvec.at<double>(0) +
                             rot_mat.at<double>(1, 0) * tvec.at<double>(1) +
                             rot_mat.at<double>(2, 0) * tvec.at<double>(2);
    translation_vector2[1] = rot_mat.at<double>(0, 1) * tvec.at<double>(0) +
                             rot_mat.at<double>(1, 1) * tvec.at<double>(1) +
                             rot_mat.at<double>(2, 1) * tvec.at<double>(2);
    translation_vector2[2] = rot_mat.at<double>(0, 2) * tvec.at<double>(0) +
                             rot_mat.at<double>(1, 2) * tvec.at<double>(1) +
                             rot_mat.at<double>(2, 2) * tvec.at<double>(2);

    SPDLOG_INFO("-.- ESTIMATED ROTATION -.-");
    SPDLOG_INFO("{}, {}, {}", rot_mat.at<double>(0, 0), rot_mat.at<double>(0, 1), rot_mat.at<double>(0, 2));
    SPDLOG_INFO("{}, {}, {}", rot_mat.at<double>(1, 0), rot_mat.at<double>(1, 1), rot_mat.at<double>(1, 2));
    SPDLOG_INFO("{}, {}, {}", rot_mat.at<double>(2, 0), rot_mat.at<double>(2, 1), rot_mat.at<double>(2, 2));
    SPDLOG_INFO("-.- ESTIMATED TRANSLATION -.-");
    SPDLOG_INFO("{}, {}, {}", tvec.at<double>(0), tvec.at<double>(1), tvec.at<double>(2));

    SPDLOG_INFO("-.- Translation vector -.-");
    SPDLOG_INFO("{}, {}, {}", translation_vector2[0], translation_vector2[1], translation_vector2[2]);

    SPDLOG_INFO("-.- Rotation vector -.-");
    SPDLOG_INFO("{}, {}, {}", rvec.at<double>(0), rvec.at<double>(1), rvec.at<double>(2));

    camHeight = translation_vector2[2] < 0 ? -translation_vector2[2] : translation_vector2[2];

    ExtrinsicParameters results;

    results.rot1 = rvec.at<double>(0);
    results.rot2 = rvec.at<double>(1);
    results.rot3 = rvec.at<double>(2);

    results.trans1 = translation_vector2[0];
    results.trans2 = translation_vector2[1];
    results.trans3 = translation_vector2[2];

    if(!calcReprojectionError(results))
    {
        SPDLOG_WARN("Extrinsic calibration not possible! Please select other 2D/3D points!");
        results.rot1 = 0;
        results.rot2 = 0;
        results.rot3 = 0;

        translation_vector2[0] = 0;
        translation_vector2[1] = 0;
        translation_vector2[2] = 0;

        results.trans1 = translation_vector2[0];
        results.trans2 = translation_vector2[1];
        results.trans3 = translation_vector2[2];

        reprojectionError = ReprojectionError{};

        PCritical(
            mMainWindow,
            QObject::tr("Petrack"),
            QObject::tr("Error: Could not calculate extrinsic calibration. Please select other 2D/3D point "
                        "correspondences for extrinsic calibration!"));
        return results;
    }

    SPDLOG_INFO("End of extern calibration!");
    mMainWindow->getScene()->update();
    return results;
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
bool ExtrCalibration::calcReprojectionError(const ExtrinsicParameters &extrParams)
{
    //////
    /// \brief error measurements
    ///
    double val, max_px = -1.0, max_pH = -1.0, max_dH = -1.0, var_px = 0, sd_px = 0, var_pH = 0, sd_pH = 0, var_dH = 0,
                sd_dH = 0, sum_px = 0, sum_pH = 0, sum_dH = 0;

    size_t num_points = get2DList().size();
    if(num_points == 0 || num_points != get3DList().size())
    {
        reprojectionError = ReprojectionError{};
        return false;
    }

    for(size_t i = 0; i < num_points; i++)
    {
        cv::Point2f p2d   = get2DList().at(i);
        cv::Point3f p3d   = get3DList().at(i);
        auto        trans = mControlWidget->getCalibCoord3DTrans();
        p3d -= trans.toCvPoint();
        cv::Point2f p3dTo2d = getImagePoint(p3d, extrParams);

        // Error measurements metric (cm)
        cv::Point3f p2dTo3d = get3DPoint(p2d, p3d.z, extrParams);

        cv::Point3f p2dTo3dMapDefaultHeight = get3DPoint(p2d, mControlWidget->getDefaultHeight(), extrParams);

        cv::Point3f p3dTo2dTo3dMapDefaultHeight = get3DPoint(p3dTo2d, mControlWidget->getDefaultHeight(), extrParams);

        val = sqrt(pow(p3d.x - p2dTo3d.x, 2) + pow(p3d.y - p2dTo3d.y, 2));
        if(val > max_pH)
        {
            max_pH = val;
        }
        sum_pH += val;

        val = sqrt(
            pow(p3dTo2dTo3dMapDefaultHeight.x - p2dTo3dMapDefaultHeight.x, 2) +
            pow(p3dTo2dTo3dMapDefaultHeight.y - p2dTo3dMapDefaultHeight.y, 2));
        if(val > max_dH)
        {
            max_dH = val;
        }
        sum_dH += val;

        // Error measurements pixel
        val = sqrt(pow(p3dTo2d.x - p2d.x, 2) + pow(p3dTo2d.y - p2d.y, 2));
        // Maximum
        if(val > max_px)
        {
            max_px = val;
        }
        sum_px += val;
    }
    for(size_t i = 0; i < num_points; i++)
    {
        cv::Point2f p2d   = get2DList().at(i);
        cv::Point3f p3d   = get3DList().at(i);
        auto        trans = mControlWidget->getCalibCoord3DTrans();
        p3d -= trans.toCvPoint();
        cv::Point2f p3d_to_2d = getImagePoint(p3d, extrParams);

        // Error measurements metric (cm)
        cv::Point3f p2d_to_3d = get3DPoint(p2d, p3d.z, extrParams);

        cv::Point3f p2d_to_3d_mapDefaultHeight =
            get3DPoint(p2d, mControlWidget->getDefaultHeight(), extrParams); // mStatusPosRealHeight->value()); ?

        cv::Point3f p3d_to2d_to3d_mapDefaultHeight =
            get3DPoint(p3d_to_2d, mControlWidget->getDefaultHeight(), extrParams);

        val = pow(sqrt(pow(p3d.x - p2d_to_3d.x, 2) + pow(p3d.y - p2d_to_3d.y, 2)) - (sum_pH / num_points), 2);
        var_pH += val;

        val =
            pow(sqrt(
                    pow(p3d_to2d_to3d_mapDefaultHeight.x - p2d_to_3d_mapDefaultHeight.x, 2) +
                    pow(p3d_to2d_to3d_mapDefaultHeight.y - p2d_to_3d_mapDefaultHeight.y, 2)) -
                    (sum_dH / num_points),
                2);
        var_dH += val;

        val = pow(sqrt(pow(p3d_to_2d.x - p2d.x, 2) + pow(p3d_to_2d.y - p2d.y, 2)) - (sum_px / num_points), 2);
        var_px += val;
    }

    // average
    sum_pH /= num_points;
    var_pH /= num_points;
    sd_pH = sqrt(var_pH);
    SPDLOG_INFO(
        "Reprojection error (pointHeight) average: {}cm (standard deviation: {}, variance: {}, max error: {}cm)",
        sum_pH,
        sd_pH,
        var_pH,
        max_pH);

    // average
    sum_dH /= num_points;
    var_dH /= num_points;
    sd_dH = sqrt(var_dH);
    SPDLOG_INFO(
        "Reprojection error (defaultHeight={}) average: {}cm (standard deviation: {}, variance: {}, max error: {}cm)",
        mControlWidget->getDefaultHeight(),
        sum_dH,
        sd_dH,
        var_dH,
        max_dH);

    // average
    sum_px /= num_points;
    var_px /= num_points;
    sd_px = sqrt(var_px);
    SPDLOG_INFO(
        "Reprojection error (Pixel) average: {}px (standard deviation: {}, variance: {}, max error: {}px)",
        sum_px,
        sd_px,
        var_px,
        max_px);

    reprojectionError = ReprojectionError{
        sum_pH,
        sd_pH,
        var_pH,
        max_pH,
        sum_dH,
        sd_dH,
        var_dH,
        max_dH,
        sum_px,
        sd_px,
        var_px,
        max_px,
        mControlWidget->getDefaultHeight()};

    return reprojectionError.pointHeightAvg() > MAX_AV_ERROR ?
               false :
               true; // Falls pixel fehler im schnitt > 20 ist das Ergebnis nicht akzeptabel
}

/**
 * @brief Projects the 3D point to the image plane
 *
 * Projection is done by multiplying with the external camera matrix
 * composed out of rotation and translation aquired in ExtrCalibration::calibExtrParams().
 * After that, the internal camera matrix is applied.
 *
 * @param p3d 3D point to transform in cm
 * @return calculated 2D projection of p3d
 */
cv::Point2f ExtrCalibration::getImagePoint(cv::Point3f p3d) const
{
    return getImagePoint(p3d, mControlWidget->getExtrinsicParameters());
}


cv::Point2f ExtrCalibration::getImagePoint(cv::Point3f p3d, const ExtrinsicParameters &extrParams) const
{
    auto swap = mControlWidget->getCalibCoord3DSwap();
    p3d.x *= swap.x ? -1 : 1;
    p3d.y *= swap.y ? -1 : 1;
    p3d.z *= swap.z ? -1 : 1;

    // Adding the coordsystem translation from petrack window
    auto trans = mControlWidget->getCalibCoord3DTrans();
    p3d += trans.toCvPoint();

    // ToDo: use projectPoints();
    int bS = mMainWindow->getImage() ? mMainWindow->getImageBorderSize() : 0;

    double rvec_array[3], translation_vector[3];

    rvec_array[0] = extrParams.rot1;
    rvec_array[1] = extrParams.rot2;
    rvec_array[2] = extrParams.rot3;

    cv::Mat rvec(3, 1, CV_64F, rvec_array), rot_inv;
    cv::Mat rot_mat(3, 3, CV_64F), e(3, 3, CV_64F);
    // Transform the rotation vector into a rotation matrix with opencvs rodrigues method
    Rodrigues(rvec, rot_mat);

    // use inverse Matrix to get translation_vector?
    rot_inv = rot_mat.inv(cv::DECOMP_SVD);

    e = rot_inv * rot_mat;

    translation_vector[0] = rot_mat.at<double>(0, 0) * extrParams.trans1 +
                            rot_mat.at<double>(0, 1) * extrParams.trans2 + rot_mat.at<double>(0, 2) * extrParams.trans3;
    translation_vector[1] = rot_mat.at<double>(1, 0) * extrParams.trans1 +
                            rot_mat.at<double>(1, 1) * extrParams.trans2 + rot_mat.at<double>(1, 2) * extrParams.trans3;
    translation_vector[2] = rot_mat.at<double>(2, 0) * extrParams.trans1 +
                            rot_mat.at<double>(2, 1) * extrParams.trans2 + rot_mat.at<double>(2, 2) * extrParams.trans3;

    cv::Point3f point3D;

    point3D.x = rot_mat.at<double>(0, 0) * p3d.x + rot_mat.at<double>(0, 1) * p3d.y + rot_mat.at<double>(0, 2) * p3d.z +
                translation_vector[0];
    point3D.y = rot_mat.at<double>(1, 0) * p3d.x + rot_mat.at<double>(1, 1) * p3d.y + rot_mat.at<double>(1, 2) * p3d.z +
                translation_vector[1];
    point3D.z = rot_mat.at<double>(2, 0) * p3d.x + rot_mat.at<double>(2, 1) * p3d.y + rot_mat.at<double>(2, 2) * p3d.z +
                translation_vector[2];

    cv::Point2f point2D = cv::Point2f(0.0, 0.0);
    if(point3D.z != 0)
    {
        const auto camMat = mControlWidget->getIntrinsicCameraParams();
        const auto fx     = camMat.getFx();
        const auto fy     = camMat.getFy();
        const auto cx     = camMat.getCx();
        const auto cy     = camMat.getCy();
        point2D.x         = (fx * point3D.x) / point3D.z + (cx - bS);
        point2D.y         = (fy * point3D.y) / point3D.z + (cy - bS);
    }
    if(false && bS > 0)
    {
        point2D.x += bS;
        point2D.y += bS;
    }
    return point2D;
}

/**
 * @brief Rotate a given vector from camera coordinate system to world coordinate system
 *
 * When the world coordinate system is not aligned to the camera-system,
 * some direction dependent calculations (like head orientation) have to be rotated to be correctly exported.
 *
 * @param camVec the Vector to be rotated matching the camera coordinate system.
 * @return the rotated vector.
 */
cv::Vec3d ExtrCalibration::camToWorldRotation(const cv::Vec3d &camVec) const
{
    // Transform the rotation vector into a rotation matrix with opencvs rodrigues method
    cv::Matx<double, 3, 3> rotMat(3, 3, CV_64F);
    const auto            &extrParams = mControlWidget->getExtrinsicParameters();
    const auto             rvec       = cv::Vec3d(extrParams.rot1, extrParams.rot2, extrParams.rot3);
    Rodrigues(rvec, rotMat);

    auto      rotInv   = rotMat.inv(cv::DECOMP_LU);
    cv::Vec3d worldVec = rotInv * camVec;
    return worldVec;
}

/**
 * @brief Tranforms a 2D point into a 3D point with given height.
 *
 * @param p2d 2D pixel point (without border)
 * @param h height i.e. distance to xy-plane in cm
 * @return calculated 3D point in cm
 */
cv::Point3f ExtrCalibration::get3DPoint(const cv::Point2f &p2d, double h) const
{
    return get3DPoint(p2d, h, mControlWidget->getExtrinsicParameters());
}


cv::Point3f ExtrCalibration::get3DPoint(const cv::Point2f &p2d, double h, const ExtrinsicParameters &extrParams) const
{
    int bS = mMainWindow->getImage() ? mMainWindow->getImageBorderSize() : 0;

    cv::Point3f resultPoint, tmpPoint;

    // Transform the rotation vector into a rotation matrix with opencvs rodrigues method
    cv::Matx<double, 3, 3> rot_inv;
    cv::Matx<double, 3, 3> rot_mat(3, 3, CV_64F);
    const cv::Mat          rvec = (cv::Mat_<double>(3, 1) << extrParams.rot1, extrParams.rot2, extrParams.rot3);
    Rodrigues(rvec, rot_mat);
    rot_inv = rot_mat.inv(cv::DECOMP_LU, nullptr);

    // Create translation vector
    cv::Vec3d translation{extrParams.trans1, extrParams.trans2, extrParams.trans3};

    const auto camMat = mControlWidget->getIntrinsicCameraParams();
    const auto fx     = camMat.getFx();
    const auto fy     = camMat.getFy();
    const auto cx     = camMat.getCx();
    const auto cy     = camMat.getCy();

    // Subtract principal point and border, so we can assume pinhole camera
    const cv::Vec2d centeredImagePoint{p2d.x - (cx - bS), p2d.y - (cy - bS)};


    /* Basic Idea:
     * All points projecting onto a point on the image plane lie on the same
     * line (cf. pinhole camera model). We can determine this line in the form:
     *
     * g: x = lambda * v
     *
     * This line exists in camera coordinates. Let v be the projection with
     * depth 1 (i.e. v_3 = 1). Then lambda is the depth of the resulting point.
     * We'll continue to call lambda z instead, to show this.
     * We now want to determine the depth at which the resulting point has height h
     * in world coordinates. The transformation from cam to world is:
     *
     * W = R * C - T
     * W   := Point in World Coords
     * C   := Point in Cam Coords
     * R,T := Rotation and Translation of Cam
     *
     * By putting in our x = z * v, we get:
     *     W          = R * (z * v) - T
     * <=> W          = z * Rv - T
     * <=> W + T      = z * Rv
     * <=> (W + T)/Rv = z
     * We select the third row of this to solve for z. Finally g(z) is transformed
     * into World Coords.
     */

    // calc Rv, (R = rot_inv)
    // rotatedProj = Rv
    const cv::Vec2d focalLength{fx, fy};
    const cv::Vec2d pinholeProjectionXY = cv::Mat(centeredImagePoint.div(focalLength));
    const cv::Vec3d pinholeProjectionXY1{pinholeProjectionXY[0], pinholeProjectionXY[1], 1};
    const cv::Vec3d rotatedProj = rot_inv * pinholeProjectionXY1;

    // determine z via formula from comment above; using 3rd row
    double z = (h + translation[2]) / rotatedProj[2];

    // Evaluate line at depth z; calc point in camera coords
    // written this way instead of z * pinholeProjectionXY1 (i.e. z * v) to not change test results due to floating
    // point precision diff
    resultPoint.x = (p2d.x - (cx - bS));
    resultPoint.y = (p2d.y - (cy - bS));
    resultPoint.z = z;

    resultPoint.x = resultPoint.x * z / fx;
    resultPoint.y = resultPoint.y * z / fy;

    // We transform from cam coords to world coords with W = R * C - T
    // we now calc: W = R * (C - R^-1*T), which is equivalent
    translation = rot_mat * translation;
    tmpPoint.x  = resultPoint.x - translation[0];
    tmpPoint.y  = resultPoint.y - translation[1];
    tmpPoint.z  = resultPoint.z - translation[2];

    resultPoint.x = rot_inv(0, 0) * (tmpPoint.x) + rot_inv(0, 1) * (tmpPoint.y) + rot_inv(0, 2) * (tmpPoint.z);
    resultPoint.y = rot_inv(1, 0) * (tmpPoint.x) + rot_inv(1, 1) * (tmpPoint.y) + rot_inv(1, 2) * (tmpPoint.z);
    resultPoint.z = rot_inv(2, 0) * (tmpPoint.x) + rot_inv(2, 1) * (tmpPoint.y) + rot_inv(2, 2) * (tmpPoint.z);


    // Coordinate Transformations
    auto trans = mControlWidget->getCalibCoord3DTrans();
    resultPoint -= trans.toCvPoint();

    auto swap = mControlWidget->getCalibCoord3DSwap();
    resultPoint.x *= swap.x ? -1 : 1;
    resultPoint.y *= swap.y ? -1 : 1;
    resultPoint.z *= swap.z ? -1 : 1;

    return resultPoint;
}

bool ExtrCalibration::isOutsideImage(cv::Point2f p2d) const
{
    int bS = mMainWindow->getImage() ? mMainWindow->getImageBorderSize() : 0;
    if(mMainWindow->getImage())
    {
        if(!isnormal(p2d.x) || !isnormal(p2d.y) || !isnormal(p2d.x) || !isnormal(p2d.y))
        {
            return true;
        }
        if(isnan(p2d.x) || isnan(p2d.y) || isinf(p2d.x) || isinf(p2d.y))
        {
            return true;
        }
        return p2d.x < -bS || p2d.x > mMainWindow->getImage()->width() - bS || p2d.y < -bS ||
               p2d.y > mMainWindow->getImage()->height() - bS;
    }
    else
    {
        return false;
    }
}

ReprojectionError ExtrCalibration::getReprojectionError()
{
    if(!reprojectionError.isValid())
    {
        calcReprojectionError(mControlWidget->getExtrinsicParameters());
    }
    return reprojectionError;
}


void ExtrCalibration::setXml(QDomElement &elem)
{
    reprojectionError.setXml(elem);
}

void ExtrCalibration::getXml(QDomElement &elem)
{
    QDomElement subElem;
    QString     styleString;

    for(subElem = elem.firstChildElement(); !subElem.isNull(); subElem = subElem.nextSiblingElement())
    {
        if(subElem.tagName() == "REPROJECTION_ERROR")
        {
            reprojectionError.getXml(subElem);
        }
    }
}

void ReprojectionError::getXml(QDomElement &subElem)
{
    if(subElem.hasAttribute("AVG_PH"))
    {
        mPointHeightAvg = subElem.attribute("AVG_PH").toDouble();
    }
    if(subElem.hasAttribute("SD_PH"))
    {
        mPointHeightStdDev = subElem.attribute("SD_PH").toDouble();
        if(mPointHeightStdDev < 0)
        {
            mPointHeightVariance = -1;
        }
        else
        {
            mPointHeightVariance = pow(mPointHeightStdDev, 2);
        }
    }
    if(subElem.hasAttribute("MAX_PH"))
    {
        mPointHeightMax = subElem.attribute("MAX_PH").toDouble();
    }
    if(subElem.hasAttribute("AVG_DH"))
    {
        mDefaultHeightAvg = subElem.attribute("AVG_DH").toDouble();
    }
    if(subElem.hasAttribute("SD_DH"))
    {
        mDefaultHeightStdDev = subElem.attribute("SD_DH").toDouble();
        if(mDefaultHeightStdDev < 0)
        {
            mDefaultHeightVariance = -1;
        }
        else
        {
            mDefaultHeightVariance = pow(mDefaultHeightStdDev, 2);
        }
    }
    if(subElem.hasAttribute("MAX_DH"))
    {
        mDefaultHeightMax = subElem.attribute("MAX_DH").toDouble();
    }
    if(subElem.hasAttribute("AVG_PX"))
    {
        mPixelAvg = subElem.attribute("AVG_PX").toDouble();
    }
    if(subElem.hasAttribute("SD_PX"))
    {
        mPixelStdDev = subElem.attribute("SD_PX").toDouble();
        if(mPixelStdDev < 0)
        {
            mPixelVariance = -1;
        }
        else
        {
            mPixelVariance = pow(mPixelStdDev, 2);
        }
    }
    if(subElem.hasAttribute("MAX_PX"))
    {
        mPixelMax = subElem.attribute("MAX_PX").toDouble();
    }
    if(subElem.hasAttribute("USED_HEIGHT"))
    {
        mUsedDefaultHeight = subElem.attribute("USED_HEIGHT").toDouble();
    }

    auto data = getData();
    mValid    = !std::any_of(data.begin(), data.end(), [](double a) { return !std::isfinite(a) || a < 0; });
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

double ReprojectionError::pointHeightAvg() const
{
    return mPointHeightAvg;
}

double ReprojectionError::pointHeightStdDev() const
{
    return mPointHeightStdDev;
}

double ReprojectionError::pointHeightVariance() const
{
    return mPointHeightVariance;
}

double ReprojectionError::pointHeightMax() const
{
    return mPointHeightMax;
}

double ReprojectionError::defaultHeightAvg() const
{
    return mDefaultHeightAvg;
}

double ReprojectionError::defaultHeightStdDev() const
{
    return mDefaultHeightStdDev;
}

double ReprojectionError::defaultHeightVariance() const
{
    return mDefaultHeightVariance;
}

double ReprojectionError::defaultHeightMax() const
{
    return mDefaultHeightMax;
}

double ReprojectionError::pixelAvg() const
{
    return mPixelAvg;
}

double ReprojectionError::pixelStdDev() const
{
    return mPixelStdDev;
}

double ReprojectionError::pixelVariance() const
{
    return mPixelVariance;
}

double ReprojectionError::pixelMax() const
{
    return mPixelMax;
}

double ReprojectionError::usedDefaultHeight() const
{
    return mUsedDefaultHeight;
}
