#include <iomanip>

#include <QPainter>
#include <QMouseEvent>

#include "qwt_plot_layout.h"
#include "qwt_plot_zoomer.h"
//#include "qwt_plot_panner.h"
//#include "qwt_plot_curve.h"
#include "qwt_symbol.h"
#include "qwt_scale_engine.h"
//#include "qwt_plot_canvas.h"
#include "qwt_compat.h"

#include "colorPlot.h"
#include "control.h"

// class SpectrogramData: public QwtRasterData
// {
// public:
//     SpectrogramData():
//         QwtRasterData(QwtDoubleRect(-1.5, -1.5, 3.0, 3.0))
//     {
//     }

//     virtual QwtRasterData *copy() const
//     {
//         return new SpectrogramData();
//     }

//     virtual QwtDoubleInterval range() const
//     {
//         return QwtDoubleInterval(0.0, 10.0);
//     }

//     virtual double value(double x, double y) const
//     {
//         const double c = 0.842;

//         const double v1 = x * x + (y-c) * (y+c);
//         const double v2 = x * (y+c) + x * (y+c);

//         return 1.0 / (v1 * v1 + v2 * v2);
//     }
// };

class ImagePlotItem: public QwtPlotItem
{
public:
    ImagePlotItem()
//         : mControlWidget(NULL)
    {
        mImage = new QImage(360,360,QImage::Format_RGB32);
//         setAxis(QwtPlot::xBottom, QwtPlot::yLeft);
    }

//     QwtDoubleRect boundingRect() const
//     {
//         return QwtDoubleRect(0., 0., 200., 200.);
//     }

    void draw(QPainter* p, const QwtScaleMap& mx, const QwtScaleMap& my, const QRectF& r) const
    {
//         debout << "---" <<endl;
//         debout << r.x() << " " << r.y() << " " << r.width() << " " << r.height()<<endl;
//         debout << "x:" <<endl;
//         debout << mx.p1() << " "<< mx.p2() << " " << mx.s1() << " "<< mx.s2() << " "<<endl;
//         debout << "y:" <<endl;
//         debout << my.p1() << " "<< my.p2() << " " << my.s1() << " "<< my.s2() << " "<<endl;

        p->save();
        p->scale(mx.p2()/(mx.s2() - mx.s1()), my.p1()/(my.s2() - my.s1()));//
//         p->scale((mx.p2() - mx.p1())/(mx.s2() - mx.s1()), (my.p1() - my.p2())/(my.s2() - my.s1()));//
        p->translate(-mx.s1(), my.s2()-((ColorPlot *) plot())->yMax());
        //mx.p1()-mx.s1(), my.s2()+my.p2()-((ColorPlot *) plot())->yMax()
        p->drawImage(0, 0, *mImage);
        p->restore();
    }

    // fuer x==y werden restliche komponenten gemaess z gewertet und das min in beide dimensionen als wert genommen
    void generateImage(int model, int x, int y, int z)
    {
//         debout << "generate image" <<endl;

//         if (!mControlWidget)
//             return;

        QColor col;
        int i, j;

        if (model == 0) // HSV //mControlWidget->recoColorModel->currentIndex()
        {
            // mControlWidget->recoColorX->currentIndex()
            for (i = 0; i < (x==0?360:256); ++i)
                for (j = 0; j < (y==0?360:256); ++j)
                {
                    col.setHsv(x==0?(y==0?min(i, j):i):(y==0?j:myRound(z*360./256.)), 
                               x==1?(y==1?min(i, j):i):(y==1?j:z), 
                               x==2?(y==2?min(i, j):i):(y==2?j:z));
                    mImage->setPixel(i, (y==0?359:255)-j, col.rgb());
                }
        }
        else // RGB
        {
            for (i = 0; i < 256; ++i)
                for (j = 0; j < 256; ++j)
                {
                    col.setRgb(x==0?(y==0?min(i, j):i):(y==0?j:z), 
                               x==1?(y==1?min(i, j):i):(y==1?j:z), 
                               x==2?(y==2?min(i, j):i):(y==2?j:z));
                    mImage->setPixel(i, 255-j, col.rgb());
                }
        }
    }

//     void setControlWidget(Control *control)
//     {
//         mControlWidget = control;

//         generateImage();
//     }

    inline QImage *getImage() const
    {
        return mImage;
    }
    
private:
    QImage *mImage;
//     Control *mControlWidget;
};


//-----------------------------------------------------------------------------------------

// die kreise liegen mgl nicht genau auf kreuz - noch zu testen
TrackerPlotItem::TrackerPlotItem()
{
    mTracker = NULL;
}

