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

#include "stereoWidget.h"

#include "importHelper.h"
#include "logger.h"
#include "petrack.h"
#include "stereoItem.h"


StereoWidget::StereoWidget(QWidget *parent) : QWidget(parent)
{
    mMainWindow = (class Petrack *) parent;

    setupUi(this);

    stereoColor->addItem("rainbow");
    stereoColor->addItem("greyscale");

    stereoDispAlgo->addItem("ptGrey");
    stereoDispAlgo->addItem("openCV block matching");
    stereoDispAlgo->addItem("openCV semi-global block matching");

    connect(stereoUseForHeight, &QCheckBox::checkStateChanged, this, &StereoWidget::onStereoUseForHeightStateChanged);
    connect(
        stereoUseForHeightEver,
        &QCheckBox::checkStateChanged,
        this,
        &StereoWidget::onStereoUseForHeightEverStateChanged);
    connect(stereoShowDisparity, &QCheckBox::checkStateChanged, this, &StereoWidget::onStereoShowDisparityStateChanged);
    connect(
        stereoColor,
        QOverload<int>::of(&QComboBox::currentIndexChanged),
        this,
        &StereoWidget::onStereoColorCurrentIndexChanged);
    connect(
        stereoDispAlgo,
        QOverload<int>::of(&QComboBox::currentIndexChanged),
        this,
        &StereoWidget::onStereoDispAlgoCurrentIndexChanged);
    connect(hideWrong, &QCheckBox::checkStateChanged, this, &StereoWidget::onHideWrongStateChanged);
    connect(
        stereoMaskSize, QOverload<int>::of(&QSpinBox::valueChanged), this, &StereoWidget::onStereoMaskSizeValueChanged);
    connect(opacity, QOverload<int>::of(&QSpinBox::valueChanged), this, &StereoWidget::onOpacityValueChanged);
    connect(edgeMaskSize, QOverload<int>::of(&QSpinBox::valueChanged), this, &StereoWidget::onEdgeMaskSizeValueChanged);
    connect(useEdge, &QCheckBox::checkStateChanged, this, &StereoWidget::onUseEdgeStateChanged);
    connect(maxDisparity, QOverload<int>::of(&QSpinBox::valueChanged), this, &StereoWidget::onMaxDisparityValueChanged);
    connect(minDisparity, QOverload<int>::of(&QSpinBox::valueChanged), this, &StereoWidget::onMinDisparityValueChanged);
    connect(stereoExport, &QPushButton::clicked, this, &StereoWidget::onStereoExportClicked);
}

//---------------------------------------
//    <STEREO>
//        <DISPARITY SHOW="1" COLOR="1" OPACITY="50" HIDE_INVALID="1">
//            <VALUES MIN="30" MAX="50">
//            <MASK SIZE="7" EDGE_SIZE="5" USE_EDGE="1">
//            <USE RECO="0" HEIGHT="1" EXPORT="1">
//        </DISPARITY>
//    </STEREO>

// store data in xml node
void StereoWidget::setXml(QDomElement &elem)
{
    QDomElement subElem;
    QDomElement subSubElem;

    subElem = (elem.ownerDocument()).createElement("DISPARITY");
    subElem.setAttribute("SHOW", stereoShowDisparity->isChecked());
    subElem.setAttribute("COLOR", stereoColor->currentIndex());
    subElem.setAttribute("ALGO", stereoDispAlgo->currentIndex());
    subElem.setAttribute("OPACITY", opacity->value());
    subElem.setAttribute("HIDE_INVALID", hideWrong->isChecked());
    elem.appendChild(subElem);

    subSubElem = (elem.ownerDocument()).createElement("VALUES");
    subSubElem.setAttribute("MIN", minDisparity->value());
    subSubElem.setAttribute("MAX", maxDisparity->value());
    subElem.appendChild(subSubElem);

    subSubElem = (elem.ownerDocument()).createElement("MASK");
    subSubElem.setAttribute("SIZE", stereoMaskSize->value());
    subSubElem.setAttribute("EDGE_SIZE", edgeMaskSize->value());
    subSubElem.setAttribute("USE_EDGE", useEdge->isChecked());
    subElem.appendChild(subSubElem);

    subSubElem = (elem.ownerDocument()).createElement("USE");
    subSubElem.setAttribute("RECO", stereoUseForReco->isChecked());
    subSubElem.setAttribute("HEIGHT", stereoUseForHeight->isChecked());
    subSubElem.setAttribute("HEIGHT_EVER", stereoUseForHeightEver->isChecked());
    subSubElem.setAttribute("EXPORT", stereoUseForExport->isChecked());
    subSubElem.setAttribute("CALIB_CENTER", stereoUseCalibrationCenter->isChecked());
    subElem.appendChild(subSubElem);
}

