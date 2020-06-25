#ifndef COLORRANGEWIDGET_H
#define COLORRANGEWIDGET_H

#include <QtWidgets>
#include "ui_colorRange.h"

#include "petrack.h"
#include "colorPlot.h"

class ColorRangeWidget: public QWidget, public Ui::ColorRange
{
    Q_OBJECT

public:
    ColorRangeWidget(QWidget *parent = 0);

    int fromHue;
    int fromSat;
    int fromVal;
    int toHue;
    int toSat;
    int toVal;

    void setInvHue(bool b);
    void setFromColor(const QColor &col);
    void setToColor(const QColor &col);
    void setControlWidget(int toHue, int fromHue, int toSat, int fromSat);

private slots:

    // functions which force a new recognition
    void on_inversHue_stateChanged(int i);

    void on_fromTriangle_colorChanged(const QColor &col);
    void on_toTriangle_colorChanged(const QColor &col);

    void on_fromColor_clicked();
    void on_toColor_clicked();

private:



    Petrack *mMainWindow;
    ColorPlot *mColorPlot;
};

#endif