void TrackerPlotItem::draw(QPainter* p, const QwtScaleMap& mapX, const QwtScaleMap& mapY, const QRectF& re) const
{
//     // alt: ohne betrachtung des z-wertes:
//     QRectF rect;
//     double sx = mapX.p2()/(mapX.s2() - mapX.s1());
//     double sy = mapY.p1()/(mapY.s2() - mapY.s1());
//     double yMax = ((ColorPlot *) plot())->yMax();
//     double circleSize = ((ColorPlot *) plot())->symbolSize();
//     int i;

//     p->save();
//     p->scale(sx, sy);
//     p->translate(-mapX.s1(), mapY.s2()-yMax);

//     rect.setWidth(circleSize/sx);
//     rect.setHeight(circleSize/sy);
//     sx = circleSize/(2.*sx);
//     sy = circleSize/(2.*sy);
//     if (mTracker)
//     {
//         for (i = 0; i < mTracker->size(); ++i)
//         {
//             if ((*mTracker)[i].color().isValid()) // insbesondere von hand eingefuegte trackpoint/persons haben keine farbe
//             {
//                 QPoint point = ((ColorPlot *) plot())->getPos((*mTracker)[i].color());
//                 rect.moveLeft(point.x()-sx);
//                 rect.moveTop(point.y()-sy);
         
//                 p->setBrush(QBrush((*mTracker)[i].color()));
//                 if (((ColorPlot *) plot())->isGrey((*mTracker)[i].color()))
//                     p->setPen(Qt::red);
//                 else
//                     p->setPen(mPen);

//                 p->drawEllipse(rect);
//             }
//         }
//     }
//     p->restore();



    QRectF rect;
    double sx = mapX.p2()/(mapX.s2() - mapX.s1());
    double sy = mapY.p1()/(mapY.s2() - mapY.s1());
    double yMax = ((ColorPlot *) plot())->yMax();
    double circleSize = ((ColorPlot *) plot())->symbolSize();
    int i, z;
    int plotZ = ((ColorPlot *) plot())->zValue();
    double diff;

    //         debout << "---" <<endl;
    //         debout << re.x() << " " << re.y() << " " << re.width() << " " << re.height()<<endl;
    //         debout << "x:" <<endl;
    //         debout << mapX.p1() << " "<< mapX.p2() << " " << mapX.s1() << " "<< mapX.s2() << " "<<endl;
    //         debout << "y:" <<endl;
    //         debout << mapY.p1() << " "<< mapY.p2() << " " << mapY.s1() << " "<< mapY.s2() << " "<<endl;
    //         debout << mModel << " "<< mX << " " << mY<<endl;

    //         p->setPen(mPen);

    // ganu leicht verschiebung: eigentlich muessten noch andere werte wie diese einfliessen:
    //         p->scale((mx.p2() - mx.p1())/(mx.s2() - mx.s1()), (my.p1() - my.p2())/(my.s2() - my.s1()));//
    //mx.p1()-mx.s1(), my.s2()+my.p2()-((ColorPlot *) plot())->yMax()

    p->save();
    p->scale(sx, sy);
    p->translate(-mapX.s1(), mapY.s2()-yMax);

    sx = circleSize/sx;
    sy = circleSize/sy;

    if (mTracker)
    {
        for (i = 0; i < mTracker->size(); ++i)
        {
            if ((*mTracker)[i].color().isValid()) // insbesondere von hand eingefuegte trackpoint/persons haben keine farbe
            {
                QPoint point = ((ColorPlot *) plot())->getPos((*mTracker)[i].color(), &z);
                diff = (255.-abs(z-plotZ))/255.;
                rect.setWidth(diff*sx);
                rect.setHeight(diff*sy);
                rect.moveLeft(point.x()-diff*sx/2.);
                rect.moveTop(point.y()-diff*sy/2.);
         
                p->setBrush(QBrush((*mTracker)[i].color()));
                if (((ColorPlot *) plot())->isGrey((*mTracker)[i].color()))
                    p->setPen(Qt::red);
                else
                    p->setPen(mPen);

                p->drawEllipse(rect);
            }
        }
    }
    p->restore();
}

void TrackerPlotItem::setPen(const QPen &pen)
{
    mPen = pen;
}

// void TrackerPlotItem::setModel(int model, int x, int y)
// {
//     mModel = model;
//     mX = x;
//     mY = y;
// }

void TrackerPlotItem::setTracker(Tracker *tracker)
{
    mTracker = tracker;
}
Tracker * TrackerPlotItem::getTracker()
{
    return mTracker;
}

//     // wird auch missbraucht, um mX etc zu setzen, da updTr immer bei aenderung aufgerufen werden muss
// void TrackerPlotItem::updateTracker(int model, int x, int y)
// {
//     if (mTracker)
//     {
//         int i;

//         QwtArray<double> xData; // == QVector
//         QwtArray<double> yData;

