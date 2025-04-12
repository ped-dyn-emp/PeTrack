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

#include "multiColorMarkerWidget.h"

#include "control.h"
#include "importHelper.h"
#include "petrack.h"

MultiColorMarkerWidget::MultiColorMarkerWidget(QWidget *parent) : QWidget(parent)
{
    mMainWindow = (class Petrack *) parent;
    setupUi(this);
    mOldMinArea = minArea->value();
    mOldMaxArea = maxArea->value();

    connect(useDot, &QCheckBox::stateChanged, this, &MultiColorMarkerWidget::onUseDotStateChanged);
    connect(
        dotSize,
        QOverload<double>::of(&QDoubleSpinBox::valueChanged),
        this,
        &MultiColorMarkerWidget::onDotSizeValueChanged);
    connect(useCodeMarker, &QCheckBox::stateChanged, this, &MultiColorMarkerWidget::onUseCodeMarkerStateChanged);
    connect(CodeMarkerParameter, &QPushButton::clicked, this, &MultiColorMarkerWidget::onCodeMarkerParameterClicked);
    connect(ignoreWithoutDot, &QCheckBox::stateChanged, this, &MultiColorMarkerWidget::onIgnoreWithoutDotStateChanged);
    connect(useColor, &QCheckBox::stateChanged, this, &MultiColorMarkerWidget::onUseColorStateChanged);
    connect(restrictPosition, &QCheckBox::stateChanged, this, &MultiColorMarkerWidget::onRestrictPositionStateChanged);
    connect(autoCorrect, &QCheckBox::stateChanged, this, &MultiColorMarkerWidget::onAutoCorrectStateChanged);
    connect(
        autoCorrectOnlyExport,
        &QCheckBox::stateChanged,
        this,
        &MultiColorMarkerWidget::onAutoCorrectOnlyExportStateChanged);
    connect(showMask, &QCheckBox::stateChanged, this, &MultiColorMarkerWidget::onShowMaskStateChanged);
    connect(maskMask, &QCheckBox::stateChanged, this, &MultiColorMarkerWidget::onMaskMaskStateChanged);
    connect(opacity, QOverload<int>::of(&QSpinBox::valueChanged), this, &MultiColorMarkerWidget::onOpacityValueChanged);
    connect(useOpen, &QCheckBox::stateChanged, this, &MultiColorMarkerWidget::onUseOpenStateChanged);
    connect(useClose, &QCheckBox::stateChanged, this, &MultiColorMarkerWidget::onUseCloseStateChanged);
    connect(
        closeRadius,
        QOverload<int>::of(&QSpinBox::valueChanged),
        this,
        &MultiColorMarkerWidget::onCloseRadiusValueChanged);
    connect(
        openRadius,
        QOverload<int>::of(&QSpinBox::valueChanged),
        this,
        &MultiColorMarkerWidget::onOpenRadiusValueChanged);
    connect(minArea, QOverload<int>::of(&QSpinBox::valueChanged), this, &MultiColorMarkerWidget::onMinAreaValueChanged);
    connect(maxArea, QOverload<int>::of(&QSpinBox::valueChanged), this, &MultiColorMarkerWidget::onMaxAreaValueChanged);
    connect(useHeadSize, &QCheckBox::stateChanged, this, &MultiColorMarkerWidget::onUseHeadSizeStateChanged);
    connect(
        maxRatio,
        QOverload<double>::of(&QDoubleSpinBox::valueChanged),
        this,
        &MultiColorMarkerWidget::onMaxRatioValueChanged);
}

