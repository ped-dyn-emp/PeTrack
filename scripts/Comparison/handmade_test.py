import argparse
import re
from copy import deepcopy
from typing import List, Tuple
from statistics import mean, median
from math import sqrt
from Person_trc import Person, Point
import matplotlib.pyplot as plt
from scipy.optimize import linear_sum_assignment
import numpy as np

args = None
if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Compare two trc files and generate an error report."
    )
    parser.add_argument("truth_filename", type=str, help="The path to the truth trc")
    parser.add_argument("test_filename", type=str, help="The path to the test trc")
    parser.add_argument(
        "--epsilon",
        type=float,
        dest="eps",
        action="store",
        default=1,
        help="The epsilon (in pixels) of the tolerated difference in pixel values (default=1)",
    )
    parser.add_argument(
        "--point_diff",
        dest="warn_pointwise",
        action="store_true",
        default=False,
        help="Print warning for every point difference bigger than epsilon",
    )
    parser.add_argument(
        "--frames",
        dest="check_frames",
        action="store_true",
        default=False,
        help="Print warning for every trajectory pair having different starting and/or ending frames",
    )
    parser.add_argument(
        "--counterpart",
        "-c",
        dest="check_counterpart",
        action="store_true",
        default=False,
        help="Check if every trajectory has a counterpart in the other file",
    )
    parser.add_argument(
        "--plot",
        action="store_true",
        default=False,
        help="Show trajectory plot of truth vs test data",
    )
    args = parser.parse_args()

EPSILON = args.eps if args is not None else 1
error_message = []
MAX_DIFF = 20
MIN_DIFF_START = 0.01
DIFF_STEP = 0.01


# Version 1-3: 9 fields (no markerID)
# Version 4+:  10 fields (with markerID)
HEADER_RE = re.compile(
    r"^"
    r"(\d+)"  # Person number (nr)
    r"\s+"
    r"(-?\d+(?:\.\d+)?)"  # Height (can be negative)
    r"\s+"
    r"(\d+)"  # First frame
    r"\s+"
    r"(\d+)"  # Last frame
    r"\s+"
    r"(\d+)"  # Color count (colCount)
    r"\s+"
    r"(-?\d+)"  # Color R
    r"\s+"
    r"(-?\d+)"  # Color G
    r"\s+"
    r"(-?\d+)"  # Color B
    r"(?:\s+(-?\d+))?"  # Marker ID (optional, version 4+)
    r"\s+"
    r"(\d+)"  # Size (number of trackpoints)
    r"$",
    re.MULTILINE,
)

# Version 1:   8 fields  (x y qual colorX colorY r g b)
# Version 2:   11 fields (+ stereoX stereoY stereoZ)
# Version 3+:  12 fields (+ markerID)
POINT_RE = re.compile(
    r"^"
    r"(-?\d+(?:\.\d+)?)"  # x coordinate
    r"\s+"
    r"(-?\d+(?:\.\d+)?)"  # y coordinate
    r"\s+"
    r"(?:(-?\d+(?:\.\d+)?)\s+)?"  # stereoPoint.x (optional, v2+)
    r"(?:(-?\d+(?:\.\d+)?)\s+)?"  # stereoPoint.y (optional, v2+)
    r"(?:(-?\d+(?:\.\d+)?)\s+)?"  # stereoPoint.z (optional, v2+)
    r"(-?\d+(?:\.\d+)?)"  # Quality value
    r"\s+"
    r"(-?\d+(?:\.\d+)?)"  # colorPoint.x
    r"\s+"
    r"(-?\d+(?:\.\d+)?)"  # colorPoint.y
    r"\s+"
    r"(-?\d+)"  # Color R
    r"\s+"
    r"(-?\d+)"  # Color G
    r"\s+"
    r"(-?\d+)"  # Color B
    r"(?:\s+(-?\d+))?"  # Marker ID (optional, v3+)
    r"$",
    re.MULTILINE,
)


def parse_trc(filename: str) -> List[Person]:
    persons = []
    id = 0

    with open(filename) as f:
        version = int(f.readline()[-2])
        f.readline()
        lines = [l.rstrip() for l in f.readlines()]

    cleaned = []
    for line in lines:
        # skip garbage lines like "2"
        if line.strip() == "":
            cleaned.append("")
            continue

        if HEADER_RE.match(line):
            cleaned.append(line)
            continue

        if POINT_RE.match(line):
            cleaned.append(line)
            continue

        cleaned.append("")

    blocks = "\n".join(cleaned).split("\n\n")

    for i in range(0, len(blocks) - 1, 2):
        header = blocks[i].strip()
        points = blocks[i + 1].strip()

        if header == "" or points == "":
            continue

        person = Person(header, points, id, version)
        id += 1
        if len(person.points) != person.numTrackedPoints:
            print(f"Error when parsing trc-file {filename}.")
            exit(1)

        persons.append(person)

    return persons