// read data from xml node
void StereoWidget::getXml(QDomElement &elem)
{
    QDomElement subElem, subSubElem;

    for(subElem = elem.firstChildElement(); !subElem.isNull(); subElem = subElem.nextSiblingElement())
    {
        if(subElem.tagName() == "DISPARITY")
        {
            loadIntValue(subElem, "OPACITY", opacity, 100);
            loadBoolValue(subElem, "SHOW", stereoShowDisparity, false);
            loadActiveIndex(subElem, "COLOR", stereoColor, 0);
            loadActiveIndex(subElem, "ALGO", stereoDispAlgo, 0);
            loadBoolValue(subElem, "HIDE_INVALID", hideWrong, false);

            for(subSubElem = subElem.firstChildElement(); !subSubElem.isNull();
                subSubElem = subSubElem.nextSiblingElement())
            {
                if(subSubElem.tagName() == "VALUES")
                {
                    loadIntValue(subSubElem, "MIN", minDisparity, 0);
                    loadIntValue(subSubElem, "MAX", maxDisparity, 100);
                }
                else if(subSubElem.tagName() == "MASK")
                {
                    loadIntValue(subSubElem, "SIZE", stereoMaskSize, 7);
                    loadIntValue(subSubElem, "EDGE_SIZE", edgeMaskSize, 5);
                    loadBoolValue(subSubElem, "USE_EDGE", useEdge, false);
                }
                else if(subSubElem.tagName() == "USE")
                {
                    loadBoolValue(subSubElem, "RECO", stereoUseForReco, false);
                    loadBoolValue(subSubElem, "HEIGHT", stereoUseForHeight, false);
                    loadBoolValue(subSubElem, "HEIGHT_EVER", stereoUseForHeightEver, true);
                    loadBoolValue(subSubElem, "EXPORT", stereoUseForExport, false);
                    loadBoolValue(subSubElem, "CALIB_CENTER", stereoUseCalibrationCenter, true);
                }
                else
                {
                    SPDLOG_ERROR("Unknown STEREO tag: {}", subSubElem.tagName());
                }
            }
        }
    }
}

void StereoWidget::onStereoUseForHeightStateChanged()
{
    if(stereoUseForHeightEver->isChecked() && stereoUseForHeight->isChecked())
        mMainWindow->updateImage();
}

void StereoWidget::onStereoUseForHeightEverStateChanged()
{
    if(stereoUseForHeightEver->isChecked() && stereoUseForHeight->isChecked())
        mMainWindow->updateImage();
}

void StereoWidget::onStereoShowDisparityStateChanged(int i)
{
    mMainWindow->getStereoItem()->setVisible(i);
    mMainWindow->getScene()->update();
}

void StereoWidget::onStereoColorCurrentIndexChanged()
{
    if(mMainWindow && mMainWindow->getScene())
        mMainWindow->getScene()->update();
}

void StereoWidget::onStereoDispAlgoCurrentIndexChanged()
{
    if(mMainWindow && mMainWindow->getScene() && mMainWindow->getStereoContext())
    {
        mMainWindow->getStereoContext()->indicateNewValues();
        mMainWindow->getScene()->update();
    }
}

void StereoWidget::onHideWrongStateChanged()
{
    if(mMainWindow && mMainWindow->getScene() && mMainWindow->getStereoContext())
    {
        mMainWindow->getStereoContext()->indicateNewValues();
        mMainWindow->getScene()->update();
    }
}

void StereoWidget::onStereoMaskSizeValueChanged(int i)
{
    if(i % 2 == 0)
    {
        stereoMaskSize->setValue(i - 1);
        return;
    }
    if(mMainWindow->getStereoContext())
    {
        mMainWindow->getStereoContext()->indicateNewValues();
        mMainWindow->getScene()->update();
    }
}

void StereoWidget::onOpacityValueChanged()
{
    mMainWindow->getScene()->update();
}

void StereoWidget::onEdgeMaskSizeValueChanged(int i)
{
    if(i % 2 == 0)
    {
        edgeMaskSize->setValue(i - 1);
        return;
    }
    if(mMainWindow->getStereoContext() && !mMainWindow->isLoading())
    {
        mMainWindow->getStereoContext()->preprocess();
        mMainWindow->getStereoContext()->indicateNewValues();
        mMainWindow->getScene()->update();
    }
}

void StereoWidget::onUseEdgeStateChanged()
{
    if(mMainWindow->getStereoContext() && !mMainWindow->isLoading())
    {
        mMainWindow->getStereoContext()->preprocess();
        mMainWindow->getStereoContext()->indicateNewValues();
        mMainWindow->getScene()->update();
    }
}

void StereoWidget::onMaxDisparityValueChanged()
{
    if(mMainWindow->getStereoContext())
    {
        mMainWindow->getStereoContext()->indicateNewValues();
        mMainWindow->getScene()->update();
    }
}

void StereoWidget::onMinDisparityValueChanged()
{
    if(mMainWindow->getStereoContext())
    {
        mMainWindow->getStereoContext()->indicateNewValues();
        mMainWindow->getScene()->update();
    }
}

void StereoWidget::onStereoExportClicked()
{
    mMainWindow->getStereoContext()->exportPointCloud();
}

#include "moc_stereoWidget.cpp"
