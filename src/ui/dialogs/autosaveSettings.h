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

#ifndef AUTOSAVESETTINGS_H
#define AUTOSAVESETTINGS_H

#include <QDialog>

namespace Ui
{
class AutosaveSettings;
}

class AutosaveSettings : public QDialog
{
    Q_OBJECT

public:
    explicit AutosaveSettings(double interval, int numChanges, QWidget *parent = nullptr);
    AutosaveSettings(const AutosaveSettings &)             = delete;
    AutosaveSettings(AutosaveSettings &&)                  = delete;
    AutosaveSettings  &operator=(const AutosaveSettings &) = delete;
    AutosaveSettings &&operator=(AutosaveSettings &&)      = delete;
    ~AutosaveSettings() override;

    double getPetSaveInterval() const;
    int    getChangesTillAutosave() const;

private:
    void setChangesTillAutosave(int numChanges);
    void setPetSaveInterval(double interval);

    Ui::AutosaveSettings *mUi;
};

#endif // AUTOSAVESETTINGS_H
