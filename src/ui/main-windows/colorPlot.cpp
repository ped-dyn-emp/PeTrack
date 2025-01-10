/*
 * PeTrack - Software for tracking pedestrians movement in videos
 * Copyright (C) 2025 Forschungszentrum Jülich GmbH, IAS-7
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

#include "colorPlot.h"

#include "control.h"
#include "logger.h"
#include "personStorage.h"
#include "tracker.h"

#include <QMouseEvent>
#include <QPainter>
#include <iomanip>
#include <qwt_plot_layout.h>
#include <qwt_plot_zoomer.h>
#include <qwt_scale_engine.h>
#include <qwt_scale_map.h>
#include <qwt_symbol.h>
#include <qwt_text.h>

class ImagePlotItem : public QwtPlotItem
{
public:
    ImagePlotItem() { mImage = new QImage(360, 360, QImage::Format_RGB32); }

    /**
     * @brief Draws the mImage at the right position in correct size.
     * @param p
     * @param mx
     * @param my
     */
    void draw(QPainter *p, const QwtScaleMap &mx, const QwtScaleMap &my, const QRectF & /*r*/) const
    {
        p->save();
        p->scale(mx.p2() / (mx.s2() - mx.s1()), my.p1() / (my.s2() - my.s1())); //
        p->translate(-mx.s1(), my.s2() - ((ColorPlot *) plot())->yMax());
        p->drawImage(0, 0, *mImage);
        p->restore();
    }

    /**
     * @brief Generates the color background or the color plot
     *
     * If x == y, the minimal values for both are used and the other two
     * dimenions get set with the value of z.
     *
     * @param model
     * @param x First dimension (dimension for x-values) RGB or HSV (e.g. Hue, Value, Red)
     * @param y Second dimension (dimension for y-values) RGB or HSV (e.g. Hue, Value, Red)
     * @param z Value for third dimension (if x == y for 2nd and 3rd)
     */
    void generateImage(int model, int x, int y, int z)
    {
        QColor col;
        int    i, j;

        if(model == 0) // HSV
        {
            for(i = 0; i < (x == 0 ? 360 : 256); ++i)
            {
                for(j = 0; j < (y == 0 ? 360 : 256); ++j)
                {
                    col.setHsv(
                        x == 0 ? (y == 0 ? std::min(i, j) : i) : (y == 0 ? j : myRound(z * 360. / 256.)),
                        x == 1 ? (y == 1 ? std::min(i, j) : i) : (y == 1 ? j : z),
                        x == 2 ? (y == 2 ? std::min(i, j) : i) : (y == 2 ? j : z));
                    mImage->setPixel(i, (y == 0 ? 359 : 255) - j, col.rgb());
                }
            }
        }
        else // RGB
        {
            for(i = 0; i < 256; ++i)
            {
                for(j = 0; j < 256; ++j)
                {
                    col.setRgb(
                        x == 0 ? (y == 0 ? std::min(i, j) : i) : (y == 0 ? j : z),
                        x == 1 ? (y == 1 ? std::min(i, j) : i) : (y == 1 ? j : z),
                        x == 2 ? (y == 2 ? std::min(i, j) : i) : (y == 2 ? j : z));
                    mImage->setPixel(i, 255 - j, col.rgb());
                }
            }
        }
    }

    inline QImage *getImage() const { return mImage; }

private:
    QImage *mImage;
};


//-----------------------------------------------------------------------------------------

// die kreise liegen mgl nicht genau auf kreuz - noch zu testen

/**
 * @brief Draws a circle in the colorplot for every color associated with a trackperson.
 *
 * @param p
 * @param mapX
 * @param mapY
 */