def calc_diff(test: List[Point], truth: List[Point]) -> float:
    """Calculates Median Euclidean Distance

    This function assumes the persons are already cropped to the same starting
    frame!!!

    :param test: List of points from person from test trc
    :type test: List[Point]
    :param truth: List of Points from person from truth trc
    :type truth: List[Point]
    :return: sum of point distances in common frames
    :rtype: float
    """

    diff = 0
    num_common_points = min(len(test), len(truth))
    for test_point, truth_point in zip(test, truth):
        diff += sqrt(
            (test_point.x - truth_point.x) ** 2 + (test_point.y - truth_point.y) ** 2
        )
    return diff / num_common_points


class HandmadeComparison:
    def __init__(
        self,
        check_frames: bool,
        warn_pointwise: bool,
        check_counterpart: bool,
        truth_filename: str,
        test_filename: str,
    ):
        self.point_diffs: List[float] = []
        self.check_frames = check_frames
        self.warn_pointwise = warn_pointwise
        self.check_counterpart = check_counterpart
        self.truth_filename = truth_filename
        self.test_filename = test_filename
        self.truth_point_count = 0
        self.output: List[str] = []
        self.indices: List[Tuple[int, int]] = (
            []
        )  # which truth person index corresponds to which test person index
        self.truth_persons: List[Person] = []
        self.test_persons: List[Person] = []

    def compare_person(self, test: Person, truth: Person):
        """Crops persons to common frames and calcs the diff

        This function crops the persons to the common frames. Then it adds
        the absolute differences and optionally outputs those with a diff.
        over a (user-defined) epsilon. It also optionally outputs different
        starting and ending frames.

        :param test: Person from test trc
        :type test: Person
        :param truth: Person from truth trc
        :type truth: Person
        """

        if test.first_frame > truth.first_frame:
            first_frame = test.first_frame
            frame_diff = test.first_frame - truth.first_frame
            test_points = test.points
            truth_points = truth.points[frame_diff:]
        elif test.first_frame < truth.first_frame:
            first_frame = truth.first_frame
            frame_diff = truth.first_frame - test.first_frame
            test_points = test.points[frame_diff:]
            truth_points = truth.points
        else:
            first_frame = test.first_frame
            test_points = test.points
            truth_points = truth.points

        if self.check_frames and test.first_frame != truth.first_frame:
            self.output.append(
                f"Different first frame: truth({truth.id+1}) {truth.first_frame} and test({test.id+1}) {test.first_frame}"
            )
        if self.check_frames and test.last_frame != truth.last_frame:
            self.output.append(
                f"Different last frame: truth({truth.id+1}) {truth.last_frame} and test({test.id+1}) {test.last_frame}"
            )

        for i, (test_point, truth_point) in enumerate(zip(test_points, truth_points)):
            diff = sqrt(
                (test_point.x - truth_point.x) ** 2
                + (test_point.y - truth_point.y) ** 2
            )
            self.point_diffs.append(diff)
            if self.warn_pointwise and diff > EPSILON:
                self.output.append(
                    f"Diff of {diff}px between test({test.id+1}) and truth({truth.id+1}) in frame {first_frame+i}"
                )

    def compare_files(self):
        for truth_id, test_id in self.indices:
            self.compare_person(
                self.test_persons[test_id], self.truth_persons[truth_id]
            )

    def associate_trajectories(self):
        """Finds the 'best' (continous) pairs of the 'same' trajectories in both files using bipartite graph matching (see linear_sum_assignment from scipy)"""
        n_truth = len(self.truth_persons)
        n_test = len(self.test_persons)

        cost = np.full((n_truth, n_test), 1e6, dtype=float)

        for i, t_person in enumerate(self.truth_persons):
            for j, s_person in enumerate(self.test_persons):

                if s_person.last_frame < t_person.first_frame:
                    continue
                if s_person.first_frame > t_person.last_frame:
                    continue

                if s_person.first_frame > t_person.first_frame:
                    frame_diff = s_person.first_frame - t_person.first_frame
                    diff = calc_diff(s_person.points, t_person.points[frame_diff:])
                elif s_person.first_frame < t_person.first_frame:
                    frame_diff = t_person.first_frame - s_person.first_frame
                    diff = calc_diff(s_person.points[frame_diff:], t_person.points)
                else:
                    diff = calc_diff(s_person.points, t_person.points)

                cost[i, j] = diff

        row_idx, col_idx = linear_sum_assignment(cost)

        result = []
        for r, c in zip(row_idx, col_idx):
            if cost[r, c] < 1e6:
                result.append((self.truth_persons[r].id, self.test_persons[c].id))

        if self.check_counterpart:
            matched_truth = {a for a, _ in result}
            matched_test = {b for _, b in result}

            for i, p in enumerate(self.truth_persons):
                if p.id not in matched_truth:
                    self.output.append(
                        f"The person {p.id + 1} in the truth file has no counterpart in test!"
                    )

            for j, p in enumerate(self.test_persons):
                if p.id not in matched_test:
                    self.output.append(
                        f"The person {p.id + 1} in the test file has no counterpart in truth!"
                    )

        return result

    def visualize(self):

        fig, ax = plt.subplots()
        plt.title("Press → or ← to navigate between persons, 'd' for drift view")
        plt.rcParams.update(
            {"font.size": 16, "savefig.bbox": "tight", "savefig.pad_inches": 0.05}
        )

        idx = 0
        drift_mode = False

        def compute_drift(truth_person, test_person):
            gt_start = truth_person.first_frame
            gt_end = truth_person.last_frame

            test_start = test_person.first_frame
            test_end = test_person.last_frame
            frames = list(range(gt_start, gt_end + 1))
            errors = []
            for f in frames:
                if test_start <= f <= test_end:
                    tr = truth_person.points[f - gt_start]
                    tp = test_person.points[f - test_start]
                    err = sqrt((tp.x - tr.x) ** 2 + (tp.y - tr.y) ** 2)
                    errors.append(err)
                else:
                    errors.append(None)
            return frames, errors

        def draw_person(i):
            ax.clear()
            truth_person = self.truth_persons[self.indices[i][0]]
            test_person = self.test_persons[self.indices[i][1]]

            if not drift_mode:
                # Trajectory view
                truth_x = [p.x for p in truth_person.points]
                truth_y = [p.y for p in truth_person.points]
                test_x = [p.x for p in test_person.points]
                test_y = [p.y for p in test_person.points]

                ax.plot(truth_x, truth_y, "g-", label="Truth")
                ax.plot(test_x, test_y, "r--", label="Test")
                ax.legend()
                ax.set_xlabel("X")
                ax.set_ylabel("Y")
                ax.set_title(f"Person {i+1}/{len(self.indices)} — Trajectory View")

                x_min, x_max = min(truth_x), max(truth_x)
                y_min, y_max = min(truth_y), max(truth_y)

                x_margin = (x_max - x_min) * 0.05
                y_margin = 500  # account for label

                ax.set_xlim(x_min - x_margin, x_max + x_margin)
                ax.set_ylim(y_min - y_margin, y_max + y_margin)
                ax.set_aspect("equal", adjustable="box")
            else:
                # Drift (error over time)
                frames, errors = compute_drift(truth_person, test_person)
                ax.set_ylim(-20, 300)

                person_first_frame = truth_person.first_frame
                person_last_frame = truth_person.last_frame
                ax.set_xlim(person_first_frame, person_last_frame)
                ax.set_aspect("auto")
                ax.autoscale(False)
                ax.plot(frames, errors, "b-", linewidth=1.5)
                ax.set_xlabel("Frame")
                ax.set_ylabel("Local Error (px)")
                ax.set_title(f"Person {i+1}/{len(self.indices)} — Drift")

            ax.grid(True)
            fig.canvas.draw_idle()

        def on_key(event):
            nonlocal idx, drift_mode
            if event.key == "right":
                idx = (idx + 1) % len(self.indices)
                draw_person(idx)
            elif event.key == "left":
                idx = (idx - 1) % len(self.indices)
                draw_person(idx)
            elif event.key == "d":
                drift_mode = not drift_mode
                draw_person(idx)
            elif event.key == "s":
                fig.savefig("output.png")

        fig.canvas.mpl_connect("key_press_event", on_key)
        draw_person(idx)
        plt.show()

    def run(self):
        self.output.clear()
        self.truth_persons = parse_trc(self.truth_filename)
        self.truth_point_count = sum(
            len(person.points) for person in self.truth_persons
        )
        self.test_persons = parse_trc(self.test_filename)
        indices = self.associate_trajectories()
        self.indices = sorted(indices, key=lambda x: x[0])
        self.compare_files()

        self.output.append(
            "\nThe mean difference in point coordinates is: "
            + str(mean(self.point_diffs))
        )
        self.output.append(
            "The median difference in point coordinates is: "
            + str(median(self.point_diffs))
        )
        self.output.append(
            "The max difference in point coordinates is: " + str(max(self.point_diffs))
        )
        self.output.append(
            f"Total matched points: {len(self.point_diffs)} / {self.truth_point_count}"
        )

        self.output.append(
            f"num_persons_test / num_persons_truth: {len(self.test_persons)} / {len(self.truth_persons)}"
        )

        point_diffs = [x for x in self.point_diffs if x != 0]
        if len(point_diffs) > 0:
            self.output.append(
                "The mean difference (nonzero only): " + str(mean(point_diffs))
            )
            self.output.append(
                "The median difference (nonzero only): " + str(median(point_diffs))
            )

        return self.output


if __name__ == "__main__":
    comp = HandmadeComparison(
        args.check_frames,
        args.warn_pointwise,
        args.check_counterpart,
        args.truth_filename,
        args.test_filename,
    )
    out = comp.run()
    print("\n".join(out))

    # Visualization (if requested)
    if args.plot:
        comp.visualize()