//         // die for-schleife innerhalb der if und nicht anders herum aus performance gruenden
//         if (model == 0) // HSV
//         {
//             if (x==0)
//                 for (i = 0; i < mTracker->size(); ++i)
//                     xData.append((*mTracker)[i].color().hue());
//             else if (x==1)
//                 for (i = 0; i < mTracker->size(); ++i)
//                     xData.append((*mTracker)[i].color().saturation());
//             else
//                 for (i = 0; i < mTracker->size(); ++i)
//                     xData.append((*mTracker)[i].color().value());
//             if (y==0)
//                 for (i = 0; i < mTracker->size(); ++i)
//                     yData.append((*mTracker)[i].color().hue());
//             else if (y==1)
//                 for (i = 0; i < mTracker->size(); ++i)
//                     yData.append((*mTracker)[i].color().saturation());
//             else
//                 for (i = 0; i < mTracker->size(); ++i)
//                     yData.append((*mTracker)[i].color().value());
//         }
//         else // RGB
//         {
//             if (x==0)
//                 for (i = 0; i < mTracker->size(); ++i)
//                     xData.append((*mTracker)[i].color().red());
//             else if (x==1)
//                 for (i = 0; i < mTracker->size(); ++i)
//                     xData.append((*mTracker)[i].color().green());
//             else
//                 for (i = 0; i < mTracker->size(); ++i)
//                     xData.append((*mTracker)[i].color().blue());
//             if (y==0)
//                 for (i = 0; i < mTracker->size(); ++i)
//                     yData.append((*mTracker)[i].color().red());
//             else if (y==1)
//                 for (i = 0; i < mTracker->size(); ++i)
//                     yData.append((*mTracker)[i].color().green());
//             else
//                 for (i = 0; i < mTracker->size(); ++i)
//                     yData.append((*mTracker)[i].color().blue());
//         }

//         //mTpc->setData(xData, yData);
//         debout << "data " << xData.size() << " " << yData.size()<<endl;
//     }    
// }

//-----------------------------------------------------------------------------------------

RectPlotItem::RectPlotItem()
    : mActIndex(-1)
{
}

// auch wenn punkt auf linie liegt, wird er als drinnen gezaehlt!!!!
// es wird einfach der erste treffer in map-list genommen - unterscheidung zwischen farbe und graustufe
// wird keine farbige map fuer farbige col gefunden, wird in grauen map der erste treffer genommen und
// andersherum: wird fuer graue col keine graue map gefunden, wird erste farbige map zurueckgegeben
// und hoehe zurueckgegeben
double RectPlotItem::map(const QColor &col) const //const TrackPerson &tp RectMap ... double x, double y
{
    double yMax = ((ColorPlot *) plot())->yMax();
    RectMap map;
    bool isGrey;
    bool contains;
    double fallback = -1; // -1 soll anzeigen, dass nichts gefunden wurde

    for (int i = 0; i < mMaps.size(); ++i)
    {
        map = mMaps[i];
        map.moveTop(yMax-map.y()-map.height()); // nicht setY!!!!!, da dann height angepast wird
        isGrey = ((ColorPlot *) plot())->isGrey(col);
        if (map.invHue())
        {
            // Split inversHue Map into 2 maps, one for the left side and one for the right side
            QRectF leftMap(0,0,map.x(),map.height()),
                   rightMap(map.x()+map.width(),0,360-map.x()-map.width(),map.height());
            contains = leftMap.contains(((ColorPlot *) plot())->getPos(col)) ||
                       rightMap.contains(((ColorPlot *) plot())->getPos(col));
            //debout << "Point: " << (((ColorPlot *) plot())->getPos(col)).x() << "," << (((ColorPlot *) plot())->getPos(col)).y() << endl;
            //debout << "Map x=" << map.x() << ", y=" << map.y() << ", width=" << map.width() << ", height=" << map.height() << endl;
            //contains = map.contains(((ColorPlot *) plot())->getPos(col));
        }else
        {
            contains = map.contains(((ColorPlot *) plot())->getPos(col));
        }
        if (!isGrey && map.colored() && contains) //         QPoint point = ;
            return map.mapHeight();
        else if (isGrey && !map.colored() && contains)
            return map.mapHeight();
        else if (!isGrey && !map.colored() && contains && fallback == -1) // fallback == -1 damit der erste gefundene genommen wird
            fallback = map.mapHeight();
        else if (isGrey && map.colored() && contains && fallback == -1)
            fallback = map.mapHeight();
    }
    return fallback; // -1 soll anzeigen, dass nichts gefunden wurde
}

int RectPlotItem::addMap(double x, double y, double w, double h, bool colored, double height, QColor &fromCol, QColor &toCol, bool invHue)
{
//    debout << "x=" << x << ", y=" << y << ", w=" << w << ", h=" << h << ", colored=" << colored << ", fromCol=" << fromCol << ", toCol=" << toCol << ", invHue=" << invHue << endl;
    mMaps.append(RectMap(x, y, w, h, colored, height));
    mMaps.last().setFromColor(fromCol);
    mMaps.last().setToColor(toCol);
    mMaps.last().setInvHue(invHue);
//    debout << "mActIndex: " << mActIndex << ", mMaps.size(): " << mMaps.size() << endl;
    if (mActIndex < 0)
        mActIndex = 0;
//    debout << "mActIndex: " << mActIndex << ", mMaps.size(): " << mMaps.size() << endl;
    return mMaps.size()-1;
}
int RectPlotItem::addMap()
{
    if (mMaps.size()>0)
        mMaps.append(mMaps.last());
    else
        mMaps.append(RectMap(0, 0, 0, 0, true, DEFAULT_HEIGHT));
    if (mActIndex < 0)
        mActIndex = 0;
    return mMaps.size()-1;
}

