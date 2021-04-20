/*
 * PeTrack - Software for tracking pedestrians movement in videos
 * Copyright (C) 2010-2020 Forschungszentrum Jülich GmbH,
 * Maik Boltes, Juliane Adrian, Ricardo Martin Brualla, Arne Graf, Paul Häger, Daniel Hillebrand,
 * Deniz Kilic, Paul Lieberenz, Daniel Salden, Tobias Schrödter, Ann Katrin Seemann
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

#include <QPointF>
#include <QRect>
#include <opencv2/opencv.hpp>
#include <opencv2/aruco.hpp>
#include <bitset>

#include "recognition.h"
#include "markerCasern.h"
#include "markerHermes.h"
#include "markerJapan.h"
#include "colorMarkerItem.h"
#include "multiColorMarkerItem.h"
#include "colorMarkerWidget.h"
#include "multiColorMarkerWidget.h"
#include "codeMarkerItem.h"
#include "codeMarkerWidget.h"
#include "helper.h"
#include "tracker.h"
#include "control.h"
#include "recognitionRoiItem.h"

using namespace::cv;
using namespace std;

namespace reco {
using namespace detail;

//#include "Psapi.h"

//#define WITH_RECT
#define ELLIPSE_DISTANCE_TO_BORDER 10

//#define SHOW_TMP_IMG

//#define TIME_MEASUREMENT

struct ColorParameters
{
    int	h_low = 0;
    int	h_high = 359;
    int	s_low = 0;
    int	s_high = 255;
    int	v_low = 0;
    int	v_high = 255;
    bool inversHue = false;
};

/*!
 *  \brief  Apply a color threshold to an image.
 *
 *  \param  src The color image.
 *  \param  dst The binarized image.
 *  \param  param The parameters.
 *
 *  Each component H, S and V must be in a given range, defined by the parameters.
 */

void thresholdHSV (const Mat &src, Mat &bin, const ColorParameters &param)
{
//    IplImage *hsvIpl;
    int h, s, v;
    int x, y;

    Mat hsv {src.size(), src.type()};

//    hsvIpl = cvCloneImage(srcIpl); // make a copy

//    cvCvtColor(srcIpl ,hsvIpl, CV_BGR2HSV); // convert to HSV color space

    cvtColor(src, hsv, COLOR_BGR2HSV);

//    unsigned char* dataImg = ((unsigned char*) hsvIpl->imageData);
//    unsigned char* yDataImg = dataImg;
//    unsigned char* data = ((unsigned char*) binIpl->imageData);
//    unsigned char* yData = data;

    for (y = 0; y < bin.rows/*binIpl->height*/; ++y)
    {
        for (x = 0; x < bin.cols/*binIpl->width*/; ++x)
        {
            Vec3b intensity = hsv.at<Vec3b>(Point(x,y));
            h = intensity.val[0];
            s = intensity.val[1];
            v = intensity.val[2];
//            debout << "h:" << h << ", s:" << s << ", v:" << v << endl;
//            h = dataImg[0];
//            s = dataImg[1];
//            v = dataImg[2];
//            debout << "h:" << h << ", s:" << s << ", v:" << v << endl;

            // apply the thresholds
            if ((!param.inversHue && (h < param.h_low || param.h_high < h)) ||
                (param.inversHue  && (h >= param.h_low && param.h_high >= h)) ||
                (s < param.s_low || param.s_high < s) ||
                (v < param.v_low || param.v_high < v))
            {
                bin.at<uchar>(Point(x, y)) = 0;
//                *data = 0;
            }else
            {
                bin.at<uchar>(Point(x, y)) = 255;
//                *data = 255;
            }
//            ++data;
//            dataImg+=3;
        }
//        data = (yData += binIpl->widthStep/sizeof(char)); //width);
//        dataImg = (yDataImg += hsvIpl->widthStep/sizeof(char)); //width);
    }
//    cvNamedWindow("hsv", CV_WINDOW_AUTOSIZE ); // 0 wenn skalierbar sein soll
//    cvShowImage("hsv", hsv);
//    binIpl = cvCreateImage(cvSize(bin.cols,bin.rows),8,3);
//    IplImage tmpIpl = bin;
//    cvCopy(&tmpIpl,binIpl);

//    cvReleaseImage(&hsvIpl);
}
#ifndef STEREO_DISABLED
void thresholdHSV (const IplImage *srcIpl, IplImage *binIpl, const ColorParameters &param)
{
    const Mat src = cvarrToMat(srcIpl);
    Mat bin = cvarrToMat(binIpl);
    thresholdHSV(src,bin,param);
}
#endif


void setColorParameter(const QColor &fromColor, const QColor &toColor, bool inversHue, ColorParameters	&param)
{
    if (fromColor.hue() > toColor.hue())
    {
        param.h_low  = toColor.hue();
        param.h_high = fromColor.hue();
    }
    else
    {
        param.h_low  = fromColor.hue();
        param.h_high = toColor.hue();
    }
    // attention: full range of V is [0,1] or [0.256),
    // depending on the range of the RGB image
    //param.h_low *= 255./359.;
    //param.h_high *= 255./359.;
    // hue scheint nach Konversion bgr nach hsv in opencv nur zwischen 0..179 zu laufen
    param.h_low /= 2;
    param.h_high /= 2;
    if (fromColor.saturation() > toColor.saturation())
    {
        param.s_low  = toColor.saturation();
        param.s_high = fromColor.saturation();
    }
    else
    {
        param.s_low  = fromColor.saturation();
        param.s_high = toColor.saturation();
    }
    if (fromColor.value() > toColor.value())
    {
        param.v_low  = toColor.value();
        param.v_high = fromColor.value();
    }
    else
    {
        param.v_low  = fromColor.value();
        param.v_high = toColor.value();
    }
    param.inversHue = inversHue;
}

//A good starting point would be GetProcessMemoryInfo, which reports various memory info about the specified process.
//You can pass GetCurrentProcess() as the process handle in order to get information about the calling process.
//Probably the WorkingSetSize member of PROCESS_MEMORY_COUNTERS is the closest match to the Mem Usage coulmn in task manager,
//but it is not going to be exactly the same.
//I would experiment with the different values to find the one that is closest to your needs.
////Usage:
//#include "windows.h"
//#include "psapi.h"
//    PROCESS_MEMORY_COUNTERS pmc;
//    GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc));
//    debout << pmc.WorkingSetSize/1000 <<"KB Usage of the process"<< endl;
////    typedef struct _PROCESS_MEMORY_COUNTERS {
////        DWORD cb;
////        DWORD PageFaultCount;
////        DWORD PeakWorkingSetSize;
////        DWORD WorkingSetSize;
////        DWORD QuotaPeakPagedPoolUsage;
////        DWORD QuotaPagedPoolUsage;
////        DWORD QuotaPeakNonPagedPoolUsage;
////        DWORD QuotaNonPagedPoolUsage;
////        DWORD PagefileUsage;
////        DWORD PeakPagefileUsage;
////    } PROCESS_MEMORY_COUNTERS,*PPROCESS_MEMORY_COUNTERS;


/**
 * @brief calculates pixel-displacement due to oblique/angular view
 *
 * Relevant for Color Markers. boxImageCentre without border. More information: Dissertation Maik pp. 138
 *
 * @param boxImageCentre
 * @param controlWidget
 * @return
 */
Vec2F autoCorrectColorMarker(Vec2F &boxImageCentre, Control *controlWidget)
{
    Petrack *mainWindow = controlWidget->getMainWindow();
    Point2f tp = mainWindow->getExtrCalibration()->getImagePoint(
                Point3f(-controlWidget->getCalibCoord3DTransX()-controlWidget->getCalibExtrTrans1(),
                        -controlWidget->getCalibCoord3DTransY()-controlWidget->getCalibExtrTrans2(), 0));
    Vec2F pixUnderCam(tp.x, tp.y); // CvPoint
    Vec2F boxImageCentreWithBorder = boxImageCentre;
    boxImageCentreWithBorder += Vec2F(mainWindow->getImageBorderSize(), mainWindow->getImageBorderSize());
    pixUnderCam += Vec2F(mainWindow->getImageBorderSize(), mainWindow->getImageBorderSize());
    float angle = 90 - mainWindow->getImageItem()->getAngleToGround(boxImageCentreWithBorder.x(), boxImageCentreWithBorder.y(), 175);// Hoehe 175 cm ist egal, da auf jeder Hoehe gleicher Winkel
    //debout << "Bordersize: " << mainWindow->getImageBorderSize() <<endl;
    //debout << "Angle: " << angle <<endl;
    //debout << "Pixel unter der Camera: " << pixUnderCam.x() << ", " << pixUnderCam.y() << endl;

    //QPointF cmPerPixel = mainWindow->getImageItem()->getCmPerPixel(boxImageCentre.x(), boxImageCentre.y(), 175); // durchschnittsgroesse von 175cm angenommen

    Vec2F moveDir = boxImageCentreWithBorder-pixUnderCam;
    moveDir.normalize();
    //debout << "Movedir: " << moveDir <<endl;

    Point3f p3x1, p3x2;
    p3x1 = mainWindow->getExtrCalibration()->get3DPoint(Point2f(boxImageCentre.x(),boxImageCentre.y()),175);
    p3x2 = mainWindow->getExtrCalibration()->get3DPoint(Point2f(boxImageCentre.x()+moveDir.x(),boxImageCentre.y()+moveDir.y()),175);
    p3x1 = p3x1-p3x2;
    Vec2F cmPerPixel(p3x1.x, p3x1.y);

    //debout << "cmPerPixel: " << p3x1 <<endl;
    //debout << "Boximgcentre: " <<boxImageCentreWithBorder.x()  <<" "<< boxImageCentreWithBorder.y() <<endl<<endl;

    return (0.12*angle/cmPerPixel.length())*moveDir; // Maik Dissertation Seite 138
}

/**
 * @brief Detects and filters colorBlobs in the given image
 *
 * This function takes the given colors from the color map and
 * searches for blobs in this color. This is done by thresholding and
 * detecting contours. To reduce noise, open and close operations can be
 * performed.
 *
 * These contours are then checkefor whether they fulfill following criteria:
 * 1. Area inside user-specified bounds
 * 2. Ratio of sides of the bounding rect inside user-specified bounds
 * 3. Blob is not at the edge of the image
 *
 * @param options all options, mostly forwarded from the UI
 * @return vector of detected ColorBlobs (see struct ColorBlob for details)
 */
