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

#include "filterBeforeBox.h"

#include "backgroundFilter.h"
#include "backgroundItem.h"
#include "borderFilter.h"
#include "brightContrastFilter.h"
#include "importHelper.h"
#include "pGroupBox.h"
#include "pMessageBox.h"
#include "swapFilter.h"
#include "ui_filterBeforeBox.h"

#include <QColorDialog>
#include <QDomElement>

FilterBeforeBox::FilterBeforeBox(
    QWidget              *parent,
    BackgroundFilter     &bgFilter,
    BrightContrastFilter &brightContrastFilter,
    BorderFilter         &borderFilter,
    SwapFilter           &swapFilter,
    std::function<void()> updateImageCallback) :
    QWidget(parent),
    mUi(new Ui::FilterBeforeBox),
    mUpdateImageCallback(std::move(updateImageCallback)),
    mBgFilter(bgFilter),
    mBrightContrastFilter(brightContrastFilter),
    mBorderFilter(borderFilter),
    mSwapFilter(swapFilter)
{
    mUi->setupUi(this);
    mShowBackgroundCache = mUi->filterBgShow->checkState();
    toggleBackgroundUi(mUi->filterBg->checkState());

    // FocusPolicy: TabFocus and first ui-element as proxy are needed for tab order
    setFocusProxy(mUi->filterBrightContrast);

    connect(
        mUi->filterBrightContrast,
        &QCheckBox::checkStateChanged,
        this,
        &FilterBeforeBox::onFilterBrightContrastStateChanged);
    connect(
        mUi->filterContrastParam, &PSlider::valueChanged, this, &FilterBeforeBox::onFilterContrastParamValueChanged);
    connect(mUi->filterBrightParam, &PSlider::valueChanged, this, &FilterBeforeBox::onFilterBrightParamValueChanged);
    connect(mUi->filterBorder, &QCheckBox::checkStateChanged, this, &FilterBeforeBox::onFilterBorderStateChanged);
    connect(
        mUi->filterBorderParamSize,
        &PSlider::valueChanged,
        this,
        &FilterBeforeBox::onFilterBorderParamSizeValueChanged);
    connect(mUi->filterBorderParamCol, &QPushButton::clicked, this, &FilterBeforeBox::onFilterBorderParamColClicked);
    connect(mUi->filterBg, &QCheckBox::checkStateChanged, this, &FilterBeforeBox::onFilterBgStateChanged);
    connect(mUi->filterBgShow, &QCheckBox::checkStateChanged, this, &FilterBeforeBox::onFilterBgShowStateChanged);
    connect(mUi->filterBgUpdate, &QCheckBox::checkStateChanged, this, &FilterBeforeBox::onFilterBgUpdateStateChanged);
    connect(mUi->filterBgReset, &QPushButton::clicked, this, &FilterBeforeBox::onFilterBgResetClicked);
    connect(mUi->filterBgSave, &QPushButton::clicked, this, &FilterBeforeBox::onFilterBgSaveClicked);
    connect(mUi->filterBgLoad, &QPushButton::clicked, this, &FilterBeforeBox::onFilterBgLoadClicked);
    connect(mUi->filterSwap, &QCheckBox::checkStateChanged, this, &FilterBeforeBox::onFilterSwapStateChanged);
    connect(mUi->filterSwapH, &QCheckBox::checkStateChanged, this, &FilterBeforeBox::onFilterSwapHStateChanged);
    connect(mUi->filterSwapV, &QCheckBox::checkStateChanged, this, &FilterBeforeBox::onFilterSwapVStateChanged);
}

FilterBeforeBox::~FilterBeforeBox()
{
    delete mUi;
}

/// BackgroundItem is created after Control, but some Items need Control to work
/// Since we do not want to separate the initializations too much, we use a non-owning
/// pointer and setter approach
void FilterBeforeBox::setBackgroundItem(BackgroundItem *item)
{
    mBgItem = item;
}

void FilterBeforeBox::setFilterSettings(const FilterSettings &settings)
{
    mUi->filterBrightContrast->setChecked(settings.useBrightContrast);
    mUi->filterBorder->setChecked(settings.useBorder);
    mUi->filterBg->setChecked(settings.useBackground);

    mUi->filterSwap->setChecked(settings.useSwap);
    mUi->filterSwapH->setChecked(settings.useSwapH);
    mUi->filterSwapV->setChecked(settings.useSwapV);
}

