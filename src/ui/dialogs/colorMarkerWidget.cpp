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

#include "colorMarkerWidget.h"

#include "importHelper.h"

ColorMarkerWidget::ColorMarkerWidget(QWidget *parent) : QWidget(parent)
{
    mMainWindow = (class Petrack *) parent;

    fromHue = 0;
    fromSat = 0;
    fromVal = 128;
    toHue   = 359;
    toSat   = 255;
    toVal   = 255;

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

// store data in xml node, as:
//<COLOR_MARKER>
//    <MASK SHOW="0" OPACITY="100" MASK="1"/>
//    <FROM_COLOR HUE="0" SATURATION="0" VALUE="0"/>
//    <TO_COLOR HUE="0" SATURATION="0" VALUE="0"/>
//    <PARAM CLOSE_RADIUS="0" CLOSE_USED="0" OPEN_RADIUS="0" OPEN_USED="0" MIN_AREA="0" MAX_AREA="0" MAX_RATIO="0.0"/>
//</COLOR_MARKER>

/// store data in xml node
void ColorMarkerWidget::setXml(QDomElement &elem)
{
    QDomElement subElem;

    subElem = (elem.ownerDocument()).createElement("MASK");
    subElem.setAttribute("SHOW", showMask->isChecked());
    subElem.setAttribute("OPACITY", opacity->value());
    subElem.setAttribute("MASK", maskMask->isChecked());
    elem.appendChild(subElem);

    subElem = (elem.ownerDocument()).createElement("FROM_COLOR");
    subElem.setAttribute("HUE", fromHue);
    subElem.setAttribute("SATURATION", fromSat);
    subElem.setAttribute("VALUE", fromVal);
    elem.appendChild(subElem);

    subElem = (elem.ownerDocument()).createElement("TO_COLOR");
    subElem.setAttribute("HUE", toHue);
    subElem.setAttribute("SATURATION", toSat);
    subElem.setAttribute("VALUE", toVal);
    elem.appendChild(subElem);

    subElem = (elem.ownerDocument()).createElement("PARAM");
    subElem.setAttribute("INVERS_HUE", inversHue->isChecked());
    subElem.setAttribute("CLOSE_RADIUS", closeRadius->value());
    subElem.setAttribute("CLOSE_USED", useClose->isChecked());
    subElem.setAttribute("OPEN_RADIUS", openRadius->value());
    subElem.setAttribute("OPEN_USED", useOpen->isChecked());
    subElem.setAttribute("MIN_AREA", minArea->value());
    subElem.setAttribute("MAX_AREA", maxArea->value());
    subElem.setAttribute("MAX_RATIO", maxRatio->value());
    elem.appendChild(subElem);
}

/// read data from xml node
void ColorMarkerWidget::getXml(QDomElement &elem)
{
    QDomElement subElem;
    int         h = 0, s = 0, v = 0; // init, damit compiler nicht meckert
    QString     styleString;
    QColor      col;
    col = col.toHsv();

    for(subElem = elem.firstChildElement(); !subElem.isNull(); subElem = subElem.nextSiblingElement())
    {
        if(subElem.tagName() == "MASK")
        {
            loadBoolValue(subElem, "SHOW", showMask);
            loadIntValue(subElem, "OPACITY", opacity);
            loadBoolValue(subElem, "MASK", maskMask);
        }
        if(subElem.tagName() == "FROM_COLOR")
        {
            h = readInt(subElem, "HUE");
            s = readInt(subElem, "SATURATION");
            v = readInt(subElem, "VALUE");

            col.setHsv(h, s, v);
            if(col.isValid())
            {
                fromHue = h;
                fromSat = s;
                fromVal = v;
                fromTriangle->setColor(col);
                styleString = QString("background-color: hsv(%1,%2,%3)").arg(h).arg(s).arg(v);
                fromColor->setStyleSheet(styleString);
            }
        }
        if(subElem.tagName() == "TO_COLOR")
        {
            h = readInt(subElem, "HUE");
            s = readInt(subElem, "SATURATION");
            v = readInt(subElem, "VALUE");

            col.setHsv(h, s, v);
            if(col.isValid())
            {
                toHue = h;
                toSat = s;
                toVal = s;
                toTriangle->setColor(col);
                styleString = QString("background-color: hsv(%1,%2,%3)").arg(h).arg(s).arg(v);
                toColor->setStyleSheet(styleString);
            }
        }
        if(subElem.tagName() == "PARAM")
        {
            loadBoolValue(subElem, "INVERS_HUE", inversHue);
            loadIntValue(subElem, "CLOSE_RADIUS", closeRadius);
            loadBoolValue(subElem, "CLOSE_USED", useClose);
            loadIntValue(subElem, "OPEN_RADIUS", openRadius);
            loadBoolValue(subElem, "OPEN_USED", useOpen);
            loadIntValue(subElem, "MIN_AREA", minArea);
            loadIntValue(subElem, "MAX_AREA", maxArea);
            loadDoubleValue(subElem, "MAX_RATIO", maxRatio);
        }
    }
}

void ColorMarkerWidget::on_fromTriangle_colorChanged(const QColor &col)
{
    fromHue             = col.hue();
    fromSat             = col.saturation();
    fromVal             = col.value();
    QString styleString = QString("background-color: hsv(%1,%2,%3)").arg(fromHue).arg(fromSat).arg(fromVal);
    fromColor->setStyleSheet(styleString);
    mMainWindow->setRecognitionChanged(true); // flag indicates that changes of recognition parameters happens
    if(!mMainWindow->isLoading())
    {
        mMainWindow->updateImage();
    }
}
void ColorMarkerWidget::on_toTriangle_colorChanged(const QColor &col)
{
    toHue               = col.hue();
    toSat               = col.saturation();
    toVal               = col.value();
    QString styleString = QString("background-color: hsv(%1,%2,%3)").arg(toHue).arg(toSat).arg(toVal);
    toColor->setStyleSheet(styleString);
    mMainWindow->setRecognitionChanged(true); // flag indicates that changes of recognition parameters happens
    if(!mMainWindow->isLoading())
    {
        mMainWindow->updateImage();
    }
}

void ColorMarkerWidget::on_fromColor_clicked()
{
    // QWindowsXpStyle uses native theming engine which causes some palette modifications not to have any effect.
    // ueber palette war der button ausser initial nicht zu aendern!!!
    QColor colBefore;
    colBefore  = QColor::fromHsv(fromHue, fromSat, fromVal);
    QColor col = (QColorDialog::getColor(colBefore, this, "Select color from which value a pixel belongs to marker"))
                     .convertTo(QColor::Hsv);
    if(col.isValid() && col != colBefore)
    {
        on_fromTriangle_colorChanged(col);
        fromTriangle->setColor(col);
    }
}

void ColorMarkerWidget::on_toColor_clicked()
{
    // QWindowsXpStyle uses native theming engine which causes some palette modifications not to have any effect.
    // ueber palette war der button ausser initial nicht zu aendern!!!
    QColor colBefore;
    colBefore  = QColor::fromHsv(toHue, toSat, toVal);
    QColor col = (QColorDialog::getColor(colBefore, this, "Select color to which value a pixel belongs to marker"))
                     .convertTo(QColor::Hsv);
    if(col.isValid() && col != colBefore)
    {
        on_toTriangle_colorChanged(col);
        toTriangle->setColor(col);
    }
}

#include "moc_colorMarkerWidget.cpp"
