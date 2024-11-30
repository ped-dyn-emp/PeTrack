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

#include "correction.h"

#include "animation.h"
#include "control.h"
#include "importHelper.h"
#include "pMessageBox.h"
#include "personStorage.h"
#include "player.h"
#include "roiItem.h"
#include "ui_correction.h"

#include <QByteArray>
#include <QFile>
#include <QFrame>
#include <QVector>
#include <regex>
#include <string>

QVariant FailedChecksTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(role == Qt::DisplayRole && orientation == Qt::Horizontal)
    {
        switch(section)
        {
            case 0:
                return QString("Person");
            case 1:
                return QString("Frame");
            case 2:
                return QString("Status");
            case 3:
                return QString("Failed check");
        }
    }
    return QVariant();
}

int FailedChecksTableModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return static_cast<int>(mFailedChecks.size());
}

int FailedChecksTableModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return 4;
}

QVariant FailedChecksTableModel::data(const QModelIndex &index, int role) const
{
    if(!index.isValid())
    {
        return QVariant();
    }


    if(role == Qt::BackgroundRole)
    {
        QColor backgroundColor;
        switch(mFailedChecks[index.row()].status)
        {
            case plausibility::CheckStatus::New:
                backgroundColor = QColor(Qt::red);
                break;
            case plausibility::CheckStatus::Changed:
                backgroundColor = QColor(Qt::GlobalColor::yellow);
                break;
            case plausibility::CheckStatus::Resolved:
                backgroundColor = QColor(Qt::GlobalColor::green);
                break;
        }

        if(index.column() == 2)
        {
            return backgroundColor;
        }
    }

    if(role != Qt::DisplayRole)
    {
        return QVariant();
    }

    switch(index.column())
    {
        case 0:
            return QVariant::fromValue(mFailedChecks[index.row()].pers);
        case 1:
            return QVariant::fromValue(mFailedChecks[index.row()].frame);
        case 2:
            return QVariant();
        case 3:
            return QString::fromStdString(mFailedChecks[index.row()].message);
    }

    return QVariant();
}


void FailedChecksTableModel::setFailedChecks(std::vector<plausibility::FailedCheck> &&failedChecks)
{
    emit layoutAboutToBeChanged();

    mFailedChecks = failedChecks;
    std::sort(mFailedChecks.begin(), mFailedChecks.end());
    QModelIndex topLeft     = index(0, 0);
    QModelIndex bottomRight = index(rowCount() - 1, columnCount() - 1);

    emit dataChanged(topLeft, bottomRight);
    emit layoutChanged();
}

bool FailedChecksTableModel::removeRows(int row, int count, const QModelIndex &parent)
{
    if(count == 0)
    {
        return true;
    }
    beginRemoveRows(parent, row, row + count - 1);

    for(int i = 0; i < count; ++i)
    {
        mFailedChecks.erase(mFailedChecks.begin() + row);
    }

    endRemoveRows();
    return true;
}

const plausibility::FailedCheck &FailedChecksTableModel::getFailedCheck(const QModelIndex &index)
{
    return mFailedChecks.at(index.row());
}

std::vector<plausibility::FailedCheck> FailedChecksTableModel::getFailedChecks()
{
    return mFailedChecks;
}

void FailedChecksTableModel::updateState(const QModelIndex &row, plausibility::CheckStatus state)
{
    emit layoutAboutToBeChanged();

    mFailedChecks.at(row.row()).status = state;

    emit dataChanged(row, row);
    emit layoutChanged();
}

plausibility::CheckStatus FailedChecksTableModel::getState(const QModelIndex &row) const
{
    return mFailedChecks.at(row.row()).status;
}


