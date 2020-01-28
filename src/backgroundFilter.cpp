#include "backgroundFilter.h"
#include "stereoContext.h"

// nur temporaer fuer anzeige
#include "highgui.hpp"
#include "helper.h"

// spaeter entfernen naechsten beiden zeilen
#include "control.h"
extern Control *cw;

// Entfernung zum Hintergrund, um als Vordergrund angesehen zu werden
// darf nicht zu gross werden, da sonst an waenden problem
// z-Wert in m (nicht cm!)
//wenn z-wert 1m unter defaultgroesse
//#define FOREGROUND_DISTANCE ((mDefaultHeight/100.-1.0) > 0. ? (mDefaultHeight/100.-1.0): 0.)
#define FOREGROUND_DISTANCE 0.4

// kleine Bereiche werden eliminiert, dies koennte in Abhaengigkeit von der disparity gemacht werden!!!!!!!!
// Anzahl der Pixel, die ein Vordergrund aufweisen muss
#define MIN_FOREGROUND_AREA -1000 // war:-400

//#define SHOW_TMP_IMG

using namespace::cv;

BackgroundFilter::BackgroundFilter()
    :Filter()
{
    setOnCopy(false); // da img nur ausgelesen, aber bnicht veraendert wird kann auf eine kopie verzichtet werden (true ist default bei filter)


//    mBgModel = createBackgroundSubtractorMOG2().dynamicCast<BackgroundSubtractor>();

//    mBgModel = method == "knn" ? createBackgroundSubtractorKNN().dynamicCast<BackgroundSubstractor>() :
//                                 createBackgroundSubtractorMOG2().dynamicCast<BackgroundSubstractor>();//.dynamicCast<BackgroundSubtractorMOG2>();
//    mBgPointCloud = NULL;
//    mForeground = NULL;
    mLastFile = "";
}

BackgroundFilter::~BackgroundFilter()
{

}


Mat BackgroundFilter::getForeground() // nutzen, wenn ueber ganzes bild foreground benutzt wird; NULL, wenn keine background subtraction aktiviert
{

    return mForeground;
}

// i entlang zeile, j entlang spalte
// zur Effizientssteigerung ohne ueberpruefung der grenzen!!!
bool BackgroundFilter::isForeground(int i, int j) // nutzen, wenn einzelne pixel abgefraget werden
{
    if (!mForeground.empty()) // && i >= 0 && i < mForeground->width && j >= 0 && j < mForeground->height)
        return (bool) mForeground.data[j*mForeground.cols+i]; // 0 background, 1 foreground
    else
        return false;
}

// zuruecksetzen, wenn zB helligkeit veraendert wird oder schaerfe
void BackgroundFilter::reset()
{
    // funktioniert nicht wirklich
//    if (!mBgModel.empty())
    if (!mForeground.empty()) mForeground = Scalar::all(0);
//      mBgModel = createBackgroundSubtractorMOG2();//->initialize(mForeground.size, mForeground.type());
    if (!mBgModel.empty()) mBgModel->clear();

    setChanged(true);

}

QString BackgroundFilter::getFilename()
{
    return mLastFile;
}
void BackgroundFilter::setFilename(const QString &fn)
{
    mLastFile = fn;
}

