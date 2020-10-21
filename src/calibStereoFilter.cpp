#include "calibStereoFilter.h"

using namespace::cv;

CalibStereoFilter::CalibStereoFilter()
    :Filter()
{
    setOnCopy(false); // da in stereoContext der Speicherplatz fuer res liegt
}



Mat CalibStereoFilter::act(Mat &/*img*/, Mat &/*res*/)
{
    return Mat();

}
void CalibStereoFilter::setStereoContext(pet::StereoContext* stereoContext)
{
    mStereoContext = stereoContext;
}

