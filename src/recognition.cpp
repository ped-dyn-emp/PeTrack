#include <QPointF>
#include <QRect>
#if not CV_MAJOR_VERION == 4
#include <cv.h>
#endif
#include <opencv.hpp>
#include <opencv2/aruco.hpp>

//#include "imgcodecs.hpp"
//#include "videoio.hpp"
//#include "highgui.hpp"

//#include "opencv2/highgui/highgui.hpp"
//#include "opencv2/imgproc/imgproc.hpp"

//folgende zeile spaeter raus
//#include <highgui.h>

//using namespace::cv;

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

using namespace::cv;

//#include "Psapi.h"

//#define WITH_RECT
#define ELLIPSE_DISTANCE_TO_BORDER 10

//#define SHOW_TMP_IMG

//#define TIME_MEASUREMENT

struct ColorParameters
{
    int	h_low;
    int	h_high;
    int	s_low;
    int	s_high;
    int	v_low;
    int	v_high;
    bool inversHue;
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

    Mat hsv = src.clone();

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

// berechnet pixelverschiebung aufgrund von schraegsicht bei einem farbmarker
// Maik Dissertation Seite 138
// boxImageCentre ohne Border
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

//  Run color blob detection on an image to find features.
//  Runs color thresholding to binarize the image and computes connected components.
//  The features are the center of gravity of each connected component.
//  offset is corner of roi
void findMultiColorMarker(Mat &img, QList<TrackPoint> *crossList, Control *controlWidget, bool ignoreWithoutMarker, Vec2F &offset)
{
    ColorParameters	param;
    Petrack *mainWindow = controlWidget->getMainWindow();
    MultiColorMarkerItem* cmItem = mainWindow->getMultiColorMarkerItem();
    MultiColorMarkerWidget* cmWidget = mainWindow->getMultiColorMarkerWidget();
    int bS = mainWindow->getImageBorderSize();

    //QColor fromColor = cmWidget->fromColor->palette().color(QPalette::Button);
    //QColor toColor   = cmWidget->toColor->palette().color(QPalette::Button);
    RectPlotItem *rectPlotItem = controlWidget->getColorPlot()->getMapItem();
    bool useClose = cmWidget->useClose->isChecked();
    bool useOpen = cmWidget->useOpen->isChecked();
    int radius_close = cmWidget->closeRadius->value();
    int radius_open  = cmWidget->openRadius->value();
    double dot_size = cmWidget->dotSize->value();
    Mat	binary;
    // siehe : http://opencv.willowgarage.com/documentation/c/image_filtering.html#createstructuringelementex
//    IplConvKernel* closeKernel = cvCreateStructuringElementEx(2*radius_close+1, 2*radius_close+1, radius_close, radius_close, CV_SHAPE_ELLIPSE);
//    IplConvKernel* openKernel = cvCreateStructuringElementEx(2*radius_open+1, 2*radius_open+1, radius_open, radius_open, CV_SHAPE_ELLIPSE);
    int minArea = cmWidget->minArea->value(), maxArea = cmWidget->maxArea->value();
    double maxRatio = cmWidget->maxRatio->value();
    bool useBlackDot = cmWidget->useDot->isChecked();
    bool restrictPosition = cmWidget->restrictPosition->isChecked();
    ignoreWithoutMarker = cmWidget->ignoreWithoutDot->isChecked(); // ueberschreiben von uebergeordnetem ignoreWithoutMarker
    bool autoCorrect = cmWidget->autoCorrect->isChecked();
    bool autoCorrectOnlyExport = cmWidget->autoCorrectOnlyExport->isChecked();

    vector<vector<Point> > contours;
    vector<vector<Point> > subContours;//, *subFirstContour;
//    CvSeq *contour;
//    CvSeq *firstContour;
    double area;
//    CvMemStorage *storage;
    QColor col, col2, midHue;
//    CvBox2D box; //float: center, size, angle
    RotatedRect box;
    double ratio;
    double maxExpansion;
//    CvPoint* pointArray;
    int i, j, x, y;
    bool atEdge;
    int threshold;

    RotatedRect subBox;

    int cx, cy, add;
    int nr;

    Rect oriRect;
    Rect cropRect;
    double subRatio, subMaxExpansion, subContourArea;
    int border=4; // zusaetzlicher rand um subrects
    Mat subImg;
    Mat subGray;
    Mat subBW;
//    IplImage *subImg;
//    IplImage *subGray;
//    IplImage *subBW;
    unsigned char* dataImg;
    unsigned char* yDataImg;
    unsigned char* data;
    unsigned char* yData;
    int dd;
    float scaleR, scaleG, scaleB, scaleSum;

    int minGrey; // minimaler Grauwert
    Vec2F subCenter;
    int maxThreshold;
    int step;
    bool foundHead;
    Vec2F moveDir, boxImageCentre;
    int xygrey;
    double markerSize, cmPerPixelAvg;
    QPointF cmPerPixel;

    for (j = 0; j < rectPlotItem->mapNum(); j++)
    {
        if (j == controlWidget->mapNr->value())
            nr = rectPlotItem->mapNum()-1;
        else if (j == rectPlotItem->mapNum()-1)
            nr = controlWidget->mapNr->value();
        else
            nr = j;

        setColorParameter(rectPlotItem->getMap(nr).fromColor(), rectPlotItem->getMap(nr).toColor(), rectPlotItem->getMap(nr).invHue(), param);

        // zentralen farbton heraussuchen
        if (param.inversHue)
            midHue.setHsv(2*((param.h_low+(param.h_high-param.h_low)/2+90)%180),255,255);
        else
            midHue.setHsv(2*(param.h_low+(param.h_high-param.h_low)/2),255,255);
//        debout << "midhue --------------------------------------: "<< param.h_low*2 <<" "<< param.h_high*2 <<" "<<param.inversHue<<" " <<midHue.hue()<<endl;

        // erzeuge speicherplatz fuer mask
        // abfrage wird in createMask gemacht
        //if (cmItem->getMask() == NULL || (cmItem->getMask() != NULL && (cmItem->getMask()->width != img->width || cmItem->getMask()->height != img->height)))
        binary = cmItem->createMask(img.cols, img.rows); // erzeugt binary mask mit groesse von img

        // color thresholding
        thresholdHSV(img , binary, param);

        // close small holes: radius ( hole ) < radius ( close )
        if (useClose)
        {
//            morphologyEx(binary,binary,NULL,closeKernel,...);
            cv::morphologyEx(binary,binary,MORPH_CLOSE,getStructuringElement(MORPH_ELLIPSE,Size(2*radius_close+1, 2*radius_close+1),Point(radius_close, radius_close)));
//            cvMorphologyEx(binary, binary, NULL, closeKernel, CV_MOP_CLOSE);
        }
        // remove small blobs: radius ( blob ) < radius ( open )
        if (useOpen)
        {
//            morphologyEx(binary,binary,NULL,openKernel,...);
            cv::morphologyEx(binary,binary,MORPH_OPEN,getStructuringElement(MORPH_ELLIPSE,Size(2*radius_open+1, 2*radius_open+1),Point(radius_open, radius_open)));
//            cvMorphologyEx(binary, binary, NULL, openKernel, CV_MOP_OPEN);
        }

        Mat clone = binary.clone();
//        IplImage *clone = cvCloneImage(binary); // clone, da binary sonst veraendert wird
//        Mat clone(binary);
//        storage = cvCreateMemStorage(0);
        cv::findContours(clone,contours,cv::RETR_EXTERNAL,cv::CHAIN_APPROX_SIMPLE);
//        cvFindContours(clone, storage, &contour, sizeof(CvContour), CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE); // clone wird auch veraendert!!!
//        findContours(clone,contours,CV_RETR_EXTERNAL,CV_CHAIN_APPROX_SIMPLE);
//        firstContour = contour;
//        cvReleaseImage(&clone);

        // test each contour
        while (!contours.empty())
        {
            vector<Point> contour = contours.back();
//            area = cvContourArea(contour, CV_WHOLE_SEQ);
            area = cv::contourArea(contour);//,CV_WHOLE_SEQ);

            foundHead = false;

            box = cv::minAreaRect(contour);
//            box = cvMinAreaRect2(contour);

            if (box.size.height > box.size.width)
            {
                ratio = box.size.height/box.size.width;
                maxExpansion = box.size.height;
            }
            else
            {
                ratio = box.size.width/box.size.height;
                maxExpansion = box.size.width;
            }

            // contour at border of roi, than neglect because of object going out of region has moving center
//            pointArray = (CvPoint*)malloc(contour->total*sizeof(CvPoint));
//            cvCvtSeqToArray(contour, pointArray, CV_WHOLE_SEQ);// Get contour point set.
            atEdge = false;
            for(i=0; i<contour.size(); i++)
                if ((contour.at(i).x <= 1) || (contour.at(i).x >= img.cols-2) ||
                    (contour.at(i).y <= 1) || (contour.at(i).y >= img.rows-2))
                    atEdge = true;
//            free(pointArray);

            if (!atEdge && area >= minArea && area <= maxArea && ratio <= maxRatio)
            {
                foundHead = true;
                cx = myRound(box.center.x);
                cy = myRound(box.center.y);
                add = min(box.size.height,box.size.width)/4.;
                // farbe der Kopfbedeckung // an 3 punkten gemassen
                col.setRgb(getValue(img,cx,cy).rgb());//getR(img, cx, cy), getG(img, cx, cy), getB(img, cx, cy));
                col2.setRgb(getValue(img,cx+add,cy).rgb());//getR(img, cx+add, cy), getG(img, cx+add, cy), getB(img, cx+add, cy));
                if (col.value()<col2.value()) // die hellere Farbe nehmen
                    col = col2;
                col2.setRgb(getValue(img,cx-add,cy).rgb());//getR(img, cx-add, cy), getG(img, cx-add, cy), getB(img, cx-add, cy));
                if (col.value()<col2.value()) // die hellere Farbe nehmen
                    col = col2;

                // centre von gefundenem rect aber nicht in roi sondern in gesamtbild
                boxImageCentre = offset;
                boxImageCentre.setX(boxImageCentre.x()+box.center.x);
                boxImageCentre.setY(boxImageCentre.y()+box.center.y);

                if (useBlackDot)
                {
                    oriRect = cv::Rect(Point(0,0),Size(img.cols,img.rows));//     img.cvGetImageROI(img);;
                    // cropRect ist orthogonal und verzichtet auf rotation, genauer waere:
                    // http://answers.opencv.org/question/38452/how-to-extract-pixels-from-a-rotated-box2d/
                    // max wegen bildrand
                    cropRect.x = max(1, myRound(box.center.x-box.size.width/2.-border));
                    cropRect.y = max(1, myRound(box.center.y-box.size.height/2.-border));
                    // 1. rundet kaufmaennisch, 2. dann rundet zur naechst kleiner geraden zahl
                    // min wegen bildrand
                    cropRect.width = min(img.cols-cropRect.x-1, 2*border+(myRound(maxExpansion) & -2));
                    cropRect.height = min(img.rows-cropRect.y-1, 2*border+(myRound(maxExpansion) & -2));//cropRect.height = cropRect.width;

                    if (restrictPosition)
                    {
                        double xy,x1, x2, y1, y2;
                        //debout << boxImageCentre <<endl;
                        //debout << "box: x/y/w/h "<< box.center.x << " "<< box.center.y<< " " <<box.size.height << " "<< box.size.width <<endl;
                        //debout << "cropRect: x/y/w/h "<< cropRect.x<< " " << cropRect.y<< "  "<<cropRect.width<< " " << cropRect.height <<endl;
                        //debout << "subImg: w/h " << subImg->width<< " " << subImg->height <<endl;
                        // getAngleToGround = 90 senkrecht zum Boden
                        //debout << bS << endl;
                        xy=mainWindow->getImageItem()->getAngleToGround(boxImageCentre.x()+bS, boxImageCentre.y()+bS, 175);// getAngleToGround(float px, float py, float height)
                        x1=mainWindow->getImageItem()->getAngleToGround(boxImageCentre.x()+bS+10, boxImageCentre.y()+bS, 175);// getAngleToGround(float px, float py, float height)
                        x2=mainWindow->getImageItem()->getAngleToGround(boxImageCentre.x()+bS-10, boxImageCentre.y()+bS, 175);// getAngleToGround(float px, float py, float height)
                        y1=mainWindow->getImageItem()->getAngleToGround(boxImageCentre.x()+bS, boxImageCentre.y()+bS+10, 175);// getAngleToGround(float px, float py, float height)
                        y2=mainWindow->getImageItem()->getAngleToGround(boxImageCentre.x()+bS, boxImageCentre.y()+bS-10, 175);// getAngleToGround(float px, float py, float height)
                        //debout << x1 <<" "<<x2<<endl;
                        //debout << y1 << " " <<y2<<endl;

                        double subFactorBig = 1.-.75*(90.-xy)/90.; //  -.5 //in 1.0..0.25 // xy in 0..90
                        double subFactorSmall = .85; // .9
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
                                //debout << "links oben"<<endl;
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
                                //debout << "links unten"<<endl;
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
                                //debout << "rechts oben"<<endl;
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
                                //debout << "rechts unten"<<endl;
                            }
                        }

                        //debout << mainWindow->getExtrCalibration()->getImagePoint(Point3f(0,0,0))<<endl;
                        //Point3f p3d(0,0,0);
                        //p3d.x += controlWidget->getCalibCoord3DTransX();
                        //p3d.y += controlWidget->getCalibCoord3DTransY();
                        //p3d.z += controlWidget->getCalibCoord3DTransZ();
                        //p3d.x *= controlWidget->getCalibCoord3DSwapX() ? -1 : 1;
                        //p3d.y *= controlWidget->getCalibCoord3DSwapY() ? -1 : 1;
                        //p3d.z *= controlWidget->getCalibCoord3DSwapZ() ? -1 : 1;
                        //debout <<mainWindow->getExtrCalibration()->getImagePoint(p3d)<<endl<<endl;

                    }


//                    cvSetImageROI(img, cropRect);
//                    subImg = cvCreateImage(cvGetSize(img), img->depth, img->nChannels);
//                    cvCopy(img, subImg, NULL);
//                    cvSetImageROI(img, oriRect);

//                    subImg = img.clone();
                    subImg = img(cropRect);

//                    subGray = cvCreateImage(cvGetSize(subImg), IPL_DEPTH_8U, 1);
                    subGray.create(Size(subImg.rows,subImg.cols),CV_8UC1);
                    // liefert sehr dunkle bilder insbesondere bei rottoenen
                    //cvCvtColor(subImg, subGray, CV_BGR2GRAY);
                    // gewichtete umwandlung RGB nach Grey
                    dataImg = ((unsigned char*) subImg.data);
                    yDataImg = dataImg;
                    data = ((unsigned char*) subGray.data);
                    yData = data;
                    scaleR = midHue.redF(); scaleG = midHue.greenF(); scaleB = midHue.blueF();
                    scaleSum = scaleR+scaleG+scaleB;
                    scaleR/=scaleSum; scaleG/=scaleSum; scaleB/=scaleSum;

                    for (y = 0; y < subGray.rows; ++y)
                    {
                        for (x = 0; x < subGray.cols; ++x)
                        {
                            dd = scaleB*dataImg[0]+scaleG*dataImg[1]+scaleR*dataImg[2];
                            *data = dd<256?dd:255;

                            ++data;
                            dataImg+=3;
                        }
                        data = (yData += subGray.cols/sizeof(char)); //width);
                        dataImg = (yDataImg += subImg.cols/sizeof(char)); //width);
                    }


//IplImage *tmpAusgabe = cvCloneImage(subImg);//subImg subGray

                    subBW = subGray.clone();
//                    subBW = cvCloneImage(subGray);
//                    cvReleaseImage(&subImg);

                    //        // smooth it, otherwise a lot of false circles may be detected
                    //        //GaussianBlur( gray, gray, Size(9, 9), 2, 2 );
                    //        vector<cv::Vec3f> circles;
                    //        cv::HoughCircles(gray, circles, CV_HOUGH_GRADIENT, 2, 10, 100, 50, 2, 20); // 22==minDist // , 1, 20
                    ////        cvHoughCircles(CvArr* image, void* circle_storage, int method, double dp, double min_dist, double param1=100, double param2=100, int min_radius=0, int max_radius=0 )
                    ////                image - 8-bit, single-channel, grayscale input image.
                    ////                circles - Output vector of found circles. Each vector is encoded as a 3-element floating-point vector (x, y, radius) .
                    ////                circle_storage - In C function this is a memory storage that will contain the output sequence of found circles.
                    ////                method - Detection method to use. Currently, the only implemented method is CV_HOUGH_GRADIENT , which is basically 21HT , described in [Yuen90].
                    ////                dp - Inverse ratio of the accumulator resolution to the image resolution. For example, if dp=1 , the accumulator has the same resolution as the input image. If dp=2 , the accumulator has half as big width and height.
                    ////                minDist - Minimum distance between the centers of the detected circles. If the parameter is too small, multiple neighbor circles may be falsely detected in addition to a true one. If it is too large, some circles may be missed.
                    ////                param1 - First method-specific parameter. In case of CV_HOUGH_GRADIENT , it is the higher threshold of the two passed to the Canny() edge detector (the lower one is twice smaller).
                    ////                param2 - Second method-specific parameter. In case of CV_HOUGH_GRADIENT , it is the accumulator threshold for the circle centers at the detection stage. The smaller it is, the more false circles may be detected. Circles, corresponding to the larger accumulator values, will be returned first.
                    ////                minRadius - Minimum circle radius.
                    ////                maxRadius - Maximum circle radius.
                    //        for( size_t i = 0; i < circles.size(); i++ )
                    //        {
                    //             cv::Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
                    //             int radius = cvRound(circles[i][2]);
                    //             // draw the circle center
                    //             //cvCircle(gray, center, 3, Scalar(0,255,0));
                    //             //cv::circle( gray, center, 3, Scalar(0,255,0), -1, 8, 0 );
                    //             // draw the circle outline
                    //             cvCircle( gray, center, radius, Scalar(0,0,255),1, 8, 0 );
                    //        }
                    //        debout << circles.size()<<endl;

//                    CvMemStorage *subStorage = cvCreateMemStorage(0);

                    maxThreshold = max(max(getValue(subGray, subGray.cols/2, subGray.rows/2).value(),
                                           getValue(subGray, subGray.cols/4, subGray.rows/2).value()),
                                           getValue(subGray, 3*subGray.cols/4, subGray.rows/2).value());
                    step = (maxThreshold-5)/5;
                    minGrey = 300;
                    //for (threshold = 40; threshold < 251 ; threshold += 30) // 40, 70, 100, 130, 160, 190, 220, 250
                    for (threshold = 5; threshold < maxThreshold; threshold += step) // col.value()
                    {
                        cv::threshold(subGray,subBW,threshold,255,cv::THRESH_BINARY);
//                        cvThreshold(subGray, subBW, threshold, 255, CV_THRESH_BINARY);

                        // find contours and store them all as a list
                        cv::findContours(subBW,subContours,cv::RETR_LIST,cv::CHAIN_APPROX_SIMPLE);
//                        cvFindContours(subBW, subStorage, &subFirstContour, sizeof(CvContour), CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE); // gray wird auch veraendert!!!

                        // test each contour
                        while (!subContours.empty())
                        {
                            vector<Point> subContour = subContours.back();

                            if (subContour.size() > 5) // This is number point in contour
                            {

// in rot contouren in eine drehrichtung, in gruen andere
//cvDrawContours(tmpAusgabe,subContours,CV_RGB(255,0,0),CV_RGB(0,255,0),0,1,8,cvPoint(0,0));

                                subBox = cv::minAreaRect(subContour);//cvMinAreaRect2(subContours);

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

                                //QPointF pp(100, 100);
                                //debout << "headsize: "<<mainWindow->getHeadSize(&pp)<<endl; // 44 pixel fuer schulexperiment
                                //QPointF ww(1400, 1400);
                                //debout << "headsize: "<<mainWindow->getHeadSize(&ww)<<endl; // 44 pixel fuer schulexperiment
                                //double headSize = 44.5; // Kopflaenge in Pixel // spaeter berechnen -> funktion von daniel an bestimmten pixel
                                //double markerSize = headSize / 4.5; // fuer schule 5cm // markerdurchmesser in pixel //spaeter berechnen
                                cmPerPixel = mainWindow->getImageItem()->getCmPerPixel(cropRect.x+subBox.center.x, cropRect.y+subBox.center.y, controlWidget->mapDefaultHeight->value());
                                cmPerPixelAvg = (cmPerPixel.x()+cmPerPixel.y())/2.;
                                //debout << 21./cmPerPixelAvg<<endl;
                                //debout << 5./cmPerPixelAvg<<endl;
                                //debout << PI*0.25 * HEAD_SIZE/cmPerPixelAvg * 14./cmPerPixelAvg<<endl;
                                markerSize = dot_size/cmPerPixelAvg;//war: 5cm// war WDG: = 16; war GymBay: = headSize / 4.5;
                                //debout << markerSize<<endl;

                                // maximalseitenverhaeltnis && minimal und maximaldurchmesser
//                                debout << "subRatio: "<<subRatio<<endl;
//                                debout << "subMaxExpansion: "<<subMaxExpansion<<endl;
//                                debout << "markerSizeminmax: "<<markerSize*1.5<<" " << markerSize/1.5 <<endl;
                                if (subRatio < 1.8 && subMaxExpansion < markerSize*1.5 && subMaxExpansion > markerSize/2)//1.5
                                {
                                    // IN OPENCV2.1 liefert cvContourArea KEIN VORZEICHEN ZUM ERKENNEN DER DREHRICHTUNG!!!! es ist ein optionaler paramter hinzugefuegt worden!!!!

                                    //else
                                    subContourArea = cv::contourArea(subContour,true);
//                                    subContourArea = cvContourArea(subContours,CV_WHOLE_SEQ, true);
                                    cx = myRound(subBox.center.x);
                                    cy = myRound(subBox.center.y);

                                    //If third argument is True, it finds the signed distance.
                                    //If False, it finds whether the point is inside or outside or on the contour +100, -100, 0 (it returns +1, -1, 0 respectively).
                                    // ist schwarzer punkt innerhalb kopfkontur
                                    //CvPoint2D32f pt;
//                                    if (subContourArea>0)
//                                    {
//                                        debout <<endl<< "Inside head: "<<cvPointPolygonTest(contour,cvPoint2D32f(cropRect.x+subBox.center.x, cropRect.y+subBox.center.y),false) <<endl; // liegt pt inside contour?
//                                        debout << "value: "<<getGrey(subGray, cx, cy)<<endl;
//                                        debout << "maxThreshold: "<<maxThreshold<<endl;
//                                    }
                                    // darker inside && dark inside &&  mittelpunkt in kopfkontur
                                    xygrey = getValue(subGray, cx, cy).value();
//                                    debout << "xygrey: " << xygrey << endl;
                                    if (subContourArea>0 && xygrey<min(150,2*maxThreshold/3) && (0<cv::pointPolygonTest(contour,Point2f(cropRect.x+subBox.center.x, cropRect.y+subBox.center.y),false))) // dark inside
                                    {
                                        if (minGrey > xygrey)
                                        {
                                            minGrey = xygrey;
                                            subCenter.set(cropRect.x+subBox.center.x, cropRect.y+subBox.center.y);
                                        }
//cvDrawContours(tmpAusgabe,subContours,CV_RGB(255,255,255),CV_RGB(255,255,255),0,1,8,cvPoint(0,0));
                                    }
                                }
                            }

                            // take the next contour
//                            subContours = subContours->h_next;
                            subContours.pop_back();
                        }
//                        if (subFirstContour)
//                            cvClearSeq(subFirstContour); // not free only available for next push
                    }
//                    cvReleaseMemStorage(&subStorage);
//                    cvReleaseImage(&subGray);
//                    cvReleaseImage(&subBW);

//////if (restrictPosition) // (j==0)
////{
//    static int zaehler = 0;
//    QString imgfn = QString("D:\\image%1.jpg").arg(zaehler);
//    zaehler+=1;
////    //QByteArray ba = imgfn.toLatin1().data();
////    //const char *c_str2 = ba.data();

////    debout << imgfn.toLatin1().data() <<endl;
//    cv::imwrite(imgfn.toLatin1().data(), tmpAusgabe);
////    //cvNamedWindow("subGray", CV_WND_PROP_ASPECTRATIO| CV_WINDOW_KEEPRATIO ); // 0 wenn skalierbar sein soll  CV_WINDOW_AUTOSIZE
////    //cv::imshow("subGray", tmpAusgabe);
////    //cvWaitKey(1000); // zahl statt null, wenn nach bestimmter zeit in Millisekunden weitergegangen werden soll
////}
                }
//                debout << "contours.size(): " << contours.size() << "Color: " << col << endl;
                if (!useBlackDot && foundHead)
                {
                    if (autoCorrect && !autoCorrectOnlyExport)
                    {
                        moveDir = autoCorrectColorMarker(boxImageCentre, controlWidget);

                        crossList->append(TrackPoint(Vec2F(box.center.x, box.center.y)+moveDir, 100, Vec2F(box.center.x, box.center.y), col)); // 100 beste qualitaet
                    }
                    else
                        crossList->append(TrackPoint(Vec2F(box.center.x, box.center.y), 100, Vec2F(box.center.x, box.center.y), col)); // 100 beste qualitaet
                }
                else if (minGrey <260) // mit gefundenem schwarzem punkt
                    crossList->append(TrackPoint(subCenter, 100, Vec2F(box.center.x, box.center.y), col)); // 100 beste qualitaet
                else if (!ignoreWithoutMarker && foundHead)
                {
                    if (autoCorrect && !autoCorrectOnlyExport)
                    {
                        moveDir = autoCorrectColorMarker(boxImageCentre, controlWidget);

                        crossList->append(TrackPoint(Vec2F(box.center.x, box.center.y)+moveDir, 100, Vec2F(box.center.x, box.center.y), col)); // 100 beste qualitaet
                    }
                    else
                        crossList->append(TrackPoint(Vec2F(box.center.x, box.center.y), 90, Vec2F(box.center.x, box.center.y), col)); // 100 beste qualitaet
                }
            }

            // take the next contour
//            contour = contour->h_next;
            contours.pop_back();
        }
//        if (firstContour)
//            cvClearSeq(firstContour);
//        cvReleaseMemStorage(&storage);

