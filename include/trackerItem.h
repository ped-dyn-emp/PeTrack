#ifndef TRACKERITEM_H
#define TRACKERITEM_H

#include <QGraphicsItem>

class Petrack;
class Control;
class Tracker;

class TrackerItem : public QGraphicsItem
{
private:
    Petrack *mMainWindow;
    Control *mControlWidget;
    Tracker *mTracker;

public:
    TrackerItem(QWidget *wParent, Tracker *tracker, QGraphicsItem * parent = 0);
//    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
//    void hoverMoveEvent(QGraphicsSceneHoverEvent *event);
    void updateData();
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
};

#endif
