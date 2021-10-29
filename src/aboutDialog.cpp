/*
 * PeTrack - Software for tracking pedestrians movement in videos
 * Copyright (C) 2010-2021 Forschungszentrum Jülich GmbH,
 * Maik Boltes, Juliane Adrian, Ricardo Martin Brualla, Arne Graf, Paul Häger, Daniel Hillebrand,
 * Deniz Kilic, Paul Lieberenz, Daniel Salden, Tobias Schrödter, Ann Katrin Seemann
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
    QWidget *                       parent,
    const QString &                 version,
    const QString &                 commitHash,
    const QString &                 commitDate,
    const QString &                 commitBranch,
    const QString &                 compiler,
    const QString &                 compilerVersion,
    const QString &                 compileDate,
    const std::vector<std::string> &authors) :
    QDialog(parent), mUi(new Ui::About)
{
    mUi->setupUi(this);

    // perform additional setup here ...
    mUi->lblVersionValue->setText(version);
    mUi->lblCommitHashValue->setText(commitHash);
    mUi->lblCommitDateValue->setText(commitDate);
    mUi->lblCommitBranchValue->setText(commitBranch);
    mUi->lblCompilerValue->setText(compiler);
    mUi->lblCompilerVersionValue->setText(compilerVersion);
    mUi->lblCompileDateValue->setText(compileDate);

    for(std::size_t i = 0; i < authors.size(); ++i)
    {
        int   row   = i / 2ul;
        int   col   = i % 2ul;
        auto *label = new QLabel();
        label->setText(QString::fromStdString(authors[i]));
        mUi->lytAuthors->addWidget(label, row, col, 1, 1);
    }
}

AboutDialog::~AboutDialog()
{
    delete mUi;
}
