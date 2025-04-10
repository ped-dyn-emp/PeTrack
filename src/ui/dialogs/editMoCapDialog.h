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

#ifndef EDITMOCAPDIALOG_H
#define EDITMOCAPDIALOG_H

#include <QDialog>

namespace Ui
{
class EditMoCapDialog;
}
class MoCapStorage;

class EditMoCapDialog : public QDialog
{
    Q_OBJECT

public:
    explicit EditMoCapDialog(QWidget *parent, MoCapStorage &moCapStorage, std::function<void()> updateOverlay);
    EditMoCapDialog(const EditMoCapDialog &)            = delete;
    EditMoCapDialog(EditMoCapDialog &&)                 = delete;
    EditMoCapDialog operator==(const EditMoCapDialog &) = delete;
    EditMoCapDialog operator==(EditMoCapDialog &&)      = delete;
    ~EditMoCapDialog() override;

private slots:
    void deselectAll();
    void selectAll();

private:
    Ui::EditMoCapDialog *mUi;
    MoCapStorage        &mStorage;
};

#endif // EDITMOCAPDIALOG_H
