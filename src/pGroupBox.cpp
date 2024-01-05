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

#include "pGroupBox.h"

#include "ui_pGroupBox.h"

PGroupBox::PGroupBox(QWidget *parent, const QString &title, QWidget *content) : QWidget(parent), mUi(new Ui::PGroupBox)
{
    mUi->setupUi(this);
    mUi->title->setText(title);

    connect(mUi->immutable, &QCheckBox::stateChanged, mUi->frame, &QFrame::setDisabled);
    mUi->frame->layout()->addWidget(content);
}

PGroupBox::~PGroupBox()
{
    delete mUi;
}

void PGroupBox::setImmutable(bool immutable)
{
    mUi->immutable->setChecked(immutable);
}

bool PGroupBox::isImmutable()
{
    return mUi->immutable->isChecked();
}
