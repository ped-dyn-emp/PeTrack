#ifndef CALIBFILTER_H
#define CALIBFILTER_H

#include "filter.h"


class CalibFilter : public Filter
{
private:
    Parameter mFx;
    Parameter mFy;
    Parameter mCx;
    Parameter mCy;
    Parameter mR2;//mK1
    Parameter mR4;//mK2
    Parameter mTx;
    Parameter mTy;
    Parameter mR6;//mK3
    Parameter mK4;
    Parameter mK5;
    Parameter mK6;

public:
    CalibFilter();

//    IplImage* act(IplImage *img, IplImage *res);
    cv::Mat act(cv::Mat &img, cv::Mat &res);

//     bool changed();

    Parameter *getFx();
    Parameter *getFy();
    Parameter *getCx();
    Parameter *getCy();
    Parameter *getR2();
    Parameter *getR4();
    Parameter *getTx();
    Parameter *getTy();
    Parameter *getR6();
    Parameter *getK4();
    Parameter *getK5();
    Parameter *getK6();
};

#endif
