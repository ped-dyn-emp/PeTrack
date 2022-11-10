/*
 * PeTrack - Software for tracking pedestrians movement in videos
 * Copyright (C) 2022 Forschungszentrum Jülich GmbH, IAS-7
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

#ifndef IO_H
#define IO_H

#include <QString>
#include <ezc3d_all.h>
#include <opencv2/opencv.hpp>
#include <string>
#include <unordered_map>
#include <variant>
class MoCapStorage;
class MoCapPerson;
class MoCapPersonMetadata;


namespace IO
{
std::variant<std::unordered_map<int, float>, std::string> readHeightFile(const QString &heightFileName);

void readMoCapC3D(MoCapStorage &storage, const MoCapPersonMetadata &metadata);
void readSkeletonC3D_XSENS(
    const ezc3d::c3d                                                           &c3d,
    MoCapPerson                                                                &person,
    const std::function<cv::Point3f(const ezc3d::DataNS::Points3dNS::Point &)> &c3dToPoint3f);

std::variant<std::unordered_map<int, int>, std::string> readMarkerIDFile(const QString &markerFileName);

std::vector<std::string> readAuthors(const QString &authorsFile);
} // namespace IO
#endif // IO_H
