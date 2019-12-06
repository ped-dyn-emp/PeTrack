#ifndef RECOGNITIONROIITEM_H
#define RECOGNITIONROIITEM_H

#include <QGraphicsRectItem>

#define DISTANCE_TO_BORDER 5
#define MIN_SIZE 10

class Petrack;
class Control;


class RecognitionRoiItem : public QGraphicsRectItem
{
    enum pressLocation{inside, top, bottom, left, right, topLeft, topRight, bottomLeft, bottomRight};

private:
    Petrack *mMainWindow;
    Control *mControlWidget;
    //QRectF mPressRect;
    QRect mPressRect;
    QPointF mPressPos;
    enum pressLocation mPressLocation;

public:
    RecognitionRoiItem(QWidget *wParent, QGraphicsItem * parent = 0);
    void mousePressEvent(QGraphicsSceneMouseEvent * event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent * event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
//     void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
//     void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
    void hoverMoveEvent(QGraphicsSceneHoverEvent *event);
//     void updateData();
//     QRectF boundingRect() const;
//     void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    void checkRect();
};

#endif
