#
# PeTrack - Software for tracking pedestrians movement in videos
# Copyright (C) 2025 Forschungszentrum Jülich GmbH, IAS-7
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.
#
class Point:

    def __init__(self, frame, x, y, z):
        self.frame = frame
        self.coordinates = []
        self.x = float(x)
        self.coordinates.append(self.x)
        self.y = float(y)
        self.coordinates.append(self.y)
        self.z = float(z[:-1])
        self.coordinates.append(self.z)

    def __str__(self):
        return ''.join(("Frame: ", self.frame, " X: ", str(self.x) , " Y: ", str(self.y), " Z: ", str(self.z)))

    def __repr__(self):
        return self.__str__()

    def __hash__(self) -> int:
        return hash((self.frame, tuple(self.coordinates)))


class Person:
    def __init__(self, person_id):
        self.person_id = person_id
        self.points = []

    def add_point(self, point):
        self.points.append(point)

    def __str__(self):
        return "ID: " + self.person_id

    def __hash__(self) -> int:
        return hash((self.person_id, tuple(self.points)))




