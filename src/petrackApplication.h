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

#ifndef PETRACK_APPLICATION_H
#define PETRACK_APPLICATION_H

#include <QApplication>

class QEvent;
class QString;

class PetrackApplication : public QApplication
{
    Q_OBJECT
public:
    PetrackApplication(int &argc, char **argv) : QApplication(argc, argv) {}
    bool event(QEvent *event) override;

signals:
    void fileOpened(const QString &file);
};

#endif
