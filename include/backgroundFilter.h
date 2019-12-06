#ifndef BACKGROUNDFILTER_H
#define BACKGROUNDFILTER_H

#include "filter.h"

#include "cvaux.h"
#include "opencv/cvaux.h"
#include "opencv2/video/background_segm.hpp"


#include <QString>

#include "stereoContext.h"
//#include "opencv2/video/background_segm.hpp"

//class pet::StereoContext;

class BackgroundFilter : public Filter
{
private:
#if CV_MAJOR_VERSION == 2
    CvBGStatModel* mBgModel;
#elif CV_MAJOR_VERSION == 3
    cv::Ptr<cv::BackgroundSubtractorMOG2> mBgModel;
    //Ptr<BackgroundSubtractor> mBgModel;
#endif
    bool mUpdate; // if 0, kein update des models, sonst schon
    pet::StereoContext** mStereoContext; // zeiger auf den zeiger in petrack mit stereocontext
    cv::Mat mBgPointCloud;
    cv::Mat mForeground;
    QString mLastFile;
    double mDefaultHeight;

public:
    BackgroundFilter();
    ~BackgroundFilter();

    void setDefaultHeight(double h);

    void setUpdate(bool b);
    bool update();

    QString getFilename();
    void setFilename(const QString &fn);

    bool load(QString dest = "");
    bool save(QString dest = "");

    void setStereoContext(pet::StereoContext **sc);
    pet::StereoContext** stereoContext();

    cv::Mat getForeground(); // nutzen, wenn ueber ganzes bild foreground benutzt wird; NULL, wenn keine background subtraction aktiviert
    bool isForeground(int i, int j); // nutzen, wenn einzelne pixel abgefraget werden

    void reset();

    cv::Mat act(cv::Mat &img, cv::Mat &res);
//    IplImage* act(IplImage *img, IplImage *res);

    void maskBg(cv::Mat &mat, float val);
};

#endif
