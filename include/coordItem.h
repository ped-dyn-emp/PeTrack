#ifndef COORDITEM_H
#define COORDITEM_H

#include <QGraphicsItem>
#include <QtWidgets>
//#include <opencv.hpp>
#include <petrack.h>

class Petrack;
class Control;
class ExtrCalibration;

class CoordItem : public QGraphicsItem
{
private:
    Petrack *mMainWindow;
    ExtrCalibration *extCalib;
    Control *mControlWidget;
    //QGraphicsScene *mScene;
    cv::Point2f ursprung, x, y, z;
    cv::Point2f calibPointsMin, calibPointsMax;
    cv::Point3f x3D, y3D, z3D;
    float mouse_x,mouse_y;
    int coordTrans_x,coordTrans_y;
    //Dimension *coordDimension
    int coordDimension;

public:
    inline void setCoordDimension(int dim/*Dimension *cDimension*/)
    {
        this->coordDimension = dim;
    }
    inline int /*Dimension*/ getCoordDimension()
    {
        return this->coordDimension;
    }

    // Constructor
    CoordItem(QWidget *wParent/*, QGraphicsScene * scene*/, QGraphicsItem * parent = 0);
    // mouseMoveEvent-Trigger
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    //void hoverMoveEvent(QGraphicsSceneHoverEvent *event);
    // Update the transformation matrix
    void updateData();
    // defines the minimal rect which contains the coordinate system
    QRectF boundingRect() const;
    // paint method to draw the perspectiv coordinate-system
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
};

#endif
