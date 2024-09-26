import argparse
from typing import Callable, List, Set, Tuple

try:
    import cv2
except ImportError:
    print("You need to install OpenCV for Python (e.g. pip install opencv-python)")
    exit(1)
from Person_trc import Person
import xml.etree.ElementTree as ET
import numpy as np
import math
import re
from pathlib import Path
from handmade_test import MAX_DIFF, calc_diff, parse_trc, HandmadeComparison


class ComparisonVideoPlayer:
    # TODO: Longterm; Make thread, so we can add UI-Elements via tkinter
    # These could be e.g. looking at a number of user-chosen trajectories
    # Which could be helpful for the 'no counterpart' case
    def __init__(self, pet: str) -> None:
        # super().__init__()
        cv2.namedWindow("Comparison", cv2.WINDOW_NORMAL)
        PET = Path(pet)

        # NOTE: will only work for tests this way. (Need this naming scheme)
        self.truth_filename = PET.parent / (str(PET.stem) + "_truth.trc")
        self.test_filename = PET.parent / (str(PET.stem) + "_test.trc")

        tree = ET.parse(PET)
        root = tree.getroot()

        vid_path = PET.with_name(root.find("MAIN").attrib["SRC"].split(";")[-1])
        intrinsics = (
            root.find("CONTROL").find("CALIBRATION").find("INTRINSIC_PARAMETERS")
        )
        assert intrinsics is not None
        self.border = int(
            float(
                root.find("CONTROL").find("CALIBRATION").find("BORDER").attrib["VALUE"]
            )
            * 2
        )

        if intrinsics.attrib["EXT_MODEL_ENABLED"] == "1":
            ext_model = intrinsics.find("EXT_MODEL")
            assert ext_model is not None
            calib = ext_model.attrib
        else:
            old_model = intrinsics.find("OLD_MODEL")
            assert old_model is not None
            calib = old_model.attrib

        camera = np.array(
            [
                [float(calib["FX"]), 0, float(calib["CX"])],
                [0, float(calib["FY"]), float(calib["CY"])],
                [0, 0, 1],
            ]
        )
        distortion = np.array(
            [
                calib["R2"],
                calib["R4"],
                calib["TX"],
                calib["TY"],
                calib["R6"],
                calib["K4"],
                calib["K5"],
                calib["K6"],
            ],
            np.float32,
        )

        self.stream: cv2.VideoCapture = cv2.VideoCapture(str(vid_path))
        _, frame = self.stream.read()
        frame = cv2.copyMakeBorder(
            frame,
            self.border,
            self.border,
            self.border,
            self.border,
            cv2.BORDER_CONSTANT,
        )

        self.map1, self.map2 = cv2.initUndistortRectifyMap(
            camera, distortion, np.eye(3), camera, frame.shape[:-1][::-1], cv2.CV_16SC2
        )

        self.test_trajectories = parse_trc(str(self.test_filename))
        self.truth_trajectories = parse_trc(str(self.truth_filename))

        self.text_offset = (50, 50)
        self.text_scale = 1.25
        self.text_thickness = 2

    def get_proximal_people(self, seed_person: Person):
        """Gets the indices of the people which could have been a match,
        according to the maximum error.

        This function finds people who could have been the match. It is used
        for people who do not have a match. If you see one truth and one test
        trajectory, it probably was just matched to the other trajectory and you
        have a double tracked person.

        :param seed_person: Person to match to
        :type seed_person: Person
        :return: (truth, test) People who match to seed_person, split into test and truth
        :rtype: Tuple[List[int], List[int]]
        """
        nearby_people_test: List[int] = []
        nearby_people_truth: List[int] = []
        for person in self.test_trajectories:
            if (
                seed_person.last_frame < person.first_frame
                or seed_person.first_frame > person.last_frame
            ):
                continue
            if seed_person.first_frame > person.first_frame:
                frame_diff = seed_person.first_frame - person.first_frame
                diff = calc_diff(seed_person.points, person.points[frame_diff:])
            elif seed_person.first_frame < person.first_frame:
                frame_diff = person.first_frame - seed_person.first_frame
                diff = calc_diff(seed_person.points[frame_diff:], person.points)
            else:
                diff = calc_diff(seed_person.points, person.points)

            if diff >= MAX_DIFF:
                continue
            nearby_people_test.append(person.id + 1)
        for person in self.truth_trajectories:
            if (
                seed_person.last_frame < person.first_frame
                or seed_person.first_frame > person.last_frame
            ):
                continue
            if seed_person.first_frame > person.first_frame:
                frame_diff = seed_person.first_frame - person.first_frame
                diff = calc_diff(seed_person.points, person.points[frame_diff:])
            elif seed_person.first_frame < person.first_frame:
                frame_diff = person.first_frame - seed_person.first_frame
                diff = calc_diff(seed_person.points[frame_diff:], person.points)
            else:
                diff = calc_diff(seed_person.points, person.points)

            if diff >= MAX_DIFF:
                continue
            nearby_people_truth.append(person.id + 1)
        return nearby_people_truth, nearby_people_test

    def play_video(
        self, start: int, end: int, draw_callback: Callable[[np.ndarray, int], None]
    ):
        """Plays the video from frame start to frame end, calling the draw_callback each frame

        This function plays the video. It uses distortion and border like PeTrack would.
        To draw on the video, a callback is provided, which draws onto the frame it's given.

        :param stream: Videostream
        :type stream: cv2.VideoCapture
        :param start: Starting frame
        :type start: int
        :param end: Ending frame
        :type end: int
        :param draw_callback: Function to call for drawing on the frame
        :type draw_callback: Callable[[np.ndarray, int], None]
        """
        displayed_frame = start
        self.stream.set(cv2.CAP_PROP_POS_FRAMES, start)

        playing = True
        while (k := cv2.waitKey(20)) != 110:
            frame_to_display = displayed_frame
            if not playing:
                if k == ord("a") or k == ord("d"):
                    if k == ord("a"):
                        frame_to_display = max(displayed_frame - 1, start)
                    if k == ord("d"):
                        frame_to_display = min(displayed_frame + 1, end)

            else:
                frame_to_display = displayed_frame + 1
                if frame_to_display > end:
                    frame_to_display = start

            if frame_to_display != displayed_frame:
                if frame_to_display <= displayed_frame:
                    self.stream.set(cv2.CAP_PROP_POS_FRAMES, frame_to_display)

                grabbed, frame = self.stream.read()
                if not grabbed:
                    self.stream.set(cv2.CAP_PROP_POS_FRAMES, start)
                    frame_to_display = start
                    _, frame = self.stream.read()

                frame = cv2.copyMakeBorder(
                    frame,
                    self.border,
                    self.border,
                    self.border,
                    self.border,
                    cv2.BORDER_CONSTANT,
                )
                frame = cv2.remap(
                    frame,
                    self.map1,
                    self.map2,
                    cv2.INTER_LINEAR,
                    borderMode=cv2.BORDER_CONSTANT,
                )

                draw_callback(frame, frame_to_display)
                cv2.putText(
                    frame,
                    "Frame: {:6d}".format(frame_to_display),
                    self.text_offset,
                    cv2.FONT_HERSHEY_DUPLEX,
                    self.text_scale,
                    (0, 0, 255),
                    self.text_thickness,
                    cv2.LINE_8,
                )

                cv2.imshow("Comparison", frame)

                displayed_frame = frame_to_display
            if k == ord("p"):
                playing = not playing

    def drawPoints(
        self,
        person: Person,
        currFrame: int,
        frame: np.ndarray,
        color: Tuple[int],
        thickness: int,
    ):
        """Draw indicators at the current and previous and following 10 positions

        Draws a cross on the current position of the pedestrian, additionally draw the previous and following 10 points,
        indicated by a circle. Depending on the color, the cross gets tilted to get a better distinction.

        Examples:
                      currFrame
                          v
        o o o o o o o o o + o o o o o o o
        or
        o o o o o o o o o X o o o o o o o

        :param person: person to draw
        :param currFrame: current frame number
        :param frame: current frame (drawing pane)
        :param color: color used to draw the indicators
        :param thickness: thickness of the indicators
        :return: Nothing
        """
        if person.first_frame <= currFrame <= person.last_frame:
            points = person.points
            for i in range(min(10, currFrame - person.first_frame)):
                if i == 0:
                    continue
                frame = cv2.circle(
                    frame,
                    (
                        int(points[currFrame - person.first_frame - i].x + self.border),
                        int(points[currFrame - person.first_frame - i].y + self.border),
                    ),
                    thickness,
                    color,
                    1,
                )

            for i in range(min(10, person.last_frame - currFrame)):
                if i == 0:
                    continue

                frame = cv2.circle(
                    frame,
                    (
                        int(points[currFrame - person.first_frame + i].x + self.border),
                        int(points[currFrame - person.first_frame + i].y + self.border),
                    ),
                    thickness,
                    color,
                    1,
                )

            marker = cv2.MARKER_CROSS
            if color == (255, 0, 0):
                marker = cv2.MARKER_TILTED_CROSS

            frame = cv2.drawMarker(
                frame,
                (
                    int(points[currFrame - person.first_frame].x + self.border),
                    int(points[currFrame - person.first_frame].y + self.border),
                ),
                color,
                marker,
                thickness=2,
                line_type=cv2.LINE_AA,
            )

    def visualize_people(self, idx_truth: int, idx_test: int):
        tr = self.truth_trajectories[idx_truth - 1]
        te = self.test_trajectories[idx_test - 1]
        start = min(tr.first_frame, te.first_frame)
        end = max(tr.last_frame, te.last_frame)

        def draw(frame: np.ndarray, currFrame: int) -> None:
            self.drawPoints(
                self.test_trajectories[idx_test - 1], currFrame, frame, (255, 0, 0), 7
            )
            self.drawPoints(
                self.truth_trajectories[idx_truth - 1], currFrame, frame, (0, 255, 0), 5
            )

            frame_text_size = cv2.getTextSize(
                "Frame: {:6d}".format(currFrame),
                fontFace=cv2.FONT_HERSHEY_DUPLEX,
                fontScale=self.text_scale,
                thickness=self.text_thickness,
            )

            if te.first_frame <= currFrame < te.last_frame:
                text_pos = (
                    int(self.text_offset[0] + 1.25 * frame_text_size[0][0]),
                    int(self.text_offset[1] + 2 * frame_text_size[0][1]),
                )

                frame = cv2.putText(
                    frame,
                    "Quality: {:7.3f}".format(
                        te.points[currFrame - te.first_frame].qual
                    ),
                    text_pos,
                    cv2.FONT_HERSHEY_DUPLEX,
                    self.text_scale,
                    (255, 0, 0),
                    self.text_thickness,
                    cv2.LINE_8,
                )
            if tr.first_frame <= currFrame < tr.last_frame:
                text_pos = (
                    int(self.text_offset[0] + 1.25 * frame_text_size[0][0]),
                    int(self.text_offset[1]),
                )

                frame = cv2.putText(
                    frame,
                    "Quality: {:7.3f}".format(
                        tr.points[currFrame - te.first_frame].qual
                    ),
                    text_pos,
                    cv2.FONT_HERSHEY_DUPLEX,
                    self.text_scale,
                    (0, 255, 0),
                    self.text_thickness,
                    cv2.LINE_8,
                )

            if (
                tr.first_frame <= currFrame < tr.last_frame
                and te.first_frame <= currFrame < te.last_frame
            ):
                truth = tr.points[currFrame - tr.first_frame]
                test = te.points[currFrame - te.first_frame]
                diff = math.sqrt((truth.x - test.x) ** 2 + (truth.y - test.y) ** 2)

                text_pos = (
                    int(self.text_offset[0]),
                    int(self.text_offset[1] + 2 * frame_text_size[0][1]),
                )
                frame = cv2.putText(
                    frame,
                    "Diff: {:3.3f}".format(diff),
                    text_pos,
                    cv2.FONT_HERSHEY_DUPLEX,
                    self.text_scale,
                    (0, 0, 255),
                    self.text_thickness,
                    cv2.LINE_8,
                )

        self.play_video(start, end, draw)

    def visualize_many(
        self,
        idxs_truth: List[int],
        idxs_test: List[int],
        idx_seed: int,
        is_seed_test: bool,
    ):
        # Idea: Try out taking the first first and last last frame, as in two people?
        if is_seed_test:
            start = self.test_trajectories[idx_seed - 1].first_frame
            end = self.test_trajectories[idx_seed - 1].last_frame
        else:
            start = self.truth_trajectories[idx_seed - 1].first_frame
            end = self.truth_trajectories[idx_seed - 1].last_frame

        def draw(frame: np.ndarray, currFrame: int):
            for id in idxs_test:
                self.drawPoints(
                    self.test_trajectories[id - 1], currFrame, frame, (255, 0, 0), 7
                )
            for id in idxs_truth:
                self.drawPoints(
                    self.truth_trajectories[id - 1], currFrame, frame, (0, 255, 0), 5
                )
            if idx_seed != None:
                if is_seed_test:
                    self.drawPoints(
                        self.test_trajectories[idx_seed - 1],
                        currFrame,
                        frame,
                        (255, 0, 255),
                        3,
                    )
                else:
                    self.drawPoints(
                        self.truth_trajectories[idx_seed - 1],
                        currFrame,
                        frame,
                        (0, 255, 255),
                        3,
                    )

        self.play_video(start, end, draw)

    def visualize_difference(self):
        """Takes output of handmade_test and starts a video
        comparison for each of the listed differences

        :param comp_output: output from handmade_test
        :type comp_output: str
        """
        different_first_frame = re.compile(
            r"Different first frame: truth\((\d+)\) \d+ and test\((\d+)\) \d+"
        )
        different_last_frame = re.compile(
            r"Different last frame: truth\((\d+)\) \d+ and test\((\d+)\) \d+"
        )
        no_counterpart_given = re.compile(
            r"The person (\d+) in the ([^\s]+) file has no counterpart in \w+!"
        )
        point_difference = re.compile(
            r"Diff of \d*\.\d* between test\((\d+)\) and truth\((\d+)\) in frame \d+"
        )
        # Pairing is characterized by truth-countperpart's index
        displayed_pairing: Set[int] = set()
        comp = HandmadeComparison(
            True, True, True, str(self.truth_filename), str(self.test_filename)
        )
        comp_output = comp.run()
        print("To go to the next comparison, press 'n'")
        print("To pause/play press 'p'")
        print(
            "To navigate frame-wise press 'd' (next frame) or 'a' (previous frame). Note: video should be paused."
        )

        for line in comp_output:
            print("\r" + " " * 80, end="", flush=True)
            print("\r", end="", flush=True)
            print(line, end="", flush=True)
            match = different_first_frame.match(line)
            if match:
                idx_truth = int(match.group(1))
                idx_test = int(match.group(2))
                if idx_truth in displayed_pairing:
                    continue
                displayed_pairing.add(idx_truth)

                self.visualize_people(idx_truth, idx_test)
                continue

            match = different_last_frame.match(line)
            if match:
                idx_truth = int(match.group(1))
                idx_test = int(match.group(2))
                if idx_truth in displayed_pairing:
                    continue
                displayed_pairing.add(idx_truth)

                self.visualize_people(idx_truth, idx_test)
                continue

            # NOTE: Implementation satisfying?
            match = no_counterpart_given.match(line)
            if match:
                if match.group(2) == "test":
                    idx_seed = int(match.group(1))
                    near_truth, near_test = self.get_proximal_people(
                        self.test_trajectories[idx_seed - 1]
                    )
                    is_seed_test = True
                else:
                    idx_seed = int(match.group(1))
                    near_truth, near_test = self.get_proximal_people(
                        self.truth_trajectories[idx_seed - 1]
                    )
                    is_seed_test = False
                self.visualize_many(near_truth, near_test, idx_seed, is_seed_test)
                continue

            match = point_difference.match(line)
            if match:
                idx_truth = int(match.group(2))
                idx_test = int(match.group(1))
                if idx_truth in displayed_pairing:
                    continue
                displayed_pairing.add(idx_truth)

                self.visualize_people(idx_truth, idx_test)
                continue


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("PetPath", help="Path to the .pet-file of the test case")
    args = parser.parse_args()
    viz = ComparisonVideoPlayer(args.PetPath)
    viz.visualize_difference()
