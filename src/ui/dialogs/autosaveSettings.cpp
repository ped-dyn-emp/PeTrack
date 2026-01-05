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

#include "autosaveSettings.h"

#include "ui_autosaveSettings.h"

AutosaveSettings::AutosaveSettings(double interval, int numChanges, QWidget *parent) :
    QDialog(parent), mUi(new Ui::AutosaveSettings)
{
    mUi->setupUi(this);
    setPetSaveInterval(interval);
    setChangesTillAutosave(numChanges);
}

void AutosaveSettings::setPetSaveInterval(double interval)
{
    mUi->spbxPetInterval->setValue(interval);
}

double AutosaveSettings::getPetSaveInterval() const
{
    return mUi->spbxPetInterval->value();
}

void AutosaveSettings::setChangesTillAutosave(int numChanges)
{
    mUi->spbxTrajChanges->setValue(numChanges);
}

int AutosaveSettings::getChangesTillAutosave() const
{
    return mUi->spbxTrajChanges->value();
}

AutosaveSettings::~AutosaveSettings()
{
    delete mUi;
}
