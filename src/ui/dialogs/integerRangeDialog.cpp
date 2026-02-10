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


#include "integerRangeDialog.h"

#include <QDialogButtonBox>
#include <QLabel>
#include <QSpinBox>
#include <QVBoxLayout>

IntegerRangeDialog::IntegerRangeDialog(int min, int max, const QString &title, QWidget *parent) :
    QDialog(parent), mSpin1(new QSpinBox(this)), mSpin2(new QSpinBox(this))
{
    auto *layout = new QVBoxLayout(this);
    layout->setSizeConstraint(QLayout::SetFixedSize);

    auto *titleLabel = new QLabel(title, this);
    titleLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(titleLabel);

    auto *spinLayout = new QHBoxLayout();
    mSpin1->setValue(min);
    mSpin2->setValue(max);

    mSpin1->setMinimum(min);
    mSpin1->setMaximum(max);

    mSpin2->setMinimum(min);
    mSpin2->setMaximum(max);

    spinLayout->addWidget(mSpin1);
    spinLayout->addWidget(mSpin2);
    layout->addLayout(spinLayout);

    connect(
        mSpin1,
        &QSpinBox::valueChanged,
        this,
        [this](int value)
        {
            mSpin2->setMinimum(value);

            if(mSpin2->value() < value)
            {
                mSpin2->setValue(value);
            }
        }); // make sure mSpin1 is always the lower bound of mSpin2

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);

    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    layout->addWidget(buttons);
}


int IntegerRangeDialog::min() const
{
    return mSpin1->value();
}

int IntegerRangeDialog::max() const
{
    return mSpin2->value();
}
