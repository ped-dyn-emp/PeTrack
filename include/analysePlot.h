#ifndef ANALYSEPLOT_H
#define ANALYSEPLOT_H

//#ifdef QWT // kann nicht nur in .pro ausgeschlossen werden, da auch von ui benutzt wird

#include <QPen>

#include <qwt_plot.h>
#include <qwt_plot_zoomer.h>
#include <qwt_plot_item.h>

#include "helper.h"
#include "tracker.h"
#include "trackerReal.h"

class Control;

//-----------------------------------------------------------------

class TrackerRealPlotItem: public QwtPlotItem
{
public:
    TrackerRealPlotItem();

    void draw(QPainter* p, const QwtScaleMap& mapX, const QwtScaleMap& mapY, const QRectF& re) const;

    void setPen(const QPen &pen);

//     void setModel(int model, int x, int y);

    void setTrackerReal(TrackerReal* trackerReal);

private:
    TrackerReal *mTrackerReal;
//     int mModel, mX, mY;
    QPen mPen;
};

//-----------------------------------------------------------------------------------------

class AnalysePlot: public QwtPlot
{
    Q_OBJECT

public:
    AnalysePlot(QWidget *parent = NULL);

    void setCursor(const QColor &col);

    QPoint getPos(const QColor &col) const;

//     double map(const QColor &col) const;

    inline void setControlWidget(Control *control)
    {
        mControlWidget = control;
    }
    inline Control * getControlWidget() const
    {
        return mControlWidget;
    }
    inline void setActFrame(int f)
    {
        mActFrame = f;
    }
    inline int getActFrame() const
    {
        return mActFrame;
    }
    void setTrackerReal(TrackerReal* TR);
    void setScale();
//     void updateTracker();

    inline double symbolSize() const
    {
        return mSymbolSize;
    }
    inline void setSymbolSize(double s)
    {
        mSymbolSize = s;
    }
    QwtPlotZoomer * getZoomer()
    {
        return mZoomer;
    }

    inline double xMax() const
    {
        return mXMax;
    }
    inline double yMax() const
    {
        return mYMax;
    }

private:
    double mSymbolSize;
    double mXMin, mXMax, mYMin, mYMax;
    Control *mControlWidget;
    TrackerReal *mTrackerReal;
//     ImagePlotItem *mImageItem;
    TrackerRealPlotItem *mTrackerRealItem;
//     RectPlotItem *mRectItem;
//     ViewAnalysePlotItem *mViewAnalyseItem;
    QwtPlotZoomer *mZoomer;
    int mActFrame;
};

//#else // QWT nicht verfuegbar
//class AnalysePlot: public QWidget
//{
//    Q_OBJECT
//}
//#endif

#endif