void TrackerPlotItem::draw(QPainter *p, const QwtScaleMap &mapX, const QwtScaleMap &mapY, const QRectF & /*re*/) const
{
    QRectF rect;
    double sx         = mapX.p2() / (mapX.s2() - mapX.s1());
    double sy         = mapY.p1() / (mapY.s2() - mapY.s1());
    double yMax       = ((ColorPlot *) plot())->yMax();
    double circleSize = ((ColorPlot *) plot())->symbolSize();
    int    z;
    int    plotZ = ((ColorPlot *) plot())->zValue();
    double diff;

    // TODO ganz leicht verschiebung: eigentlich muessten noch andere werte wie diese einfliessen:
    //         p->scale((mx.p2() - mx.p1())/(mx.s2() - mx.s1()), (my.p1() - my.p2())/(my.s2() - my.s1()));//
    // mx.p1()-mx.s1(), my.s2()+my.p2()-((ColorPlot *) plot())->yMax()

    p->save();
    p->scale(sx, sy);
    p->translate(-mapX.s1(), mapY.s2() - yMax);

    sx = circleSize / sx;
    sy = circleSize / sy;

    if(mPersonStorage)
    {
        const auto &persons = mPersonStorage->getPersons();
        for(const auto &person : persons)
        {
            if(person.color().isValid()) // insbesondere von hand eingefuegte trackpoint/persons haben keine farbe
            {
                QPoint point = ((ColorPlot *) plot())->getPos(person.color(), &z);
                diff         = (255. - abs(z - plotZ)) / 255.;
                rect.setWidth(diff * sx);
                rect.setHeight(diff * sy);
                rect.moveLeft(point.x() - diff * sx / 2.);
                rect.moveTop(point.y() - diff * sy / 2.);

                p->setBrush(QBrush(person.color()));
                if(((ColorPlot *) plot())->isGrey(person.color()))
                {
                    p->setPen(Qt::red);
                }
                else
                {
                    p->setPen(mPen);
                }

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

void TrackerPlotItem::setPersonStorage(const PersonStorage *storage)
{
    mPersonStorage = storage;
}


//-----------------------------------------------------------------------------------------

RectPlotItem::RectPlotItem() : mActIndex(-1) {}

/// auch wenn punkt auf linie liegt, wird er als drinnen gezaehlt!!!!
/// es wird einfach der erste treffer in map-list genommen - unterscheidung zwischen farbe und graustufe
/// wird keine farbige map fuer farbige col gefunden, wird in grauen map der erste treffer genommen und
/// andersherum: wird fuer graue col keine graue map gefunden, wird erste farbige map zurueckgegeben
/// und hoehe zurueckgegeben
double RectPlotItem::map(const QColor &col) const // const TrackPerson &tp RectMap ... double x, double y
{
    double  yMax = ((ColorPlot *) plot())->yMax();
    RectMap map;
    bool    isGrey;
    bool    contains;
    double  fallback = -1; // -1 soll anzeigen, dass nichts gefunden wurde

    for(int i = 0; i < mMaps.size(); ++i)
    {
        map = mMaps[i];
        map.moveTop(yMax - map.y() - map.height()); // nicht setY!!!!!, da dann height angepast wird
        isGrey = ((ColorPlot *) plot())->isGrey(col);
        if(map.invHue())
        {
            // Split inversHue Map into 2 maps, one for the left side and one for the right side
            QRectF leftMap(0, 0, map.x(), map.height()),
                rightMap(map.x() + map.width(), 0, 360 - map.x() - map.width(), map.height());
            contains = leftMap.contains(((ColorPlot *) plot())->getPos(col)) ||
                       rightMap.contains(((ColorPlot *) plot())->getPos(col));
        }
        else
        {
            contains = map.contains(((ColorPlot *) plot())->getPos(col));
        }
        if(!isGrey && map.colored() && contains)
        {
            return map.mapHeight();
        }
        else if(isGrey && !map.colored() && contains)
        {
            return map.mapHeight();
        }
        else if(!isGrey && !map.colored() && contains && fallback == -1) // fallback == -1 damit der erste gefundene
                                                                         // genommen wird
        {
            fallback = map.mapHeight();
        }
        else if(isGrey && map.colored() && contains && fallback == -1)
        {
            fallback = map.mapHeight();
        }
    }
    return fallback; // -1 soll anzeigen, dass nichts gefunden wurde
}

int RectPlotItem::addMap(
    double  x,
    double  y,
    double  w,
    double  h,
    bool    colored,
    double  height,
    QColor &fromCol,
    QColor &toCol,
    bool    invHue)
{
    mMaps.append(RectMap(x, y, w, h, colored, height));
    mMaps.last().setFromColor(fromCol);
    mMaps.last().setToColor(toCol);
    mMaps.last().setInvHue(invHue);
    if(mActIndex < 0)
    {
        mActIndex = 0;
    }
    return mMaps.size() - 1;
}
int RectPlotItem::addMap()
{
    if(mMaps.size() > 0)
    {
        mMaps.append(mMaps.last());
    }
    else
    {
        mMaps.append(RectMap(0, 0, 0, 0, true, DEFAULT_HEIGHT));
    }
    if(mActIndex < 0)
    {
        mActIndex = 0;
    }
    return mMaps.size() - 1;
}

void RectPlotItem::delMap(int index)
{
    if(mMaps.size() == 1)
    {
        if(index != 0)
        {
            SPDLOG_INFO("Invalid index for map deletion!");
            return;
        }
        mMaps.replace(0, RectMap());
        return;
    }

    if(index >= 0 && index < mMaps.size() && mMaps.size() > 0)
    {
        mMaps.removeAt(index);
        return;
    }

    SPDLOG_INFO("Invalid index for map deletion!");
    return;
}

void RectPlotItem::changeMap(int index, double x, double y, double w, double h, bool colored, double mapHeight)
{
    if(index >= 0 && index < mMaps.size())
    {
        mMaps[index].setRect(x, y, w, h); // wenn
        mMaps[index].setColored(colored);
        mMaps[index].setMapHeight(mapHeight);
        mActIndex = index;
    }
}

void RectPlotItem::changeActMapInvHue(bool b)
{
    if(mActIndex >= 0 && mActIndex < mMaps.size())
    {
        mMaps[mActIndex].setInvHue(b);
    }
}

void RectPlotItem::changeActMapFromColor(const QColor &fromCol)
{
    if(mActIndex >= 0 && mActIndex < mMaps.size())
    {
        mMaps[mActIndex].setFromColor(fromCol);
    }
}

void RectPlotItem::changeActMapToColor(const QColor &toCol)
{
    if(mActIndex >= 0 && mActIndex < mMaps.size())
    {
        mMaps[mActIndex].setToColor(toCol);
    }
}

bool RectPlotItem::getActMapInvHue()
{
    if(mActIndex >= 0 && mActIndex < mMaps.size())
    {
        return mMaps[mActIndex].invHue();
    }
    return false; // Dependent on other fail checks
}

QColor RectPlotItem::getActMapToColor()
{
    if(mActIndex >= 0 && mActIndex < mMaps.size())
    {
        return mMaps[mActIndex].toColor();
    }
    return QColor::Invalid;
}

QColor RectPlotItem::getActMapFromColor()
{
    if(mActIndex >= 0 && mActIndex < mMaps.size())
    {
        return mMaps[mActIndex].fromColor();
    }
    return QColor::Invalid;
}

RectMap RectPlotItem::getMap(int index) const
{
    if(index >= 0 && index < mMaps.size())
    {
        return mMaps[index];
    }
    else
    {
        return RectMap();
    }
}

/**
 * @brief Draws rectangles representing the color maps into the color plot.
 *
 * The currently active/selected map is drawn green, the rest red.
 *
 * @param p
 * @param mapX
 * @param mapY
 */
void RectPlotItem::draw(QPainter *p, const QwtScaleMap &mapX, const QwtScaleMap &mapY, const QRectF & /*re*/) const
{
    QRectF rect;
    double sx   = mapX.p2() / (mapX.s2() - mapX.s1());
    double sy   = mapY.p1() / (mapY.s2() - mapY.s1());
    double xMax = ((ColorPlot *) plot())->xMax();
    double yMax = ((ColorPlot *) plot())->yMax();
    int    i;
    float  rX, rW;

    // TODO ganz leicht verschiebung: eigentlich muessten noch andere werte wie diese einfliessen:
    //         p->scale((mx.p2() - mx.p1())/(mx.s2() - mx.s1()), (my.p1() - my.p2())/(my.s2() - my.s1()));//
    // mx.p1()-mx.s1(), my.s2()+my.p2()-((ColorPlot *) plot())->yMax()

    p->save();
    p->scale(sx, sy);
    p->translate(-mapX.s1(), mapY.s2() - yMax);

    for(i = 0; i < mMaps.size(); ++i)
    {
        if(i == mActIndex)
        {
            p->setPen(Qt::green);
        }
        else if(!mMaps[i].colored())
        {
            p->setPen(Qt::red);
        }
        else
        {
            p->setPen(mPen);
        }

        if(mMaps[i].invHue())
        {
            rect = mMaps[i];
            rect.moveTop(yMax - rect.y() - rect.height()); // nicht setY!!!!!, da dann height angepast wird
            rX = rect.x();
            rW = rect.width();
            rect.setX(0);
            rect.setWidth(rX);
            p->drawRect(rect);
            rect.setX(rX + rW);
            rect.setWidth(xMax - rX - rW);
            p->drawRect(rect);
        }
        else
        {
            rect = mMaps[i];
            rect.moveTop(yMax - rect.y() - rect.height()); // nicht setY!!!!!, da dann height angepast wird
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
class Zoomer : public QwtPlotZoomer
{
public:
    Zoomer(int xAxis, int yAxis, QWidget *canvas) : QwtPlotZoomer(xAxis, yAxis, canvas)
    {
        // RightButton: zoom out by 1
        // Ctrl+RightButton: zoom out to full size

        setMousePattern(QwtEventPattern::MouseSelect2, Qt::RightButton, Qt::ControlModifier);

        setMousePattern(QwtEventPattern::MouseSelect3, Qt::RightButton);

        setRubberBand(QwtPicker::RectRubberBand);
        setTrackerMode(QwtPicker::AlwaysOn); // ActiveOnly (only when the selection is active), AlwaysOff
    }

    // ueberschrieben, da so kommazahlen unterdrueckt werden, da intervall mindestens 5 umfasst
    QSizeF minZoomSize() const override { return QSizeF(5., 5.); }

    /**
     * @brief Allows movement in the zoomed in color plot via dragging with a pressed down middle mouse button.
     * @param e
     */
    void widgetMouseMoveEvent(QMouseEvent *e) override
    {
        static int lastX = -1;
        static int lastY = -1;
        int        dx    = e->x() - lastX;
        int        dy    = e->y() - lastY;

        lastX = e->x();
        lastY = e->y();

        if(e->buttons() == Qt::MiddleButton)
        {
            plot()->setAutoReplot(false);
            for(int axis = 0; axis < QwtPlot::axisCnt; axis++)
            {
                if(axis == QwtPlot::xBottom || axis == QwtPlot::yLeft)
                {
                    const QwtScaleMap map = plot()->canvasMap(axis);

                    const int i1 = map.transform(plot()->axisScaleDiv(axis).lowerBound());
                    const int i2 = map.transform(plot()->axisScaleDiv(axis).upperBound());

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

                    if(d1 < 0)
                    {
                        d2 = d2 - d1;
                        d1 = 0;
                    }
                    else if((axis == QwtPlot::xBottom) && (d2 > zoomBase().width()))
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


    QwtText trackerText(const QPoint &pos) const override
    {
        QString text;
        text = QString::asprintf("%d, %d", myRound(pos.x()), myRound(pos.y()));
        return QwtText(text);
    }

    QwtText trackerTextF(const QPointF &pos) const override
    {
        QString text;
        text = QString::asprintf("%d, %d", myRound(pos.x()), myRound(pos.y()));
        return QwtText(text);
    }
};

//-----------------------------------------------------------------------------------------

class ViewColorPlotItem : public QwtPlotItem
{
public:
    ViewColorPlotItem() {}

    /**
     * @brief Marks the point in color plot with the color under the cursor in the image.
     *
     * This function draws a little x at the place in the colorplot corresponding
     * to the color the image/video frame has under the cursor of the user.
     *
     * @param p
     * @param mapX
     * @param mapY
     */
    void draw(QPainter *p, const QwtScaleMap &mapX, const QwtScaleMap &mapY, const QRectF & /*re*/) const
    {
        double sx   = mapX.p2() / (mapX.s2() - mapX.s1());
        double sy   = mapY.p1() / (mapY.s2() - mapY.s1());
        double yMax = ((ColorPlot *) plot())->yMax();
        double sS   = ((ColorPlot *) plot())->symbolSize() * 0.35355339;

        // TODO ganz leicht verschiebung: eigentlich muessten noch andere werte wie diese einfliessen:
        //         p->scale((mx.p2() - mx.p1())/(mx.s2() - mx.s1()), (my.p1() - my.p2())/(my.s2() - my.s1()));//
        // mx.p1()-mx.s1(), my.s2()+my.p2()-((ColorPlot *) plot())->yMax()

        p->save();

        p->scale(sx, sy);
        p->translate(-mapX.s1(), mapY.s2() - yMax);

        p->setPen(mPen);

        QPointF p1, p2;
        QPointF point = mPoint;

        p1.setX(point.x() + sS / sx);
        p1.setY(point.y() + sS / sy);
        p2.setX(point.x() - sS / sx);
        p2.setY(point.y() - sS / sy);
        p->drawLine(p1, p2);
        p1.setY(point.y() - sS / sy);
        p2.setY(point.y() + sS / sy);
        p->drawLine(p1, p2);

        p->restore();
    }

    void setPen(const QPen &pen) { mPen = pen; }

    inline void setPoint(const QPoint &p) { mPoint = p; }

    inline QPoint point() const { return mPoint; }

private:
    QPoint mPoint;
    QPen   mPen;
};

//-----------------------------------------------------------------------------------------

ColorPlot::ColorPlot(QWidget *parent) : QwtPlot(parent)
{
    mControlWidget = nullptr;
    mGreyDiff      = 50;
    mSymbolSize    = 10.;


    QwtText titleX("x");
    QwtText titleY("y");

    setAxisTitle(xBottom, titleX); //"x"
    setAxisTitle(yLeft, titleY);   //"y"
    plotLayout()->setAlignCanvasToScales(true);

    mImageItem = new ImagePlotItem();
    mImageItem->attach(this);

    mZoomer = new Zoomer(xBottom, yLeft, canvas());

    mTrackerItem = new TrackerPlotItem();
    mTrackerItem->attach(this);

    mRectItem = new RectPlotItem();
    mRectItem->attach(this);

    mViewColorItem = new ViewColorPlotItem();
    mViewColorItem->attach(this);
}

void ColorPlot::replot()
{
    QwtPlot::replot();
}

double ColorPlot::map(const QColor &col) const
{
    double height = mRectItem->map(col);
    if(height < 0)
    {
        return mControlWidget->getDefaultHeight();
    }
    else
    {
        return height;
    }
}

// gibt false zurueck, wenn es keine groessenverteilung ueber farbe gab
bool ColorPlot::printDistribution() const
{
    QMap<double, int>                 dict;
    QMap<double, int>::const_iterator j;
    int                               anz = 0;

    for(auto const &person : mPersonStorage->getPersons())
    {
        if(person.color().isValid()) // insbesondere von hand eingefuegte trackpoint/persons haben keine farbe
        {
            ++dict[map(person.color())];
        }
    }
    j = dict.constBegin();
    while(j != dict.constEnd())
    {
        anz += j.value();
        ++j;
    }
    if(anz == 0)
    {
        return false;
    }
    j = dict.constBegin();
    while(j != dict.constEnd())
    {
        SPDLOG_INFO("height: {:5.1f} - number {:3d} ({:4d}%)", j.key(), j.value(), (100. * j.value()) / anz);
        ++j;
    }
    return true;
}

bool ColorPlot::isGrey(const QColor &col) const
{
    if(abs(col.red() - col.green()) < mGreyDiff && abs(col.green() - col.blue()) < mGreyDiff &&
       abs(col.blue() - col.red()) < mGreyDiff)
    {
        return true;
    }
    else
    {
        return false;
    }
}

void ColorPlot::setCursor(const QColor &col)
{
    mViewColorItem->setPoint(getPos(col));
}


/**
 * @brief Calculate position of color in current colormap
 *
 * Calculates the position of color in current colormap, i.e. with the current
 * mapping from x,y to hsv/rgb. E.g. x->H and y->S.
 *
 * ueber z kann die zur plotebene senkrechte kooerdinate zurueckgegeben werden,
 * wenn default NULL nicht zutrifft
 *
 * @param col color for which to determine position
 * @param z coordinate orthogonal to the plot plane
 * @return position of color in current colormap
 */
QPoint ColorPlot::getPos(const QColor &col, int *z) const
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
            if(z != nullptr)
            {
                if(x != 0 && y != 0)
                {
                    *z = col.hue();
                }
                else if(x != 1 && y != 1)
                {
                    *z = col.saturation();
                }
                else
                {
                    *z = col.value();
                }
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
            if(z != nullptr)
            {
                if(x != 0 && y != 0)
                {
                    *z = col.red();
                }
                else if(x != 1 && y != 1)
                {
                    *z = col.green();
                }
                else
                {
                    *z = col.blue();
                }
            }
        }
    }

    return p;
}

void ColorPlot::setControlWidget(Control *control)
{
    mControlWidget = control;
}

void ColorPlot::setPersonStorage(const PersonStorage *storage)
{
    mPersonStorage = storage;
    mTrackerItem->setPersonStorage(storage);
}

void ColorPlot::setScale()
{
    if(mControlWidget)
    {
        int model = mControlWidget->getRecoColorModel();
        int x     = mControlWidget->getRecoColorX();
        int y     = mControlWidget->getRecoColorY();

        QRectF base(0., 0., 255., 255.);

        mXMax = 255.;
        mYMax = 255.;
        if(model == 0) // HSV
        {
            if(x == 0)
            {
                mXMax = 359.;
            }
            if(y == 0)
            {
                mYMax = 359.;
            }
        }

        setAxisScale(QwtPlot::xBottom, 0., mXMax);
        setAxisScale(QwtPlot::yLeft, 0., mYMax);

        replot(); // why, see:
                  // file:///C:/Programme/qwt-5.0.1/doc/html/class_qwt_plot_zoomer.html#7a1711597f441223efdb7d9931fe19b9

        base.setWidth(mXMax);
        base.setHeight(mYMax);
        mZoomer->setZoomBase(base);
    }
}

void ColorPlot::generateImage()
{
    if(mControlWidget)
    {
        int model = mControlWidget->getRecoColorModel();
        int x     = mControlWidget->getRecoColorX();
        int y     = mControlWidget->getRecoColorY();
        int z     = mControlWidget->getRecoColorZ();

        mImageItem->generateImage(model, x, y, z);

        // farbe anpassen, damit besser auf bild zu sehen
        // 255 immer genommen, da einfacher und es nicht so genau drauf ankommt
        int midValue = (QColor(mImageItem->getImage()->pixel(0, 0)).value() +
                        QColor(mImageItem->getImage()->pixel(255, 0)).value() +
                        QColor(mImageItem->getImage()->pixel(0, 255)).value() +
                        QColor(mImageItem->getImage()->pixel(255, 255)).value()) /
                       4;
        if(midValue < 130)
        {
            mZoomer->setTrackerPen(QColor(Qt::white));
            mTrackerItem->setPen(QPen(Qt::white));
            mRectItem->setPen(QPen(Qt::white));
            mViewColorItem->setPen(QPen(Qt::white));
        }
        else
        {
            mZoomer->setTrackerPen(QColor(Qt::black));
            mTrackerItem->setPen(QPen(Qt::black));
            mRectItem->setPen(QPen(Qt::black));
            mViewColorItem->setPen(QPen(Qt::black));
        }
    }
}

int ColorPlot::zValue() const
{
    if(mControlWidget)
    {
        return mControlWidget->getRecoColorZ();
    }
    else
    {
        return 0;
    }
}

#include "moc_colorPlot.cpp"