// rueckgabe, ob speichern geklappt hat
bool BackgroundFilter::save(QString dest) //default = ""
{
#ifndef STEREO_DISABLED
    if (*stereoContext() && !mBgPointCloud.empty())
    {
        // if no destination file or folder is given
        if (dest.isEmpty())
        {
            dest = QFileDialog::getSaveFileName(NULL, "Select file for background subtraction in stereo mode", mLastFile,
                                                "Background picture (*.png);;All files (*.*)");
        }
        if (!dest.isEmpty())
        {
            if (dest.right(4) != ".png")
                dest = dest + ".png";

            Mat bgImg(Size(mBgPointCloud.cols,mBgPointCloud.rows),CV_8UC1);
//            IplImage *bgImg = cvCreateImage(cvSize(mBgPointCloud->width, mBgPointCloud->height), IPL_DEPTH_8U, 1); // CV_8UC1 8, 1

            // minimum der z-Werte bestimmen
            float* data = (float*) mBgPointCloud.data;
            float* yData = data;
            int x,y;
            float min = FLT_MAX;
            float max = FLT_MIN;

            for (y = 0; y < mBgPointCloud.rows; ++y)
            {
                for (x = 0; x < mBgPointCloud.cols; ++x)
                {
                    if (data[2] != -1)
                    {
                        if (data[2] > max)
                            max = data[2];
                        else if (data[2] < min)
                            min = data[2];
                    }
                    data+=3;
                }
                data = (yData += mBgPointCloud.cols/sizeof(float));
            }

            // uebetragen der z-werte in 8-bit-bild ---------------------------------------------------------
            // quantisierung fuehrt dazu, dass bei 10m maximaler spannweite pro grauwertstufe 4cm
            data = (float*) mBgPointCloud.data;
            yData = data;
            float scale = (255-1)/(max-min); // 10 wird als unterster Wert genommen, damit 0 fuer ungueltigen wert stehen kann
            unsigned char* bgImgData = (unsigned char*) bgImg.data;
            unsigned char* ybgImgData = bgImgData;

            for (y = 0; y < mBgPointCloud.rows; ++y)
            {
                for (x = 0; x < mBgPointCloud.cols; ++x)
                {
                    if (data[2] != -1)
                        *bgImgData = 1+scale*(data[2]-min);
                    else // schwarz bedeutet keine Info
                        *bgImgData = 0;
                    data+=3;
                    ++bgImgData;
                }
                data = (yData += mBgPointCloud.cols/sizeof(float));
                bgImgData = (ybgImgData += bgImg.cols);
            }

            // min max in den ersten bytes des bildes verstecken ----------------------------------------
            if (sizeof(float) != 4)
            {
                debout << "Warning: the height range coded inside the background picture is not portable!" << endl;
            }


            memmove(bgImg.data, &min, sizeof(float));
            memmove(bgImg.data+sizeof(float), &max, sizeof(float));
//            double dmin, dmax;
//            cvMinMaxLoc(bgImg, &dmin, &dmax);
//            debout << dmin << " " << dmax << endl;
//            debout << bgImg->depth << " " << bgImg->nChannels << endl;
//            debout << (int)*((char *) bgImg->imageData) << " "
//                   << (int)*(((char *) bgImg->imageData)+1) << " "
//                   << (int)*(((char *) bgImg->imageData)+2) << " "
//                   << (int)*(((char *) bgImg->imageData)+3) <<endl;
//            debout << min << " " << max << endl;

            // wg, koennte es auch ueber meta-daten uebertragen werden CVAPI(int) cvSaveImage( const char* filename, const CvArr* image, const int* params CV_DEFAULT(0) );
            imwrite(dest.toStdString(),bgImg);
//            cvSaveImage(dest.toStdString().c_str(), bgImg);

            mLastFile = dest;

            debout << "export background subtraction file for stereo mode: " << dest << " (minimal z value "<<min<<", maximal z value "<<max<<")." << endl;
        }
        else
            return false;
    }
    else // nicht stereo
    {
        ;
    }
#endif
    return true;
}

// rueckgabe, ob einlesen geklappt hat
// das mappen von foat->char->float fuehrt dazu, dass beim laden nicht mehr volle 256 beim erneuten herausschreiben vorhanden sind!!!!!!!!!!!!!!!!!!!!!
// wenn mit photoshop backgroundImgage veraendert wird muss darauf geachtet werden, dass photoshop in greyscale modus arbeitet!!!!!!!!!!!!!!!!!!!!!!!!
bool BackgroundFilter::load(QString dest) //default = ""
{
    if (!mBgPointCloud.empty() || mBgModel != NULL) // initialisierung
        debout << "Warning: Eliminate existing background!" <<endl;

    if (1) // (*stereoContext()) // auskommentiert, damit bg auch angelegt werden kann, wenn noch kein video geladen wurde
    {
        // if no destination file or folder is given
        if (dest.isEmpty())
        {
            dest = QFileDialog::getOpenFileName(NULL, "Select file for background subtraction in stereo mode", mLastFile,
                                                "Background picture (*.png);;All files (*.*)");
        }

        if (!dest.isEmpty())
        {
            //* CV_LOAD_IMAGE_COLOR the loaded image is forced to be a 3-channel color image
            //* CV_LOAD_IMAGE_GRAYSCALE the loaded image is forced to be grayscale
            //* CV_LOAD_IMAGE_UNCHANGED the loaded image will be loaded as is.

            Mat bgImg = imread(dest.toStdString(),IMREAD_GRAYSCALE);
//            IplImage *bgImg = cvLoadImage(dest.toStdString().c_str(), CV_LOAD_IMAGE_GRAYSCALE);
            if (bgImg.empty())
            {
                debout << "Error: could not read background subtraction file " << dest << "!" <<endl;
                return false;
            }

            if (mBgPointCloud.empty())
                mBgPointCloud.create(bgImg.rows,bgImg.cols, CV_32FC3);

            float min, max;
            // min max aus den ersten bytes auslesen
            memmove(&min, bgImg.data, sizeof(float));
            memmove(&max, bgImg.data+sizeof(float), sizeof(float));

//            debout << bgImg->depth << " " << bgImg->nChannels << endl;
//            debout << (int)*((char *) bgImg->imageData) << " "
//                   << (int)*(((char *) bgImg->imageData)+1) << " "
//                   << (int)*(((char *) bgImg->imageData)+2) << " "
//                   << (int)*(((char *) bgImg->imageData)+3) <<endl;
//            debout << min << " " << max << endl;
//            double dmin, dmax;
//            cvMinMaxLoc(bgImg, &dmin, &dmax);
//            debout << dmin << " " << dmax << endl;

//cvNamedWindow("CVdisp", CV_WINDOW_AUTOSIZE);
//cvShowImage("CVdisp", bgImg);

            if (sizeof(float) != 4)
                debout << "Warning: the height range coded inside the background picture is not portable!" << endl;

            // uebetragen der z-werte in 8-bit-bild ---------------------------------------------------------
            // quantisierung fuehrt dazu, dass bei 10m maximaler spannweite pro grauwertstufe 4cm
            float *data = (float*) mBgPointCloud.data;
            float *yData = data;
            float scale = (max-min)/(255-1); // 10 wird als unterster Wert genommen, damit 0 fuer ungueltigen wert stehen kann
            unsigned char* bgImgData = (unsigned char*) bgImg.data;
            unsigned char* ybgImgData = bgImgData;
            int x, y;

            for (y = 0; y < mBgPointCloud.rows; ++y)
            {
                for (x = 0; x < mBgPointCloud.cols; ++x)
                {
                    if (*bgImgData != 0)
                        data[2] = min+scale*(*bgImgData-1); // -1, da 0 nicht gueltig markiert
                    else // schwarz bedeutet keine Info
                        data[2] = -1;
                    data += 3;
                    ++bgImgData;
                }
                data = (yData += mBgPointCloud.cols/sizeof(float));
                bgImgData = (ybgImgData += bgImg.cols);
            }

            // size war in bild versteckt - umsetzung muss nun eliminert werden
            data = (float*) mBgPointCloud.data;
            for (y = 0; y < ((int) (2*sizeof(float))); ++y)
            {
                data[2] = -1;
                data += 3;
            }

            setChanged(true);
            mLastFile = dest;

            if (mForeground.empty()) // wenn zuvor noch keine background subtraction an war oder ist
                mForeground.create(Size(bgImg.cols,bgImg.rows),CV_8UC1);// = cvCreateImage(cvSize(bgImg->width, bgImg->height), IPL_DEPTH_8U, 1); // CV_8UC1 8, 1

            debout << "import background subtraction file (min "<<min<<", max "<<max<<") for stereo mode: " << dest << "." << endl;
        }
        else
            return false; // keine dest ausgesucht
    }
    else // nicht stereo
    {
        ;
    }
    return true;
}

