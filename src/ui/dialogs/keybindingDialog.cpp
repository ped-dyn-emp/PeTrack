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

#include "keybindingDialog.h"

#include "ui_keybindingDialog.h"

#include <QLabel>
#include <QScrollBar>
#include <cstring>

KeybindingDialog::KeybindingDialog(QWidget *parent, std::vector<KeyBindingGroup> groups) :
    QDialog(parent), mUi(new Ui::keybindingDialog)
{
    mUi->setupUi(this);

    Qt::WindowFlags flags = Qt::Window | Qt::WindowSystemMenuHint | Qt::WindowMinimizeButtonHint |
                            Qt::WindowMaximizeButtonHint | Qt::WindowCloseButtonHint;
    this->setWindowFlags(flags);

    for(auto &group : groups)
    {
        auto *title = new QLabel(QString("<b>%1</b>").arg(group.title));
        addRow(title);
        for(auto &entry : group.entries)
        {
            addLine();
            // alternatively make members private and do conversion in ctor (can then take const ref instead as well)
            entry.prepare();
            auto *shortcut    = new QLabel(entry.keybinding);
            auto *explanation = new QLabel(entry.explanation);
            addRow(explanation, shortcut);
        }
        addLine();
    }
    addRow(new QLabel(
        "<p>Beside the space bar and Ctrl+z all bindings only work with focus on the video/sequence.<br>"
        "You can find further key bindings next to the entries of the menus.</p>"));

    auto scrollBarWidth = mUi->scrollArea->verticalScrollBar()->width();
    auto frameWidth     = 2 * mUi->scrollArea->frameWidth();
    mUi->scrollArea->setMinimumWidth(mUi->scrollArea->sizeHint().width() + scrollBarWidth + frameWidth);
    resize(mUi->gridLayoutTop->sizeHint());
}

KeybindingDialog::~KeybindingDialog()
{
    delete mUi;
}

void KeybindingDialog::addRow(QWidget *widget)
{
    mUi->gridLayout->addWidget(widget, mUi->gridLayout->rowCount(), 0, 1, 2, Qt::AlignHCenter);
}

void KeybindingDialog::addLine()
{
    auto *line = new QFrame(this);
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Raised);
    line->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    mUi->gridLayout->addWidget(line, mUi->gridLayout->rowCount(), 0, 1, 2);
}

void KeybindingDialog::addRow(QWidget *left, QWidget *right)
{
    mUi->gridLayout->addWidget(left, mUi->gridLayout->rowCount(), 0, 1, 1, Qt::AlignLeft);
    mUi->gridLayout->addWidget(right, mUi->gridLayout->rowCount() - 1, 1, 1, 1, Qt::AlignLeft);
}

QString KeyBindingEntry::replaceSpecialCharacters(QString &in)
{
    const QString ctrlSign   = (std::strcmp(COMPILE_OS, "Darwin") != 0) ? "⌃ Ctrl" : "⌘ Cmd";
    const QString shiftSign  = "⇧ Shift";
    const QString altSign    = (std::strcmp(COMPILE_OS, "Darwin") != 0) ? "⎇ Alt" : "⌥ Option";
    const QString arrowUp    = "Arrow up ↑";
    const QString arrowDown  = "Arrow down ↓";
    const QString arrowLeft  = "Arrow left ←";
    const QString arrowRight = "Arrow right →";
    return in.replace(QString{"Ctrl"}, ctrlSign, Qt::CaseInsensitive)
        .replace(QString{"Shift"}, shiftSign, Qt::CaseInsensitive)
        .replace(QString{"Alt"}, altSign, Qt::CaseInsensitive)
        .replace(QString{"Arrow up"}, arrowUp, Qt::CaseInsensitive)
        .replace(QString{"Arrow left"}, arrowLeft, Qt::CaseInsensitive)
        .replace(QString{"Arrow down"}, arrowDown, Qt::CaseInsensitive)
        .replace(QString{"Arrow right"}, arrowRight, Qt::CaseInsensitive);
}

void KeyBindingEntry::prepare()
{
    replaceSpecialCharacters(keybinding);
    replaceSpecialCharacters(explanation);
}
