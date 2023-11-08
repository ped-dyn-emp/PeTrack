/*
 * PeTrack - Software for tracking pedestrians movement in videos
 * Copyright (C) 2023 Forschungszentrum Jülich GmbH, IAS-7
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

#ifndef CORRECTION_H
#define CORRECTION_H

#include "plausibility.h"

#include <QAbstractTableModel>
#include <QDomElement>
#include <QWidget>

namespace Ui
{
class Correction;
}

class FailedChecksTableModel : public QAbstractTableModel
{
    Q_OBJECT
private:
    std::vector<plausibility::FailedCheck> mFailedChecks;

public:
    explicit FailedChecksTableModel(QObject *parent) : QAbstractTableModel(parent) {}

    void setFailedChecks(std::vector<plausibility::FailedCheck> &&failedChecks);

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    int      rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int      columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
    std::vector<plausibility::FailedCheck> getFailedChecks();
    const plausibility::FailedCheck       &getFailedCheck(const QModelIndex &row);

    void updateState(const QModelIndex &row, plausibility::CheckStatus state);
};


class Correction : public QWidget
{
    Q_OBJECT

public:
    explicit Correction(Petrack *petrack, const PersonStorage &personStorage, QWidget *parent = nullptr);

    Correction(Correction const &other)            = delete;
    Correction &operator=(Correction const &other) = delete;
    Correction(Correction &&other)                 = delete;
    Correction &operator=(Correction &&other)      = delete;
    ~Correction() override;

    bool getTestEqualChecked() const;
    void setTestEqualCheckState(Qt::CheckState testEqual);

    bool getTestVelocityChecked() const;
    void setTestVelocityCheckState(Qt::CheckState testVelocity);

    bool getTestLengthChecked() const;
    void setTestLengthCheckState(Qt::CheckState testLength);

    bool getTestInsideChecked() const;
    void setTestInsideCheckState(Qt::CheckState testInside);

    void setXml(QDomElement &elem) const;
    bool getXml(const QDomElement &elem);

    void clear();

private:
    Petrack             *mPetrack;
    const PersonStorage &mPersonStorage;

    Ui::Correction         *mUi;
    FailedChecksTableModel *mTableModel;
    bool                    mChecksExecuted = false;

    std::vector<plausibility::FailedCheck> rerunChecks();

private slots:
    void selectedRowChanged();
    void checkButtonClicked();
    void openContextMenu();
    void markResolved(QList<QModelIndex> pos);
    void changePersonState(size_t index);
    void removePerson(size_t index);
    void removePersonInFrameRange(size_t index, int startFrame, int endFrame);
    void splitPerson(size_t, size_t newIndex, int frame);
};

#endif // CORRECTION_H
