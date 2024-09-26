/*
 * PeTrack - Software for tracking pedestrians movement in videos
 * Copyright (C) 2024 Forschungszentrum Jülich GmbH, IAS-7
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

#include "recognition.h"

#include "codeMarkerItem.h"
#include "codeMarkerWidget.h"
#include "colorMarkerItem.h"
#include "colorMarkerWidget.h"
#include "control.h"
#include "helper.h"
#include "logger.h"
#include "markerCasern.h"
#include "markerHermes.h"
#include "markerJapan.h"
#include "multiColorMarkerItem.h"
#include "multiColorMarkerWidget.h"
#include "pMessageBox.h"
#include "roiItem.h"
#include "tracker.h"
#include "worldImageCorrespondence.h"

#include <QPointF>
#include <QRect>
#include <bitset>
#include <opencv2/objdetect/aruco_detector.hpp>
#include <opencv2/objdetect/aruco_dictionary.hpp>
#include <opencv2/opencv.hpp>
namespace reco
{
using namespace detail;

#define ELLIPSE_DISTANCE_TO_BORDER 10

struct ColorParameters
{
    int  h_low     = 0;
    int  h_high    = 359;
    int  s_low     = 0;
    int  s_high    = 255;
    int  v_low     = 0;
    int  v_high    = 255;
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

void thresholdHSV(const cv::Mat &src, cv::Mat &bin, const ColorParameters &param)
{
    int h, s, v;
    int x, y;

    cv::Mat hsv{src.size(), src.type()};
    cv::cvtColor(src, hsv, cv::COLOR_BGR2HSV);

    for(y = 0; y < bin.rows; ++y)
    {
        for(x = 0; x < bin.cols; ++x)
        {
            cv::Vec3b intensity = hsv.at<cv::Vec3b>(cv::Point(x, y));
            h                   = intensity.val[0];
            s                   = intensity.val[1];
            v                   = intensity.val[2];

            // apply the thresholds
            if((!param.inversHue && (h < param.h_low || param.h_high < h)) ||
               (param.inversHue && (h >= param.h_low && param.h_high >= h)) || (s < param.s_low || param.s_high < s) ||
               (v < param.v_low || param.v_high < v))
            {
                bin.at<uchar>(cv::Point(x, y)) = 0;
            }
            else
            {
                bin.at<uchar>(cv::Point(x, y)) = 255;
            }
        }
    }
}


void setColorParameter(const QColor &fromColor, const QColor &toColor, bool inversHue, ColorParameters &param)
{
    if(fromColor.hue() > toColor.hue())
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
    // param.h_low *= 255./359.;
    // param.h_high *= 255./359.;
    // hue scheint nach Konversion bgr nach hsv in opencv nur zwischen 0..179 zu laufen
    param.h_low /= 2;
    param.h_high /= 2;
    if(fromColor.saturation() > toColor.saturation())
    {
        param.s_low  = toColor.saturation();
        param.s_high = fromColor.saturation();
    }
    else
    {
        param.s_low  = fromColor.saturation();
        param.s_high = toColor.saturation();
    }
    if(fromColor.value() > toColor.value())
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

/**
 * @brief calculates pixel-displacement due to oblique/angular view
 *
 * Relevant for Color Markers. boxImageCentre without border. More information: Dissertation Maik pp. 138
 *
 * @param boxImageCentre
 * @param controlWidget
 * @return
 */
