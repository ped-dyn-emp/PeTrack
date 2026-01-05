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

#ifndef PMESSAGEBOX_H
#define PMESSAGEBOX_H

#include <QCloseEvent>
#include <QDialog>
#include <QDialogButtonBox>
#include <spdlog/spdlog.h>

class QLabel;

// need to define these macros to get the file/line/function of caller
// Should be replaced with std::source_location once C++20 is used
#define PInformation(...) PMessageBox::information(__FILE__, __func__, __LINE__, __VA_ARGS__)
#define PWarning(...)     PMessageBox::warning(__FILE__, __func__, __LINE__, __VA_ARGS__)
#define PCritical(...)    PMessageBox::critical(__FILE__, __func__, __LINE__, __VA_ARGS__)
#define PQuestion(...)    PMessageBox::question(__FILE__, __func__, __LINE__, __VA_ARGS__)
#define PCustom(...)      PMessageBox::custom(__FILE__, __func__, __LINE__, __VA_ARGS__)


/**
 * @brief The PMessageBox class is similar to QMessageBox, but also allows usage in unsupervied scripts.
 *
 * The PMessageBox can be constructed like a QMessageBox. If the user is using PeTrack interactively,
 * it also opens a messagebox, like QMessageBox. But it also logs the message. If the user uses
 * PeTrack with an auto-option, the graphical boxes are not displayed anymore and only logging remains (TO BE DONE).
 */
class PMessageBox final : public QDialog
{
    Q_OBJECT

public:
    using StandardButton  = QDialogButtonBox::StandardButton;
    using StandardButtons = QDialogButtonBox::StandardButtons;

    PMessageBox(
        QWidget        *parent,
        const QString  &title,
        const QString  &msg,
        const QIcon    &icon,
        const QString  &informativeText = QString(),
        StandardButtons buttons         = StandardButton::Ok,
        StandardButton  defaultButton   = StandardButton::NoButton);
    PMessageBox(
        QWidget       *parent,
        const QString &title,
        const QString &msg,
        const QIcon   &icon,
        const QString &informativeText = QString(),
        QStringList    customButtons   = QStringList(),
        QString        defaultButton   = QString());

    static int information(
        const char     *file,
        const char     *func,
        int             line,
        QWidget        *parent,
        const QString  &title,
        const QString  &text,
        StandardButtons buttons       = StandardButton::Ok,
        StandardButton  defaultButton = StandardButton::NoButton);
    static int warning(
        const char     *file,
        const char     *func,
        int             line,
        QWidget        *parent,
        const QString  &title,
        const QString  &text,
        StandardButtons buttons       = StandardButton::Ok,
        StandardButton  defaultButton = StandardButton::NoButton);
    static int critical(
        const char     *file,
        const char     *func,
        int             line,
        QWidget        *parent,
        const QString  &title,
        const QString  &text,
        StandardButtons buttons       = StandardButton::Ok,
        StandardButton  defaultButton = StandardButton::NoButton);
    [[nodiscard]] static int question(
        const char     *file,
        const char     *func,
        int             line,
        QWidget        *parent,
        const QString  &title,
        const QString  &text,
        StandardButtons buttons       = (StandardButton::Yes | StandardButton::No),
        StandardButton  defaultButton = StandardButton::NoButton);
    static int custom(
        const char    *file,
        const char    *func,
        int            line,
        QWidget       *parent,
        const QString &title,
        const QString &text,
        QStringList    customButtons = QStringList(),
        QString        defaultButton = QString());

private:
    static void setMinimumWidth(QLabel *textLabel);
    void        closeEvent(QCloseEvent *event);
};

#endif // PMESSAGEBOX_H
