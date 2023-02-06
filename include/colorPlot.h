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

#ifndef COLORPLOT_H
#define COLORPLOT_H

#include "helper.h"

#include <QPen>
#include <qwt_plot.h>

inline constexpr double DEFAULT_HEIGHT = 180.0;

class ImagePlotItem;
class Tracker;
class TrackerPlotItem;
class Control;
class Zoomer;
class RectPlotItem;
class PersonStorage;
class ViewColorPlotItem;


class TrackerPlotItem : public QwtPlotItem
{
public:
    void draw(QPainter *p, const QwtScaleMap &mapX, const QwtScaleMap &mapY, const QRectF &re) const;

    void setPen(const QPen &pen);

    void setModel(int model, int x, int y);

    void setPersonStorage(const PersonStorage *storage);

private:
    const PersonStorage *mPersonStorage = nullptr;
    QPen                 mPen;
};


class RectMap : public QRectF
{
public:
    RectMap() : mColored(true), mMapHeight(DEFAULT_HEIGHT), mInversHue(false)
    {
        setRect(0., 0., 0., 0.);
        mFromCol = QColor::fromHsv(0, 0, 0);
        mToCol   = QColor::fromHsv(359, 255, 255);
    }
    RectMap(QRectF r) : QRectF(r), mInversHue(false)
    {
        setRect(0., 0., 0., 0.);
        mFromCol = QColor::fromHsv(0, 0, 0);
        mToCol   = QColor::fromHsv(359, 255, 255);
    }
    RectMap(double x, double y, double w, double h, bool colored, double mapHeight) :
        QRectF(x, y, w, h), mColored(colored), mMapHeight(mapHeight), mInversHue(false)
    {
        mFromCol = QColor::fromHsv(0, 0, 0);
        mToCol   = QColor::fromHsv(359, 255, 255);
    }
    RectMap(QRectF r, bool colored, double mapHeight) :
        QRectF(r), mColored(colored), mMapHeight(mapHeight), mInversHue(false)
    {
        mFromCol = QColor::fromHsv(0, 0, 0);
        mToCol   = QColor::fromHsv(359, 255, 255);
    }
    inline bool    colored() const { return mColored; }
    inline void    setColored(bool b) { mColored = b; }
    inline double  mapHeight() const { return mMapHeight; }
    inline void    setMapHeight(double height) { mMapHeight = height; }
    inline void    setInvHue(bool b) { mInversHue = b; }
    inline void    setFromColor(const QColor &fromCol) { mFromCol = fromCol; }
    inline void    setToColor(const QColor &toCol) { mToCol = toCol; }
    inline bool    invHue() const { return mInversHue; }
    inline QColor &toColor() { return mToCol; }
    inline QColor &fromColor() { return mFromCol; }

    bool   mColored;
    double mMapHeight;
    QColor mFromCol;
    QColor mToCol;
    bool   mInversHue;
};


class RectPlotItem : public QwtPlotItem
{
public:
    RectPlotItem();

    double map(const QColor &col) const; // TrackPerson &tp RectMap ... double x, double y

    int addMap(
        double  x,
        double  y,
        double  w,
        double  h,
        bool    colored,
        double  height,
        QColor &fromCol,
        QColor &toCol,
        bool    invHue);
    int         addMap();
    void        changeMap(int index, double x, double y, double w, double h, bool colored, double mapHeight);
    void        changeActMapInvHue(bool b);
    void        changeActMapFromColor(const QColor &fromCol);
    void        changeActMapToColor(const QColor &toCol);
    bool        getActMapInvHue();
    QColor      getActMapToColor();
    QColor      getActMapFromColor();
    RectMap     getMap(int index) const;
    void        delMap(int index);
    inline void delMaps() { mMaps.clear(); }
    inline int  mapNum() const { return mMaps.size(); }

    void draw(QPainter *p, const QwtScaleMap &mapX, const QwtScaleMap &mapY, const QRectF &re) const;

    void setPen(const QPen &pen);

private:
    QList<RectMap> mMaps;
    QPen           mPen;
    int            mActIndex;
};


class ColorPlot : public QwtPlot
{
    Q_OBJECT

public:
    ColorPlot(QWidget *parent = nullptr);

    void replot();

    void setCursor(const QColor &col);

    QPoint getPos(const QColor &col, int *z = nullptr) const;

    bool isGrey(const QColor &col) const;

    double map(const QColor &col) const;
    bool   printDistribution() const;

    void setControlWidget(Control *control);
    void setPersonStorage(const PersonStorage *storage);
    void setScale();
    void generateImage();

    inline double symbolSize() const { return mSymbolSize; }
    inline void   setSymbolSize(double s) { mSymbolSize = s; }

    inline int    greyDiff() const { return mGreyDiff; }
    inline void   setGreyDiff(int s) { mGreyDiff = s; }
    inline double xMax() const { return mXMax; }
    inline double yMax() const { return mYMax; }
    int           zValue() const;

    inline TrackerPlotItem *getTrackerItem() const { return mTrackerItem; }
    inline RectPlotItem    *getMapItem() const { return mRectItem; }

private:
    const PersonStorage *mPersonStorage;
    double               mSymbolSize;
    double               mXMax;
    double               mYMax;
    Control             *mControlWidget;
    ImagePlotItem       *mImageItem;
    TrackerPlotItem     *mTrackerItem;
    RectPlotItem        *mRectItem;
    ViewColorPlotItem   *mViewColorItem;
    Zoomer              *mZoomer;
    int                  mGreyDiff;
};

#endif