void RectPlotItem::delMap(int index)
{
    if (index >= 0 && index < mMaps.size() && mMaps.size() > 0)
        mMaps.removeAt(index);
}

void RectPlotItem::changeMap(int index, double x, double y, double w, double h, bool colored, double mapHeight)
{
    if (index >= 0 && index < mMaps.size())
    {
        mMaps[index].setRect(x, y, w, h); // wenn
        mMaps[index].setColored(colored);
        mMaps[index].setMapHeight(mapHeight);
        mActIndex = index;
    }
}

void RectPlotItem::changeActMapInvHue(bool b)
{
    if (mActIndex >= 0 && mActIndex < mMaps.size())
    {
        mMaps[mActIndex].setInvHue(b);
    }
}

void RectPlotItem::changeActMapFromColor(const QColor &fromCol)
{
    if (mActIndex >= 0 && mActIndex < mMaps.size())
    {
        mMaps[mActIndex].setFromColor(fromCol);
    }
}

void RectPlotItem::changeActMapToColor(const QColor &toCol)
{
    if (mActIndex >= 0 && mActIndex < mMaps.size())
    {
        mMaps[mActIndex].setToColor(toCol);
    }
}

RectMap RectPlotItem::getMap(int index) const
{
    if (index >= 0 && index < mMaps.size())
        return mMaps[index];
    else
        return RectMap();
}

void RectPlotItem::draw(QPainter* p, const QwtScaleMap& mapX, const QwtScaleMap& mapY, const QRectF& re) const
{
    QRectF rect;
    double sx = mapX.p2()/(mapX.s2() - mapX.s1());
    double sy = mapY.p1()/(mapY.s2() - mapY.s1());
    double xMax = ((ColorPlot *) plot())->xMax();
    double yMax = ((ColorPlot *) plot())->yMax();
    int i;
    float rX,rW;

    //         debout << "---" <<endl;
    //         debout << re.x() << " " << re.y() << " " << re.width() << " " << re.height()<<endl;
    //         debout << "x:" <<endl;
    //         debout << mapX.p1() << " "<< mapX.p2() << " " << mapX.s1() << " "<< mapX.s2() << " "<<endl;
    //         debout << "y:" <<endl;
    //         debout << mapY.p1() << " "<< mapY.p2() << " " << mapY.s1() << " "<< mapY.s2() << " "<<endl;
    //         debout << mModel << " "<< mX << " " << mY<<endl;

    // ganu leicht verschiebung: eigentlich muessten noch andere werte wie diese einfliessen:
    //         p->scale((mx.p2() - mx.p1())/(mx.s2() - mx.s1()), (my.p1() - my.p2())/(my.s2() - my.s1()));//
    //mx.p1()-mx.s1(), my.s2()+my.p2()-((ColorPlot *) plot())->yMax()

    p->save();
    p->scale(sx, sy);
    p->translate(-mapX.s1(), mapY.s2()-yMax);

    for (i = 0; i < mMaps.size(); ++i)
    {
        if (i == mActIndex)
            p->setPen(Qt::green);
        else if (!mMaps[i].colored())
            p->setPen(Qt::red);
        else
            p->setPen(mPen);

        if (mMaps[i].invHue())
        {
            rect = mMaps[i]; //(QRectF) cast nicht noetig
            rect.moveTop(yMax-rect.y()-rect.height()); // nicht setY!!!!!, da dann height angepast wird
            rX = rect.x();
            rW = rect.width();
            rect.setX(0);
            rect.setWidth(rX);
            p->drawRect(rect);
            rect.setX(rX+rW);
            rect.setWidth(xMax-rX-rW);
            p->drawRect(rect);
        }
        else
        {
            rect = mMaps[i]; //(QRectF) cast nicht noetig
            rect.moveTop(yMax-rect.y()-rect.height()); // nicht setY!!!!!, da dann height angepast wird
            p->drawRect(rect);
        }
    }
    p->restore();
}

void RectPlotItem::setPen(const QPen &pen)
{
    mPen = pen;
}

