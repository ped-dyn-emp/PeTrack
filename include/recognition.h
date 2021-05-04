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

#ifndef RECOGNITION_H
#define RECOGNITION_H

#include <QList>
#include "vector.h"
#include <opencv2/aruco.hpp>

class TrackPoint;
class QRect;
class BackgroundFilter;
class Control;
class ImageItem;

namespace reco {
    /**
     * Different recognition methods used in PeTrack
     *
     * Note: be aware that changing the explicitly assigned integer values will break backwards compatibility!
     */
    enum class RecognitionMethod
    {
        Casern = 0,
        Hermes = 1,
        Stereo = 2,
        Color = 3,
        Japan = 4,
        MultiColor = 5,
        Code = 6,
    };

    // berechnet pixelverschiebung aufgrund von schraegsicht bei einem farbmarker
    // Maik Dissertation Seite 138
    // boxImageCentre ohne Border
    Vec2F autoCorrectColorMarker(Vec2F &boxImageCentre, Control *controlWidget);

    //void getMarkerPos(IplImage *iplImg, QRect roi, QList<TrackPoint> *crossList, int markerBrightness, int borderSize, bool ignoreWithoutMarker, bool autoWB, BackgroundFilter *bgFilter, int recoMethod);
    void getMarkerPos(cv::Mat &img, QRect &roi, QList<TrackPoint> *crossList, Control *controlWidget, int borderSize, BackgroundFilter *bgFilter);

    namespace detail {
        struct ColorBlob{
            cv::RotatedRect        box;          ///< bounding box
            Vec2F                  imageCenter;  ///< Center in whole image (instead of ROI)
            QColor                 color;        ///< color of detected blob
            std::vector<cv::Point> contour;      ///< detected contour
            double                 maxExpansion; ///< length of longer side of bounding rect
        };

        struct ColorBlobDetectionParams{
            QColor  fromColor;            ///< from color of the colormap
            QColor  toColor;              ///< to color of the colormap
            bool    invHue       = false; ///< flag indicating if hue should be inverted (for red-ish colors)
            bool    useOpen      = true;  ///< should open-operation be used
            bool    useClose     = true;  ///< should close-operation be used
            int     radiusOpen  = 5;     ///< radius of open-kernel
            int     radiusClose = 5;     ///< radius of close-kernel
            int     minArea      = 1000;  ///< min area of the contour
            int     maxArea      = 5000;  ///< max area of the contour
            Vec2F   offset;               ///< offset of ROI to image
            double  maxRatio = 2;         ///< maximum allowed ratio of sides for bounding rect
            cv::Mat img;                  ///< img in which to detect the blobs
            cv::Mat binary;               ///< img for the binary mask
        };

        struct BlackDotOptions
        {
            int  borderSize = 0; ///< size of border from borderFilter
            bool restrictPosition =
                false; ///< should the position of the black dot be restricted (see restrictPositionBlackDot())
            bool ignoreWithoutMarker   = true;  ///< should markers without dot be ignored?
            bool autoCorrect           = false; ///< should perspective correction be performed
            bool autoCorrectOnlyExport = false; ///< should perspective correction only be performed when exporting trajectories
            ImageItem *imageItem       = nullptr; ///< used for getAngleToGround and such
            QColor     midHue;                    ///< middle hue of the color map
            double     dotSize      = 5;         ///< size of the black dot
            Control *  controlWidget = nullptr;   ///< pointer to Control used for autoCorrect
        };

        struct ArucoOptions
        {
            Control *controlWidget       = nullptr; ///< pointer to Control used for autoCorrect
            bool     ignoreWithoutMarker = true;    ///< should a blob without valid arucoMarker be ignored
            bool     autoCorrect         = false;   ///< should perspective correction be performed
            bool autoCorrectOnlyExport   = false;   ///< should perspective correction only be performed when exporting trajectories
        };

        std::vector<ColorBlob> findColorBlob(const ColorBlobDetectionParams &options);
        void restrictPositionBlackDot(ColorBlob& blob, ImageItem *imageItem, int bS, cv::Rect& cropRect);
        cv::Mat customBgr2Gray(const cv::Mat& subImg, const QColor& midHue);
        void refineWithBlackDot(std::vector<ColorBlob>& blobs, const cv::Mat& img, QList<TrackPoint>& crossList, const BlackDotOptions& options);
        void refineWithAruco(std::vector<ColorBlob> &blobs, const cv::Mat& img, QList<TrackPoint> &crossList, ArucoOptions& options);

        void findCodeMarker(cv::Mat &img, QList<TrackPoint> *crossList, Control *controlWidget, Vec2F offsetCropRect2Roi=Vec2F{0,0});
        cv::Ptr<cv::aruco::Dictionary> getDictMip36h12();
    }
}

Q_DECLARE_METATYPE(reco::RecognitionMethod)
#endif
