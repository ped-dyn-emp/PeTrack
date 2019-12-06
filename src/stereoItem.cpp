#include <QtWidgets>

#include "petrack.h"
#include "view.h"
#include "stereoItem.h"
#include "stereoWidget.h"
#include "tracker.h"
#include "animation.h"

// in x und y gleichermassen skaliertes koordinatensystem,
// da von einer vorherigen intrinsischen kamerakalibrierung ausgegenagen wird,
// so dass pixel quadratisch 
StereoItem::StereoItem(QWidget *wParent, QGraphicsItem * parent)
    : QGraphicsItem(parent)
{
    mMainWindow = (class Petrack*) wParent;
    mImage = NULL;
    mDispNew = true;
    setAcceptHoverEvents(true);

    //    setEnabled(false); // all mouse events connot access this item, but it will be seen
    // einzig move koennte interessant sein, um grid zu verschieben?!
//     setAcceptsHoverEvents(true);
}
// // bounding box wird durch linke obere ecke und breite/hoehe angegeben
// // wenn an den rand gescrollt wurde im view, dann wird durch das dynamische anpassen
// // bei trans und scale zwar zuerst alles neu gezeichnet durch update, 
// // aber beim verkleinern des scrollbereichs nur der teil von tracker neu gezeichnet
QRectF StereoItem::boundingRect() const
{
    if (mMainWindow->getImage())
        return QRectF(-mMainWindow->getImageBorderSize(), -mMainWindow->getImageBorderSize(), mMainWindow->getImage()->width(), mMainWindow->getImage()->height());
//         return QRectF(-mMainWindow->getImageBorderSize(), -mMainWindow->getImageBorderSize(), mImage->width(), mImage->height());
    else
        return QRectF(0, 0, 0, 0);
//     // bounding box wird in lokalen koordinaten angegeben!!! (+-10 wegen zahl "1")
//     if (mControlWidget->getCalibCoordShow())
//         return QRectF(-110., -110., 220., 220.);
//     else                    ;

//         return QRectF(0., 0., 0., 0.);

//     // sicher ware diese boundingbox, da alles
//     //     return QRectF(xMin, yMin, xMax-xMin, yMax-yMin);
//     // eigentlich muesste folgende Zeile reichen, aber beim ranzoomen verschwindet dann koord.sys.
//     //     return QRectF(mControlWidget->getCalibCoordTransX()/10.-scale, mControlWidget->getCalibCoordTransY()/10.-scale, 2*scale, 2*scale);
}

// event, of moving mouse while button is pressed
void StereoItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
//    if (!mControlWidget->getCalibCoordFix())
//    {
//        setCursor(Qt::ClosedHandCursor);
//        QPointF diff = event->scenePos()-event->lastScenePos(); //screenPos()-buttonDownScreenPos(Qt::RightButton) also interesting
//        if (event->buttons() == Qt::RightButton) // event->button() doesnt work
//        {
//            mControlWidget->setCalibCoordRotate(mControlWidget->getCalibCoordRotate()+(int)(3.*(diff.x()+diff.y()))); //10* nicht noetig, da eh nur relativ
//            //cout << diff.x()-diff.y() <<endl; //globalPos()
//        }
//        else if (event->buttons() == Qt::LeftButton)
//        {
//            mControlWidget->setCalibCoordTransX(mControlWidget->getCalibCoordTransX()+(int)(10.*diff.x()));
//            mControlWidget->setCalibCoordTransY(mControlWidget->getCalibCoordTransY()+(int)(10.*diff.y()));
//        }
//        else if (event->buttons() == Qt::MidButton)
//        {
//            mControlWidget->setCalibCoordScale(mControlWidget->getCalibCoordScale()+(int)(10.*(diff.x()-diff.y())));
//        }
//    }
//    else
//        QGraphicsItem::mouseMoveEvent(event); // drag mach ich selber
}