//-----------------------------------------------------------------
class Zoomer: public QwtPlotZoomer
{
public:
    Zoomer(int xAxis, int yAxis, QWidget *canvas)
        : QwtPlotZoomer(xAxis, yAxis, canvas)
    {
        ////// Macht Probleme mit Qwt 6
        //////setSelectionFlags(QwtPicker::DragSelection | QwtPicker::CornerToCorner);
        // default: setRubberBand(QwtPicker::NoRubberBand);

        // RightButton: zoom out by 1
        // Ctrl+RightButton: zoom out to full size

        setMousePattern(QwtEventPattern::MouseSelect2, Qt::RightButton, Qt::ControlModifier);

        setMousePattern(QwtEventPattern::MouseSelect3, Qt::RightButton);

        setRubberBand(QwtPicker::RectRubberBand);
        //mZoomer->setRubberBandPen(QColor(Qt::green));
        setTrackerMode(QwtPicker::AlwaysOn); //ActiveOnly (only when the selection is active), AlwaysOff
        //mZoomer->setTrackerPen(QColor(Qt::white));
    }

    // ueberschrieben, da so kommazahlen unterdrueckt werden, da intervall mindestens 5 umfasst
// Qwt 5: QwtDoubleSize minZoomSize() const //QwtPlotZoomer::
    QSizeF minZoomSize() const
    {
        return QwtDoubleSize(5., 5.);
    }

// QwtPlot *plot()
// {
//     QObject *w = canvas();
//     if ( w )
//     {
//         w = w->parent();
//         if ( w && w->inherits("QwtPlot") )
//             return (QwtPlot *)w;
//     }

//     return NULL;
// }

//widgetWheelEvent  	(   	QWheelEvent *   	 e  	 )

    void widgetMouseMoveEvent(QMouseEvent *e)
    {
        static int lastX = -1;
        static int lastY = -1;
        int dx = e->x()-lastX;
        int dy = e->y()-lastY;

        lastX = e->x();
        lastY = e->y();

        //if (e->button() == Qt::MidButton) fkt bei move nicht
        if (e->buttons() == Qt::MidButton)
        {
            plot()->setAutoReplot(false);
            for (int axis = 0; axis < QwtPlot::axisCnt; axis++)
            {
                if (axis == QwtPlot::xBottom || axis == QwtPlot::yLeft)
                {
                    const QwtScaleMap map = plot()->canvasMap(axis);

                    const int i1 = map.transform(plot()->axisScaleDiv(axis).lowerBound()); // war in aelterer qwt version: axisScaleDiv(axis)->lBound()
                    const int i2 = map.transform(plot()->axisScaleDiv(axis).upperBound()); // war in aelterer qwt version: axisScaleDiv(axis)->hBound()

                    double d1, d2;
                    if ( axis == QwtPlot::xBottom || axis == QwtPlot::xTop )
                    {
                        d1 = map.invTransform(i1 - dx);
                        d2 = map.invTransform(i2 - dx);
                    }
                    else
                    {
                        d1 = map.invTransform(i1 - dy);
                        d2 = map.invTransform(i2 - dy);
                    }

                    //debout << zoomBase().width() << " " << zoomBase().height() << endl;
                    //debout << plot()->axisScaleDiv(axis)->lBound() << " " << plot()->axisScaleDiv(axis)->hBound() << endl;
                    if (d1 < 0)
                    {
                        d2 = d2-d1;
                        d1 = 0;
                    } 
                    else if ((axis == QwtPlot::xBottom) && (d2 > zoomBase().width()))
                    {
                        d1 = d1-(d2-zoomBase().width());
                        d2 = zoomBase().width();
                    } 
                    else if ((axis == QwtPlot::yLeft) && (d2 > zoomBase().height()))
                    {
                        d1 = d1-(d2-zoomBase().height());
                        d2 = zoomBase().height();
                    } 
                    plot()->setAxisScale(axis, d1, d2);
                }
            }
            plot()->setAutoReplot(true);
            plot()->replot();

        }
        QwtPlotZoomer::widgetMouseMoveEvent(e);
    }
//     // damit bei pan nicht text angezeigt wird // wenn eingebauter pan benutzt wird!!!
//     void widgetMousePressEvent(QMouseEvent *e)
//     {
//         if (e->button() == Qt::MidButton)
//             setTrackerMode(QwtPicker::AlwaysOff);

//         QwtPlotZoomer::widgetMousePressEvent(e);
//     }
//     void widgetMouseReleaseEvent(QMouseEvent *e)
//     {
//         if (e->button() == Qt::MidButton)
//             setTrackerMode(QwtPicker::AlwaysOn);

//         QwtPlotZoomer::widgetMouseReleaseEvent(e);
//     }

    QwtText trackerText(const QwtDoublePoint &pos) const
    {
        QString text;
 
//         switch(rubberBand())
//         {
//             case HLineRubberBand:
//                 text.sprintf("%.4f", pos.y());
//                 break;
//             case VLineRubberBand:
//                 text.sprintf("%.4f", pos.x());
//                 break;
//             default:
//                 text.sprintf("%.4f, %.4f", pos.x(), pos.y());
//         }
        text.sprintf("%d, %d", myRound(pos.x()), myRound(pos.y()));
        return QwtText(text);
//         return QwtPlotZoomer::trackerText(pos);
    }
};

