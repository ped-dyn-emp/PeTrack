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

#ifndef PETRACK_COLORLIST_H
#define PETRACK_COLORLIST_H

#include <QColor>
#include <stdexcept>

/**
 * Utility class for a cyclic list of colors that can be traversed through.
 *
 * This means getting the next color of the last entry will move the index back to the first entry.
 */
class ColorList
{
public:
    /**
     * Create a new Color List with a vector of predefined colors.
     *
     * @throws std::invalid_argument if the given vector is empty
     */
    ColorList(const std::vector<QColor> &colors) : mColors(colors)
    {
        if(colors.empty())
        {
            throw std::invalid_argument("List of colors may not be empty!");
        }
    }

    /**
     * Get next color and move index.
     */
    QColor next()
    {
        QColor color = peekNext();
        mIndex       = (mIndex + 1) % mColors.size();
        return color;
    }

    /**
     * Get next color without changing the index.
     */
    QColor peekNext() const { return mColors.at(mIndex); }

    /**
     * Get previous color and move index back.
     */
    QColor previous()
    {
        mIndex = (mIndex > 0) ? mIndex - 1 : mColors.size() - 1;
        return mColors.at(mIndex);
    }

    /**
     * @return a predefined ColorList of different shades of red.
     *
     * @see https://coolors.co/palette/641220-6e1423-85182a-a11d33-a71e34-b21e35-bd1f36-c71f37-da1e37-e01e37
     */
    static ColorList red()
    {
        return ColorList(
            {QColor(0x641220),
             QColor(0xda1e37),
             QColor(0x85182a),
             QColor(0xbd1f36),
             QColor(0xa11d33),
             QColor(0xb21e35),
             QColor(0xa71e34),
             QColor(0xc71f37),
             QColor(0x6e1423),
             QColor(0xe01e37)});
    }

    /**
     * @return a predefined ColorList of different shades of yellow.
     *
     * @see https://coolors.co/palette/ffe169-fad643-edc531-dbb42c-c9a227-b69121-ad881e-a47e1b-926c15-805b10
     */
    static ColorList yellow()
    {
        return ColorList(
            {QColor(0xffe169),
             QColor(0xb69121),
             QColor(0xfad643),
             QColor(0xa47e1b),
             QColor(0xedc531),
             QColor(0xad881e),
             QColor(0xdbb42c),
             QColor(0x926c15),
             QColor(0xc9a227),
             QColor(0x805b10)});
    }


    /**
     * @return a predefined ColorList of different shades of green.
     *
     * @see https://coolors.co/palette/004b23-006400-007200-008000-38b000-70e000-9ef01a-ccff33
     */
    static ColorList green()
    {
        return ColorList(
            {QColor(0x004b23),
             QColor(0x38b000),
             QColor(0x006400),
             QColor(0x70e000),
             QColor(0x007200),
             QColor(0x9ef01a),
             QColor(0x008000),
             QColor(0xccff33)});
    }

private:
    std::vector<QColor> mColors;
    size_t              mIndex{0};
};


#endif // PETRACK_COLORLIST_H
