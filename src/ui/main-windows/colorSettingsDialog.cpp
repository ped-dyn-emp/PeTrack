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

#include "colorSettingsDialog.h"

#include "moCapController.h"
#include "ui_colorSettingsDialog.h"

#include <QColorDialog>
#include <QPushButton>

namespace
{
QColor getButtonColor(const QPushButton *button)
{
    return button->palette().color(QPalette::Button);
}

void setButtonColor(QPushButton *button, const QColor &col)
{
    QPalette pal = button->palette();
    pal.setColor(QPalette::Button, col);
    button->setPalette(pal);
}
} // namespace

ColorSettingsDialog::ColorSettingsDialog(QWidget *parent) : QDialog(parent), mUi(new Ui::ColorSettingsDialog)
{
    mUi->setupUi(this);

    // default colors
    setButtonColor(mUi->trackPathButton, DEFAULT_TRACK_PATH_COLOR);
    setButtonColor(mUi->trackGroundPathButton, DEFAULT_GROUND_PATH_COLOR);
    setButtonColor(mUi->trackPersonNumberButton, DEFAULT_PERSON_NUMBER_COLOR);
    setButtonColor(mUi->trackRecognizedButton, DEFAULT_RECOGNIZED_COLOR);
    setButtonColor(mUi->trackTrackedButton, DEFAULT_TRACKED_COLOR);
    setButtonColor(mUi->moCapButton, MoCapController::DEFAULT_COLOR);

    for(auto *button : {
            mUi->trackPathButton,
            mUi->trackGroundPathButton,
            mUi->trackPersonNumberButton,
            mUi->trackRecognizedButton,
            mUi->trackTrackedButton,
        })
    {
        connect(
            button,
            &QPushButton::clicked,
            this,
            [this, button]()
            {
                const QColor col = pickColor(button);
                if(col.isValid())
                {
                    setButtonColor(button, col);
                    emit colorChanged();
                }
            });
    }
    connect(
        mUi->moCapButton,
        &QPushButton::clicked,
        this,
        [this]()
        {
            const QColor col = pickColor(mUi->moCapButton);
            if(col.isValid())
            {
                emit moCapColorSelected(col);
            }
        });
}

ColorSettingsDialog::~ColorSettingsDialog()
{
    delete mUi;
}

QColor ColorSettingsDialog::pickColor(const QPushButton *button)
{
    return QColorDialog::getColor(getButtonColor(button), this);
}

QColor ColorSettingsDialog::getTrackPathColor() const
{
    return getButtonColor(mUi->trackPathButton);
}

void ColorSettingsDialog::setTrackPathColor(const QColor &col)
{
    setButtonColor(mUi->trackPathButton, col);
}

QColor ColorSettingsDialog::getTrackGroundPathColor() const
{
    return getButtonColor(mUi->trackGroundPathButton);
}

void ColorSettingsDialog::setTrackGroundPathColor(const QColor &col)
{
    setButtonColor(mUi->trackGroundPathButton, col);
}

QColor ColorSettingsDialog::getTrackPersonNumberColor() const
{
    return getButtonColor(mUi->trackPersonNumberButton);
}

void ColorSettingsDialog::setTrackPersonNumberColor(const QColor &col)
{
    setButtonColor(mUi->trackPersonNumberButton, col);
}

QColor ColorSettingsDialog::getTrackRecognizedColor() const
{
    return getButtonColor(mUi->trackRecognizedButton);
}

void ColorSettingsDialog::setTrackRecognizedColor(const QColor &col)
{
    setButtonColor(mUi->trackRecognizedButton, col);
}

QColor ColorSettingsDialog::getTrackTrackedColor() const
{
    return getButtonColor(mUi->trackTrackedButton);
}

void ColorSettingsDialog::setTrackTrackedColor(const QColor &col)
{
    setButtonColor(mUi->trackTrackedButton, col);
}

QColor ColorSettingsDialog::getMoCapColor() const
{
    return getButtonColor(mUi->moCapButton);
}

void ColorSettingsDialog::setMoCapColor(const QColor &col)
{
    setButtonColor(mUi->moCapButton, col);
}