void BackgroundFilter::setDefaultHeight(double h)
{
    mDefaultHeight = h;
}

void BackgroundFilter::setUpdate(bool b)
{
    mUpdate = b;
}

bool BackgroundFilter::update()
{
    return mUpdate;
}

void BackgroundFilter::setStereoContext(pet::StereoContext **sc)
{
    mStereoContext = sc;
}
pet::StereoContext** BackgroundFilter::stereoContext()
{
    return mStereoContext;
}

// interessante dateien zur backgroundsubtraction:
// c:/Programme/OpenCV/samples/c/bgfg_segm.cpp
// c:/Programme/OpenCV/cvaux/include/cvaux.h
// http://opencvlibrary.sourceforge.net/VideoSurveillance
// opencv buch: S. 265
// opencv pdf: S. 414
Mat BackgroundFilter::act(Mat &img, Mat &res)
{
//    Mat img = cvarrToMat(imgIpl);
//        res = cvarrToMat(resIpl);
//    return act(img,res);
#ifdef SHOW_TMP_IMG
namedWindow("BackgroundFilter");
imshow("BackgroundFilter",img);
waitKey();
#endif
//    debout << "BackgroundFilter act ... img->nChannels: " << img->nChannels << " res->nChannels: " << res->nChannels << endl;
//    static IplImage * img3c = NULL;
//    static CvFGDStatModelParams* params = NULL; //new CvFGDStatModelParams;


//    cvRunningAvg(img, acc, 1./num);
//    // nach lernen
//    cvAbsDiff(acc, img, mask);
//    cvThreshold(mask, mask, 15, 255, CV_THRESH_BINARY);

//BackgroundSubtractorMOG

    if (mBgPointCloud.empty() && mBgModel.empty() || mForeground.empty() || mForeground.size != img.size) // initialisierung wenn entwerder stereo oder model
    {
#ifndef STEREO_DISABLED
// For StereoImaging use heightfiled for foreground extraction -------------------------------------------------------------------------------
        if (*stereoContext())
        {
            // aktuell wird pro pixel nur der z-wert betrachtet,
            // um aber entlang waenden personen auch vom hintergrund abheben zu koennen
            // muesste die xyz-wolke  betrachtet werden und eine abfrage eines z-wertes an der selben xy-stelle erfolgen
            // dafuer datenstruktue (zB aequidistantes gitter) in dem korrespondierender z-wert gesucht wird
            // -> plan view statistic

            //bgDisparity = cvCloneImage((*stereoContext())->getDisparity());
            mBgPointCloud = cvarrToMat((*stereoContext())->getPointCloud()).clone();// = cvCloneMat((*stereoContext())->getPointCloud());

            // interpolate z-values inbetween innerhalb zeile
            int x, y;
            float *vorPtr;  // zeigt auf den letzten gueltigen Wert vor ungueltigem wert in einer Zeile
            float *nachPtr; // zeigt auf den ersten gueltigen Wert nach ungueltigen in einer Zeile
            float *fPtr;
            float step;

            float *bgPcData = (float *) mBgPointCloud.data;
            float *bgyPcData = bgPcData;

            for (y = 0; y < mBgPointCloud.rows; ++y)
            {
                vorPtr = nachPtr = NULL;
                for (x = 0; x < (mBgPointCloud.cols)-1; ++x) // -1 da in zukunft getestet wird, dadurch letztes pixel ungeprueft
                {
                    if (vorPtr == NULL && bgPcData[2] != -1 && bgPcData[5] == -1) // eine unbestimmte gap beginnt
                        vorPtr = bgPcData;
                    else if (nachPtr == NULL && bgPcData[2] == -1 && bgPcData[5] != -1) // eine unbestimmte gap endet
                    {
                        if (vorPtr != NULL) // innenliegende gap
                            nachPtr = bgPcData+3;
                        else // zeilenbeginn
                        {
                            for (fPtr = bgyPcData; fPtr < bgPcData+3; fPtr+=3)
                            {
                                fPtr[2] = bgPcData[5];
                            }
                        }
                    }
                    if (vorPtr != NULL && nachPtr != NULL) // gap bestimmt, fuellung mit interpolerten werten
                    {
                        //debout << vorPtr[2] <<endl;
                        //debout << nachPtr[2] <<endl;
                        step = ((nachPtr[2]-vorPtr[2])/(nachPtr-vorPtr)); // lange zeit war falsch: /9.; // /3 ein /3 durch (fPtr-vorPtr) und nicht (fPtr-vorPtr)/3 in schleife
                        for (fPtr = vorPtr+3; fPtr < nachPtr; fPtr+=3)
                        {
                            //fPtr[0] = ; // x
                            //fPtr[1] = ; // y
                            fPtr[2] = vorPtr[2]+(fPtr-vorPtr)*step;
                            //debout << fPtr[2] <<endl;
                        }
                        vorPtr = nachPtr = NULL;
                    }
                    bgPcData+=3;
                }
                if (vorPtr != NULL) // gap am zeilenende
                {
                    for (fPtr = vorPtr; fPtr < bgPcData; fPtr+=3)
                    {
                        fPtr[2] = vorPtr[-1];
                    }
                }
                bgPcData = (bgyPcData += mBgPointCloud.cols/sizeof(float));
            }

            mForeground.create(Size(img.cols,img.rows),CV_8UC1);
//            mForeground = cvCreateImage(cvSize(img->width, img->height), IPL_DEPTH_8U, 1); // CV_8UC1 8, 1
        }

        else // nicht stereo
#endif // STEREO_DISABLED
        {

// GaussBGStatModel ---------------------------------------------------------------------------------------------------------------------



        if(!mBgModel.empty())
            mBgModel->clear();

        mBgModel = createBackgroundSubtractorMOG2();//.dynamicCast<BackgroundSubtractor>();
//        mBgModel->setVarThreshold(10);
//        mBgModel->setNMixtures(5);
//        mBgModel->setHistory(75);
//        mBgModel->setDetectShadows(false);
//        mBgModel->setBackgroundRatio(2);
//        mBgModel->set("noiseSigma", 30);
//        mBgModel->setInt("learningRate", 200);
//        mForeground = mBgModel->getForegroundImage();


        mForeground.create(Size(img.cols,img.rows),CV_8UC1);
        //Mat temp_img, temp_foreg_img;

        mBgModel->apply(img,mForeground,1);

#ifdef SHOW_TMP_IMG
namedWindow("BackgroundFilter");
imshow("BackgroundFilter",mForeground);
waitKey();
#endif
        //mBgModel->apply(img,mForegroundMask, update_bg_model ? -1 : 0);
        //mBgModel->getForegroundImage(mForeground);
        //mBgModel->getBackgroundImage(mBackground);
        //mBgModel->apply(img,mForeground);

// FGDStatModel ---------------------------------------------------------------------------------------------------------------------

        // DOCUMENTATION: http://opencv.willowgarage.com/wiki/VideoSurveillance

//        if (img3c == NULL)
//            img3c = cvCreateImage(cvSize(img->width, img->height), 8, 3); // wird nie freigegeben aber auch nur 1 x angelegt

//        cvConvertImage(img, img3c);

////        int    Lc;			/* Quantized levels per 'color' component. Power of two, typically 32, 64 or 128.				*/
////        int    N1c;			/* Number of color vectors used to model normal background color variation at a given pixel.			*/
////        int    N2c;			/* Number of color vectors retained at given pixel.  Must be > N1c, typically ~ 5/3 of N1c.			*/
////                                    /* Used to allow the first N1c vectors to adapt over time to changing background.				*/

////        int    Lcc;			/* Quantized levels per 'color co-occurrence' component.  Power of two, typically 16, 32 or 64.			*/
////        int    N1cc;		/* Number of color co-occurrence vectors used to model normal background color variation at a given pixel.	*/
////        int    N2cc;		/* Number of color co-occurrence vectors retained at given pixel.  Must be > N1cc, typically ~ 5/3 of N1cc.	*/
////                                    /* Used to allow the first N1cc vectors to adapt over time to changing background.				*/

////        int    is_obj_without_holes;/* If TRUE we ignore holes within foreground blobs. Defaults to TRUE.						*/
////        int    perform_morphing;	/* Number of erode-dilate-erode foreground-blob cleanup iterations.						*/
////                                    /* These erase one-pixel junk blobs and merge almost-touching blobs. Default value is 1.			*/
//        // 0 am schnellsten, 1 glaettet insb raender etwas, 2 wird schon zu viel verworfen

////        float  alpha1;		/* How quickly we forget old background pixel values seen.  Typically set to 0.1  				*/
////        float  alpha2;		/* "Controls speed of feature learning". Depends on T. Typical value circa 0.005. 				*/
////        float  alpha3;		/* Alternate to alpha2, used (e.g.) for quicker initial convergence. Typical value 0.1.				*/
//        // alpha2 = 0 + alpha3 = 1  laesst nicht neu lernen

////        float  delta;		/* Affects color and color co-occurrence quantization, typically set to 2.					*/
////        float  T;			/* "A percentage value which determines when new features can be recognized as new background." (Typically 0.9).*/
////        float  minArea;		/* Discard foreground blobs whose bounding box is smaller than this threshold.					*/

//        mBgModel = cvCreateFGDStatModel(img3c); // ohne , params werden default-werte gesetzt // <- fkt frueher mit grossen bildern wg speichermengenproblem nicht, nun muss nur noch 3 channel bild sein!!! https://code.ros.org/trac/opencv/ticket/31
//        params = &(((CvFGDStatModel *)mBgModel)->params); // cast, da mBgModel allgemeiner pointer auf model
////        debout << params->Lc << endl;
////        debout << params->N1c << endl;
////        debout << params->N2c << endl;
////        debout << params->Lcc << endl;
////        debout << params->N1cc << endl;
////        debout << params->N2cc << endl;
////        debout << params->is_obj_without_holes << endl;
////        debout << params->perform_morphing << endl;
////        debout << params->alpha1 << endl;
////        debout << params->alpha2 << endl;
////        debout << params->alpha3 << endl;
////        debout << params->delta << endl;
////        debout << params->T << endl;
////        debout << params->minArea << endl;

//        params->perform_morphing = 0;
//        params->alpha2 = 0.1;
//        params->alpha3 = 1;
//        params->minArea = 0;
//        cvUpdateBGStatModel(img3c, mBgModel); // das erste mal mit neuen parametern und mit initalem bild

        } // nicht stereo
    }
    else // nicht initialisierung
    {

// SteroBild beruecksichtigen nur disparity --------------------------------------------------------------------------------------------------------
            if (*stereoContext())
            {
#ifndef STEREO_DISABLED
// fuer update waere bei stereo denkbar: mittelwert der disp/zwerte, aber Achtung: invalidDisp beruecksichtigen!


//                unsigned short* bgData = (unsigned short*) bgDisparity->imageData; // disparity zum reset-Zeitpunkt (Hintergrund)
//                unsigned short* bgyData = bgData;

//                IplImage *disparity = (*stereoContext())->getDisparity();
//                unsigned short* data = (unsigned short*) disparity->imageData;
//                unsigned short* yData = data;

//                int x,y;

//                for (y = 0; y < bgDisparity->height; ++y)
//                {
//                    for (x = 0; x < bgDisparity->width; ++x)
//                    {
//                        if ((*stereoContext())->dispValueValid(*data) && (*stereoContext())->dispValueValid(*bgData))
//                        {
//                            if (fabs(*data-*bgData) > 100)
//                                mForeground->imageData[y*bgDisparity->width+x] = 1;
//                        }
//                        ++data;
//                        ++bgData;
//                        pcData+=3;
//                        bgPcData+=3;
//                    }
//                    data = (yData += disparity->width);
//                    bgData = (bgyData += bgDisparity->width);
//                }

// SteroBild beruecksichtigen z-wert --------------------------------------------------------------------------------------------------------

                int x,y;

                float *bgPcData = (float *) mBgPointCloud.data;
                float *bgyPcData = bgPcData;

                Mat pointCloud = cvarrToMat((*stereoContext())->getPointCloud());
                float *pcData = (float *) pointCloud.data;
                float *yPcData = pcData;

                mForeground = Scalar(0,0,0);
//                cvSet(mForeground, CV_RGB(0,0,0));
//                cvFillImage(mForeground, 0); //um vorherige foreground-berechungen zu vergessen

                for (y = 0; y < pointCloud.rows; ++y)
                {
                    for (x = 0; x < pointCloud.cols; ++x)
                    {
                        if ((bgPcData[2] != -1) && (pcData[2] != -1))
                        {
                            if ((bgPcData[2]-pcData[2]) > FOREGROUND_DISTANCE) // z-Wert in m (nicht cm!) wenn z-wert 1m unter defaultgroesse
                            {
                                mForeground.data[y*pointCloud.cols+x] = 1;
                            }
                            //if (y == 488)
                            //    debout << bgPcData[2] << " " << pcData[2] <<endl;
                        }
                        pcData+=3;
                        bgPcData+=3;
                    }
                    pcData = (yPcData += pointCloud.cols/sizeof(float));
                    bgPcData = (bgyPcData += mBgPointCloud.cols/sizeof(float));
                }

#endif
           }
else // nicht stereo
            {

            //        params = &(((CvFGDStatModel *)mBgModel)->params);
            ////        params->perform_morphing = cw->r2->value()*1000;
            ////        params->alpha1 = cw->r4->value()*1000;
            ////        params->alpha2 = cw->tx->value()*1000;
            ////        params->alpha3 = cw->ty->value()*1000;
            ////        debout << params->perform_morphing << endl;
            ////        debout << params->alpha1 << endl;
            ////        debout << params->alpha2 << endl;
            ////        debout << params->alpha3 << endl;

            //        cvConvertImage(img, img3c);

            //        if (update())
            //        {
            //            params->alpha2 = 0.1;
            //            cvUpdateBGStatModel(img3c, mBgModel); // default -1 das Hintergrundmodell wird mit aktuellem Bild aktualisiert
            //        }
            //        else
            //        {
            //            params->alpha2 = 0;
            //            cvUpdateBGStatModel(img3c, mBgModel, 0); // der letzte Parameter bewirkt bei FGD nichts, daher param geaendert
            //        }


// GaussBGStatModel ---------------------------------------------------------------------------------------------------------------------


                mBgModel->apply(img, mForeground, update() ? -1 : 0);

//                if (update())
//                {
//                    mBgModel->apply(img,mForeground,-1);
//                    //mBgModel->apply(img,mForeground);
//                }
//                else
//                {
//                    mBgModel->apply(img,mForeground,0);
//                    //mBgModel->apply(img,mForeground,0);
//                }

#ifdef SHOW_TMP_IMG
imshow("BackgroundFilter",img);
waitKey();
#endif

    }
#ifdef SHOW_TMP_IMG
imshow("BackgroundFilter",mForeground);
waitKey();
#endif
        // einfache methode, um kleine gebiete in maske zu eliminieren und ausfransungen zu entfernen ---------------------------------------------------

            cv::erode(mForeground,mForeground,cv::getStructuringElement(MORPH_RECT,Size(3,3)),Point(-1,-1),1);
#ifdef SHOW_TMP_IMG
imshow("BackgroundFilter",mForeground);
waitKey();
#endif
            cv::dilate(mForeground,mForeground,cv::getStructuringElement(MORPH_RECT,Size(3,3)),Point(-1,-1),2);
#ifdef SHOW_TMP_IMG
imshow("BackgroundFilter",mForeground);
waitKey();
#endif
            cv::GaussianBlur(mForeground,mForeground,Size(11,11),3.5,3.5);
#ifdef SHOW_TMP_IMG
imshow("BackgroundFilter",mForeground);
waitKey();
#endif
            cv::threshold(mForeground,mForeground,20,255,THRESH_BINARY);

#ifdef SHOW_TMP_IMG
imshow("BackgroundFilter",mForeground);
waitKey();
#endif
            //            cvErode(mForeground, mForeground, NULL, 1);
//            cvDilate(mForeground, mForeground, NULL, 2);
             //cvErode(mForeground, mForeground, NULL, 1); // weggelassen, dadurch hintergrund etwas groesser

        // loecher schliessen ------------------------------------------------------------------------------------------------------------
             // floodfill den aussenbereich und suchen nach ungefuelltem koennte auch loechder finden alssen
             vector<vector<Point> > contours;
             double contourArea;
//             CvPoint* pointArray;
             // find contours and store them all as a list
//             CvMemStorage *storage = cvCreateMemStorage(0);
             cv::findContours(mForeground,contours,cv::RETR_LIST,cv::CHAIN_APPROX_SIMPLE);
//             cvFindContours(mForeground, storage, &contour, sizeof(CvContour),
//                            CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE); // foreground wird auch veraendert???
             // test each contour

             while (!contours.empty())
             {
                 vector<Point> contour = contours.back();

                 contourArea = cv::contourArea(contour, true);
                 if (contourArea > 0 && contourArea < 400) // kleine innere loecher schliessen
                 {
//                     pointArray = (CvPoint*)malloc(contour->total*sizeof(CvPoint));
                     // Get contour point set.
//                     cvCvtSeqToArray(contour, pointArray, CV_WHOLE_SEQ);
                     //debout << contourArea << " " << pointArray->x << " " << pointArray->y <<   endl;
                     cv::fillPoly(mForeground,Mat(contour),Scalar::all(1));
//                     cvFillPoly(mForeground, &pointArray , &(contour->total), 1, cvScalar(1,1,1,1)); // int line_type CV_DEFAULT(8), int shift CV_DEFAULT(0));
                     // folgende fkt nur fuer konvexe, daher bleiben einige bereiche nicht eingezeichnet
                     //cvFillConvexPoly(mForeground, pointArray , contour->total, cvScalar(1,1,1,1)); // int line_type CV_DEFAULT(8), int shift CV_DEFAULT(0));
//                     free(pointArray);
                 }
                 else if (contourArea > MIN_FOREGROUND_AREA -400) // kleine Bereiche werden eliminiert, dies koennte in Abhaengigkeit von der disparity gemacht werden!!!!!!!!
                 {
//                     pointArray = (CvPoint*)malloc(contour->total*sizeof(CvPoint));
                     // Get contour point set.
//                     cvCvtSeqToArray(contour, pointArray, CV_WHOLE_SEQ);
                     //debout << contourArea << " " << pointArray->x << " " << pointArray->y <<   endl;
                     cv::fillPoly(mForeground,Mat(contour),Scalar::all(0));
//                     cvFillPoly(mForeground, &pointArray , &(contour->total), 1, cvScalar(0,0,0,0)); // int line_type CV_DEFAULT(8), int shift CV_DEFAULT(0));
//                     free(pointArray);
                 }
                 // in rot contouren in eine drehrichtung, in gruen andere
                 //cvDrawContours(img,contour,CV_RGB(255,0,0),CV_RGB(0,255,0),0,1,8,cvPoint(0,0));
                 //cvDrawContours(res,contour,CV_RGB(255,0,0),CV_RGB(0,255,0),0,1,8,cvPoint(0,0));
                 // take the next contour
                 contours.pop_back();
//                 contour = contour->h_next;
             }

#ifdef SHOW_TMP_IMG
imshow("BackgroundFilter",mForeground);
waitKey();
#endif
//             cvReleaseMemStorage(&storage);
    }

// Bild gemaess maske einfaerben ---------------------------------------------------------------------------------------------------------------------

//    int x, y;

//    // background set to color
//    int r = 255;
//    int g = 255;
//    int b = 0;
////     debout << res->height << " " << res->width << " " << res->nChannels << " " << res->depth << endl;
////     debout << mForeground->height << " " << mForeground->width << " " << mForeground->nChannels << " " << mForeground->depth << endl;

//    // Pointer to the data information in the IplImage
//    unsigned char *dataIn  = (unsigned char *) img->imageData;
//    unsigned char *dataOut = (unsigned char *) res->imageData;
//    // foreground ist entweder 0=Hintergrund oder 255=fordergrund
//    unsigned char *mask = (unsigned char *) mForeground->imageData;
//    // set poiner to value before array, because ++i is more effective than i++
//    --dataIn; --dataOut; --mask;

//    if (res->nChannels == 3)
//    {
//        for (y = 0; y < res->height; ++y)
//        {
//            for (x = 0; x < res->width; ++x)
//            {

//                if (((int)*(++mask)) != 0)
//                {
//                    *(++dataOut) = *(++dataIn);
//                    *(++dataOut) = *(++dataIn);
//                    *(++dataOut) = *(++dataIn);
//                }
//                else
//                {
//                    dataIn+=3;
//                    *(++dataOut) = b;
//                    *(++dataOut) = g;
//                    *(++dataOut) = r;
//                }
//                //             if ((x < 410) && (y < 410) && (x > 400) && (y > 400))
//                //                 debout << ((int)*mask)<<endl;
//            }
//        }
//    }
//    else
//    {
//        for (y = 0; y < res->height; ++y)
//        {
//            for (x = 0; x < res->width; ++x)
//            {

//                if (((int)*(++mask)) != 0) // *mask == 1, 2, 130
//                {
//                    *(++dataOut) = *(++dataIn);
//                }
//                else
//                {
//                    ++dataIn;
//                    *(++dataOut) = r;
//                }
//                //             if ((x < 410) && (y < 410) && (x > 400) && (y > 400))
//                //                 debout << ((int)*mask)<<endl;
//            }
//        }
//    }
////     debout << res->height << " " << res->width << " " << res->nChannels << " " << res->depth << endl;
////     debout << mForeground->height << " " << mForeground->width << " " << mForeground->nChannels << " " << mForeground->depth << endl;
////             cvNamedWindow("BG", 1);
////             cvNamedWindow("FG", 1);
////                 cvShowImage("BG", mBgModel->background);
////                 cvShowImage("FG", mBgModel->foreground);

//    resIpl = cvCreateImage(cvSize(res.cols,res.rows),8,3);
//    IplImage tmpIpl = res;
//    cvCopy(&tmpIpl,resIpl);
    return res;
}


