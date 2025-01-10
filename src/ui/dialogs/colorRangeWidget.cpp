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

#include "colorRangeWidget.h"

#include "control.h"
#include "petrack.h"


ColorRangeWidget::ColorRangeWidget(QWidget *parent) : QWidget(parent)
{
    mMainWindow = (class Petrack *) parent;
    mColorPlot  = mMainWindow->getControlWidget()->getColorPlot();

    mFromHue = 0;
    mFromSat = 0;
    mFromVal = 128;
    mToHue   = 359;
    mToSat   = 255;
    mToVal   = 255;

    setupUi(this);

    QString styleString = QString("background-color: hsv(%1,%2,%3)").arg(mFromHue).arg(mFromSat).arg(mFromVal);
    fromColor->setStyleSheet(styleString);
    styleString = QString("background-color: hsv(%1,%2,%3)").arg(mToHue).arg(mToSat).arg(mToVal);
    toColor->setStyleSheet(styleString);

    QColor col;
    col = QColor::fromHsv(mFromHue, mFromSat, mFromVal);
    fromTriangle->setColor(col);
    col.setHsv(mToHue, mToSat, mToVal);
    toTriangle->setColor(col);
}

// functions which force a new recognition
void ColorRangeWidget::on_inversHue_stateChanged(int i)
{
    mColorPlot->getMapItem()->changeActMapInvHue(i == Qt::Checked);
    mMainWindow->setRecognitionChanged(true); // flag indicates that changes of recognition parameters happens
    if(!mMainWindow->isLoading())
    {
        mMainWindow->updateImage();
    }
    mColorPlot->replot();
}

/**
 * @brief sets x and y slider from colormap widget
 * @pre x is hue, and y saturation
 * Sets the x and y slider to the right values according to fromColor and toColor,
 * which were selected via the color picker.
 */
void ColorRangeWidget::setControlWidget(int toHue, int fromHue, int toSat, int fromSat)
{
    int fH, tH, fS, tS;
    if(toHue < fromHue)
    {
        fH = toHue;
        tH = fromHue;
    }
    else
    {
        tH = toHue;
        fH = fromHue;
    }
    if(toSat < fromSat)
    {
        fS = toSat;
        tS = fromSat;
    }
    else
    {
        tS = toSat;
        fS = fromSat;
    }
    mMainWindow->getControlWidget()->setMapX(fH * 2);
    mMainWindow->getControlWidget()->setMapW((tH - fH));
    mMainWindow->getControlWidget()->setMapY(fS * 2);
    mMainWindow->getControlWidget()->setMapH((tS - fS));
}

void ColorRangeWidget::on_fromTriangle_colorChanged(const QColor &col)
{
    mFromHue            = col.hue();
    mFromSat            = col.saturation();
    mFromVal            = col.value();
    QString styleString = QString("background-color: hsv(%1,%2,%3)").arg(mFromHue).arg(mFromSat).arg(mFromVal);
    fromColor->setStyleSheet(styleString);

    mColorPlot->getMapItem()->changeActMapFromColor(col);
    setControlWidget(mToHue, mFromHue, mToSat, mFromSat);

    mMainWindow->setRecognitionChanged(true); // flag indicates that changes of recognition parameters happens
    if(!mMainWindow->isLoading())
    {
        mMainWindow->updateImage();
    }
}
void ColorRangeWidget::on_toTriangle_colorChanged(const QColor &col)
{
    mToHue              = col.hue();
    mToSat              = col.saturation();
    mToVal              = col.value();
    QString styleString = QString("background-color: hsv(%1,%2,%3)").arg(mToHue).arg(mToSat).arg(mToVal);
    toColor->setStyleSheet(styleString);

    mColorPlot->getMapItem()->changeActMapToColor(col);
    setControlWidget(mToHue, mFromHue, mToSat, mFromSat);

    mMainWindow->setRecognitionChanged(true); // flag indicates that changes of recognition parameters happens
    if(!mMainWindow->isLoading())
    {
        mMainWindow->updateImage();
    }
}

void ColorRangeWidget::on_fromColor_clicked()
{
    // QWindowsXpStyle uses native theming engine which causes some palette modifications not to have any effect.
    // ueber palette war der button ausser initial nicht zu aendern!!!

    QColor colBefore;
    colBefore  = QColor::fromHsv(mFromHue, mFromSat, mFromVal);
    QColor col = (QColorDialog::getColor(colBefore, this, "Select color from which value a pixel belongs to marker"))
                     .convertTo(QColor::Hsv);
    if(col.isValid() && col != colBefore)
    {
        on_fromTriangle_colorChanged(col);
        fromTriangle->setColor(col);
    }
}

void ColorRangeWidget::on_toColor_clicked()
{
    // QWindowsXpStyle uses native theming engine which causes some palette modifications not to have any effect.
    // ueber palette war der button ausser initial nicht zu aendern!!!
    QColor colBefore;
    colBefore  = QColor::fromHsv(mToHue, mToSat, mToVal);
    QColor col = (QColorDialog::getColor(colBefore, this, "Select color to which value a pixel belongs to marker"))
                     .convertTo(QColor::Hsv);
    if(col.isValid() && col != colBefore)
    {
        on_toTriangle_colorChanged(col);
        toTriangle->setColor(col);
    }
}

void ColorRangeWidget::setInvHue(bool b)
{
    inversHue->setChecked(b);
}

void ColorRangeWidget::setFromColor(const QColor &col)
{
    if(col.isValid())
    {
        mFromHue            = col.hue();
        mFromSat            = col.saturation();
        mFromVal            = col.value();
        QString styleString = QString("background-color: hsv(%1,%2,%3)").arg(mFromHue).arg(mFromSat).arg(mFromVal);
        fromColor->setStyleSheet(styleString);
        fromTriangle->setColor(col);
    }
}

void ColorRangeWidget::setToColor(const QColor &col)
{
    if(col.isValid())
    {
        mToHue              = col.hue();
        mToSat              = col.saturation();
        mToVal              = col.value();
        QString styleString = QString("background-color: hsv(%1,%2,%3)").arg(mToHue).arg(mToSat).arg(mToVal);
        toColor->setStyleSheet(styleString);
        toTriangle->setColor(col);
    }
}

#include "moc_colorRangeWidget.cpp"
