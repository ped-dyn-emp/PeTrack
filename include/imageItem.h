#ifndef IMAGEITEM_H
#define IMAGEITEM_H

#include <QGraphicsItem>
//#include <QtWidgets>
//#include <QDragEnterEvent>
//#include <QDropEvent>

class Petrack;
class Control;

class ImageItem: public QGraphicsItem
{
private:
    Petrack *mMainWindow;
    Control *mControlWidget;
//     QPixmap *mImage;
    QImage *mImage;
    QGraphicsItem *mCoordItem;
public:
    ImageItem(QWidget *wParent, QGraphicsItem * parent = 0);

    QRectF boundingRect() const; 
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

//     void setLogo(QImage *img);
    void setImage(QImage *img);
    void setCoordItem(QGraphicsItem *ci);
    double getCmPerPixel();
    QPointF getCmPerPixel(float px, float py, float h = 0.);
    double getAngleToGround(float px, float py, float h = 0);
    QPointF getPosImage(QPointF pos, float height = 0.);
    QPointF getPosReal(QPointF pos, double height = 0.);

//    void dragEnterEvent(QDragEnterEvent *event);
//    void dropEvent(QDropEvent * event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent * event);
    void hoverMoveEvent(QGraphicsSceneHoverEvent * event);
//     bool sceneEvent(QEvent * event);
};

#endif
