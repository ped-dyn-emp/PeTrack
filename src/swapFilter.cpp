#include "swapFilter.h"
#include "helper.h"

SwapFilter::SwapFilter()
    :Filter()
{
    mSwapVertically.setMinimum(0.);
    mSwapVertically.setMaximum(1.);
    mSwapVertically.setValue(0.);
    mSwapVertically.setFilter(this);

    mSwapHorizontally.setMinimum(0.);
    mSwapHorizontally.setMaximum(1.);
    mSwapHorizontally.setValue(0.);
    mSwapHorizontally.setFilter(this);

    setOnCopy(true);
    //    setOnCopy(false);
}

Mat SwapFilter::act(Mat &img, Mat &res)
{

//    Mat img = cvarrToMat(imgIpl),
//        res = cvarrToMat(resIpl);

//    bool useMat = true;

    bool sV = (bool) mSwapVertically.getValue();
    bool sH = (bool) mSwapHorizontally.getValue();

//    if (!useMat)
//    {
//        int x, y, xStep, yStep;

//        CvSize sz = cvGetSize(imgIpl);

//        if (imgIpl != resIpl)
//            cvReleaseImage(&resIpl);
//        resIpl = cvCreateImage(sz, 8, imgIpl->nChannels); // war 8, 3
//        //     resIpl->origin = imgIpl->origin; // because 1 - bottom-left origin (Windows bitmaps style) is not default!!!
//        setResStored(true);
//        // Pointer to the data information in the IplImage
//        char *dataIn = imgIpl->imageData - 1;
//        char *dataOut;
//        // set poiner to value before array, because ++i is more effective than i++
//        if (sH)
//        {
//            dataOut = resIpl->imageData + resIpl->nChannels * resIpl->width;
//            xStep = - imgIpl->nChannels;
//        }
//        else
//        {
//            dataOut = resIpl->imageData - resIpl->nChannels;
//            xStep = imgIpl->nChannels;
//        }
//        if (sV)
//        {
//            dataOut += (resIpl->height-1) * resIpl->widthStep;
//            yStep = -resIpl->widthStep;
//        }
//        else
//        {
//            yStep = resIpl->widthStep;
//        }
//        char *yDataIn=dataIn, *yDataOut=dataOut;

//        //                char *dataIn;
//        //                char *dataOut = mImage->imageData - 1;
//        //                int lineLen = tempImg->width*tempImg->nChannels;

//        //                for (int i = mImage->height-1; i >= 0 ; --i)
//        //                {
//        //                    dataIn  = tempImg->imageData + i*lineLen - 1; // to swap data we go from bottom to top
//        //                    for (int j = 0; j < mImage->width; ++j)
//        //                    {
//        //                        //cout << i << " " << j << endl;
//        //                        *(++dataOut) = *(++dataIn);
//        //                        *(++dataOut) = *(++dataIn);
//        //                        *(++dataOut) = *(++dataIn);
//        //                    }
//        //                }


//        if (imgIpl->nChannels == 1)
//        {
//            for (y = 0; y < resIpl->height; ++y)
//            {
//                for (x = 0; x < resIpl->width; ++x)
//                {
//                    dataOut+=xStep;
//                    *(dataOut) = *(++dataIn);
//                }
//                // we need this, because sometimes width != widthstep eg for width%4 != 0
//                dataIn = (yDataIn+=imgIpl->widthStep);
//                dataOut = yDataOut+=yStep;
//            }
//        }
//        else if (imgIpl->nChannels == 3)
//        {
//            for (y = 0; y < resIpl->height; ++y)
//            {
//                for (x = 0; x < resIpl->width; ++x)
//                {
//                    dataOut+=xStep;
//                    *(dataOut) = *(++dataIn);
//                    *(dataOut+1) = *(++dataIn);
//                    *(dataOut+2) = *(++dataIn);
//                }
//                // we need this, because sometimes width != widthstep eg for width%4 != 0
//                dataIn = (yDataIn+=imgIpl->widthStep);
//                dataOut = yDataOut+=yStep;
//            }
//        }
//        else
//        {
//            debout << "Error: Channel different than 1 and 3 should not be allowed!" << endl;
//        }
//    }else
//    {


        if (sV && sH)
            cv::flip(img, res, -1); // both
        else if (sV)
            cv::flip(img, res,  0); // vertical
        else if (sH)
            cv::flip(img, res,  1); // horizontal
        else
            res = img;          // nothing to do


    return res;
}

Parameter* SwapFilter::getSwapHorizontally()
{
    return &mSwapHorizontally;
}
Parameter* SwapFilter::getSwapVertically()
{
    return &mSwapVertically;
}
