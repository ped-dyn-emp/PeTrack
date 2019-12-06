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

    vector<int> mIds;
    vector<vector<Point2f> > mCorners, mRejected;
    Vec2F mUlc;  // upper left corner to draw

public:
    CodeMarkerItem(QWidget *wParent, QGraphicsItem * parent = NULL);
    QRectF boundingRect() const;
    void setRect(Vec2F& v);
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    void setDetectedMarkers(vector<vector<Point2f> > corners, vector<int> ids);
    void setRejectedMarkers(vector<vector<Point2f> > rejected);
};

#endif
