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
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef PLAUSIBILITY_H
#define PLAUSIBILITY_H

#include "personStorage.h"

#include <string>
#include <vector>

class QProgressDialog;
class PersonStorage;

namespace plausibility
{
enum class CheckStatus
{
    New,
    Resolved,
    Changed
};

enum class CheckType
{
    Velocity,
    Length,
    Inside,
    Equality
};

struct FailedCheck
{
    size_t      pers{};
    int         frame{};
    std::string message{};
    CheckType   type{};
    CheckStatus status = CheckStatus::New; //< all checks are new when created
};

inline bool operator==(const FailedCheck &first, const FailedCheck &second)
{
    return (first.pers == second.pers && first.frame == second.frame && first.type == second.type);
}

inline bool operator<(const FailedCheck &first, const FailedCheck &second)
{
    return std::tie(first.pers, first.frame) < std::tie(second.pers, second.frame);
}


std::vector<FailedCheck>
checkLength(const PersonStorage &personStorage, QProgressDialog *progressDialog, int minLength);

std::vector<FailedCheck> checkInside(
    const PersonStorage &personStorage,
    QProgressDialog     *progressDialog,
    const cv::Size      &sequenceSize,
    int                  imageBorderSize,
    QRectF               rect,
    int                  firstFrame,
    int                  lastFrame,
    int                  margin);

std::vector<FailedCheck> checkVelocityVariation(const PersonStorage &personStorage, QProgressDialog *progressDialog);

std::vector<FailedCheck> checkEquality(
    const PersonStorage &personStorage,
    QProgressDialog     *progressDialog,
    Petrack             &petrack,
    double               headSizeFactor);

} // namespace plausibility

Q_DECLARE_METATYPE(plausibility::FailedCheck)

#endif // PLAUSIBILITY_H