Correction::Correction(Petrack *petrack, const PersonStorage &personStorage, QWidget *parent) :
    QWidget(parent), mPetrack(petrack), mPersonStorage(personStorage), mUi(new Ui::Correction)
{
    mUi->setupUi(this);
    mTableModel = new FailedChecksTableModel(this); // NOLINTLINE

    mUi->tblFailedChecks->setModel(mTableModel);

    mUi->tblFailedChecks->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeMode::ResizeToContents);
    mUi->tblFailedChecks->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeMode::ResizeToContents);
    mUi->tblFailedChecks->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeMode::ResizeToContents);
    mUi->tblFailedChecks->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeMode::Stretch);

    connect(mUi->btnCheck, &QPushButton::clicked, this, &Correction::checkButtonClicked);

    connect(
        mUi->tblFailedChecks->selectionModel(),
        &QItemSelectionModel::selectionChanged,
        this,
        &Correction::selectedRowChanged);

    connect(mUi->tblFailedChecks, &QTableWidget::customContextMenuRequested, this, &Correction::openContextMenu);

    connect(&personStorage, &PersonStorage::deletedPerson, this, &Correction::removePerson);
    connect(&personStorage, &PersonStorage::deletedPersonFrameRange, this, &Correction::removePersonInFrameRange);
    connect(&personStorage, &PersonStorage::changedPerson, this, &Correction::changePersonState);
    connect(&personStorage, &PersonStorage::splitPersonAtFrame, this, &Correction::splitPerson);
}

bool Correction::getTestEqualChecked() const
{
    return mUi->chbEqual->isChecked();
}
void Correction::setTestEqualChecked(bool testEqual)
{
    mUi->chbEqual->setChecked(testEqual);
}

bool Correction::getTestVelocityChecked() const
{
    return mUi->chbVelocity->isChecked();
}
void Correction::setTestVelocityChecked(bool testVelocity)
{
    mUi->chbVelocity->setChecked(testVelocity);
}

bool Correction::getTestLengthChecked() const
{
    return mUi->chbLength->isChecked();
}
void Correction::setTestLengthChecked(bool testLength)
{
    mUi->chbLength->setChecked(testLength);
}

bool Correction::getTestInsideChecked() const
{
    return mUi->chbInside->isChecked();
}
void Correction::setTestInsideChecked(bool testInside)
{
    mUi->chbInside->setChecked(testInside);
}

void Correction::selectedRowChanged()
{
    if(mUi->tblFailedChecks->selectionModel()->hasSelection())
    {
        auto selectedIndex = mUi->tblFailedChecks->selectionModel()->selectedIndexes();
        auto selectedCheck = mTableModel->getFailedCheck(selectedIndex[0]);

        size_t pers  = selectedCheck.pers;
        int    frame = selectedCheck.frame;

        if(selectedCheck.type == plausibility::CheckType::Equality)
        {
            if(std::smatch match; std::regex_search(selectedCheck.message, match, std::regex("\\d+")))
            {
                int secondPerson = std::stoi(match[0].str());
                mPetrack->getControlWidget()->trackShowOnlyNrList()->setText(
                    QString::fromStdString(fmt::format("{},{}", pers, secondPerson)));
            }
            mPetrack->getControlWidget()->setTrackShowOnly(Qt::Unchecked);
            mPetrack->getControlWidget()->setTrackShowOnlyListChecked(true);
        }
        else
        {
            mPetrack->getControlWidget()->setTrackShowOnlyNr(static_cast<int>(pers));
            mPetrack->getControlWidget()->setTrackShowOnlyListChecked(false);

            mPetrack->getControlWidget()->setTrackShowOnly(Qt::Checked);
        }
        mPetrack->getPlayer()->skipToFrame(frame);
    }
}

void Correction::openContextMenu()
{
    auto selectedIndex = mUi->tblFailedChecks->selectionModel()->selectedIndexes();

    if(selectedIndex.empty())
    {
        return;
    }

    QMenu      contextMenu(tr("&Correction"), this);
    const auto checkStatus = getStatus(selectedIndex);
    QString    actionText("Mark as resolved");
    auto       actionStatus = plausibility::CheckStatus::Resolved;

    if(checkStatus == plausibility::CheckStatus::Resolved)
    {
        actionText   = "Unresolve";
        actionStatus = plausibility::CheckStatus::Changed;
    }
    QAction markResolvedAction(actionText, this);
    connect(
        &markResolvedAction,
        &QAction::triggered,
        this,
        [this, &selectedIndex, &actionStatus]() { this->changeStatus(selectedIndex, actionStatus); });
    contextMenu.addAction(&markResolvedAction);
    contextMenu.exec(QCursor::pos());
}

