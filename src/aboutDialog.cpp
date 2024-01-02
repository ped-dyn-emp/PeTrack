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

#include "aboutDialog.h"

#include "ui_about.h"

AboutDialog::AboutDialog(
    QWidget                        *parent,
    const QString                  &version,
    const QString                  &commitHash,
    const QString                  &commitDate,
    const QString                  &commitBranch,
    const QString                  &compiler,
    const QString                  &compilerVersion,
    const QString                  &compileDate,
    const std::vector<std::string> &authors) :
    QDialog(parent), mUi(new Ui::About)
{
    mUi->setupUi(this);

    /* clang-format off */
    mUi->versionInfo->setText(
        "<table> <tr> <td>Version</td> <td>" + version + "</td> </tr> " +
                "<tr> </tr> "+
                "<tr> <td>Commit Hash: </td> " + "<td>" + commitHash + "</td> </tr> " +
                "<tr> <td>Commit Date: </td> <td>" + commitDate + "</td> </tr> " +
                "<tr> <td>Commit Branch: </td> <td>" + commitBranch + "</td> </tr> " +
                "<tr> </tr> " +
                "<tr> <td>Compiler: </td> <td>" + compiler + "</td> </tr> " +
                "<tr> <td>Compiler Version: </td> <td>" + compilerVersion + "</td> </tr> " +
                "<tr> <td>Compile Date: </td> <td>" + compileDate + "</td> </tr> " +
        "</table>");
    /* clang-format on */

    for(std::size_t i = 0; i < authors.size(); ++i)
    {
        int   row   = static_cast<int>(i / 2ul);
        int   col   = static_cast<int>(i % 2ul);
        auto *label = new QLabel();
        label->setText(QString::fromStdString(authors[i]));
        mUi->lytAuthors->addWidget(label, row, col, 1, 1);
    }
}

AboutDialog::~AboutDialog()
{
    delete mUi;
}