// store data in xml node
void MultiColorMarkerWidget::setXml(QDomElement &elem)
{
    QDomElement subElem;

    subElem = (elem.ownerDocument()).createElement("BLACK_DOT");
    subElem.setAttribute("USE", useDot->isChecked());
    subElem.setAttribute("SIZE", dotSize->value());
    subElem.setAttribute("IGNORE_WITHOUT", ignoreWithoutDot->isChecked());
    subElem.setAttribute("USE_COLOR", useColor->isChecked());
    subElem.setAttribute("RESTRICT_POSITION", restrictPosition->isChecked());
    elem.appendChild(subElem);

    subElem = (elem.ownerDocument()).createElement("CODE_MARKER");
    subElem.setAttribute("USE", useCodeMarker->isChecked());
    elem.appendChild(subElem);

    subElem = (elem.ownerDocument()).createElement("AUTO_CORRECT");
    subElem.setAttribute("USE", autoCorrect->isChecked());
    subElem.setAttribute("ONLY_EXPORT", autoCorrectOnlyExport->isChecked());
    elem.appendChild(subElem);

    subElem = (elem.ownerDocument()).createElement("MASK");
    subElem.setAttribute("SHOW", showMask->isChecked());
    subElem.setAttribute("OPACITY", opacity->value());
    subElem.setAttribute("MASK", maskMask->isChecked());
    elem.appendChild(subElem);

    subElem = (elem.ownerDocument()).createElement("PARAM");
    subElem.setAttribute("CLOSE_RADIUS", closeRadius->value());
    subElem.setAttribute("CLOSE_USED", useClose->isChecked());
    subElem.setAttribute("OPEN_RADIUS", openRadius->value());
    subElem.setAttribute("OPEN_USED", useOpen->isChecked());
    subElem.setAttribute("MIN_AREA", minArea->value());
    subElem.setAttribute("MAX_AREA", maxArea->value());
    subElem.setAttribute("USE_HEAD_SIZE", useHeadSize->isChecked());
    subElem.setAttribute("MAX_RATIO", maxRatio->value());
    elem.appendChild(subElem);
}

// read data from xml node
void MultiColorMarkerWidget::getXml(QDomElement &elem)
{
    QDomElement subElem;
    QString     styleString;

    for(subElem = elem.firstChildElement(); !subElem.isNull(); subElem = subElem.nextSiblingElement())
    {
        if(subElem.tagName() == "MASK")
        {
            loadBoolValue(subElem, "SHOW", showMask);
            loadIntValue(subElem, "OPACITY", opacity);
            loadBoolValue(subElem, "MASK", maskMask);
        }
        if(subElem.tagName() == "BLACK_DOT")
        {
            loadBoolValue(subElem, "USE", useDot);
            loadDoubleValue(subElem, "SIZE", dotSize);
            loadBoolValue(subElem, "IGNORE_WITHOUT", ignoreWithoutDot);
            loadBoolValue(subElem, "USE_COLOR", useColor);
            loadBoolValue(subElem, "RESTRICT_POSITION", restrictPosition);
        }
        if(subElem.tagName() == "CODE_MARKER")
        {
            loadBoolValue(subElem, "USE", useCodeMarker);
            // ignoreWithoutDot handles black dot and code marker and is therefore only read once
        }
        if(subElem.tagName() == "AUTO_CORRECT")
        {
            loadBoolValue(subElem, "USE", autoCorrect);
            loadBoolValue(subElem, "ONLY_EXPORT", autoCorrectOnlyExport, false);
        }
        if(subElem.tagName() == "PARAM")
        {
            loadIntValue(subElem, "CLOSE_RADIUS", closeRadius);
            loadBoolValue(subElem, "CLOSE_USED", useClose);
            loadIntValue(subElem, "OPEN_RADIUS", openRadius);
            loadBoolValue(subElem, "OPEN_USED", useOpen);
            loadIntValue(subElem, "MIN_AREA", minArea);
            loadIntValue(subElem, "MAX_AREA", maxArea);
            loadBoolValue(subElem, "USE_HEAD_SIZE", useHeadSize, false);
            loadDoubleValue(subElem, "MAX_RATIO", maxRatio);
        }
    }
}

void MultiColorMarkerWidget::onUseDotStateChanged()
{
    mMainWindow->setRecognitionChanged(true);
    mMainWindow->updateImage();
}

void MultiColorMarkerWidget::onDotSizeValueChanged()
{
    mMainWindow->setRecognitionChanged(true);
    mMainWindow->updateImage();
}

void MultiColorMarkerWidget::onUseCodeMarkerStateChanged()
{
    mMainWindow->setRecognitionChanged(true);
    mMainWindow->updateImage();
}

void MultiColorMarkerWidget::onCodeMarkerParameterClicked()
{
    mMainWindow->getCodeMarkerWidget()->show();
}

void MultiColorMarkerWidget::onIgnoreWithoutDotStateChanged()
{
    mMainWindow->setRecognitionChanged(true);
    mMainWindow->updateImage();
}

void MultiColorMarkerWidget::onUseColorStateChanged()
{
    mMainWindow->setRecognitionChanged(true);
    mMainWindow->updateImage();
}

