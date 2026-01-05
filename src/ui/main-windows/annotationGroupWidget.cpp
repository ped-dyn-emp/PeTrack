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


#include "annotationGroupWidget.h"

#include "animation.h"
#include "annotationGroupIO.h"
#include "annotationGroupManager.h"
#include "annotationGroupTreeItem.h"
#include "annotationGrouping.h"
#include "createAnnotationGroupDialog.h"
#include "editAnnotationGroupDialog.h"
#include "logger.h"
#include "pMessageBox.h"
#include "petrack.h"
#include "pspinbox.h"
#include "ui_annotationGroups.h"

#include <QComboBox>
#include <QPushButton>
#include <memory>
#include <utility>

AnnotationGroupWidget::AnnotationGroupWidget(
    AnnotationGroupManager &groupManager,
    const Animation        &animation,
    QWidget                *parent) :
    QWidget(parent), mGroupManager(groupManager), mUi(new Ui::AnnotationGroupUI), mAnimation(animation)
{
    mUi->setupUi(this);

    connect(mUi->ckbxShow, &QCheckBox::clicked, &mGroupManager, &AnnotationGroupManager::setVisualization);
    connect(
        mUi->spinRadius,
        QOverload<int>::of(&PSpinBox::valueChanged),
        [&](int value) { mGroupManager.setVisualizationRadius((value >= 0) ? value : 0); });

    connect(mUi->btnAddGroup, &QPushButton::pressed, this, &AnnotationGroupWidget::addGroup);
    connect(mUi->btnEditGroup, &QPushButton::pressed, this, &AnnotationGroupWidget::btnEditGroup);


    connect(mUi->btnExport, &QPushButton::pressed, this, &AnnotationGroupWidget::exportData);
    connect(mUi->btnImport, &QPushButton::pressed, this, &AnnotationGroupWidget::importData);

    connect(
        &mGroupManager,
        &AnnotationGroupManager::trajectoryAssignmentChanged,
        this,
        &AnnotationGroupWidget::populateTreeView);
    connect(mUi->btnAddTrajectories, &QPushButton::pressed, this, &AnnotationGroupWidget::addTrajectories);

    // group selection combo box
    connect(&mGroupManager, &AnnotationGroupManager::groupsChanged, this, &AnnotationGroupWidget::updateComboBox);
    connect(
        mUi->comboGroup,
        QOverload<int>::of(&QComboBox::currentIndexChanged),
        this,
        &AnnotationGroupWidget::comboBoxSelectionChanged);

    // hide polygon button
    // TODO unhide, when implementing polygon selection feature
    mUi->btnPolygonSelection->setVisible(false);
    mUi->btnReset->setVisible(false);


    mUi->treeView->setModel(&mTreeModel);
    populateTreeView();
}

AnnotationGroupWidget::~AnnotationGroupWidget()
{
    delete mUi;
}


void AnnotationGroupWidget::addGroup()
{
    CreateAnnotationGroupDialog dialog{mGroupManager.getTopLevelGroups(), mGroupManager.getKnownTypes(), this};

    std::vector<QColor> colors;
    for(auto &tlg : mGroupManager.getTopLevelGroups())
    {
        QColor const color = mGroupManager.getNextTLGColor(tlg.id);
        colors.push_back((color == Qt::transparent) ? Qt::white : color);
    }

    dialog.setPredefinedColors(colors);
    dialog.exec();
    if(dialog.result() == QDialog::Accepted && dialog.isValid())
    {
        auto group   = dialog.getGroup();
        auto groupId = mGroupManager.createGroup(group);
        if(groupId >= 0)
        {
            populateTreeView();
        }
        else
        {
            PWarning(parentWidget(), "Could not create group", "Group create was not successfull due to invalid data");
        }
    }
}

void AnnotationGroupWidget::exportData()
{
    auto filename = annotationGroups::getFilename();
    if(filename.isEmpty())
    {
        return;
    }
    annotationGroups::writeConfigurationToFile(mGroupManager.saveConfig(), filename);
}

void AnnotationGroupWidget::importData()
{
    auto file = annotationGroups::getFilename("", false);
    if(file.isEmpty())
    {
        return;
    }
    auto config = annotationGroups::readConfigurationFromFile(file);
    mGroupManager.loadConfig(config);
    populateTreeView();
}
void AnnotationGroupWidget::comboBoxSelectionChanged()
{
    auto groupId = mUi->comboGroup->currentData().toInt();
    if(!mGroupManager.isValidGroupId(groupId))
    {
        SPDLOG_ERROR("Selected group in ComboBox is not valid. Ignoring selection");
        return;
    }

    if(groupId == -1)
    {
        mUi->lblColorPreview->setStyleSheet("");
        return;
    }

    const auto &group = mGroupManager.getGroup(groupId);
    mUi->lblColorPreview->setStyleSheet(QString("QLabel {background-color: %1}").arg(group.color.name()));
}

