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

#ifndef MOCAPSELECTIONWIDGET_H
#define MOCAPSELECTIONWIDGET_H

#include "moCapPersonMetadata.h"

#include <QWidget>

namespace Ui
{
class MoCapSelectionWidget;
}

class OpenMoCapDialog;

class MoCapSelectionWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MoCapSelectionWidget(QWidget *parent, const QMap<QString, MoCapSystem> &moCapSystems);
    explicit MoCapSelectionWidget(
        QWidget                          *parent,
        const QMap<QString, MoCapSystem> &moCapSystems,
        const MoCapPersonMetadata        &metadata);
    MoCapSelectionWidget(const MoCapSelectionWidget &)            = delete;
    MoCapSelectionWidget(MoCapSelectionWidget &&)                 = delete;
    MoCapSelectionWidget &operator=(const MoCapSelectionWidget &) = delete;
    MoCapSelectionWidget &operator=(MoCapSelectionWidget &&)      = delete;
    ~MoCapSelectionWidget() override;

    void                setFileName();
    void                setFileName(QString filename);
    MoCapPersonMetadata getMetadata() const;

    bool isFilledOut() const;

private:
    Ui::MoCapSelectionWidget         *mUi;
    const QMap<QString, MoCapSystem> &mMoCapSystems;
    bool                              mFilledOut = false;
};

#endif // MOCAPSELECTIONWIDGET_H