// Three cpp files whose names are starting with "cvbgfg" under OpenCV\cvaux\src
// They are "cvbgfg_common.cpp", "cvbgfg_gaussmix.cpp" and "cvbgfg_acmmm2003.cpp".

// I'm not sure about "cvbgfg_common.cpp"(Maybe, I guess from the name,
// it has some common functions of background subtraction methods).

// "cvbgfg_gaussmix.cpp" has to do with CvGaussBGModel and the
// implementation is based on the paper "An Improved Adaptive
// Background Mixture Model for Real-time Tracking and Shadow
// Detection" by P. KaewTraKulPong and R. Bowden in Proc. 2nd European
// Workshp on Advanced Video-Based Surveillance Systems, 2001.

// "cvbgfg_acmmm2003.cpp" has to do with CvFGDStatModel and the
// implemenation is based on the paper "Foreground Object Detection
// from Videos Containing Complex Background." by Liyuan Li, Weimin
// Huang, Irene Y.H. Gu, and Qi Tian in ACM-MM 2003.

// So there are two ways of background subtraction I'm not sure about
// the difference between the two approaches. However I'm using
// CvGaussBGModel since I saw the word "real-time" in the paper title,
// even though the paper of CvFGDStatModel is more recent.

// So if you want to use CvGaussBGModel, the following will be an
// example.

