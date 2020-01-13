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


#include <opencv.hpp>

#ifndef STEREO_DISABLED
#include "cxcore.h"
#include "cvaux.h"
#endif
#include "opencv.hpp"

class Petrack;
class Control;


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