[[nodiscard]] std::vector<ColorBlob> detail::findColorBlob(const ColorBlobDetectionParams &options){
    std::vector<ColorBlob> colorBlobs;
    ColorParameters	param;

    setColorParameter(options.fromColor, options.toColor, options.invHue, param);

    cv::Mat img = options.img;
    cv::Mat binary = options.binary;

    // color thresholding
    thresholdHSV(img , binary, param);

    // close small holes: radius ( hole ) < radius ( close )
    if (options.useClose)
    {
        const int radiusClose = options.radiusClose;
        cv::morphologyEx(binary,binary,MORPH_CLOSE,getStructuringElement(MORPH_ELLIPSE,Size(2*radiusClose+1, 2*radiusClose+1),Point(radiusClose, radiusClose)));
    }

    // remove small blobs: radius ( blob ) < radius ( open )
    if (options.useOpen)
    {
        const int radiusOpen = options.radiusOpen;
        cv::morphologyEx(binary,binary,MORPH_OPEN,getStructuringElement(MORPH_ELLIPSE,Size(2*radiusOpen+1, 2*radiusOpen+1),Point(radiusOpen, radiusOpen)));
    }

    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(binary,contours,cv::RETR_EXTERNAL,cv::CHAIN_APPROX_SIMPLE);

    for(const auto& contour : contours){
        double area = cv::contourArea(contour);

        cv::RotatedRect box = cv::minAreaRect(contour);

        float maxExpansion;
        double ratio;
        if (box.size.height > box.size.width)
        {
            ratio = static_cast<double>(box.size.height/box.size.width);
            maxExpansion = box.size.height;
        }
        else
        {
            ratio = static_cast<double>(box.size.width/box.size.height);
            maxExpansion = box.size.width;
        }

        // contour at border of roi, than neglect because of object going out of region has moving center
        bool atEdge = std::any_of(contour.cbegin(), contour.cend(), [&](const cv::Point& p){
            return (p.x <= 1) || (p.x >= img.cols-2) ||
                   (p.y <= 1) || (p.y >= img.rows-2);
        });

        if(atEdge){
            continue;
        }
        if(area < options.minArea || area > options.maxArea){
            continue;
        }
        if(ratio > options.maxRatio){
            continue;
        }

        int cx = static_cast<int>(std::lround(box.center.x));
        int cy = static_cast<int>(std::lround(box.center.y));
        int add = static_cast<int>(min(box.size.height,box.size.width)/4);

        QColor col, col2;
        // color for hat measured at 3 points
        col.setRgb(getValue(img,cx,cy).rgb());
        col2.setRgb(getValue(img,cx+add,cy).rgb());
        if (col.value()<col2.value()) // take brighter color
        {
            col = col2;
        }
        col2.setRgb(getValue(img,cx-add,cy).rgb());
        if (col.value()<col2.value()) // take brighter color
        {
            col = col2;
        }

        // centre of rect in the coordinates of the whole image (instead of ROI)
        Vec2F boxImageCentre = options.offset;
        boxImageCentre.setX(boxImageCentre.x()+static_cast<double>(box.center.x));
        boxImageCentre.setY(boxImageCentre.y()+static_cast<double>(box.center.y));

        // TODO maybe use move constructor to increase perf for contour(then not const anymore)
        colorBlobs.push_back({box, boxImageCentre, col, contour, static_cast<double>(maxExpansion)});
    }
    return colorBlobs;
}

/**
 * @brief Restricts the position in which the black dot should be according to position in image.
 *
 * @param[in] blob ColorBlob for which to restrict the position
 * @param[in] mainWindow pointer to Petrack for getImageItem
 * @param[in] bS bordersize
 * @param[in,out] cropRect cropRect to restrict/resize
 */
void detail::restrictPositionBlackDot(ColorBlob& blob, ImageItem *imageItem, int bS, cv::Rect& cropRect)
{
    double xy,x1, x2, y1, y2;

    Vec2F &boxImageCentre = blob.imageCenter;
    xy=imageItem->getAngleToGround(boxImageCentre.x()+bS, boxImageCentre.y()+bS, 175);
    x1=imageItem->getAngleToGround(boxImageCentre.x()+bS+10, boxImageCentre.y()+bS, 175);
    x2=imageItem->getAngleToGround(boxImageCentre.x()+bS-10, boxImageCentre.y()+bS, 175);
    y1=imageItem->getAngleToGround(boxImageCentre.x()+bS, boxImageCentre.y()+bS+10, 175);
    y2=imageItem->getAngleToGround(boxImageCentre.x()+bS, boxImageCentre.y()+bS-10, 175);

    double subFactorBig = 1.-.75*(90.-xy)/90.; //  -.5 //in 1.0..0.25 // xy in 0..90
    constexpr double subFactorSmall = .85; // .9
    if (x1>x2) // Person links von Achse
    {
        if (y1>y2) // Person links oben von Achse
        {
            if (x1>y1) // starker Beschnitt rechts, kleiner Beschnitt unten
            {
                cropRect.width *= subFactorBig; // ist int
                cropRect.height *= subFactorSmall; // ist int
            }
            else // starker Beschnitt unten, kleiner Beschnitt rechts
            {
                cropRect.width *= subFactorSmall; // ist int
                cropRect.height *= subFactorBig; // ist int
            }
        }
        else // Person links unten von Achse
        {
            if (x1>y2) // starker Beschnitt rechts, kleiner Beschnitt oben
            {
                cropRect.y = cropRect.y+cropRect.height*(1-subFactorSmall);
                cropRect.width *= subFactorBig; // ist int
                cropRect.height *= subFactorSmall; // ist int
            }
            else // starker Beschnitt oben, kleiner Beschnitt rechts
            {
                cropRect.y = cropRect.y+cropRect.height*(1-subFactorBig);
                cropRect.width *= subFactorSmall; // ist int
                cropRect.height *= subFactorBig; // ist int
            }
        }
    }
    else // Person rechts von Achse
    {
        if (y1>y2) // Person rechts oben von Achse
        {
            if (x2>y1) // starker Beschnitt links, kleiner Beschnitt unten
            {
                cropRect.x = cropRect.x+cropRect.width*(1-subFactorBig);
                cropRect.width *= subFactorBig; // ist int
                cropRect.height *= subFactorSmall; // ist int
            }
            else // starker Beschnitt unten, kleiner Beschnitt links
            {
                cropRect.x = cropRect.x+cropRect.width*(1-subFactorSmall);
                cropRect.width *= subFactorSmall; // ist int
                cropRect.height *= subFactorBig; // ist int
            }
        }
        else // Person rechts unten von Achse
        {
            if (x2>y2) // starker Beschnitt links, kleiner Beschnitt oben
            {
                cropRect.x = cropRect.x+cropRect.width*(1-subFactorBig);
                cropRect.y = cropRect.y+cropRect.height*(1-subFactorSmall);
                cropRect.width *= subFactorBig; // ist int
                cropRect.height *= subFactorSmall; // ist int
            }
            else // starker Beschnitt oben, kleiner Beschnitt links
            {
                cropRect.x = cropRect.x+cropRect.width*(1-subFactorSmall);
                cropRect.y = cropRect.y+cropRect.height*(1-subFactorBig);
                cropRect.width *= subFactorSmall; // ist int
                cropRect.height *= subFactorBig; // ist int
            }
        }
    }
}

/**
 * @brief Custom BGR to Gray conversion weighted via midHue
 *
 * This conversion is used instead of cvtColor for the detection of the
 * black dot, since cvtColor makes red result in too dark shade.
 *
 * @param subImg BGR image to convert into Grayscale
 * @param midHue midHue used for weigthed conversion
 * @return image converted to grayscale
 */
cv::Mat detail::customBgr2Gray(const cv::Mat& subImg, const QColor& midHue)
{
    cv::Mat subGray;

    float scaleR = midHue.redF();
    float scaleG = midHue.greenF();
    float scaleB = midHue.blueF();
    float scaleSum = scaleR+scaleG+scaleB;
    scaleR/=scaleSum;
    scaleG/=scaleSum;
    scaleB/=scaleSum;
    cv::Mat bgrToGray = (Mat_<double>(1,3) << scaleB, scaleG, scaleR);

    // Performs the matrix-vector multiplication MxN for each pixel of subImg, where M is bgrToGray
    // and N is a vector of all channels of a pixel, the result will be saved in subGray.
    cv::transform(subImg, subGray, bgrToGray);
    return subGray;
}

/**
 * @brief Refines the detection of multicolor-markers with a black dot
 *
 * A black dot is searched inside the bounding rect of the detected head.
 * This bounding rect might be resized according to the position in the image,
 * when the restrict posititon option is enabled.
 *
 * If a black dot is found, its position is used instead of the multcolor-detection.
 * This is saved with a quality of 100, as with contour markers. When the black dot
 * is not found, the ColorBlob might still count as detection, if the ignoreWithoutMarker
 * option is disabled.
 *
 * @param blobs detected color blobs
 * @param img img in which the color blobs were detected
 * @param crossList list of all detected people
 * @param options BlackDotOptions struct mostly forwarding all options from UI
 */
void detail::refineWithBlackDot(std::vector<ColorBlob>& blobs, const cv::Mat& img, QList<TrackPoint>& crossList, const BlackDotOptions& options)
{
    constexpr int border=4; // zusaetzlicher rand um subrects
    const int bS = options.borderSize;
    const bool restrictPosition = options.restrictPosition;
    ImageItem *imageItem = options.imageItem;
    const QColor midHue = options.midHue;
    const double dotSize = options.dotSize;
    const bool ignoreWithoutMarker = options.ignoreWithoutMarker;
    const bool autoCorrect = options.autoCorrect;
    const bool autoCorrectOnlyExport = options.autoCorrectOnlyExport;
    Control *controlWidget = options.controlWidget;

    for(ColorBlob& blob : blobs)
    {
        cv::Rect cropRect;
        cv::RotatedRect &box = blob.box;
        cropRect.x = max(1, myRound(box.center.x-box.size.width/2-border));
        cropRect.y = max(1, myRound(box.center.y-box.size.height/2-border));
        // 1. rundet kaufmaennisch, 2. dann rundet zur naechst kleiner geraden zahl
        // min wegen bildrand
        cropRect.width = min(img.cols-cropRect.x-1, 2*border+(myRound(blob.maxExpansion) & -2));
        cropRect.height = min(img.rows-cropRect.y-1, 2*border+(myRound(blob.maxExpansion) & -2));//cropRect.height = cropRect.width;

        if (restrictPosition)
        {
            restrictPositionBlackDot(blob, imageItem, bS, cropRect);
        }

        // cvtColor results in really dark images, especially with red shades
        // so using custom conversion weighted by midHue
        cv::Mat subImg = img(cropRect);
        cv::Mat subGray = customBgr2Gray(subImg, midHue);
        cv::Mat subBW;

        double maxThreshold = max(max(getValue(subGray, subGray.cols/2, subGray.rows/2).value(),
                               getValue(subGray, subGray.cols/4, subGray.rows/2).value()),
                           getValue(subGray, 3*subGray.cols/4, subGray.rows/2).value());
        int step = static_cast<int>((maxThreshold-5))/5;
        int minGrey = 300;
        Vec2F subCenter;
        for (int threshold = 5; threshold < maxThreshold; threshold += step)
        {
            cv::threshold(subGray,subBW,threshold,255,cv::THRESH_BINARY);

            // find contours and store them all as a list
            std::vector<std::vector<cv::Point>> subContours;
            cv::findContours(subBW,subContours,cv::RETR_LIST,cv::CHAIN_APPROX_SIMPLE);

            // test each contour
            for(auto& subContour : subContours)
            {
                if (subContour.size() > 5) // This is number point in contour
                {
                    cv::RotatedRect subBox = cv::minAreaRect(subContour);
                    double subRatio;
                    double subMaxExpansion;
                    if (subBox.size.height > subBox.size.width)
                    {
                        subRatio = subBox.size.height/subBox.size.width;
                        subMaxExpansion = subBox.size.height;
                    }
                    else
                    {
                        subRatio = subBox.size.width/subBox.size.height;
                        subMaxExpansion = subBox.size.width;
                    }

                    QPointF cmPerPixel =imageItem->getCmPerPixel(cropRect.x+subBox.center.x, cropRect.y+subBox.center.y, controlWidget->mapDefaultHeight->value());
                    double cmPerPixelAvg = (cmPerPixel.x()+cmPerPixel.y())/2.;
                    double markerSize = dotSize/cmPerPixelAvg;//war: 5cm// war WDG: = 16; war GymBay: = headSize / 4.5;

                    // maximalseitenverhaeltnis && minimal und maximaldurchmesser
                    if (subRatio < 1.8 && subMaxExpansion < markerSize*1.5 && subMaxExpansion > markerSize/2)//1.5
                    {
                        double subContourArea = cv::contourArea(subContour,true);
                        int cx = myRound(subBox.center.x);
                        int cy = myRound(subBox.center.y);

                        // darker inside && dark inside &&  mittelpunkt in kopfkontur
                        int xygrey = getValue(subGray, cx, cy).value();
                        //                                    debout << "xygrey: " << xygrey << endl;
                        if (subContourArea>0 && xygrey<min(150.,2*maxThreshold/3) && (0<cv::pointPolygonTest(blob.contour,Point2f(cropRect.x+subBox.center.x, cropRect.y+subBox.center.y),false))) // dark inside
                        {
                            if (minGrey > xygrey)
                            {
                                minGrey = xygrey;
                                subCenter.set(cropRect.x+subBox.center.x, cropRect.y+subBox.center.y);
                            }
                        }
                    }
                }
            }
        }

        if (minGrey <260) // mit gefundenem schwarzem punkt
        {
            crossList.append(TrackPoint(subCenter, 100, Vec2F(box.center.x, box.center.y), blob.color)); // 100 beste qualitaet
        }
        else if (!ignoreWithoutMarker)
        {
            if (autoCorrect && !autoCorrectOnlyExport)
            {
                Vec2F moveDir = autoCorrectColorMarker(blob.imageCenter, controlWidget);

                crossList.append(TrackPoint(Vec2F(box.center.x, box.center.y)+moveDir, 100, Vec2F(box.center.x, box.center.y), blob.color)); // 100 beste qualitaet
            }
            else
                crossList.append(TrackPoint(Vec2F(box.center.x, box.center.y), 90, Vec2F(box.center.x, box.center.y), blob.color)); // 100 beste qualitaet
        }

    }
}

