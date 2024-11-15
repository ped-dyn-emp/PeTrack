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

#ifndef LOGWINDOW_H
#define LOGWINDOW_H

#include "spdlog//spdlog.h"
#include "spdlog/logger.h"
#include "spdlog/sinks/dist_sink.h"
#include "spdlog/sinks/qt_sinks.h"

#include <QWidget>

namespace Ui
{
class LogWindow;
}

class LogWindow : public QWidget
{
    Q_OBJECT

public:
    void saveLog();

    explicit LogWindow(QWidget *parent, Ui::LogWindow *mUi);
    virtual ~LogWindow();
    LogWindow(const LogWindow &)       = delete;
    LogWindow &operator=(LogWindow)    = delete;
    LogWindow(const LogWindow &&)      = delete;
    LogWindow &operator=(LogWindow &&) = delete;

private:
    Ui::LogWindow                                        *mUi;
    std::shared_ptr<spdlog::sinks::dist_sink<std::mutex>> distSink;
    std::shared_ptr<spdlog::sinks::qt_sink<std::mutex>>   qtSink;
};

#endif // LOGWINDOW_H
