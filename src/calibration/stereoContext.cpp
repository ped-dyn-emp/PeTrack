/*
 * PeTrack - Software for tracking pedestrians movement in videos
 * Copyright (C) 2025 Forschungszentrum Jülich GmbH, IAS-7
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "stereoContext.h"

#include "animation.h"
#include "control.h"
#include "ellipse.h"
#include "helper.h"
#include "pMessageBox.h"
#include "person.h"
#include "petrack.h"
#include "stereoItem.h"
#include "stereoWidget.h"

#include <QDir>

#ifdef STEREO

//#define TMP_STEREO_SEQ // Stereo Bildsequenz einer Virtuelle Szene
//#define TMP_STEREO_SEQ_DISP // neben der Einzelbildfolge soll auch das berechnete Hoehenfeld eingelesen werden
//#define STEREO_SEQ_FILEBASE "D:/diss/personModel/ownPerson/movie/one/one_"
//#define STEREO_SEQ_FILEBASE "D:/diss/personModel/ownPerson/movie/withHermesMarker/more_"
#define STEREO_SEQ_FILEBASE        "D:/diss/personModel/ownPerson/movie/schraeg/d1/d1.0_"
#define STEREO_SEQ_FIRST_FRAME     1   // 18
#define STEREO_SEQ_LAST_FRAME      132 // maximal 127 bei kleiner stereo-datein 165
#define STEREO_SEQ_MAX_PERS_HEIGHT 180

// war direkt neben dem load, aber sollte ggf in init() auch benutzt werden
#ifdef TMP_STEREO_SEQ
static IplImage *tempLeftImg  = NULL;
static IplImage *tempRightImg = NULL;
#endif

#endif

using namespace ::cv;

pet::StereoContext::StereoContext(Petrack *main)
{
    mMain      = main;
    mAnimation = main->getAnimation();
    mStatus    = clean;

    QString calFile, calFileInt;
    QDate   fileDate = mAnimation->getFileInfo().lastModified().date();
    QString version;

    if((mAnimation->getFileBase()).contains("cam1"))
    {
        if(fileDate < QDate(2009, 11, 30))
        {
            version    = "old";
            calFileInt = ":/calibCam1";
        }
        else
        {
            version    = "new";
            calFileInt = ":/calibCam1New";
        }
        calFile = QDir::tempPath() + "/" + "cam1_7280765.cal";
    }
    else if((mAnimation->getFileBase()).contains("cam2"))
    {
        if(fileDate < QDate(2009, 11, 30))
        {
            version    = "old";
            calFileInt = ":/calibCam2";
        }
        else
        {
            version    = "new";
            calFileInt = ":/calibCam2New";
        }
        calFile = QDir::tempPath() + "/" + "cam2_7280791.cal";
    }
    else
    {
        SPDLOG_ERROR("No cam1 or cam2 string in video filename to detect used camera!");
        return;
    }
    QFile calFpInt(calFileInt);
    QFile calFp(calFile);
    calFp.remove();
    calFpInt.open(QIODevice::ReadOnly);
    calFp.open(QIODevice::WriteOnly);
    calFp.write(calFpInt.readAll());
    calFp.close();
    if(!QFile::exists(calFile))
    {
        SPDLOG_ERROR("Calibration file {} could not be created!", calFile.toStdString());
        return;
    }
    SPDLOG_INFO("Using {} ({}) for calibration.", calFile.toStdString(), version.toStdString());
#ifdef STEREO
    TriclopsError triclopsError;
    triclopsError =
        triclopsGetDefaultContextFromFile(&mTriclopsContext, static_cast<char *>(calFile.toLatin1().data()));
    if(triclopsError != TriclopsErrorOk)
        SPDLOG_ERROR(triclopsErrorToString(triclopsError));

    triclopsError = triclopsSetResolution(mTriclopsContext, 960, 1280); // 240, 320  // 1536, 2048
    if(triclopsError != TriclopsErrorOk)
        SPDLOG_ERROR(triclopsErrorToString(triclopsError));
    triclopsError = triclopsSetSubpixelInterpolation(mTriclopsContext, 1); // turn on sub-pixel interpolation
    if(triclopsError != TriclopsErrorOk)
        SPDLOG_ERROR(triclopsErrorToString(triclopsError));
    triclopsError =
        triclopsSetStrictSubpixelValidation(mTriclopsContext, 1); // make sure strict subpixel validation is on
    if(triclopsError != TriclopsErrorOk)
        SPDLOG_ERROR(triclopsErrorToString(triclopsError));

    //    triclopsError = triclopsSetDisparityMapping(mTriclopsContext, 0,255);
    //    if (triclopsError != TriclopsErrorOk)
    //        debout << triclopsErrorToString(triclopsError) << endl;
    //    triclopsError = triclopsSetDisparityMappingOn(mTriclopsContext, 1); // !!! ausschalten, wenn disparity map
    //    fuer 3d-auswertung genutzt wird - nur zum angucken besser if (triclopsError != TriclopsErrorOk)
    //        debout << triclopsErrorToString(triclopsError) << endl;

    // triclopsSetRectImgQuality(...) entzerrung soll verbessert werden
    triclopsError = triclopsSetStereoQuality(mTriclopsContext, TriStereoQlty_ENHANCED);
    if(triclopsError != TriclopsErrorOk)
        SPDLOG_ERROR(triclopsErrorToString(triclopsError));

    // lets turn off all validation except subpixel and surface
    // this works quite well
    triclopsError = triclopsSetTextureValidation(mTriclopsContext, 0);
    if(triclopsError != TriclopsErrorOk)
        SPDLOG_ERROR(triclopsErrorToString(triclopsError));
    triclopsError = triclopsSetUniquenessValidation(mTriclopsContext, 0);
    if(triclopsError != TriclopsErrorOk)
        SPDLOG_ERROR(triclopsErrorToString(triclopsError));

    // turn on surface validation
    triclopsError = triclopsSetSurfaceValidation(mTriclopsContext, 1);
    if(triclopsError != TriclopsErrorOk)
        SPDLOG_ERROR(triclopsErrorToString(triclopsError));
    triclopsError = triclopsSetSurfaceValidationSize(mTriclopsContext, 200);
    if(triclopsError != TriclopsErrorOk)
        SPDLOG_ERROR(triclopsErrorToString(triclopsError));
    triclopsError = triclopsSetSurfaceValidationDifference(mTriclopsContext, 0.5);
    if(triclopsError != TriclopsErrorOk)
        SPDLOG_ERROR(triclopsErrorToString(triclopsError));
    // turn on back-forth validation
    triclopsError = triclopsSetBackForthValidation(mTriclopsContext, 1);
    if(triclopsError != TriclopsErrorOk)
        SPDLOG_ERROR(triclopsErrorToString(triclopsError));

    triclopsGetSurfaceValidationMapping(mTriclopsContext, &mSurfaceValue);
    triclopsGetBackForthValidationMapping(mTriclopsContext, &mBackForthValue);
#endif
    mMin     = USHRT_MAX;
    mMax     = 0;
    mBMState = nullptr;
}

pet::StereoContext::~StereoContext()
{
#ifdef STEREO
    if(mTriclopsContext)
        triclopsDestroyContext(mTriclopsContext);
#endif
}

// wenn viewImg NULL dann werden die nativ eingelesenen stereobilder genommen
// default: IplImage *viewImg = NULL
void pet::StereoContext::init(Mat &viewImg) //  = NULL
{
    //    static IplImage *lastViewImg = NULL;
    //    if (viewImg == NULL) // for edge mask
    //        viewImg = lastViewImg;
    //    lastViewImg = viewImg;

    if(!viewImg.empty() && (viewImg.cols != 1280 || viewImg.rows != 960))
    {
        SPDLOG_WARN("no images beside 1280x960!");
        return;
    }

    cv::Mat leftImg, rightImg;

#ifdef STEREO
    TriclopsError triclopsError;


    if(!viewImg.empty() && (mAnimation->getCaptureStereo()->getCamera() == cameraLeft))
    {
        leftImg        = cv::Mat(viewImg.size(), viewImg.type());
        cv::Mat tmpImg = viewImg;
        tmpImg.copyTo(leftImg);
    }
    else
        leftImg = mAnimation->getCaptureStereo()->getFrame(cameraLeft);
    if(!viewImg.empty() && (mAnimation->getCaptureStereo()->getCamera() == cameraRight))
    {
        rightImg       = cv::Mat(viewImg.size(), viewImg.type());
        cv::Mat tmpImg = viewImg;
        tmpImg.copyTo(rightImg);
    }
    else
        rightImg = mAnimation->getCaptureStereo()->getFrame(cameraRight);

    // Background subtraction
    //        if (brightContrastChanged || borderChanged || calibChanged)
    //            mBackgroundFilter.reset(); // alle gesammelten hintergrundinfos werden verworfen und bg.changed auf
    //            true gesetzt
    //        if (imageChanged || mBackgroundFilter.changed())
    //            mIplImgFiltered = mBackgroundFilter.apply(mIplImgFiltered);
    //        else
    //            mIplImgFiltered = mBackgroundFilter.getLastResult();

    //    leftImg = mBackgroundFilterLeft.apply(leftImg);
    //    rightImg = mBackgroundFilterRight.apply(rightImg);


    //// um ptgrey die schon entzerrten virtuellen Bilder unterzuschieben
    //// dazu muesste zudem 2x der bereich um triclopsGetImage auskommentiert werden
    //#ifdef TMP_STEREO_SEQ
    //            // temporaere code um stereo-einzelbildfolge einzulesen:
    //            if (tempLeftImg != NULL)
    //                cvReleaseImage(&tempLeftImg);

    //            QString fn = QString(STEREO_SEQ_FILEBASE) +
    //            QString("left_%1.png").arg(mAnimation->getCurrentFrameNum()%(STEREO_SEQ_LAST_FRAME-STEREO_SEQ_FIRST_FRAME+1)
    //            + STEREO_SEQ_FIRST_FRAME, 3, 10, QChar('0')); // [2,159] leftImg=cvLoadImage((char *)
    //            fn.toAscii().data(), CV_LOAD_IMAGE_GRAYSCALE); //-1);

    //            // temporaere code um stereo-einzelbildfolge einzulesen:
    //            -----------------------------------------------------------------------------------------------------
    //            if (tempRightImg != NULL)
    //                cvReleaseImage(&tempRightImg);
    //            fn = QString(STEREO_SEQ_FILEBASE) +
    //            QString("right_%1.png").arg(mAnimation->getCurrentFrameNum()%(STEREO_SEQ_LAST_FRAME-STEREO_SEQ_FIRST_FRAME+1)
    //            + STEREO_SEQ_FIRST_FRAME, 3, 10, QChar('0')); // [2,159] rightImg=cvLoadImage((char *)
    //            fn.toAscii().data(), CV_LOAD_IMAGE_GRAYSCALE); //-1);
    //#endif


    triclopsError = triclopsBuildRGBTriclopsInput(
        leftImg.cols,
        leftImg.rows,
        static_cast<int>(leftImg.step),
        0,
        0,
        rightImg.data,
        leftImg.data,
        NULL,
        &mTriclopsInput);
    if(triclopsError != TriclopsErrorOk)
        SPDLOG_ERROR(triclopsErrorToString(triclopsError));
#endif
    mDisparity = cv::Mat(leftImg.size(), CV_16FC1);

    setStatus(buildInput);

    preprocess();
}

void pet::StereoContext::preprocess()
{
    if(mStatus & buildInput)
    {
#ifdef STEREO
        TriclopsError triclopsError;

        triclopsError = triclopsSetEdgeCorrelation(mTriclopsContext, mMain->getStereoWidget()->useEdge->isChecked());
        if(triclopsError != TriclopsErrorOk)
            SPDLOG_ERROR(triclopsErrorToString(triclopsError));
        triclopsError = triclopsSetEdgeMask(mTriclopsContext, mMain->getStereoWidget()->edgeMaskSize->value());
        if(triclopsError != TriclopsErrorOk)
            SPDLOG_ERROR("{} for the edge mask size", triclopsErrorToString(triclopsError));

        // Preprocessing the images
        triclopsError = triclopsPreprocess(mTriclopsContext, &mTriclopsInput);
        if(triclopsError != TriclopsErrorOk)
            SPDLOG_ERROR(triclopsErrorToString(triclopsError));
        setStatus(preprocessed);
#endif
        //        TriclopsBool b;
        //        triclopsGetLowpass(mTriclopsContext, &b);
        //        debout << b <<endl; == true !!!!
    }
}

// cameraRight is default, da disp mit diesem identisch
cv::Mat pet::StereoContext::getRectified(enum Camera camera)
{
#ifdef TIME_MEASUREMENT
    //        "==========: "
    debout << "in rectify: " << getElapsedTime() << endl;
#endif

    if((camera != cameraRight) && (camera != cameraLeft))
        camera = mAnimation->getCaptureStereo()->getCamera();

    if(mStatus & preprocessed)
    {
#ifdef STEREO
        TriclopsError triclopsError;

        if(camera == cameraLeft)
        {
            triclopsError = triclopsGetImage(mTriclopsContext, TriImg_RECTIFIED, TriCam_LEFT, &mTriRectLeft);
            if(triclopsError != TriclopsErrorOk)
            {
                SPDLOG_ERROR(triclopsErrorToString(triclopsError));
                return cv::Mat(); // NOTE Fehlerbehandlung
            }

            mRectLeft = cv::Mat(cv::Size(mTriRectLeft.ncols, mTriRectLeft.nrows), CV_8UC1, mTriRectLeft.data);

            addStatus(rectified); // nicht set, da rectified image kann mehrmals abgefragt werden, wobei die disp schon
                                  // fertig gerechnet wurde


            //            IplImage* tempImg = cvLoadImage(D:/diss/personModel/ownPerson/movie/left_090.png, -1);
            //            // Is it created?
            //            if (tempImg == NULL)
            //                return NULL;
            //            // delete first old data
            //            //cvReleaseImage(&mImage); darf nicht freigegeben werden -> absturz, aber speicher wird
            //            woanders freigegeben, keine speicheranhaeufung
            //            // We create and copy the frame to our IplImage pointer
            //            cvReleaseImage(&mImage); // lieber vorher loeschen statt kopieren da ggf borderfilter bild
            //            groesser amcht?! mImage = cvCloneImage(tempImg);
            //            // naechsten beiden zeilen entsprechen oberer:
            //            //mImage = cvCreateImage(cvGetSize(mTempImg), 8, 3);
            //            //cvCopy(mTempImg, mImage, 0);
            //            // We release the temporary IplImage
            //            cvReleaseImage(&tempImg);


#ifdef TMP_STEREO_SEQ
            // temporaere code um stereo-einzelbildfolge einzulesen:
            // static IplImage* tempLeftImg = NULL;
            if(tempLeftImg != NULL)
                cvReleaseImage(&tempLeftImg);

            QString fn =
                QString(STEREO_SEQ_FILEBASE) +
                QString("left_%1.png")
                    .arg(
                        mAnimation->getCurrentFrameNum() % (STEREO_SEQ_LAST_FRAME - STEREO_SEQ_FIRST_FRAME + 1) +
                            STEREO_SEQ_FIRST_FRAME,
                        3,
                        10,
                        QChar('0')); // [2,159]
            // debout << fn << endl;
            return tempLeftImg = cvLoadImage((char *) fn.toAscii().data(), CV_LOAD_IMAGE_GRAYSCALE); //-1);
#endif


            return mRectLeft;
        }
        else if(camera == cameraRight)
        {
            triclopsError = triclopsGetImage(mTriclopsContext, TriImg_RECTIFIED, TriCam_RIGHT, &mTriRectRight);
            if(triclopsError != TriclopsErrorOk)
            {
                SPDLOG_ERROR(triclopsErrorToString(triclopsError));
                return cv::Mat(); // NOTE Fehlerbehandlung
            }
            mRectRight = cv::Mat(cv::Size(mTriRectRight.ncols, mTriRectRight.nrows), CV_8UC1, mTriRectRight.data);

            addStatus(rectified);


#ifdef TMP_STEREO_SEQ
            // temporaere code um stereo-einzelbildfolge einzulesen:
            // -----------------------------------------------------------------------------------------------------
            // static IplImage* tempImg = NULL;
            if(tempRightImg != NULL)
                cvReleaseImage(&tempRightImg);
            QString fn =
                QString(STEREO_SEQ_FILEBASE) +
                QString("right_%1.png")
                    .arg(
                        mAnimation->getCurrentFrameNum() % (STEREO_SEQ_LAST_FRAME - STEREO_SEQ_FIRST_FRAME + 1) +
                            STEREO_SEQ_FIRST_FRAME,
                        3,
                        10,
                        QChar('0')); // [2,159]
            // debout << fn << endl;
            return tempRightImg = cvLoadImage((char *) fn.toAscii().data(), CV_LOAD_IMAGE_GRAYSCALE); //-1);
#endif


#ifdef TIME_MEASUREMENT
            //        "==========: "
            debout << "ou rectify: " << getElapsedTime() << endl;
#endif


            return mRectRight;
        }
        else
            return cv::Mat(); // NOTE fehlerbehandlung
#endif
    }
    return cv::Mat();
}

//// A function to draw the histogram
// IplImage* DrawHistogram(CvHistogram *hist, float scaleX=1, float scaleY=1)
//{
//         // Find the maximum value of the histogram to scale
//         // other values accordingly
//         float histMax = 0;
//         cvGetMinMaxHistValue(hist, 0, &histMax, 0, 0);

//        // Create a new blank image based on scaleX and scaleY
//        IplImage* imgHist = cvCreateImage(cvSize(256*scaleX, 64*scaleY), 8 ,1);
//        cvZero(imgHist);

//        // Go through each bin
//        for(int i=0;i<255;++i)
//        {
//                float histValue = cvQueryHistValue_1D(hist, i);		// Get value for the current bin...
//                float nextValue = cvQueryHistValue_1D(hist, i+1);	// ... and the next bin

//                // Calculate the four points of the polygon that these two
//                // bins enclose
//                CvPoint pt1 = cvPoint(i*scaleX, 64*scaleY);
//                CvPoint pt2 = cvPoint(i*scaleX+scaleX, 64*scaleY);
//                CvPoint pt3 = cvPoint(i*scaleX+scaleX, (64-nextValue*64/histMax)*scaleY);
//                CvPoint pt4 = cvPoint(i*scaleX, (64-histValue*64/histMax)*scaleY);

//                // A close convex polygon
//                int numPts = 5;
//                CvPoint pts[] = {pt1, pt2, pt3, pt4, pt1};

//                // Draw it to the image
//                cvFillConvexPoly(imgHist, pts, numPts, cvScalar(255));
//        }

//        // Return it... make sure you delete it once you're done!
//        return imgHist;
//}


cv::Mat pet::StereoContext::getDisparity(bool *dispNew)
{
#ifdef TIME_MEASUREMENT
    //        "==========: "
    debout << "in    disp: " << getElapsedTime() << std::endl;
#endif

    if(dispNew != NULL)
        *dispNew = false;
    if((mStatus & preprocessed) && !(mStatus & genDisparity))
    {
#ifdef STEREO
        TriclopsError triclopsError;
#endif
        if(mMain->getStereoWidget()->minDisparity->value() >= mMain->getStereoWidget()->maxDisparity->value())
        {
            SPDLOG_ERROR("Invalid min/max settings for disparity (min>=max)!");
            return cv::Mat(); // NOTE ERROR HANDLING
        }

        // opencv disp BM block matching
        // ----------------------------------------------------------------------------------------------

        //        CvMat* BMdisparity = cvCreateMat(mRectRight.height, mRectRight.width, CV_32FC1); // CV_16S
        //        CvStereoBMState *BMState = cvCreateStereoBMState(CV_STEREO_BM_BASIC, 64); // 16; durch 16 teilbar
        //        if (BMState == NULL)
        //            debout << "Error: setting up the block matching state!"  << endl;
        ////        int edgeMaskSize = mMain->getStereoWidget()->edgeMaskSize->value()*4+1; // +1 wg ungerade
        ////        BMState->preFilterSize = (edgeMaskSize>255?255:(edgeMaskSize<5?5:edgeMaskSize));//19; 41
        ////        debout << BMState->preFilterSize << endl;
        ////        BMState->preFilterCap=19; 31
        //        BMState->minDisparity        = mMain->getStereoWidget()->minDisparity->value();
        //        BMState->numberOfDisparities =
        //        16*(1+(mMain->getStereoWidget()->maxDisparity->value()-mMain->getStereoWidget()->minDisparity->value())/16);
        //        // muss durch 16 teilbar sein BMState->SADWindowSize       =
        //        mMain->getStereoWidget()->stereoMaskSize->value()>=5?mMain->getStereoWidget()->stereoMaskSize->value():5;
        //        // minimum 5 erlaubt
        ////        BMState->textureThreshold=d; 10
        ////        BMState->uniquenessRatio=e; 15
        //        debout << BMState->numberOfDisparities<<endl;

        //        //Computes the disparity map using block matching algorithm.
        //        //    * disparity � The output single-channel 16-bit signed, or 32-bit floating-point disparity map of
        //        the same size as input images.
        //        //                  In the first case the computed disparities are represented as fixed-point numbers
        //        with 4 fractional bits
        //        //                  (i.e. the computed disparity values are multiplied by 16 and rounded to integers).
        //        //The function cvFindStereoCorrespondenceBM computes disparity map for the input rectified stereo
        //        pair.
        //        //Invalid pixels (for which disparity can not be computed) are set to state->minDisparity - 1 (or to
        //        (state->minDisparity-1)*16
        //        //in the case of 16-bit fixed-point disparity map)

        //        cvFindStereoCorrespondenceBM(getRectified(cameraLeft), getRectified(cameraRight), BMdisparity,
        //        BMState); // &mRectLeft, &mRectRight

        // opencv disp GC graph cut matching
        // ---------------------------------------------------------------------------------------------- folgende fkt
        // soll genauer sein, dauert aber sehr lange und hat keine subpixelgenauigkeit

        //        CvMat* GCdisparityLeft = cvCreateMat(mRectRight.height, mRectRight.width, CV_16S); // CV_32FC1
        //        CvMat* GCdisparityRight = cvCreateMat(mRectRight.height, mRectRight.width, CV_16S); // CV_32FC1
        //        CvStereoGCState* GCState = cvCreateStereoGCState(0, 2); // min 0, Iterationen 2

        //        GCState->minDisparity        = mMain->getStereoWidget()->minDisparity->value();
        //        GCState->numberOfDisparities =
        //        mMain->getStereoWidget()->maxDisparity->value()-mMain->getStereoWidget()->minDisparity->value();
        //        //16*(1+(mMain->getStereoWidget()->maxDisparity->value()-mMain->getStereoWidget()->minDisparity->value())/16);
        //        // muss durch 16 teilbar sein

        //        cvFindStereoCorrespondenceGC(getRectified(cameraLeft), getRectified(cameraRight), GCdisparityLeft,
        //        GCdisparityRight, GCState); // &mRectLeft, &mRectRight

        //        cvReleaseStereoGCState(&state);

        //        CvMat* GCdisparityLeft_visual = cvCreateMat(mRectRight.height, mRectRight.width, CV_8U);
        //        cvConvertScale(GCdisparityLeft, GCdisparityLeft_visual, -16);
        //        cvNamedWindow("CVdispGCleft", CV_WINDOW_AUTOSIZE);
        //        cvShowImage("CVdispGCleft", GCdisparityLeft_visual);

        //        CvMat* GCdisparityRight_visual = cvCreateMat(mRectRight.height, mRectRight.width, CV_8U);
        //        cvConvertScale(GCdisparityRight, GCdisparityRight_visual, 16);
        //        cvNamedWindow("CVdispGCright", CV_WINDOW_AUTOSIZE);
        //        cvShowImage("CVdispGCright", GCdisparityRight_visual);

        // minmax --------------------------

        //        float GCdisparityMin = FLT_MAX;
        //        float GCdisparityMax = 0;

        //        float* GCdata = (float*) GCdisparityRight->data.fl;//  .imageData;
        //        float* GCyData = data;
        //        int x,y;
        //        int noDispAnz=0;

        //        for (y = 0; y < GCdisparityRight->height; ++y)
        //        {
        //            for (x = 0; x < GCdisparityRight->width; ++x)
        //            {
        //                if (*data != (GCState->minDisparity - 1))
        //                {
        //                    if (*data > GCdisparityMax)
        //                        GCdisparityMax = *data;
        //                    else if (*data < GCdisparityMin)
        //                        GCdisparityMin = *data;
        //                }
        //                else
        //                    ++noDispAnz;
        //                ++data;
        //            }
        //            data = (yData += (GCdisparityRight->step/sizeof(float))); //width);
        //        }

        //        debout << noDispAnz << " " << GCdisparityMin << " " << GCdisparityMax <<endl;

        // bestimmung von min max
        // ---------------------------------------------------------------------------------------------------------------

        //            float BMdisparityMin = FLT_MAX;
        //            float BMdisparityMax = 0;

        //            float* data = (float*) BMdisparity->data.fl;//  .imageData;
        //            float* yData = data;
        //            int x,y;
        //            int noDispAnz=0;

        //            for (y = 0; y < BMdisparity->height; ++y)
        //            {
        //                for (x = 0; x < BMdisparity->width; ++x)
        //                {
        //                    if (*data != (BMState->minDisparity - 1))
        //                    {
        //                        if (*data > BMdisparityMax)
        //                            BMdisparityMax = *data;
        //                        else if (*data < BMdisparityMin)
        //                            BMdisparityMin = *data;
        //                    }
        //                    else
        //                        ++noDispAnz;
        //                    ++data;
        //                }
        //                data = (yData += (BMdisparity->step/sizeof(float))); //width);
        //            }

        //            debout << noDispAnz << " " << BMdisparityMin << " " << BMdisparityMax <<endl;

        //            double dMin, dMax;
        //            cvMinMaxLoc(BMdisparity, &dMin, &dMax);
        //            debout << (BMState->minDisparity - 1) << " " << dMin << " " << dMax <<endl;

        //            BMdisparityMin = mMain->getStereoWidget()->minDisparity->value();
        //            BMdisparityMax = mMain->getStereoWidget()->maxDisparity->value();
        //            debout << BMdisparityMin << " " << BMdisparityMax <<endl;

        // Ausgabe der opencv disp
        // ---------------------------------------------------------------------------------------------------------------

        //            IplImage* BMdisparityImg =
        //            cvCreateImage(cvSize(mRectRight.width,mRectRight.height),IPL_DEPTH_8U,3); unsigned char* dataImg =
        //            ((unsigned char*) BMdisparityImg->imageData); // char* unsigned char* yDataImg = dataImg; QColor
        //            color; int hue;

        //            yData = data = (float*) BMdisparity->data.fl;
        //            --dataImg;

        //            for (y = 0; y < BMdisparity->height; ++y)
        //            {
        //                for (x = 0; x < BMdisparity->width; ++x)
        //                {
        //                    if (*data != (BMState->minDisparity - 1))
        //                    {
        //                        hue = ((*data-BMdisparityMin)*240.)/(BMdisparityMax-BMdisparityMin);
        //                        if (hue<0 || hue>240)
        //                            color.setHsv(0, 0, 255, 255); // white for values outside min/max
        //                        else
        //                            color.setHsv(hue, 255, 255, 255); // hue<0?0:(hue>240?240:hue)
        //                    }
        //                    else
        //                        color.setHsv(0, 0, 0, 255); // black, if no value

        //                    ++data;

        //                    *(++dataImg) = color.red();
        //                    *(++dataImg) = color.green();
        //                    *(++dataImg) = color.blue();
        //                }
        //                data = (yData += (BMdisparity->step/sizeof(float))); //width);
        //                dataImg = (yDataImg += BMdisparityImg->widthStep/sizeof(char)); //width);
        //            }

        //            cvNamedWindow("CVdisp", CV_WINDOW_AUTOSIZE);
        //            cvShowImage("CVdisp", BMdisparityImg);


        //            // histogram
        //            ----------------------------------------------------------------------------------------------------------

        // http://opencv.willowgarage.com/documentation/c/histograms.html
        //            // let's quantize the hue to 30 levels
        //            // and the saturation to 32 levels
        //            int bins = 256; //, sbins = 32;
        //            int histSize[] = {bins}; //, sbins};
        //            //float range[] = { 0, 256 };
        //            //float* ranges[] = {range}; //s, sranges };
        //            CvHistogram* hist = cvCreateHist(1, histSize, CV_HIST_ARRAY); // ,ranges
        //            cvCalcHist((IplImage**) &BMdisparity, hist);

        ////            double maxVal=0;
        ////            minMaxLoc(hist, 0, &maxVal, 0, 0);

        //            cvNamedWindow("Histogram", CV_WINDOW_AUTOSIZE);
        //            cvShowImage("Histogram", DrawHistogram(hist, 1, 1)); // DrawHistogram ist auskommentierte fkt von
        //            oiben

        //            // freigabe
        //            ---------------------------------------------------------------------------------------------------

        //            cvReleaseImage(&BMdisparityImg);
        //            cvReleaseMat(&BMdisparity);
        //            cvReleaseStereoBMState(&BMState);

        //            //------------------------------------------------------------------------------------------------------------------------

        if(mMain->getStereoWidget()->stereoDispAlgo->currentIndex() ==
           0) // ptGrey -------------------------------------------------------
        {
            //// Description: This structure is used for image output from the Triclops
            ////   system for image types that require 16-bits per pixel.  This is the format
            ////   for subpixel interpolated images.   The structure is identical to the
            ////   TriclopsImage structure except that the data contains unsigned shorts
            ////   rather than unsigned chars.  Rowinc is still the number of bytes between
            ////   the beginning of a row and the beginning of the following row
            ////   (NOT number of pixels).
            ////
            // typedef struct TriclopsImage16
            //{
            //    // The number of rows in the image.
            //    int 		     nrows;
            //    // The number of columns in the image.
            //    int 		     ncols;
            //    // The number row increment of the image.
            //    int		     rowinc;
            //    // The pixel data of the image.
            //    unsigned short*   data;
            //
            // } TriclopsImage16;
#ifdef STEREO

            // set values for stereo processing
            triclopsError = triclopsSetDisparity(
                mTriclopsContext,
                mMain->getStereoWidget()->minDisparity->value(),
                mMain->getStereoWidget()->maxDisparity->value()); // ,100// 22, 66); // set disparity range 14..36 bei
                                                                  // mu oder mo  untere kamera
            if(triclopsError != TriclopsErrorOk)
                SPDLOG_ERROR("{} for the disparity range", triclopsErrorToString(triclopsError));

            triclopsError = triclopsSetStereoMask(mTriclopsContext, mMain->getStereoWidget()->stereoMaskSize->value());
            if(triclopsError != TriclopsErrorOk)
                SPDLOG_ERROR("{} for the stereo mask size!", triclopsErrorToString(triclopsError));

            // Do stereo processing
            triclopsError = triclopsStereo(mTriclopsContext);
            if(triclopsError != TriclopsErrorOk)
            {
                SPDLOG_ERROR(triclopsErrorToString(triclopsError));
                return cv::Mat(); // NOTE Fehlerbehandlung
            }

            triclopsError = triclopsGetImage16(mTriclopsContext, TriImg16_DISPARITY, TriCam_REFERENCE, &mTriDisparity);
            if(triclopsError != TriclopsErrorOk)
            {
                SPDLOG_ERROR(triclopsErrorToString(triclopsError));
                return cv::Mat(); // NOTE Fehlerbehandlung
            }

            mDisparity.data = (uchar *) mTriDisparity.data;
#endif
        }
        else if(
            mMain->getStereoWidget()->stereoDispAlgo->currentIndex() ==
            1) // openCV Block Matching
               // ----------------------------------------------------------------------------------------------------------------------------
        {
            // Bestimmung CV_16S und Ausgabe der opencv disp

            if(!mBMState) // noch =NULL
            {
                mBMState = cv::StereoBM::create(64); // durch 16 teilbar
                if(mBMState == nullptr)
                    SPDLOG_ERROR("setting up the block matching state!");
            }
            //        int edgeMaskSize = mMain->getStereoWidget()->edgeMaskSize->value()*4+1; // +1 wg ungerade
            //        mBMState->preFilterSize = (edgeMaskSize>255?255:(edgeMaskSize<5?5:edgeMaskSize));//19; 41
            //        debout << mBMState->preFilterSize << endl;
            //        mBMState->preFilterCap=19; 31


            int blockSize = mMain->getStereoWidget()->stereoMaskSize->value();
            blockSize     = myClip(blockSize, 5, 21);
            mBMState->setBlockSize(
                blockSize); // minimum 5 erlaubt, ab 21 wird zuviel als mindens und maxdens angezeigt!
            //        mBMState->textureThreshold=d; 10
            //        mBMState->uniquenessRatio=e; 15

            // umkehrung der disparity, da links und rechts vertauscht werden musste, damit disp fuer rechtes bild
            // berechnet wird
            mBMState->setMinDisparity(
                -(mMain->getStereoWidget()->maxDisparity->value()) +
                1); // +1, weil es dann mit ptgrey fuer min besser passt
            mBMState->setNumDisparities(
                16 *
                ((mMain->getStereoWidget()->maxDisparity->value() - mMain->getStereoWidget()->minDisparity->value()) /
                 16));                             // muss durch 16 teilbar sein
            if(mBMState->getNumDisparities() < 16) // mind 16 ist vorgeschrieben
                mBMState->setNumDisparities(16);

            if(mBMdisparity16.empty())
            {
                mBMdisparity16 = cv::Mat(
                    mDisparity.size(),
                    CV_16S); // mRectRight.height, mRectRight.width gehtz nicht da ggf noch nicht vorliegend
            }

            // folgendes kann nicht genutzt werden um rechenzeiut zu sparen, da rectified nach erstem durchlauf dieser
            // fkt auch mit ptgrey gesetzt wird, dabei aber nicht mrectleft gesetzt wird
            //            if (mStatus & rectified)
            //                cvFindStereoCorrespondenceBM(&mRectRight, &mRectLeft, mBMdisparity16, mBMState);
            //            else
            //
            //            cv::StereoBM::compute(getRectified(cameraRight),getRectified(cameraLeft),mBMdisparity16);

            mBMState->compute(getRectified(cameraLeft), getRectified(cameraRight), mBMdisparity16);

            //            cvMinMaxLoc(mBMdisparity16, &dMin, &dMax);
            //            debout << (mBMState->minDisparity-1)*16 << " " << dMin << " " << dMax <<endl;

            //            CvMat* BMdisparity_visual = cvCreateMat(mRectRight.height, mRectRight.width, CV_8U);
            //            cvConvertScale(mBMdisparity16, BMdisparity_visual, 255/dMax);//,
            //            255/((mBMState->minDisparity+mBMState->numberOfDisparities)*16));//, 255/dMax);
            //            //255/(dMax-mBMState->minDisparity*16), -mBMState->minDisparity); // scale, shift
            //            // weiss sind ausreisser nach oben
            //            // mittlerer Grauwert zeigt an, dass nichts gefunden wurde
            //            // schwarz ist 0 und liegt untzer minDisparity
            //            cvMinMaxLoc(BMdisparity_visual, &dMin, &dMax);
            //            debout << dMin << " " << dMax <<endl;
            //            cvNamedWindow("CVdispBM16", CV_WINDOW_AUTOSIZE);
            //            cvShowImage("CVdispBM16", BMdisparity_visual);

            // exchange/replace value in mBMdisparity16 so that the error value is the same like in pointgrey
            short *data16  = (short *) mBMdisparity16.data;
            short *yData16 = data16;

            //            int noDispAnz1 = 0, noDispAnz2 = 0, noDispAnz3 = 0;
            //            double dMin, dMax;
            //            cvMinMaxLoc(mBMdisparity16, &dMin, &dMax);
            //            debout << endl << mBMState->minDisparity << " " <<
            //            mBMState->minDisparity+mBMState->numberOfDisparities <<endl; debout << dMin << " " << dMax
            //            <<endl;

            for(int y = 0; y < mBMdisparity16.rows; ++y)
            {
                for(int x = 0; x < mBMdisparity16.cols; ++x)
                {
                    // if ((*data16 < (mBMState->minDisparity)*16) || (*data16 >
                    // (mBMState->minDisparity+mBMState->numberOfDisparities)*16)) // zeigt genau den Bereich, der auch
                    // algo uebergeben wurde
                    if((-*data16 < (mMain->getStereoWidget()->minDisparity->value()) * 16) ||
                       (-*data16 > (mMain->getStereoWidget()->maxDisparity->value()) *
                                       16) || // laesst nur den Teil ueber, der in gui eingestellt wurde
                       (*data16 <= (mBMState->getMinDisparity()) *
                                       16)) // enthaelt auch: (*data16 == (mBMState->minDisparity-1)*16)) // marker fuer
                                            // nicht berechneten wert
                    {
                        //                        if (*data16 == (mBMState->minDisparity-1)*16)
                        //                            ++noDispAnz1;
                        //                        else
                        //                            ++noDispAnz2;
                        *data16 = static_cast<short>(0xFF00); // fehlercode gemaess ptgrey
                    }
                    //                    if (*data16 == (mBMState->minDisparity-1)*16) // eigentlicher fehler-wert,
                    //                    wenn nichts erkannt wird; wird durch vorherige abfrage mit geloescht
                    //                    {
                    ////                        ++noDispAnz;
                    //                        *data16 = 0xFF00;
                    //                    }
                    //                    else if (*data16 > 0) // passiert nicht mehr
                    //                    {
                    //                        *data16 = 0; //*data16+0x8000; // da signed <-> unsigned: zB bei char:
                    //                        -127 und 255 gleiche bitrepraesentation
                    ////                        ++noDispAnz3;
                    //                    }
                    else
                        // vorzeichen umkehren und
                        // disparityFloat = (float) disparity16 / 256.0; //16-bit in triclops umrechung
                        *data16 = -*data16 *
                                  16; // *16, da cv disp nur faktor 16 fuer subpixelgenauigkeit hat und triclops *256
                    //                    else
                    //                        *data16 = (unsigned int) *data16;
                    ++data16;
                }
                data16 = (yData16 += (mBMdisparity16.step1())); // width);
            }

            //            debout << noDispAnz1 << " " << noDispAnz2 << " " << noDispAnz3 << " " <<endl;
            //            cvMinMaxLoc(mBMdisparity16, &dMin, &dMax);
            //            debout << dMin << " " << dMax <<endl;

            //            unsigned short int uu; short int ss;
            //            uu = 0x8000; ss = 0x8000;
            //            debout << uu << " " << ss << endl;

            mDisparity.data = mBMdisparity16.data;
        }
        else if(
            mMain->getStereoWidget()->stereoDispAlgo->currentIndex() ==
            2) // openCV semi-global block matching
               // http://opencv.willowgarage.com/documentation/cpp/camera_calibration_and_3d_reconstruction.html#stereosgbm
               // ----------------------------------------------------------------------------------------------------------------------------
        {
            int SADWindowSize = 11;
            int nDisparities =
                (mMain->getStereoWidget()->maxDisparity->value() - mMain->getStereoWidget()->minDisparity->value()) /
                16;
            // mSgbm = StereoSGBM::create(nDisparities, SADWindowSize);
            mSgbm = StereoSGBM::create(
                0,                                       // minDisparity
                16 * nDisparities,                       // numDisparities
                11,                                      // blockSize
                8 * SADWindowSize * SADWindowSize,       // P1
                4 * (8 * SADWindowSize * SADWindowSize), // P2
                1,                                       // disp12MaxDiff
                63,                                      // preFilterCap
                15,                                      // uniquenessRatio
                3,                                       // speckleWindowSize
                3,                                       // speckleRange
                StereoSGBM::MODE_SGBM                    // mode
            );

            if(mBMdisparity16.empty())
            {
                mBMdisparity16 = cv::Mat(mDisparity.size(), CV_16S);
            }


            cv::Mat MR(getRectified(cameraRight));
            cv::Mat ML(getRectified(cameraLeft));
            mSgbm->compute(ML, MR, mBMdisparity16);

            // exchange/replace value in mBMdisparity16 so that the error value is the same like in pointgrey
            short *data16  = (short *) mBMdisparity16.data;
            short *yData16 = data16;

            for(int y = 0; y < mBMdisparity16.rows; ++y)
            {
                for(int x = 0; x < mBMdisparity16.cols; ++x)
                {
                    if((*data16 < (mMain->getStereoWidget()->minDisparity->value()) * 16) ||
                       (*data16 > (mMain->getStereoWidget()->maxDisparity->value()) *
                                      16) || // laesst nur den Teil ueber, der in gui eingestellt wurde

                       (*data16 <= (-(mMain->getStereoWidget()->maxDisparity->value()) + 1) * 16))
                    {
                        *data16 = static_cast<short>(0xFF00); // fehlercode gemaess ptgrey
                    }
                    else // vorzeichen umkehren und
                    {
                        if(*data16 * 16 > SHRT_MAX)
                        {
                            *data16 = SHRT_MAX;
                            SPDLOG_WARN("Disparity set to SHRT_MAX for Pixel {}/{}", x, y);
                        }
                        else
                            *data16 =
                                *data16 *
                                16; // *16, da cv disp nur faktor 16 fuer subpixelgenauigkeit hat und triclops *256
                    }
                    ++data16;
                }
                data16 = (yData16 += (mBMdisparity16.step1()));
            }

            mDisparity = mBMdisparity16;
        }

        setStatus(genDisparity);

        if(mMin == USHRT_MAX) // das allererste Mal
            calcMinMax();

        if(dispNew != nullptr)
            *dispNew = true;
        mMain->getStereoItem()->setDispNew(true); // das zu zeichnende Bild neu berechnen


#ifdef TMP_STEREO_SEQ_DISP
        // temporaere code um hoehe [0..ca.176] stereo-einzelbildfolge einzulesen:
        // ----------------------------------------------------------------------------------------------------- hier
        // nur um farbbild als disp in petrack anzeigen zu koennen, ist nicht passendes disp bild
        QString fn = QString(STEREO_SEQ_FILEBASE) +
                     QString("pointCloud_%1.hgt")
                         .arg(
                             mAnimation->getCurrentFrameNum() % (STEREO_SEQ_LAST_FRAME - STEREO_SEQ_FIRST_FRAME + 1) +
                                 STEREO_SEQ_FIRST_FRAME,
                             3,
                             10,
                             QChar('0')); // [2,159]
        debout << "Read height field for disparity image" << fn << endl;
        QFile file(fn);
        if(!file.open(QIODevice::ReadOnly))
        {
            PInformation(NULL, "show", "Failed to read");
        }
        QByteArray buffer = file.readAll(); // reinterpret_cast<double *> mDisparity.height*mDisparity.width*8
        // debout << "Number of bytes in height field from virtual scene: " << buffer.size() <<endl;
        double *dBuffer = (double *) buffer.data();
        file.close();

        if(!mBMdisparity16)
        {
            mBMdisparity16 = cvCreateMat(mDisparity.height, mDisparity.width, CV_16S);
            if(mBMdisparity16 == NULL)
                debout << "Error: create matrix for libelas disparity map!" << endl;
        }
        short *data16  = (short *) mBMdisparity16->data.s;
        short *yData16 = data16;
        int    i, j;
        double disp;

        for(i = mDisparity.height - 1; i >= 0; --i) // 960 // dateiinhalt liegt in y gespiegelt vor
        // for (i = 0; i < mDisparity.height; ++i) // 960
        {
            for(j = 0; j < mDisparity.width; ++j) // 1280
            {
                disp = dBuffer[3 * (j + i * mDisparity.width) + 2];
                if(disp > -0.9)
                    *data16 = SHRT_MAX * disp /
                              STEREO_SEQ_MAX_PERS_HEIGHT; // von maximal STEREO_SEQ_MAX_PERS_HEIGHT cm personengroesse
                                                          // wird ausgegangen // dateiinhalt liegt in y gespiegelt vor
                else
                    *data16 = 0xFFFF; // erlaubte werte: disp < 0xFF00 xxx
                ++data16;
            }
            data16 = (yData16 += (mBMdisparity16->step / sizeof(short)));
        }
        mDisparity.imageData = (char *) mBMdisparity16->data.ptr;
        mMin                 = 0;
        mMax                 = SHRT_MAX;
#endif


#ifdef TIME_MEASUREMENT
        //        "==========: "
        debout << "out   disp: " << getElapsedTime() << endl;
#endif


        return mDisparity;
    }
    else if(mStatus & genDisparity)
    {
        return mDisparity;
    }
    else
        return cv::Mat(); // NOTE Error Handling
}

// gibt die cm pro pixel in der entfernung von z Metern von der Kamera zurueck
double pet::StereoContext::getCmPerPixel([[maybe_unused]] float z)
{
#ifdef STEREO

    float row1, row2;
    float col;
    float disp;

    // col ist fuer gleiches x identisch
    // exemplarisch wird anzahl an pixel zwischen (1m,1m) bis (1m,2m) bestimmt
    triclopsXYZToRCD(mTriclopsContext, 1., 1., z, &row1, &col, &disp);
    triclopsXYZToRCD(mTriclopsContext, 1., 2., z, &row2, &col, &disp);
    return 100. / (row2 - row1);
#else
    return -1;
#endif
}

void pet::StereoContext::calcMinMax()
{
    if(mStatus & genDisparity)
    {
        mMin = USHRT_MAX;
        mMax = 0;

        unsigned short *data  = (unsigned short *) mDisparity.data;
        unsigned short *yData = data;
        int             x, y;

        for(y = 0; y < mDisparity.rows; ++y)
        {
            for(x = 0; x < mDisparity.cols; ++x)
            {
                if(dispValueValid(*data))
                {
                    if(*data > mMax)
                        mMax = *data;
                    else if(*data < mMin)
                        mMin = *data;
                }
                ++data;
            }
            data = (yData += mDisparity.step1());
        }
    }
    // debout << "Set disparity color code min: " << mMin << ", max: " << mMax << endl;
}

void pet::StereoContext::indicateNewValues()
{
    setStatus(rectified);
    getDisparity();
    calcMinMax();
}

// col, row beginnt bei 0, 0
// x, y, z in cm
bool pet::StereoContext::getXYZ(int col, int row, float *x, float *y, float *z)
{
    if(mStatus & genDisparity)
    {
        unsigned short int disp = *(((unsigned short *) mDisparity.data) + mDisparity.step1() * row + col);

        if(dispValueValid(disp))
        {
#ifdef STEREO
            triclopsRCD16ToXYZ(mTriclopsContext, row, col, disp, x, y, z);
#endif
            *x *= 100.;
            *y *= 100.;
            *z *= 100.;
            return true;
        }
        else
        {
            return false;
        }
    }
    else
        return false;
}

// liefert zu einer Disparit�t die Entfernung in cm
// die Entfernung ist fuer jedes Pixel gleich (hier 0,0 genommen)
float pet::StereoContext::getZfromDisp([[maybe_unused]] unsigned short int disp)
{
#ifdef STEREO
    float x;
    float y;
    float z;

    triclopsRCD16ToXYZ(mTriclopsContext, 0, 0, disp, &x, &y, &z);
    return z * 100.; // in cm
#else
    return -1;
#endif
}

// col, row beginnt bei 0, 0
// x, y, z in cm (median der aus disp-werten berechneten cm in 5x5 pixelumfeld)
// return false, if no disparity information is found around col/row (x,y,z not set)
// Achtung: COL ROW getauscht zu triclopsRCD16ToXYZ
bool pet::StereoContext::getMedianXYZaround(int col, int row, float *x, float *y, float *z)
{
    if(mStatus & genDisparity)
    {
        static int                     size = 5; // region scanned for median
        static QVector<unsigned short> values(size * size);

        values.fill(65280); // 65280 == 0xff00 ab wo angezeigt wird, dass es fehler sind

        // umstellung von row und col von mitte des 5x5x auf ecke oben links und ueberpruefung auf rand
        row = ((row < 2) ? 0 : ((row > mDisparity.rows - 3) ? mDisparity.rows - 5 : row - 2));
        col = ((col < 2) ? 0 : ((col > mDisparity.cols - 3) ? mDisparity.cols - 5 : col - 2));

        unsigned short *data =
            (unsigned short
                 *) (((unsigned short *) mDisparity.data) + mDisparity.step1() * row + col); // mDisparity.imageData;
        unsigned short *yData = data;
        int             i, j, nr = 0; // nr zeigt anzahl der gefundenen disp an

        for(j = 0; j < size; ++j)
        {
            for(i = 0; i < size; ++i)
            {
                if(dispValueValid(*data))
                {
                    values[nr++] = *data;
                }
                ++data;
            }
            data = (yData += mDisparity.step1());
        }

        if(nr != 0)
        {
            std::sort(values.begin(), values.begin() + nr); // sort first nr elements in values

            // debout << row+2 << " " << col+2 << " " << nr << " "<< values[nr/2] << endl;
#ifdef STEREO
            triclopsRCD16ToXYZ(mTriclopsContext, row + 2, col + 2, values[nr / 2], x, y, z);
#endif
            *x *= 100.;
            *y *= 100.;
            *z *= 100.;
            return true;
        }
        else
            return false;
    }
    else
        return false;
}

bool pet::StereoContext::dispValueValid(unsigned short int disp)
{
    if(disp < 0xFF00)
        // if ((disp != mSurfaceValue+65280) && (disp != mBackForthValue+65280))// 65280 == 0xff00
        return true;
    else
        return false;
}

cv::Mat pet::StereoContext::getPointCloud()
{
    if(!(mStatus & genDisparity)) // falls disparity noch nicht berechnet wurde
        getDisparity();

    if(mStatus & genDisparity)
    {
#ifdef STEREO

        if(mPointCloud.empty()) // Speicherplatz anlegen
        {
            mPointCloud = cv::Mat(mDisparity.rows, mDisparity.cols, CV_32FC3);
            // NOTE Speicherueberpruefung ggf. auch hier durchfuehren (Exception fangen)
        }

        unsigned short *data   = (unsigned short *) mDisparity.data;
        float          *pcData = (float *) mPointCloud.data;
        // float* pcyData = pcData;
        // float x, y, z;
        int i, j;

        for(i = 0; i < mDisparity.rows; ++i)
        {
            data   = mDisparity.ptr<ushort>(i);
            pcData = mPointCloud.ptr<float>(i);
            for(j = 0; j < mDisparity.cols; ++j)
            {
                if(dispValueValid(*data))
                {
                    // convert the 16 bit disparity value to floating point x,y,z
                    triclopsRCD16ToXYZ(mTriclopsContext, i, j, *data, pcData, pcData + 1, pcData + 2);
                }
                else
                {
                    mPointCloud.at<cv::Vec3f>(i, j) = {-1, -1, -1};
                }
                ++data;
                pcData += 3;
            }
        }
        //        debout << mDisparity.width << " " << mDisparity.widthStep << endl;
        //        debout << mPointCloud->width << " " << mPointCloud->step << endl;


#ifdef TMP_STEREO_SEQ_DISP
        // temporaere code um hoehe [0..ca.176] stereo-einzelbildfolge einzulesen:
        // -----------------------------------------------------------------------------------------------------
        QString fn = QString(STEREO_SEQ_FILEBASE) +
                     QString("pointCloud_%1.hgt")
                         .arg(
                             mAnimation->getCurrentFrameNum() % (STEREO_SEQ_LAST_FRAME - STEREO_SEQ_FIRST_FRAME + 1) +
                                 STEREO_SEQ_FIRST_FRAME,
                             3,
                             10,
                             QChar('0')); // [2,159]
        debout << "Import for pointcloud: " << fn << endl;
        QFile file(fn);
        if(!file.open(QIODevice::ReadOnly))
        {
            PInformation(NULL, "show", "Failed to read");
        }
        QByteArray buffer = file.readAll(); // reinterpret_cast<double *> mDisparity.height*mDisparity.width*8
        // debout << "Number of bytes in height field from virtual scene: " << buffer.size() <<endl;
        double *dBuffer = (double *) buffer.data();
        file.close();

        data    = (unsigned short *) mDisparity.imageData;
        yData   = data;
        pcData  = (float *) mPointCloud->data.fl;
        pcyData = pcData;
        for(i = mDisparity.height - 1; i >= 0; --i) // 960 // dateiinhalt liegt in y gespiegelt vor
        {
            for(j = 0; j < mDisparity.width; ++j) // 1280
            {
                if(*data != -1)
                {
                    pcData[0] = .01 * dBuffer[3 * (j + i * mDisparity.width)];
                    pcData[1] = .01 * dBuffer[3 * (j + i * mDisparity.width) + 1];
                    pcData[2] =
                        .01 * (784 - dBuffer[3 * (j + i * mDisparity.width) + 2]); // entfernung von kamera ueber
                                                                                   // hoehenfeld der person
                }
                else
                    pcData[0] = pcData[1] = pcData[2] = -1;
                ++data;
                pcData += 3;
                // if (dBuffer[j+i*mDisparity.width] > 0)
            }
            data   = (yData += mDisparity.width);
            pcData = (pcyData += mPointCloud->step / sizeof(float));
        }
#endif // TMP_STEREO_SEQ_DISP
#endif // STEREO

        return mPointCloud;
    }
    else
        return cv::Mat(); // NOTE Error Handling


    //    else // kann nun nicht mehr passieren, da disp berechnet wird, wenn nichjt schon geschehen
    //    {
    //        debout << "Warning: generate disparity before pointCloud!" << endl;
    //        return NULL;
    //    }
}

//// erzeugt ein aequidistantes xy-gitter
//// von min bis max (dadurch koennten ausreisser das netz unnoetig auseinanderziehen und viele pkte fallen zusammen)
// CvMat* StereoContext::getRectifiedPointCloud(float *xMin, float *xMax, float *yMin, float *yMax, float *zMin, float
// *zMax)
//{
//     if (!(mStatus & genDisparity) || (mPointCloud == NULL))
//         getPointCloud();

//    if (mPointCloud != NULL)
//    {
//        if (mRectPointCloud == NULL) // Speicherplatz anlegen
//        {
//            mRectPointCloud = cvCreateMat(mDisparity.height, mDisparity.width, CV_32FC3); // CvMat*
//            if (mRectPointCloud == NULL)
//            {
//                debout << "Error: not enaugh memory for rectified pointCloud" << endl;
//                return NULL;
//            }
//        }

////        unsigned short* data = (unsigned short*) mDisparity.imageData;
////        unsigned short* yData = data;
////        float* pcData = (float*) mPointCloud->data.fl;
////        float* pcyData = pcData;
////        int i, j;

////        for (i = 0; i < mDisparity.height; ++i)
////        {
////            for (j = 0; j < mDisparity.width; ++j)
////            {
////                if (dispValueValid(*data))
////                {
////                    // convert the 16 bit disparity value to floating point x,y,z
////                    triclopsRCD16ToXYZ(mTriclopsContext, i, j, *data, pcData, pcData+1, pcData+2) ;

////                }
////                else
////                {
////                    pcData[0] = pcData[1] = pcData[2] = -1;
////                }
////                ++data;
////                pcData+=3;
////            }
////            data = (yData += mDisparity.width);
////            pcData = (pcyData += mPointCloud->step/sizeof(float));//  ->step);
////        }
//    }
//}


// return shows, if export was sucessfull
bool pet::StereoContext::exportPointCloud([[maybe_unused]] QString dest) // default = ""
{
#ifndef STEREO
    PCritical(nullptr, "No stereo support", "This version of PeTrack was compiled without stereo support.");
    return false;
#else
    static QString lastFile = "";

    if(mStatus & genDisparity)
    {
        // if no destination file or folder is given
        if(dest.isEmpty())
        {
            if(lastFile.isEmpty() && !mMain->getProFileName().isEmpty())
                lastFile = QFileInfo(mMain->getProFileName()).path() + QDir::separator() +
                           QFileInfo(mMain->getProFileName()).baseName() + ".pts";

            dest = QFileDialog::getSaveFileName(
                mMain,
                QObject::tr("Select file for exporting point cloud"),
                lastFile,
                QObject::tr("Triclops points (*.pts);;All supported types (*.pts *.xxx);;All files (*.*)"));
        }

        if(!dest.isEmpty())
        {
            if(dest.right(4) == ".pts")
            {
                QFile file(dest);

                if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
                {
                    PCritical(
                        mMain,
                        QObject::tr("Petrack"),
                        QObject::tr("Cannot open %1:\n%2.").arg(dest).arg(file.errorString()));
                    return false;
                }

                SPDLOG_INFO("export point cloud to {}...", dest.toStdString());
                QTextStream out(&file);

                unsigned short *data  = (unsigned short *) mDisparity.data;
                unsigned short *yData = data;
                float           x, y, z;
                int             i, j, k = 0;
                int             nPoints = 0;
                unsigned char   c;
                unsigned char  *iD = (unsigned char *) getRectified(cameraRight).data;

                for(i = 0; i < mDisparity.rows; ++i)
                {
                    for(j = 0; j < mDisparity.cols; ++j)
                    {
                        if(dispValueValid(*data) && ((mMain->getBackgroundFilter()->getEnabled() &&
                                                      mMain->getBackgroundFilter()->isForeground(j, i)) ||
                                                     !mMain->getBackgroundFilter()->getEnabled()))
                        {
                            // convert the 16 bit disparity value to floating point x,y,z
                            triclopsRCD16ToXYZ(mTriclopsContext, i, j, *data, &x, &y, &z);
                            // look at points within a range
                            // if ( z < 5.0 )
                            c = iD[k];
                            out << x << " " << y << " " << z << " " << c << " " << c << " " << c << Qt::endl;
                            ++nPoints;
                        }
                        ++data;
                        ++k;
                    }
                    data = (yData += mDisparity.step1());
                }


                //                int i, j, k, pixelinc = mTriDisparity.rowinc/2;
                //                unsigned short *row;
                //                unsigned short disp;
                //                float x, y, z;
                //                int nPoints = 0;

                //                for (i = 0, k = 0; i < mTriDisparity.nrows; ++i)
                //                {
                //                    row = mTriDisparity.data + i * pixelinc;
                //                    for (j = 0; j < mTriDisparity.ncols; ++j, ++k)
                //                    {
                //                        disp = row[j];
                //                        // filter invalid points
                //                        if (dispValueValid(disp))
                //                        {
                //                            // convert the 16 bit disparity value to floating point x,y,z
                //                            triclopsRCD16ToXYZ(mTriclopsContext, i, j, disp, &x, &y, &z) ;
                //                            // look at points within a range
                //                            //if ( z < 5.0 )
                //                            out << x << " " << y << " " << z << " " << mTriRectRight.data[k] << " " <<
                //                            mTriRectRight.data[k] << " " << mTriRectRight.data[k] << endl;
                //                            ++nPoints;
                //                        }
                //                    }
                //                }

                file.close();
                std::cout << " with " << nPoints << " points finished " << std::endl;
            }
            else
            {
                PCritical(
                    mMain,
                    QObject::tr("Petrack"),
                    QObject::tr("Cannot save %1 maybe because of wrong file extension.").arg(dest));
                return false;
            }
            lastFile = dest;
        }
        return true;
    }
    else
    {
        PCritical(
            mMain,
            QObject::tr("Petrack"),
            QObject::tr("Cannot export point cloud, because disparity has not been generated."));
        return false;
    }
#endif // STEREO defined
}
