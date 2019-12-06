#ifndef BACKGROUNDITEM_H
#define BACKGROUNDITEM_H

#include <QGraphicsItem>

class Petrack;
class Control;
class Tracker;

class BackgroundItem : public QGraphicsItem
{
private:
    Petrack *mMainWindow;
    QImage *mImage;

public:
    BackgroundItem(QWidget *wParent, QGraphicsItem * parent = NULL);
    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
};

#endif