//     IplImage* img = NULL;
//     CvCapture* cap = NULL;
//     cap = cvCaptureFromFile(file_name);
//     img = cvQueryFrame(cap);
//     CvBGStatModel* background_model = cvCreateGaussianBGModel(img);
//     for(int fr = 1; img; img = cvQueryFrame(cap), fr++) {
//         cvUpdateBGStatModel(img, background_model);
//     }

// And if you want to use CvFGDStatModel, you can just change the
// initialization as following.

//     CvBGStatModel* background_model = cvCreateFGDStatModel(img);
//     for(int fr = 1; img; img = cvQueryFrame(cap), fr++) {
//         cvUpdateBGStatModel(img, background_model);
//     }


// siehe auch mitgeliefertes Beispiel unter C:\Programme\OpenCV\samples\c\bgfg_segm.cpp

// cap = cvCaptureFromFile("pedes.avi");
//     tmp_frame = cvQueryFrame(cap);
//     if(!tmp_frame)
//     {
//         printf("bad video \n");
//         exit(0);
//     }
//     cvNamedWindow("BG", 1);
//     cvNamedWindow("FG", 1);
//     //create BG model
//     CvBGStatModel* bg_model = cvCreateGaussianBGModel( tmp_frame );
//     for( int fr = 1;tmp_frame; tmp_frame = cvQueryFrame(cap), fr++ )
//     {
//         double t = (double)cvGetTickCount();
//         cvUpdateBGStatModel( tmp_frame, bg_model );
//         t = (double)cvGetTickCount() - t;
//         printf( "%.1f\n", t/(cvGetTickFrequency()*1000.) );
//         cvShowImage("BG", bg_model->background);
//         cvShowImage("FG", bg_model->foreground);
//         int k = cvWaitKey(5);
//         if( k == 27 ) break;
//         printf("frame# %d \n", fr);
//     }
//     cvReleaseBGStatModel( &bg_model );
//     cvReleaseCapture(&cap);



