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

#ifndef RECOGNITION_H
#define RECOGNITION_H

#include "vector.h"

#include <QList>
#include <QObject>
#include <opencv2/aruco.hpp>


class TrackPoint;
class QRect;
class BackgroundFilter;
class Control;
class ImageItem;
class CodeMarkerItem;
struct IntrinsicCameraParams;

namespace reco
{
/**
 * Different recognition methods used in PeTrack
 *
 * Note: be aware that changing the explicitly assigned integer values will break backwards compatibility!
 */
enum class RecognitionMethod
{
    Casern     = 0,
    Hermes     = 1,
    Stereo     = 2,
    Color      = 3,
    Japan      = 4,
    MultiColor = 5,
    Code       = 6,
};

class ArucoCodeParams
{
    double minMarkerPerimeter                    = 5;
    double maxMarkerPerimeter                    = 15;
    double minCornerDistance                     = 0.05;
    double minMarkerDistance                     = 0.05;
    int    adaptiveThreshWinSizeMin              = 3;
    int    adaptiveThreshWinSizeMax              = 27;
    int    adaptiveThreshWinSizeStep             = 10;
    int    adaptiveThreshConstant                = 7;
    double polygonalApproxAccuracyRate           = 0.03;
    int    minDistanceToBorder                   = 3;
    bool   doCornerRefinement                    = false;
    int    cornerRefinementWinSize               = 5;
    int    cornerRefinementMaxIterations         = 30;
    double cornerRefinementMinAccuracy           = 0.1;
    int    markerBorderBits                      = 1;
    int    perspectiveRemovePixelPerCell         = 4;
    double perspectiveRemoveIgnoredMarginPerCell = 0.13;
    double maxErroneousBitsInBorderRate          = 0.35;
    double minOtsuStdDev                         = 5;
    double errorCorrectionRate                   = 0.6;

public:
    friend inline constexpr bool operator==(const ArucoCodeParams &lhs, const ArucoCodeParams &rhs) noexcept
    {
        return (lhs.minMarkerPerimeter == rhs.minMarkerPerimeter) &&
               ((lhs.maxMarkerPerimeter == rhs.maxMarkerPerimeter) &&
                ((lhs.minCornerDistance == rhs.minCornerDistance) &&
                 ((lhs.minMarkerDistance == rhs.minMarkerDistance) &&
                  ((lhs.adaptiveThreshWinSizeMin == rhs.adaptiveThreshWinSizeMin) &&
                   ((lhs.adaptiveThreshWinSizeMax == rhs.adaptiveThreshWinSizeMax) &&
                    ((lhs.adaptiveThreshWinSizeStep == rhs.adaptiveThreshWinSizeStep) &&
                     ((lhs.adaptiveThreshConstant == rhs.adaptiveThreshConstant) &&
                      ((lhs.polygonalApproxAccuracyRate == rhs.polygonalApproxAccuracyRate) &&
                       ((lhs.minDistanceToBorder == rhs.minDistanceToBorder) &&
                        ((static_cast<int>(lhs.doCornerRefinement) == static_cast<int>(rhs.doCornerRefinement)) &&
                         ((lhs.cornerRefinementWinSize == rhs.cornerRefinementWinSize) &&
                          ((lhs.cornerRefinementMaxIterations == rhs.cornerRefinementMaxIterations) &&
                           ((lhs.cornerRefinementMinAccuracy == rhs.cornerRefinementMinAccuracy) &&
                            ((lhs.markerBorderBits == rhs.markerBorderBits) &&
                             ((lhs.perspectiveRemovePixelPerCell == rhs.perspectiveRemovePixelPerCell) &&
                              ((lhs.perspectiveRemoveIgnoredMarginPerCell ==
                                rhs.perspectiveRemoveIgnoredMarginPerCell) &&
                               ((lhs.maxErroneousBitsInBorderRate == rhs.maxErroneousBitsInBorderRate) &&
                                ((lhs.minOtsuStdDev == rhs.minOtsuStdDev) &&
                                 (lhs.errorCorrectionRate == rhs.errorCorrectionRate)))))))))))))))))));
    }
    friend inline constexpr bool operator!=(const ArucoCodeParams &lhs, const ArucoCodeParams &rhs)
    {
        return !(lhs == rhs);
    }
    double getMinMarkerPerimeter() const;
    void   setMinMarkerPerimeter(double newMinMarkerPerimeter);
    double getMaxMarkerPerimeter() const;
    void   setMaxMarkerPerimeter(double newMaxMarkerPerimeter);
    double getMinCornerDistance() const;
    void   setMinCornerDistance(double newMinCornerDistance);
    double getMinMarkerDistance() const;
    void   setMinMarkerDistance(double newMinMarkerDistance);
    int    getAdaptiveThreshWinSizeMin() const;
    void   setAdaptiveThreshWinSizeMin(int newAdaptiveThreshWinSizeMin);
    int    getAdaptiveThreshWinSizeMax() const;
    void   setAdaptiveThreshWinSizeMax(int newAdaptiveThreshWinSizeMax);
    int    getAdaptiveThreshWinSizeStep() const;
    void   setAdaptiveThreshWinSizeStep(int newAdaptiveThreshWinSizeStep);
    int    getAdaptiveThreshConstant() const;
    void   setAdaptiveThreshConstant(int newAdaptiveThreshConstant);
    double getPolygonalApproxAccuracyRate() const;
    void   setPolygonalApproxAccuracyRate(double newPolygonalApproxAccuracyRate);
    int    getMinDistanceToBorder() const;
    void   setMinDistanceToBorder(int newMinDistanceToBorder);
    bool   getDoCornerRefinement() const;
    void   setDoCornerRefinement(bool newDoCornerRefinement);
    int    getCornerRefinementWinSize() const;
    void   setCornerRefinementWinSize(int newCornerRefinementWinSize);
    int    getCornerRefinementMaxIterations() const;
    void   setCornerRefinementMaxIterations(int newCornerRefinementMaxIterations);
    double getCornerRefinementMinAccuracy() const;
    void   setCornerRefinementMinAccuracy(double newCornerRefinementMinAccuracy);
    int    getMarkerBorderBits() const;
    void   setMarkerBorderBits(int newMarkerBorderBits);
    int    getPerspectiveRemovePixelPerCell() const;
    void   setPerspectiveRemovePixelPerCell(int newPerspectiveRemovePixelPerCell);
    double getPerspectiveRemoveIgnoredMarginPerCell() const;
    void   setPerspectiveRemoveIgnoredMarginPerCell(double newPerspectiveRemoveIgnoredMarginPerCell);
    double getMaxErroneousBitsInBorderRate() const;
    void   setMaxErroneousBitsInBorderRate(double newMaxErroneousBitsInBorderRate);
    double getMinOtsuStdDev() const;
    void   setMinOtsuStdDev(double newMinOtsuStdDev);
    double getErrorCorrectionRate() const;
    void   setErrorCorrectionRate(double newErrorCorrectionRate);
};


class CodeMarkerOptions : public QObject
{
    Q_OBJECT

private:
    CodeMarkerItem *codeMarkerItem;
    int             indexOfMarkerDict = 16;