//-----------------------------------------------------------------------------------------

class ViewColorPlotItem: public QwtPlotItem
{
public:
    ViewColorPlotItem()
    {
    }

    void draw(QPainter* p, const QwtScaleMap& mapX, const QwtScaleMap& mapY, const QRectF& re) const
    {
        QRectF rect;
        double sx = mapX.p2()/(mapX.s2() - mapX.s1());
        double sy = mapY.p1()/(mapY.s2() - mapY.s1());
        double yMax = ((ColorPlot *) plot())->yMax();
        double sS = ((ColorPlot *) plot())->symbolSize()*0.35355339;
        //         debout << "---" <<endl;
        //         debout << re.x() << " " << re.y() << " " << re.width() << " " << re.height()<<endl;
        //         debout << "x:" <<endl;
        //         debout << mapX.p1() << " "<< mapX.p2() << " " << mapX.s1() << " "<< mapX.s2() << " "<<endl;
        //         debout << "y:" <<endl;
        //         debout << mapY.p1() << " "<< mapY.p2() << " " << mapY.s1() << " "<< mapY.s2() << " "<<endl;
        //         debout << mModel << " "<< mX << " " << mY<<endl;

        // ganu leicht verschiebung: eigentlich muessten noch andere werte wie diese einfliessen:
        //         p->scale((mx.p2() - mx.p1())/(mx.s2() - mx.s1()), (my.p1() - my.p2())/(my.s2() - my.s1()));//
        //mx.p1()-mx.s1(), my.s2()+my.p2()-((ColorPlot *) plot())->yMax()

        p->save();

        p->scale(sx, sy);
        p->translate(-mapX.s1(), mapY.s2()-yMax);

        p->setPen(mPen);

        QPointF p1, p2;
        QPointF point = mPoint;

        //point.setY(yMax-mPoint.y());

        p1.setX(point.x()+sS/sx);
        p1.setY(point.y()+sS/sy);
        p2.setX(point.x()-sS/sx);
        p2.setY(point.y()-sS/sy);
        p->drawLine(p1, p2);
        p1.setY(point.y()-sS/sy);
        p2.setY(point.y()+sS/sy);
        p->drawLine(p1, p2);

        p->restore();
    }

    void setPen(const QPen &pen)
    {
        mPen = pen;
    }

    inline void setPoint(const QPoint &p)
    {
        mPoint = p;
    }

    inline QPoint point() const
    {
        return mPoint;
    }

private:
    QPoint mPoint;
    QPen mPen;
};

//-----------------------------------------------------------------------------------------

ColorPlot::ColorPlot(QWidget *parent) // default= NULL
        : QwtPlot(parent)
{
    mControlWidget = NULL;
//     mTracker = NULL;
    mGreyDiff = 50;
    mSymbolSize = 10.;

    // fkt nicht:
    //setFrameShape(Box); // damit rahmen gleich wie alle anderen group boxen
    //setFrameShadow(Sunken);

    QFont f("Courier", 10, QFont::Normal); //Times Helvetica, Normal Bold
    QwtText titleX("x");
    QwtText titleY("y");
    titleX.setFont(f);
    titleY.setFont(f);

    setAxisTitle(xBottom, titleX); //"x"
    setAxisTitle(yLeft, titleY); //"y"
    //setAxisFont(xBottom, f);
    //setAxisFont(yLeft, f);
    plotLayout()->setAlignCanvasToScales(true);
    //axisScaleEngine(xBottom)->setAttribute(QwtScaleEngine::Floating, false);
    //axisScaleEngine(yLeft)->setAttribute(QwtScaleEngine::Floating, false);
    //canvas()->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    //canvas()->setFixedWidth(400); //geometry().width()-
    //canvas()->setFixedHeight(400); //geometry().height()-

    mImageItem = new ImagePlotItem();
    mImageItem->attach(this);

    mZoomer = new Zoomer(xBottom, yLeft, canvas());

    //(new QwtPlotPanner(canvas()))->setMouseButton(Qt::MidButton);

//     mTpc = new QwtPlotCurve("color of tracking points");
//     mTpc->attach(this);
//     mTpc->setStyle(QwtPlotCurve::NoCurve);
//     mTpc->setSymbol(QwtSymbol(QwtSymbol::XCross, QBrush(Qt::white), QPen(Qt::black), QSize(4, 4)));
//     // Ellipse, Rect, Diamond, Triangle, DTriangle, UTriangle, LTriangle, RTriangle, Cross, XCross, HLine, VLine, Star1, Star2, Hexagon, StyleCnt 
//     //    mTpc->setPen(...);

     mTrackerItem = new TrackerPlotItem();
     mTrackerItem->attach(this);

     mRectItem = new RectPlotItem();
     mRectItem->attach(this);

     mViewColorItem = new ViewColorPlotItem();
     mViewColorItem->attach(this);

//  test:    mRectItem->addMap(10, 10, 100, 100, true, 1.40);

     //paintEngine()->hasFeature(QPaintEngine::Antialiasing);
    //setAutoReplot(true);

//      setMouseTracking(true); // mouse move events werden auch bei nichtdruecken einer moustaste ausgeloest
}

