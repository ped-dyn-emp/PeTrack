#ifndef SWAPFILTER_H
#define SWAPFILTER_H

#include "filter.h"

using namespace::cv;

class SwapFilter : public Filter
{
private:
    Parameter mSwapHorizontally; // 0.0 false; 1.0 true
    Parameter mSwapVertically; // 0.0 false; 1.0 true

public:
    SwapFilter();

//    IplImage* act(IplImage *img, IplImage *res);
    cv::Mat act(cv::Mat &img, cv::Mat &res);

    Parameter *getSwapHorizontally();
    Parameter *getSwapVertically();
};

#endif
