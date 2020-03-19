#include <QtWidgets>

#include "petrack.h"
#include "control.h"
#include "view.h"
#include "trackingRoiItem.h"

using namespace::cv;

TrackingRoiItem::TrackingRoiItem(QWidget *wParent, QGraphicsItem *parent)
    : QGraphicsRectItem(parent)
{
    mMainWindow = (class Petrack*) wParent;
    mControlWidget = mMainWindow->getControlWidget();
    setRect(0, 0, 0, 0); //qreal x, qreal y, qreal width, qreal height
    QPen pen(Qt::blue);
    setPen(pen);
    setAcceptHoverEvents(true);
    setFlags(ItemIsMovable); // default in control
    hide(); // default in control
    //   setEnabled(false); // all mouse events connot access this item, but it will be seen
}

// QRectF RecognitionRoiItem::boundingRect() const
// {
//     // QRectF rect = QGraphicsRectItem::boundingRect();
//     // debout << rect.x() << " " << rect.y() << " " << rect.width() << " " << rect.height() <<endl;
//     return QGraphicsRectItem::boundingRect();
// }
// // bounding box wird durch linke obere ecke und breite/hoehe angegeben
// // wenn an den rand gescrollt wurde im view, dann wird durch das dynamische anpassen
// // bei trans und scale zwar zuerst alles neu gezeichnet durch update,
// // aber beim verkleinern des scrollbereichs nur der teil von coord neu gezeichnet
// QRectF RecognitionRoiItem::boundingRect() const
// {
//     // bounding box wird in lokalen koordinaten angegeben!!! (+-10 wegen zahl "1")
//     if (mControlWidget->getCalibCoordShow())
//         return QRectF(-110., -110., 220., 220.);
//     else
//         return QRectF(0., 0., 0., 0.);

//     // sicher ware diese boundingbox, da alles
//     //     return QRectF(xMin, yMin, xMax-xMin, yMax-yMin);
//     // eigentlich muesste folgende Zeile reichen, aber beim ranzoomen verschwindet dann koord.sys.
//     //     return QRectF(mControlWidget->getCalibCoordTransX()/10.-scale, mControlWidget->getCalibCoordTransY()/10.-scale, 2*scale, 2*scale);
// }

void TrackingRoiItem::mousePressEvent(QGraphicsSceneMouseEvent * event)
{
    if (!mControlWidget->getTrackRoiFix())
    {
        //mPressRect = rect();
        mPressRect = QRect(myRound(rect().left()), myRound(rect().top()), myRound(rect().width()), myRound(rect().height()));
        mPressPos = event->pos();
        if ((event->pos()).x() < DISTANCE_TO_BORDER+mPressRect.x())
        {
            if ((event->pos()).y() < DISTANCE_TO_BORDER+mPressRect.y())
            {
                mPressLocation = topLeft;
                setCursor(Qt::SizeFDiagCursor);
            }
            else if ((event->pos()).y() > mPressRect.height()+mPressRect.y()-DISTANCE_TO_BORDER)
            {
                mPressLocation = bottomLeft;
                setCursor(Qt::SizeBDiagCursor);
            }
            else
            {
                mPressLocation = left;
                setCursor(Qt::SizeHorCursor);
            }
        }
        else if ((event->pos()).x() > mPressRect.width()+mPressRect.x()-DISTANCE_TO_BORDER)
        {
            if ((event->pos()).y() < DISTANCE_TO_BORDER+mPressRect.y())
            {
                mPressLocation = topRight;
                setCursor(Qt::SizeBDiagCursor);
            }
            else if ((event->pos()).y() > mPressRect.height()+mPressRect.y()-DISTANCE_TO_BORDER)
            {
                mPressLocation = bottomRight;
                setCursor(Qt::SizeFDiagCursor);
            }
            else
            {
                mPressLocation = right;
                setCursor(Qt::SizeHorCursor);
            }
        }
        else if ((event->pos()).y() < DISTANCE_TO_BORDER+mPressRect.y())
        {
            mPressLocation = top;
            setCursor(Qt::SizeVerCursor);
        }
        else if ((event->pos()).y() > mPressRect.height()+mPressRect.y()-DISTANCE_TO_BORDER)
        {
            mPressLocation = bottom;
            setCursor(Qt::SizeVerCursor);
        }
        else
        {
            mPressLocation = inside;
            setCursor(Qt::ClosedHandCursor);
        }
    }

    QGraphicsRectItem::mousePressEvent(event);
}

void TrackingRoiItem::mouseReleaseEvent(QGraphicsSceneMouseEvent * event)
{
    mMainWindow->setTrackChanged(true);
    //mMainWindow->setRecognitionChanged(true);
    if( !mMainWindow->isLoading() )
        mMainWindow->updateImage();

    QGraphicsRectItem::mouseReleaseEvent(event);
}

