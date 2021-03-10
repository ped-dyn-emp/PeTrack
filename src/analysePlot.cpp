/*
 * PeTrack - Software for tracking pedestrians movement in videos
 * Copyright (C) 2010-2020 Forschungszentrum Jülich GmbH,
 * Maik Boltes, Juliane Adrian, Ricardo Martin Brualla, Arne Graf, Paul Häger, Daniel Hillebrand,
 * Deniz Kilic, Paul Lieberenz, Daniel Salden, Tobias Schrödter, Ann Katrin Seemann
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <QPainter>
#include <QMouseEvent>

#include "qwt_plot_layout.h"
#include "qwt_plot_zoomer.h"
//#include "qwt_plot_panner.h"
//#include "qwt_plot_curve.h"
#include "qwt_symbol.h"
#include "qwt_scale_engine.h"
#include "qwt_plot_grid.h"
//#include "qwt_text.h"
#include "qwt_compat.h"
//#include "qwt_plot.h"

#include "analysePlot.h"
#include "control.h"
#include "petrack.h"
#include "animation.h"

//-----------------------------------------------------------------
class AnalyseZoomer: public QwtPlotZoomer
{
public:
    AnalyseZoomer(int xAxis, int yAxis, QWidget *canvas)
        : QwtPlotZoomer(xAxis, yAxis, canvas)
    {


////// Macht Probleme mit Qwt 6
//////       setSelectionFlags(QwtPicker::DragSelection | QwtPicker::CornerToCorner);
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

//     // ueberschrieben, da so kommazahlen unterdrueckt werden, da intervall mindestens 5 umfasst
//     QwtDoubleSize minZoomSize() const //QwtPlotZoomer::
//     {
//         return QwtDoubleSize(5., 5.);
//     }

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

    void widgetMouseMoveEvent(QMouseEvent *e) override
    {
        static int lastX = -1;
        static int lastY = -1;
        int dx = e->x()-lastX;
        int dy = e->y()-lastY;

        lastX = e->x();
        lastY = e->y();

        //if (e->button() == Qt::MidButton) fkt bei move nicht
        if (e->buttons() == Qt::MiddleButton)
        {
            plot()->setAutoReplot(false);
            for (int axis = 0; axis < QwtPlot::axisCnt; axis++)
            {
                if (axis == QwtPlot::xBottom || axis == QwtPlot::yLeft)
                {
                    const QwtScaleMap map = plot()->canvasMap(axis);

                    const int i1 = map.transform(plot()->axisScaleDiv(axis).lowerBound()); // war in alter qwt version: axisScaleDiv(axis)->lBound()
                    const int i2 = map.transform(plot()->axisScaleDiv(axis).upperBound()); // war in alter qwt version: axisScaleDiv(axis)->hBound()

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
//                     if (d1 < 0)
//                     {
//                         d2 = d2-d1;
//                         d1 = 0;
//                     } 
//                     else 
                    if ((axis == QwtPlot::xBottom) && (d2 > zoomBase().width()))
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

    QwtText trackerTextF(const QPointF &pos) const override
    {
        QString text;
        text = QString::asprintf("%d, %.2f", myRound(pos.x()), pos.y());
        return QwtText(text);
    }
};

//-----------------------------------------------------------------------------------------

// die kreise liegen mgl nicht genau auf kreuz - noch zu testen
TrackerRealPlotItem::TrackerRealPlotItem()
{
    mTrackerReal = nullptr;
}

void TrackerRealPlotItem::draw(QPainter* p, const QwtScaleMap& mapX, const QwtScaleMap& mapY, const QRectF& /*re*/) const
{
    Control *controlWidget = ((AnalysePlot *) plot())->getControlWidget();
    if (mTrackerReal && (mTrackerReal->size() > 0) && controlWidget != nullptr)
    {
        QRectF rect;
        double sx = (mapX.p2() - mapX.p1())/(mapX.s2() - mapX.s1());
        double sy = (mapY.p2() - mapY.p1())/(mapY.s2() - mapY.s1());
        double circleSize = ((AnalysePlot *) plot())->symbolSize();
        int i, j;
        QPointF point, lastPoint;

        //     debout << "---" <<endl;
        //     debout << re.x() << " " << re.y() << " " << re.width() << " " << re.height()<<endl;
        //     debout << "x:" <<endl;
        //     debout << mapX.p1() << " "<< mapX.p2() << " " << mapX.s1() << " "<< mapX.s2() << " "<<endl;
        //     debout << "y:" <<endl;
        //     debout << mapY.p1() << " "<< mapY.p2() << " " << mapY.s1() << " "<< mapY.s2() << " "<<endl;

        p->save();
        p->translate(mapX.p1(), mapY.p2()); // in pixelkoordinaten vor skalierung
        p->scale(sx, sy);
        p->translate(-mapX.s1(), -mapY.s2());

        //     p->drawLine(QPointF(-10, -10), QPointF(-10, 10));
        //     p->drawLine(QPointF(-10, 10), QPointF(10, 10));
        //     p->drawLine(QPointF(10, 10), QPointF(10, -10));
        //     p->drawLine(QPointF(10, -10), QPointF(-10, -10));

        p->setPen(Qt::red);

        rect.setWidth(circleSize/sx);
        rect.setHeight(circleSize/sy);
        sx = circleSize/(2.*sx);
        sy = circleSize/(2.*sy);

        bool anaConsiderX = controlWidget->anaConsiderX->isChecked();
        bool anaConsiderY = controlWidget->anaConsiderY->isChecked();
        bool anaConsiderAbs = controlWidget->anaConsiderAbs->isChecked();
        bool anaConsiderRev = controlWidget->anaConsiderRev->isChecked();

        // Beschriftung
        static QFont f("Courier", 10, QFont::Normal); //Times Helvetica, Normal Bold
        QwtText titleX("t [frame]", QwtText::RichText); //"x" TeXText
        QwtText titleY;
        if (anaConsiderX && anaConsiderY)
        {
//             if (anaConsiderAbs)
//                 titleY.setText("|v| [m/s]", QwtText::RichText);
//             else if (anaConsiderRev)
//                 titleY.setText("-v [m/s]", QwtText::RichText);
//             else
            titleY.setText("v [m/s]", QwtText::RichText);
        }
        else if (anaConsiderX)
        {
            if (anaConsiderAbs)
                titleY.setText("v<sub>|x|</sub> [m/s]", QwtText::RichText);
            else if (anaConsiderRev)
                titleY.setText("v<sub>-x</sub> [m/s]", QwtText::RichText);
            else
                titleY.setText("v<sub>x</sub> [m/s]", QwtText::RichText);
        }
        else // == if (anaConsiderY)
        {
            if (anaConsiderAbs)
                titleY.setText("v<sub>|y|</sub> [m/s]", QwtText::RichText);
            else if (anaConsiderRev)
                titleY.setText("v<sub>-y</sub> [m/s]", QwtText::RichText);
            else
                titleY.setText("v<sub>y</sub> [m/s]", QwtText::RichText);
        }
        //QwtText titleY("v<sub>y</sub> [m/s]", QwtText::RichText); //"y"
        titleX.setFont(f);
        titleY.setFont(f);
        ((AnalysePlot *) plot())->setAxisTitle(QwtPlot::xBottom, titleX); //"x"
        ((AnalysePlot *) plot())->setAxisTitle(QwtPlot::yLeft, titleY); //"y"
        //setAxisFont(xBottom, f);
        //setAxisFont(yLeft, f);

        //         for (i = 0; i < mTrackerReal->size(); ++i)
//         {
//             for (j = 0; j < mTrackerReal->at(i).size(); ++j)
//             {
//                 point = mTrackerReal->at(i).at(j).toQPointF();
//                 if (j!=0)
//                     ;//p->drawLine(point, lastPoint);
//                 rect.moveLeft(point.x()-sx);
//                 rect.moveTop(point.y()-sy);
//                 //p->setPen(Qt::red);
//                 //p->setBrush(QBrush((*mTracker)[i].color()));
//                 //p->drawEllipse(rect);
//                 p->drawPoint(point);
//                 p->drawRect(rect);
//                 lastPoint = point;
//             }
//         }

        int step = controlWidget->anaStep->value(); //1
        int frame, animFrame, velVecActIdx = -1;
        double vel; // geschwindigkeit
        int largestLastFrame = mTrackerReal->largestLastFrame();
        QVector<int> velAnzVec(largestLastFrame, 0); //mControlWidget->getMainWindow()->getAnimation()->getNumFrames()
        QVector<double> velVec(largestLastFrame, 0.);
        int actFrame = ((AnalysePlot *) plot())->getActFrame();
        //int actFrame = ((AnalysePlot *) plot())->getControlWidget()->getMainWindow()->getAnimation()->getCurrentFrameNum();
        bool markAct = controlWidget->anaMarkAct->isChecked();
        double fps = controlWidget->getMainWindow()->getAnimation()->getFPS();
        if (fps < 0)
            fps =DEFAULT_FPS;

//         QPen linePen;
//         linePen.setColor(Qt::red);
//         linePen.setWidthF(3/sx);
//         p->setPen(linePen);
//         p->pen().setWidth(3); // geht nicht
//         p->setPen(Qt::red);
//         p->setBrush(Qt::red);
        //p->setBrush(Qt::red); //QBrush(Qt::red)

        if (!markAct)
        {
            p->setPen(Qt::green);
            p->setBrush(Qt::green);
        }
        for (i = 0; i < mTrackerReal->size(); ++i)
        {
            for (j = 0; j < mTrackerReal->at(i).size()-step; ++j) // -step, damit geschwindigkeit ermittelt werden kann
            {
                frame = mTrackerReal->at(i).firstFrame()+j;
                animFrame = mTrackerReal->at(i).at(j).frameNum(); // ohne eingefuegte frames bei auslassungen
                if (markAct)
                {
                    if (animFrame == actFrame)
                    {
                        p->setPen(Qt::red);
                        p->setBrush(Qt::red);
                        velVecActIdx = frame;
                    }
                    else //if (frame == actFrame+1)
                    {
                        p->setPen(Qt::green);
                        p->setBrush(Qt::green);
                    }
                }

                if (anaConsiderX && anaConsiderY)
                {
//                     if (anaConsiderAbs)
//                     else if (anaConsiderRev)
//                     else
                    vel = (mTrackerReal->at(i).at(j+step).distanceToPoint(mTrackerReal->at(i).at(j)));
                }
                else if (anaConsiderX)
                {
                    if (anaConsiderAbs)
                        vel = fabs((mTrackerReal->at(i).at(j+step).x()-mTrackerReal->at(i).at(j).x()));
                    else if (anaConsiderRev)
                        vel = (mTrackerReal->at(i).at(j).x()-mTrackerReal->at(i).at(j+step).x());
                    else
                        vel = (mTrackerReal->at(i).at(j+step).x()-mTrackerReal->at(i).at(j).x());
                }
                else // == if (anaConsiderY)
                {
                    if (anaConsiderAbs)
                        vel = fabs((mTrackerReal->at(i).at(j+step).y()-mTrackerReal->at(i).at(j).y()));
                    else if (anaConsiderRev)
                        vel = (mTrackerReal->at(i).at(j).y()-mTrackerReal->at(i).at(j+step).y());
                    else
                        vel = (mTrackerReal->at(i).at(j+step).y()-mTrackerReal->at(i).at(j).y());
                }
                vel /=((100./fps)*step); // m/s, war: 100cm/25frames =4 => vel /=(4.*step);

                point.setX(frame);
                point.setY(vel);
                rect.moveLeft(point.x()-sx);
                rect.moveTop(point.y()-sy);
                p->drawEllipse(rect);
                //p->drawPoint(point);

                // j - j+step, da gegen die x-achse gelaufen wird

                ++velAnzVec[frame];
                velVec[frame] += vel;
            }
//         p->setPen(Qt::red);
//         p->setBrush(Qt::red);
        }

        rect.setWidth(2*rect.width());
        rect.setHeight(2*rect.height());
        p->setPen(Qt::blue);
        p->setBrush(Qt::blue);
        for (i = 0; i < velAnzVec.size(); ++i)
        {
            if (velAnzVec[i] != 0)
            {
                point.setX(i);
                point.setY(velVec[i]/velAnzVec[i]);
                if ((i != 0) && (velAnzVec[i-1] != 0)) // nicht ganz hundertprozentig
                    p->drawLine(lastPoint, point);
                lastPoint = point;
                if (markAct && (i == velVecActIdx))
                {
                    rect.moveLeft(point.x()-2*sx);
                    rect.moveTop(point.y()-2*sy);
                    p->drawEllipse(rect);
                }
            }
        }
        //0..getAnimation()->getNumFrames();
        //smallestFirstFrame()..largestLastFrame()
        //int smallestFirstFrame = mTrackerReal->smallestFirstFrame();
        //int largestLastFrame = mTrackerReal->largestLastFrame();
        //for (i = smallestFirstFrame; i <= largestLastFrame; ++i)

        p->restore();
    }
}

