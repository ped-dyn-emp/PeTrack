#ifndef STEREOCONTEXT_H
#define STEREOCONTEXT_H

#include <QString>

#ifdef STEREO
#include <triclops.h>
#endif

#ifdef STEREO
#include "opencv2/calib3d/calib3d_c.h"
#include "opencv2/calib3d.hpp"
#include "pgrAviFile.h"
#else
#include "stereoAviFile.h"
#endif
//#include "backgroundFilter.h"
#include "opencv2/calib3d/calib3d_c.h"
#include "opencv2/calib3d.hpp"

class Petrack;
class Animation;
class BackgroundFilter;

/* Namespace to stop multiple definition problem of Class StereoContext in Petrack and Triclops-Library */
namespace pet{

enum stereoStatus {clean=0, buildInput=1, preprocessed=2, rectified=4, genDisparity=8};

class StereoContext
{
public:

    StereoContext(Petrack* main);

    ~StereoContext();

    // ---------------------------------------------------

    // to use before every new picture pair
    void init(cv::Mat &viewImg);

    void preprocess();
#ifndef STEREO_DISABLED
    IplImage *getRectified(enum Camera camera=cameraRight);

    IplImage *getDisparity(bool *dispNew = NULL);
#endif

    // von person.cpp benoetigt, um frame nummer zu erhalten
    inline Animation *getAnimation()
    {
        return mAnimation;
    }

    // ---------------------------------------------------


    double getCmPerPixel(float z);

    void calcMinMax();

    void indicateNewValues();

    bool getXYZ(int row, int col, float* x, float* y, float* z);

    bool getMedianXYZaround(int col, int row, float* x, float* y, float* z);

    float getZfromDisp(unsigned short int disp);

    bool dispValueValid(unsigned short int disp);

    // ---------------------------------------------------
#ifdef STEREO
    inline TriclopsContext getContext()
    {
        return mTriclopsContext;
    }
#endif
    inline unsigned char getSurfaceValue()
    {
        return mSurfaceValue;
    }
    inline unsigned char getBackForthValue()
    {
        return mBackForthValue;
    }

    inline unsigned short int getMin()
    {
        return mMin;
    }
    inline unsigned short int getMax()
    {
        return mMax;
    }

    inline void addStatus(enum stereoStatus s)
    {
        mStatus |= s;
    }
    inline void setStatus(enum stereoStatus s) // set status up to this level
    {
        mStatus = s*2-1;
    }
    inline void resetStatus()
    {
        mStatus = clean;
    }

    CvMat* getPointCloud();

    bool exportPointCloud(QString dest = "");

protected:

    Animation         *mAnimation;
    Petrack           *mMain;
#ifdef STEREO
    TriclopsContext    mTriclopsContext;
    TriclopsInput      mTriclopsInput;
    TriclopsImage      mTriRectLeft;
    TriclopsImage      mTriRectRight;
    TriclopsImage16    mTriDisparity;
#endif
    BackgroundFilter   *mBackgroundFilterLeft;
    BackgroundFilter   *mBackgroundFilterRight;
#ifndef STEREO_DISABLED
    IplImage           mRectLeft;
    IplImage           mRectRight;
    IplImage           mDisparity;
    CvStereoBMState   *mBMState;
#endif


    cv::Ptr<cv::StereoSGBM>    mSgbm;
    CvMat             *mBMdisparity16;
    CvMat             *mPointCloud;
    unsigned char      mSurfaceValue;
    unsigned char      mBackForthValue;
    unsigned short int mMin;
    unsigned short int mMax;
    short int          mStatus;
};


}

#endif // #ifndef STEREOCONTEXT
