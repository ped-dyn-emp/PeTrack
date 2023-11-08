/*
 * PeTrack - Software for tracking pedestrians movement in videos
 * Copyright (C) 2023 Forschungszentrum Jülich GmbH, IAS-7
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

#include "backgroundFilter.h"

#include "stereoContext.h"

// nur temporaer fuer anzeige
#include "helper.h"
#include "logger.h"

#include <QFileDialog>
#include <opencv2/highgui.hpp>


/// Entfernung zum Hintergrund, um als Vordergrund angesehen zu werden
/// darf nicht zu gross werden, da sonst an waenden problem
#define FOREGROUND_DISTANCE 0.4

/// kleine Bereiche werden eliminiert, dies koennte in Abhaengigkeit von der disparity gemacht werden!!!!!!!!
/// Anzahl der Pixel, die ein Vordergrund aufweisen muss
#define MIN_FOREGROUND_AREA -1000 // war:-400


BackgroundFilter::BackgroundFilter() : Filter()
{
    // da img nur ausgelesen, aber bnicht veraendert wird kann auf eine kopie verzichtet werden (true ist default bei
    // filter)
    setOnCopy(false);

    mLastFile = "";
}

BackgroundFilter::~BackgroundFilter() {}


// nutzen, wenn ueber ganzes bild foreground benutzt wird; NULL, wenn keine background subtraction aktiviert
cv::Mat BackgroundFilter::getForeground()
{
    return mForeground;
}

/**
 * @brief Determines if pixel is in foreground. Use for single pixels.
 *
 * Does not check if pixel position is valid! (For efficiency)
 *
 * @param coloumn
 * @param row
 * @return true if foreground
 */
bool BackgroundFilter::isForeground(int coloumn, int row)
{
    if(!mForeground.empty())
    {
        return (bool) mForeground.data[row * mForeground.cols + coloumn]; // 0 background, 1 foreground
    }
    else
    {
        return false;
    }
}

