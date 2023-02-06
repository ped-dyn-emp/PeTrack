/*
 * PeTrack - Software for tracking pedestrians movement in videos
 * Copyright (C) 2023 Forschungszentrum Jülich GmbH, IAS-7
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

#ifndef ANALYSEPLOT_H
#define ANALYSEPLOT_H

#include "helper.h"
#include "tracker.h"
#include "trackerReal.h"

#include <QPen>
#include <qwt_plot.h>
#include <qwt_plot_item.h>
#include <qwt_plot_zoomer.h>

class Control;

class TrackerRealPlotItem : public QwtPlotItem
{
public:
    TrackerRealPlotItem();

    void draw(QPainter *p, const QwtScaleMap &mapX, const QwtScaleMap &mapY, const QRectF &re) const;

    void setPen(const QPen &pen);

    void setTrackerReal(TrackerReal *trackerReal);

private:
    TrackerReal *mTrackerReal;
    QPen         mPen;
};


class AnalysePlot : public QwtPlot
{
    Q_OBJECT

public:
    AnalysePlot(QWidget *parent = nullptr);

    QPoint getPos(const QColor &col) const;

    inline void     setControlWidget(Control *control) { mControlWidget = control; }
    inline Control *getControlWidget() const { return mControlWidget; }
    inline void     setActFrame(int f) { mActFrame = f; }
    inline int      getActFrame() const { return mActFrame; }
    void            setTrackerReal(TrackerReal *TR);
    void            setScale();

    inline double  symbolSize() const { return mSymbolSize; }
    inline void    setSymbolSize(double s) { mSymbolSize = s; }
    QwtPlotZoomer *getZoomer() { return mZoomer; }

    inline double xMax() const { return mXMax; }
    inline double yMax() const { return mYMax; }

private:
    double               mSymbolSize;
    double               mXMin, mXMax, mYMin, mYMax;
    Control             *mControlWidget;
    TrackerReal         *mTrackerReal;
    TrackerRealPlotItem *mTrackerRealItem;
    QwtPlotZoomer       *mZoomer;
    int                  mActFrame;
};

#endif