/**
 * @brief Refined colorBlobs with Aruco Markers
 *
 * An aruco marker inside the detected bounding rect is searched. If it is found,
 * the position of the marker is taken for the detected person. This results in a
 * quality of 100 as with contour markers.
 *
 * If no valid aruco marker is detected, the colorBlob might still count as person,
 * if the ignoreWithoutMarker option is disabled. Missing frames where code is not
 * recognized are interpolated in trackerReal.cpp and Marker ID is set to -1
 *
 * @param blobs detected color blobs
 * @param img img in which the color blobs were detected
 * @param crossList list of all detected people
 * @param options all options, mostly forwarded from the GUI
 */
void detail::refineWithAruco(std::vector<ColorBlob> &blobs, const cv::Mat& img, QList<TrackPoint> &crossList, ArucoOptions& options)
{
    constexpr int border=4; // zusaetzlicher rand um subrects

    Control *controlWidget = options.controlWidget;
    bool ignoreWithoutMarker = options.ignoreWithoutMarker;
    bool autoCorrect = options.autoCorrect;
    bool autoCorrectOnlyExport = options.autoCorrectOnlyExport;

    for(ColorBlob& blob : blobs)
    {
        // cropRect has coordinates of rechtangele around color blob with respect to lower left corner (as in the beginning of useBlackDot)
        cv::Rect cropRect;
        const cv::RotatedRect &box = blob.box;
        int extendRect = myRound(blob.maxExpansion*.3); // scalar to increase area of cropRect for better detection of codemarkers when marker appears to stick out of the colored head because of tilted heads; value of .3 chosen arbitrarily after discussion
        cropRect.x = max(1, myRound(box.center.x-box.size.width/2.-border));
        cropRect.y = max(1, myRound(box.center.y-box.size.height/2.-border));
        cropRect.width = min(img.cols-cropRect.x-extendRect-1, 2*border+((myRound(blob.maxExpansion)+extendRect) & -2));
        cropRect.height = min(img.rows-cropRect.y-extendRect-1, 2*border+((myRound(blob.maxExpansion)+extendRect) & -2));
        cv::Mat subImg = img(cropRect); // --> shallow copy (points to original data)

        int lengthini = crossList.size(); // initial length of crossList (before findCodeMarker() is called)

        Vec2F offsetCropRect2Roi; // needed for drawing detected ArucoCode-Candidates correctly -> passed on to findCodeMarker()-Function
        offsetCropRect2Roi.setX(cropRect.x);
        offsetCropRect2Roi.setY(cropRect.y);

        if (subImg.empty())
            continue;
        findCodeMarker(subImg, &crossList, controlWidget, offsetCropRect2Roi);

        // The next three statements each:
        // - set the offset of subImg with regards to ROI //(ROI to original image is archieved later in the code for all methods)
        // - add the functionality of autocorrection
        // - deal with functionality of ignore/not ignore heads without identified ArucoMarker
        if (ignoreWithoutMarker && lengthini!=crossList.size()) // if CodeMarker-Call returns crossList containing a new element (identified the ArucoMarker)
        {
            Vec2F moveDir;
            if (autoCorrect && !autoCorrectOnlyExport)
            {
                moveDir = autoCorrectColorMarker(blob.imageCenter, controlWidget);
            }
            else
            {
                moveDir = Vec2F(0, 0);
            }
            crossList.back().setCol(blob.color);
            crossList.back().setColPoint(Vec2F(box.center.x, box.center.y));
            crossList.back() = crossList.back() + (Vec2F(cropRect.x, cropRect.y) + moveDir);
        }
        else if(!ignoreWithoutMarker && (lengthini==crossList.size())) // in case ignoreWithoutMarker is checked and CodeMarker-Call returns empty crossList (could not identify a marker) the center of the smallest rectangle around the colorblobb is used as position
        {
            offsetCropRect2Roi.setX(0); // set to zero as cooridinates are directly used from cropRect
            offsetCropRect2Roi.setY(0);

            Vec2F moveDir;
            if (autoCorrect && !autoCorrectOnlyExport)
            {
                moveDir = autoCorrectColorMarker(blob.imageCenter, controlWidget);
            }
            else
            {
                moveDir = Vec2F(0,0);
            }
            crossList.append(TrackPoint(Vec2F(box.center.x, box.center.y) + moveDir, 90, Vec2F(box.center.x, box.center.y), blob.color)); // 100 beste qualitaet
        }
        else if(!ignoreWithoutMarker && (crossList.size()!=lengthini)) // in case ignoreWithoutMarker is checked and CodeMarker-Call returns non-empty crossList (could identify a marker)
        {
            Vec2F moveDir;
            if (autoCorrect && !autoCorrectOnlyExport)
            {
                moveDir = autoCorrectColorMarker(blob.imageCenter, controlWidget);
            }
            else
            {
                moveDir = Vec2F(0, 0);
            }
            crossList.back().setCol(blob.color);
            crossList.back().setColPoint(Vec2F(box.center.x, box.center.y));
            crossList.back() = crossList.back() + (Vec2F(cropRect.x, cropRect.y) + moveDir);
        }
    }
}

/**
 * @brief Run color blob detection on an image to find features
 *
 * Runs color thresholding to binarize the image and computes connected components.
 * The features are the center of gravity of each connected component.
 * offset is corner of roi.
 *
 * @param img
 * @param crossList
 * @param controlWidget
 * @param ignoreWithoutMarker (is ignored->overwritten by cmWidget->ignoreWithoutDot->isChecked())
 * @param offset
 */
void findMultiColorMarker(Mat &img, QList<TrackPoint> *crossList, Control *controlWidget, bool ignoreWithoutMarker, Vec2F &offset)
{
    Petrack *mainWindow = controlWidget->getMainWindow();
    MultiColorMarkerItem* cmItem = mainWindow->getMultiColorMarkerItem();
    MultiColorMarkerWidget* cmWidget = mainWindow->getMultiColorMarkerWidget();
    int bS = mainWindow->getImageBorderSize();

    RectPlotItem *rectPlotItem = controlWidget->getColorPlot()->getMapItem();
    bool useClose = cmWidget->useClose->isChecked();
    bool useOpen = cmWidget->useOpen->isChecked();
    int radiusClose = cmWidget->closeRadius->value();
    int radiusOpen  = cmWidget->openRadius->value();
    double dotSize = cmWidget->dotSize->value();
    int minArea = cmWidget->minArea->value(), maxArea = cmWidget->maxArea->value();
    double maxRatio = cmWidget->maxRatio->value();
    bool useBlackDot = cmWidget->useDot->isChecked();
    bool useCodeMarker = cmWidget->useCodeMarker->isChecked();
    bool restrictPosition = cmWidget->restrictPosition->isChecked();
    ignoreWithoutMarker = cmWidget->ignoreWithoutDot->isChecked(); // ueberschreiben von uebergeordnetem ignoreWithoutMarker
    bool autoCorrect = cmWidget->autoCorrect->isChecked();
    bool autoCorrectOnlyExport = cmWidget->autoCorrectOnlyExport->isChecked();

    for (int j = 0; j < rectPlotItem->mapNum(); j++)
    {
        int nr;
        if (j == controlWidget->mapNr->value())
            nr = rectPlotItem->mapNum()-1;
        else if (j == rectPlotItem->mapNum()-1)
            nr = controlWidget->mapNr->value();
        else
            nr = j;

        ColorBlobDetectionParams param;
        param.fromColor = rectPlotItem->getMap(nr).fromColor();
        param.toColor = rectPlotItem->getMap(nr).toColor();
        param.invHue = rectPlotItem->getMap(nr).invHue();
        param.minArea = minArea;
        param.maxArea = maxArea;
        param.maxRatio = maxRatio;
        param.useClose = useClose;
        param.radiusClose = radiusClose;
        param.useOpen = useOpen;
        param.radiusOpen = radiusOpen;
        param.offset = offset;
        param.img = img;
        param.binary = cmItem->createMask(img.cols, img.rows);

        auto blobs = findColorBlob(param);


        if (useBlackDot)
        {
            ColorParameters colParam;
            setColorParameter(rectPlotItem->getMap(nr).fromColor(), rectPlotItem->getMap(nr).toColor(), rectPlotItem->getMap(nr).invHue(), colParam);
            // zentralen farbton heraussuchen
            QColor midHue;
            if (colParam.inversHue)
                midHue.setHsv(2*((colParam.h_low+(colParam.h_high-colParam.h_low)/2+90)%180),255,255);
            else
                midHue.setHsv(2*(colParam.h_low+(colParam.h_high-colParam.h_low)/2),255,255);

            BlackDotOptions options;
            options.ignoreWithoutMarker = ignoreWithoutMarker;
            options.autoCorrect = autoCorrect;
            options.autoCorrectOnlyExport = autoCorrectOnlyExport;
            options.restrictPosition = restrictPosition;
            options.borderSize = bS;
            options.midHue = midHue;
            options.dotSize = dotSize;
            options.controlWidget = controlWidget;
            options.imageItem = mainWindow->getImageItem();

            // adds to crosslist
            refineWithBlackDot(blobs, img, *crossList, options);
        }else if (useCodeMarker)
        {
            ArucoOptions options;
            options.autoCorrect = autoCorrect;
            options.autoCorrectOnlyExport = autoCorrectOnlyExport;
            options.ignoreWithoutMarker = ignoreWithoutMarker;
            options.controlWidget = controlWidget;

            // adds to crosslist
            refineWithAruco(blobs, img, *crossList, options);
        }else
        {
            Vec2F moveDir;
            for(ColorBlob& blob : blobs)
            {
                if (autoCorrect && !autoCorrectOnlyExport)
                {
                    moveDir = autoCorrectColorMarker(blob.imageCenter, controlWidget);

                    crossList->append(TrackPoint(Vec2F(blob.box.center.x, blob.box.center.y)+moveDir, 100, Vec2F(blob.box.center.x, blob.box.center.y), blob.color)); // 100 beste qualitaet
                }
                else
                {
                    crossList->append(TrackPoint(Vec2F(blob.box.center.x, blob.box.center.y), 100, Vec2F(blob.box.center.x, blob.box.center.y), blob.color)); // 100 beste qualitaet
                }
            }
        }
    }
}

