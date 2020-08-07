#ifndef RECOGNITION_H
#define RECOGNITION_H

//#include <cxcore.h>

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
