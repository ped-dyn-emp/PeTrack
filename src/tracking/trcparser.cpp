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

#include "trcparser.h"


TrcLineParser::TrcLineParser(QStringView line, int lineNum) : mCurrentIndex(0), mLineNumber(lineNum)
{
    mTokens = line.split(' ');
}

bool TrcLineParser::hasMoreTokens() const
{
    return mCurrentIndex < mTokens.size();
}

int TrcLineParser::remainingTokens() const
{
    return static_cast<int>(mTokens.size() - mCurrentIndex);
}

ParseResult TrcLineParser::parseInt(int &value)
{
    if(!hasMoreTokens())
    {
        return {"Expected integer but reached end of line", mLineNumber};
    }
    bool ok;
    value = mTokens[mCurrentIndex].toInt(&ok);
    if(!ok)
    {
        return {QString("Expected integer but found '%1'").arg(mTokens[mCurrentIndex]), mLineNumber};
    }
    ++mCurrentIndex;
    return {};
}


ParseResult TrcLineParser::parseDouble(double &value)
{
    if(!hasMoreTokens())
    {
        return {"Expected double but reached end of line", mLineNumber};
    }
    bool ok;
    value = mTokens[mCurrentIndex].toDouble(&ok);
    if(!ok)
    {
        return {QString("Expected double but found '%1'").arg(mTokens[mCurrentIndex]), mLineNumber};
    }
    ++mCurrentIndex;
    return {};
}

ParseResult TrcLineParser::parseColor(QColor &color)
{
    if(remainingTokens() < 3)
    {
        return {QString("Expected 3 RGB values but only %1 tokens remaining").arg(remainingTokens())};
    }

    int  r, g, b;
    bool ok1, ok2, ok3;

    r = mTokens[mCurrentIndex].toInt(&ok1);
    g = mTokens[mCurrentIndex + 1].toInt(&ok2);
    b = mTokens[mCurrentIndex + 2].toInt(&ok3);

    if(!ok1 || !ok2 || !ok3)
    {
        return {
            QString("Invalid RGB values: '%1 %2 %3'")
                .arg(mTokens[mCurrentIndex])
                .arg(mTokens[mCurrentIndex + 1])
                .arg(mTokens[mCurrentIndex + 2]),
            mLineNumber};
    }

    // Handle special case of -1 -1 -1 (invalid/no color)
    if(r == -1 && g == -1 && b == -1)
    {
        color = QColor(); // Invalid color
    }
    else
    {
        // check rgb values
        if(r < 0 || r > 255)
        {
            return {"red value of rgb out of bounds (0, 255)", mLineNumber};
        }

        if(g < 0 || g > 255)
        {
            return {"green value of rgb out of bounds (0, 255)", mLineNumber};
        }

        if(b < 0 || b > 255)
        {
            return {"blue value of rgb out of bounds (0, 255)", mLineNumber};
        }
        color = QColor(r, g, b);
    }

    mCurrentIndex += 3;
    return {};
}

ParseResult TrcLineParser::validateTokenCount(int expected) const
{
    if(mTokens.size() != expected)
    {
        return {QString("Expected %1 tokens but found %2").arg(expected).arg(mTokens.size()), mLineNumber};
    }
    return {};
}
