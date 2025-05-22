/*
 * PeTrack - Software for tracking pedestrians movement in videos
 * Copyright (C) 2025 Forschungszentrum Jülich GmbH, IAS-7
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

#include "logwindow.h"

#include "logger.h"
#include "spdlog/sinks/dist_sink.h"
#include "spdlog/sinks/ringbuffer_sink.h"
#include "ui_logwindow.h"

#include <QFileDialog>
#include <QTextStream>

LogWindow::LogWindow(QWidget *parent, Ui::LogWindow *ui) : QWidget(parent)
{
    if(!ui)
    {
        mUi = new Ui::LogWindow();
    }
    else
    {
        mUi = ui;
    }

    mUi->setupUi(this);
    mUi->logText->setReadOnly(true);

    // extract messages from the ringbuffer_sink
    auto ringbufferSink = static_cast<spdlog::sinks::ringbuffer_sink_mt *>(spdlog::default_logger()->sinks()[1].get());
    std::vector<std::string> logMessages = ringbufferSink->last_formatted(10);
    for(std::string s : logMessages)
    {
        s.erase(std::remove(s.begin(), s.end(), '\n'), s.cend());
        mUi->logText->appendPlainText(QString::fromStdString(s));
    }

    distSink = std::make_shared<spdlog::sinks::dist_sink_mt>();
    qtSink   = std::make_shared<spdlog::sinks::qt_sink_mt>(mUi->logText, "appendPlainText");
    distSink->add_sink(qtSink);
    spdlog::default_logger()->sinks().push_back(distSink);
    spdlog::default_logger()->set_pattern(logger::LOG_FORMAT);
    connect(mUi->saveLogButton, &QPushButton::clicked, this, &LogWindow::saveLog);
}
void LogWindow::saveLog()
{
    QString fileName =
        QFileDialog::getSaveFileName(this, tr("Select txt file"), "", tr("txt file (*.txt);;All files (*.*)"));

    QFile logFile(fileName);
    logFile.open(QIODevice::Append | QIODevice::Text);
    QTextStream outstream(&logFile);
    outstream << mUi->logText->toPlainText() << Qt::endl;
    logFile.close();
}
LogWindow::~LogWindow()
{
    delete mUi;
    distSink->remove_sink(qtSink);
}
