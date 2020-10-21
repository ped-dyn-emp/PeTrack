#include <QtWidgets>

#include "petrack.h"
#include "view.h"
#include "backgroundItem.h"

using namespace::cv;

// in x und y gleichermassen skaliertes koordinatensystem,
// da von einer vorherigen intrinsischen kamerakalibrierung ausgegenagen wird,
// so dass pixel quadratisch 
BackgroundItem::BackgroundItem(QWidget *wParent, QGraphicsItem * parent)
    : QGraphicsItem(parent)
{
    mMainWindow = (class Petrack*) wParent;
    mImage = NULL;
    //    setEnabled(false); // all mouse events connot access this item, but it will be seen
    // einzig move koennte interessant sein, um grid zu verschieben?!
//     setAcceptsHoverEvents(true);
}
// // bounding box wird durch linke obere ecke und breite/hoehe angegeben
// // wenn an den rand gescrollt wurde im view, dann wird durch das dynamische anpassen
// // bei trans und scale zwar zuerst alles neu gezeichnet durch update, 
// // aber beim verkleinern des scrollbereichs nur der teil von tracker neu gezeichnet
QRectF BackgroundItem::boundingRect() const
{
    if (mMainWindow->getImage())
        return QRectF(-mMainWindow->getImageBorderSize(), -mMainWindow->getImageBorderSize(), mMainWindow->getImage()->width(), mMainWindow->getImage()->height());
    else
        return QRectF(0, 0, 0, 0);
}
    
void BackgroundItem::paint(QPainter *painter, const QStyleOptionGraphicsItem */*option*/, QWidget */*widget*/)
{
    Mat fg;

    if (mMainWindow->getBackgroundFilter())
        fg = mMainWindow->getBackgroundFilter()->getForeground();
    else
        return;

    if (!fg.empty())
    {
        if ((mImage != NULL) && ((mImage->width() != fg.cols) || (mImage->height() != fg.rows)))
            delete mImage;
        if (mImage == NULL) // zu Beginn oder wenn sich die Groesse aendert
            mImage = new QImage(fg.cols, fg.rows, QImage::Format_ARGB32);

        int x,y;
        auto* data =  fg.data;
        auto* yData = data;

        for (y = 0; y < fg.rows; y++)
        {
            // Pointer to the data information in the QImage for just one column
            // set pointer to value before, because ++p is faster than p++
            auto* p = mImage->scanLine(y)-1;
            for (x = 0; x < fg.cols; x++)
            {
//                if (*data == 1)
//                {
//                        value = ((*data-min)*255)/(max-min); // for greyscale
//                        color.setHsv(0, 0, value<0?0:(value>255?255:value), 255); // 0=rot, 240=blau, nicht 359, da sich Farbkreis wieder schliesst
//                }
//                else
//                {
//                    color.setRgb(0, 0, 0, (!mMainWindow->getStereoWidget()->hideWrong->isChecked())*255); //.setAlpha(0);
//                }
                *(++p) = 255; // color.red();
                *(++p) = 255; // color.green();
                *(++p) = 255; // color.blue();
                *(++p) = (1-*data)*255; // color.alpha(); // 255;
                ++data;
            }
            data = (yData += fg.cols);
        }

        painter->setOpacity(0.7);
        painter->drawImage(-mMainWindow->getImageBorderSize(),-mMainWindow->getImageBorderSize(), *mImage);
    }
}
