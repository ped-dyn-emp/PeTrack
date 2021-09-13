from dataclasses import dataclass
from typing import List


@dataclass
class Point():
    x: float
    y: float
    stereo_x: float
    stereo_y: float
    stereo_z: float
    qual: int
    col_x: float
    col_y: float
    color: str
    markerID: int

    def __hash__(self) -> int:
        return hash((self.x,self.y,self.markerID,self.stereo_x,self.stereo_y,self.color,self.qual))

# TODO Hash Funktion vereinfachen; performance
class Person:
    def __init__(self, person_str: str, points_str: str, id: int, trc_version: int):
        self.id = int(id)
        self.points: List[Point] = []

        # Version 4, sonst nichts unterstützt
        person_values = person_str.split(' ')
        self.nr = int(person_values[0])
        self.height = float(person_values[1])
        self.first_frame = int(person_values[2])
        self.last_frame = int(person_values[3])
        self.colCount = int(person_values[4])
        self.color = person_values[5] + " " + person_values[6] + " " + person_values[7]
        if trc_version >= 4:
            self.markerID = int(person_values[8])
            self.numTrackedPoints = int(person_values[9])
        else:
            self.numTrackedPoints = int(person_values[8])

        points_str = points_str.split("\n")
        for line in points_str:
            point_values = line.split(' ')
            x = float(point_values[0])
            y = float(point_values[1])
            stereo_x = float(point_values[2])
            stereo_y = float(point_values[3])
            stereo_z = float(point_values[4])
            qual = int(point_values[5])
            col_x = float(point_values[6])
            col_y = float(point_values[7])
            color = point_values[8] + " " + point_values[9] + " " + point_values[10]
            markerID = int(point_values[11])
            self.points.append(Point(x,y,stereo_x,stereo_y,stereo_z,qual,col_x,col_y,color,markerID))

    def __hash__(self) -> int:
        return hash((self.id, self.nr, self.height, self.first_frame, self.last_frame, self.colCount, self.color, self.numTrackedPoints))

    def __str__(self) -> str:
        return "ID: " + str(self.id)

    def __repr__(self) -> str:
        return self.__str__()