    ArucoCodeParams detectorParams;

    Control *controlWidget;
    Vec2F    offsetCropRect2Roi = Vec2F{0, 0};

public:
    // TODO: Remove getter and setter for pointers;
    // cannot properly set these in constructor because of
    // bidirectional dependecies
    CodeMarkerItem *getCodeMarkerItem() const { return codeMarkerItem; }
    Control        *getControlWidget() const { return controlWidget; }
    ArucoCodeParams getDetectorParams() const { return detectorParams; }
    int             getIndexOfMarkerDict() const { return indexOfMarkerDict; }

public:
    void setCodeMarkerItem(CodeMarkerItem *item) { codeMarkerItem = item; }
    void setControlWidget(Control *control) { controlWidget = control; }
    void setDetectorParams(ArucoCodeParams params);
    void setIndexOfMarkerDict(int idx);

    const Vec2F &getOffsetCropRect2Roi() const;
    void         setOffsetCropRect2Roi(const Vec2F &newOffsetCropRect2Roi);

signals:
    void detectorParamsChanged();
    void indexOfMarkerDictChanged();
};


class Recognizer : public QObject
{
    Q_OBJECT

private:
    // TODO add options for each marker type
    CodeMarkerOptions mCodeMarkerOptions;

    // default multicolor marker (until 11/2016 hermes marker)
    RecognitionMethod mRecoMethod = RecognitionMethod::MultiColor;

public:
    QList<TrackPoint> getMarkerPos(
        cv::Mat                     &img,
        QRect                       &roi,
        Control                     *controlWidget,
        int                          borderSize,
        BackgroundFilter            *bgFilter,
        const IntrinsicCameraParams &intrinsicCameraParams);