void TrackerRealPlotItem::setPen(const QPen &pen)
{
    mPen = pen;
}

void TrackerRealPlotItem::setTrackerReal(TrackerReal *trackerReal)
{
    mTrackerReal = trackerReal;
}

//-----------------------------------------------------------------------------------------

AnalysePlot::AnalysePlot(QWidget *parent) // default= NULL
        : QwtPlot(parent)
{
    setAutoReplot(false);
    //setTitle("Frequency Response of a Second-Order System");
    setCanvasBackground(QColor(QColor(220, 220, 255)));

    mControlWidget = nullptr;
    mSymbolSize = 3.;
    mActFrame = 0;

    // default in controlWidget - ansonsten wird es in plotitem geaendert
    QFont f("Courier", 10, QFont::Normal); //Times Helvetica, Normal Bold
    QwtText titleX("t [frame]", QwtText::RichText); //"x" TeXText
    QwtText titleY("v<sub>y</sub> [m/s]", QwtText::RichText); //"y"
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

    mZoomer = new AnalyseZoomer(xBottom, yLeft, canvas()); //QwtPlotZoomer

    //(new QwtPlotPanner(canvas()))->setMouseButton(Qt::MidButton);

//     mTpc = new QwtPlotCurve("color of tracking points");
//     mTpc->attach(this);
//     mTpc->setStyle(QwtPlotCurve::NoCurve);
//     mTpc->setSymbol(QwtSymbol(QwtSymbol::XCross, QBrush(Qt::white), QPen(Qt::black), QSize(4, 4)));
//     // Ellipse, Rect, Diamond, Triangle, DTriangle, UTriangle, LTriangle, RTriangle, Cross, XCross, HLine, VLine, Star1, Star2, Hexagon, StyleCnt 
//     //    mTpc->setPen(...);

    mTrackerRealItem = new TrackerRealPlotItem();
    mTrackerRealItem->setZ(1);
    mTrackerRealItem->attach(this);

//      mRectItem = new RectPlotItem();
//      mRectItem->attach(this);

//      mViewAnalyseItem = new ViewAnalysePlotItem();
//      mViewAnalyseItem->attach(this);

//  test:    mRectItem->addMap(10, 10, 100, 100, true, 1.40);

     //paintEngine()->hasFeature(QPaintEngine::Antialiasing);
    //setAutoReplot(true);

//      setMouseTracking(true); // mouse move events werden auch bei nichtdruecken einer moustaste ausgeloest
//         setAxisAutoScale(QwtPlot::xBottom);
//         setAxisAutoScale(QwtPlot::yLeft);
//         setAxisAutoScale(QwtPlot::xTop);
//         setAxisAutoScale(QwtPlot::yRight);

    // grid 
    QwtPlotGrid *grid = new QwtPlotGrid;
    grid->enableXMin(true);
    grid->enableYMin(true);
    grid->setMajorPen(QPen(QColor(130, 130, 130), 0, Qt::DotLine));
    grid->setMinorPen(QPen(QColor(170, 170, 170), 0 , Qt::DotLine));
    grid->setZ(0);
    grid->attach(this);

}