// event, of moving mouse while mouse button is pressed
void TrackingRoiItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (!mControlWidget->getTrackRoiFix())
    {
        QImage *img = mMainWindow->getImage();
        //QPointF diff = event->pos()-mPressPos;
        QPoint diff = QPoint(myRound((event->pos()-mPressPos).x()), myRound((event->pos()-mPressPos).y()));
        // raender des bildes nicht ueberscheiten
        // swappen des rechtecks vermeiden, damit keine negativen width...
        if (img != NULL)
        {
            if (mPressLocation == inside || mPressLocation == topLeft || mPressLocation == left || mPressLocation == bottomLeft)
            {
                if (mPressRect.x()+diff.x() < -mMainWindow->getImageBorderSize())
                    diff.setX(-mPressRect.x()-mMainWindow->getImageBorderSize());
                if (mPressLocation != inside && mPressRect.width()-diff.x() < MIN_SIZE)
                    diff.setX(mPressRect.width()-MIN_SIZE);
            }
            if (mPressLocation == inside || mPressLocation == topLeft || mPressLocation == top || mPressLocation == topRight)
            {
                if (mPressRect.y()+diff.y() < -mMainWindow->getImageBorderSize())
                    diff.setY(-mPressRect.y()-mMainWindow->getImageBorderSize());
                if (mPressLocation != inside && mPressRect.height()-diff.y() < MIN_SIZE)
                    diff.setY(mPressRect.height()-MIN_SIZE);
            }
            if (mPressLocation == inside || mPressLocation == topRight || mPressLocation == right || mPressLocation == bottomRight)
            {
                if (mPressRect.x()+diff.x()+mPressRect.width() > img->width()-mMainWindow->getImageBorderSize())
                    diff.setX(img->width()-mPressRect.x()-mPressRect.width()-mMainWindow->getImageBorderSize());
                if (mPressLocation != inside && mPressRect.width()+diff.x() < MIN_SIZE)
                    diff.setX(-mPressRect.width()+MIN_SIZE);
            }
            if (mPressLocation == inside || mPressLocation == bottomLeft || mPressLocation == bottom || mPressLocation == bottomRight)
            {
                if (mPressRect.y()+diff.y()+mPressRect.height() > img->height()-mMainWindow->getImageBorderSize())
                    diff.setY(img->height()-mPressRect.y()-mPressRect.height()-mMainWindow->getImageBorderSize());
                if (mPressLocation != inside && mPressRect.height()+diff.y() < MIN_SIZE)
                    diff.setY(-mPressRect.height()+MIN_SIZE);
            }
        }
        if (mPressLocation == topLeft)
            setRect(mPressRect.x()+diff.x(), mPressRect.y()+diff.y(), mPressRect.width()-diff.x(), mPressRect.height()-diff.y());
        //            setRect(mPressRect.x()+diff.x(), mPressRect.y()+mPressRect.x()+diff.x(), mPressRect.y()+diff.y(), mPressRect.width()-diff.x(), mPressRect.height()-diff.y()diff.y(), mPressRect.width()-diff.x(), mPressRect.height()-diff.y());
        else if (mPressLocation == topRight)
            setRect(mPressRect.x(), mPressRect.y()+diff.y(), mPressRect.width()+diff.x(), mPressRect.height()-diff.y());
        else if (mPressLocation == bottomLeft)
            setRect(mPressRect.x()+diff.x(), mPressRect.y(), mPressRect.width()-diff.x(), mPressRect.height()+diff.y());
        else if (mPressLocation == bottomRight)
            setRect(mPressRect.x(), mPressRect.y(), mPressRect.width()+diff.x(), mPressRect.height()+diff.y());
        else if (mPressLocation == left)
            setRect(mPressRect.x()+diff.x(), mPressRect.y(), mPressRect.width()-diff.x(), mPressRect.height());
        else if (mPressLocation == right)
            setRect(mPressRect.x(), mPressRect.y(), mPressRect.width()+diff.x(), mPressRect.height());
        else if (mPressLocation == top)
            setRect(mPressRect.x(), mPressRect.y()+diff.y(), mPressRect.width(), mPressRect.height()-diff.y());
        else if (mPressLocation == bottom)
            setRect(mPressRect.x(), mPressRect.y(), mPressRect.width(), mPressRect.height()+diff.y());
        else // entspricht: if (mPressLocation == inside)
            setRect(mPressRect.x()+diff.x(), mPressRect.y()+diff.y(), mPressRect.width(), mPressRect.height());
            // nicht, da sonst koordinatensystem verschoben wird: QGraphicsRectItem::mouseMoveEvent(event); // drag
    }
    else // drag mach ich selber
        QGraphicsRectItem::mouseMoveEvent(event);
}