/// return false if element was not handled (completely)
bool FilterBeforeBox::getXmlSub(QDomElement &subSubElem)
{
    if(subSubElem.tagName() == "FILTER_BEFORE")
    {
        if(this->parent())
        {
            auto *parent = dynamic_cast<PGroupBox *>(this->parent()->parent());
            if(parent)
            {
                parent->setImmutable(readBool(subSubElem, "IMMUTABLE", false));
            }
        }
    }
    else if(subSubElem.tagName() == "BRIGHTNESS")
    {
        loadBoolValue(subSubElem, "ENABLED", mUi->filterBrightContrast);
        loadIntValue(subSubElem, "VALUE", mUi->filterBrightParam);
    }
    else if(subSubElem.tagName() == "CONTRAST")
    {
        loadBoolValue(subSubElem, "ENABLED", mUi->filterBrightContrast);
        loadIntValue(subSubElem, "VALUE", mUi->filterContrastParam);
    }
    else if(subSubElem.tagName() == "BORDER")
    {
        loadBoolValue(subSubElem, "ENABLED", mUi->filterBorder);
        loadIntValue(subSubElem, "VALUE", mUi->filterBorderParamSize);
        // bgColor still needs to be read by control
        return false;
    }
    else if(subSubElem.tagName() == "SWAP")
    {
        loadBoolValue(subSubElem, "ENABLED", mUi->filterSwap);
        loadBoolValue(subSubElem, "HORIZONTALLY", mUi->filterSwapH);
        loadBoolValue(subSubElem, "VERTICALLY", mUi->filterSwapV);
    }
    else if(subSubElem.tagName() == "BG_SUB")
    {
        loadBoolValue(subSubElem, "ENABLED", mUi->filterBg);
        loadBoolValue(subSubElem, "UPDATE", mUi->filterBgUpdate, false);
        loadBoolValue(subSubElem, "SHOW", mUi->filterBgShow, false);
        loadBoolValue(subSubElem, "DELETE", mUi->filterBgDeleteTrj, true);
        loadIntValue(subSubElem, "DELETE_NUMBER", mUi->filterBgDeleteNumber, 3);
        // control still needs to read file
        return false;
    }
    else
    {
        return false;
    }
    return true;
}

void FilterBeforeBox::setXml(QDomElement &subElem, QColor bgColor, const QString &bgFilename) const
{
    QDomElement subSubElem;

    if(this->parent())
    {
        auto *parent = dynamic_cast<PGroupBox *>(this->parent()->parent());
        if(parent)
        {
            subSubElem = (subElem.ownerDocument()).createElement("FILTER_BEFORE");
            subSubElem.setAttribute("IMMUTABLE", parent->isImmutable());
            subElem.appendChild(subSubElem);
        }
    }

    subSubElem = (subElem.ownerDocument()).createElement("BRIGHTNESS");
    subSubElem.setAttribute("ENABLED", mUi->filterBrightContrast->isChecked());
    subSubElem.setAttribute("VALUE", mUi->filterBrightParam->value());
    subElem.appendChild(subSubElem);

    subSubElem = (subElem.ownerDocument()).createElement("CONTRAST");
    subSubElem.setAttribute("ENABLED", mUi->filterBrightContrast->isChecked());
    subSubElem.setAttribute("VALUE", mUi->filterContrastParam->value());
    subElem.appendChild(subSubElem);


    subSubElem = (subElem.ownerDocument()).createElement("BORDER");
    subSubElem.setAttribute("ENABLED", mUi->filterBorder->isChecked());
    subSubElem.setAttribute("VALUE", mUi->filterBorderParamSize->value());
    subSubElem.setAttribute("COLOR", bgColor.name());
    subElem.appendChild(subSubElem);

    subSubElem = (subElem.ownerDocument()).createElement("SWAP");
    subSubElem.setAttribute("ENABLED", mUi->filterSwap->isChecked());
    subSubElem.setAttribute("HORIZONTALLY", mUi->filterSwapH->isChecked());
    subSubElem.setAttribute("VERTICALLY", mUi->filterSwapV->isChecked());
    subElem.appendChild(subSubElem);

    subSubElem = (subElem.ownerDocument()).createElement("BG_SUB");
    subSubElem.setAttribute("ENABLED", isFilterBgChecked());
    subSubElem.setAttribute("UPDATE", mUi->filterBgUpdate->isChecked());
    subSubElem.setAttribute("SHOW", mUi->filterBgShow->isChecked());
    subSubElem.setAttribute("FILE", bgFilename);
    subSubElem.setAttribute("DELETE", mUi->filterBgDeleteTrj->isChecked());
    subSubElem.setAttribute("DELETE_NUMBER", mUi->filterBgDeleteNumber->value());
    subElem.appendChild(subSubElem);
}

int FilterBeforeBox::getFilterBorderSize() const
{
    return mUi->filterBorderParamSize->value();
}
void FilterBeforeBox::setFilterBorderSizeMin(int i)
{
    mUi->filterBorderParamSize->setMinimum(i);
    mUi->filterBorderParamSize_spin->setMinimum(i);
}
void FilterBeforeBox::setFilterBorderSizeMax(int i)
{
    mUi->filterBorderParamSize->setMaximum(i);
    mUi->filterBorderParamSize_spin->setMaximum(i);
}

bool FilterBeforeBox::isFilterBgChecked() const
{
    return mUi->filterBg->isChecked();
}

bool FilterBeforeBox::isFilterBgDeleteTrjChecked() const
{
    return mUi->filterBgDeleteTrj->isChecked();
}

