#ifndef ColorMarkerItem_H
#define ColorMarkerItem_H

#include <QGraphicsItem>

#include "vector.h"

class Petrack;
class Control;
class Tracker;

class ColorMarkerItem : public QGraphicsItem
{
private:
    Petrack *mMainWindow;
    QImage *mImage;
//    IplImage *mMask;
    cv::Mat mMask;
    Vec2F mUlc;  // upper left corner to draw

public:
    ColorMarkerItem(QWidget *wParent, QGraphicsItem * parent = NULL);
    QRectF boundingRect() const;
    void setRect(Vec2F& v);
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    cv::Mat getMask()
    {
        return mMask;
    }
    void setMask(cv::Mat &mask);
    cv::Mat createMask(int w, int h);
};

#endif
