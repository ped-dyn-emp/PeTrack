/*
 * PeTrack - Software for tracking pedestrians movement in videos
 * Copyright (C) 2024 Forschungszentrum Jülich GmbH, IAS-7
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

#include "analysePlot.h"

#include "animation.h"
#include "control.h"
#include "petrack.h"

#include <QMouseEvent>
#include <QPainter>
#include <qwt_plot_grid.h>
#include <qwt_plot_layout.h>
#include <qwt_plot_zoomer.h>
#include <qwt_scale_engine.h>
#include <qwt_scale_map.h>
#include <qwt_symbol.h>
#include <qwt_text.h>

//-----------------------------------------------------------------
class AnalyseZoomer : public QwtPlotZoomer
{
public:
    AnalyseZoomer(int xAxis, int yAxis, QWidget *canvas) : QwtPlotZoomer(xAxis, yAxis, canvas)
    {
        // RightButton: zoom out by 1
        // Ctrl+RightButton: zoom out to full size

        setMousePattern(QwtEventPattern::MouseSelect2, Qt::RightButton, Qt::ControlModifier);

        setMousePattern(QwtEventPattern::MouseSelect3, Qt::RightButton);

        setRubberBand(QwtPicker::RectRubberBand);
        setTrackerMode(QwtPicker::AlwaysOn); // ActiveOnly (only when the selection is active), AlwaysOff
    }

    void widgetMouseMoveEvent(QMouseEvent *e) override
    {
        static int lastX = -1;
        static int lastY = -1;
        int        dx    = e->x() - lastX;
        int        dy    = e->y() - lastY;

        lastX = e->x();
        lastY = e->y();

        // if (e->button() == Qt::MidButton) fkt bei move nicht
        if(e->buttons() == Qt::MiddleButton)
        {
            plot()->setAutoReplot(false);
            for(int axis = 0; axis < QwtPlot::axisCnt; axis++)
            {
                if(axis == QwtPlot::xBottom || axis == QwtPlot::yLeft)
                {
                    const QwtScaleMap map = plot()->canvasMap(axis);

                    const int i1 =
                        map.transform(plot()
                                          ->axisScaleDiv(axis)
                                          .lowerBound()); // war in alter qwt version: axisScaleDiv(axis)->lBound()
                    const int i2 =
                        map.transform(plot()
                                          ->axisScaleDiv(axis)
                                          .upperBound()); // war in alter qwt version: axisScaleDiv(axis)->hBound()

                    double d1, d2;
                    if(axis == QwtPlot::xBottom || axis == QwtPlot::xTop)
                    {
                        d1 = map.invTransform(i1 - dx);
                        d2 = map.invTransform(i2 - dx);
                    }
                    else
                    {
                        d1 = map.invTransform(i1 - dy);
                        d2 = map.invTransform(i2 - dy);
                    }

                    if((axis == QwtPlot::xBottom) && (d2 > zoomBase().width()))
                    {
                        d1 = d1 - (d2 - zoomBase().width());
                        d2 = zoomBase().width();
                    }
                    else if((axis == QwtPlot::yLeft) && (d2 > zoomBase().height()))
                    {
                        d1 = d1 - (d2 - zoomBase().height());
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

void TrackerRealPlotItem::draw(QPainter *p, const QwtScaleMap &mapX, const QwtScaleMap &mapY, const QRectF & /*re*/)
    const
{
    Control *controlWidget = ((AnalysePlot *) plot())->getControlWidget();
    if(mTrackerReal && (mTrackerReal->size() > 0) && controlWidget != nullptr)
    {
        QRectF  rect;
        double  sx         = (mapX.p2() - mapX.p1()) / (mapX.s2() - mapX.s1());
        double  sy         = (mapY.p2() - mapY.p1()) / (mapY.s2() - mapY.s1());
        double  circleSize = ((AnalysePlot *) plot())->symbolSize();
        int     i, j;
        QPointF point, lastPoint;

        p->save();
        p->translate(mapX.p1(), mapY.p2()); // in pixelkoordinaten vor skalierung
        p->scale(sx, sy);
        p->translate(-mapX.s1(), -mapY.s2());

        p->setPen(Qt::red);

        rect.setWidth(circleSize / sx);
        rect.setHeight(circleSize / sy);
        sx = circleSize / (2. * sx);
        sy = circleSize / (2. * sy);

        bool anaConsiderX   = controlWidget->isAnaConsiderXChecked();
        bool anaConsiderY   = controlWidget->isAnaConsiderYChecked();
        bool anaConsiderAbs = controlWidget->isAnaConsiderAbsChecked();
        bool anaConsiderRev = controlWidget->isAnaConsiderRevChecked();

        // Beschriftung
        static QFont f("Courier", 10, QFont::Normal);        // Times Helvetica, Normal Bold
        QwtText      titleX("t [frame]", QwtText::RichText); //"x" TeXText
        QwtText      titleY;
        if(anaConsiderX && anaConsiderY)
        {
            titleY.setText("v [m/s]", QwtText::RichText);
        }
        else if(anaConsiderX)
        {
            if(anaConsiderAbs)
            {
                titleY.setText("v<sub>|x|</sub> [m/s]", QwtText::RichText);
            }
            else if(anaConsiderRev)
            {
                titleY.setText("v<sub>-x</sub> [m/s]", QwtText::RichText);
            }
            else
            {
                titleY.setText("v<sub>x</sub> [m/s]", QwtText::RichText);
            }
        }
        else // == if (anaConsiderY)
        {
            if(anaConsiderAbs)
            {
                titleY.setText("v<sub>|y|</sub> [m/s]", QwtText::RichText);
            }
            else if(anaConsiderRev)
            {
                titleY.setText("v<sub>-y</sub> [m/s]", QwtText::RichText);
            }
            else
            {
                titleY.setText("v<sub>y</sub> [m/s]", QwtText::RichText);
            }
        }
        titleX.setFont(f);
        titleY.setFont(f);
        ((AnalysePlot *) plot())->setAxisTitle(QwtPlot::xBottom, titleX); //"x"
        ((AnalysePlot *) plot())->setAxisTitle(QwtPlot::yLeft, titleY);   //"y"

        int             step = controlWidget->getAnaStep(); // 1
        int             frame, animFrame, velVecActIdx = -1;
        double          vel; // geschwindigkeit
        int             largestLastFrame = mTrackerReal->largestLastFrame();
        QVector<int>    velAnzVec(largestLastFrame, 0);
        QVector<double> velVec(largestLastFrame, 0.);
        int             actFrame = ((AnalysePlot *) plot())->getActFrame();
        bool            markAct  = controlWidget->isAnaMarkActChecked();
        double          fps      = controlWidget->getMainWindow()->getAnimation()->getSequenceFPS();
        if(fps < 0)
        {
            fps = DEFAULT_FPS;
        }

        if(!markAct)
        {
            p->setPen(Qt::green);
            p->setBrush(Qt::green);
        }
        for(i = 0; i < mTrackerReal->size(); ++i)
        {
            for(j = 0; j < mTrackerReal->at(i).size() - step; ++j) // -step, damit geschwindigkeit ermittelt werden kann
            {
                frame     = mTrackerReal->at(i).firstFrame() + j;
                animFrame = mTrackerReal->at(i).at(j).frameNum(); // ohne eingefuegte frames bei auslassungen
                if(markAct)
                {
                    if(animFrame == actFrame)
                    {
                        p->setPen(Qt::red);
                        p->setBrush(Qt::red);
                        velVecActIdx = frame;
                    }
                    else // if (frame == actFrame+1)
                    {
                        p->setPen(Qt::green);
                        p->setBrush(Qt::green);
                    }
                }

                if(anaConsiderX && anaConsiderY)
                {
                    vel = (mTrackerReal->at(i).at(j + step).distanceToPoint(mTrackerReal->at(i).at(j)));
                }
                else if(anaConsiderX)
                {
                    if(anaConsiderAbs)
                    {
                        vel = fabs((mTrackerReal->at(i).at(j + step).x() - mTrackerReal->at(i).at(j).x()));
                    }
                    else if(anaConsiderRev)
                    {
                        vel = (mTrackerReal->at(i).at(j).x() - mTrackerReal->at(i).at(j + step).x());
                    }
                    else
                    {
                        vel = (mTrackerReal->at(i).at(j + step).x() - mTrackerReal->at(i).at(j).x());
                    }
                }
                else // == if (anaConsiderY)
                {
                    if(anaConsiderAbs)
                    {
                        vel = fabs((mTrackerReal->at(i).at(j + step).y() - mTrackerReal->at(i).at(j).y()));
                    }
                    else if(anaConsiderRev)
                    {
                        vel = (mTrackerReal->at(i).at(j).y() - mTrackerReal->at(i).at(j + step).y());
                    }
                    else
                    {
                        vel = (mTrackerReal->at(i).at(j + step).y() - mTrackerReal->at(i).at(j).y());
                    }
                }
                vel /= ((100. / fps) * step); // m/s, war: 100cm/25frames =4 => vel /=(4.*step);

                point.setX(frame);
                point.setY(vel);
                rect.moveLeft(point.x() - sx);
                rect.moveTop(point.y() - sy);
                p->drawEllipse(rect);

                // j - j+step, da gegen die x-achse gelaufen wird

                ++velAnzVec[frame];
                velVec[frame] += vel;
            }
        }

        rect.setWidth(2 * rect.width());
        rect.setHeight(2 * rect.height());
        p->setPen(Qt::blue);
        p->setBrush(Qt::blue);
        for(i = 0; i < velAnzVec.size(); ++i)
        {
            if(velAnzVec[i] != 0)
            {
                point.setX(i);
                point.setY(velVec[i] / velAnzVec[i]);
                if((i != 0) && (velAnzVec[i - 1] != 0)) // nicht ganz hundertprozentig
                {
                    p->drawLine(lastPoint, point);
                }
                lastPoint = point;
                if(markAct && (i == velVecActIdx))
                {
                    rect.moveLeft(point.x() - 2 * sx);
                    rect.moveTop(point.y() - 2 * sy);
                    p->drawEllipse(rect);
                }
            }
        }

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
    :
    QwtPlot(parent)
{
    setAutoReplot(false);
    setCanvasBackground(QColor(QColor(220, 220, 255)));

    mControlWidget = nullptr;
    mSymbolSize    = 3.;
    mActFrame      = 0;

    // default in controlWidget - ansonsten wird es in plotitem geaendert
    QFont   f("Courier", 10, QFont::Normal);                  // Times Helvetica, Normal Bold
    QwtText titleX("t [frame]", QwtText::RichText);           //"x" TeXText
    QwtText titleY("v<sub>y</sub> [m/s]", QwtText::RichText); //"y"
    titleX.setFont(f);
    titleY.setFont(f);

    setAxisTitle(xBottom, titleX); //"x"
    setAxisTitle(yLeft, titleY);   //"y"

    plotLayout()->setAlignCanvasToScales(true);

    mZoomer = new AnalyseZoomer(xBottom, yLeft, canvas()); // QwtPlotZoomer

    mTrackerRealItem = new TrackerRealPlotItem();
    mTrackerRealItem->setZ(1);
    mTrackerRealItem->attach(this);

    // grid
    QwtPlotGrid *grid = new QwtPlotGrid;
    grid->enableXMin(true);
    grid->enableYMin(true);
    grid->setMajorPen(QPen(QColor(130, 130, 130), 0, Qt::DotLine));
    grid->setMinorPen(QPen(QColor(170, 170, 170), 0, Qt::DotLine));
    grid->setZ(0);
    grid->attach(this);
}

QPoint AnalysePlot::getPos(const QColor &col) const
{
    QPoint p;

    if(mControlWidget)
    {
        int x    = mControlWidget->getRecoColorX();
        int y    = mControlWidget->getRecoColorY();
        int ymax = (int) yMax();

        if(mControlWidget->getRecoColorModel() == 0) // HSV
        {
            if(x == 0) // nicht setX und setY, weil das width und height anpasst
            {
                p.setX(col.hue());
            }
            else if(x == 1)
            {
                p.setX(col.saturation());
            }
            else
            {
                p.setX(col.value());
            }
            if(y == 0)
            {
                p.setY(ymax - col.hue());
            }
            else if(y == 1)
            {
                p.setY(ymax - col.saturation());
            }
            else
            {
                p.setY(ymax - col.value());
            }
        }
        else // RGB
        {
            if(x == 0)
            {
                p.setX(col.red());
            }
            else if(x == 1)
            {
                p.setX(col.green());
            }
            else
            {
                p.setX(col.blue());
            }
            if(y == 0)
            {
                p.setY(ymax - col.red());
            }
            else if(y == 1)
            {
                p.setY(ymax - col.green());
            }
            else
            {
                p.setY(ymax - col.blue());
            }
        }
    }

    return p;
}

void AnalysePlot::setTrackerReal(TrackerReal *trackerReal)
{
    mTrackerReal = trackerReal;
    mTrackerRealItem->setTrackerReal(trackerReal);
}

void AnalysePlot::setScale()
{
    if(mControlWidget)
    {
        int frameNum =
            mTrackerReal->largestLastFrame(); // mControlWidget->getMainWindow()->getAnimation()->getNumFrames();
        mXMin = mTrackerReal->smallestFirstFrame() - 10; // 0-spacerX;
        mXMax = frameNum + 10;                           // frameNum+spacerX;
        mYMin = -0.2;                                    // 0-spacerY;
        mYMax = 2.;                                      // 2+spacerY;

        setAxisScale(QwtPlot::xBottom, mXMin, mXMax);
        setAxisScale(QwtPlot::yLeft, mYMin, mYMax);
        replot();
        mZoomer->setZoomBase(QRectF(mXMin, mYMin, mXMax - mXMin, mYMax - mYMin));
    }
}

#include "moc_analysePlot.cpp"
