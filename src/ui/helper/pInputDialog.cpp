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
#include "pInputDialog.h"

#include <QPlainTextEdit>
#include <QTimer>

QString PInputDialog::getMultiLineText(
    QWidget        *parent,
    const QString  &title,
    const QString  &label,
    const QString  &text,
    bool           *ok,
    Qt::WindowFlags flags)
{
    QInputDialog dialog{parent, flags};
    dialog.setWindowTitle(title);
    dialog.setLabelText(label);
    dialog.setTextValue(text);
    dialog.setInputMode(QInputDialog::TextInput);
    dialog.setOption(QInputDialog::UsePlainTextEditForTextInput, true);

    QPlainTextEdit *textEdit = dialog.findChild<QPlainTextEdit *>();
    if(textEdit)
    {
        textEdit->setPlainText(text);
        QTimer::singleShot(
            0,
            &dialog,
            [textEdit]()
            {
                QTextCursor cursor = textEdit->textCursor();
                cursor.movePosition(QTextCursor::End);
                cursor.clearSelection();
                textEdit->setTextCursor(cursor);
                textEdit->setFocus();
            });
    }
    const int result = dialog.exec();
    QString   value  = (result == QDialog::Accepted) ? dialog.textValue() : QString();
    if(ok)
    {
        *ok = (result == QDialog::Accepted);
    }
    return value;
}