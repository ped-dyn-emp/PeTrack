#ifndef CodeMarkerItem_H
#define CodeMarkerItem_H

#include <QGraphicsItem>

#include "vector.h"

class Petrack;
class Control;
class Tracker;

class CodeMarkerItem : public QGraphicsItem
{
private:
    Petrack *mMainWindow;
//    QImage *mImage;

    std::vector<int> mIds;
    std::vector<std::vector<cv::Point2f> > mCorners, mRejected;
    Vec2F mUlc;  // upper left corner to draw
    Vec2F mOffsetCropRect2Roi; //

public:
    CodeMarkerItem(QWidget *wParent, QGraphicsItem * parent = NULL);
    QRectF boundingRect() const;
    void setRect(Vec2F& v);
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    void setDetectedMarkers(std::vector<std::vector<cv::Point2f> > corners, std::vector<int> ids);
    void setRejectedMarkers(std::vector<std::vector<cv::Point2f> > rejected);
    void setOffsetCropRect2Roi(Vec2F offsetCropRect2Roi);
};

#endif
