#ifndef CALIBSTEREOFILTER_H
#define CALIBSTEREOFILTER_H

#include "filter.h"
#include "stereoContext.h"


class CalibStereoFilter : public Filter
{
private:
    pet::StereoContext *mStereoContext;

public:
    CalibStereoFilter();

//    IplImage* act(IplImage *img, IplImage *res);
    cv::Mat act(cv::Mat &img, cv::Mat &res);

    void setStereoContext(pet::StereoContext* stereoContext);
};



#endif