/// zuruecksetzen, wenn zB helligkeit veraendert wird oder schaerfe
void BackgroundFilter::reset()
{
    if(!mForeground.empty())
    {
        mForeground = cv::Scalar::all(0);
    }
    if(!mBgModel.empty())
    {
        mBgModel->clear();
    }

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
bool BackgroundFilter::save([[maybe_unused]] QString dest) // default = ""
{
    if(*stereoContext() && !mBgPointCloud.empty())
    {
        // if no destination file or folder is given
        if(dest.isEmpty())
        {
            dest = QFileDialog::getSaveFileName(
                NULL,
                "Select file for background subtraction in stereo mode",
                mLastFile,
                "Background picture (*.png);;All files (*.*)");
        }
        if(!dest.isEmpty())
        {
            if(!dest.endsWith(".png", Qt::CaseInsensitive))
                dest = dest + ".png";

            cv::Mat bgImg(cv::Size(mBgPointCloud.cols, mBgPointCloud.rows), CV_8UC1);
            //            IplImage *bgImg = cvCreateImage(cvSize(mBgPointCloud->width, mBgPointCloud->height),
            //            IPL_DEPTH_8U, 1); // CV_8UC1 8, 1

            // minimum der z-Werte bestimmen
            float *data  = (float *) mBgPointCloud.data;
            float *yData = data;
            int    x, y;
            float  min = FLT_MAX;
            float  max = FLT_MIN;

            for(y = 0; y < mBgPointCloud.rows; ++y)
            {
                for(x = 0; x < mBgPointCloud.cols; ++x)
                {
                    if(data[2] != -1)
                    {
                        if(data[2] > max)
                            max = data[2];
                        else if(data[2] < min)
                            min = data[2];
                    }
                    data += 3;
                }
                data = (yData += mBgPointCloud.cols / sizeof(float));
            }

            // uebetragen der z-werte in 8-bit-bild ---------------------------------------------------------
            // quantisierung fuehrt dazu, dass bei 10m maximaler spannweite pro grauwertstufe 4cm
            data        = (float *) mBgPointCloud.data;
            yData       = data;
            float scale = (255 - 1) /
                          (max - min); // 10 wird als unterster Wert genommen, damit 0 fuer ungueltigen wert stehen kann
            unsigned char *bgImgData  = (unsigned char *) bgImg.data;
            unsigned char *ybgImgData = bgImgData;

            for(y = 0; y < mBgPointCloud.rows; ++y)
            {
                for(x = 0; x < mBgPointCloud.cols; ++x)
                {
                    if(data[2] != -1)
                        *bgImgData = 1 + scale * (data[2] - min);
                    else // schwarz bedeutet keine Info
                        *bgImgData = 0;
                    data += 3;
                    ++bgImgData;
                }
                data      = (yData += mBgPointCloud.cols / sizeof(float));
                bgImgData = (ybgImgData += bgImg.cols);
            }

            // min max in den ersten bytes des bildes verstecken ----------------------------------------
            if(sizeof(float) != 4)
            {
                SPDLOG_WARN("the height range coded inside the background picture is not portable!");
            }


            memmove(bgImg.data, &min, sizeof(float));
            memmove(bgImg.data + sizeof(float), &max, sizeof(float));
            //            double dmin, dmax;
            //            cvMinMaxLoc(bgImg, &dmin, &dmax);
            //            debout << dmin << " " << dmax << endl;
            //            debout << bgImg->depth << " " << bgImg->nChannels << endl;
            //            debout << (int)*((char *) bgImg->imageData) << " "
            //                   << (int)*(((char *) bgImg->imageData)+1) << " "
            //                   << (int)*(((char *) bgImg->imageData)+2) << " "
            //                   << (int)*(((char *) bgImg->imageData)+3) <<endl;
            //            debout << min << " " << max << endl;

            // wg, koennte es auch ueber meta-daten uebertragen werden CVAPI(int) cvSaveImage( const char* filename,
            // const CvArr* image, const int* params CV_DEFAULT(0) );
            imwrite(dest.toStdString(), bgImg);
            //            cvSaveImage(dest.toStdString().c_str(), bgImg);

            mLastFile = dest;

            SPDLOG_INFO(
                "export background subtraction file for stereo mode: {} (minimal z value {}, maximal z value {}).",
                dest,
                min,
                max);
        }
        else
            return false;
    }
    else // nicht stereo
    {
        ;
    }
    return true;
}

// rueckgabe, ob einlesen geklappt hat
// das mappen von foat->char->float fuehrt dazu, dass beim laden nicht mehr volle 256 beim erneuten herausschreiben
// vorhanden sind!!!!!!!!!!!!!!!!!!!!! wenn mit photoshop backgroundImgage veraendert wird muss darauf geachtet werden,
// dass photoshop in greyscale modus arbeitet!!!!!!!!!!!!!!!!!!!!!!!!
bool BackgroundFilter::load(QString dest) // default = ""
{
    if(!mBgPointCloud.empty() || mBgModel != nullptr) // initialisierung
    {
        SPDLOG_WARN("Eliminate existing background!");
    }

    if(1) // (*stereoContext()) // auskommentiert, damit bg auch angelegt werden kann, wenn noch kein video geladen
          // wurde
    {
        // if no destination file or folder is given
        if(dest.isEmpty())
        {
            dest = QFileDialog::getOpenFileName(
                nullptr,
                "Select file for background subtraction in stereo mode",
                mLastFile,
                "Background picture (*.png);;All files (*.*)");
        }

        if(!dest.isEmpty())
        {
            cv::Mat bgImg = cv::imread(dest.toStdString(), cv::IMREAD_GRAYSCALE);
            if(bgImg.empty())
            {
                SPDLOG_ERROR("could not read background subtraction file {}!", dest);
                return false;
            }

            if(mBgPointCloud.empty())
            {
                mBgPointCloud.create(bgImg.rows, bgImg.cols, CV_32FC3);
            }

            float min, max;
            // min max aus den ersten bytes auslesen
            memmove(&min, bgImg.data, sizeof(float));
            memmove(&max, bgImg.data + sizeof(float), sizeof(float));

            if constexpr(sizeof(float) != 4)
            {
                SPDLOG_WARN("the height range coded inside the background picture is not portable!");
            }

            // uebetragen der z-werte in 8-bit-bild ---------------------------------------------------------
            // quantisierung fuehrt dazu, dass bei 10m maximaler spannweite pro grauwertstufe 4cm
            float *data  = (float *) mBgPointCloud.data;
            float *yData = data;
            float  scale = (max - min) /
                          (255 - 1); // 10 wird als unterster Wert genommen, damit 0 fuer ungueltigen wert stehen kann
            unsigned char *bgImgData  = (unsigned char *) bgImg.data;
            unsigned char *ybgImgData = bgImgData;
            int            x, y;

            for(y = 0; y < mBgPointCloud.rows; ++y)
            {
                for(x = 0; x < mBgPointCloud.cols; ++x)
                {
                    if(*bgImgData != 0)
                    {
                        data[2] = min + scale * (*bgImgData - 1); // -1, da 0 nicht gueltig markiert
                    }
                    else // schwarz bedeutet keine Info
                    {
                        data[2] = -1;
                    }
                    data += 3;
                    ++bgImgData;
                }
                data      = (yData += mBgPointCloud.cols / sizeof(float));
                bgImgData = (ybgImgData += bgImg.cols);
            }

            // size war in bild versteckt - umsetzung muss nun eliminert werden
            data = (float *) mBgPointCloud.data;
            for(y = 0; y < ((int) (2 * sizeof(float))); ++y)
            {
                data[2] = -1;
                data += 3;
            }

            setChanged(true);
            mLastFile = dest;

            if(mForeground.empty()) // wenn zuvor noch keine background subtraction an war oder ist
            {
                mForeground.create(
                    cv::Size(bgImg.cols, bgImg.rows),
                    CV_8UC1); // = cvCreateImage(cvSize(bgImg->width, bgImg->height), IPL_DEPTH_8U, 1); // CV_8UC1 8, 1
            }

            SPDLOG_INFO("import background subtraction file (min {}, max {}) for stereo mode: {}.", min, max, dest);
        }
        else
        {
            return false; // keine dest ausgesucht
        }
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

bool BackgroundFilter::update() const
{
    return mUpdate;
}

void BackgroundFilter::setStereoContext(pet::StereoContext **sc)
{
    mStereoContext = sc;
}
pet::StereoContext **BackgroundFilter::stereoContext()
{
    return mStereoContext;
}

cv::Mat BackgroundFilter::act(cv::Mat &img, cv::Mat &res)
{
#ifdef SHOW_TMP_IMG
    namedWindow("BackgroundFilter");
    imshow("BackgroundFilter", img);
    waitKey();
#endif
    if((mBgPointCloud.empty() && mBgModel.empty()) || mForeground.empty() ||
       mForeground.size != img.size) // initialisierung wenn entwerder stereo oder model
    {
        // For StereoImaging use heightfiled for foreground extraction
        // -------------------------------------------------------------------------------
        if(*stereoContext())
        {
            // aktuell wird pro pixel nur der z-wert betrachtet,
            // um aber entlang waenden personen auch vom hintergrund abheben zu koennen
            // muesste die xyz-wolke  betrachtet werden und eine abfrage eines z-wertes an der selben xy-stelle erfolgen
            // dafuer datenstruktue (zB aequidistantes gitter) in dem korrespondierender z-wert gesucht wird
            // -> plan view statistic

            // bgDisparity = cvCloneImage((*stereoContext())->getDisparity());
            mBgPointCloud =
                (*stereoContext())->getPointCloud().clone(); // = cvCloneMat((*stereoContext())->getPointCloud());

            // interpolate z-values inbetween innerhalb zeile
            int    x, y;
            float *vorPtr;  // zeigt auf den letzten gueltigen Wert vor ungueltigem wert in einer Zeile
            float *nachPtr; // zeigt auf den ersten gueltigen Wert nach ungueltigen in einer Zeile
            float *fPtr;
            float  step;

            float *bgPcData  = (float *) mBgPointCloud.data;
            float *bgyPcData = bgPcData;

            for(y = 0; y < mBgPointCloud.rows; ++y)
            {
                vorPtr = nachPtr = NULL;
                for(x = 0; x < (mBgPointCloud.cols) - 1;
                    ++x) // -1 da in zukunft getestet wird, dadurch letztes pixel ungeprueft
                {
                    if(vorPtr == NULL && bgPcData[2] != -1 && bgPcData[5] == -1) // eine unbestimmte gap beginnt
                        vorPtr = bgPcData;
                    else if(nachPtr == NULL && bgPcData[2] == -1 && bgPcData[5] != -1) // eine unbestimmte gap endet
                    {
                        if(vorPtr != NULL) // innenliegende gap
                            nachPtr = bgPcData + 3;
                        else // zeilenbeginn
                        {
                            for(fPtr = bgyPcData; fPtr < bgPcData + 3; fPtr += 3)
                            {
                                fPtr[2] = bgPcData[5];
                            }
                        }
                    }
                    if(vorPtr != NULL && nachPtr != NULL) // gap bestimmt, fuellung mit interpolerten werten
                    {
                        // debout << vorPtr[2] <<endl;
                        // debout << nachPtr[2] <<endl;
                        step =
                            ((nachPtr[2] - vorPtr[2]) /
                             (nachPtr - vorPtr)); // lange zeit war falsch: /9.; // /3 ein /3 durch (fPtr-vorPtr) und
                                                  // nicht (fPtr-vorPtr)/3 in schleife
                        for(fPtr = vorPtr + 3; fPtr < nachPtr; fPtr += 3)
                        {
                            // fPtr[0] = ; // x
                            // fPtr[1] = ; // y
                            fPtr[2] = vorPtr[2] + (fPtr - vorPtr) * step;
                            // debout << fPtr[2] <<endl;
                        }
                        vorPtr = nachPtr = NULL;
                    }
                    bgPcData += 3;
                }
                if(vorPtr != NULL) // gap am zeilenende
                {
                    for(fPtr = vorPtr; fPtr < bgPcData; fPtr += 3)
                    {
                        fPtr[2] = vorPtr[-1];
                    }
                }
                bgPcData = (bgyPcData += mBgPointCloud.step1());
            }

            mForeground.create(cv::Size(img.cols, img.rows), CV_8UC1);
            //            mForeground = cvCreateImage(cvSize(img->width, img->height), IPL_DEPTH_8U, 1); // CV_8UC1 8, 1
        }

        else // nicht stereo
        {
            // GaussBGStatModel
            // ---------------------------------------------------------------------------------------------------------------------


            if(!mBgModel.empty())
            {
                mBgModel->clear();
            }

            mBgModel = cv::createBackgroundSubtractorMOG2();

            mForeground.create(cv::Size(img.cols, img.rows), CV_8UC1);

            mBgModel->apply(img, mForeground, 1);

#ifdef SHOW_TMP_IMG
            namedWindow("BackgroundFilter");
            imshow("BackgroundFilter", mForeground);
            waitKey();
#endif

            // FGDStatModel
            // ---------------------------------------------------------------------------------------------------------------------

            // DOCUMENTATION: http://opencv.willowgarage.com/wiki/VideoSurveillance

        } // nicht stereo
    }
    else // nicht initialisierung
    {
        // SteroBild beruecksichtigen nur disparity
        // --------------------------------------------------------------------------------------------------------
        if(*stereoContext())
        {
            // fuer update waere bei stereo denkbar: mittelwert der disp/zwerte, aber Achtung: invalidDisp
            // beruecksichtigen!


            //                unsigned short* bgData = (unsigned short*) bgDisparity->imageData; // disparity zum
            //                reset-Zeitpunkt (Hintergrund) unsigned short* bgyData = bgData;

            //                IplImage *disparity = (*stereoContext())->getDisparity();
            //                unsigned short* data = (unsigned short*) disparity->imageData;
            //                unsigned short* yData = data;

            //                int x,y;

            //                for (y = 0; y < bgDisparity->height; ++y)
            //                {
            //                    for (x = 0; x < bgDisparity->width; ++x)
            //                    {
            //                        if ((*stereoContext())->dispValueValid(*data) &&
            //                        (*stereoContext())->dispValueValid(*bgData))
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

            // SteroBild beruecksichtigen z-wert
            // --------------------------------------------------------------------------------------------------------

            int x, y;

            float *bgPcData  = (float *) mBgPointCloud.data;
            float *bgyPcData = bgPcData;

            cv::Mat pointCloud = (*stereoContext())->getPointCloud();
            float  *pcData     = (float *) pointCloud.data;
            float  *yPcData    = pcData;

            mForeground = cv::Scalar(0, 0, 0);
            //                cvSet(mForeground, CV_RGB(0,0,0));
            //                cvFillImage(mForeground, 0); //um vorherige foreground-berechungen zu vergessen

            for(y = 0; y < pointCloud.rows; ++y)
            {
                for(x = 0; x < pointCloud.cols; ++x)
                {
                    if((bgPcData[2] != -1) && (pcData[2] != -1))
                    {
                        if((bgPcData[2] - pcData[2]) >
                           FOREGROUND_DISTANCE) // z-Wert in m (nicht cm!) wenn z-wert 1m unter defaultgroesse
                        {
                            mForeground.data[y * pointCloud.cols + x] = 1;
                        }
                        // if (y == 488)
                        //    debout << bgPcData[2] << " " << pcData[2] <<endl;
                    }
                    pcData += 3;
                    bgPcData += 3;
                }
                pcData   = (yPcData += pointCloud.step1());
                bgPcData = (bgyPcData += mBgPointCloud.step1());
            }
        }
        else // nicht stereo
        {
            // GaussBGStatModel
            // ---------------------------------------------------------------------------------------------------------------------


            mBgModel->apply(img, mForeground, update() ? -1 : 0);

#ifdef SHOW_TMP_IMG
            imshow("BackgroundFilter", img);
            waitKey();
#endif
        }
#ifdef SHOW_TMP_IMG
        imshow("BackgroundFilter", mForeground);
        waitKey();
#endif
        // einfache methode, um kleine gebiete in maske zu eliminieren und ausfransungen zu entfernen
        // ---------------------------------------------------

        cv::erode(
            mForeground, mForeground, cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3)), cv::Point(-1, -1), 1);
#ifdef SHOW_TMP_IMG
        imshow("BackgroundFilter", mForeground);
        waitKey();
#endif
        cv::dilate(
            mForeground, mForeground, cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3)), cv::Point(-1, -1), 2);
#ifdef SHOW_TMP_IMG
        imshow("BackgroundFilter", mForeground);
        waitKey();
#endif
        cv::GaussianBlur(mForeground, mForeground, cv::Size(11, 11), 3.5, 3.5);
#ifdef SHOW_TMP_IMG
        imshow("BackgroundFilter", mForeground);
        waitKey();
#endif
        cv::threshold(mForeground, mForeground, 20, 255, cv::THRESH_BINARY);

#ifdef SHOW_TMP_IMG
        imshow("BackgroundFilter", mForeground);
        waitKey();
#endif
        // loecher schliessen
        // ------------------------------------------------------------------------------------------------------------
        // floodfill den aussenbereich und suchen nach ungefuelltem koennte auch loechder finden alssen
        std::vector<std::vector<cv::Point>> contours;
        double                              contourArea;
        // find contours and store them all as a list
        cv::findContours(mForeground, contours, cv::RETR_LIST, cv::CHAIN_APPROX_SIMPLE);

        // test each contour

        while(!contours.empty())
        {
            std::vector<cv::Point> contour = contours.back();

            contourArea = cv::contourArea(contour, true);
            if(contourArea > 0 && contourArea < 400) // kleine innere loecher schliessen
            {
                // Get contour point set.
                cv::fillPoly(mForeground, cv::Mat(contour), cv::Scalar::all(1));
            }
            else if(contourArea > MIN_FOREGROUND_AREA - 400) // kleine Bereiche werden eliminiert, dies koennte in
                                                             // Abhaengigkeit von der disparity gemacht werden!!!!!!!!
            {
                // Get contour point set.
                cv::fillPoly(mForeground, cv::Mat(contour), cv::Scalar::all(0));
            }
            // take the next contour
            contours.pop_back();
        }

#ifdef SHOW_TMP_IMG
        imshow("BackgroundFilter", mForeground);
        waitKey();
#endif
    }

    return res;
}

// setzt in mat alle bg pixel auf val
// bei mehreren Kanaelen nur den letzten Kanal
// bisher nur fuer float
void BackgroundFilter::maskBg(cv::Mat &mat, float val)
{
    if(getEnabled() && !mForeground.empty())
    {
        int x, y;

        int    lineLen = mat.cols / sizeof(float);
        int    ch      = lineLen / mat.cols;
        float *pcData  = ((float *) mat.data) + ch - 1; // ch-1 um beim letzten element zu stehen (wie pcData[2])
        float *yPcData = pcData;

        unsigned char *fgData  = (unsigned char *) mForeground.data;
        unsigned char *yFgData = fgData;

        for(y = 0; y < mat.rows; ++y)
        {
            for(x = 0; x < mat.cols; ++x)
            {
                // 0 background, 1 foreground
                if(!*fgData)
                {
                    *pcData = val;
                }
                ++fgData;
                pcData += ch;
            }
            pcData = (yPcData += lineLen);
            fgData = (yFgData += mForeground.step1());
        }
    }
}