void AnnotationGroupWidget::btnEditGroup()
{
    auto selectedIndex = mUi->treeView->selectionModel()->selectedIndexes();
    if(selectedIndex.isEmpty())
    {
        return;
    }

    auto index = selectedIndex.front();
    if(!mTreeModel.isIndexValid(index))
    {
        return;
    }

    auto *item = static_cast<AnnotationGroupTreeItem *>(index.internalPointer());
    if(item->isTLG())
    {
        return;
    }
    int  groupId = item->getId();
    auto group   = mGroupManager.getGroup(groupId);


    EditAnnotationGroupDialog dialog{mGroupManager.getTopLevelGroups(), mGroupManager.getKnownTypes(), group, this};
    const auto               &traj = mGroupManager.getTrajectoriesOfGroup(groupId);
    dialog.setTrajectories(traj, mAnimation.getCurrentFrameNum());
    dialog.exec();
    const int result = dialog.result();
    if(result == QDialog::Accepted)
    {
        if(dialog.deleteCalled())
        {
            mGroupManager.deleteGroup(groupId);
        }
        else
        {
            if(dialog.isValid())
            {
                auto changed = dialog.getGroup();
                changed.id   = group.id;
                mGroupManager.updateGroup(changed);
            }
        }
        populateTreeView();
    }
}

void AnnotationGroupWidget::addTrajectories()
{
    // check group validity
    int grpId = mUi->comboGroup->currentData().toInt();

    if(!mGroupManager.isValidGroupId(grpId) && grpId != -1)
    {
        SPDLOG_WARN("Invalid group selected. Check input");
        PWarning(this->parentWidget(), tr("Warning"), tr("Invalid group selected."));
        return;
    }


    std::set<int> trajectories;
    try
    {
        trajectories = splitCompactString(mUi->txtTrajectoriesToAdd->text().toStdString());
    }
    catch(...)
    {
        SPDLOG_WARN("Invalid input string");
        PWarning(this->parentWidget(), tr("Warning"), tr("Could not parse trajectory selection."));
    }

    auto group = mGroupManager.getGroup(grpId);

    std::vector<int> fails;

    for(const int id : trajectories)
    {
        if(grpId == -1)
        {
            SPDLOG_DEBUG("adding to NO GROUP");
            if(!mGroupManager.removeTrajectoryAssignment(id - 1))
            {
                fails.push_back(id);
            }
        }
        else if(!mGroupManager.addTrajectoryToGroup(id - 1, group.id))
        {
            fails.push_back(id);
        }
    }

    for(int fail : fails)
    {
        SPDLOG_WARN("Failed adding trajectory:  {}. Maybe it does not exist?", fail);
        PWarning(
            this->parentWidget(),
            tr("Warning"),
            QString("Could not add trajectory %1, maybe it does not exist?").arg(fail));
    }
}

void AnnotationGroupWidget::updateComboBox()
{
    auto combo = mUi->comboGroup;

    combo->clear();

    auto groups = mGroupManager.getGroups();
    if(groups.empty())
    {
        return;
    }

    std::sort(groups.begin(), groups.end(), [&](const auto a, const auto b) { return a.tlgId < b.tlgId; });

    combo->insertItem(0, QString("No Group (removes assignment from trajectory)"), QVariant::fromValue(-1));
    combo->insertSeparator(1);

    annotationGroups::Group last  = groups.front();
    int                     index = 2;
    for(const auto &group : groups)
    {
        if(last.tlgId != group.tlgId)
        {
            combo->insertSeparator(index++);
        }
        combo->insertItem(
            index,
            QString::fromStdString(group.name).append(QString(" (%1)").arg(QString::fromStdString(group.type))),
            QVariant::fromValue(group.id));
        last = group;
        index++;
    }

    combo->setCurrentIndex(0);
}

void AnnotationGroupWidget::populateTreeView()
{
    mUi->treeView->clearSelection();
    auto root = std::make_unique<AnnotationGroupTreeItem>();
    root->setData(0, "key");
    root->setData(1, "value");
    for(const auto &tlg : mGroupManager.getTopLevelGroups())
    {
        auto tlgElem = std::make_unique<TopLevelGroupTreeItem>(tlg.id, tlg.name, root.get());

        const std::vector<annotationGroups::Group> groups = mGroupManager.getGroupsOfTlg(tlg.id);

        tlgElem->setChildCount((int) groups.size());
        for(const auto &grp : groups)
        {
            auto grpElem = std::make_unique<GroupTreeItem>(grp.id, grp.name, grp.type, tlgElem.get());
            grpElem->setColor(grp.color);
            grpElem->setChildCount((int) mGroupManager.getTrajectoriesOfGroup(grp.id).size());
            tlgElem->appendChild(std::move(grpElem));
        }
        root->appendChild(std::move(tlgElem));
    }

    mTreeModel.setRootItem(std::move(root));

    mUi->treeView->update();
    mTreeModel.layoutChanged();
    mUi->treeView->expandAll();
    mUi->treeView->resizeColumnToContents(0);
}