/**
 * @brief Run color blob detection on an image to find features.
 *
 * Runs color thresholding to binarize the image and computes connected components.
 * The features are the center of gravity of each connected component.
 *
 * @param img
 * @param crossList
 * @param controlWidget
 */
void findColorMarker(Mat &img, QList<TrackPoint> *crossList, Control *controlWidget)
{
    ColorParameters	param;
    ColorMarkerItem*     cmItem = controlWidget->getMainWindow()->getColorMarkerItem();
    ColorMarkerWidget* cmWidget = controlWidget->getMainWindow()->getColorMarkerWidget();

    QColor fromColor = cmWidget->fromColor->palette().color(QPalette::Button);
    QColor   toColor = cmWidget->toColor->palette().color(QPalette::Button);

    setColorParameter(fromColor, toColor, cmWidget->inversHue->isChecked(), param);

    // run detection
    Mat	binary;
    //cv::Mat	kernel;
//    IplConvKernel* kernel;

#ifdef TIME_MEASUREMENT
    //        "==========: "
    debout << "start col : " << getElapsedTime() <<endl;
#endif

    // erzeuge speicherplatz fuer mask
    // abfrage wird in createMask gemacht
    //if (cmItem->getMask() == NULL || (cmItem->getMask() != NULL && (cmItem->getMask()->width != img->width || cmItem->getMask()->height != img->height)))
    binary = cmItem->createMask(img.cols, img.rows); // erzeugt binary mask mit groesse von img

    // color thresholding
    thresholdHSV(img , binary, param);

#ifdef SHOW_TMP_IMG
    namedWindow("image", CV_WINDOW_AUTOSIZE ); // 0 wenn skalierbar sein soll
    imshow("image", binary);
    waitKey();
#endif

    // close small holes: radius ( hole ) < radius ( close )
    if (cmWidget->useClose->isChecked())
    {
        int radius_close = cmWidget->closeRadius->value();
        // siehe : http://opencv.willowgarage.com/documentation/c/image_filtering.html#createstructuringelementex
//        kernel = cvCreateStructuringElementEx(2*radius_close+1, 2*radius_close+1, radius_close, radius_close, CV_SHAPE_ELLIPSE);
//        cvMorphologyEx(binary, binary, NULL, kernel, CV_MOP_CLOSE);
        cv::morphologyEx(binary,binary,MORPH_OPEN,
                         getStructuringElement( MORPH_ELLIPSE,
                                                Size(2*radius_close+1,2*radius_close+1),
                                                Point(radius_close,radius_close)));
    }
    // remove small blobs: radius ( blob ) < radius ( open )
    if (cmWidget->useOpen->isChecked())
    {
        int radius_open  = cmWidget->openRadius->value();
//        kernel = cvCreateStructuringElementEx(2*radius_open+1, 2*radius_open+1, radius_open, radius_open, CV_SHAPE_ELLIPSE);
//        cvMorphologyEx(binary, binary, NULL, kernel, CV_MOP_OPEN);
        cv::morphologyEx(binary,binary,MORPH_CLOSE,
                         getStructuringElement( MORPH_ELLIPSE,
                                                Size(2*radius_open+1,2*radius_open+1),
                                                Point(radius_open)));
    }
#ifdef SHOW_TMP_IMG
    imshow("image", binary);
    waitKey();
#endif

//    CvSeq *contour;
//    CvSeq *firstContour;
    vector<vector<Point> > contours;
    double area;
//    CvMemStorage *storage = cvCreateMemStorage(0);
    QColor col;
//    CvMoments moments;
//    double m00, m01, m10;
//    int count;
    RotatedRect box;
    double ratio;
//    CvPoint* pointArray;
    bool atEdge;

#ifdef TIME_MEASUREMENT
    //        "==========: "
    debout << "vor cont  : " << getElapsedTime() << endl;
#endif

//    IplImage *clone = cvCloneImage(binary); // clone, da binary sonst veraendert wird
    Mat clone = binary.clone();//(cvarrToMat(binary),true);

    cv::findContours(clone,contours,cv::RETR_EXTERNAL,cv::CHAIN_APPROX_SIMPLE);
//    cvFindContours(clone, storage, &contour, sizeof(CvContour),
//        CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE); // clone wird auch veraendert!!!

#ifdef SHOW_TMP_IMG
    drawContours(clone,contours,0,CV_RGB(255,255,255));
    imshow("image", clone);
    waitKey();
#endif

//    CV_RETR_EXTERNAL gives "outer" contours, so if you have (say) one contour enclosing another (like concentric circles), only the outermost is given.
//    CV_RETR_LIST gives all the contours and doesn't even bother calculating the hierarchy -- good if you only want the contours and don't care whether one is nested inside another.
//    CV_RETR_CCOMP gives contours and organises them into outer and inner contours. Every contour is either the outline of an object, or the outline of an object inside another object (i.e. hole). The hierarchy is adjusted accordingly. This can be useful if (say) you want to find all holes.
//    CV_RETR_TREE calculates the full hierarchy of the contours. So you can say that object1 is nested 4 levels deep within object2 and object3 is also nested 4 levels deep.

//    firstContour = contour;
//    cvReleaseImage(&clone);
    // test each contour
    while (!contours.empty())
    {
        vector<Point> contour = contours.back();
        //count = contour->total; // This is number of point in one contour
        area = cv::contourArea(contour);
//        area = cvContourArea(contour, CV_WHOLE_SEQ);

//#if ((CV_MAJOR_VERSION < 2) || ((CV_MAJOR_VERSION == 2) && (CV_MINOR_VERSION < 1)))
//                    //if  ((CV_MAJOR_VERSION < 2) || ((CV_MAJOR_VERSION == 2) && (CV_MINOR_VERSION < 1)))
//                        contourArea = cvContourArea(contours,CV_WHOLE_SEQ);
//#else
//                    //else
//                        contourArea = cvContourArea(contours,CV_WHOLE_SEQ, true);
//#endif

        box = cv::minAreaRect(contour);//cvMinAreaRect2(contour);

        if (box.size.height > box.size.width)
        {
            ratio = box.size.height/box.size.width;
        }
        else
        {
            ratio = box.size.width/box.size.height;
        }

//        debout << box.center.x << " "<< box.center.y <<endl;
//        debout << box.size.height << " "<< box.size.width <<endl;
//        debout << img->width<< " " << img->height <<endl;
//        if ((box.center.x+maxExpansion/2 > img->width-2)  || (box.center.x-maxExpansion/2 < 2) ||
//            (box.center.y+maxExpansion/2 > img->height-2) || (box.center.y-maxExpansion/2 < 2))
//            debout << box.center.x << " "<< box.center.y <<endl;

        // contour at border of roi, than neglect because of object going out of region has moving center
//        pointArray = (CvPoint*)malloc(contour->total*sizeof(CvPoint));
//        cvCvtSeqToArray(contour, pointArray, CV_WHOLE_SEQ);// Get contour point set.
        atEdge = false;
        for(size_t i=0; i<contour.size(); i++)
            if ((contour.at(i).x <= 1) || (contour.at(i).x >= img.cols-2) ||
                (contour.at(i).y <= 1) || (contour.at(i).y >= img.rows-2))
                atEdge = true;
//        free(pointArray);

//        cvMoments(contour, &moments);
//        //moments.m00;
//        m00 = cvGetCentralMoment(&moments, 0, 0); // == area
//        m01 = cvGetCentralMoment(&moments, 0, 1); // irgendwie immer 0
//        m10 = cvGetCentralMoment(&moments, 1, 0); // irgendwie immer 0
//        crossList->append(TrackPoint(Vec2F(m10/m00, m01/m00), 100, col)); // 100 beste qualitaet

        if (!atEdge && area >= cmWidget->minArea->value() && area <= cmWidget->maxArea->value() && ratio <= cmWidget->maxRatio->value())
        {
            // eine mittelung waere ggf sinnvoll, aber am rand aufpassen
            col.setRgb(getValue(img,myRound(box.center.x),myRound(box.center.y)).rgb());
//                       getR(img, myRound(box.center.x), myRound(box.center.y)),
//                       getG(img, myRound(box.center.x), myRound(box.center.y)),
//                       getB(img, myRound(box.center.x), myRound(box.center.y)));
            crossList->append(TrackPoint(Vec2F(box.center.x, box.center.y), 100, Vec2F(box.center.x, box.center.y), col)); // 100 beste qualitaet
        }

        // take the next contour
        contours.pop_back();
//        contour = contour->h_next;
    }
//    if (firstContour)
//        cvClearSeq(firstContour);
//    cvReleaseMemStorage(&storage);
#ifdef TIME_MEASUREMENT
    //        "==========: "
    debout << "nach cont : " << getElapsedTime() <<endl;
#endif


//    if (contours.size() == 0)
//        return;

//    // go through all the connected components
//    assert(hierarchy[0][3] < 0);   // first entry on top level (no parent)

//    while (idx >= 0)
//    {

//        //mmnt = moments(contours[idx], false);

//        crossList->append(TrackPoint(Vec2F(contours[idx][0].x, contours[idx][0].y), 100, col)); // 100 beste qualitaet
//        //crossList->append(TrackPoint(Vec2F(mmnt.m10/mmnt.m00 , mmnt.m01/mmnt.m00), 100, col)); // 100 beste qualitaet
//        // save more information about the blob and filter later or
//        // filter the blobs by size, shape, ...
//        // ...

//        idx = hierarchy[idx][0];   // jump to next contour
//    }

    // results
    //cout << "Found " << crossList->size() << " features.\n";
}