void ColorPlot::replot()
{
    QwtPlot::replot();
}

double ColorPlot::map(const QColor &col) const
{
    double height = mRectItem->map(col);
    if (height < 0)
        return mControlWidget->mapDefaultHeight->value();
    else
        return height;
}

// gibt false zurueck, wenn es keine groessenverteilung ueber farbe gab
bool ColorPlot::printDistribution() const
{
    QMap<double, int> dict;
    QMap<double, int>::const_iterator j;
    Tracker *tr = mTrackerItem->getTracker();
    int i, anz=0;

    for (i = 0; i < tr->size(); ++i)
    {
        if ((*tr)[i].color().isValid()) // insbesondere von hand eingefuegte trackpoint/persons haben keine farbe
        {
            ++dict[map((*tr)[i].color())];
        }
    }
    j = dict.constBegin();
    while (j != dict.constEnd()) {
        anz += j.value();
        ++j;
    }
    if (anz == 0)
        return false;
    j = dict.constBegin();
    while (j != dict.constEnd()) {
        debout << "height " << fixed << setprecision(1) << setw(5) << j.key() << " - number " << setw(3) << j.value() << " (" << setw(4) << (100.*j.value())/anz << "%)" << endl;
        ++j;
    }
    return true;
}

bool ColorPlot::isGrey(const QColor &col) const
{
    if (abs(col.red()-col.green()) < mGreyDiff && 
        abs(col.green()-col.blue()) < mGreyDiff && 
        abs(col.blue()-col.red()) < mGreyDiff)
        return true;
    else
        return false;
}
// void ColorPlot::mouseMoveEvent(QMouseEvent *event) 
// {
//     QFrame::mouseMoveEvent(event);
//     debout << "moveEvent" << endl;
// }

void ColorPlot::setCursor(const QColor &col)
{
    //QPoint pos = getPos(col);
    //QMouseEvent event(QEvent::MouseMove, pos, Qt::NoButton, Qt::NoButton, Qt::NoModifier);
    mViewColorItem->setPoint(getPos(col));
    //mZoomer->mouseMoveEvent(&event); //widgetM
    //QFrame::mouseMoveEvent(&event);
}

// ueber z kann die zur plotebene senkrechte kooerdinate zurueckgegeben werden,
// wenn default NULL nicht zutrifft
QPoint ColorPlot::getPos(const QColor &col, int *z) const
{
    QPoint p;

    if (mControlWidget)
    {
        int x = mControlWidget->recoColorX->currentIndex();
        int y = mControlWidget->recoColorY->currentIndex();
        int ymax = (int) yMax();

        if (mControlWidget->recoColorModel->currentIndex() == 0) // HSV
        {
            if (x==0) // nicht setX und setY, weil das width und height anpasst
                p.setX(col.hue());
            else if (x==1)
                p.setX(col.saturation());
            else
                p.setX(col.value());
            if (y==0)
                p.setY(ymax-col.hue());
            else if (y==1)
                p.setY(ymax-col.saturation());
            else
                p.setY(ymax-col.value());
            if (z != NULL)
            {
                if (x!=0 && y!=0)
                    *z = col.hue();
                else if (x!=1 && y!=1)
                    *z = col.saturation();
                else
                    *z = col.value();
            }
        }
        else // RGB
        {
            if (x==0)
                p.setX(col.red());
            else if (x==1)
                p.setX(col.green());
            else
                p.setX(col.blue());
            if (y==0)
                p.setY(ymax-col.red());
            else if (y==1)
                p.setY(ymax-col.green());
            else
                p.setY(ymax-col.blue());
            if (z != NULL)
            {
                if (x!=0 && y!=0)
                    *z = col.red();
                else if (x!=1 && y!=1)
                    *z = col.green();
                else
                    *z = col.blue();
            }
        }
    }

    return p;
}

void ColorPlot::setControlWidget(Control *control)
{
    mControlWidget = control;
}

void ColorPlot::setTracker(Tracker *tracker)
{
//     mTracker = tracker;
    mTrackerItem->setTracker(tracker);
}

