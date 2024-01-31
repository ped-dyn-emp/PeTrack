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

#ifndef MANUALTRACKPOINTMOVER_H
#define MANUALTRACKPOINTMOVER_H

#include "frameRange.h"
#include "personStorage.h"

class ManualTrackpointMover
{
public:
    bool selectTrackPoint(
        const QPointF       &pos,
        const PersonStorage &personStore,
        const QSet<size_t>  &peds,
        const FrameRange    &range);
    void moveTrackPoint(const QPointF &pos, PersonStorage &personStore) const;
    void setTrackPoint();

private:
    PersonFrame mSelectedPerson = PersonFrame{-1, -1};
};

#endif // MANUALTRACKPOINTMOVER_H