plausibility::CheckStatus Correction::getStatus(QList<QModelIndex> pos) const
{
    return mTableModel->getState(pos[0]);
}
void Correction::changeStatus(QList<QModelIndex> pos, plausibility::CheckStatus status)
{
    mTableModel->updateState(pos[0], status);
}

void Correction::changePersonState(size_t index)
{
    if(!mChecksExecuted)
    {
        return;
    }

    auto failedChecks = mTableModel->getFailedChecks();

    for(auto &failedCheck : failedChecks)
    {
        bool person          = failedCheck.pers == (index + 1);
        bool personMentioned = failedCheck.message.find(std::to_string(index + 1)) != std::string::npos &&
                               failedCheck.type == plausibility::CheckType::Equality;

        if(person || personMentioned)
        {
            failedCheck.status = plausibility::CheckStatus::Changed;
        }
    }
    mTableModel->setFailedChecks(std::move(failedChecks));
}

void Correction::checkButtonClicked()
{
    std::vector<plausibility::FailedCheck> failedChecks;

    QProgressDialog progress("Check Plausibility", nullptr, 0, 400, mPetrack);
    progress.setWindowTitle("Check plausibility");
    progress.setWindowModality(Qt::WindowModal);
    progress.setVisible(true);
    progress.setValue(0);
    progress.setLabelText("Check Plausibility...");

    if(mUi->chbLength->isChecked())
    {
        auto failedLengthChecks{plausibility::checkLength(mPersonStorage, &progress, mUi->spbxMinFrameLength->value())};
        failedChecks.insert(failedChecks.end(), failedLengthChecks.begin(), failedLengthChecks.end());
    }

    if(mUi->chbInside->isChecked())
    {
        auto failedInsideChecks{plausibility::checkInside(
            mPersonStorage,
            &progress,
            mPetrack->getImageFiltered().size(),
            mPetrack->getImageBorderSize(),
            mPetrack->getRecoRoiItem()->rect(),
            mPetrack->getPlayer()->getFrameInNum(),
            mPetrack->getPlayer()->getFrameOutNum(),
            mUi->spbxInsideMargin->value())};
        failedChecks.insert(failedChecks.end(), failedInsideChecks.begin(), failedInsideChecks.end());
    }
    if(mUi->chbVelocity->isChecked())
    {
        auto failedVelocityChecks{plausibility::checkVelocityVariation(mPersonStorage, &progress)};
        failedChecks.insert(failedChecks.end(), failedVelocityChecks.begin(), failedVelocityChecks.end());
    }

    if(mUi->chbEqual->isChecked())
    {
        auto failedEqualityChecks{
            plausibility::checkEquality(mPersonStorage, &progress, *mPetrack, mUi->spbxEqualityDistance->value())};
        failedChecks.insert(failedChecks.end(), failedEqualityChecks.begin(), failedEqualityChecks.end());
    }

    // Restore the status if the same check for a person in a frame was already in the table
    const auto &previousChecks = mTableModel->getFailedChecks();
    for(auto &failedCheck : failedChecks)
    {
        auto previousCheck = std::find(previousChecks.cbegin(), previousChecks.cend(), failedCheck);
        if(previousCheck != previousChecks.cend())
        {
            failedCheck.status = previousCheck->status;
        }
    }

    mTableModel->setFailedChecks(std::move(failedChecks));
    mChecksExecuted = true;
}

void Correction::removePerson(size_t index)
{
    if(!mChecksExecuted)
    {
        return;
    }

    auto failedChecks = mTableModel->getFailedChecks();

    // Delete every row where person == index + 1
    failedChecks.erase(
        std::remove_if(
            failedChecks.begin(),
            failedChecks.end(),
            [index](const plausibility::FailedCheck &failedCheck) { return failedCheck.pers == (index + 1); }),
        failedChecks.end());

    // Update every row where person > index + 1 by decreasing person by one
    for(plausibility::FailedCheck &failedCheck : failedChecks)
    {
        if(failedCheck.pers > (index + 1))
        {
            failedCheck.pers--;
        }
    }

    // Delete all failed equality checks where the deleted person is mentioned
    failedChecks.erase(
        std::remove_if(
            failedChecks.begin(),
            failedChecks.end(),
            [index](const plausibility::FailedCheck &failedCheck)
            {
                bool personMentioned = failedCheck.message.find(std::to_string(index + 1)) != std::string::npos;
                bool checks          = failedCheck.type == plausibility::CheckType::Equality;
                return personMentioned && checks;
            }),
        failedChecks.end());

    mTableModel->setFailedChecks(std::move(failedChecks));
}

