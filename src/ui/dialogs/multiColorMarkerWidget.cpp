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

void MultiColorMarkerWidget::on_useDot_stateChanged(int)
{
    mMainWindow->setRecognitionChanged(true); // flag indicates that changes of recognition parameters happens
    mMainWindow->updateImage();
}

void MultiColorMarkerWidget::on_dotSize_valueChanged(double)
{
    mMainWindow->setRecognitionChanged(true); // flag indicates that changes of recognition parameters happens
    mMainWindow->updateImage();
}

void MultiColorMarkerWidget::on_useCodeMarker_stateChanged(int)
{
    mMainWindow->setRecognitionChanged(true);
    mMainWindow->updateImage();
}

void MultiColorMarkerWidget::on_CodeMarkerParameter_clicked()
{
    mMainWindow->getCodeMarkerWidget()->show();
}

void MultiColorMarkerWidget::on_ignoreWithoutDot_stateChanged(int)
{
    mMainWindow->setRecognitionChanged(true); // flag indicates that changes of recognition parameters happens
    mMainWindow->updateImage();
}

void MultiColorMarkerWidget::on_useColor_stateChanged(int) // eigentlich nichts noetig, da nur beim Tracing aktiv
{
    mMainWindow->setRecognitionChanged(true); // flag indicates that changes of recognition parameters happens
    mMainWindow->updateImage();
}

void MultiColorMarkerWidget::on_restrictPosition_stateChanged(int)
{
    mMainWindow->setRecognitionChanged(true); // flag indicates that changes of recognition parameters happens
    mMainWindow->updateImage();
}

void MultiColorMarkerWidget::on_autoCorrect_stateChanged(int)
{
    mMainWindow->setRecognitionChanged(true); // flag indicates that changes of recognition parameters happens
    mMainWindow->updateImage();
}

void MultiColorMarkerWidget::on_autoCorrectOnlyExport_stateChanged(int)
{
    mMainWindow->setRecognitionChanged(true); // flag indicates that changes of recognition parameters happens
    mMainWindow->updateImage();
}

void MultiColorMarkerWidget::on_showMask_stateChanged(int i)
{
    mMainWindow->getMultiColorMarkerItem()->setVisible(i);
    mMainWindow->getScene()->update();
}

void MultiColorMarkerWidget::on_maskMask_stateChanged(int)
{
    mMainWindow->getScene()->update();
}

void MultiColorMarkerWidget::on_opacity_valueChanged(int)
{
    mMainWindow->getScene()->update();
}

void MultiColorMarkerWidget::on_useOpen_stateChanged(int)
{
    mMainWindow->setRecognitionChanged(true); // flag indicates that changes of recognition parameters happens
    if(!mMainWindow->isLoading())
    {
        mMainWindow->updateImage();
    }
}

void MultiColorMarkerWidget::on_useClose_stateChanged(int)
{
    mMainWindow->setRecognitionChanged(true); // flag indicates that changes of recognition parameters happens
    if(!mMainWindow->isLoading())
    {
        mMainWindow->updateImage();
    }
}

void MultiColorMarkerWidget::on_closeRadius_valueChanged(int)
{
    mMainWindow->setRecognitionChanged(true); // flag indicates that changes of recognition parameters happens
    if(!mMainWindow->isLoading())
    {
        mMainWindow->updateImage();
    }
}

void MultiColorMarkerWidget::on_openRadius_valueChanged(int)
{
    mMainWindow->setRecognitionChanged(true); // flag indicates that changes of recognition parameters happens
    if(!mMainWindow->isLoading())
    {
        mMainWindow->updateImage();
    }
}

void MultiColorMarkerWidget::on_minArea_valueChanged(int)
{
    mMainWindow->setRecognitionChanged(true); // flag indicates that changes of recognition parameters happens
    if(!mMainWindow->isLoading())
    {
        mMainWindow->updateImage();
    }
}

void MultiColorMarkerWidget::on_maxArea_valueChanged(int)
{
    mMainWindow->setRecognitionChanged(true); // flag indicates that changes of recognition parameters happens
    if(!mMainWindow->isLoading())
    {
        mMainWindow->updateImage();
    }
}

void MultiColorMarkerWidget::on_useHeadSize_stateChanged(int i)
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

    mMainWindow->setRecognitionChanged(true); // flag indicates that changes of recognition parameters happens
    mMainWindow->updateImage();
}

void MultiColorMarkerWidget::on_maxRatio_valueChanged(double)
{
    mMainWindow->setRecognitionChanged(true); // flag indicates that changes of recognition parameters happens
    if(!mMainWindow->isLoading())
    {
        mMainWindow->updateImage();
    }
}

#include "moc_multiColorMarkerWidget.cpp"
