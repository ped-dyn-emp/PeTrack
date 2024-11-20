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


#ifndef PETRACK_ANNOTATIONGROUPWIDGET_H
#define PETRACK_ANNOTATIONGROUPWIDGET_H

#include "annotationGroupTreeModel.h"

#include <QWidget>

class AnnotationGroupManager;
class Animation;

namespace Ui
{
class AnnotationGroupUI;
}

class AnnotationGroupWidget : public QWidget
{
    Q_OBJECT
private:
    AnnotationGroupManager &mGroupManager;
    Ui::AnnotationGroupUI  *mUi;

    AnnotationGroupTreeModel mTreeModel;

    const Animation &mAnimation;

public:
    AnnotationGroupWidget(AnnotationGroupManager &groupManager, const Animation &animation, QWidget *parent = nullptr);
    ~AnnotationGroupWidget() override;

    AnnotationGroupWidget(const AnnotationGroupWidget &)            = delete;
    AnnotationGroupWidget(AnnotationGroupWidget &&)                 = delete;
    AnnotationGroupWidget &operator=(const AnnotationGroupWidget &) = delete;
    AnnotationGroupWidget &operator=(AnnotationGroupWidget &&)      = delete;

private:
    /**
     * Populates the tree view in the main panel.
     * This reads the group data from the manager and tanslates it into GroupItem instances which are used for the
     * custom TreeModel.
     */
    void populateTreeView();

    void addTrajectories();

    /**
     * Fetches available groups and sets them to the combo box
     */
    void updateComboBox();
    void btnEditGroup();
    void addGroup();
    void exportData();
    void importData();
    void comboBoxSelectionChanged();
};


#endif // PETRACK_ANNOTATIONGROUPWIDGET_H