        //debout << "Found " << crossList->size() << " features.\n";
    }

//    cvReleaseStructuringElement(&closeKernel);
//    cvReleaseStructuringElement(&openKernel);
}


//  Run color blob detection on an image to find features.
//  Runs color thresholding to binarize the image and computes connected components.
//  The features are the center of gravity of each connected component.
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
    double maxExpansion;
//    CvPoint* pointArray;
    int i;
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
            maxExpansion = box.size.height;
        }
        else
        {
            ratio = box.size.width/box.size.height;
            maxExpansion = box.size.width;
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
        for(i=0; i<contour.size(); i++)
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

void findCodeMarker(Mat &img, QList<TrackPoint> *crossList, Control *controlWidget)
{
//#if 0 // Maik temporaer, damit es auf dem Mac laeuft

    CodeMarkerItem* codeMarkerItem = controlWidget->getMainWindow()->getCodeMarkerItem();
    CodeMarkerWidget* codeMarkerWidget = controlWidget->getMainWindow()->getCodeMarkerWidget();

//    Mat mat = cvarrToMat(img);//.clone(); // for marker detection and debug output (show detected candidates)

//    debout << "w: " << img->width << " h: " << img->height << endl;
//    IplImage* binary = codeMarkerItem->createMask(img->width, img->height); // create binary mask with size of img

    Ptr<aruco::Dictionary> dictionary = aruco::getPredefinedDictionary(aruco::PREDEFINED_DICTIONARY_NAME(codeMarkerWidget->dictList->currentIndex()));

    Ptr<aruco::DetectorParameters> detectorParams = aruco::DetectorParameters::create();

    double minMarkerPerimeterRate = 0.03, maxMarkerPerimeterRate = 4, minCornerDistanceRate = 0.05, minMarkerDistanceRate = 0.05;

    Petrack *mainWindow = controlWidget->getMainWindow();

    int bS = mainWindow->getImageBorderSize();

    //debout << "Image w=" << mainWindow->getImage()->width() << ", h=" << mainWindow->getImage()->height() << " bS=" << bS << endl;

    if (controlWidget->getCalibCoordDimension() == 0) // 3D
    {
        QRect rect(myRound(mainWindow->getRecoRoiItem()->rect().x()),//+controlWidget->getMainWindow()->getImageBorderSize()),
                   myRound(mainWindow->getRecoRoiItem()->rect().y()),//+controlWidget->getMainWindow()->getImageBorderSize()),
                   myRound(mainWindow->getRecoRoiItem()->rect().width()),
                   myRound(mainWindow->getRecoRoiItem()->rect().height()));
        QPointF p1 = mainWindow->getImageItem()->getCmPerPixel(rect.x(),rect.y(),controlWidget->mapDefaultHeight->value()),
                p2 = mainWindow->getImageItem()->getCmPerPixel(rect.x()+rect.width(),rect.y(),controlWidget->mapDefaultHeight->value()),
                p3 = mainWindow->getImageItem()->getCmPerPixel(rect.x(),rect.y()+rect.height(),controlWidget->mapDefaultHeight->value()),
                p4 = mainWindow->getImageItem()->getCmPerPixel(rect.x()+rect.width(),rect.y()+rect.height(),controlWidget->mapDefaultHeight->value());

//        QPointF p1 = mainWindow->getImageItem()->getCmPerPixel(0,0,controlWidget->mapDefaultHeight->value()),
//                p2 = mainWindow->getImageItem()->getCmPerPixel(mainWindow->getImage()->width()-bS,0,controlWidget->mapDefaultHeight->value()),
//                p3 = mainWindow->getImageItem()->getCmPerPixel(0,mainWindow->getImage()->height()-bS,controlWidget->mapDefaultHeight->value()),
//                p4 = mainWindow->getImageItem()->getCmPerPixel(mainWindow->getImage()->width()-bS,mainWindow->getImage()->height()-bS,controlWidget->mapDefaultHeight->value());

        double cmPerPixel_min = min(min(min(p1.x(), p1.y()), min(p2.x(), p2.y())),
                                    min(min(p3.x(), p3.y()), min(p4.x(), p4.y())));
        double cmPerPixel_max = max(max(max(p1.x(), p1.y()), max(p2.x(), p2.y())),
                                    max(max(p3.x(), p3.y()), max(p4.x(), p4.y())));

//        minMarkerPerimeterRate = (codeMarkerWidget->minMarkerPerimeter->value()*4/cmPerPixel_max)/max(mainWindow->getImage()->width()-bS,mainWindow->getImage()->height()-bS);
//        maxMarkerPerimeterRate = (codeMarkerWidget->maxMarkerPerimeter->value()*4/cmPerPixel_min)/max(mainWindow->getImage()->width()-bS,mainWindow->getImage()->height()-bS);

        minMarkerPerimeterRate = (codeMarkerWidget->minMarkerPerimeter->value()*4/cmPerPixel_max)/max(rect.width(),rect.height());
        maxMarkerPerimeterRate = (codeMarkerWidget->maxMarkerPerimeter->value()*4/cmPerPixel_min)/max(rect.width(),rect.height());

        minCornerDistanceRate = codeMarkerWidget->minCornerDistance->value();
        minMarkerDistanceRate = codeMarkerWidget->minMarkerDistance->value();
        //debout << "image w=" << mainWindow->getImage()->width() << ", h=" << mainWindow->getImage()->height() << ", bS=" << bS << endl;
        //debout << "cm/px: min=" << cmPerPixel_min << " max=" << cmPerPixel_max << " minRate: " << minMarkerPerimeterRate << " maxRate: " << maxMarkerPerimeterRate << endl;

    }else // 2D
    {
        double cmPerPixel = mainWindow->getImageItem()->getCmPerPixel();
        minMarkerPerimeterRate = (codeMarkerWidget->minMarkerPerimeter->value()*4/cmPerPixel)/max(mainWindow->getImage()->width()-bS,mainWindow->getImage()->height()-bS);
        maxMarkerPerimeterRate = (codeMarkerWidget->maxMarkerPerimeter->value()*4/cmPerPixel)/max(mainWindow->getImage()->width()-bS,mainWindow->getImage()->height()-bS);
//        debout << "cm/px: " << cmPerPixel << " minRate: " << minMarkerPerimeterRate << " maxRate: " << maxMarkerPerimeterRate << endl;

        minCornerDistanceRate = codeMarkerWidget->minCornerDistance->value();
        minMarkerDistanceRate = codeMarkerWidget->minMarkerDistance->value();
    }

//    debout << "MinMarkerPerimeterRate: " << minMarkerPerimeterRate << " MaxMarkerPerimeterRate: " << maxMarkerPerimeterRate << endl;

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
        detectorParams->cornerRefinementMethod            = cv::aruco::CornerRefineMethod::CORNER_REFINE_CONTOUR;
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

    //debout << "findCodeMarker: dictID: " << codeMarkerWidget->dictList->currentIndex() << endl; // used dictionary

    vector<int> ids;
    vector<vector<Point2f> > corners, rejected;
    ids.clear();
    corners.clear();
    rejected.clear();

#ifdef TIME_MEASUREMENT
    //        "==========: "
    debout << "start detectCodeMarkers : " << getElapsedTime() <<endl;
#endif

//    Mat copy = cvarrToMat(img).clone();
//    debout << "img(" << bS << "): " << img << " channels: " << img->nChannels << " depth: " << img->depth << endl;
//    debout << "test" << endl;
    aruco::detectMarkers(img/*copy.clone()*/, dictionary, corners, ids, detectorParams, rejected);
//    debout << " --> ok " << endl;
//    aruco::detectMarkers(cvarrToMat(img),dictionary,corners,ids);
#ifdef TIME_MEASUREMENT
    //        "==========: "
    debout << "end detectCodeMarkers  : " << getElapsedTime() <<endl;
#endif

    codeMarkerItem->setDetectedMarkers(corners,ids);
    codeMarkerItem->setRejectedMarkers(rejected);

    double x,y;
    int i;



    // detected code markers
    for(i = 0; i<ids.size(); i++)
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

        x = (corners.at(i).at(0).x+corners.at(i).at(1).x+corners.at(i).at(2).x+corners.at(i).at(3).x)*0.25;
        y = (corners.at(i).at(0).y+corners.at(i).at(1).y+corners.at(i).at(2).y+corners.at(i).at(3).y)*0.25;

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

    int i, threshold, plus, count;
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
                    if (box.size.width<box.size.height)
                        angle -= PI/2;
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



// returns list of mid point of black crosses on white ground in region of interest roi
// in image iplImg
void getMarkerPos(Mat &img, QRect &roi, QList<TrackPoint> *crossList, Control *controlWidget,
                 int borderSize, BackgroundFilter *bgFilter)
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