// if you have an image of the background, you can subtract the
// background from the given image with cvSub.

// Example: cvSub(image_of_background, given_image, destination_image, NULL);

// To totaly eleminate the background you can compare background_image to
// given_image to create a mask:

// cvAbsDiff(image_of_background, given_image, mask_image);

// The outcome of this is a binary mask, which can now be combined with
// given_image:

// IplImage* tmp = cvCloneImage(given_image);
// cvZero(given_image);
// cvCopy(tmp, given_image, mask);
// cvReleaseImage( &tmp);

// Now given_image contains only foreground objects.


// setzt in mat alle bg pixel auf val
// bei mehreren Kanaelen nur den letzten Kanal
// bisher nur fuer float
void BackgroundFilter::maskBg(Mat &mat, float val)
{
    if (getEnabled() && !mForeground.empty())
    {

//        debout << mat->type <<endl;
//        debout << CV_32F <<endl;
//        debout << CV_32FC3 <<endl;
//        if (mat->type == CV_32F)  //CV_32FC3
//        {
            int x, y;

            int lineLen = mat.cols/sizeof(float);
            int ch = lineLen/mat.cols;
            float *pcData = ((float *) mat.data) + ch-1; // ch-1 um beim letzten element zu stehen (wie pcData[2])
            float *yPcData = pcData;

            unsigned char* fgData = (unsigned char*) mForeground.data;
            unsigned char* yFgData = fgData;

            for (y = 0; y < mat.rows; ++y)
            {
                for (x = 0; x < mat.cols; ++x)
                {
                    //if (!isForeground(x, y))
                    if(!*fgData)  // mForeground->imageData[y*mForeground->width+x]  0 background, 1 foreground
                    {
                        *pcData = val;
                    }
                    ++fgData;
                    pcData += ch;
                }
                pcData = (yPcData += lineLen);
                fgData = (yFgData += mForeground.cols);
            }
//        }
//        else
//            debout << "Error: only type float is possible." <<endl;
    }
}
