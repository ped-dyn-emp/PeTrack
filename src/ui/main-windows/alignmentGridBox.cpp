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

#include "alignmentGridBox.h"

#include "importHelper.h"
#include "pGroupBox.h"
#include "ui_alignmentGridBox.h"

#include <QDomElement>

AlignmentGridBox::AlignmentGridBox(QWidget *parent) : QWidget(parent), mUi(new Ui::AlignmentGridBox)
{
    mUi->setupUi(this);

    setFocusProxy(mUi->gridShow);

    connect(mUi->gridShow, &QCheckBox::checkStateChanged, this, &AlignmentGridBox::onGridShowStateChanged);
    connect(mUi->gridFix, &QCheckBox::checkStateChanged, this, &AlignmentGridBox::onGridFixStateChanged);
    connect(mUi->gridRotate, &PSlider::valueChanged, this, &AlignmentGridBox::onGridRotateValueChanged);
    connect(mUi->gridTransX, &PSlider::valueChanged, this, &AlignmentGridBox::onGridTransXValueChanged);
    connect(mUi->gridTransY, &PSlider::valueChanged, this, &AlignmentGridBox::onGridTransYValueChanged);
    connect(mUi->gridScale, &PSlider::valueChanged, this, &AlignmentGridBox::onGridScaleValueChanged);
    connect(mUi->gridTab, &QTabWidget::currentChanged, this, &AlignmentGridBox::onGridTabCurrentChanged);
    connect(mUi->grid3DTransX, &PSlider::valueChanged, this, &AlignmentGridBox::onGrid3DTransXValueChanged);
    connect(mUi->grid3DTransY, &PSlider::valueChanged, this, &AlignmentGridBox::onGrid3DTransYValueChanged);
    connect(mUi->grid3DTransZ, &PSlider::valueChanged, this, &AlignmentGridBox::onGrid3DTransZValueChanged);
    connect(mUi->grid3DResolution, &PSlider::valueChanged, this, &AlignmentGridBox::onGrid3DResolutionValueChanged);
}

AlignmentGridBox::~AlignmentGridBox()
{
    delete mUi;
}

Grid AlignmentGridBox::getGridParameters()
{
    return assembleGrid();
}

bool AlignmentGridBox::isShow()
{
    return mUi->gridShow->isChecked();
}

void AlignmentGridBox::setShow(bool b)
{
    mUi->gridShow->setChecked(b);
}

bool AlignmentGridBox::isFix()
{
    return mUi->gridFix->isChecked();
}

void AlignmentGridBox::setFix(bool b)
{
    mUi->gridFix->setChecked(b);
}


void AlignmentGridBox::setTrans3DX(double x)
{
    mUi->grid3DTransX->setValue(x);
}

void AlignmentGridBox::setTrans3DY(double y)
{
    mUi->grid3DTransY->setValue(y);
}

void AlignmentGridBox::setResolution(double res)
{
    mUi->grid3DResolution->setValue(res);
}

void AlignmentGridBox::setTrans2DX(double x)
{
    mUi->gridTransX->setValue(x);
}

void AlignmentGridBox::setTrans2DY(double y)
{
    mUi->gridTransY->setValue(y);
}

void AlignmentGridBox::setScale(double scale)
{
    mUi->gridScale->setValue(scale);
}

void AlignmentGridBox::setRotation(double angle)
{
    mUi->gridRotate->setValue(angle);
}

