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
 * along with this program.  If not, see <https://cdwww.gnu.org/licenses/>.
 */

#include "pMessageBox.h"

#include "helper.h"
#include "logger.h"

#include <QApplication>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QStyle>
#include <QTextDocument>

/**
 * @brief Constructs a PMessageBox
 *
 * @param parent pointer to parent widget
 * @param title title of the dialog window
 * @param msg message to display on the dialog
 * @param icon icon to display (e.g. yellow warning triangle)
 * @param informativeText longer text (or table via html/rich text) to show
 * @param buttons buttons for the user to click (default: only Ok)
 * @param defaultButton button used when pressing enter
 */
PMessageBox::PMessageBox(
    QWidget        *parent,
    const QString  &title,
    const QString  &msg,
    const QIcon    &icon,
    const QString  &informativeText,
    StandardButtons buttons,
    StandardButton  defaultButton) :
    QDialog(
        parent,
        Qt::MSWindowsFixedSizeDialogHint | Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint)
{
    QGridLayout *layout = new QGridLayout();
    setLayout(layout);

    QFont                    f = QApplication::font("QMessageBox");
    Qt::TextInteractionFlags flags(style()->styleHint(QStyle::SH_MessageBox_TextInteractionFlags, nullptr, this));

    QLabel *detailedText = new QLabel(this);
    if(!informativeText.isEmpty())
    {
        detailedText->setWordWrap(true);
        detailedText->setTextInteractionFlags(flags);
        detailedText->setFont(f);
        detailedText->setText(informativeText);
        layout->addWidget(detailedText, 1, 1);
    }

    QLabel *text = new QLabel(this);
    text->setWordWrap(true);
    text->setTextInteractionFlags(flags);
    text->setFont(f);
    text->setText(msg);
    layout->addWidget(text, 0, 1);

    setWindowTitle(title);
    QLabel *infoIcon = new QLabel(this);
    int     iconSize = style()->pixelMetric(QStyle::PM_MessageBoxIconSize, nullptr, this);
    infoIcon->setPixmap(icon.pixmap(iconSize, iconSize));
    layout->addWidget(infoIcon, 0, 0);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(buttons, this);
    layout->addWidget(buttonBox, 2, 1);
    if(defaultButton != StandardButton::NoButton)
    {
        QPushButton *def = buttonBox->button(defaultButton);
        if(def)
        {
            def->setAutoDefault(false);
            def->setDefault(true);
        }
        else
        {
            SPDLOG_WARN("Given default button does is non-specified button");
        }
    }

    // return the clicked button; -1 if none was clicked
    connect(
        buttonBox,
        &QDialogButtonBox::clicked,
        this,
        [=](QAbstractButton *button)
        {
            int ret = buttonBox->standardButton(button);
            this->done(ret);
        });

    layout->setSpacing(20);

    setMinimumWidth(text);
    setMinimumWidth(detailedText);

    setFixedSize(sizeHint());
}
/**
 * @brief Constructs a PMessageBox
 *
 * @param parent pointer to parent widget
 * @param title title of the dialog window
 * @param msg message to display on the dialog
 * @param icon icon to display (e.g. yellow warning triangle)
 * @param informativeText longer text (or table via html/rich text) to show
 * @param customButtons custom buttons for the user to click
 * @param defaultButton button used when pressing enter
 */