int FilterBeforeBox::getFilterBgDeleteNumber() const
{
    return mUi->filterBgDeleteNumber->value();
}

void FilterBeforeBox::toggleBackgroundUi(Qt::CheckState state)
{
    if(state == Qt::Checked)
    {
        mUi->filterBgShow->setEnabled(true);
        mUi->filterBgUpdate->setEnabled(true);
        mUi->filterBgReset->setEnabled(true);
        mUi->filterBgSave->setEnabled(true);
        mUi->filterBgLoad->setEnabled(true);
        if(mShowBackgroundCache)
        {
            mUi->filterBgShow->setCheckState(Qt::Checked);
        }
        mUi->filterBgDeleteNumber->setEnabled(true);
        mUi->filterBgDeleteTrj->setEnabled(true);
        mUi->label_65->setEnabled(true);
    }
    else if(state == Qt::Unchecked)
    {
        mUi->filterBgShow->setEnabled(false);
        mUi->filterBgUpdate->setEnabled(false);
        mUi->filterBgReset->setEnabled(false);
        mUi->filterBgSave->setEnabled(false);
        mUi->filterBgLoad->setEnabled(false);
        mShowBackgroundCache = mUi->filterBgShow->isChecked();
        mUi->filterBgShow->setCheckState(Qt::Unchecked);
        mUi->filterBgDeleteNumber->setEnabled(false);
        mUi->filterBgDeleteTrj->setEnabled(false);
        mUi->label_65->setEnabled(false);
    }
}

void FilterBeforeBox::onFilterBrightContrastStateChanged(int i)
{
    if(i == Qt::Checked)
    {
        mBrightContrastFilter.enable();
    }
    else if(i == Qt::Unchecked)
    {
        mBrightContrastFilter.disable();
    }
    mUpdateImageCallback();
}

void FilterBeforeBox::onFilterContrastParamValueChanged(int i)
{
    mBrightContrastFilter.getContrast().setValue(i);
    mUpdateImageCallback();
}

void FilterBeforeBox::onFilterBrightParamValueChanged(int i)
{
    mBrightContrastFilter.getBrightness().setValue(i);
    mUpdateImageCallback();
}

void FilterBeforeBox::onFilterBorderStateChanged(int i)
{
    if(i == Qt::Checked)
    {
        mBorderFilter.enable();
    }
    else if(i == Qt::Unchecked)
    {
        mBorderFilter.disable();
    }
    mUpdateImageCallback();
}

void FilterBeforeBox::onFilterBorderParamSizeValueChanged(int i)
{
    // 2* because undistored has problem with sizes not dividable  of 4
    mBorderFilter.getBorderSize().setValue(2 * i);
    mUpdateImageCallback();
}

void FilterBeforeBox::onFilterBorderParamColClicked()
{
    QColor color = QColorDialog::getColor(
        QColor(
            mBorderFilter.getBorderColR().getValue(),
            mBorderFilter.getBorderColG().getValue(),
            mBorderFilter.getBorderColB().getValue()),
        this);
    mBorderFilter.getBorderColR().setValue(color.red());
    mBorderFilter.getBorderColG().setValue(color.green());
    mBorderFilter.getBorderColB().setValue(color.blue());
    mUpdateImageCallback();
}

void FilterBeforeBox::onFilterSwapStateChanged(int i)
{
    if(i == Qt::Checked)
    {
        mSwapFilter.enable();
    }
    else if(i == Qt::Unchecked)
    {
        mSwapFilter.disable();
    }
    mUpdateImageCallback();
}

void FilterBeforeBox::onFilterSwapHStateChanged(int i)
{
    mSwapFilter.getSwapHorizontally().setValue(i == Qt::Checked);

    mUpdateImageCallback();
}

void FilterBeforeBox::onFilterSwapVStateChanged(int i)
{
    mSwapFilter.getSwapVertically().setValue(i == Qt::Checked);

    mUpdateImageCallback();
}

void FilterBeforeBox::onFilterBgStateChanged(int i)
{
    toggleBackgroundUi(static_cast<Qt::CheckState>(i));
    if(i == Qt::Checked)
    {
        mBgFilter.enable();
    }
    else
    {
        mBgFilter.disable();
    }
    mUpdateImageCallback();
}

void FilterBeforeBox::onFilterBgUpdateStateChanged(int i)
{
    if(i == Qt::Checked)
    {
        mBgFilter.setUpdate(true);
    }
    else if(i == Qt::Unchecked)
    {
        mBgFilter.setUpdate(false);
    }
}

void FilterBeforeBox::onFilterBgResetClicked()
{
    mBgFilter.reset();
    mUpdateImageCallback();
}

void FilterBeforeBox::onFilterBgShowStateChanged(int i)
{
    if(mBgItem)
    {
        mBgItem->setVisible(i);
        mUpdateImageCallback();
    }
}

void FilterBeforeBox::onFilterBgSaveClicked()
{
    mBgFilter.save();
}

void FilterBeforeBox::onFilterBgLoadClicked()
{
    mBgFilter.load();
    mUpdateImageCallback();
}
