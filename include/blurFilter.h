#ifndef BLURFILTER_H
#define BLURFILTER_H

#include "filter.h"
#include "cv.h" 

class BlurFilter : public Filter
{
public:
    BlurFilter()
        :Filter()
    {
        p=1;
    }
    cv::Mat act(cv::Mat &img, cv::Mat &res);
    void setParam(double newp);

private:
    //parameter
    double p;
};

#endif
