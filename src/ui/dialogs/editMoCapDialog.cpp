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

#include "editMoCapDialog.h"

#include "logger.h"
#include "moCapEditingWidget.h"
#include "moCapPerson.h"
#include "ui_editMoCapDialog.h"

#include <QScrollBar>

EditMoCapDialog::EditMoCapDialog(QWidget *parent, MoCapStorage &moCapStorage, std::function<void()> updateOverlay) :
    QDialog(parent), mUi(new Ui::EditMoCapDialog), mStorage(moCapStorage)
{
    mUi->setupUi(this);
    setWindowFlags(windowFlags().setFlag(Qt::WindowContextHelpButtonHint, false));

    auto &persons = mStorage.getPersons();
    for(auto &person : persons)
    {
        auto *widget = new MoCapEditingWidget(this, person, updateOverlay);
        mUi->editWidgetLayout->addWidget(widget);
    }

    connect(mUi->deselectAll, &QPushButton::clicked, this, &EditMoCapDialog::deselectAll);
    connect(mUi->selectAll, &QPushButton::clicked, this, &EditMoCapDialog::selectAll);

    mUi->editWidgets->setMinimumSize(mUi->editWidgets->minimumSizeHint());
    // Scroll Area needs to be set manually; should respect minimum size of widget, but does not
    // add some extra width for the frame and scrollbar
    mUi->scrollArea->setMinimumWidth(
        mUi->editWidgets->sizeHint().width() + 2 * mUi->scrollArea->frameWidth() +
        mUi->scrollArea->verticalScrollBar()->sizeHint().width());
    resize(mUi->mainLayout->sizeHint());
}

EditMoCapDialog::~EditMoCapDialog()
{
    delete mUi;
}

void EditMoCapDialog::deselectAll()
{
    for(int i = 0; i < mUi->editWidgetLayout->count(); ++i)
    {
        auto editingWidget = dynamic_cast<MoCapEditingWidget *>(mUi->editWidgetLayout->itemAt(i)->widget());
        if(editingWidget == nullptr)
        {
            continue;
        }

        editingWidget->deselect();
    }
}

void EditMoCapDialog::selectAll()
{
    for(int i = 0; i < mUi->editWidgetLayout->count(); ++i)
    {
        auto editingWidget = dynamic_cast<MoCapEditingWidget *>(mUi->editWidgetLayout->itemAt(i)->widget());
        if(editingWidget == nullptr)
        {
            continue;
        }

        editingWidget->select();
    }
}
