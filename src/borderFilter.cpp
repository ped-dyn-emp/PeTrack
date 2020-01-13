#include "borderFilter.h"
#include "helper.h"

using namespace::cv;

BorderFilter::BorderFilter()
    :Filter()
{
    mS.setMinimum(0.);
    mS.setMaximum(200.);
    mS.setValue(0.);
    mS.setFilter(this);

    mR.setMinimum(0.);
    mR.setMaximum(200.);
    mR.setValue(0.);
    mR.setFilter(this);

    mG.setMinimum(0.);
    mG.setMaximum(200.);
    mG.setValue(0.);
    mG.setFilter(this);

    mB.setMinimum(0.);
    mB.setMaximum(200.);
    mB.setValue(0.);
    mB.setFilter(this);

    setOnCopy(false);
//    setOnCopy(true);
}

Mat BorderFilter::act(Mat &img, Mat &res)
{
    //return img;


//    int x, y;
//    bool useOpenCVBorderFilter = true;

//    debout << "start borderFilter: " << getElapsedTime() <<endl;
    // border size
    int s = (int) mS.getValue(); // schon beim zuweisen auf param gemacht (2*, da undistord filter anscheinend sonst probleme hat)
    int r = (int) mR.getValue();
    int g = (int) mG.getValue();
    int b = (int) mB.getValue();

//    setResStored(true); // wofuer?

//    if (useOpenCVBorderFilter)
//    {
//        Mat img = cvarrToMat(imgIpl),
//            res = cvarrToMat(resIpl);
//        res.create(img.rows+2*s,img.cols+2*s,img.depth());//CV_8UC(img.channels()));
//        debout << "res: " << res.cols << "x" << res.rows << endl;
        cv::copyMakeBorder(img,res, s, s, s, s,cv::BORDER_CONSTANT,cv::Scalar(b,g,r));
//        debout << "res: " << res.cols << "x" << res.rows << endl;
        //        cvCopyMakeBorder( img, res, cv::Point2f(0,0), cv::BORDER_CONSTANT , cv::Scalar(r,g,b) );
//        cvReleaseImage(&resIpl);

//        resIpl = cvCreateImage(cvSize(res.cols,res.rows),8,3);
//        IplImage tmpIpl = res;
//        cvCopy(&tmpIpl,resIpl);

//    }else
//    {
//        CvSize sz = cvGetSize(imgIpl);

//        sz.width  += 2*s;
//        sz.height += 2*s;

//        if (imgIpl != resIpl)
//            cvReleaseImage(&resIpl);

//        resIpl = cvCreateImage(sz, 8, imgIpl->nChannels); // war 8, 3
//            //     resIpl->origin = imgIpl->origin; // because 1 - bottom-left origin (Windows bitmaps style) is not default!!!

//        // Pointer to the data information in the IplImage
//        unsigned char *dataIn  = (unsigned char *) imgIpl->imageData;
//        unsigned char *dataOut = (unsigned char *) resIpl->imageData;
//        // set poiner to value before array, because ++i is more effective than i++
//        --dataIn; --dataOut;
//        unsigned char *yDataIn=dataIn, *yDataOut=dataOut;
//        if (imgIpl->nChannels == 1)
//        {
//            for (y = 0; y < resIpl->height; ++y)
//            {
//                for (x = 0; x < resIpl->width; ++x)
//                {
//                    if ((x >= s) && (x < s+imgIpl->width) && (y >= s) && (y < s+imgIpl->height))
//                        *(++dataOut) = *(++dataIn);
//                    else
//                        // for 1 channel just blue is taken as grey value, because qt does not support grey color dialogs
//                        *(++dataOut) = b;
//                }
//                // we need this, because sometimes width != widthstep eg for width%4 != 0
//                if ((y >= s) && (y < s+imgIpl->height))
//                    dataIn = (yDataIn+=imgIpl->widthStep);
//                dataOut = (yDataOut+=resIpl->widthStep);
//            }
//        }
//        else if (imgIpl->nChannels == 3)
//        {
//            for (y = 0; y < resIpl->height; ++y)
//            {
//                for (x = 0; x < resIpl->width; ++x)
//                {
//                    if ((x >= s) && (x < s+imgIpl->width) && (y >= s) && (y < s+imgIpl->height))
//                    {
//                        *(++dataOut) = *(++dataIn);
//                        *(++dataOut) = *(++dataIn);
//                        *(++dataOut) = *(++dataIn);
//                    }
//                    else
//                    {
//                        *(++dataOut) = b;
//                        *(++dataOut) = g;
//                        *(++dataOut) = r;
//                    }
//                }
//                // we need this, because sometimes width != widthstep eg for width%4 != 0
//                if ((y >= s) && (y < s+imgIpl->height))
//                    dataIn = (yDataIn+=imgIpl->widthStep);
//                dataOut = (yDataOut+=resIpl->widthStep);
//            }
//        }
//        else
//        {
//            debout << "Error: Channel different than 1 and 3 should not be allowed!" << endl;
//        }
//    }
//    debout << "end borderFilter: " << getElapsedTime() <<endl;
    return res;
}

Parameter* BorderFilter::getBorderSize()
{
    return &mS;
}
Parameter* BorderFilter::getBorderColR()
{
    return &mR;
}
Parameter* BorderFilter::getBorderColG()
{
    return &mG;
}
Parameter* BorderFilter::getBorderColB()
{
    return &mB;
}
