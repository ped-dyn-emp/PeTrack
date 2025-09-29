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

#include <QColor>
#include <QList>
#include <QStringView>
#include <qcontainerfwd.h>
#include <utility>

struct ParseResult
{
    bool    success;
    QString errorMessage;
    int     lineNumber;

    ParseResult() : success(true), lineNumber(0) {}
    ParseResult(QString error, int line = 0) : success(false), errorMessage(std::move(error)), lineNumber(line) {}
};


class TrcLineParser
{
private:
    QList<QStringView> mTokens;
    int                mCurrentIndex;
    int                mLineNumber;

public:
    TrcLineParser(QStringView line, int lineNum);

    bool hasMoreTokens() const;
    int  remainingTokens() const;

    ParseResult parseInt(int &value);
    ParseResult parseDouble(double &value);
    ParseResult parseColor(QColor &color);

    ParseResult validateTokenCount(int expected) const;
};