/**
 * @brief uses OpenCV libraries to detect Aruco CodeMarkers
 * @param img
 * @param crossList[out] list of detected TrackPoints
 * @param controlWidget
 */
void detail::findCodeMarker(Mat &img, QList<TrackPoint> *crossList, Control *controlWidget, Vec2F offsetCropRect2Roi /*=(0,0)*/)
{
//#if 0 // Maik temporaer, damit es auf dem Mac laeuft

    CodeMarkerItem* codeMarkerItem = controlWidget->getMainWindow()->getCodeMarkerItem();
    CodeMarkerWidget* codeMarkerWidget = controlWidget->getMainWindow()->getCodeMarkerWidget();

    cv::Ptr<cv::aruco::Dictionary> dictionary = aruco::getPredefinedDictionary(aruco::PREDEFINED_DICTIONARY_NAME(codeMarkerWidget->dictList->currentIndex()));

    if (codeMarkerWidget->dictList->currentIndex() == 17) //for usage of DICT_mip_36h12 as it is not predifined in opencv
    {
        dictionary = detail::getDictMip36h12();
    }

    Ptr<aruco::DetectorParameters> detectorParams = aruco::DetectorParameters::create();

    double minMarkerPerimeterRate = 0.03, maxMarkerPerimeterRate = 4, minCornerDistanceRate = 0.05, minMarkerDistanceRate = 0.05;

    Petrack *mainWindow = controlWidget->getMainWindow();

    int bS = mainWindow->getImageBorderSize();
    int recoMethod = controlWidget->getRecoMethod();

    if (controlWidget->getCalibCoordDimension() == 0) // 3D
    {
        if (recoMethod == 6)  // for usage of codemarker with CodeMarker-function (-> without MulticolorMarker)
        {
            QRect rect(myRound(mainWindow->getRecoRoiItem()->rect().x()),//+controlWidget->getMainWindow()->getImageBorderSize()),
                       myRound(mainWindow->getRecoRoiItem()->rect().y()),//+controlWidget->getMainWindow()->getImageBorderSize()),
                       myRound(mainWindow->getRecoRoiItem()->rect().width()),
                       myRound(mainWindow->getRecoRoiItem()->rect().height()));
            QPointF p1 = mainWindow->getImageItem()->getCmPerPixel(rect.x(),rect.y(),controlWidget->mapDefaultHeight->value()),
                    p2 = mainWindow->getImageItem()->getCmPerPixel(rect.x()+rect.width(),rect.y(),controlWidget->mapDefaultHeight->value()),
                    p3 = mainWindow->getImageItem()->getCmPerPixel(rect.x(),rect.y()+rect.height(),controlWidget->mapDefaultHeight->value()),
                    p4 = mainWindow->getImageItem()->getCmPerPixel(rect.x()+rect.width(),rect.y()+rect.height(),controlWidget->mapDefaultHeight->value());

            double cmPerPixel_min = min(min(min(p1.x(), p1.y()), min(p2.x(), p2.y())),
                                        min(min(p3.x(), p3.y()), min(p4.x(), p4.y())));
            double cmPerPixel_max = max(max(max(p1.x(), p1.y()), max(p2.x(), p2.y())),
                                        max(max(p3.x(), p3.y()), max(p4.x(), p4.y())));

            minMarkerPerimeterRate = (codeMarkerWidget->minMarkerPerimeter->value()*4./cmPerPixel_max)/max(rect.width(),rect.height());
            maxMarkerPerimeterRate = (codeMarkerWidget->maxMarkerPerimeter->value()*4./cmPerPixel_min)/max(rect.width(),rect.height());
        }

        if (recoMethod == 5)   // for usage of codemarker with MulticolorMarker
        {
            QRect rect(0,0, img.rows, img.cols);

            // from Aruco Documentation values 0 + 4 all info is considered (low performance issues expected due to small image size)
            minMarkerPerimeterRate = 0.02;
            maxMarkerPerimeterRate = 4.;
        }

        minCornerDistanceRate = codeMarkerWidget->minCornerDistance->value();
        minMarkerDistanceRate = codeMarkerWidget->minMarkerDistance->value();

    }
    else // 2D
    {
        double cmPerPixel = mainWindow->getImageItem()->getCmPerPixel();
        minMarkerPerimeterRate = (codeMarkerWidget->minMarkerPerimeter->value()*4/cmPerPixel)/max(mainWindow->getImage()->width()-bS,mainWindow->getImage()->height()-bS);
        maxMarkerPerimeterRate = (codeMarkerWidget->maxMarkerPerimeter->value()*4/cmPerPixel)/max(mainWindow->getImage()->width()-bS,mainWindow->getImage()->height()-bS);

        minCornerDistanceRate = codeMarkerWidget->minCornerDistance->value();
        minMarkerDistanceRate = codeMarkerWidget->minMarkerDistance->value();
    }

    detectorParams->adaptiveThreshWinSizeMin              = codeMarkerWidget->adaptiveThreshWinSizeMin->value();
    detectorParams->adaptiveThreshWinSizeMax              = codeMarkerWidget->adaptiveThreshWinSizeMax->value();
    detectorParams->adaptiveThreshWinSizeStep             = codeMarkerWidget->adaptiveThreshWinSizeStep->value();
    detectorParams->adaptiveThreshConstant                = codeMarkerWidget->adaptiveThreshConstant->value();
    detectorParams->minMarkerPerimeterRate                = minMarkerPerimeterRate;
    detectorParams->maxMarkerPerimeterRate                = maxMarkerPerimeterRate;
    detectorParams->polygonalApproxAccuracyRate           = codeMarkerWidget->polygonalApproxAccuracyRate->value();
    detectorParams->minCornerDistanceRate                 = minCornerDistanceRate;
    detectorParams->minDistanceToBorder                   = codeMarkerWidget->minDistanceToBorder->value();
    detectorParams->minMarkerDistanceRate                 = minMarkerDistanceRate;
    // No refinement is default value
    // TODO Check if this is the best MEthod for our usecase
    if(codeMarkerWidget->doCornerRefinement->isChecked()){
        detectorParams->cornerRefinementMethod            = cv::aruco::CornerRefineMethod::CORNER_REFINE_SUBPIX;
    }
    //detectorParams->cornerRefinementMethod                = codeMarkerWidget->doCornerRefinement->isChecked();
    detectorParams->cornerRefinementWinSize               = codeMarkerWidget->cornerRefinementWinSize->value();
    detectorParams->cornerRefinementMaxIterations         = codeMarkerWidget->cornerRefinementMaxIterations->value();
    detectorParams->cornerRefinementMinAccuracy           = codeMarkerWidget->cornerRefinementMinAccuracy->value();
    detectorParams->markerBorderBits                      = codeMarkerWidget->markerBorderBits->value();
    detectorParams->perspectiveRemovePixelPerCell         = codeMarkerWidget->perspectiveRemovePixelPerCell->value();
    detectorParams->perspectiveRemoveIgnoredMarginPerCell = codeMarkerWidget->perspectiveRemoveIgnoredMarginPerCell->value();
    detectorParams->maxErroneousBitsInBorderRate          = codeMarkerWidget->maxErroneousBitsInBorderRate->value();
    detectorParams->minOtsuStdDev                         = codeMarkerWidget->minOtsuStdDev->value();
    detectorParams->errorCorrectionRate                   = codeMarkerWidget->errorCorrectionRate->value();

    vector<int> ids;
    vector<vector<Point2f> > corners, rejected;
    ids.clear();
    corners.clear();
    rejected.clear();

#ifdef TIME_MEASUREMENT
    //        "==========: "
    debout << "start detectCodeMarkers : " << getElapsedTime() <<endl;
#endif

    aruco::detectMarkers(img/*copy.clone()*/, dictionary, corners, ids, detectorParams, rejected);

#ifdef TIME_MEASUREMENT
    //        "==========: "
    debout << "end detectCodeMarkers  : " << getElapsedTime() <<endl;
#endif

    codeMarkerItem->addDetectedMarkers(corners,ids, offsetCropRect2Roi);
    codeMarkerItem->addRejectedMarkers(rejected, offsetCropRect2Roi);

    // detected code markers
    for(size_t i = 0; i<ids.size(); i++)
    {
//        debout << "Detected MarkerID: " << ids.at(i) << " [( " << corners.at(i).at(0).x << "," << corners.at(i).at(0).y << "),( "
//                                                               << corners.at(i).at(1).x << "," << corners.at(i).at(1).y << "),( "
//                                                               << corners.at(i).at(2).x << "," << corners.at(i).at(2).y << "),( "
//                                                               << corners.at(i).at(3).x << "," << corners.at(i).at(3).y << ")]" <<  endl;

//        if (codeMarkerWidget->showMask->isChecked())
//        {
//            detected_points[0] = CvPoint(corners.at(i).at(0).x, corners.at(i).at(0).y);
//            detected_points[1] = CvPoint(corners.at(i).at(1).x, corners.at(i).at(1).y);
//            detected_points[2] = CvPoint(corners.at(i).at(2).x, corners.at(i).at(2).y);
//            detected_points[3] = CvPoint(corners.at(i).at(3).x, corners.at(i).at(3).y);

//            cvFillConvexPoly(binary,detected_points,4,RGB(255,255,255));
//        }

        double x = (corners.at(i).at(0).x+corners.at(i).at(1).x+corners.at(i).at(2).x+corners.at(i).at(3).x)*0.25;
        double y = (corners.at(i).at(0).y+corners.at(i).at(1).y+corners.at(i).at(2).y+corners.at(i).at(3).y)*0.25;

        crossList->append(TrackPoint(Vec2F(x,y), 100, ids.at(i))); // 100 beste qualitaet
    }

#ifdef TIME_MEASUREMENT
    //        "==========: "
    debout << "nach cont : " << getElapsedTime() <<endl;
#endif

//#endif
}