// event, of moving mouse
void StereoItem::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{

    if (mMainWindow->getStereoContext())
    {
        static int lastX = -1, lastY = -1;
        QPointF pos = event->scenePos();
        pos.setX(pos.x() + mMainWindow->getImageBorderSize());
        pos.setY(pos.y() + mMainWindow->getImageBorderSize());
        if ((lastX != (int) pos.x()) || (lastY != (int) pos.y()))
        {
            float x, y, z;
            lastX = (int) pos.x();
            lastY = (int) pos.y();
            if (mMainWindow->getStereoContext()->getXYZ(lastX, lastY, &x, &y, &z)) // nicht runden: myround, dan 0.5 zuviel
            {
                mMainWindow->setStatusStereo(x, y, z);
            }
            else
            {
                mMainWindow->setStatusStereo(-1, -1, -1);
            }

            //debout <<lastX<<" "<< lastY<<" "<< x << " " << y << " " << z << endl;
        }
    }

    // ACHTUNG: border nicht beachtet!!!
    //QPointF pos = event->scenePos();
    //pos.setX(pos.x() + mMainWindow->getImageBorderSize());
    //pos.setY(pos.y() + mMainWindow->getImageBorderSize());
    mMainWindow->setMousePosOnImage(event->scenePos()); //pos);

//    if (pos.x() >=0 && pos.x() <= mMainWindow->getImage()->width() && pos.y() >=0 && pos.y() <= mMainWindow->getImage()->height())
//        mMainWindow->setMousePosOnImage(pos);
//
//    if (!mControlWidget->getCalibCoordFix())
//        setCursor(Qt::OpenHandCursor);
//    else
//        setCursor(Qt::CrossCursor);
//

    QGraphicsItem::hoverMoveEvent(event);
}

void StereoItem::updateData(IplImage *disp)
{
    if (disp != NULL)
    {
        if ((mImage != NULL) && ((mImage->width() != disp->width) || (mImage->height() != disp->height)))
            delete mImage;
        if (mImage == NULL) // zu Beginn oder wenn sich die Groesse aendert
            mImage = new QImage(disp->width, disp->height, QImage::Format_ARGB32);

        int x,y;
        unsigned short* data = (unsigned short*) disp->imageData; // char*
        unsigned short* yData = data;
        char *p;
        QColor color;
        unsigned short min = mMainWindow->getStereoContext()->getMin();
        unsigned short max = mMainWindow->getStereoContext()->getMax();
        double scale = max-min;
        int hue, value;

        if (scale > 0) // nicht min=max !
        {
            if (mMainWindow->getStereoWidget()->stereoColor->currentIndex() == 0) // rainbow
                scale=240./scale;
            else  // greyscale
                scale=255./scale;
        }

        for (y = 0; y < disp->height; y++)
        {
            // Pointer to the data information in the QImage for just one column
            // set pointer to value before, because ++p is faster than p++
            p = ((char*)mImage->scanLine(y))-1;
            for (x = 0; x < disp->width; x++)
            {
                if (mMainWindow->getStereoContext()->dispValueValid(*data)) //(*data != mMainWindow->getStereoContext()->getSurfaceValue()+65280) && (*data != mMainWindow->getStereoContext()->getBackForthValue()+65280)) // 65280 == 0xff00
                {
                    if (mMainWindow->getStereoWidget()->stereoColor->currentIndex() == 0) // rainbow
                    {
                        hue = scale*(*data-min);
                        color.setHsv(hue<0?0:(hue>240?240:hue), 255, 255, 255); // 0=rot, 240=blau, nicht 359, da sich Farbkreis wieder schliesst
                    }
                    else // greyscale
                    {
                        value = scale*(*data-min); // for greyscale
                        color.setHsv(0, 0, value<0?0:(value>255?255:value), 255); // 0=rot, 240=blau, nicht 359, da sich Farbkreis wieder schliesst
                    }
                    //c = (char) (((*data-min)*255)/(max-min)); // 255 nur 1x (nicht gleichverteilt!)
                }
                else
                {
                    color.setRgb(0, 0, 0, (!mMainWindow->getStereoWidget()->hideWrong->isChecked())*255); //.setAlpha(0);
                }
                *(++p) = color.red();
                *(++p) = color.green();
                *(++p) = color.blue();
                *(++p) = color.alpha(); // 255;
                //printf("%d ", (int)*(data));
                ++data;
            }
            data = (yData += disp->width); // falsch, da nicht mehr char sondern short: (yData += disp->widthStep); // because sometimes widthStep != width
            //printf("\n");
        }
    }
}
    
void StereoItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    IplImage *disp = NULL;
    bool mDispNewWhilePainting;

    if (mMainWindow->getStereoContext())
        disp = mMainWindow->getStereoContext()->getDisparity(&mDispNewWhilePainting);
    else
        return;

    if (mDispNew ||  // disparity ist nach zeichnen neu berechnet worden
        mDispNewWhilePainting) // disp ist wegen des zeichnens neu berechnet worden
        updateData(disp);

    if (mImage != NULL)
    {
        painter->setOpacity(mMainWindow->getStereoWidget()->opacity->value()/100.);
        painter->drawImage(-mMainWindow->getImageBorderSize(),-mMainWindow->getImageBorderSize(), *mImage);

        mDispNew = false;
    }
}

void StereoItem::setDispNew(bool d) // default: d = true
{
    mDispNew = d;
}
