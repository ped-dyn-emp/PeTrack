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

#ifndef TRACKERCONSTANTS_H
#define TRACKERCONSTANTS_H

// war 1.5, aber bei bildauslassungen kann es ungewollt zuschlagen (bei 3 ist ein ausgelassener frame mgl, bei 2 wieder
// ein problem)
inline constexpr double EXTRAPOLATE_FACTOR = 3.;

// maximale anzahl an gleichzeitig getrackten personen
inline constexpr int MAX_COUNT = 1500;

// maximale zahl an frames zwischen denen noch getrackt wird
inline constexpr int MAX_STEP_TRACK = 5;

// maximaler fehler beim tracken, so das noch punkt hinzugefuegt wird
// um am ende des tracking nicht am bildrand herumzukrakseln!
inline constexpr float MAX_TRACK_ERROR = 200.F;

inline constexpr float MAX_BACKTRACK_DIST = 20.f;

// minimale Personengroesse zur Identifizierung, dass Hoehe nicht gesetz, war -1, was aber an Treppen schlecht war
inline constexpr double MIN_HEIGHT = -100'000.;

#endif
