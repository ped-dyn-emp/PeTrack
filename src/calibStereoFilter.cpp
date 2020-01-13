#include "calibStereoFilter.h"

using namespace::cv;

CalibStereoFilter::CalibStereoFilter()
    :Filter()
{
    setOnCopy(false); // da in stereoContext der Speicherplatz fuer res liegt
}



Mat CalibStereoFilter::act(Mat &img, Mat &res)
{
#ifndef STEREO_DISABLED
    return cvarrToMat(mStereoContext->getRectified(cameraUnset)); // kein Zugriff auf mAnimation->getCaptureStereo()->getCamera()
#else
    return Mat();
#endif
}
void CalibStereoFilter::setStereoContext(pet::StereoContext* stereoContext)
{
    mStereoContext = stereoContext;
}