void Correction::removePersonInFrameRange(size_t index, int startFrame, int endFrame)
{
    if(!mChecksExecuted)
    {
        return;
    }

    auto failedChecks = rerunChecks();

    // Handle equality checks differently as they take too long to be re-run after each removal of a frame range
    // Delete equality checks of person in frame range
    const auto &previousChecks = mTableModel->getFailedChecks();

    std::copy_if(
        previousChecks.begin(),
        previousChecks.end(),
        std::back_inserter(failedChecks),
        [startFrame, endFrame, index](const plausibility::FailedCheck &failedCheck)
        {
            bool person          = failedCheck.pers == index + 1;
            bool personMentioned = failedCheck.message.find(std::to_string(index + 1)) != std::string::npos;

            bool equalityCheck   = (failedCheck.type == plausibility::CheckType::Equality);
            bool notInFrameRange = !((startFrame <= failedCheck.frame) && (failedCheck.frame <= endFrame));
            return equalityCheck && (notInFrameRange || (!person && !personMentioned));
        });

    // Restore the status if the same check for a person in a frame was already in the table
    for(auto &failedCheck : failedChecks)
    {
        auto previousCheck = std::find(previousChecks.cbegin(), previousChecks.cend(), failedCheck);
        if(previousCheck != previousChecks.cend())
        {
            failedCheck.status = previousCheck->status;
        }
    }

    mTableModel->setFailedChecks(std::move(failedChecks));
}

void Correction::splitPerson(size_t index, size_t newIndex, int frame)
{
    if(!mChecksExecuted)
    {
        return;
    }
    auto failedChecks = rerunChecks();

    // Handle equality checks differently as they take too long to be re-run after each removal of a frame range
    // Delete equality checks of person in frame range
    auto previousChecks = mTableModel->getFailedChecks();

    for(auto &previousCheck : previousChecks)
    {
        if(previousCheck.frame > frame)
        {
            if(previousCheck.pers == (index + 1))
            {
                previousCheck.pers = (newIndex + 1);
            }

            if(previousCheck.message.find(std::to_string(index + 1)) != std::string::npos)
            {
                previousCheck.message = fmt::format("Trajectory is very close to Person {}!", newIndex + 1);
            }
        }
    }

    std::copy_if(
        previousChecks.begin(),
        previousChecks.end(),
        std::back_inserter(failedChecks),
        [index, newIndex](const plausibility::FailedCheck &failedCheck)
        {
            bool person          = failedCheck.pers == (index + 1);
            bool personMentioned = failedCheck.message.find(std::to_string(index + 1)) != std::string::npos;

            bool newPerson          = failedCheck.pers == (newIndex + 1);
            bool newPersonMentioned = failedCheck.message.find(std::to_string(newIndex + 1)) != std::string::npos;

            bool equalityCheck = (failedCheck.type == plausibility::CheckType::Equality);
            return equalityCheck && (person || personMentioned || newPerson || newPersonMentioned);
        });

    // Restore the status if the same check for a person in a frame was already in the table
    for(auto &failedCheck : failedChecks)
    {
        auto previousCheck = std::find(previousChecks.cbegin(), previousChecks.cend(), failedCheck);
        if(previousCheck != previousChecks.cend())
        {
            failedCheck.status = previousCheck->status;
        }
    }

    mTableModel->setFailedChecks(std::move(failedChecks));
}

void Correction::clear()
{
    auto success = mUi->tblFailedChecks->model()->removeRows(0, mUi->tblFailedChecks->model()->rowCount());
    if(!success)
    {
        PWarning(this, "Correction", "Could not clear table.");
    }
    mChecksExecuted = false;
}

