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

#include "filterBeforeBox.h"

#include "backgroundFilter.h"
#include "backgroundItem.h"
#include "borderFilter.h"
#include "brightContrastFilter.h"
#include "helper.h"
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
                parent->setImmutable(subSubElem.attribute("IMMUTABLE").toInt());
            }
        }
    }
    else if(subSubElem.tagName() == "BRIGHTNESS")
    {
        if(subSubElem.hasAttribute("ENABLED"))
        {
            mUi->filterBrightContrast->setCheckState(
                subSubElem.attribute("ENABLED").toInt() ? Qt::Checked : Qt::Unchecked);
        }
        if(subSubElem.hasAttribute("VALUE"))
        {
            mUi->filterBrightParam->setValue(subSubElem.attribute("VALUE").toInt());
        }
    }
    else if(subSubElem.tagName() == "CONTRAST")
    {
        if(subSubElem.hasAttribute("ENABLED"))
        {
            mUi->filterBrightContrast->setCheckState(
                subSubElem.attribute("ENABLED").toInt() ? Qt::Checked : Qt::Unchecked);
        }
        if(subSubElem.hasAttribute("VALUE"))
        {
            mUi->filterContrastParam->setValue(subSubElem.attribute("VALUE").toInt());
        }
    }
    else if(subSubElem.tagName() == "BORDER")
    {
        if(subSubElem.hasAttribute("ENABLED"))
        {
            mUi->filterBorder->setCheckState(subSubElem.attribute("ENABLED").toInt() ? Qt::Checked : Qt::Unchecked);
        }
        if(subSubElem.hasAttribute("VALUE"))
        {
            setValue(mUi->filterBorderParamSize, subSubElem.attribute("VALUE").toInt());
        }
        // bgColor still needs to be read by control
        return false;
    }
    else if(subSubElem.tagName() == "SWAP")
    {
        if(subSubElem.hasAttribute("ENABLED"))
        {
            mUi->filterSwap->setCheckState(subSubElem.attribute("ENABLED").toInt() ? Qt::Checked : Qt::Unchecked);
        }
        if(subSubElem.hasAttribute("HORIZONTALLY"))
        {
            mUi->filterSwapH->setCheckState(subSubElem.attribute("HORIZONTALLY").toInt() ? Qt::Checked : Qt::Unchecked);
        }
        if(subSubElem.hasAttribute("VERTICALLY"))
        {
            mUi->filterSwapV->setCheckState(subSubElem.attribute("VERTICALLY").toInt() ? Qt::Checked : Qt::Unchecked);
        }
    }
    else if(subSubElem.tagName() == "BG_SUB")
    {
        if(subSubElem.hasAttribute("ENABLED"))
        {
            mUi->filterBg->setCheckState(subSubElem.attribute("ENABLED").toInt() ? Qt::Checked : Qt::Unchecked);
        }
        if(subSubElem.hasAttribute("UPDATE"))
        {
            mUi->filterBgUpdate->setCheckState(subSubElem.attribute("UPDATE").toInt() ? Qt::Checked : Qt::Unchecked);
        }
        if(subSubElem.hasAttribute("SHOW"))
        {
            mUi->filterBgShow->setCheckState(subSubElem.attribute("SHOW").toInt() ? Qt::Checked : Qt::Unchecked);
        }
        if(subSubElem.hasAttribute("DELETE"))
        {
            mUi->filterBgDeleteTrj->setCheckState(subSubElem.attribute("DELETE").toInt() ? Qt::Checked : Qt::Unchecked);
        }
        if(subSubElem.hasAttribute("DELETE_NUMBER"))
        {
            mUi->filterBgDeleteNumber->setValue(subSubElem.attribute("DELETE_NUMBER").toInt());
        }
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

void FilterBeforeBox::on_filterBrightContrast_stateChanged(int i)
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

void FilterBeforeBox::on_filterContrastParam_valueChanged(int i)
{
    mBrightContrastFilter.getContrast().setValue(i);
    mUpdateImageCallback();
}

void FilterBeforeBox::on_filterBrightParam_valueChanged(int i)
{
    mBrightContrastFilter.getBrightness().setValue(i);
    mUpdateImageCallback();
}

void FilterBeforeBox::on_filterBorder_stateChanged(int i)
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

void FilterBeforeBox::on_filterBorderParamSize_valueChanged(int i)
{
    // 2* because undistored has problem with sizes not dividable  of 4
    mBorderFilter.getBorderSize().setValue(2 * i);
    mUpdateImageCallback();
}

void FilterBeforeBox::on_filterBorderParamCol_clicked()
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

void FilterBeforeBox::on_filterSwap_stateChanged(int i)
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

void FilterBeforeBox::on_filterSwapH_stateChanged(int i)
{
    mSwapFilter.getSwapHorizontally().setValue(i == Qt::Checked);

    mUpdateImageCallback();
}

void FilterBeforeBox::on_filterSwapV_stateChanged(int i)
{
    mSwapFilter.getSwapVertically().setValue(i == Qt::Checked);

    mUpdateImageCallback();
}

void FilterBeforeBox::on_filterBg_stateChanged(int i)
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

void FilterBeforeBox::on_filterBgUpdate_stateChanged(int i)
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

void FilterBeforeBox::on_filterBgReset_clicked()
{
    mBgFilter.reset();
    mUpdateImageCallback();
}

void FilterBeforeBox::on_filterBgShow_stateChanged(int i)
{
    if(mBgItem)
    {
        mBgItem->setVisible(i);
        mUpdateImageCallback();
    }
}

void FilterBeforeBox::on_filterBgSave_clicked()
{
    mBgFilter.save();
}

void FilterBeforeBox::on_filterBgLoad_clicked()
{
    mBgFilter.load();
    mUpdateImageCallback();
}
