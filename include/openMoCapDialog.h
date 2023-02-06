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
#ifndef OPENMOCAPDIALOG_H
#define OPENMOCAPDIALOG_H

#include "IO.h"
#include "moCapController.h"
#include "moCapPersonMetadata.h"

#include <QDialog>
#include <QMap>

namespace Ui
{
class OpenMoCapDialog;
}

/**
 * @brief The OpenMoCapDialog class is the dialog when opening a MoCap file (i.e. c3d)
 *
 * This class is the logic behind the dialog opening a motion capture file. It asks for
 * some additional information (e.g. time offset) and validates them as good as possible
 * (e.g. negative sample rate is impossible). Filepath, Offset and Samplerate are saved in MoCapController.
 * The actual reading is implemented in IO::readMoCapC3D.
 */
class OpenMoCapDialog : public QDialog
{
    Q_OBJECT

public:
    explicit OpenMoCapDialog(QWidget *parent, MoCapController &controller);
    OpenMoCapDialog()                                   = delete;
    OpenMoCapDialog(const OpenMoCapDialog &)            = delete;
    OpenMoCapDialog(OpenMoCapDialog &&)                 = delete;
    OpenMoCapDialog &operator=(const OpenMoCapDialog &) = delete;
    OpenMoCapDialog &operator=(OpenMoCapDialog &&)      = delete;
    ~OpenMoCapDialog() override;

    void clickedOk();

private slots:
    void on_btnAddSelection_clicked();

private:
    Ui::OpenMoCapDialog       *mUi;
    QMap<QString, MoCapSystem> mMoCapSystems;
    MoCapController           &mController;
    QWidget                   *mParent;
};

#endif // OPENMOCAPDIALOG_H