// // waere noetig, da sonst beim ersten pixel, wenn man objekt betritt, der cursor noch nicht richtig ist
// void RecognitionRoiItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
// {
//     hoverMoveEvent(event);
//     QGraphicsRectItem::hoverEnterEvent(event);
// }
// void RecognitionRoiItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
// {
//     //hoverMoveEvent(event);
//     setCursor(Qt::CrossCursor);
//     QGraphicsRectItem::hoverLeaveEvent(event);
// }

// event, of moving mouse
void TrackingRoiItem::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    QPointF pos = event->scenePos();
    pos.setX(pos.x() + mMainWindow->getImageBorderSize());
    pos.setY(pos.y() + mMainWindow->getImageBorderSize());
    // abfrage auf width() ..., da durch rectLinie die recoBox etwas groesser ist als das Bild und
    // es bei mMainWindow->setMousePosOnImage(pos); zum fehler beim bildzugriff kommen kann!!!
    if (mMainWindow->getImage() && //(pos.x() > 0) && (pos.y() > 0) &&
        (pos.x() < mMainWindow->getImage()->width()) && (pos.y() < mMainWindow->getImage()->height()))
    {
        mMainWindow->setMousePosOnImage(pos);

        if (!mControlWidget->getTrackRoiFix())
        {
            //QRectF r = rect();
            QRect r = QRect(myRound(rect().left()), myRound(rect().top()), myRound(rect().width()), myRound(rect().height()));
            if ((event->pos()).x() < DISTANCE_TO_BORDER+r.x())
            {
                if ((event->pos()).y() < DISTANCE_TO_BORDER+r.y())
                    setCursor(Qt::SizeFDiagCursor);
                else if ((event->pos()).y() > r.height()+r.y()-DISTANCE_TO_BORDER)
                    setCursor(Qt::SizeBDiagCursor);
                else
                    setCursor(Qt::SizeHorCursor);
            }
            else if ((event->pos()).x() > r.width()+r.x()-DISTANCE_TO_BORDER)
            {
                if ((event->pos()).y() < DISTANCE_TO_BORDER+r.y())
                    setCursor(Qt::SizeBDiagCursor);
                else if ((event->pos()).y() > r.height()+r.y()-DISTANCE_TO_BORDER)
                    setCursor(Qt::SizeFDiagCursor);
                else
                    setCursor(Qt::SizeHorCursor);
            }
            else if (((event->pos()).y() < DISTANCE_TO_BORDER+r.y()) || ((event->pos()).y() > r.height()+r.y()-DISTANCE_TO_BORDER))
                setCursor(Qt::SizeVerCursor);
            else
                setCursor(Qt::OpenHandCursor);
        }
        else // wird nur einmal durchaufen - ruecksetzen in control.cpp
        {
            setAcceptHoverEvents(false); // verhoindert nicht, dass wenn objekt darunter liegt, was andereen cursor haette - cursor wird weiterhin beim drueberfahren auf cross gesetzt
            setCursor(Qt::CrossCursor);
        }
    }

    QGraphicsRectItem::hoverMoveEvent(event);
}

// check rect because bordersize changes and without mouse event nothing changes the rect
void TrackingRoiItem::checkRect()
{
    Mat img = mMainWindow->getImageFiltered();
    // nicht QImage *img = mMainWindow->getImage(); da groesse noch nicht angepasst
    if (!img.empty())
    {
        QRect r = QRect(myRound(rect().left()), myRound(rect().top()), myRound(rect().width()), myRound(rect().height()));
        if (r.x() > img.cols-mMainWindow->getImageBorderSize()-MIN_SIZE ||
            r.y() > img.rows-mMainWindow->getImageBorderSize()-MIN_SIZE ||
            r.x()+r.width() < -mMainWindow->getImageBorderSize()+MIN_SIZE ||
            r.y()+r.height() < -mMainWindow->getImageBorderSize()+MIN_SIZE)
            setRect(-mMainWindow->getImageBorderSize(), -mMainWindow->getImageBorderSize(), img.cols, img.rows);
        else
        {
            if (r.x() < -mMainWindow->getImageBorderSize())
                setRect(-mMainWindow->getImageBorderSize(), r.y(), r.width()+(mMainWindow->getImageBorderSize()+r.x()), r.height());
            if (r.y() < -mMainWindow->getImageBorderSize())
                setRect(r.x(), -mMainWindow->getImageBorderSize(), r.width(), r.height()+(mMainWindow->getImageBorderSize()+r.y()));
            if (r.x()+mMainWindow->getImageBorderSize()+r.width() > img.cols)
            {
                //debout << r.x()+mMainWindow->getImageBorderSize()+r.width() << endl;
                setRect(r.x(), r.y(), img.cols-r.x()-mMainWindow->getImageBorderSize(), r.height());
            }
            if (r.y()+mMainWindow->getImageBorderSize()+r.height() > img.rows)
                setRect(r.x(), r.y(), r.width(), img.rows-r.y()-mMainWindow->getImageBorderSize());
        }
    }
    else
        setRect(0, 0, 0, 0);
}

