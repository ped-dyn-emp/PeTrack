/*
 * PeTrack - Software for tracking pedestrians movement in videos
 * Copyright (C) 2026 Forschungszentrum Jülich GmbH, IAS-7
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

#include "colorMarkerItem.h"
#include "importHelper.h"
#include "petrack.h"


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

    connect(showMask, &QCheckBox::checkStateChanged, this, &ColorMarkerWidget::onShowMaskStateChanged);
    connect(maskMask, &QCheckBox::checkStateChanged, this, &ColorMarkerWidget::onMaskMaskStateChanged);
    connect(opacity, QOverload<int>::of(&QSpinBox::valueChanged), this, &ColorMarkerWidget::onOpacityValueChanged);
    connect(inversHue, &QCheckBox::checkStateChanged, this, &ColorMarkerWidget::onInversHueStateChanged);
    connect(useOpen, &QCheckBox::checkStateChanged, this, &ColorMarkerWidget::onUseOpenStateChanged);
    connect(useClose, &QCheckBox::checkStateChanged, this, &ColorMarkerWidget::onUseCloseStateChanged);
    connect(
        closeRadius, QOverload<int>::of(&QSpinBox::valueChanged), this, &ColorMarkerWidget::onCloseRadiusValueChanged);
    connect(
        openRadius, QOverload<int>::of(&QSpinBox::valueChanged), this, &ColorMarkerWidget::onOpenRadiusValueChanged);
    connect(minArea, QOverload<int>::of(&QSpinBox::valueChanged), this, &ColorMarkerWidget::onMinAreaValueChanged);
    connect(maxArea, QOverload<int>::of(&QSpinBox::valueChanged), this, &ColorMarkerWidget::onMaxAreaValueChanged);
    connect(
        maxRatio,
        QOverload<double>::of(&QDoubleSpinBox::valueChanged),
        this,
        &ColorMarkerWidget::onMaxRatioValueChanged);
    connect(fromTriangle, &QtColorTriangle::colorChanged, this, &ColorMarkerWidget::onFromTriangleColorChanged);
    connect(toTriangle, &QtColorTriangle::colorChanged, this, &ColorMarkerWidget::onToTriangleColorChanged);
    connect(fromColor, &QPushButton::clicked, this, &ColorMarkerWidget::onFromColorClicked);
    connect(toColor, &QPushButton::clicked, this, &ColorMarkerWidget::onToColorClicked);
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

void ColorMarkerWidget::onShowMaskStateChanged(int i)
{
    mMainWindow->getColorMarkerItem()->setVisible(i);
    mMainWindow->getScene()->update();
}

void ColorMarkerWidget::onMaskMaskStateChanged()
{
    mMainWindow->getScene()->update();
}

void ColorMarkerWidget::onOpacityValueChanged()
{
    mMainWindow->getScene()->update();
}

void ColorMarkerWidget::onInversHueStateChanged()
{
    mMainWindow->setRecognitionChanged(true); // flag indicates that changes of recognition parameters happens
    if(!mMainWindow->isLoading())
    {
        mMainWindow->updateImage();
    }
}

void ColorMarkerWidget::onUseOpenStateChanged()
{
    mMainWindow->setRecognitionChanged(true); // flag indicates that changes of recognition parameters happens
    if(!mMainWindow->isLoading())
    {
        mMainWindow->updateImage();
    }
}

void ColorMarkerWidget::onUseCloseStateChanged()
{
    mMainWindow->setRecognitionChanged(true); // flag indicates that changes of recognition parameters happens
    if(!mMainWindow->isLoading())
    {
        mMainWindow->updateImage();
    }
}

void ColorMarkerWidget::onCloseRadiusValueChanged()
{
    mMainWindow->setRecognitionChanged(true); // flag indicates that changes of recognition parameters happens
    if(!mMainWindow->isLoading())
    {
        mMainWindow->updateImage();
    }
}

void ColorMarkerWidget::onOpenRadiusValueChanged()
{
    mMainWindow->setRecognitionChanged(true); // flag indicates that changes of recognition parameters happens
    if(!mMainWindow->isLoading())
    {
        mMainWindow->updateImage();
    }
}

void ColorMarkerWidget::onMinAreaValueChanged()
{
    mMainWindow->setRecognitionChanged(true); // flag indicates that changes of recognition parameters happens
    if(!mMainWindow->isLoading())
    {
        mMainWindow->updateImage();
    }
}

void ColorMarkerWidget::onMaxAreaValueChanged()
{
    mMainWindow->setRecognitionChanged(true); // flag indicates that changes of recognition parameters happens
    if(!mMainWindow->isLoading())
    {
        mMainWindow->updateImage();
    }
}

void ColorMarkerWidget::onMaxRatioValueChanged()
{
    mMainWindow->setRecognitionChanged(true); // flag indicates that changes of recognition parameters happens
    if(!mMainWindow->isLoading())
    {
        mMainWindow->updateImage();
    }
}

void ColorMarkerWidget::onFromTriangleColorChanged(const QColor &col)
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
void ColorMarkerWidget::onToTriangleColorChanged(const QColor &col)
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

void ColorMarkerWidget::onFromColorClicked()
{
    // QWindowsXpStyle uses native theming engine which causes some palette modifications not to have any effect.
    // ueber palette war der button ausser initial nicht zu aendern!!!
    QColor colBefore;
    colBefore  = QColor::fromHsv(fromHue, fromSat, fromVal);
    QColor col = (QColorDialog::getColor(colBefore, this, "Select color from which value a pixel belongs to marker"))
                     .convertTo(QColor::Hsv);
    if(col.isValid() && col != colBefore)
    {
        onFromTriangleColorChanged(col);
        fromTriangle->setColor(col);
    }
}

void ColorMarkerWidget::onToColorClicked()
{
    // QWindowsXpStyle uses native theming engine which causes some palette modifications not to have any effect.
    // ueber palette war der button ausser initial nicht zu aendern!!!
    QColor colBefore;
    colBefore  = QColor::fromHsv(toHue, toSat, toVal);
    QColor col = (QColorDialog::getColor(colBefore, this, "Select color to which value a pixel belongs to marker"))
                     .convertTo(QColor::Hsv);
    if(col.isValid() && col != colBefore)
    {
        onToTriangleColorChanged(col);
        toTriangle->setColor(col);
    }
}

#include "moc_colorMarkerWidget.cpp"