PMessageBox::PMessageBox(
    QWidget       *parent,
    const QString &title,
    const QString &msg,
    const QIcon   &icon,
    const QString &informativeText,
    QStringList    customButtons,
    QString        defaultButton) :
    QDialog(
        parent,
        Qt::MSWindowsFixedSizeDialogHint | Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint)
{
    QGridLayout *layout = new QGridLayout();
    setLayout(layout);

    QFont                    f = QApplication::font("QMessageBox");
    Qt::TextInteractionFlags flags(style()->styleHint(QStyle::SH_MessageBox_TextInteractionFlags, nullptr, this));

    QLabel *detailedText = new QLabel(this);
    if(!informativeText.isEmpty())
    {
        detailedText->setWordWrap(true);
        detailedText->setTextInteractionFlags(flags);
        detailedText->setFont(f);
        detailedText->setText(informativeText);
        layout->addWidget(detailedText, 1, 1);
    }

    QLabel *text = new QLabel(this);
    text->setWordWrap(true);
    text->setTextInteractionFlags(flags);
    text->setFont(f);
    text->setText(msg);
    layout->addWidget(text, 0, 1);

    setWindowTitle(title);
    QLabel *infoIcon = new QLabel(this);
    int     iconSize = style()->pixelMetric(QStyle::PM_MessageBoxIconSize, nullptr, this);
    infoIcon->setPixmap(icon.pixmap(iconSize, iconSize));
    layout->addWidget(infoIcon, 0, 0);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(this);
    if(!customButtons.isEmpty())
    {
        for(auto button : customButtons)
        {
            buttonBox->addButton(button, QDialogButtonBox::ActionRole);
        }
    }
    else
    {
        SPDLOG_WARN("Message Box contains no buttons");
    }
    layout->addWidget(buttonBox, 2, 1);
    if(!defaultButton.isEmpty() and !customButtons.isEmpty())
    {
        bool found = false;
        for(auto button : buttonBox->buttons())
        {
            QPushButton *def = (QPushButton *) button;
            def->setAutoDefault(false);
            if(button->text() == defaultButton)
            {
                found = true;
                def->setDefault(true);
            }
        }
        if(!found)
        {
            QPushButton *def = (QPushButton *) buttonBox->buttons()[0];
            def->setDefault(true);
            SPDLOG_WARN("Given default button does is non-specified button");
        }
    }
    // return the index of clicked button, -1 if none was clicked
    connect(
        buttonBox,
        &QDialogButtonBox::clicked,
        this,
        [=](QAbstractButton *button)
        {
            int ret = buttonBox->buttons().indexOf(button);
            this->done(ret);
        });
    layout->setSpacing(20);

    setMinimumWidth(text);
    setMinimumWidth(detailedText);

    setFixedSize(sizeHint());
}
/**
 * @brief Opens a dialog with given title, text and buttons and also logs the message.
 *
 * This Method opens a modal dialog with given text and buttons. The
 * dialogue has an information-icon and is logged with info-level.
 *
 * The pressed button is returned, i.e. PMessageBox::StandardButton::Yes.
 *
 * @param parent Pointer to parent
 * @param title title of dialog window
 * @param text message to the user
 * @param buttons buttons to use (default: OK)
 * @param defaultButton button to press, when pressing enter
 * @return clicked button
 */
int PMessageBox::information(
    const char                  *file,
    const char                  *func,
    int                          line,
    QWidget                     *parent,
    const QString               &title,
    const QString               &text,
    PMessageBox::StandardButtons buttons,
    PMessageBox::StandardButton  defaultButton)
{
    auto logger = spdlog::get(logger::messageBoxLoggerName);
    SPDLOG_LOGGER_INFO(logger, logger::messageBoxLogFormat, file_name(file), line, func, "info", text);

    // if no GUI gets displayed and no button except OK is used (no return Value used)
    // Then only log it
    if(QGuiApplication::platformName() == "offscreen")
    {
        if(buttons & (~StandardButton::Ok))
        {
            throw std::runtime_error(
                QString("user-interaction demanded during offscreen mode with message\n%1").arg(text).toStdString());
        }

        return StandardButton::NoButton;
    }

    QIcon       icon = QApplication::style()->standardIcon(QStyle::SP_MessageBoxInformation);
    PMessageBox msg  = PMessageBox(parent, title, text, icon, QString(), buttons, defaultButton);
    return msg.exec();
}

/**
 * @brief Opens a dialog with given title, text and buttons and also logs the message.
 *
 * This Method opens a modal dialog with given text and buttons. The
 * dialogue has an warning-icon and is logged with warning-level.
 *
 * The pressed button is returned, i.e. PMessageBox::StandardButton::Yes.
 *
 * @param parent Pointer to parent
 * @param title title of dialog window
 * @param text message to the user
 * @param buttons buttons to use (default: OK)
 * @param defaultButton button to press, when pressing enter
 * @return clicked button
 */
int PMessageBox::warning(
    const char                  *file,
    const char                  *func,
    int                          line,
    QWidget                     *parent,
    const QString               &title,
    const QString               &text,
    PMessageBox::StandardButtons buttons,
    PMessageBox::StandardButton  defaultButton)
{
    auto logger = spdlog::get(logger::messageBoxLoggerName);
    SPDLOG_LOGGER_WARN(logger, logger::messageBoxLogFormat, file_name(file), line, func, "warning", text);

    // if no GUI gets displayed and no button except OK is used (no return Value used)
    // Then only log it
    if(QGuiApplication::platformName() == "offscreen")
    {
        if(buttons & (~StandardButton::Ok))
        {
            throw std::runtime_error(
                QString("user-interaction demanded during offscreen mode with message\n%1").arg(text).toStdString());
        }

        return StandardButton::NoButton;
    }

    QIcon       icon = QApplication::style()->standardIcon(QStyle::SP_MessageBoxWarning);
    PMessageBox msg  = PMessageBox(parent, title, text, icon, QString(), buttons, defaultButton);
    return msg.exec();
}