void MultiColorMarkerWidget::onRestrictPositionStateChanged()
{
    mMainWindow->setRecognitionChanged(true);
    mMainWindow->updateImage();
}

void MultiColorMarkerWidget::onAutoCorrectStateChanged()
{
    mMainWindow->setRecognitionChanged(true);
    mMainWindow->updateImage();
}

void MultiColorMarkerWidget::onAutoCorrectOnlyExportStateChanged()
{
    mMainWindow->setRecognitionChanged(true);
    mMainWindow->updateImage();
}

void MultiColorMarkerWidget::onShowMaskStateChanged(int i)
{
    mMainWindow->getMultiColorMarkerItem()->setVisible(i);
    mMainWindow->getScene()->update();
}

void MultiColorMarkerWidget::onMaskMaskStateChanged()
{
    mMainWindow->getScene()->update();
}

void MultiColorMarkerWidget::onOpacityValueChanged()
{
    mMainWindow->getScene()->update();
}

void MultiColorMarkerWidget::onUseOpenStateChanged()
{
    mMainWindow->setRecognitionChanged(true);
    if(!mMainWindow->isLoading())
    {
        mMainWindow->updateImage();
    }
}

void MultiColorMarkerWidget::onUseCloseStateChanged()
{
    mMainWindow->setRecognitionChanged(true);
    if(!mMainWindow->isLoading())
    {
        mMainWindow->updateImage();
    }
}

void MultiColorMarkerWidget::onCloseRadiusValueChanged()
{
    mMainWindow->setRecognitionChanged(true);
    if(!mMainWindow->isLoading())
    {
        mMainWindow->updateImage();
    }
}

void MultiColorMarkerWidget::onOpenRadiusValueChanged()
{
    mMainWindow->setRecognitionChanged(true);
    if(!mMainWindow->isLoading())
    {
        mMainWindow->updateImage();
    }
}

void MultiColorMarkerWidget::onMinAreaValueChanged()
{
    mMainWindow->setRecognitionChanged(true);
    if(!mMainWindow->isLoading())
    {
        mMainWindow->updateImage();
    }
}

void MultiColorMarkerWidget::onMaxAreaValueChanged()
{
    mMainWindow->setRecognitionChanged(true);
    if(!mMainWindow->isLoading())
    {
        mMainWindow->updateImage();
    }
}

void MultiColorMarkerWidget::onUseHeadSizeStateChanged(int i)
{
    if(i)
    {
        if(mMainWindow->getImageItem() && mMainWindow->getImage() && mMainWindow->getControlWidget())
        {
            const auto &worldImgCorr = mMainWindow->getWorldImageCorrespondence();
            QPointF cmPerPixel1 = worldImgCorr.getCmPerPixel(0, 0, mMainWindow->getControlWidget()->getDefaultHeight());
            QPointF cmPerPixel2 = worldImgCorr.getCmPerPixel(
                mMainWindow->getImage()->width() - 1,
                mMainWindow->getImage()->height() - 1,
                mMainWindow->getControlWidget()->getDefaultHeight());
            double cmPerPixelAvg = (cmPerPixel1.x() + cmPerPixel1.y() + cmPerPixel2.x() + cmPerPixel2.y()) / 4.;
            if(cmPerPixelAvg > 0)
            {
                double area = PI * 0.25 * HEAD_SIZE / cmPerPixelAvg * 14. /
                              cmPerPixelAvg; // 14. Kopfbreite // Elipse: A=Pi*a*b (a,b Halbachsen)
                mOldMinArea = minArea->value();
                mOldMaxArea = maxArea->value();

                minArea->setValue(area * 0.75);
                maxArea->setValue(area * 2.5);
            }
        }
        minArea->setDisabled(true);
        maxArea->setDisabled(true);
    }
    else
    {
        minArea->setDisabled(false);
        maxArea->setDisabled(false);
        minArea->setValue(mOldMinArea);
        maxArea->setValue(mOldMaxArea);
    }

    mMainWindow->setRecognitionChanged(true);
    mMainWindow->updateImage();
}

void MultiColorMarkerWidget::onMaxRatioValueChanged()
{
    mMainWindow->setRecognitionChanged(true);
    if(!mMainWindow->isLoading())
    {
        mMainWindow->updateImage();
    }
}

#include "moc_multiColorMarkerWidget.cpp"
