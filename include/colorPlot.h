#ifndef COLORPLOT_H
#define COLORPLOT_H

#include <QPen>

#include <qwt_plot.h>

#include "helper.h"
#include "tracker.h"

#define DEFAULT_HEIGHT 180.0

class ImagePlotItem;
class TrackerPlotItem;
class Control;
class Zoomer;
class RectPlotItem;
class ViewColorPlotItem;

//-----------------------------------------------------------------

class TrackerPlotItem: public QwtPlotItem
{
public:
    TrackerPlotItem();

    void draw(QPainter* p, const QwtScaleMap& mapX, const QwtScaleMap& mapY, const QRectF& re) const;

    void setPen(const QPen &pen);

    void setModel(int model, int x, int y);

    void setTracker(Tracker *tracker);
    Tracker * getTracker();

private:
    Tracker *mTracker;
//     int mModel, mX, mY;
    QPen mPen;
};

//-----------------------------------------------------------------

class RectMap: public QRectF
{
public:
    RectMap()
        : mColored(true), mMapHeight(DEFAULT_HEIGHT), mInversHue(false)
    {
        setRect(0., 0., 0., 0.);
        mFromCol = mFromCol.toHsv();
        mToCol = mToCol.toHsv();
    }
    RectMap(QRectF r)
        : QRectF(r), mInversHue(false)
    {
        setRect(0., 0., 0., 0.);
        mFromCol = mFromCol.toHsv();
        mToCol = mToCol.toHsv();
    }
    RectMap(double x, double y, double w, double h, bool colored, double mapHeight)
        : QRectF(x, y, w, h), mColored(colored), mMapHeight(mapHeight), mInversHue(false)
    {
        mFromCol = mFromCol.toHsv();
        mToCol =mToCol.toHsv();
    }
    RectMap(QRectF r, bool colored, double mapHeight)
        : QRectF(r), mColored(colored), mMapHeight(mapHeight), mInversHue(false)
    {
        mFromCol = mFromCol.toHsv();
        mToCol = mToCol.toHsv();
    }
    inline bool colored() const
    {
        return mColored;
    }
    inline void setColored(bool b)
    {
        mColored = b;
    }
    inline double mapHeight() const
    {
        return mMapHeight;
    }
    inline void setMapHeight(double h)
    {
        mMapHeight = h;
    }
    inline void setInvHue(bool b)
    {
        mInversHue = b;
    }
    inline void setFromColor(const QColor &fromCol)
    {
        mFromCol = fromCol;
    }
    inline void setToColor(const QColor &toCol)
    {
        mToCol = toCol;
    }
    inline bool invHue() const
    {
        return mInversHue;
    }
    inline QColor & toColor()
    {
        return mToCol;
    }
    inline QColor & fromColor()
    {
        return mFromCol;
    }

    bool mColored;
    double mMapHeight;
    // bool contains ( qreal x, qreal y ) const gibt zurueck, ob pkt x, y in rect liegt
    QColor mFromCol;
    QColor mToCol;
    bool mInversHue;
};

//-----------------------------------------------------------------------------------------

class RectPlotItem: public QwtPlotItem
{
public:
    RectPlotItem();

    double map(const QColor &col) const; //TrackPerson &tp RectMap ... double x, double y

    int addMap(double x, double y, double w, double h, bool colored, double height, QColor &fromCol, QColor &toCol, bool invHue);
    int addMap();
    void changeMap(int index, double x, double y, double w, double h, bool colored, double mapHeight);
    void changeActMapInvHue(bool b);
    void changeActMapFromColor(const QColor &fromCol);
    void changeActMapToColor(const QColor &toCol);
    bool getActMapInvHue();
    QColor getActMapToColor();
    QColor getActMapFromColor();
    RectMap getMap(int index) const;
    void delMap(int index);
    inline void delMaps()
    {
        mMaps.clear();
    }
    inline int mapNum() const
    {
        return mMaps.size();
    }

    void draw(QPainter* p, const QwtScaleMap& mapX, const QwtScaleMap& mapY, const QRectF& re) const;

    void setPen(const QPen &pen);

private:
    QList<RectMap> mMaps;
    QPen mPen;
    int mActIndex;
};

//-----------------------------------------------------------------

class ColorPlot: public QwtPlot
{
    Q_OBJECT

public:
    ColorPlot(QWidget *parent = NULL);

    void replot();

    void setCursor(const QColor &col);

    QPoint getPos(const QColor &col, int *z=NULL) const;

    bool isGrey(const QColor &col) const;

    double map(const QColor &col) const;
    bool printDistribution() const;

    void setControlWidget(Control *control);
    void setTracker(Tracker *tracker);
    void setScale();
//     void updateTracker();
    void generateImage();

    inline double symbolSize() const
    {
        return mSymbolSize;
    }
    inline void setSymbolSize(double s)
    {
        mSymbolSize = s;
    }

    inline int greyDiff() const
    {
        return mGreyDiff;
    }
    inline void setGreyDiff(int s)
    {
        mGreyDiff = s;
    }
    inline double xMax() const
    {
        return mXMax;
    }
    inline double yMax() const
    {
        return mYMax;
    }
    int zValue() const;

    inline TrackerPlotItem *getTrackerItem() const
    {
        return mTrackerItem;
    }
    inline RectPlotItem *getMapItem() const
    {
        return mRectItem;
    }
//     inline ImagePlotItem *getImageItem() const
//     {
//         return mImageItem;
//     }

private:
    double mSymbolSize;
    double mXMax;
    double mYMax;
    Control *mControlWidget;
    ImagePlotItem *mImageItem;
    TrackerPlotItem *mTrackerItem;
    RectPlotItem *mRectItem;
    ViewColorPlotItem *mViewColorItem;
    Zoomer *mZoomer;
    int mGreyDiff;
//     QwtPlotCurve *mTpc;
//     Tracker *mTracker;

// public slots:
//     void showContour(bool on);
//     void showSpectrogram(bool on);

// private:
//     QwtPlotSpectrogram *d_spectrogram;
};

#endif
