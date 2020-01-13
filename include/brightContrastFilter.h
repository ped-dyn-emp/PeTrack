#ifndef BRIGHTCONTRASTFILTER_H
#define BRIGHTCONTRASTFILTER_H

#include "filter.h"
#include "helper.h"


class BrightContrastFilter : public Filter
{
private:
    Parameter mB;
    Parameter mC;

//    uchar mLut[256];
//    CvMat* mLutMat;

public:
    BrightContrastFilter();

//    IplImage* act(IplImage *img, IplImage *res);
    cv::Mat act(cv::Mat &img, cv::Mat &res);

    Parameter *getBrightness();
    Parameter *getContrast();
};

#endif