void Correction::setXml(QDomElement &elem) const
{
    auto equalElement = (elem.ownerDocument()).createElement("EQUALITY");
    equalElement.setAttribute("ENABLED", mUi->chbEqual->isChecked());
    equalElement.setAttribute("DISTANCE", mUi->spbxEqualityDistance->value());
    elem.appendChild(equalElement);

    auto velocityElement = (elem.ownerDocument()).createElement("VELOCITY");
    velocityElement.setAttribute("ENABLED", mUi->chbVelocity->isChecked());
    elem.appendChild(velocityElement);

    auto lengthElement = (elem.ownerDocument()).createElement("LENGTH");
    lengthElement.setAttribute("ENABLED", mUi->chbLength->isChecked());
    lengthElement.setAttribute("MIN_LENGTH", mUi->spbxMinFrameLength->value());
    elem.appendChild(lengthElement);

    auto insideElement = (elem.ownerDocument()).createElement("INSIDE");
    insideElement.setAttribute("ENABLED", mUi->chbInside->isChecked());
    insideElement.setAttribute("MARGIN", mUi->spbxInsideMargin->value());
    elem.appendChild(insideElement);
}

bool Correction::getXml(const QDomElement &correctionElem)
{
    for(auto subElem = correctionElem.firstChildElement(); !subElem.isNull(); subElem = subElem.nextSiblingElement())
    {
        if(subElem.tagName() == "EQUALITY")
        {
            loadBoolValue(subElem, "ENABLED", mUi->chbEqual);
            loadDoubleValue(subElem, "DISTANCE", mUi->spbxEqualityDistance);
        }
        else if(subElem.tagName() == "VELOCITY")
        {
            loadBoolValue(subElem, "ENABLED", mUi->chbVelocity);
        }
        else if(subElem.tagName() == "LENGTH")
        {
            loadBoolValue(subElem, "ENABLED", mUi->chbLength);
            loadDoubleValue(subElem, "MIN_LENGTH", mUi->spbxMinFrameLength);
        }
        else if(subElem.tagName() == "INSIDE")
        {
            loadBoolValue(subElem, "ENABLED", mUi->chbInside);
            loadIntValue(subElem, "MARGIN", mUi->spbxInsideMargin);
        }
        else
        {
            SPDLOG_WARN("Unknown CORRECTION tag: {}", subElem.tagName().toStdString());
        }
    }
    return true;
}

std::vector<plausibility::FailedCheck> Correction::rerunChecks()
{
    // Rerun tests (except equality)
    QProgressDialog progress("Check Plausibility", nullptr, 0, 400, mPetrack);
    progress.setWindowTitle("Check plausibility");
    progress.setWindowModality(Qt::WindowModal);
    progress.setVisible(false);
    progress.setValue(0);
    progress.setLabelText("Check Plausibility...");

    auto failedChecks = std::vector<plausibility::FailedCheck>();

    if(mUi->chbLength->isChecked())
    {
        auto failedLengthChecks{plausibility::checkLength(mPersonStorage, &progress, mUi->spbxMinFrameLength->value())};
        failedChecks.insert(failedChecks.end(), failedLengthChecks.begin(), failedLengthChecks.end());
    }

    if(mUi->chbInside->isChecked())
    {
        auto failedInsideChecks{plausibility::checkInside(
            mPersonStorage,
            &progress,
            mPetrack->getImageFiltered().size(),
            mPetrack->getImageBorderSize(),
            mPetrack->getRecoRoiItem()->rect(),
            mPetrack->getPlayer()->getFrameInNum(),
            mPetrack->getPlayer()->getFrameOutNum(),
            mUi->spbxInsideMargin->value())};
        failedChecks.insert(failedChecks.end(), failedInsideChecks.begin(), failedInsideChecks.end());
    }

    if(mUi->chbVelocity->isChecked())
    {
        auto failedVelocityChecks{plausibility::checkVelocityVariation(mPersonStorage, &progress)};
        failedChecks.insert(failedChecks.end(), failedVelocityChecks.begin(), failedVelocityChecks.end());
    }
    return failedChecks;
}

Correction::~Correction()
{
    delete mUi;
}