void findContourMarker(Mat &img, QList<TrackPoint> *crossList, int markerBrightness, bool ignoreWithoutMarker, bool autoWB, int recoMethod, float headSize)
{
    int threshold, plus, count;
    double angle;
    //MarkerCasernList markerList;
    //MarkerHermesList markerList;
    MarkerHermesList markerHermesList;
    MarkerCasernList markerCasernList;
    MarkerJapanList markerJapanList(headSize);
    //MarkerColorList markerList;
//    void * markerList;
    // 0 == Kaserne, 1 == Hermes, 2 == Ohne
//    if (recoMethod == 0)
//        markerList = (MarkerHermesList *) new MarkerHermesList;
//    else if (recoMethod == 1)
//        markerList = (MarkerHermesList *) new MarkerHermesList;
//    else
//        debout << "Error: this should never happen!" <<endl;
    Size sz = Size(img.cols & -2, img.rows & -2);
    Mat gray = Mat(sz,CV_8UC1);
    Mat tgray;
    Mat grayFix;
//    IplImage *gray = cvCreateImage(sz, 8, 1);
//    IplImage *tgray;
//    IplImage *grayFix;
//    CvPoint* PointArray;
    vector<vector<Point> > contours;
//    CvSeq *contours;
//    CvSeq *firstContour;


//    Mat PointArray2D32f;
    RotatedRect box;
    //CvBox2D box;
//     CvPoint center;
    int expansion;
    double contourArea;
    //debout << "MemStorage" << endl;
//    CvMemStorage *storage = cvCreateMemStorage(0);
    bool added = false;

#ifdef WITH_RECT
            double area, length;
            CvSeq* result;
            Vec2F v[4];
#endif
    
    //     // down-scale and upscale the image to filter out the noise
    //     cvPyrDown(timg, pyr, 7); // 7 =? CV_GAUSSIAN_5x5
    //     cvPyrUp(pyr, timg, 7);
    if (img.channels() == 3)
    {
        tgray = Mat(sz,CV_8UC1);//cvCreateImage(sz, 8, 1);
        // folgende Zeilen erledigen das gleiche wie cvCvtColor(img, tgray, CV_RGB2GRAY);
        // noetig gewesen, da opencv Probleme machte
        //Y=0.299*R + 0.587*G + 0.114*B;
        //        int x,y;
        //        char *data = img->imageData;
        //        char *greyData = tgray->imageData;
        //        char *yData = data;
        //        char *yGreyData = greyData;
        //        for (y = 0; y < img->height; y++)
        //        {
        //            for (x = 0; x < img->width; x++)
        //                *(greyData) = 0.299*data[0]+0.587*data[1]+0.114*data[2];
        //            data = (yData += img->widthStep); // because sometimes widthStep != width
        //            greyData = (yGreyData += tgray->widthStep); // because sometimes widthStep != width
        //
        //        }
        cvtColor(img,tgray,COLOR_RGB2GRAY);
//        cvCvtColor(img, tgray, CV_RGB2GRAY);
    }
    //debout << "Threshold" << endl;

#ifdef SHOW_TMP_IMG
namedWindow("img", CV_WINDOW_AUTOSIZE); // 0 wenn skalierbar sein soll
//IplImage *tmpAusgabe = cvCreateImage(cvGetSize(img), 8, 3);
//cvCvtColor(img ,tmpAusgabe, CV_GRAY2RGB);
    Mat tmpAusgabe = img.clone(); // nur grauweert, wenn grauwertbild!!!
    Mat tmpAusgabe2 = img.clone(); // nur grauweert, wenn grauwertbild!!!

    imShow("img", img);
    waitKey();
#endif

    // try several threshold levels
    plus = (250-72)/10;
    // andere richtung der schwellwertanpassung koennte andere ergebnisse leifern
    // cw->markerBrightness->value()==markerBrightness hat default 50
    for (threshold = 60+markerBrightness; threshold < 251 ; threshold += plus) //70..255, 20 //155+cw->temp2->value()
    {
#ifdef TIME_MEASUREMENT
    //        "==========: "
    debout << "vor hellig: " << getElapsedTime() <<endl;
#endif

        //// hack: use Canny instead of zero threshold level.
        //// Canny helps to catch squares with gradient shading   
        //if(l == 0)
        //{
        //    // apply Canny. Take the upper threshold from slider
        //    // and set the lower to 0 (which forces edges merging) 
        //    cvCanny(tgray, gray, 0, thresh, 5);
        //    // dilate canny output to remove potential
        //    // holes between edge segments 
        //    cvDilate(gray, gray, 0, 1);
        //}
        // cvCanny(tgray, gray, 4000, 2500, 5); guter kantenoperator aber keine geschlossene contour
        if (img.channels() == 3)
        {
            // folgende Zeilen erledigen das gleiche wie cvThreshold(tgray, gray, threshold, 255, CV_THRESH_BINARY);
            // noetig gewesen, da opencv Probleme machte
            //            int x, y;
            //            char *data = gray->imageData;
            //            char *greyData = tgray->imageData;
            //            char *yData = data;
            //            char *yGreyData = greyData;
            //            for (y = 0; y < gray->height; y++)
            //            {
            //                for (x = 0; x < gray->width; x++)
            //                {
            //                    if (*(greyData) > threshold)
            //                        *(data) = 255;
            //                    else
            //                        *(data) = 0;
            //                }
            //                data = (yData += gray->widthStep); // because sometimes widthStep != width
            //                greyData = (yGreyData += tgray->widthStep); // because sometimes widthStep != width
            //            }
            cv::threshold(tgray,gray,threshold, 255, cv::THRESH_BINARY);
//            cvThreshold(tgray, gray, threshold, 255, CV_THRESH_BINARY);
        }
        else if (img.channels() == 1)
            cv::threshold(img,gray,threshold,255, cv::THRESH_BINARY);//cvThreshold(img, gray, threshold, 255, CV_THRESH_BINARY);
        else
            debout << "Error: Wrong number of channels: " << img.channels() <<endl;
        grayFix = gray.clone();// = cvCloneImage(gray); // make a copy


        //cvShowImage("img", grayFix);
        //cvWaitKey();

#ifdef TIME_MEASUREMENT
    //        "==========: "
    debout << "vor    iso: " << getElapsedTime() <<endl;
#endif
        // find contours and store them all as a list
//#if CV_MAJOR_VERSION == 2
        findContours(gray,contours,cv::RETR_LIST,cv::CHAIN_APPROX_SIMPLE);
//        cvFindContours(gray, storage, &firstContour, sizeof(CvContour),
//            CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE); // gray wird auch veraendert!!!
//        contours = firstContour;

//        int count_contours = 0;

//        while(contours){

//            debout << "contour[" << ++count_contours << "]: " << contours->total << endl;
//            contours = contours->h_next;
//        }
//        debout << "firstContour" << endl;
//        contours = firstContour;
//#elif CV_MAJOR_VERSION == 3
    //Mat threshold_output;
    //vector<Vec4i> hierarchy;
    //threshold(cvarrToMat(tgray), threshold_output, threshold, 255, THRESH_BINARY);
    //vector<vector<Point> > v_contours;

    //findContours(cvarrToMat(gray), contours,/* hierarchy,*/ CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0,0) );
//#endif
//IplImage *tmpAusgabe = cvCloneImage(img); // make a copy
// char outstr[256];

#ifdef TIME_MEASUREMENT
    //        "==========: "
    debout << "nach   iso: " << getElapsedTime() <<endl;
#endif

        // test each contour
        while (!contours.empty())
        {
            vector<Point> contour = contours.back();
            //koennten auch interessant sein:
            //MinAreaRect2
            //MinEnclosingCircle
            // um kreise zu suchen koennte auch cvHoughCircles genutzt werden

            count = contour.size();
//            count = contours->total; // This is number point in contour

            // man koennte das Seitenverhaeltnis, contour-gesamtlaenge vorher ueberpruefen, um cont rauszuwerfen
            //for (int iii=0; iii<contours->total; ++iii)
            //{
            //    CvPoint* ppp = (CvPoint*) cvGetSeqElem(contours, iii);
            //    printf("(%d,%d)\n", ppp->x, ppp->y);
            //}
            //printf("\n");
            //debout << "count: " << count << endl;
            if (count > 5)
            {
#ifdef SHOW_TMP_IMG
// in rot contouren in eine drehrichtung, in gruen andere
cvDrawContours(tmpAusgabe,contours,CV_RGB(255,0,0),CV_RGB(0,255,0),0,1,8,cvPoint(0,0));
cvShowImage("img", tmpAusgabe);
cvWaitKey();

////// in weiss alle konturen - behindern jedoch weitere untersuchungen
////cvDrawContours(img,contours,CV_RGB(255,255,255),CV_RGB(255,255,255),0,1,8,cvPoint(0,0));
#endif


//                PointArray2D32f.create(count,2, CV_32F);
//                //Mat(cvarrToMat(contours)).convertTo(PointArray2D32f.at(i), CV_32F);
//                // Convert CvPoint set to Mat set.
//                for(i=0; i<count; i++)
//                {
//                    //debout << "in for" << endl;
//                    //debout << "(" << i << ") " << PointArray[i].x << ", " << PointArray[i].y << endl;
//                    PointArray2D32f.at<float>(i,0) = (float)PointArray[i].x;
//                    PointArray2D32f.at<float>(i,1) = (float)PointArray[i].y;
//                    //debout << "[" << i << "] " << PointArray2D32f.at<double>(i,0) << ", " << PointArray2D32f.at<double>(i,1) << endl;
//                }
                //debout << "FitEllipse" << endl;
                // Fits ellipse to current contour.
                //debout << "count: " << count << endl;
                Mat pointsf;
                Mat(contour).convertTo(pointsf, CV_32F);
                box = fitEllipse(pointsf);

//                box = fitEllipse(PointArray2D32f);//, count, &box);

                // neuer:
                //// Fits ellipse to current contour.
                //CvBox2D box = cvFitEllipse2(PointArray2D32f);
                //debout << "expansion" << endl;
                //ellipses are not allowed near border !!!
                expansion = box.size.width > box.size.height ? myRound(box.size.width*0.5) : myRound(box.size.height*0.5);
                //debout << "if" << endl;
                if (box.center.x-expansion > ELLIPSE_DISTANCE_TO_BORDER && box.center.x+expansion < gray.cols-ELLIPSE_DISTANCE_TO_BORDER &&
                    box.center.y-expansion > ELLIPSE_DISTANCE_TO_BORDER && box.center.y+expansion < gray.rows-ELLIPSE_DISTANCE_TO_BORDER)
                {
                  //  debout << "in if" << endl;
//                     // Convert ellipse data from float to integer representation.
//                     center.x = myRound(box.center.x);
//                     center.y = myRound(box.center.y);
                    //box.angle = -box.angle; // war wohl in aelteren opencv versionen noetig

                    angle = (box.angle)/180.*PI;
                    if (box.size.width<box.size.height) {
                      angle -= PI / 2;
                    }
                    //debout << "contourArea" << endl;
                    //cvContourArea(contours,CV_WHOLE_SEQ) koennte mit MyEllipse.are() verglichen werden und bei grossen abweichungen verworfenwerden!!!
                    // IN OPENCV2.1 liefert cvContourArea KEIN VORZEICHEN ZUM ERKENNEN DER DREHRICHTUNG!!!! es ist ein optionaler paramter hinzugefuegt worden!!!!

                    //else
                    contourArea = cv::contourArea(contour,true);
//                        contourArea = cvContourArea(contours,CV_WHOLE_SEQ, true);

                    //contourArea koennte mit MyEllipse.area() verglichen werden und bei grossen abweichungen verworfenwerden!!!
                    //debout << contourArea << " " << box.center.x << " " << box.center.y << " " << box.size.width <<" " << box.size.height <<endl;
                    //debout << e.area() << " " << fabs(contourArea)<<  endl;
                    //debout << "MyEllipse" << endl;
                    MyEllipse e(box.center.x, box.center.y, box.size.width*0.5, box.size.height*0.5, angle);
                    if (recoMethod == 0) // Casern
                        added = markerCasernList.mayAddEllipse(grayFix, e, (contourArea > 0));
                    else if (recoMethod == 1) // Hermes
                        added = markerHermesList.mayAddEllipse(grayFix, e, (contourArea > 0));
                    else if (recoMethod == 4) // Japan
                        added = markerJapanList.mayAddEllipse(grayFix, e, (contourArea > 0));

                    if (added)
{
//  debout << cvContourArea(contours,CV_WHOLE_SEQ) << endl; // PointArray gerichteter Flaecheninhalt von const CvArr*

#ifdef SHOW_TMP_IMG
Size sizeTmp;
Point ctr;
sizeTmp.width = myRound(box.size.width*0.5);
sizeTmp.height = myRound(box.size.height*0.5);
ctr.x = myRound(box.center.x);
ctr.y = myRound(box.center.y);

//  MyEllipse e(box.center.x, box.center.y, box.size.width*0.5, box.size.height*0.5, angle);
// int cx = myRound(e.center().x());
// int cy = myRound(e.center().y());

// //  debout << "****" << getR(img, 0, 0) << " " << getG(img, 0, 0) << " " << getB(img, 0, 0) << endl;
// //  debout << cvGetDims(img)<<endl;
// //  debout << ch<<endl;
// //  debout << img->width<<endl;
// //  debout << img->widthStep<<endl;
// //  debout << cvGet2D(img, cx, cy).val[2] << " " << cvGet2D(img, cx, cy).val[1] << " " << cvGet2D(img, cx, cy).val[0] << endl;
// // QRgb col = qRgb(((int)(*(data+ch*(cx+cy*iw)+2)))+127, ((int)(*(data+ch*(cx+cy*iw)+1)))+127, ((int)(*(data+ch*(cx+cy*iw))))+127);
//  QRgb col = qRgb(getR(img, cx, cy), getG(img, cx, cy), getB(img, cx, cy));
//  QString s("#%1%2%3");
//  s = s.arg(qRed(col), 2, 16, QChar('0')).arg(qGreen(col), 2, 16, QChar('0')).arg(qBlue(col), 2, 16, QChar('0'));
//  debout << cx+1 << " " << cy+1 << ": ";
//  cout << s <<endl;
//  QColor col2 = QColor(getR(img, cx, cy), getG(img, cx, cy), getB(img, cx, cy));
//  // hue: 0..359, 
//  debout << "Farbe: " << col2.hue() << ", Saettigung: " << col2.saturation() << ", Helligkeit: " << col2.value() << endl; 
// //  debout << cx << " " << cy+1 << ": ";
// //  cout << (int)*(data+ch*(cx+cy*iw-1)+2)+127 << " " << (int)*(data+ch*(cx+cy*iw-1)+1)+127 << " " << (int)*(data+ch*(cx+cy*iw-1))+127 << endl;
// //  debout << cx+1 << " " << cy+1 << ": ";
// //  cout << (int)*(data+ch*(cx+cy*iw)+2)+127 << " " << (int)*(data+ch*(cx+cy*iw)+1)+127 << " " << (int)*(data+ch*(cx+cy*iw))+127 << endl;
// //  debout << cx+2 << " " << cy+1 << ": ";
// //  cout << (int)*(data+ch*(cx+cy*iw+1)+2)+127 << " " << (int)*(data+ch*(cx+cy*iw+1)+1)+127 << " " << (int)*(data+ch*(cx+cy*iw+1))+127 << endl;


if (contourArea > 0)
    ellipse(tmpAusgabe2, ctr, sizeTmp, box.angle, 0, 360, CV_RGB(0, 255, 0), 1, CV_AA, 0);
else
    ellipse(tmpAusgabe2, ctr, sizeTmp, box.angle, 0, 360, CV_RGB(255, 0, 0), 1, CV_AA, 0);
namedWindow("img", CV_WINDOW_AUTOSIZE); // 0 wenn skalierbar sein soll
imShow("img", tmpAusgabe);
namedWindow("img2", CV_WINDOW_AUTOSIZE); // 0 wenn skalierbar sein soll
imShow("img2", tmpAusgabe2);
//static QLabel imgLabel;
//showImg(& imgLabel, tmpAusgabe2);
//cvWaitKey(0); // zahl statt null, wenn nach bestimmter zeit weitergegangen werden soll
#endif
}

                }
                //debout << "FreePointArray" << endl;
//                free(PointArray);

//                PointArray2D32f.release();
            }
            //debout << "Free PointArray" << endl;

#ifdef WITH_RECT
            // approximate contour with accuracy proportional
            // to the contour perimeter
            approxPolyDP(Mat(contour, contour, 0.08*cv::arcLength(contour,true), false);
//            result = cvApproxPoly(contours.at(k), sizeof(CvContour), storage,
//                CV_POLY_APPROX_DP, 0.08*cvContourPerimeter(contours), 0); //cvContourPerimeter(contours)*0.02
            // square contours should have 4 vertices after approximation
            // relatively large area (to filter out noisy contours)
            // and be convex.
            // Note: absolute value of an area is used because
            // area may be positive or negative - in accordance with the
            // contour orientation
            area = cv::contourArea(contour);
            length = cv::arcLength(result,false);
            // cvContourPerimeter(contours) ist immer etwas groesser als length - warum?
            if(contour.size() == 4 &&//&&result->total < 11 &&
                area > 1500 && area < 10000 && //cvCheckContourConvexity(result)
                length >150 && length < 500)
            {
                for(i = 0; i < 4; ++i)
                    v[i] = (Vec2F)(CvPoint*) cvGetSeqElem(contours.at(k), i);
                if (recoMethod == 0) // Casern
                    markerCasernList.mayAddQuadrangle(v);
                else if (recoMethod == 1) // Hermes
                    markerHermesList.mayAddQuadrangle(v);
                else if (recoMethod == 4) // Japan
                    markerJapanList.mayAddQuadrangle(v);

            }
#endif
            //debout << "total number of contours: " << contours->total << endl;
            // take the next contour
//            contours = contours->h_next;
            contours.pop_back();
        }
       // debout << "after contours" << endl;

//cvNamedWindow("img", CV_WINDOW_AUTOSIZE ); // 0 wenn skalierbar sein soll
//cvShowImage("img", tmpAusgabe);
//  sprintf(outstr,"c:/%d.png",threshold);
// cvSaveImage(outstr, tmpAusgabe);
//cvWaitKey( 0 ); // zahl statt null, wenn nach bestimmter zeit weitergegangen werden soll

        // nicht noetig, aber so kann der neu erzeugte speicherplatz reduziert werden
        // freigabe des speicherplatz erst bei cvClearMemStorage(storage)
//        if (firstContour)
//            cvClearSeq(firstContour); // not free only available for next push
//        cvReleaseImage(&grayFix);
    }
#ifdef TIME_MEASUREMENT
    //        "==========: "
    debout << "nach ellip: " << getElapsedTime() <<endl;
#endif

    //cvReleaseMemStorage()
    if (recoMethod == 0) // Casern
    {
        markerCasernList.organize(img, autoWB);
        markerCasernList.toCrossList(crossList, ignoreWithoutMarker);
    }
    else if (recoMethod == 1) // Hermes
    {
        markerHermesList.organize(img, autoWB);
        markerHermesList.toCrossList(crossList, ignoreWithoutMarker);
    }
    else if (recoMethod == 4) // Japan
    {
        markerJapanList.organize(img, autoWB);
        markerJapanList.toCrossList(crossList, ignoreWithoutMarker);
    }
#ifdef TIME_MEASUREMENT
    //        "==========: "
    debout << "nach ident: " << getElapsedTime() <<endl;
#endif

#ifdef SHOW_TMP_IMG
//markerList.draw(tmpAusgabe);    //diese zeile zeichnet alle marker, nun aber in trackeritem.cpp
//cvReleaseImage(&tmpAusgabe);
//cvReleaseImage(&tmpAusgabe2);
#endif

    // release all the temporary images
//    cvReleaseImage(&gray);
//    if (img->nChannels == 3)
//        cvReleaseImage(&tgray);

//    cvReleaseMemStorage(&storage); // stand vorher in uebergeordneter Funktion
//    //cvClearMemStorage(storage); // gibt nicht speicher an bs frei, sondern nur fuer neue sequenzen
}


/**
 * @brief Detects position of markers from user-chosen type
 *
 * @param img
 * @param roi Region of interest for recognition
 * @param crossList[out] detected TrackPoints
 * @param controlWidget
 * @param borderSize
 * @param bgFilter
 */
void getMarkerPos(Mat &img, QRect &roi, QList<TrackPoint> *crossList, Control *controlWidget, int borderSize, BackgroundFilter *bgFilter)
{
    int markerBrightness = controlWidget->markerBrightness->value();
    bool ignoreWithoutMarker = (controlWidget->markerIgnoreWithout->checkState() == Qt::Checked);
    bool autoWB = (controlWidget->recoAutoWB->checkState() == Qt::Checked);
    int recoMethod = controlWidget->getRecoMethod();

    Mat tImg;
    Rect rect;
    //debout << "recoMethod: " << recoMethod << endl;
//    tImg = getRoi(img, roi, rect, !((recoMethod == 3)||(recoMethod == 5)||(recoMethod == 6)));
    tImg = getRoi(img, roi, rect, !((recoMethod == 3) || (recoMethod == 5) || (recoMethod == 6)));

    if (tImg.empty())
        return;

    // offset of rect
    Vec2F v(rect.x-borderSize, rect.y-borderSize);
    //debout << "method: " << recoMethod << endl;
    // 0 == Kaserne, 1 == Hermes, 2 == Ohne, 3 == Color, 4 == Japan, 5 == MultiColor, 6 == CodeMarker
    if (recoMethod == 5)
        findMultiColorMarker(tImg, crossList, controlWidget, ignoreWithoutMarker, v);
    else if (recoMethod == 3)
        findColorMarker(tImg, crossList, controlWidget);
    else if (recoMethod == 6)
        findCodeMarker(tImg, crossList, controlWidget);
    else
        findContourMarker(tImg, crossList, markerBrightness, ignoreWithoutMarker, autoWB, recoMethod, controlWidget->getMainWindow()->getHeadSize());
    //debout << "releaseImage" << endl;
//    cvReleaseImage(&tImg); //war: cvFree(&tImg); // war: cvFree(&header); // (void**) only header

    // must be set because else hovermoveevent of recognitionRec moves also the colorMaskItem
    controlWidget->getMainWindow()->getColorMarkerItem()->setRect(v);
    // must be set because else hovermoveevent of recognitionRec moves also the colorMaskItem
    controlWidget->getMainWindow()->getMultiColorMarkerItem()->setRect(v);
    // must be set because else hovermoveevent of recognitionRec moves also the colorMaskItem
    controlWidget->getMainWindow()->getCodeMarkerItem()->setRect(v);
    // set cross position relative to original image size
    for (int i=0; i < crossList->size(); ++i)
    {
         (*crossList)[i] += v;
         (*crossList)[i].setColPoint((*crossList)[i].colPoint() + v);
    }

    if (bgFilter->getEnabled()) // nur fuer den fall von bgSubtraction durchfuehren
    {
        for (int i=0; i < crossList->size(); ++i)
        {
            if (! bgFilter->isForeground(crossList->at(i).x(), crossList->at(i).y()))
            {
                crossList->removeAt(i);
                --i;
            }
        }
    }


    //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // rauschen durch median-filter verringern
    // subtract background - bringt bei den aufnahmen wenig
    //farbwerte ausblenden (schwarz/rot?), da weisse pappe gesucht und sonst in grauwert knallrot ueberbewerttet
    // segmentation
    // floodfill -> minarea rect um pappe zu finden
    // laplace -> fitellipse (auch fuer ohne marker)
    // tracking: lkdemo, motemp, 
    // matchcontours matchtemplate matchshapes cvcamshift cvkalman (unter estimators)
    // squere.exe
    // motion templates
    //optical flow
    //back projection sagt voraus, wie das naechste bild aussehen wird
    // richtige contour in graubild waere schoener als bi-bild, da bessere angleichung

    // color mapping - tracking nutzen und in allen bildern nach farbe suchen - addieren und durchschnitt?

    // wenn keine marker gefunden wurden, sollte ellipsenmitte genommen werden aber als schlechter bewertet werden

    //aequidensiten 2. ordnung (s.409) interessant
    //cvAdaptiveThreshold nicht so interessant, da gerad enur extrme gesucht und nicht lokale kleinere aenderungen
    //versch algorithmen zur auswahl stellen, die kreuz finden

    // die untere schwelle fuer threshold um 72 einstellbat machen !!!!!!!!!!!!!!!!!!!!

    // farben herausfiltern: wenn kanaele sich stark unterscheiden, dann pixel schwarz, wenn umgebung dunkel und
    // weiss wenn umgebung hell

            // suchen vom KREUZ:
            // MatchTemplate
            // MatchShapes

    //     int i, bin_w;
    //     float max_value = 0;
    //     static IplImage *hist_image = 0;
    //     CvHistogram *hist;
    //     int hist_size = 64;
    //     float range_0[]={0,256};
    //     float* ranges[] = { range_0 };

    //     cvReleaseHist(&hist); // vom vorherigen lauf
    //     hist_image = cvCreateImage(cvSize(320,200), 8, 1);
    //     hist = cvCreateHist(1, &hist_size, CV_HIST_ARRAY, ranges, 1);
    //     cvNamedWindow("histogram", 0);
    //     hist = cvCreateHist(1, &hist_size, CV_HIST_ARRAY, ranges, 1);
    //     cvCalcHist(&img, hist, 0, NULL);
    //     cvGetMinMaxHistValue(hist, 0, &max_value, 0, 0);
    //     cvScale(hist->bins, hist->bins, ((double)hist_image->height)/max_value, 0);
    //     /*cvNormalizeHist(hist, 1000);*/

    //     cvSet(hist_image, cvScalarAll(255), 0);
    //     bin_w = myRound((double)hist_image->width/hist_size);

    //     autoStart" value="1"for(i = 0; i < hist_size; i++)
    //         cvRectangle(hist_image, cvPoint(i*bin_w, hist_image->height),
    //                      cvPoint((i+1)*bin_w, hist_image->height - myRound(cvGetReal1D(hist->bins,i))),
    //                      cvScalarAll(0), -1, 8, 0);
   
    //     cvShowImage("histogram", hist_image);
}


/**
 * @brief getMip36h12Dict() overrides current dictionary with dictionary from 'aruco_mip_36h12_dict'
 *
 *
 * bitListDictMip36h12 contains 250 aruco code markers (size 6x6, 1 bit black border)
 * markers consist of 0s and 1s, built from upper left to lower right corner in hexadecimal notation.
 * aruco markers availlable via https://sourceforge.net/projects/aruco/files/
 */
cv::Ptr<cv::aruco::Dictionary> detail::getDictMip36h12()
{
    std::array<uint64_t, 250> bitListDictMip36h12 {0xd2b63a09dUL,0x6001134e5UL,0x1206fbe72UL,0xff8ad6cb4UL,0x85da9bc49UL,0xb461afe9cUL,0x6db51fe13UL,0x5248c541fUL,0x8f34503UL,0x8ea462eceUL,0xeac2be76dUL,0x1af615c44UL,0xb48a49f27UL,0x2e4e1283bUL,0x78b1f2fa8UL,0x27d34f57eUL,0x89222fff1UL,0x4c1669406UL,0xbf49b3511UL,0xdc191cd5dUL,0x11d7c3f85UL,0x16a130e35UL,0xe29f27effUL,0x428d8ae0cUL,0x90d548477UL,0x2319cbc93UL,0xc3b0c3dfcUL,0x424bccc9UL,0x2a081d630UL,0x762743d96UL,0xd0645bf19UL,0xf38d7fd60UL,0xc6cbf9a10UL,0x3c1be7c65UL,0x276f75e63UL,0x4490a3f63UL,0xda60acd52UL,0x3cc68df59UL,0xab46f9daeUL,0x88d533d78UL,0xb6d62ec21UL,0xb3c02b646UL,0x22e56d408UL,0xac5f5770aUL,0xaaa993f66UL,0x4caa07c8dUL,0x5c9b4f7b0UL,0xaa9ef0e05UL,0x705c5750UL,0xac81f545eUL,0x735b91e74UL,0x8cc35cee4UL,0xe44694d04UL,0xb5e121de0UL,0x261017d0fUL,0xf1d439eb5UL,0xa1a33ac96UL,0x174c62c02UL,0x1ee27f716UL,0x8b1c5ece9UL,0x6a05b0c6aUL,0xd0568dfcUL,0x192d25e5fUL,0x1adbeccc8UL,0xcfec87f00UL,0xd0b9dde7aUL,0x88dcef81eUL,0x445681cb9UL,0xdbb2ffc83UL,0xa48d96df1UL,0xb72cc2e7dUL,0xc295b53fUL,0xf49832704UL,0x9968edc29UL,0x9e4e1af85UL,0x8683e2d1bUL,0x810b45c04UL,0x6ac44bfe2UL,0x645346615UL,0x3990bd598UL,0x1c9ed0f6aUL,0xc26729d65UL,0x83993f795UL,0x3ac05ac5dUL,0x357adff3bUL,0xd5c05565UL,0x2f547ef44UL,0x86c115041UL,0x640fd9e5fUL,0xce08bbcf7UL,0x109bb343eUL,0xc21435c92UL,0x35b4dfce4UL,0x459752cf2UL,0xec915b82cUL,0x51881eed0UL,0x2dda7dc97UL,0x2e0142144UL,0x42e890f99UL,0x9a8856527UL,0x8e80d9d80UL,0x891cbcf34UL,0x25dd82410UL,0x239551d34UL,0x8fe8f0c70UL,0x94106a970UL,0x82609b40cUL,0xfc9caf36UL,0x688181d11UL,0x718613c08UL,0xf1ab7629UL,0xa357bfc18UL,0x4c03b7a46UL,0x204dedce6UL,0xad6300d37UL,0x84cc4cd09UL,0x42160e5c4UL,0x87d2adfa8UL,0x7850e7749UL,0x4e750fc7cUL,0xbf2e5dfdaUL,0xd88324da5UL,0x234b52f80UL,0x378204514UL,0xabdf2ad53UL,0x365e78ef9UL,0x49caa6ca2UL,0x3c39ddf3UL,0xc68c5385dUL,0x5bfcbbf67UL,0x623241e21UL,0xabc90d5ccUL,0x388c6fe85UL,0xda0e2d62dUL,0x10855dfe9UL,0x4d46efd6bUL,0x76ea12d61UL,0x9db377d3dUL,0xeed0efa71UL,0xe6ec3ae2fUL,0x441faee83UL,0xba19c8ff5UL,0x313035eabUL,0x6ce8f7625UL,0x880dab58dUL,0x8d3409e0dUL,0x2be92ee21UL,0xd60302c6cUL,0x469ffc724UL,0x87eebeed3UL,0x42587ef7aUL,0x7a8cc4e52UL,0x76a437650UL,0x999e41ef4UL,0x7d0969e42UL,0xc02baf46bUL,0x9259f3e47UL,0x2116a1dc0UL,0x9f2de4d84UL,0xeffac29UL,0x7b371ff8cUL,0x668339da9UL,0xd010aee3fUL,0x1cd00b4c0UL,0x95070fc3bUL,0xf84c9a770UL,0x38f863d76UL,0x3646ff045UL,0xce1b96412UL,0x7a5d45da8UL,0x14e00ef6cUL,0x5e95abfd8UL,0xb2e9cb729UL,0x36c47dd7UL,0xb8ee97c6bUL,0xe9e8f657UL,0xd4ad2ef1aUL,0x8811c7f32UL,0x47bde7c31UL,0x3adadfb64UL,0x6e5b28574UL,0x33e67cd91UL,0x2ab9fdd2dUL,0x8afa67f2bUL,0xe6a28fc5eUL,0x72049cdbdUL,0xae65dac12UL,0x1251a4526UL,0x1089ab841UL,0xe2f096ee0UL,0xb0caee573UL,0xfd6677e86UL,0x444b3f518UL,0xbe8b3a56aUL,0x680a75cfcUL,0xac02baea8UL,0x97d815e1cUL,0x1d4386e08UL,0x1a14f5b0eUL,0xe658a8d81UL,0xa3868efa7UL,0x3668a9673UL,0xe8fc53d85UL,0x2e2b7edd5UL,0x8b2470f13UL,0xf69795f32UL,0x4589ffc8eUL,0x2e2080c9cUL,0x64265f7dUL,0x3d714dd10UL,0x1692c6ef1UL,0x3e67f2f49UL,0x5041dad63UL,0x1a1503415UL,0x64c18c742UL,0xa72eec35UL,0x1f0f9dc60UL,0xa9559bc67UL,0xf32911d0dUL,0x21c0d4ffcUL,0xe01cef5b0UL,0x4e23a3520UL,0xaa4f04e49UL,0xe1c4fcc43UL,0x208e8f6e8UL,0x8486774a5UL,0x9e98c7558UL,0x2c59fb7dcUL,0x9446a4613UL,0x8292dcc2eUL,0x4d61631UL,0xd05527809UL,0xa0163852dUL,0x8f657f639UL,0xcca6c3e37UL,0xcb136bc7aUL,0xfc5a83e53UL,0x9aa44fc30UL,0xbdec1bd3cUL,0xe020b9f7cUL,0x4b8f35fb0UL,0xb8165f637UL,0x33dc88d69UL,0x10a2f7e4dUL,0xc8cb5ff53UL,0xde259ff6bUL,0x46d070dd4UL,0x32d3b9741UL,0x7075f1c04UL,0x4d58dbea0UL};

    int markerSize = 6;
    cv::Ptr<cv::aruco::Dictionary> dictionary = new cv::aruco::Dictionary();
    dictionary->markerSize = markerSize;
    dictionary->maxCorrectionBits = 3;

    // transform from hexadecimal notation to format in dictionary class
    for (auto code : bitListDictMip36h12)
    {
        std::bitset<36> bits(code);
        std::array<uchar, 36> codeAsVector{};
        for (std::size_t i = 0; i < bits.size(); ++i)
        {
            codeAsVector[i] = bits[i];
        }
        cv::Mat markerBits (markerSize, markerSize, CV_8UC1, codeAsVector.data());
        cv::Mat markerCompressed = cv::aruco::Dictionary::getByteListFromBits(markerBits);
        dictionary->bytesList.push_back(markerCompressed);
    }

    return dictionary;
}

} // namespace reco