void AlignmentGridBox::setXml(QDomElement &subSubElem)
{
    subSubElem.setAttribute("GRID_DIMENSION", mUi->gridTab->currentIndex());
    subSubElem.setAttribute("SHOW", mUi->gridShow->isChecked());
    subSubElem.setAttribute("FIX", mUi->gridFix->isChecked());
    subSubElem.setAttribute("ROTATE", mUi->gridRotate->value());
    subSubElem.setAttribute("TRANS_X", mUi->gridTransX->value());
    subSubElem.setAttribute("TRANS_Y", mUi->gridTransY->value());
    subSubElem.setAttribute("SCALE", mUi->gridScale->value());
    subSubElem.setAttribute("GRID3D_TRANS_X", mUi->grid3DTransX->value());
    subSubElem.setAttribute("GRID3D_TRANS_Y", mUi->grid3DTransY->value());
    subSubElem.setAttribute("GRID3D_TRANS_Z", mUi->grid3DTransZ->value());
    subSubElem.setAttribute("GRID3D_RESOLUTION", mUi->grid3DResolution->value());

    if(this->parent())
    {
        auto *parent = dynamic_cast<PGroupBox *>(this->parent()->parent());
        if(parent)
        {
            subSubElem.setAttribute("IMMUTABLE", parent->isImmutable());
        }
    }
}

bool AlignmentGridBox::getXml(QDomElement &subSubElem)
{
    if(subSubElem.tagName() == "ALIGNMENT_GRID")
    {
        loadActiveIndex(subSubElem, "GRID_DIMENSION", mUi->gridTab, 1);
        loadBoolValue(subSubElem, "SHOW", mUi->gridShow, false);
        loadBoolValue(subSubElem, "FIX", mUi->gridFix, false);
        loadIntValue(subSubElem, "ROTATE", mUi->gridRotate, 0);
        loadIntValue(subSubElem, "TRANS_X", mUi->gridTransX, 0);
        loadIntValue(subSubElem, "TRANS_Y", mUi->gridTransY, 0);
        loadIntValue(subSubElem, "SCALE", mUi->gridScale, 100);
        loadIntValue(subSubElem, "GRID3D_TRANS_X", mUi->grid3DTransX, 0);
        loadIntValue(subSubElem, "GRID3D_TRANS_Y", mUi->grid3DTransY, 0);
        loadIntValue(subSubElem, "GRID3D_TRANS_Z", mUi->grid3DTransZ, 0);
        loadIntValue(subSubElem, "GRID3D_RESOLUTION", mUi->grid3DResolution, 100);

        if(subSubElem.hasAttribute("IMMUTABLE"))
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
        return true;
    }
    return false;
}

Grid2D AlignmentGridBox::assemble2DGrid()
{
    return Grid2D{
        Vec2F(mUi->gridTransX->value(), mUi->gridTransY->value()), mUi->gridRotate->value(), mUi->gridScale->value()};
}

Grid3D AlignmentGridBox::assemble3DGrid()
{
    return Grid3D{
        Vec3F(mUi->grid3DTransX->value(), mUi->grid3DTransY->value(), mUi->grid3DTransZ->value()),
        mUi->grid3DResolution->value()};
}

Grid AlignmentGridBox::assembleGrid()
{
    if(mUi->gridTab->currentIndex() == 0)
    {
        return assemble3DGrid();
    }
    else
    {
        return assemble2DGrid();
    }
}

void AlignmentGridBox::onGridTabCurrentChanged()
{
    emit gridChanged();
}

void AlignmentGridBox::onGridShowStateChanged()
{
    if(!isShow())
    {
        setFix(true);
    }
    emit gridChanged();
}

void AlignmentGridBox::onGridFixStateChanged()
{
    emit gridChanged();
}

void AlignmentGridBox::onGridRotateValueChanged()
{
    emit gridChanged();
}

void AlignmentGridBox::onGridTransXValueChanged()
{
    emit gridChanged();
}

void AlignmentGridBox::onGridTransYValueChanged()
{
    emit gridChanged();
}

void AlignmentGridBox::onGridScaleValueChanged()
{
    emit gridChanged();
}

void AlignmentGridBox::onGrid3DTransXValueChanged()
{
    emit gridChanged();
}

void AlignmentGridBox::onGrid3DTransYValueChanged()
{
    emit gridChanged();
}

void AlignmentGridBox::onGrid3DTransZValueChanged()
{
    emit gridChanged();
}

void AlignmentGridBox::onGrid3DResolutionValueChanged()
{
    emit gridChanged();
}
