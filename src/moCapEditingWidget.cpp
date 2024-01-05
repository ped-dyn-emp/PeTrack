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

#include "moCapEditingWidget.h"

#include "moCapPerson.h"
#include "ui_moCapEditingWidget.h"

#include <utility>

MoCapEditingWidget::MoCapEditingWidget(QWidget *parent, MoCapPerson &moCapPerson, std::function<void()> updateOverlay) :
    QFrame(parent), mUi(new Ui::MoCapEditingWidget), mPerson(moCapPerson), mUpdateOverlay(std::move(updateOverlay))
{
    mUi->setupUi(this);
    mUi->fileName->setText(QString::fromStdString(mPerson.getFilename()));
    mUi->timeOffset->setValue(mPerson.getMetadata().getUserTimeOffset());
    mUi->showPerson->setChecked(mPerson.isVisible());
    mUi->angle->setValue(mPerson.getMetadata().getAngle());
    auto trans = mPerson.getMetadata().getTranslation().translation();
    mUi->trans_x->setValue(trans[0]);
    mUi->trans_y->setValue(trans[1]);
    mUi->trans_z->setValue(trans[2]);

    connect(
        mUi->timeOffset,
        qOverload<double>(&PDoubleSpinBox::valueChanged),
        this,
        &MoCapEditingWidget::onTimeOffsetChanged);
    connect(mUi->showPerson, &QCheckBox::stateChanged, this, &MoCapEditingWidget::onVisibleChanged);
    connect(mUi->angle, qOverload<double>(&PDoubleSpinBox::valueChanged), this, &MoCapEditingWidget::onRotationChanged);
    connect(
        mUi->trans_x,
        qOverload<double>(&PDoubleSpinBox::valueChanged),
        this,
        &MoCapEditingWidget::onTranslationChanged);
    connect(
        mUi->trans_y,
        qOverload<double>(&PDoubleSpinBox::valueChanged),
        this,
        &MoCapEditingWidget::onTranslationChanged);
    connect(
        mUi->trans_z,
        qOverload<double>(&PDoubleSpinBox::valueChanged),
        this,
        &MoCapEditingWidget::onTranslationChanged);

    setMinimumSize(mUi->mainLayout->minimumSize());
}

MoCapEditingWidget::~MoCapEditingWidget()
{
    delete mUi;
}

void MoCapEditingWidget::select()
{
    mUi->showPerson->setChecked(true);
}

void MoCapEditingWidget::deselect()
{
    mUi->showPerson->setChecked(false);
}

void MoCapEditingWidget::onTimeOffsetChanged(double newOffset)
{
    mPerson.setUserTimeOffset(newOffset);
    mUpdateOverlay();
}

void MoCapEditingWidget::onVisibleChanged(int newState)
{
    mPerson.setVisible(newState == Qt::Checked);
    mUpdateOverlay();
}

void MoCapEditingWidget::onTranslationChanged(double /*newVal*/)
{
    cv::Vec3f trans(mUi->trans_x->value(), mUi->trans_y->value(), mUi->trans_z->value());
    mPerson.setTranslation(trans);
    mUpdateOverlay();
}

void MoCapEditingWidget::onRotationChanged(double newAngle)
{
    mPerson.setRotation(newAngle);
    mUpdateOverlay();
}
