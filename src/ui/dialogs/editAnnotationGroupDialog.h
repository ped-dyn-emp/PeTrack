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

#ifndef PETRACK_EDITANNOTATIONGROUPDIALOG_H
#define PETRACK_EDITANNOTATIONGROUPDIALOG_H

#include "annotationGrouping.h"

#include <QDialog>
#include <QRadioButton>
#include <QStandardItemModel>
#include <set>
#include <vector>

namespace Ui
{
class EditAnnotationGroupDialog;
}

class EditAnnotationGroupDialog : public QDialog
{
    Q_OBJECT

public:
    EditAnnotationGroupDialog(
        const std::vector<annotationGroups::TopLevelGroup> &tlgs,
        const std::set<std::string>                        &types,
        const annotationGroups::Group                      &group,
        QWidget                                            *parent = nullptr);
    ~EditAnnotationGroupDialog() override;

    void   btnTLGRadioClicked();
    size_t getSelectedTopLevelGroupIndex() const;
    int    getSelectedTopLevelGroup() const;

    void btnColorClicked();

    annotationGroups::Group getGroup() const;
    bool                    isValid() const;

    void setTrajectories(std::vector<annotationGroups::TrajectoryGroupEntry> trajectories, int currentFrame = -1);


    bool deleteCalled() const { return mDeletePressed; }

private:
    void setSelectedColor(const QColor &color);

    Ui::EditAnnotationGroupDialog *mUi;

    std::vector<QRadioButton *> mRadioButtons;
    std::map<size_t, int>       mTlgButtonIdMap;
    size_t                      mSelectedButton = -1;

    annotationGroups::Group                      mGroup;
    std::vector<annotationGroups::TopLevelGroup> mTopLevelGroups;

    QStandardItemModel           tableModel;
    std::vector<QStandardItem *> items;

    bool mDeletePressed = false;
};


#endif // PETRACK_EDITANNOTATIONGROUPDIALOG_H
