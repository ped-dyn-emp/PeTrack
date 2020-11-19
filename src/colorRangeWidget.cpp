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

#include "colorRangeWidget.h"
#include "control.h"

ColorRangeWidget::ColorRangeWidget(QWidget *parent)
    : QWidget(parent)
{
    mMainWindow = (class Petrack*) parent;
    mColorPlot = mMainWindow->getControlWidget()->getColorPlot();

    fromHue = 0;
    fromSat = 0;
    fromVal = 128;
    toHue = 359;
    toSat = 255;
    toVal = 255;

    setupUi(this);

    QString styleString = QString("background-color: hsv(%1,%2,%3)").arg(fromHue).arg(fromSat).arg(fromVal);
    fromColor->setStyleSheet(styleString);
    styleString = QString("background-color: hsv(%1,%2,%3)").arg(toHue).arg(toSat).arg(toVal);
    toColor->setStyleSheet(styleString);

    QColor col;
    col = QColor::fromHsv(fromHue, fromSat, fromVal);
    fromTriangle->setColor(col);
    col.setHsv(toHue, toSat, toVal);
    toTriangle->setColor(col);
}

// functions which force a new recognition
void ColorRangeWidget::on_inversHue_stateChanged(int i)
{
    mColorPlot->getMapItem()->changeActMapInvHue(i == Qt::Checked);
    mMainWindow->setRecognitionChanged(true);// flag indicates that changes of recognition parameters happens
    if( !mMainWindow->isLoading() )
        mMainWindow->updateImage();
    mColorPlot->replot();
}

// WARNING Setzt voraus, dass x als Hue und y als Saturation ausgewählt ist
// Zwar Standardeinstellung, aber nicht zwingend der Fall
/**
 * @brief sets x and y slider from colormap widget
 *
 * Sets the x and y slider to the right values according to fromColor and toColor,
 * which were selected via the color picker.
 */
void ColorRangeWidget::setControlWidget(int toHue, int fromHue, int toSat, int fromSat)
{

    int fH, tH, fS, tS;
    if (toHue < fromHue)
    {
        fH = toHue;
        tH = fromHue;
    }
    else
    {
        tH = toHue;
        fH = fromHue;
    }
    if (toSat < fromSat)
    {
        fS = toSat;
        tS = fromSat;
    }
    else
    {
        tS = toSat;
        fS = fromSat;
    }
    mMainWindow->getControlWidget()->mapX->setValue(fH*2);
    mMainWindow->getControlWidget()->mapW->setValue((tH-fH));
    mMainWindow->getControlWidget()->mapY->setValue(fS*2);
    mMainWindow->getControlWidget()->mapH->setValue((tS-fS));
}

void ColorRangeWidget::on_fromTriangle_colorChanged(const QColor &col)
{
    fromHue = col.hue(); fromSat = col.saturation(); fromVal = col.value();
    QString styleString = QString("background-color: hsv(%1,%2,%3)").arg(fromHue).arg(fromSat).arg(fromVal);
    //QString styleString = QString("background-color: rgb(%1,%2,%3)").arg(col.red()).arg(col.green()).arg(col.blue());
    fromColor->setStyleSheet(styleString);

    mColorPlot->getMapItem()->changeActMapFromColor(col);
    setControlWidget(toHue, fromHue, toSat, fromSat);

    mMainWindow->setRecognitionChanged(true);// flag indicates that changes of recognition parameters happens
    if( !mMainWindow->isLoading() )
        mMainWindow->updateImage();
}
void ColorRangeWidget::on_toTriangle_colorChanged(const QColor &col)
{
    toHue = col.hue(); toSat = col.saturation(); toVal = col.value();
    QString styleString = QString("background-color: hsv(%1,%2,%3)").arg(toHue).arg(toSat).arg(toVal);
    //QString styleString = QString("background-color: rgb(%1,%2,%3)").arg(col.red()).arg(col.green()).arg(col.blue());
    toColor->setStyleSheet(styleString);

    mColorPlot->getMapItem()->changeActMapToColor(col);
    setControlWidget(toHue, fromHue, toSat, fromSat);

    mMainWindow->setRecognitionChanged(true);// flag indicates that changes of recognition parameters happens
    if( !mMainWindow->isLoading() )
        mMainWindow->updateImage();
}

