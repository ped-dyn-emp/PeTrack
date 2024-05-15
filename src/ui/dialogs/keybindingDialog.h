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

#ifndef KEYBINDINGDIALOG_H
#define KEYBINDINGDIALOG_H

#include <QDialog>

namespace Ui
{
class keybindingDialog;
}

struct KeyBindingEntry
{
    QString keybinding;
    QString explanation;

    static QString replaceSpecialCharacters(QString &in);
    void           prepare();
};

struct KeyBindingGroup
{
    QString                      title;
    std::vector<KeyBindingEntry> entries;
};

class KeybindingDialog : public QDialog
{
    Q_OBJECT

public:
    explicit KeybindingDialog(QWidget *parent, std::vector<KeyBindingGroup> groups);
    ~KeybindingDialog();

private:
    void addRow(QWidget *widget);
    void addRow(QWidget *left, QWidget *right);
    void addLine();

    Ui::keybindingDialog *mUi;
};

#endif // KEYBINDINGDIALOG_H
