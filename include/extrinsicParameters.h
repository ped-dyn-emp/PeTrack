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

#ifndef EXTRINSICPARAMETERS_H
#define EXTRINSICPARAMETERS_H

struct ExtrinsicParameters
{
    double trans1 = 0;
    double trans2 = 0;
    double trans3 = -500;

    double rot1 = 0;
    double rot2 = 0;
    double rot3 = 0;

    friend bool operator==(const ExtrinsicParameters &lhs, const ExtrinsicParameters &rhs)
    {
        return lhs.rot1 == rhs.rot1 && lhs.rot2 == rhs.rot2 && lhs.rot3 == rhs.rot3 && lhs.trans1 == rhs.trans1 &&
               lhs.trans2 == rhs.trans2 && lhs.trans3 == rhs.trans3;
    }
};

#endif // EXTRINSICPARAMETERS_H