void ColorRangeWidget::on_fromColor_clicked()
{
    // QWindowsXpStyle uses native theming engine which causes some palette modifications not to have any effect.
    // ueber palette war der button ausser initial nicht zu aendern!!!

    QColor colBefore;//fromColor->palette().color(QPalette::Button);
    colBefore = QColor::fromHsv(fromHue, fromSat, fromVal);
    QColor col = (QColorDialog::getColor(colBefore, this, "Select color from which value a pixel belongs to marker")).convertTo(QColor::Hsv);
    if (col.isValid() && col != colBefore)
    {
        on_fromTriangle_colorChanged(col);
        fromTriangle->setColor(col);
    }
}

void ColorRangeWidget::on_toColor_clicked()
{
    // QWindowsXpStyle uses native theming engine which causes some palette modifications not to have any effect.
    // ueber palette war der button ausser initial nicht zu aendern!!!
    QColor colBefore;//toColor->palette().color(QPalette::Button);
    colBefore = QColor::fromHsv(toHue, toSat, toVal);
    QColor col = (QColorDialog::getColor(colBefore, this, "Select color to which value a pixel belongs to marker")).convertTo(QColor::Hsv);
    //QPalette palette = toColor->palette();
    //palette.setColor(QPalette::Button, col);
    //toColor->setPalette(palette);
    //toColor->setAutoFillBackground(true);
    if (col.isValid() && col != colBefore)
    {
        on_toTriangle_colorChanged(col);
        toTriangle->setColor(col);
    }
// Versuch mit hsv stabil werte einzugeben - wandern in hsv und rgb zur folge, wenn auch wenig
//        // QWindowsXpStyle uses native theming engine which causes some palette modifications not to have any effect.
//        // ueber palette war der button ausser initial nicht zu aendern!!!
//        QColor col = QColorDialog::getColor(toColor->palette().color(QPalette::Button).toHsv(), this, "").toHsv();
//        //debout << col.hue()<<endl;
//        //debout << col.saturation()<<endl;
//        //debout << col.value()<<endl;
//        //QPalette palette = toColor->palette();
//        //palette.setColor(QPalette::Button, col);
//        //toColor->setPalette(palette);
//        //toColor->setAutoFillBackground(true);
//        if (col.isValid())
//        {
//            QString styleString = QString("background-color: hsv(%1,%2,%3)").arg(col.hue()).arg(col.saturation()).arg(col.value());
//            toColor->setStyleSheet(styleString);
//        }
}

void ColorRangeWidget::setInvHue(bool b)
{
    inversHue->setChecked(b);//setCheckState();
}

void ColorRangeWidget::setFromColor(const QColor &col)
{
    if (col.isValid())
    {
        fromHue = col.hue(); fromSat = col.saturation(); fromVal = col.value();
        QString styleString = QString("background-color: hsv(%1,%2,%3)").arg(fromHue).arg(fromSat).arg(fromVal);
        fromColor->setStyleSheet(styleString);
        fromTriangle->setColor(col);
    }
}

void ColorRangeWidget::setToColor(const QColor &col)
{
    if (col.isValid())
    {
        toHue = col.hue(); toSat = col.saturation(); toVal = col.value();
        QString styleString = QString("background-color: hsv(%1,%2,%3)").arg(toHue).arg(toSat).arg(toVal);
        toColor->setStyleSheet(styleString);
        toTriangle->setColor(col);
    }
}

#include "moc_colorRangeWidget.cpp"
