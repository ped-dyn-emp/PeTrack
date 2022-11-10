/*
 * PeTrack - Software for tracking pedestrians movement in videos
 * Copyright (C) 2022 Forschungszentrum Jülich GmbH, IAS-7
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
#include <QDialog>

#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

namespace Ui
{
class About;
}

class AboutDialog : public QDialog
{
    Q_OBJECT

public:
    AboutDialog(
        QWidget                        *parent,
        const QString                  &version,
        const QString                  &commitHash,
        const QString                  &commitDate,
        const QString                  &commitBranch,
        const QString                  &compiler,
        const QString                  &compilerVersion,
        const QString                  &compileDate,
        const std::vector<std::string> &authors);

    AboutDialog(const AboutDialog &)       = delete;
    AboutDialog &operator=(AboutDialog)    = delete;
    AboutDialog(const AboutDialog &&)      = delete;
    AboutDialog &operator=(AboutDialog &&) = delete;

    ~AboutDialog() override;

private:
    Ui::About *mUi;
};
#endif // ABOUTDIALOG_H
