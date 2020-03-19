#include "brightContrastFilter.h"

using namespace::cv;

BrightContrastFilter::BrightContrastFilter()
    :Filter()
{
    mB.setMinimum(-100.);
    mB.setMaximum(100.);
    mB.setValue(0.);
    mB.setFilter(this);

    mC.setMinimum(-100.);
    mC.setMaximum(100.);
    mC.setValue(0.);
    mC.setFilter(this);

//    mLutMat = cvCreateMatHeader(1, 256, CV_8UC1);
//    cvSetData(mLutMat, mLut, 0);
//    setOnCopy(true);
}

Mat BrightContrastFilter::act(Mat &img, Mat &res)
{

    int i, v;
    double delta, a, b;
    /*
     * The algorithm is by Werner D. Streidt
     * (http://visca.com/ffactory/archives/5-99/msg00021.html)
     */
    if (mC.getValue() > 0)
    {
        delta = 127.*mC.getValue()/100.;
        a = 255./(255. - delta*2.);
        b = a*(mB.getValue() - delta);
//        for (i = 0; i < 256; i++)
//        {
//            v = cvRound(a*i + b);
//            if (v < 0)
//                v = 0;
//            else if (v > 255)
//                v = 255;
//            mLut[i] = (uchar) v;
//        }
    }
    else
    {
        delta = -128.*mC.getValue()/100.;
        a = (256.-delta*2.)/255.;
        b = a*mB.getValue() + delta;
//        for (i = 0; i < 256; i++)
//        {
//            v = cvRound(a*i + b);
//            if (v < 0)
//                v = 0;
//            else if (v > 255)
//                v = 255;
//            mLut[i] = (uchar) v;
//        }
    }

//    cvLUT(imgIpl, resIpl, mLutMat);

    img.convertTo(res,-1,a,b);
//    LUT(img,mLutMat,res);
//    cvReleaseImage(&resIpl);
//    resIpl = cvCreateImage(cvSize(res.cols,res.rows),8,3);
//    IplImage tmpIpl = res;
//    cvCopy(&tmpIpl,resIpl);


    return res;
}

Parameter* BrightContrastFilter::getBrightness()
{
    return &mB;
}

Parameter* BrightContrastFilter::getContrast()
{
    return &mC;
}