    RecognitionMethod  getRecoMethod() const { return mRecoMethod; }
    CodeMarkerOptions &getCodeMarkerOptions() { return mCodeMarkerOptions; }

public slots:
    void userChangedRecoMethod(RecognitionMethod method)
    {
        if(method != mRecoMethod)
        {
            mRecoMethod = method;
            emit recoMethodChanged(mRecoMethod);
        }
    }

signals:
    void recoMethodChanged(RecognitionMethod method);
};

// berechnet pixelverschiebung aufgrund von schraegsicht bei einem farbmarker
// Maik Dissertation Seite 138
// boxImageCentre ohne Border
Vec2F autoCorrectColorMarker(const Vec2F &boxImageCentre, Control *controlWidget);


namespace detail
{
    struct ColorBlob
    {
        cv::RotatedRect        box;          ///< bounding box
        Vec2F                  imageCenter;  ///< Center in whole image (instead of ROI)
        QColor                 color;        ///< color of detected blob
        std::vector<cv::Point> contour;      ///< detected contour
        double                 maxExpansion; ///< length of longer side of bounding rect
    };

    struct ColorBlobDetectionParams
    {
        QColor  fromColor;           ///< from color of the colormap
        QColor  toColor;             ///< to color of the colormap
        bool    invHue      = false; ///< flag indicating if hue should be inverted (for red-ish colors)
        bool    useOpen     = true;  ///< should open-operation be used
        bool    useClose    = true;  ///< should close-operation be used
        int     radiusOpen  = 5;     ///< radius of open-kernel
        int     radiusClose = 5;     ///< radius of close-kernel
        int     minArea     = 1000;  ///< min area of the contour
        int     maxArea     = 5000;  ///< max area of the contour
        Vec2F   offset;              ///< offset of ROI to image
        double  maxRatio = 2;        ///< maximum allowed ratio of sides for bounding rect
        cv::Mat img;                 ///< img in which to detect the blobs
        cv::Mat binary;              ///< img for the binary mask
    };

    struct BlackDotOptions
    {
        int  borderSize = 0; ///< size of border from borderFilter
        bool restrictPosition =
            false; ///< should the position of the black dot be restricted (see restrictPositionBlackDot())
        bool ignoreWithoutMarker = true;  ///< should markers without dot be ignored?
        bool autoCorrect         = false; ///< should perspective correction be performed
        bool autoCorrectOnlyExport =
            false;                      ///< should perspective correction only be performed when exporting trajectories
        ImageItem *imageItem = nullptr; ///< used for getAngleToGround and such
        QColor     midHue;              ///< middle hue of the color map
        double     dotSize       = 5;   ///< size of the black dot
        Control   *controlWidget = nullptr; ///< pointer to Control used for autoCorrect
    };

    struct ArucoOptions
    {
        Control *controlWidget       = nullptr; ///< pointer to Control used for autoCorrect
        bool     ignoreWithoutMarker = true;    ///< should a blob without valid arucoMarker be ignored
        bool     autoCorrect         = false;   ///< should perspective correction be performed
        bool     autoCorrectOnlyExport =
            false; ///< should perspective correction only be performed when exporting trajectories
        RecognitionMethod method =
            RecognitionMethod::Code; ///< Used recognition method; could be called from findMulticolorMarker
        CodeMarkerOptions &codeOpt;
    };

    std::vector<ColorBlob> findColorBlob(const ColorBlobDetectionParams &options);
    void                   restrictPositionBlackDot(ColorBlob &blob, ImageItem *imageItem, int bS, cv::Rect &cropRect);
    cv::Mat                customBgr2Gray(const cv::Mat &subImg, const QColor &midHue);
    void                   refineWithBlackDot(
                          std::vector<ColorBlob> &blobs,
                          const cv::Mat          &img,
                          QList<TrackPoint>      &crossList,
                          const BlackDotOptions  &options);
    void refineWithAruco(
        std::vector<ColorBlob>      &blobs,
        const cv::Mat               &img,
        QList<TrackPoint>           &crossList,
        ArucoOptions                &options,
        const IntrinsicCameraParams &intrinsicCameraParams);

    void resolveMoreThanOneCode(
        const int          lengthini,
        QList<TrackPoint> &crossList,
        const ColorBlob   &blob,
        const Vec2F        offset);

    void findCodeMarker(
        cv::Mat                     &img,
        QList<TrackPoint>           &crossList,
        RecognitionMethod            recoMethod,
        const CodeMarkerOptions     &opt,
        const IntrinsicCameraParams &intrinsicCameraParams);
    cv::Ptr<cv::aruco::Dictionary> getDictMip36h12();
} // namespace detail
} // namespace reco

Q_DECLARE_METATYPE(reco::RecognitionMethod)
#endif