/**
 * @brief Opens a dialog with given title, text and buttons and also logs the message.
 *
 * This Method opens a modal dialog with given text and buttons. The
 * dialogue has an critical-icon and is logged with critical(error?)-level.
 *
 * The pressed button is returned, i.e. PMessageBox::StandardButton::Yes.
 *
 * @param parent Pointer to parent
 * @param title title of dialog window
 * @param text message to the user
 * @param buttons buttons to use (default: OK)
 * @param defaultButton button to press, when pressing enter
 * @return clicked button
 */
int PMessageBox::critical(
    const char                  *file,
    const char                  *func,
    int                          line,
    QWidget                     *parent,
    const QString               &title,
    const QString               &text,
    PMessageBox::StandardButtons buttons,
    PMessageBox::StandardButton  defaultButton)
{
    auto logger = spdlog::get(logger::messageBoxLoggerName);
    SPDLOG_LOGGER_ERROR(logger, logger::messageBoxLogFormat, file_name(file), line, func, "error", text);

    // if no GUI gets displayed and no button except OK is used (no return Value used)
    // Then only log it
    if(QGuiApplication::platformName() == "offscreen")
    {
        if(buttons & (~StandardButton::Ok))
        {
            throw std::runtime_error(
                QString("user-interaction demanded during offscreen mode with message\n%1").arg(text).toStdString());
        }

        return StandardButton::NoButton;
    }

    QIcon       icon = QApplication::style()->standardIcon(QStyle::SP_MessageBoxCritical);
    PMessageBox msg  = PMessageBox(parent, title, text, icon, QString(), buttons, defaultButton);
    return msg.exec();
}

/**
 * @brief Opens a dialog with given title, text and buttons and returns chosen action.
 *
 * This Method opens a modal dialog with given text and buttons. The
 * Dialog defaults to a yes and no button and returns accordingly.
 *
 * The pressed button is returned, i.e. PMessageBox::StandardButton::Yes.
 *
 * @param parent Pointer to parent
 * @param title title of dialog window
 * @param text message to the user
 * @param buttons buttons to use (default: OK)
 * @param defaultButton button to press, when pressing enter
 * @return clicked button
 */
int PMessageBox::question(
    const char * /*file*/,
    const char * /*func*/,
    int /*line*/,
    QWidget                     *parent,
    const QString               &title,
    const QString               &text,
    PMessageBox::StandardButtons buttons,
    PMessageBox::StandardButton  defaultButton)
{
    QIcon       icon = QApplication::style()->standardIcon(QStyle::SP_MessageBoxQuestion);
    PMessageBox msg  = PMessageBox(parent, title, text, icon, QString(), buttons, defaultButton);

    // no debout, since question **demands** user interaction; also no support for scripting to come
    if(QGuiApplication::platformName() == "offscreen")
    {
        throw std::runtime_error(
            QString("user-interaction demanded during offscreen mode with message\n%1").arg(text).toStdString());
    }

    return msg.exec();
}
/**
 * @brief Opens a dialog with given title, text and buttons and returns chosen action.
 *
 * This Method opens a modal dialog with given text and buttons. The
 *
 * The index of the pressed button is returned, -1 if none was pressed
 *
 * @param parent Pointer to parent
 * @param title title of dialog window
 * @param text message to the user
 * @param customButtons customButtons to use
 * @param defaultButton button to press, when pressing enter
 * @return clicked button index
 */
int PMessageBox::custom(
    const char * /*file*/,
    const char * /*func*/,
    int /*line*/,
    QWidget       *parent,
    const QString &title,
    const QString &text,
    QStringList    customButtons,
    QString        defaultButton)
{
    QIcon       icon = QApplication::style()->standardIcon(QStyle::SP_MessageBoxInformation);
    PMessageBox msg  = PMessageBox(parent, title, text, icon, QString(), customButtons, defaultButton);

    if(QGuiApplication::platformName() == "offscreen")
    {
        throw std::runtime_error(
            QString("user-interaction demanded during offscreen mode with message\n%1").arg(text).toStdString());
    }

    return msg.exec();
}

void PMessageBox::setMinimumWidth(QLabel *textLabel)
{
    QTextDocument doc;
    if(Qt::mightBeRichText(textLabel->text()))
    {
        doc.setHtml(textLabel->text());
    }
    else
    {
        doc.setPlainText(textLabel->text());
    }
    doc.setDefaultFont(textLabel->font());
    doc.setTextWidth(textLabel->fontMetrics().averageCharWidth() * 120);

    textLabel->setMinimumWidth(static_cast<int>(doc.idealWidth()));
}
void PMessageBox::closeEvent(QCloseEvent *event)
{
    this->done(-1);
    event->accept();
}
