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

// berechnet pixelverschiebung aufgrund von schraegsicht bei einem farbmarker
// Maik Dissertation Seite 138
// boxImageCentre ohne Border
Vec2F autoCorrectColorMarker(Vec2F &boxImageCentre, Control *controlWidget);

//void getMarkerPos(IplImage *iplImg, QRect roi, QList<TrackPoint> *crossList, int markerBrightness, int borderSize, bool ignoreWithoutMarker, bool autoWB, BackgroundFilter *bgFilter, int recoMethod);
void getMarkerPos(cv::Mat &img, QRect &roi, QList<TrackPoint> *crossList, Control *controlWidget, int borderSize, BackgroundFilter *bgFilter);
void findCodeMarker(cv::Mat &img, QList<TrackPoint> *crossList, Control *controlWidget, Vec2F offsetCropRect2Roi=Vec2F{0,0});
cv::Ptr<cv::aruco::Dictionary> getDictMip36h12();
#endif