void ColorPlot::setScale()
{
    if (mControlWidget)
    {
        int model = mControlWidget->recoColorModel->currentIndex();
        int x = mControlWidget->recoColorX->currentIndex();
        int y = mControlWidget->recoColorY->currentIndex();
        
//         mTrackerItem->setModel(model, x, y);

        QwtDoubleRect base(0., 0., 255., 255.);
        
        mXMax = 255.;
        mYMax = 255.;
        if (model == 0) // HSV //mControlWidget->recoColorModel->currentIndex()
        {
            if (x==0)
                mXMax = 359.;
            if (y==0)
                mYMax = 359.;
        }

//         setAutoReplot(false); // hiermit koennte temporaer das automatische skalieren auf werte von setaxisscale unterdrueckt werden
        setAxisScale(QwtPlot::xBottom, 0., mXMax);
        setAxisScale(QwtPlot::yLeft, 0., mYMax);
//         setAutoReplot(true);
//         replot();

//         setAxisAutoScale(QwtPlot::xBottom);
//         setAxisAutoScale(QwtPlot::yLeft);

        replot(); // why, see: file:///C:/Programme/qwt-5.0.1/doc/html/class_qwt_plot_zoomer.html#7a1711597f441223efdb7d9931fe19b9

        base.setWidth(mXMax);
        base.setHeight(mYMax);
        mZoomer->setZoomBase(base);
    }
}

void ColorPlot::generateImage() // int model, int x, int y, int z
{
    if (mControlWidget)
    {
        int model = mControlWidget->recoColorModel->currentIndex();
        int x = mControlWidget->recoColorX->currentIndex();
        int y = mControlWidget->recoColorY->currentIndex();
        int z = mControlWidget->recoColorZ->value();
        
        mImageItem->generateImage(model, x, y, z);

        // farbe anpassen, damit besser auf bild zu sehen
        // 255 immer genommen, da einfacher und es nicht so genau drauf ankommt
        int midValue = (QColor(mImageItem->getImage()->pixel(0, 0)).value()+
            QColor(mImageItem->getImage()->pixel(255, 0)).value()+
            QColor(mImageItem->getImage()->pixel(0, 255)).value()+
            QColor(mImageItem->getImage()->pixel(255, 255)).value())/4;
        if (midValue < 130)
        {
            mZoomer->setTrackerPen(QColor(Qt::white));
//             mTpc->setPen(QPen(Qt::white));
            mTrackerItem->setPen(QPen(Qt::white));
            mRectItem->setPen(QPen(Qt::white));
            mViewColorItem->setPen(QPen(Qt::white));
        }
        else
        {
            mZoomer->setTrackerPen(QColor(Qt::black));
//             mTpc->setPen(QPen(Qt::black));
            mTrackerItem->setPen(QPen(Qt::black));
            mRectItem->setPen(QPen(Qt::black));
            mViewColorItem->setPen(QPen(Qt::black));
        }
    }
}

// void ColorPlot::updateTracker()
// {
//     if (mTracker)
//     {
//         int model = mControlWidget->recoColorModel->currentIndex();
//         int x = mControlWidget->recoColorX->currentIndex();
//         int y = mControlWidget->recoColorY->currentIndex();
//         int i;

// //         mTrackerItem->updateTracker(model, x, y);

//         QwtArray<double> xData; // == QVector
//         QwtArray<double> yData;

//         // die for-schleife innerhalb der if und nicht anders herum aus performance gruenden
//         if (model == 0) // HSV
//         {
//             if (x==0)
//                 for (i = 0; i < mTracker->size(); ++i)
//                     xData.append((*mTracker)[i].color().hue());
//             else if (x==1)
//                 for (i = 0; i < mTracker->size(); ++i)
//                     xData.append((*mTracker)[i].color().saturation());
//             else
//                 for (i = 0; i < mTracker->size(); ++i)
//                     xData.append((*mTracker)[i].color().value());
//             if (y==0)
//                 for (i = 0; i < mTracker->size(); ++i)
//                     yData.append((*mTracker)[i].color().hue());
//             else if (y==1)
//                 for (i = 0; i < mTracker->size(); ++i)
//                     yData.append((*mTracker)[i].color().saturation());
//             else
//                 for (i = 0; i < mTracker->size(); ++i)
//                     yData.append((*mTracker)[i].color().value());
//         }
//         else // RGB
//         {
//             if (x==0)
//                 for (i = 0; i < mTracker->size(); ++i)
//                     xData.append((*mTracker)[i].color().red());
//             else if (x==1)
//                 for (i = 0; i < mTracker->size(); ++i)
//                     xData.append((*mTracker)[i].color().green());
//             else
//                 for (i = 0; i < mTracker->size(); ++i)
//                     xData.append((*mTracker)[i].color().blue());
//             if (y==0)
//                 for (i = 0; i < mTracker->size(); ++i)
//                     yData.append((*mTracker)[i].color().red());
//             else if (y==1)
//                 for (i = 0; i < mTracker->size(); ++i)
//                     yData.append((*mTracker)[i].color().green());
//             else
//                 for (i = 0; i < mTracker->size(); ++i)
//                     yData.append((*mTracker)[i].color().blue());
//         }

//         mTpc->setData(xData, yData);
//         debout << "data " << xData.size() << " " << yData.size()<<endl;
//     }    
// }

int ColorPlot::zValue() const
{
    if (mControlWidget)
        return mControlWidget->recoColorZ->value();
    else
        return 0;
}

