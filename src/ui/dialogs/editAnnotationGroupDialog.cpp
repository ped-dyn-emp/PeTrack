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

#include "editAnnotationGroupDialog.h"

#include "logger.h"
#include "ui_editAnnotationGroupDialog.h"

#include <QColorDialog>
#include <QRadioButton>

using namespace annotationGroups;
using namespace std;

EditAnnotationGroupDialog::EditAnnotationGroupDialog(
    const std::vector<annotationGroups::TopLevelGroup> &tlgs,
    const std::set<std::string>                        &types,
    const annotationGroups::Group                      &group,
    QWidget                                            *parent) :
    QDialog(parent), mUi(new Ui::EditAnnotationGroupDialog()), mGroup(group), mTopLevelGroups(tlgs)
{
    mUi->setupUi(this);
    this->setWindowTitle("Edit group");

    mUi->lblName->setText(QString::fromStdString(group.name));

    // color button
    setSelectedColor(group.color);
    connect(mUi->btnColor, &QPushButton::clicked, [&, this]() { this->btnColorClicked(); });


    mUi->txtName->setText(QString::fromStdString(group.name));
    connect(
        mUi->txtName,
        &QLineEdit::textEdited,
        [&, this](const QString &change) { this->mGroup.name = change.toStdString(); });


    // combobox
    auto &typebox = mUi->cmbxType;
    for(const auto &type : types)
    {
        typebox->addItem(QString::fromStdString(type));
    }
    typebox->setEditable(true);
    typebox->setCurrentText(QString::fromStdString(group.type));
    connect(
        typebox,
        &QComboBox::currentTextChanged,
        [&, this](const QString &change) { this->mGroup.type = change.toStdString(); });

    // top level groups
    int i = 0;
    for(const auto &tlg : tlgs)
    {
        auto  btnName = tlg.name;
        auto *radio   = new QRadioButton(QString::fromStdString(btnName));
        mUi->groupBoxTLG->layout()->addWidget(radio);
        mTlgButtonIdMap[i] = tlg.id;
        mRadioButtons.push_back(radio);
        connect(radio, &QRadioButton::clicked, this, &EditAnnotationGroupDialog::btnTLGRadioClicked);
        if(tlg.id == group.tlgId)
        {
            radio->click();
        }
        i++;
    }

    setTabOrder(mUi->cmbxType, mRadioButtons.front());

    // trajectories
    auto *trajectoryTableView = mUi->tblTrajectories;
    trajectoryTableView->setModel(&tableModel);
    trajectoryTableView->verticalHeader()->hide();
    trajectoryTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    trajectoryTableView->setSelectionBehavior(QAbstractItemView::SelectRows);

    // btn delete group
    connect(
        mUi->btnDeleteGroup,
        &QPushButton::clicked,
        [&, this]()
        {
            mDeletePressed = true;
            this->accept();
        });
}

EditAnnotationGroupDialog::~EditAnnotationGroupDialog()
{
    for(auto &btn : mRadioButtons)
    {
        delete btn;
    }
    for(auto &item : items)
    {
        delete item;
    }
}

annotationGroups::Group EditAnnotationGroupDialog::getGroup() const
{
    return mGroup;
}

bool EditAnnotationGroupDialog::isValid() const
{
    return !mGroup.name.empty();
}

size_t EditAnnotationGroupDialog::getSelectedTopLevelGroupIndex() const
{
    for(size_t i = 0; i < mRadioButtons.size(); ++i)
    {
        if(mRadioButtons.at(i)->isChecked())
        {
            return i;
        }
    }

    return -1;
}

int EditAnnotationGroupDialog::getSelectedTopLevelGroup() const
{
    return mTlgButtonIdMap.at(mSelectedButton);
}

void EditAnnotationGroupDialog::btnTLGRadioClicked()
{
    if(getSelectedTopLevelGroupIndex() != mSelectedButton)
    {
        mSelectedButton = getSelectedTopLevelGroupIndex();
        mGroup.tlgId    = mTlgButtonIdMap.at(mSelectedButton);
    }
}

void EditAnnotationGroupDialog::setSelectedColor(const QColor &color)
{
    if(color == Qt::transparent)
    {
        return;
    }
    QPalette pal = mUi->btnColor->palette();
    pal.setColor(QPalette::ColorRole::Button, color);
    mUi->btnColor->setPalette(pal);
    mGroup.color = color;
}


void EditAnnotationGroupDialog::btnColorClicked()
{
    QColor const col = QColorDialog::getColor(mGroup.color, this);
    if(col.isValid())
    {
        setSelectedColor(col);
    }
}
void EditAnnotationGroupDialog::setTrajectories(
    vector<annotationGroups::TrajectoryGroupEntry> trajectories,
    int                                            currentFrame)
{
    int countCurrent = 0;

    tableModel.clear();

    std::sort(
        trajectories.begin(),
        trajectories.end(),
        [](const annotationGroups::TrajectoryGroupEntry &a, const annotationGroups::TrajectoryGroupEntry &b)
        { return a.trackPersonId < b.trackPersonId; });

    int     index = 0;
    QString framerange;

    for(size_t i = 0; i < trajectories.size(); i++)
    {
        const auto &entry = trajectories.at(i);


        framerange.append(QString("%1 - %2")
                              .arg(entry.frameBegin)
                              .arg((entry.frameEnd >= 0) ? QString::number(entry.frameEnd) : QString("end")));

        if(i < trajectories.size() - 1 && trajectories.at(i + 1).trackPersonId == entry.trackPersonId)
        {
            framerange.append(", ");
        }
        else
        {
            // person id + 1, because the view starts counting at 1, but the vector at 0
            auto *traj  = new QStandardItem(QString("%1").arg(entry.trackPersonId + 1));
            auto *frame = new QStandardItem(framerange);

            tableModel.setItem(index, 0, traj);
            tableModel.setItem(index, 1, frame);
            items.push_back(traj);
            items.push_back(frame);
            framerange = QString();

            if(currentFrame >= entry.frameBegin && (currentFrame < entry.frameEnd || entry.frameEnd == -1))
            {
                countCurrent++;
                tableModel.setData(tableModel.index(index, 0), QColor(0xe4f2f7), Qt::BackgroundRole);
                tableModel.setData(tableModel.index(index, 1), QColor(0xe4f2f7), Qt::BackgroundRole);
            }
            index++;
        }
    }

    tableModel.setHorizontalHeaderLabels({QString("Trajectory"), QString("Occurring Frames")});

    mUi->lbl_total->setText(QString::number(trajectories.size()));
    mUi->lbl_current->setText(QString::number(countCurrent));
}
