#include "calibFilter.h"
#include "helper.h"

using namespace::cv;

CalibFilter::CalibFilter()
    :Filter()
{
    getFx()->setMinimum(500.);
    getFx()->setMaximum(5000.);
    getFx()->setValue(881.);
    getFx()->setFilter(this);

    getFy()->setMinimum(500.);
    getFy()->setMaximum(5000.);
    getFy()->setValue(881.);
    getFy()->setFilter(this);

    getCx()->setMinimum(0.);
    getCx()->setMaximum(4096 /*1023.*/);
    getCx()->setValue(511.5);
    getCx()->setFilter(this);

    getCy()->setMinimum(0.);
    getCy()->setMaximum(2160 /*767.*/);
    getCy()->setValue(383.5);
    getCy()->setFilter(this);

    getR2()->setMinimum(-5.);
    getR2()->setMaximum(5.);
    getR2()->setValue(0.);
    getR2()->setFilter(this);

    getR4()->setMinimum(-5.);
    getR4()->setMaximum(5.);
    getR4()->setValue(0.);
    getR4()->setFilter(this);

    getTx()->setMinimum(-5.);
    getTx()->setMaximum(5.);
    getTx()->setValue(0.);
    getTx()->setFilter(this);

    getTy()->setMinimum(-5.);
    getTy()->setMaximum(5.);
    getTy()->setValue(0.);
    getTy()->setFilter(this);

    getR6()->setMinimum(-5.);
    getR6()->setMaximum(5.);
    getR6()->setValue(0.);
    getR6()->setFilter(this);

    getK4()->setMinimum(-5.);
    getK4()->setMaximum(5.);
    getK4()->setValue(0.);
    getK4()->setFilter(this);

    getK5()->setMinimum(-5.);
    getK5()->setMaximum(5.);
    getK5()->setValue(0.);
    getK5()->setFilter(this);

    getK6()->setMinimum(-5.);
    getK6()->setMaximum(5.);
    getK6()->setValue(0.);
    getK6()->setFilter(this);
}

Mat CalibFilter::act(Mat &img, Mat &res)
{
//#if (CV_MAJOR_VERSION == 2)
//    static double _camera[9], _dist[8];
//    static CvMat camera = cvMat(3, 3, CV_64F, _camera);
//    static CvMat dist = cvMat(1, 8, CV_64F, _dist);

//    _camera[0] = getFx()->getValue(); _camera[1] = 0;                   _camera[2] = getCx()->getValue();
//    _camera[3] = 0;                   _camera[4] = getFy()->getValue(); _camera[5] = getCy()->getValue();
//    _camera[6] = 0;                   _camera[7] = 0;                   _camera[8] = 1;
//    _dist[0] = getR2()->getValue(); _dist[1] = getR4()->getValue(); _dist[4] = getR6()->getValue();
//    _dist[2] = getTx()->getValue(); _dist[3] = getTy()->getValue();
//    _dist[5] = getK4()->getValue(); _dist[5] = getK5()->getValue(); _dist[7] = getK6()->getValue();

//    // opencv 2 kann bei undistort2 anscheinend nur noch 8bit bilder verarbeiten?! ==> nicht mehr aktuell fuer OpenCV 3? funktioniert bisher ohne Probleme um eine halb sekunde schneller pro frame!
//    // http://stackoverflow.com/questions/2333868/cvundistort2-and-cvremap-crash

//    IplImage *r = cvCreateImage(cvGetSize(imgIpl),8,1);
//    IplImage *g = cvCreateImage(cvGetSize(imgIpl),8,1);
//    IplImage *b = cvCreateImage(cvGetSize(imgIpl),8,1);
//    IplImage *r2 = cvCreateImage(cvGetSize(imgIpl),8,1);
//    IplImage *g2 = cvCreateImage(cvGetSize(imgIpl),8,1);
//    IplImage *b2 = cvCreateImage(cvGetSize(imgIpl),8,1);
//    if( imgIpl->nChannels == 3 )
//    {
//        cvSplit(imgIpl, r,g,b, NULL);

////      debout << "Distortioncoefficients: " << endl;
////      debout << "r2: " << _dist[0] << "r4: " << _dist[1] << "r6: " << _dist[4]
////           << "tx: " << _dist[2] << "ty: " << _dist[3]
////           << "k4: " << _dist[5] << "k5: " << _dist[6] << "k6: " << _dist[7] << endl;

//        cvUndistort2(r, r2, &camera, &dist);
//        cvUndistort2(g, g2, &camera, &dist);
//        cvUndistort2(b, b2, &camera, &dist);
//        cvMerge(r2, g2, b2, NULL, resIpl);
//    }else
//    {
//        cvUndistort2(imgIpl,resIpl,&camera, &dist);
//    }
//    cvReleaseImage(&r);
//    cvReleaseImage(&g);
//    cvReleaseImage(&b);
//    cvReleaseImage(&r2);
//    cvReleaseImage(&g2);
//    cvReleaseImage(&b2);

//    return resIpl;
//#else
//    Mat img = cvarrToMat(imgIpl),
//        res = cvarrToMat(resIpl);

    Mat camera = (Mat_<float>(3,3) << getFx()->getValue(), 0,                   getCx()->getValue(),
                                          0,                   getFy()->getValue(), getCy()->getValue(),
                                          0,                   0,                   1                   );
    Mat dist = (Mat_<float>(1,8) << getR2()->getValue(), getR4()->getValue(),
                                        getTx()->getValue(), getTy()->getValue(),
                                        getR6()->getValue(),
                                        getK4()->getValue(), getK5()->getValue(), getK6()->getValue());

    undistort(img, res, camera, dist);
//    cvReleaseImage(&resIpl);
//    resIpl = cvCreateImage(cvSize(res.cols,res.rows),8,3);
//    IplImage tmpIpl = res;
//    cvCopy(&tmpIpl,resIpl);

    return res;
//#endif
}

// bool CalibFilter::changed()
// {
//     return Filter::changed() || mFx.changed() || mFy.changed() || mCx.changed() ||  mCy.changed() ||  mR2.changed() ||  mR4.changed() || mTx.changed() ||  mTy.changed();   
// }

Parameter* CalibFilter::getFx()
{
    return &mFx;
}
Parameter* CalibFilter::getFy()
{
    return &mFy;
}
Parameter* CalibFilter::getCx()
{
    return &mCx;
}
Parameter* CalibFilter::getCy()
{
    return &mCy;
}
Parameter* CalibFilter::getR2()
{
    return &mR2;
}
Parameter* CalibFilter::getR4()
{
    return &mR4;
}
Parameter* CalibFilter::getTx()
{
    return &mTx;
}
Parameter* CalibFilter::getTy()
{
    return &mTy;
}
Parameter* CalibFilter::getR6()
{
    return &mR6;
}
Parameter* CalibFilter::getK4()
{
    return &mK4;
}
Parameter* CalibFilter::getK5()
{
    return &mK5;
}
Parameter* CalibFilter::getK6()
{
    return &mK6;
}

