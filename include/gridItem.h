#ifndef GRIDITEM_H
#define GRIDITEM_H

class Petrack;
class Control;

#include "extrCalibration.h"
#include "petrack.h"

class GridItem : public QGraphicsItem
{
private:
    Petrack *mMainWindow;
    ExtrCalibration *extCalib;
    Control *mControlWidget;
    float mouse_x, mouse_y;
    int gridTrans_x, gridTrans_y;
    int/*Petrack::Dimension*/ gridDimension;

public:
    inline void setGridDimension(int/*Petrack::Dimension*/ gDimension)
    {
        this->gridDimension = gDimension;
    }
    inline int/*Petrack::Dimension*/ getGridDimension()
    {
        return this->gridDimension;
    }
    GridItem(QWidget *wParent, QGraphicsItem * parent = 0);
    QRectF boundingRect() const;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
//    void hoverMoveEvent(QGraphicsSceneHoverEvent *event);
    int drawLine(QPainter *painter, cv::Point2f *p, int y_offset);
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
};

#endif