Vec2F autoCorrectColorMarker(const Vec2F &boxImageCentre, Control *controlWidget)
{
    Petrack    *mainWindow = controlWidget->getMainWindow();
    auto        extrParams = controlWidget->getExtrinsicParameters();
    auto        trans      = controlWidget->getCalibCoord3DTrans();
    cv::Point2f tp         = mainWindow->getExtrCalibration()->getImagePoint(
        cv::Point3f(-trans.x() - extrParams.trans1, -trans.y() - extrParams.trans2, 0));
    Vec2F pixUnderCam(tp.x, tp.y); // CvPoint
    Vec2F boxImageCentreWithBorder = boxImageCentre;
    boxImageCentreWithBorder += Vec2F(mainWindow->getImageBorderSize(), mainWindow->getImageBorderSize());
    pixUnderCam += Vec2F(mainWindow->getImageBorderSize(), mainWindow->getImageBorderSize());
    float angle = 90 - mainWindow->getWorldImageCorrespondence().getAngleToGround(
                           boxImageCentreWithBorder.x(),
                           boxImageCentreWithBorder.y(),
                           175); // Hoehe 175 cm ist egal, da auf jeder Hoehe gleicher Winkel

    Vec2F moveDir = boxImageCentreWithBorder - pixUnderCam;
    moveDir.normalize();

    cv::Point3f p3x1, p3x2;
    p3x1 = mainWindow->getExtrCalibration()->get3DPoint(cv::Point2f(boxImageCentre.x(), boxImageCentre.y()), 175);
    p3x2 = mainWindow->getExtrCalibration()->get3DPoint(
        cv::Point2f(boxImageCentre.x() + moveDir.x(), boxImageCentre.y() + moveDir.y()), 175);
    p3x1 = p3x1 - p3x2;
    Vec2F cmPerPixel(p3x1.x, p3x1.y);

    return (0.12 * angle / cmPerPixel.length()) * moveDir; // Maik Dissertation Seite 138
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
[[nodiscard]] std::vector<ColorBlob> detail::findColorBlob(const ColorBlobDetectionParams &options)
{
    std::vector<ColorBlob> colorBlobs;
    ColorParameters        param;

    setColorParameter(options.fromColor, options.toColor, options.invHue, param);
    // Marker Color is set according to the color which is searched
    // Calculating is only neccessary for casern-marker like markers
    const QColor markerColor = [param]()
    {
        if(param.inversHue)
        {
            return QColor::fromHsv(
                2 * ((param.h_low + (param.h_high - param.h_low) / 2 + 90) % 180),
                (param.s_high + param.s_low) / 2,
                (param.v_high + param.v_low) / 2);
        }
        return QColor::fromHsv(
            2 * (param.h_low + (param.h_high - param.h_low) / 2),
            (param.s_high + param.s_low) / 2,
            (param.v_high + param.v_low) / 2);
    }(); // Lambda so markerColor can be const

    cv::Mat img    = options.img;
    cv::Mat binary = options.binary;

    // color thresholding
    thresholdHSV(img, binary, param);

    // close small holes: radius ( hole ) < radius ( close )
    if(options.useClose)
    {
        const int radiusClose = options.radiusClose;
        cv::morphologyEx(
            binary,
            binary,
            cv::MORPH_CLOSE,
            getStructuringElement(
                cv::MORPH_ELLIPSE,
                cv::Size(2 * radiusClose + 1, 2 * radiusClose + 1),
                cv::Point(radiusClose, radiusClose)));
    }

    // remove small blobs: radius ( blob ) < radius ( open )
    if(options.useOpen)
    {
        const int radiusOpen = options.radiusOpen;
        cv::morphologyEx(
            binary,
            binary,
            cv::MORPH_OPEN,
            getStructuringElement(
                cv::MORPH_ELLIPSE,
                cv::Size(2 * radiusOpen + 1, 2 * radiusOpen + 1),
                cv::Point(radiusOpen, radiusOpen)));
    }

    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(binary, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    for(const auto &contour : contours)
    {
        double area = cv::contourArea(contour);

        cv::RotatedRect box = cv::minAreaRect(contour);

        float  maxExpansion;
        double ratio;
        if(box.size.height > box.size.width)
        {
            ratio        = static_cast<double>(box.size.height / box.size.width);
            maxExpansion = box.size.height;
        }
        else
        {
            ratio        = static_cast<double>(box.size.width / box.size.height);
            maxExpansion = box.size.width;
        }

        // contour at border of roi, than neglect because of object going out of region has moving center
        bool atEdge = std::any_of(
            contour.cbegin(),
            contour.cend(),
            [&](const cv::Point &p)
            { return (p.x <= 1) || (p.x >= img.cols - 2) || (p.y <= 1) || (p.y >= img.rows - 2); });

        if(atEdge)
        {
            continue;
        }
        if(area < options.minArea || area > options.maxArea)
        {
            continue;
        }
        if(ratio > options.maxRatio)
        {
            continue;
        }

        // centre of rect in the coordinates of the whole image (instead of ROI)
        Vec2F boxImageCentre = options.offset;
        boxImageCentre.setX(boxImageCentre.x() + static_cast<double>(box.center.x));
        boxImageCentre.setY(boxImageCentre.y() + static_cast<double>(box.center.y));

        // TODO maybe use move constructor to increase perf for contour(then not const anymore)
        colorBlobs.push_back({box, boxImageCentre, markerColor, contour, static_cast<double>(maxExpansion)});
    }
    return colorBlobs;
}

/**
 * @brief Restricts the position in which the black dot should be according to position in image.
 *
 * @param[in] blob ColorBlob for which to restrict the position
 * @param[in] worldImageCorr reference to world to image correspondence
 * @param[in] bS bordersize
 * @param[in,out] cropRect cropRect to restrict/resize
 */
void detail::restrictPositionBlackDot(
    ColorBlob                      &blob,
    const WorldImageCorrespondence *worldImageCorr,
    int                             bS,
    cv::Rect                       &cropRect)
{
    double xy, x1, x2, y1, y2;

    Vec2F &boxImageCentre = blob.imageCenter;

    xy = worldImageCorr->getAngleToGround(boxImageCentre.x() + bS, boxImageCentre.y() + bS, 175);
    x1 = worldImageCorr->getAngleToGround(boxImageCentre.x() + bS + 10, boxImageCentre.y() + bS, 175);
    x2 = worldImageCorr->getAngleToGround(boxImageCentre.x() + bS - 10, boxImageCentre.y() + bS, 175);
    y1 = worldImageCorr->getAngleToGround(boxImageCentre.x() + bS, boxImageCentre.y() + bS + 10, 175);
    y2 = worldImageCorr->getAngleToGround(boxImageCentre.x() + bS, boxImageCentre.y() + bS - 10, 175);

    double           subFactorBig   = 1. - .75 * (90. - xy) / 90.; //  -.5 //in 1.0..0.25 // xy in 0..90
    constexpr double subFactorSmall = .85;                         // .9
    if(x1 > x2)                                                    // Person links von Achse
    {
        if(y1 > y2) // Person links oben von Achse
        {
            if(x1 > y1) // starker Beschnitt rechts, kleiner Beschnitt unten
            {
                cropRect.width *= subFactorBig;    // ist int
                cropRect.height *= subFactorSmall; // ist int
            }
            else // starker Beschnitt unten, kleiner Beschnitt rechts
            {
                cropRect.width *= subFactorSmall; // ist int
                cropRect.height *= subFactorBig;  // ist int
            }
        }
        else // Person links unten von Achse
        {
            if(x1 > y2) // starker Beschnitt rechts, kleiner Beschnitt oben
            {
                cropRect.y = cropRect.y + cropRect.height * (1 - subFactorSmall);
                cropRect.width *= subFactorBig;    // ist int
                cropRect.height *= subFactorSmall; // ist int
            }
            else // starker Beschnitt oben, kleiner Beschnitt rechts
            {
                cropRect.y = cropRect.y + cropRect.height * (1 - subFactorBig);
                cropRect.width *= subFactorSmall; // ist int
                cropRect.height *= subFactorBig;  // ist int
            }
        }
    }
    else // Person rechts von Achse
    {
        if(y1 > y2) // Person rechts oben von Achse
        {
            if(x2 > y1) // starker Beschnitt links, kleiner Beschnitt unten
            {
                cropRect.x = cropRect.x + cropRect.width * (1 - subFactorBig);
                cropRect.width *= subFactorBig;    // ist int
                cropRect.height *= subFactorSmall; // ist int
            }
            else // starker Beschnitt unten, kleiner Beschnitt links
            {
                cropRect.x = cropRect.x + cropRect.width * (1 - subFactorSmall);
                cropRect.width *= subFactorSmall; // ist int
                cropRect.height *= subFactorBig;  // ist int
            }
        }
        else // Person rechts unten von Achse
        {
            if(x2 > y2) // starker Beschnitt links, kleiner Beschnitt oben
            {
                cropRect.x = cropRect.x + cropRect.width * (1 - subFactorBig);
                cropRect.y = cropRect.y + cropRect.height * (1 - subFactorSmall);
                cropRect.width *= subFactorBig;    // ist int
                cropRect.height *= subFactorSmall; // ist int
            }
            else // starker Beschnitt oben, kleiner Beschnitt links
            {
                cropRect.x = cropRect.x + cropRect.width * (1 - subFactorSmall);
                cropRect.y = cropRect.y + cropRect.height * (1 - subFactorBig);
                cropRect.width *= subFactorSmall; // ist int
                cropRect.height *= subFactorBig;  // ist int
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
cv::Mat detail::customBgr2Gray(const cv::Mat &subImg, const QColor &midHue)
{
    cv::Mat subGray;

    float scaleR   = midHue.redF();
    float scaleG   = midHue.greenF();
    float scaleB   = midHue.blueF();
    float scaleSum = scaleR + scaleG + scaleB;
    scaleR /= scaleSum;
    scaleG /= scaleSum;
    scaleB /= scaleSum;
    cv::Mat bgrToGray = (cv::Mat_<double>(1, 3) << scaleB, scaleG, scaleR);

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
void detail::refineWithBlackDot(
    std::vector<ColorBlob> &blobs,
    const cv::Mat          &img,
    QList<TrackPoint>      &crossList,
    const BlackDotOptions  &options)
{
    constexpr int                   border                = 4; // zusaetzlicher rand um subrects
    const int                       bS                    = options.borderSize;
    const bool                      restrictPosition      = options.restrictPosition;
    const WorldImageCorrespondence *worldImageCorr        = options.worldImageCorr;
    const QColor                    midHue                = options.midHue;
    const double                    dotSize               = options.dotSize;
    const bool                      ignoreWithoutMarker   = options.ignoreWithoutMarker;
    const bool                      autoCorrect           = options.autoCorrect;
    const bool                      autoCorrectOnlyExport = options.autoCorrectOnlyExport;
    Control                        *controlWidget         = options.controlWidget;

    for(ColorBlob &blob : blobs)
    {
        cv::Rect         cropRect;
        cv::RotatedRect &box = blob.box;
        cropRect.x           = std::max(1, myRound(box.center.x - box.size.width / 2 - border));
        cropRect.y           = std::max(1, myRound(box.center.y - box.size.height / 2 - border));
        // 1. rundet kaufmaennisch, 2. dann rundet zur naechst kleiner geraden zahl
        // min wegen bildrand
        cropRect.width  = std::min(img.cols - cropRect.x - 1, 2 * border + (myRound(blob.maxExpansion) & -2));
        cropRect.height = std::min(
            img.rows - cropRect.y - 1,
            2 * border + (myRound(blob.maxExpansion) & -2)); // cropRect.height = cropRect.width;

        if(restrictPosition)
        {
            restrictPositionBlackDot(blob, worldImageCorr, bS, cropRect);
        }

        // cvtColor results in really dark images, especially with red shades
        // so using custom conversion weighted by midHue
        cv::Mat subImg  = img(cropRect);
        cv::Mat subGray = customBgr2Gray(subImg, midHue);
        cv::Mat subBW;

        double maxThreshold = std::max(
            std::max(
                getValue(subGray, subGray.cols / 2, subGray.rows / 2).value(),
                getValue(subGray, subGray.cols / 4, subGray.rows / 2).value()),
            getValue(subGray, 3 * subGray.cols / 4, subGray.rows / 2).value());
        int   step    = static_cast<int>((maxThreshold - 5)) / 5;
        int   minGrey = 300;
        Vec2F subCenter;
        for(int threshold = 5; threshold < maxThreshold; threshold += step)
        {
            cv::threshold(subGray, subBW, threshold, 255, cv::THRESH_BINARY);

            // find contours and store them all as a list
            std::vector<std::vector<cv::Point>> subContours;
            cv::findContours(subBW, subContours, cv::RETR_LIST, cv::CHAIN_APPROX_SIMPLE);

            // test each contour
            for(auto &subContour : subContours)
            {
                if(subContour.size() > 5) // This is number point in contour
                {
                    cv::RotatedRect subBox = cv::minAreaRect(subContour);
                    double          subRatio;
                    double          subMaxExpansion;
                    if(subBox.size.height > subBox.size.width)
                    {
                        subRatio        = subBox.size.height / subBox.size.width;
                        subMaxExpansion = subBox.size.height;
                    }
                    else
                    {
                        subRatio        = subBox.size.width / subBox.size.height;
                        subMaxExpansion = subBox.size.width;
                    }

                    QPointF cmPerPixel = worldImageCorr->getCmPerPixel(
                        cropRect.x + subBox.center.x, cropRect.y + subBox.center.y, controlWidget->getDefaultHeight());
                    double cmPerPixelAvg = (cmPerPixel.x() + cmPerPixel.y()) / 2.;
                    double markerSize =
                        dotSize / cmPerPixelAvg; // war: 5cm// war WDG: = 16; war GymBay: = headSize / 4.5;

                    // maximalseitenverhaeltnis && minimal und maximaldurchmesser
                    if(subRatio < 1.8 && subMaxExpansion < markerSize * 1.5 && subMaxExpansion > markerSize / 2) // 1.5
                    {
                        double subContourArea = cv::contourArea(subContour, true);
                        int    cx             = myRound(subBox.center.x);
                        int    cy             = myRound(subBox.center.y);

                        // darker inside && dark inside &&  mittelpunkt in kopfkontur
                        int xygrey = getValue(subGray, cx, cy).value();
                        //                                    debout << "xygrey: " << xygrey << endl;
                        if(subContourArea > 0 && xygrey < std::min(150., 2 * maxThreshold / 3) &&
                           (0 < cv::pointPolygonTest(
                                    blob.contour,
                                    cv::Point2f(cropRect.x + subBox.center.x, cropRect.y + subBox.center.y),
                                    false))) // dark inside
                        {
                            if(minGrey > xygrey)
                            {
                                minGrey = xygrey;
                                subCenter.set(cropRect.x + subBox.center.x, cropRect.y + subBox.center.y);
                            }
                        }
                    }
                }
            }
        }

        if(minGrey < 260) // mit gefundenem schwarzem punkt
        {
            crossList.append(
                TrackPoint(subCenter, 100, Vec2F(box.center.x, box.center.y), blob.color)); // 100 beste qualitaet
        }
        else if(!ignoreWithoutMarker)
        {
            if(autoCorrect && !autoCorrectOnlyExport)
            {
                Vec2F moveDir = autoCorrectColorMarker(blob.imageCenter, controlWidget);

                crossList.append(TrackPoint(
                    Vec2F(box.center.x, box.center.y) + moveDir,
                    100,
                    Vec2F(box.center.x, box.center.y),
                    blob.color)); // 100 beste qualitaet
            }
            else
            {
                crossList.append(TrackPoint(
                    Vec2F(box.center.x, box.center.y),
                    90,
                    Vec2F(box.center.x, box.center.y),
                    blob.color)); // 100 beste qualitaet
            }
        }
    }
}

/**
 * Filters passed codes and removes all that are not inside the given bounding rect
 * @param codes the detected codes
 * @param bound the Rect where the codes have to be inside of
 * @return A List of all TrackPoints inside the boundingRect of the given blob.
 */
QList<TrackPoint>
detail::filterCodesByBoundingRect(const QList<TrackPoint> &codes, const cv::Rect &bound, const Vec2F offset)
{
    QList<TrackPoint> pointsInside;

    for(const auto &code : codes)
    {
        if((code + offset).toCvPoint().inside(bound))
        {
            pointsInside.append(code);
        }
    }

    return pointsInside;
}

/**
 * @brief Choose the correct out of multiple candidate codes used for recognition.
 *
 * If multiple codes are present, take the one that is closest to the reference point.
 *
 * @param codes the list of codes of which one should be chosen.
 * @param reference the point, to which the closest code is chosen.
 *
 * @return the code closest to the reference point.
 */
TrackPoint detail::resolveMoreThanOneCandidateCode(QList<TrackPoint> &codes, const Vec2F reference)
{
    return *std::min_element(
        codes.begin(),
        codes.end(),
        [&](const TrackPoint &a, const TrackPoint &b)
        { return reference.distanceToPoint(a) < reference.distanceToPoint(b); });
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
 * @param intrinsicCameraParams camera parameters needed to estimate arucomarker orientation
 */
void detail::refineWithAruco(
    std::vector<ColorBlob>      &blobs,
    const cv::Mat               &img,
    QList<TrackPoint>           &crossList,
    ArucoOptions                &options,
    const IntrinsicCameraParams &intrinsicCameraParams)
{
    constexpr int border = 4; // zusaetzlicher rand um subrects

    Control *controlWidget         = options.controlWidget;
    bool     ignoreWithoutMarker   = options.ignoreWithoutMarker;
    bool     autoCorrect           = options.autoCorrect;
    bool     autoCorrectOnlyExport = options.autoCorrectOnlyExport;

    CodeMarkerOptions &codeOpt = options.codeOpt;
    for(ColorBlob &blob : blobs)
    {
        // cropRect has coordinates of rechtangele around color blob with respect to lower left corner (as in the
        // beginning of useBlackDot)
        const cv::RotatedRect &box      = blob.box;
        cv::Rect               cropRect = box.boundingRect();


        int extendRect = myRound(
            blob.maxExpansion *
            2); // scalar to increase area of cropRect for better detection of codemarkers when marker appears to stick
                // out of the colored head because of tilted heads; value of .3 chosen arbitrarily after discussion
        const int sideLength = 2 * border + ((myRound(blob.maxExpansion) + extendRect) & -2);

        const auto borderVec     = cv::Point(border, border);
        const auto topLeftCorner = cropRect - borderVec;
        cropRect.x               = std::max(1, topLeftCorner.x - sideLength / 2);
        cropRect.y               = std::max(1, topLeftCorner.y - sideLength / 2);

        // (x&-2) == std::floor(x - x%2) | Ensures it's divisible by 2
        const int maxWidth  = img.cols - cropRect.x - 1;
        cropRect.width      = std::min(maxWidth, sideLength);
        const int maxHeight = img.rows - cropRect.y - 1;
        cropRect.height     = std::min(maxHeight, sideLength);

        cv::Mat subImg = img(cropRect); // --> shallow copy (points to original data)

        Vec2F offsetCropRect2Roi; // needed for drawing detected ArucoCode-Candidates correctly -> passed on to
                                  // findCodeMarker()-Function
        offsetCropRect2Roi.setX(cropRect.x);
        offsetCropRect2Roi.setY(cropRect.y);

        if(subImg.empty())
        {
            continue;
        }
        // TODO: Use Reference to actual codeMarkerOptions in MulticolorMarkerOptions
        // NOTE: For now, add as parameter of findMulticolorMarker
        codeOpt.setOffsetCropRect2Roi(offsetCropRect2Roi);

        QList<TrackPoint> addedCodes = findCodeMarker(subImg, options.method, codeOpt, intrinsicCameraParams, true);
        codeOpt.setOffsetCropRect2Roi({0, 0});

        // remove all detected codes in the image, that are not inside the bounding box of the color blob
        addedCodes = filterCodesByBoundingRect(addedCodes, blob.box.boundingRect(), offsetCropRect2Roi);

        // used for autocorrection (if enabled)
        Vec2F moveDir(0, 0);
        if(autoCorrect && !autoCorrectOnlyExport)
        {
            moveDir = autoCorrectColorMarker(blob.imageCenter, controlWidget);
        }

        // there are detected markers or candidates
        if(!addedCodes.empty())
        {
            // the aruco detection first adds detected codes followed by candidates
            bool isDetected = addedCodes.first().getMarkerID() >= 0;

            if(isDetected)
            {
                // codes are already filtered to be inside bounding rect, so just take the first
                auto code = addedCodes.at(0);

                code.setCol(blob.color);
                code = code + Vec2F(cropRect.x, cropRect.y) + moveDir;
                crossList.append(code);
                // if a code was fully detected we are done
                continue;
            }

            // reference center point of blob
            Vec2F      referencePosition(box.center.x, box.center.y);
            TrackPoint trackPoint(referencePosition + moveDir, 90, Vec2F(box.center.x, box.center.y), blob.color);

            auto resolvedCode = resolveMoreThanOneCandidateCode(addedCodes, referencePosition - offsetCropRect2Roi);

            resolvedCode.setQual(TrackPoint::bestDetectionQual);
            resolvedCode.setCol(blob.color);
            resolvedCode = resolvedCode + Vec2F(cropRect.x, cropRect.y) + moveDir;
            crossList.append(resolvedCode);
            continue;
        }

        // case not even candidates were detected
        if(addedCodes.empty() && !ignoreWithoutMarker)
        {
            // set to zero as coordinates are directly used from cropRect
            crossList.append(TrackPoint(
                Vec2F(box.center.x, box.center.y) + moveDir, 90, Vec2F(box.center.x, box.center.y), blob.color));
            continue;
        }

    } // end for
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
 * @param intrinsicCameraParams needed for aruco-refinement
 */
void findMultiColorMarker(
    cv::Mat                     &img,
    QList<TrackPoint>           &crossList,
    Control                     *controlWidget,
    bool                         ignoreWithoutMarker,
    Vec2F                       &offset,
    RecognitionMethod            method,
    CodeMarkerOptions           &codeOpt,
    const IntrinsicCameraParams &intrinsicCameraParams)
{
    Petrack                *mainWindow = controlWidget->getMainWindow();
    MultiColorMarkerItem   *cmItem     = mainWindow->getMultiColorMarkerItem();
    MultiColorMarkerWidget *cmWidget   = mainWindow->getMultiColorMarkerWidget();
    int                     bS         = mainWindow->getImageBorderSize();

    RectPlotItem *rectPlotItem = controlWidget->getColorPlot()->getMapItem();
    bool          useClose     = cmWidget->useClose->isChecked();
    bool          useOpen      = cmWidget->useOpen->isChecked();
    int           radiusClose  = cmWidget->closeRadius->value();
    int           radiusOpen   = cmWidget->openRadius->value();
    double        dotSize      = cmWidget->dotSize->value();
    int           minArea = cmWidget->minArea->value(), maxArea = cmWidget->maxArea->value();
    double        maxRatio         = cmWidget->maxRatio->value();
    bool          useBlackDot      = cmWidget->useDot->isChecked();
    bool          useCodeMarker    = cmWidget->useCodeMarker->isChecked();
    bool          restrictPosition = cmWidget->restrictPosition->isChecked();
    ignoreWithoutMarker =
        cmWidget->ignoreWithoutDot->isChecked(); // ueberschreiben von uebergeordnetem ignoreWithoutMarker
    bool autoCorrect           = cmWidget->autoCorrect->isChecked();
    bool autoCorrectOnlyExport = cmWidget->autoCorrectOnlyExport->isChecked();

    for(int j = 0; j < rectPlotItem->mapNum(); j++)
    {
        int nr;
        if(j == controlWidget->getMapNr())
        {
            nr = rectPlotItem->mapNum() - 1;
        }
        else if(j == rectPlotItem->mapNum() - 1)
        {
            nr = controlWidget->getMapNr();
        }
        else
        {
            nr = j;
        }

        ColorBlobDetectionParams param;
        param.fromColor   = rectPlotItem->getMap(nr).fromColor();
        param.toColor     = rectPlotItem->getMap(nr).toColor();
        param.invHue      = rectPlotItem->getMap(nr).invHue();
        param.minArea     = minArea;
        param.maxArea     = maxArea;
        param.maxRatio    = maxRatio;
        param.useClose    = useClose;
        param.radiusClose = radiusClose;
        param.useOpen     = useOpen;
        param.radiusOpen  = radiusOpen;
        param.offset      = offset;
        param.img         = img;
        param.binary      = cmItem->createMask(img.cols, img.rows);

        auto blobs = findColorBlob(param);


        if(useBlackDot)
        {
            ColorParameters colParam;
            setColorParameter(
                rectPlotItem->getMap(nr).fromColor(),
                rectPlotItem->getMap(nr).toColor(),
                rectPlotItem->getMap(nr).invHue(),
                colParam);
            // zentralen farbton heraussuchen
            QColor midHue;
            if(colParam.inversHue)
            {
                midHue.setHsv(2 * ((colParam.h_low + (colParam.h_high - colParam.h_low) / 2 + 90) % 180), 255, 255);
            }
            else
            {
                midHue.setHsv(2 * (colParam.h_low + (colParam.h_high - colParam.h_low) / 2), 255, 255);
            }

            BlackDotOptions options;
            options.ignoreWithoutMarker   = ignoreWithoutMarker;
            options.autoCorrect           = autoCorrect;
            options.autoCorrectOnlyExport = autoCorrectOnlyExport;
            options.restrictPosition      = restrictPosition;
            options.borderSize            = bS;
            options.midHue                = midHue;
            options.dotSize               = dotSize;
            options.controlWidget         = controlWidget;
            options.worldImageCorr        = &mainWindow->getWorldImageCorrespondence();

            // adds to crosslist
            refineWithBlackDot(blobs, img, crossList, options);
        }
        else if(useCodeMarker)
        {
            ArucoOptions options{
                controlWidget, ignoreWithoutMarker, autoCorrect, autoCorrectOnlyExport, method, codeOpt};

            // adds to crosslist
            refineWithAruco(blobs, img, crossList, options, intrinsicCameraParams);
        }
        else
        {
            Vec2F moveDir;
            for(ColorBlob &blob : blobs)
            {
                if(autoCorrect && !autoCorrectOnlyExport)
                {
                    moveDir = autoCorrectColorMarker(blob.imageCenter, controlWidget);

                    crossList.append(TrackPoint(
                        Vec2F(blob.box.center.x, blob.box.center.y) + moveDir,
                        100,
                        Vec2F(blob.box.center.x, blob.box.center.y),
                        blob.color)); // 100 beste qualitaet
                }
                else
                {
                    crossList.append(TrackPoint(
                        Vec2F(blob.box.center.x, blob.box.center.y),
                        100,
                        Vec2F(blob.box.center.x, blob.box.center.y),
                        blob.color)); // 100 beste qualitaet
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
void findColorMarker(cv::Mat &img, QList<TrackPoint> &crossList, Control *controlWidget)
{
    ColorParameters    param;
    ColorMarkerItem   *cmItem   = controlWidget->getMainWindow()->getColorMarkerItem();
    ColorMarkerWidget *cmWidget = controlWidget->getMainWindow()->getColorMarkerWidget();

    QColor fromColor = cmWidget->fromColor->palette().color(QPalette::Button);
    QColor toColor   = cmWidget->toColor->palette().color(QPalette::Button);

    setColorParameter(fromColor, toColor, cmWidget->inversHue->isChecked(), param);

    // run detection
    cv::Mat binary;

    // erzeuge speicherplatz fuer mask
    // abfrage wird in createMask gemacht
    // if (cmItem->getMask() == NULL || (cmItem->getMask() != NULL && (cmItem->getMask()->width != img->width ||
    // cmItem->getMask()->height != img->height)))
    binary = cmItem->createMask(img.cols, img.rows); // erzeugt binary mask mit groesse von img

    // color thresholding
    thresholdHSV(img, binary, param);

    // close small holes: radius ( hole ) < radius ( close )
    if(cmWidget->useClose->isChecked())
    {
        int radius_close = cmWidget->closeRadius->value();
        // siehe : http://opencv.willowgarage.com/documentation/c/image_filtering.html#createstructuringelementex
        cv::morphologyEx(
            binary,
            binary,
            cv::MORPH_OPEN,
            getStructuringElement(
                cv::MORPH_ELLIPSE,
                cv::Size(2 * radius_close + 1, 2 * radius_close + 1),
                cv::Point(radius_close, radius_close)));
    }
    // remove small blobs: radius ( blob ) < radius ( open )
    if(cmWidget->useOpen->isChecked())
    {
        int radius_open = cmWidget->openRadius->value();
        cv::morphologyEx(
            binary,
            binary,
            cv::MORPH_CLOSE,
            getStructuringElement(
                cv::MORPH_ELLIPSE, cv::Size(2 * radius_open + 1, 2 * radius_open + 1), cv::Point(radius_open)));
    }
    std::vector<std::vector<cv::Point>> contours;
    double                              area;
    QColor                              col;
    cv::RotatedRect                     box;
    double                              ratio;
    bool                                atEdge;

    cv::Mat clone = binary.clone(); //(cvarrToMat(binary),true);

    cv::findContours(clone, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    // test each contour
    while(!contours.empty())
    {
        std::vector<cv::Point> contour = contours.back();
        area                           = cv::contourArea(contour);
        box                            = cv::minAreaRect(contour);

        if(box.size.height > box.size.width)
        {
            ratio = box.size.height / box.size.width;
        }
        else
        {
            ratio = box.size.width / box.size.height;
        }

        // contour at border of roi, than neglect because of object going out of region has moving center
        atEdge = false;
        for(size_t i = 0; i < contour.size(); i++)
        {
            if((contour.at(i).x <= 1) || (contour.at(i).x >= img.cols - 2) || (contour.at(i).y <= 1) ||
               (contour.at(i).y >= img.rows - 2))
            {
                atEdge = true;
            }
        }

        if(!atEdge && area >= cmWidget->minArea->value() && area <= cmWidget->maxArea->value() &&
           ratio <= cmWidget->maxRatio->value())
        {
            // eine mittelung waere ggf sinnvoll, aber am rand aufpassen
            col.setRgb(getValue(img, myRound(box.center.x), myRound(box.center.y)).rgb());
            crossList.append(TrackPoint(
                Vec2F(box.center.x, box.center.y), 100, Vec2F(box.center.x, box.center.y), col)); // 100 beste qualitaet
        }

        // take the next contour
        contours.pop_back();
    }
}


/**
 * @brief Estimates the pose of detected ArUco markers using the default solvePnP method.
 *
 * This function mimics the old `cv::aruco::estimatePoseSingleMarkers` method.
 * It calculates the rotation and translation vectors (pose) of each detected marker
 * given their 2D corner points in the image and the known 3D geometry of the markers.
 *
 * The method uses the `cv::solvePnP` function to estimate the pose of each marker.
 *
 * @param corners           A vector of vectors, where each inner vector contains
 *                          the 2D corner points of a detected marker in image coordinates.
 * @param markerLength      The side length of the markers in the same unit as the
 *                          camera calibration (e.g., meters or centimeters).
 * @param cameraMatrix      The camera intrinsic matrix (3x3 matrix) obtained from
 *                          camera calibration.
 * @param distCoeff         The camera distortion coefficients vector (1x5, 1x8, etc.).
 * @param rotationVectors   Output vector of 3D rotation vectors (one per marker),
 *                          where each rotation vector is in Rodrigues form (3x1).
 * @param translationVectors Output vector of 3D translation vectors (one per marker),
 *                           which represent the translation from the camera to the marker.
 *
 * @note This function assumes that the markers are square and lie flat on the XY plane,
 *       with the Z-axis pointing out of the plane.
 *
 * @see cv::solvePnP
 */
void detail::estimatePoseSingleMarkers(
    const std::vector<std::vector<cv::Point2f>> &corners,
    float                                        markerLength,
    const cv::Mat                               &cameraMatrix,
    const cv::Mat                               &distCoeff,
    std::vector<cv::Vec3d>                      &rotationVectors,
    std::vector<cv::Vec3d>                      &translationVectors)
{
    // Define the 3D points of a single square marker
    std::vector<cv::Point3f> markerPoints = {
        {-markerLength / 2.0f, markerLength / 2.0f, 0}, // Top-left
        {markerLength / 2.0f, markerLength / 2.0f, 0},  // Top-right
        {markerLength / 2.0f, -markerLength / 2.0f, 0}, // Bottom-right
        {-markerLength / 2.0f, -markerLength / 2.0f, 0} // Bottom-left
    };

    // Clear the output vectors
    rotationVectors.clear();
    translationVectors.clear();

    // Iterate over each detected marker
    for(size_t i = 0; i < corners.size(); ++i)
    {
        cv::Vec3d rvec, tvec; // Variables to store rotation and translation vectors

        bool success = cv::solvePnP(markerPoints, corners[i], cameraMatrix, distCoeff, rvec, tvec, false);

        if(success)
        {
            // Append the rotation and translation vectors
            rotationVectors.push_back(rvec);
            translationVectors.push_back(tvec);
        }
    }
}

/**
 * @brief uses OpenCV libraries to detect Aruco CodeMarkers
 * @param img image to find codes in
 * @param opt arucomarker parameters used for detection
 * @param intrinsicCameraParams used for estimating arucomarker orientation
 * @param appendRejectedCodes append trackpoints of rejected codes to the list of detected codes.
 *          OpenCV returns rejected code candidates. These are often the correct codes, but the information is
 *          unreadable. If this flag is set to true, these rejected candidates get added as trackpoint
 *          like usual detected codes do, but without markerID.
 *          These points will only be appended if no code was detected.
 * @return list of all detected codes in given image
 */
QList<TrackPoint> detail::findCodeMarker(
    cv::Mat                     &img,
    RecognitionMethod            recoMethod,
    const CodeMarkerOptions     &opt,
    const IntrinsicCameraParams &intrinsicCameraParams,
    bool                         appendRejectedCodes)
{
    CodeMarkerItem *codeMarkerItem = opt.getCodeMarkerItem();
    Control        *controlWidget  = opt.getControlWidget();
    const auto     &parameters     = opt.getDetectorParams();

    auto dictionary =
        (opt.getIndexOfMarkerDict() != 17) ?
            cv::aruco::getPredefinedDictionary(cv::aruco::PredefinedDictionaryType(opt.getIndexOfMarkerDict())) :
            detail::getDictMip36h12(); // for usage of DICT_mip_36h12 as it is not predifined in opencv

    double minMarkerPerimeterRate = std::numeric_limits<double>::quiet_NaN();
    double maxMarkerPerimeterRate = std::numeric_limits<double>::quiet_NaN();
    double minCornerDistanceRate  = parameters.getMinCornerDistance();
    double minMarkerDistanceRate  = parameters.getMinMarkerDistance();

    Petrack *mainWindow = controlWidget->getMainWindow();

    int borderSize = mainWindow->getImageBorderSize();

    // 3D - Case
    if(controlWidget->getCalibCoordDimension() == 0)
    {
        QRect rect(
            myRound(mainWindow->getRecoRoiItem()->rect().x()),
            myRound(mainWindow->getRecoRoiItem()->rect().y()),
            myRound(mainWindow->getRecoRoiItem()->rect().width()),
            myRound(mainWindow->getRecoRoiItem()->rect().height()));
        QPointF p1 = mainWindow->getWorldImageCorrespondence().getCmPerPixel(
            rect.x(), rect.y(), controlWidget->getDefaultHeight());
        QPointF p2 = mainWindow->getWorldImageCorrespondence().getCmPerPixel(
            rect.x() + rect.width(), rect.y(), controlWidget->getDefaultHeight());
        QPointF p3 = mainWindow->getWorldImageCorrespondence().getCmPerPixel(
            rect.x(), rect.y() + rect.height(), controlWidget->getDefaultHeight());
        QPointF p4 = mainWindow->getWorldImageCorrespondence().getCmPerPixel(
            rect.x() + rect.width(), rect.y() + rect.height(), controlWidget->getDefaultHeight());

        double cmPerPixelMin = std::min({p1.x(), p1.y(), p2.x(), p2.y(), p3.x(), p3.y(), p4.x(), p4.y()});
        double cmPerPixelMax = std::max({p1.x(), p1.y(), p2.x(), p2.y(), p3.x(), p3.y(), p4.x(), p4.y()});

        if(recoMethod ==
           RecognitionMethod::Code) // for usage of codemarker with CodeMarker-function (-> without MulticolorMarker)
        {
            minMarkerPerimeterRate =
                (parameters.getMinMarkerPerimeter() * 4. / cmPerPixelMax) / std::max(rect.width(), rect.height());
            maxMarkerPerimeterRate =
                (parameters.getMaxMarkerPerimeter() * 4. / cmPerPixelMin) / std::max(rect.width(), rect.height());
        }
        else if(recoMethod == RecognitionMethod::MultiColor) // for usage of codemarker with MulticolorMarker
        {
            minMarkerPerimeterRate =
                (parameters.getMinMarkerPerimeter() * 4. / cmPerPixelMax) / std::max(img.cols, img.rows);
            maxMarkerPerimeterRate =
                (parameters.getMaxMarkerPerimeter() * 4. / cmPerPixelMin) / std::max(img.cols, img.rows);
        }
    }
    else // 2D
    {
        double cmPerPixel = mainWindow->getWorldImageCorrespondence().getCmPerPixel();
        minMarkerPerimeterRate =
            (parameters.getMinMarkerPerimeter() * 4 / cmPerPixel) /
            std::max(mainWindow->getImage()->width() - borderSize, mainWindow->getImage()->height() - borderSize);
        maxMarkerPerimeterRate =
            (parameters.getMaxMarkerPerimeter() * 4 / cmPerPixel) /
            std::max(mainWindow->getImage()->width() - borderSize, mainWindow->getImage()->height() - borderSize);
    }

    cv::aruco::DetectorParameters detectorParams;

    detectorParams.adaptiveThreshWinSizeMin    = parameters.getAdaptiveThreshWinSizeMin();
    detectorParams.adaptiveThreshWinSizeMax    = parameters.getAdaptiveThreshWinSizeMax();
    detectorParams.adaptiveThreshWinSizeStep   = parameters.getAdaptiveThreshWinSizeStep();
    detectorParams.adaptiveThreshConstant      = parameters.getAdaptiveThreshConstant();
    detectorParams.minMarkerPerimeterRate      = minMarkerPerimeterRate;
    detectorParams.maxMarkerPerimeterRate      = maxMarkerPerimeterRate;
    detectorParams.polygonalApproxAccuracyRate = parameters.getPolygonalApproxAccuracyRate();
    detectorParams.minCornerDistanceRate       = minCornerDistanceRate;
    detectorParams.minDistanceToBorder         = parameters.getMinDistanceToBorder();
    detectorParams.minMarkerDistanceRate       = minMarkerDistanceRate;
    // No refinement is default value
    // TODO Check if this is the best method for our usecase
    if(parameters.getDoCornerRefinement())
    {
        detectorParams.cornerRefinementMethod = cv::aruco::CornerRefineMethod::CORNER_REFINE_SUBPIX;
    }
    detectorParams.cornerRefinementWinSize               = parameters.getCornerRefinementWinSize();
    detectorParams.cornerRefinementMaxIterations         = parameters.getCornerRefinementMaxIterations();
    detectorParams.cornerRefinementMinAccuracy           = parameters.getCornerRefinementMinAccuracy();
    detectorParams.markerBorderBits                      = parameters.getMarkerBorderBits();
    detectorParams.perspectiveRemovePixelPerCell         = parameters.getPerspectiveRemovePixelPerCell();
    detectorParams.perspectiveRemoveIgnoredMarginPerCell = parameters.getPerspectiveRemoveIgnoredMarginPerCell();
    detectorParams.maxErroneousBitsInBorderRate          = parameters.getMaxErroneousBitsInBorderRate();
    detectorParams.minOtsuStdDev                         = parameters.getMinOtsuStdDev();
    detectorParams.errorCorrectionRate                   = parameters.getErrorCorrectionRate();

    std::vector<int>                      ids;
    std::vector<std::vector<cv::Point2f>> corners;
    std::vector<std::vector<cv::Point2f>> rejected;

    const cv::aruco::ArucoDetector arucoDetector(dictionary, detectorParams);
    arucoDetector.detectMarkers(img, corners, ids, rejected);

    codeMarkerItem->addDetectedMarkers(corners, ids, opt.getOffsetCropRect2Roi());
    codeMarkerItem->addRejectedMarkers(rejected, opt.getOffsetCropRect2Roi());

    if(appendRejectedCodes && !rejected.empty())
    {
        corners.insert(corners.end(), rejected.begin(), rejected.end());
    }

    if(corners.empty())
    {
        // if no markers are found return to prevent opencv errors because of empty corners and thus empty
        // rotationVectors vectors
        return {};
    }

    // value only relevant for axis length when drawing axes
    float                  markerLength = (float) opt.getDetectorParams().getMinCornerDistance();
    std::vector<cv::Vec3d> rotationVectors;
    std::vector<cv::Vec3d> translationVectors;
    const cv::Mat         &cameraMatrix = intrinsicCameraParams.cameraMatrix;
    const cv::Mat         &distCoeff    = cv::Mat::zeros(cv::Size(1, 5), CV_32F);
    detail::estimatePoseSingleMarkers(
        corners, markerLength, cameraMatrix, distCoeff, rotationVectors, translationVectors);

    // store all detected codes as TrackPoints in a list to return
    QList<TrackPoint> trackPoints;


    // detected code markers
    for(size_t i = 0; i < corners.size(); i++)
    {
        const auto &codeCorners = corners.at(i);

        double codeX = (codeCorners.at(0).x + codeCorners.at(1).x + codeCorners.at(2).x + codeCorners.at(3).x) / 4.;
        double codeY = (codeCorners.at(0).y + codeCorners.at(1).y + codeCorners.at(2).y + codeCorners.at(3).y) / 4.;

        cv::Matx<double, 3, 3> rotMat;
        cv::Rodrigues(rotationVectors[i], rotMat);

        cv::Vec3d orientation = cv::normalize(rotMat * cv::Vec3d(0, 1, 0));


        // use best quality for code markers, even for candidates
        TrackPoint trackPoint(Vec2F(codeX, codeY), TrackPoint::bestDetectionQual);

        // overwrite qual and ID if not candidate
        if(i < ids.size())
        {
            // code marker should have the best possible quality i.e. 100
            trackPoint.setMarkerID(ids.at(i));
        }
        trackPoint.setOrientation(orientation);

        trackPoints.append(trackPoint);
    }
    return trackPoints;
}

void findContourMarker(
    cv::Mat           &img,
    QList<TrackPoint> *crossList,
    int                markerBrightness,
    bool               ignoreWithoutMarker,
    bool               autoWB,
    RecognitionMethod  recoMethod,
    float              headSize)
{
    int                                 threshold, plus;
    size_t                              count;
    double                              angle;
    MarkerHermesList                    markerHermesList;
    MarkerCasernList                    markerCasernList;
    MarkerJapanList                     markerJapanList(headSize);
    cv::Size                            sz   = cv::Size(img.cols & -2, img.rows & -2);
    cv::Mat                             gray = cv::Mat(sz, CV_8UC1);
    cv::Mat                             tgray;
    cv::Mat                             grayFix;
    std::vector<std::vector<cv::Point>> contours;

    cv::RotatedRect box;
    int             expansion;
    double          contourArea;

    if(img.channels() == 3)
    {
        tgray = cv::Mat(sz, CV_8UC1); // cvCreateImage(sz, 8, 1);
        cv::cvtColor(img, tgray, cv::COLOR_RGB2GRAY);
    }
    // try several threshold levels
    plus = (250 - 72) / 10;
    // andere richtung der schwellwertanpassung koennte andere ergebnisse leifern
    // cw->markerBrightness->value()==markerBrightness hat default 50
    for(threshold = 60 + markerBrightness; threshold < 251; threshold += plus) // 70..255, 20 //155+cw->temp2->value()
    {
        if(img.channels() == 3)
        {
            cv::threshold(tgray, gray, threshold, 255, cv::THRESH_BINARY);
        }
        else if(img.channels() == 1)
        {
            cv::threshold(
                img,
                gray,
                threshold,
                255,
                cv::THRESH_BINARY); // cvThreshold(img, gray, threshold, 255, CV_THRESH_BINARY);
        }
        else
        {
            SPDLOG_ERROR("wrong number of channels: {}", img.channels());
        }
        grayFix = gray.clone();


        // find contours and store them all as a list
        findContours(gray, contours, cv::RETR_LIST, cv::CHAIN_APPROX_SIMPLE);

        // test each contour
        while(!contours.empty())
        {
            std::vector<cv::Point> contour = contours.back();
            // koennten auch interessant sein:
            // MinAreaRect2
            // MinEnclosingCircle
            //  um kreise zu suchen koennte auch cvHoughCircles genutzt werden

            count = contour.size();

            // man koennte das Seitenverhaeltnis, contour-gesamtlaenge vorher ueberpruefen, um cont rauszuwerfen

            if(count > 5)
            {
                // Fits ellipse to current contour.
                cv::Mat pointsf;
                cv::Mat(contour).convertTo(pointsf, CV_32F);
                box = fitEllipse(pointsf);

                expansion =
                    box.size.width > box.size.height ? myRound(box.size.width * 0.5) : myRound(box.size.height * 0.5);

                if(box.center.x - expansion > ELLIPSE_DISTANCE_TO_BORDER &&
                   box.center.x + expansion < gray.cols - ELLIPSE_DISTANCE_TO_BORDER &&
                   box.center.y - expansion > ELLIPSE_DISTANCE_TO_BORDER &&
                   box.center.y + expansion < gray.rows - ELLIPSE_DISTANCE_TO_BORDER)
                {
                    angle = (box.angle) / 180. * PI;
                    if(box.size.width < box.size.height)
                    {
                        angle -= PI / 2;
                    }

                    contourArea = cv::contourArea(contour, true);

                    // contourArea koennte mit MyEllipse.area() verglichen werden und bei grossen abweichungen verworfen
                    // werden!!!
                    MyEllipse e(box.center.x, box.center.y, box.size.width * 0.5, box.size.height * 0.5, angle);
                    if(recoMethod == RecognitionMethod::Casern)
                    {
                        markerCasernList.mayAddEllipse(grayFix, e, (contourArea > 0));
                    }
                    else if(recoMethod == RecognitionMethod::Hermes)
                    {
                        markerHermesList.mayAddEllipse(grayFix, e, (contourArea > 0));
                    }
                    else if(recoMethod == RecognitionMethod::Japan)
                    {
                        markerJapanList.mayAddEllipse(grayFix, e, (contourArea > 0));
                    }
                }
            }
            contours.pop_back();
        }
    }
    if(recoMethod == RecognitionMethod::Casern) // Casern
    {
        markerCasernList.organize(img, autoWB);
        markerCasernList.toCrossList(crossList, ignoreWithoutMarker);
    }
    else if(recoMethod == RecognitionMethod::Hermes) // Hermes
    {
        markerHermesList.organize(img, autoWB);
        markerHermesList.toCrossList(crossList, ignoreWithoutMarker);
    }
    else if(recoMethod == RecognitionMethod::Japan) // Japan
    {
        markerJapanList.organize(img, autoWB);
        markerJapanList.toCrossList(crossList, ignoreWithoutMarker);
    }
}


/**
 * @brief Detects position of markers from user-chosen type
 *
 * @param img
 * @param roi Region of interest for recognition
 * @param controlWidget
 * @param borderSize
 * @param bgFilter
 * @param intrinsicCameraParams intrinsic parameters of the camera. Used for e.g. estimation of arucomarkers
 *
 * @return List of detected TrackPoints
 */
QList<TrackPoint> Recognizer::getMarkerPos(
    cv::Mat                     &img,
    QRect                       &roi,
    Control                     *controlWidget,
    int                          borderSize,
    BackgroundFilter            *bgFilter,
    const IntrinsicCameraParams &intrinsicCameraParams)
{
    int  markerBrightness    = controlWidget->getMarkerBrightness();
    bool ignoreWithoutMarker = controlWidget->isMarkerIgnoreWithoutChecked();
    bool autoWB              = controlWidget->isRecoAutoWBChecked();

    auto rect = qRectToCvRect(
        roi,
        img,
        (mRecoMethod != RecognitionMethod::Color) && (mRecoMethod != RecognitionMethod::MultiColor) &&
            (mRecoMethod != RecognitionMethod::Code));

    auto tImg = img(rect);

    if(tImg.empty())
    {
        return QList<TrackPoint>{};
    }

    QList<TrackPoint> crossList;
    // offset of rect
    Vec2F v(rect.x - borderSize, rect.y - borderSize);

    switch(mRecoMethod)
    {
        case RecognitionMethod::MultiColor:
            findMultiColorMarker(
                tImg,
                crossList,
                controlWidget,
                ignoreWithoutMarker,
                v,
                mRecoMethod,
                mCodeMarkerOptions,
                intrinsicCameraParams);
            break;
        case RecognitionMethod::Color:
            findColorMarker(tImg, crossList, controlWidget);
            break;
        case RecognitionMethod::Code:
            crossList = findCodeMarker(tImg, mRecoMethod, mCodeMarkerOptions, intrinsicCameraParams);
            break;
        case RecognitionMethod::Casern:
            [[fallthrough]];
        case RecognitionMethod::Hermes:
            [[fallthrough]];
        case RecognitionMethod::Japan:
            findContourMarker(
                tImg,
                &crossList,
                markerBrightness,
                ignoreWithoutMarker,
                autoWB,
                mRecoMethod,
                controlWidget->getMainWindow()->getHeadSize());
            break;
        case RecognitionMethod::Stereo:
            throw std::invalid_argument(
                "Stereo marker are not handled in getMarkerPos, but in PersonList::calcPersonPos");
    }

    // must be set because else hovermoveevent of recognitionRec moves also the colorMaskItem
    controlWidget->getMainWindow()->getColorMarkerItem()->setRect(v);
    // must be set because else hovermoveevent of recognitionRec moves also the colorMaskItem
    controlWidget->getMainWindow()->getMultiColorMarkerItem()->setRect(v);
    // must be set because else hovermoveevent of recognitionRec moves also the colorMaskItem
    controlWidget->getMainWindow()->getCodeMarkerItem()->setRect(v);

    // set cross position relative to original image size
    for(auto &point : crossList)
    {
        point += v;
        point.setColPoint(point.colPoint() + v);
    }

    if(bgFilter->getEnabled()) // nur fuer den fall von bgSubtraction durchfuehren
    {
        crossList.erase(
            std::remove_if(
                crossList.begin(),
                crossList.end(),
                [bgFilter](TrackPoint &tp) { return !bgFilter->isForeground(tp.x(), tp.y()); }),
            crossList.end());
    }

    return crossList;
}

const Vec2F &CodeMarkerOptions::getOffsetCropRect2Roi() const
{
    return offsetCropRect2Roi;
}

void CodeMarkerOptions::setOffsetCropRect2Roi(const Vec2F &newOffsetCropRect2Roi)
{
    offsetCropRect2Roi = newOffsetCropRect2Roi;
}

void CodeMarkerOptions::setDetectorParams(ArucoCodeParams params)
{
    if(params != detectorParams)
    {
        detectorParams = params;
        emit detectorParamsChanged();
    }
}

void CodeMarkerOptions::setIndexOfMarkerDict(int idx)
{
    if(idx != indexOfMarkerDict)
    {
        indexOfMarkerDict = idx;
        emit indexOfMarkerDictChanged();
    }
}

/**
 * @brief getMip36h12Dict() overrides current dictionary with dictionary from 'aruco_mip_36h12_dict'
 *
 *
 * bitListDictMip36h12 contains 250 aruco code markers (size 6x6, 1 bit black border)
 * markers consist of 0s and 1s, built from upper left to lower right corner in hexadecimal notation.
 * aruco markers availlable via https://sourceforge.net/projects/aruco/files/
 */
cv::aruco::Dictionary detail::getDictMip36h12()
{
    std::array<uint64_t, 250> bitListDictMip36h12{
        0xd2b63a09dUL, 0x6001134e5UL, 0x1206fbe72UL, 0xff8ad6cb4UL, 0x85da9bc49UL, 0xb461afe9cUL, 0x6db51fe13UL,
        0x5248c541fUL, 0x8f34503UL,   0x8ea462eceUL, 0xeac2be76dUL, 0x1af615c44UL, 0xb48a49f27UL, 0x2e4e1283bUL,
        0x78b1f2fa8UL, 0x27d34f57eUL, 0x89222fff1UL, 0x4c1669406UL, 0xbf49b3511UL, 0xdc191cd5dUL, 0x11d7c3f85UL,
        0x16a130e35UL, 0xe29f27effUL, 0x428d8ae0cUL, 0x90d548477UL, 0x2319cbc93UL, 0xc3b0c3dfcUL, 0x424bccc9UL,
        0x2a081d630UL, 0x762743d96UL, 0xd0645bf19UL, 0xf38d7fd60UL, 0xc6cbf9a10UL, 0x3c1be7c65UL, 0x276f75e63UL,
        0x4490a3f63UL, 0xda60acd52UL, 0x3cc68df59UL, 0xab46f9daeUL, 0x88d533d78UL, 0xb6d62ec21UL, 0xb3c02b646UL,
        0x22e56d408UL, 0xac5f5770aUL, 0xaaa993f66UL, 0x4caa07c8dUL, 0x5c9b4f7b0UL, 0xaa9ef0e05UL, 0x705c5750UL,
        0xac81f545eUL, 0x735b91e74UL, 0x8cc35cee4UL, 0xe44694d04UL, 0xb5e121de0UL, 0x261017d0fUL, 0xf1d439eb5UL,
        0xa1a33ac96UL, 0x174c62c02UL, 0x1ee27f716UL, 0x8b1c5ece9UL, 0x6a05b0c6aUL, 0xd0568dfcUL,  0x192d25e5fUL,
        0x1adbeccc8UL, 0xcfec87f00UL, 0xd0b9dde7aUL, 0x88dcef81eUL, 0x445681cb9UL, 0xdbb2ffc83UL, 0xa48d96df1UL,
        0xb72cc2e7dUL, 0xc295b53fUL,  0xf49832704UL, 0x9968edc29UL, 0x9e4e1af85UL, 0x8683e2d1bUL, 0x810b45c04UL,
        0x6ac44bfe2UL, 0x645346615UL, 0x3990bd598UL, 0x1c9ed0f6aUL, 0xc26729d65UL, 0x83993f795UL, 0x3ac05ac5dUL,
        0x357adff3bUL, 0xd5c05565UL,  0x2f547ef44UL, 0x86c115041UL, 0x640fd9e5fUL, 0xce08bbcf7UL, 0x109bb343eUL,
        0xc21435c92UL, 0x35b4dfce4UL, 0x459752cf2UL, 0xec915b82cUL, 0x51881eed0UL, 0x2dda7dc97UL, 0x2e0142144UL,
        0x42e890f99UL, 0x9a8856527UL, 0x8e80d9d80UL, 0x891cbcf34UL, 0x25dd82410UL, 0x239551d34UL, 0x8fe8f0c70UL,
        0x94106a970UL, 0x82609b40cUL, 0xfc9caf36UL,  0x688181d11UL, 0x718613c08UL, 0xf1ab7629UL,  0xa357bfc18UL,
        0x4c03b7a46UL, 0x204dedce6UL, 0xad6300d37UL, 0x84cc4cd09UL, 0x42160e5c4UL, 0x87d2adfa8UL, 0x7850e7749UL,
        0x4e750fc7cUL, 0xbf2e5dfdaUL, 0xd88324da5UL, 0x234b52f80UL, 0x378204514UL, 0xabdf2ad53UL, 0x365e78ef9UL,
        0x49caa6ca2UL, 0x3c39ddf3UL,  0xc68c5385dUL, 0x5bfcbbf67UL, 0x623241e21UL, 0xabc90d5ccUL, 0x388c6fe85UL,
        0xda0e2d62dUL, 0x10855dfe9UL, 0x4d46efd6bUL, 0x76ea12d61UL, 0x9db377d3dUL, 0xeed0efa71UL, 0xe6ec3ae2fUL,
        0x441faee83UL, 0xba19c8ff5UL, 0x313035eabUL, 0x6ce8f7625UL, 0x880dab58dUL, 0x8d3409e0dUL, 0x2be92ee21UL,
        0xd60302c6cUL, 0x469ffc724UL, 0x87eebeed3UL, 0x42587ef7aUL, 0x7a8cc4e52UL, 0x76a437650UL, 0x999e41ef4UL,
        0x7d0969e42UL, 0xc02baf46bUL, 0x9259f3e47UL, 0x2116a1dc0UL, 0x9f2de4d84UL, 0xeffac29UL,   0x7b371ff8cUL,
        0x668339da9UL, 0xd010aee3fUL, 0x1cd00b4c0UL, 0x95070fc3bUL, 0xf84c9a770UL, 0x38f863d76UL, 0x3646ff045UL,
        0xce1b96412UL, 0x7a5d45da8UL, 0x14e00ef6cUL, 0x5e95abfd8UL, 0xb2e9cb729UL, 0x36c47dd7UL,  0xb8ee97c6bUL,
        0xe9e8f657UL,  0xd4ad2ef1aUL, 0x8811c7f32UL, 0x47bde7c31UL, 0x3adadfb64UL, 0x6e5b28574UL, 0x33e67cd91UL,
        0x2ab9fdd2dUL, 0x8afa67f2bUL, 0xe6a28fc5eUL, 0x72049cdbdUL, 0xae65dac12UL, 0x1251a4526UL, 0x1089ab841UL,
        0xe2f096ee0UL, 0xb0caee573UL, 0xfd6677e86UL, 0x444b3f518UL, 0xbe8b3a56aUL, 0x680a75cfcUL, 0xac02baea8UL,
        0x97d815e1cUL, 0x1d4386e08UL, 0x1a14f5b0eUL, 0xe658a8d81UL, 0xa3868efa7UL, 0x3668a9673UL, 0xe8fc53d85UL,
        0x2e2b7edd5UL, 0x8b2470f13UL, 0xf69795f32UL, 0x4589ffc8eUL, 0x2e2080c9cUL, 0x64265f7dUL,  0x3d714dd10UL,
        0x1692c6ef1UL, 0x3e67f2f49UL, 0x5041dad63UL, 0x1a1503415UL, 0x64c18c742UL, 0xa72eec35UL,  0x1f0f9dc60UL,
        0xa9559bc67UL, 0xf32911d0dUL, 0x21c0d4ffcUL, 0xe01cef5b0UL, 0x4e23a3520UL, 0xaa4f04e49UL, 0xe1c4fcc43UL,
        0x208e8f6e8UL, 0x8486774a5UL, 0x9e98c7558UL, 0x2c59fb7dcUL, 0x9446a4613UL, 0x8292dcc2eUL, 0x4d61631UL,
        0xd05527809UL, 0xa0163852dUL, 0x8f657f639UL, 0xcca6c3e37UL, 0xcb136bc7aUL, 0xfc5a83e53UL, 0x9aa44fc30UL,
        0xbdec1bd3cUL, 0xe020b9f7cUL, 0x4b8f35fb0UL, 0xb8165f637UL, 0x33dc88d69UL, 0x10a2f7e4dUL, 0xc8cb5ff53UL,
        0xde259ff6bUL, 0x46d070dd4UL, 0x32d3b9741UL, 0x7075f1c04UL, 0x4d58dbea0UL};

    int                   markerSize = 6;
    cv::aruco::Dictionary dictionary;
    dictionary.markerSize        = markerSize;
    dictionary.maxCorrectionBits = 3;

    // transform from hexadecimal notation to format in dictionary class
    for(auto code : bitListDictMip36h12)
    {
        std::bitset<36>       bits(code);
        std::array<uchar, 36> codeAsVector{};
        for(std::size_t i = 0; i < bits.size(); ++i)
        {
            codeAsVector[i] = bits[i];
        }
        cv::Mat markerBits(markerSize, markerSize, CV_8UC1, codeAsVector.data());
        cv::Mat markerCompressed = cv::aruco::Dictionary::getByteListFromBits(markerBits);
        dictionary.bytesList.push_back(markerCompressed);
    }

    return dictionary;
}

double ArucoCodeParams::getMaxMarkerPerimeter() const
{
    return maxMarkerPerimeter;
}

void ArucoCodeParams::setMaxMarkerPerimeter(double newMaxMarkerPerimeter)
{
    if(newMaxMarkerPerimeter < minMarkerPerimeter)
    {
        throw std::invalid_argument("Max perimeter length needs to be at least as big as min perimiter length");
    }
    maxMarkerPerimeter = newMaxMarkerPerimeter;
}

double ArucoCodeParams::getMinCornerDistance() const
{
    return minCornerDistance;
}

void ArucoCodeParams::setMinCornerDistance(double newMinCornerDistance)
{
    if(newMinCornerDistance < 0)
    {
        throw std::invalid_argument("Min corner distance cannot be negative");
    }
    minCornerDistance = newMinCornerDistance;
}

double ArucoCodeParams::getMinMarkerDistance() const
{
    return minMarkerDistance;
}

void ArucoCodeParams::setMinMarkerDistance(double newMinMarkerDistance)
{
    minMarkerDistance = newMinMarkerDistance;
}

int ArucoCodeParams::getAdaptiveThreshWinSizeMin() const
{
    return adaptiveThreshWinSizeMin;
}

void ArucoCodeParams::setAdaptiveThreshWinSizeMin(int newAdaptiveThreshWinSizeMin)
{
    if(newAdaptiveThreshWinSizeMin > adaptiveThreshWinSizeMax)
    {
        throw std::invalid_argument("Min adaptiveTreshWinSize needs to be at most as large as max");
    }
    if(newAdaptiveThreshWinSizeMin < 3)
    {
        throw std::invalid_argument("Min winsize must be at least 3");
    }
    adaptiveThreshWinSizeMin = newAdaptiveThreshWinSizeMin;
}

int ArucoCodeParams::getAdaptiveThreshWinSizeMax() const
{
    return adaptiveThreshWinSizeMax;
}

void ArucoCodeParams::setAdaptiveThreshWinSizeMax(int newAdaptiveThreshWinSizeMax)
{
    if(newAdaptiveThreshWinSizeMax < adaptiveThreshWinSizeMin)
    {
        throw std::invalid_argument("Max adaptiveThreshWinSize needs to be at least as large as the minimum");
    }
    if(newAdaptiveThreshWinSizeMax < 3)
    {
        throw std::invalid_argument("Max adaptive winsize needs to be at lest 3");
    }
    adaptiveThreshWinSizeMax = newAdaptiveThreshWinSizeMax;
}

int ArucoCodeParams::getAdaptiveThreshWinSizeStep() const
{
    return adaptiveThreshWinSizeStep;
}

void ArucoCodeParams::setAdaptiveThreshWinSizeStep(int newAdaptiveThreshWinSizeStep)
{
    if(newAdaptiveThreshWinSizeStep <= 0)
    {
        throw std::invalid_argument("Winsize step needs to be larger than 0");
    }
    adaptiveThreshWinSizeStep = newAdaptiveThreshWinSizeStep;
}

int ArucoCodeParams::getAdaptiveThreshConstant() const
{
    return adaptiveThreshConstant;
}

void ArucoCodeParams::setAdaptiveThreshConstant(int newAdaptiveThreshConstant)
{
    adaptiveThreshConstant = newAdaptiveThreshConstant;
}

double ArucoCodeParams::getPolygonalApproxAccuracyRate() const
{
    return polygonalApproxAccuracyRate;
}

void ArucoCodeParams::setPolygonalApproxAccuracyRate(double newPolygonalApproxAccuracyRate)
{
    polygonalApproxAccuracyRate = newPolygonalApproxAccuracyRate;
}

int ArucoCodeParams::getMinDistanceToBorder() const
{
    return minDistanceToBorder;
}

void ArucoCodeParams::setMinDistanceToBorder(int newMinDistanceToBorder)
{
    if(newMinDistanceToBorder < 0)
    {
        throw std::invalid_argument("Min distance to border cannot be negative");
    }
    minDistanceToBorder = newMinDistanceToBorder;
}

bool ArucoCodeParams::getDoCornerRefinement() const
{
    return doCornerRefinement;
}

void ArucoCodeParams::setDoCornerRefinement(bool newDoCornerRefinement)
{
    doCornerRefinement = newDoCornerRefinement;
}

int ArucoCodeParams::getCornerRefinementWinSize() const
{
    return cornerRefinementWinSize;
}

void ArucoCodeParams::setCornerRefinementWinSize(int newCornerRefinementWinSize)
{
    if(newCornerRefinementWinSize < 1)
    {
        throw std::invalid_argument("Winsize needs to be at least 1");
    }
    cornerRefinementWinSize = newCornerRefinementWinSize;
}

int ArucoCodeParams::getCornerRefinementMaxIterations() const
{
    return cornerRefinementMaxIterations;
}

void ArucoCodeParams::setCornerRefinementMaxIterations(int newCornerRefinementMaxIterations)
{
    if(newCornerRefinementMaxIterations < 1)
    {
        throw std::invalid_argument("Max iterations needs to be at least 1");
    }
    cornerRefinementMaxIterations = newCornerRefinementMaxIterations;
}

double ArucoCodeParams::getCornerRefinementMinAccuracy() const
{
    return cornerRefinementMinAccuracy;
}

void ArucoCodeParams::setCornerRefinementMinAccuracy(double newCornerRefinementMinAccuracy)
{
    if(newCornerRefinementMinAccuracy <= 0)
    {
        throw std::invalid_argument("Min accuracy needs to be larger than 0");
    }
    cornerRefinementMinAccuracy = newCornerRefinementMinAccuracy;
}

int ArucoCodeParams::getMarkerBorderBits() const
{
    return markerBorderBits;
}

void ArucoCodeParams::setMarkerBorderBits(int newMarkerBorderBits)
{
    if(newMarkerBorderBits < 1)
    {
        throw std::invalid_argument("Marker Borderbits needs to be at least 1");
    }
    markerBorderBits = newMarkerBorderBits;
}

int ArucoCodeParams::getPerspectiveRemovePixelPerCell() const
{
    return perspectiveRemovePixelPerCell;
}

void ArucoCodeParams::setPerspectiveRemovePixelPerCell(int newPerspectiveRemovePixelPerCell)
{
    perspectiveRemovePixelPerCell = newPerspectiveRemovePixelPerCell;
}

double ArucoCodeParams::getPerspectiveRemoveIgnoredMarginPerCell() const
{
    return perspectiveRemoveIgnoredMarginPerCell;
}

void ArucoCodeParams::setPerspectiveRemoveIgnoredMarginPerCell(double newPerspectiveRemoveIgnoredMarginPerCell)
{
    perspectiveRemoveIgnoredMarginPerCell = newPerspectiveRemoveIgnoredMarginPerCell;
}

double ArucoCodeParams::getMaxErroneousBitsInBorderRate() const
{
    return maxErroneousBitsInBorderRate;
}

void ArucoCodeParams::setMaxErroneousBitsInBorderRate(double newMaxErroneousBitsInBorderRate)
{
    maxErroneousBitsInBorderRate = newMaxErroneousBitsInBorderRate;
}

double ArucoCodeParams::getMinOtsuStdDev() const
{
    return minOtsuStdDev;
}

void ArucoCodeParams::setMinOtsuStdDev(double newMinOtsuStdDev)
{
    if(newMinOtsuStdDev <= 0)
    {
        throw std::invalid_argument("Min Otsu stddev needs to be larger than 0");
    }
    minOtsuStdDev = newMinOtsuStdDev;
}

double ArucoCodeParams::getErrorCorrectionRate() const
{
    return errorCorrectionRate;
}

void ArucoCodeParams::setErrorCorrectionRate(double newErrorCorrectionRate)
{
    errorCorrectionRate = newErrorCorrectionRate;
}

double ArucoCodeParams::getMinMarkerPerimeter() const
{
    return minMarkerPerimeter;
}

void ArucoCodeParams::setMinMarkerPerimeter(double newMinMarkerPerimeter)
{
    if(newMinMarkerPerimeter > maxMarkerPerimeter)
    {
        throw std::invalid_argument("Min marker perimeter needs to be at most as large as the max perimeter.");
    }
    if(newMinMarkerPerimeter <= 0)
    {
        throw std::invalid_argument("Min marker perimeter must be larger than 0");
    }
    minMarkerPerimeter = newMinMarkerPerimeter;
}

} // namespace reco
