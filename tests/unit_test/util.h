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

#ifndef UTIL_H
#define UTIL_H

#include <QDomNode>
#include <QTextStream>

namespace
{

/*
 * @brief Transforms node to QString for debugging/logging
 *
 */
QString nodeToString(QDomNode &node)
{
    QString     str;
    QTextStream stream(&str);
    node.save(stream, 4 /*indent*/);
    return str;
}

} // anonymous namespace
#endif // UTIL_H
