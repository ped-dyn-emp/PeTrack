#ifndef BORDERFILTER_H
#define BORDERFILTER_H

#include "filter.h"


class BorderFilter : public Filter
{
private:
    Parameter mR; // red
    Parameter mG; // green
    Parameter mB; // blue
    Parameter mS; // size

public:
    BorderFilter();

//    IplImage* act(IplImage *img, IplImage *res);
    cv::Mat act(cv::Mat &img, cv::Mat &res);

    Parameter *getBorderSize();
    Parameter *getBorderColR();
    Parameter *getBorderColG();
    Parameter *getBorderColB();
};

#endif
