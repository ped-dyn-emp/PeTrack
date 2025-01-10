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

#include "manualTrackpointMover.h"

#include "pMessageBox.h"

bool ManualTrackpointMover::selectTrackPoint(
    const QPointF       &pos,
    const PersonStorage &personStore,
    const QSet<size_t>  &peds,
    const FrameRange    &range)
{
    auto res = personStore.getProximalPersons(pos, peds, range);

    bool successfulSelection = res.size() == 1;

    if(successfulSelection)
    {
        mSelectedPerson = res.front();
    }
    else if(res.size() > 1)
    {
        PWarning(
            nullptr,
            "Too many trajectories",
            "PeTrack can't determine which point you meant. Try selecting fewer trajectories first.");
    }
    return successfulSelection;
}

void ManualTrackpointMover::moveTrackPoint(const QPointF &pos, PersonStorage &personStore) const
{
    if(mSelectedPerson.personID != -1 && mSelectedPerson.frame != -1)
    {
        personStore.moveTrackPoint(mSelectedPerson.personID, mSelectedPerson.frame, Vec2F{pos});
    }
}

void ManualTrackpointMover::setTrackPoint()
{
    mSelectedPerson = {-1, -1};
}