// double AnalysePlot::map(const QColor &col) const
// {
//     double height = 1.;
//     if (height < 0)
//         return mControlWidget->mapDefaultHeight->value();
//     else
//         return height;
// }

// void AnalysePlot::mouseMoveEvent(QMouseEvent *event) 
// {
//     QFrame::mouseMoveEvent(event);
//     debout << "moveEvent" << endl;
// }

void AnalysePlot::setCursor(const QColor &/*col*/)
{
    //QPoint pos = getPos(col);
    //QMouseEvent event(QEvent::MouseMove, pos, Qt::NoButton, Qt::NoButton, Qt::NoModifier);
//     mViewAnalyseItem->setPoint(getPos(col));
    //mZoomer->mouseMoveEvent(&event); //widgetM
    //QFrame::mouseMoveEvent(&event);
}

QPoint AnalysePlot::getPos(const QColor &col) const
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
        }
    }

    return p;
}

void AnalysePlot::setTrackerReal(TrackerReal* trackerReal)
{
    mTrackerReal = trackerReal;
    mTrackerRealItem->setTrackerReal(trackerReal);
}

void AnalysePlot::setScale()
{
    if (mControlWidget)
    {
//         int model = mControlWidget->recoColorModel->currentIndex();
//         int x = mControlWidget->recoColorX->currentIndex();
//         int y = mControlWidget->recoColorY->currentIndex();
        
//         mXMax = 255.;
//         mYMax = 255.;
//         if (model == 0) // HSV //mControlWidget->recoColorModel->currentIndex()
//         {
//             if (x==0)
//                 mXMax = 359.;
//             if (y==0)
//                 mYMax = 359.;
//         }

//         double spacerX = (mTrackerReal->xMax()-mTrackerReal->xMin())/20.;
//         double spacerY = (mTrackerReal->yMax()-mTrackerReal->yMin())/20.;
//         mXMin = mTrackerReal->xMin()-spacerX; 
//         mXMax = mTrackerReal->xMax()+spacerX; 
//         mYMin = mTrackerReal->yMin()-spacerY; 
//         mYMax = mTrackerReal->yMax()+spacerY;

        int frameNum = mTrackerReal->largestLastFrame(); //mControlWidget->getMainWindow()->getAnimation()->getNumFrames();
        //double spacerX = frameNum/20.;
        //double spacerY = 0.5;
        mXMin = mTrackerReal->smallestFirstFrame()-10; //0-spacerX; 
        mXMax = frameNum+10; //frameNum+spacerX; 
        mYMin = -0.2; //0-spacerY; 
        mYMax = 2.; //2+spacerY;

        setAxisScale(QwtPlot::xBottom, mXMin, mXMax);
        setAxisScale(QwtPlot::yLeft, mYMin, mYMax);
        replot(); // why, see: file:///C:/Programme/qwt-5.0.2/doc/html/class_qwt_plot_zoomer.html#7a1711597f441223efdb7d9931fe19b9
        mZoomer->setZoomBase(QwtDoubleRect(mXMin, mYMin, mXMax-mXMin, mYMax-mYMin));

//         setAutoReplot(false); // hiermit koennte temporaer das automatische skalieren auf werte von setaxisscale unterdrueckt werden
//         setAutoReplot(true);
//         setAxisAutoScale(QwtPlot::xBottom);
//         setAxisAutoScale(QwtPlot::yLeft);


    }
}

#include "moc_analysePlot.cpp"
