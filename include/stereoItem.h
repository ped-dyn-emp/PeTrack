#ifndef STEREOITEM_H
#define STEREOITEM_H

#include <QGraphicsItem>

//class Petrack;
//class Control;
//class Tracker;
#include "petrack.h"

class StereoItem : public QGraphicsItem
{
private:
    Petrack *mMainWindow;
    QImage *mImage;
    bool mDispNew; // indicates that a new disparity has been generated after the last drawing

public:
    StereoItem(QWidget *wParent, QGraphicsItem * parent = NULL);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void hoverMoveEvent(QGraphicsSceneHoverEvent *event);
#ifndef STEREO_DISABLED
    void updateData(IplImage *disp);
#endif
    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    void setDispNew(bool d = true);
};

#endif
