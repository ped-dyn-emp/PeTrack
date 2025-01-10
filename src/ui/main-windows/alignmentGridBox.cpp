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

#include "alignmentGridBox.h"

#include "importHelper.h"
#include "pGroupBox.h"
#include "ui_alignmentGridBox.h"

#include <QDomElement>

AlignmentGridBox::AlignmentGridBox(QWidget *parent) : QWidget(parent), mUi(new Ui::AlignmentGridBox)
{
    mUi->setupUi(this);

    setFocusProxy(mUi->gridShow);
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

bool AlignmentGridBox::isFix()
{
    return mUi->gridFix->isChecked();
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

void AlignmentGridBox::on_gridTab_currentChanged(int /*index*/)
{
    emit gridChanged();
}

void AlignmentGridBox::on_gridShow_stateChanged(int /*i*/)
{
    emit gridChanged();
}

void AlignmentGridBox::on_gridFix_stateChanged(int /*i*/)
{
    emit gridChanged();
}

void AlignmentGridBox::on_gridRotate_valueChanged(int /*i*/)
{
    emit gridChanged();
}

void AlignmentGridBox::on_gridTransX_valueChanged(int /*i*/)
{
    emit gridChanged();
}

void AlignmentGridBox::on_gridTransY_valueChanged(int /*i*/)
{
    emit gridChanged();
}

void AlignmentGridBox::on_gridScale_valueChanged(int /*i*/)
{
    emit gridChanged();
}

void AlignmentGridBox::on_grid3DTransX_valueChanged(int /*value*/)
{
    emit gridChanged();
}

void AlignmentGridBox::on_grid3DTransY_valueChanged(int /*value*/)
{
    emit gridChanged();
}

void AlignmentGridBox::on_grid3DTransZ_valueChanged(int /*value*/)
{
    emit gridChanged();
}

void AlignmentGridBox::on_grid3DResolution_valueChanged(int /*value*/)
{
    emit gridChanged();
}
