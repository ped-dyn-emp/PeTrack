import argparse
import queue
from copy import deepcopy
from typing import List, Tuple
from statistics import mean, median
from math import sqrt
from Person_trc import Person, Point

args = None
if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Compare two trc files and generate an error report."
    )
    parser.add_argument(
        "truth_filename", type=str, action="store", help="The path to the truth trc"
    )
    parser.add_argument(
        "test_filename", type=str, action="store", help="The path to the test trc"
    )
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
    args = parser.parse_args()

EPSILON = (
    1 if args == None else args.eps
)  # in pixels # default value 1, can be changed in CLI
error_message = []
MAX_DIFF = 20  # in pixels
MIN_DIFF_START = 0.01  # Should this be a CLI parameter as well?
DIFF_STEP = 0.01  # Should this be a CLI parameter as well?


def parse_trc(filename: str) -> List[Person]:
    # NOTE assumes Version 4 of trc; Should be fractored into a factory outside of constructor
    content: List[Person] = []
    id = 0

    with open(filename) as file:
        version = int(file.readline()[-2])  # Read header
        file.readline()  # Read header
        file = file.read()
        file = file.split("\n\n")
        file = file[:-1]  # empty line at the end of file

    it = iter(file)
    for person_str in it:
        points_str = next(it)
        person = Person(person_str, points_str, id, version)
        id += 1
        if len(person.points) != person.numTrackedPoints:
            print(f"Error when parsing trc-file {filename}.")
            exit(1)
        content.append(person)

    return content


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
    # Assumes the persons already are cropped to have the same starting frame
    # Zip then crops to the same ending frame
    diff = 0
    num_common_points = len(test) if len(test) < len(truth) else len(truth)
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

        self.output: List[str] = []

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
        first_frame: int
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
            self.point_diffs.append(diff)  # save for overall statistics
            if self.warn_pointwise and diff > EPSILON:
                self.output.append(
                    f"Diff of {diff} between test({test.id+1}) and truth({truth.id+1}) in frame {first_frame+i}"
                )

    def compare_files(
        self, test: List[Person], truth: List[Person], indices: List[Tuple[int, int]]
    ):
        for truth_id, test_id in indices:
            self.compare_person(test[test_id], truth[truth_id])

    # NOTE: Maybe use an established algorithm for bipartite graph matching instead
    def associate_trajectories(
        self, test: List[Person], truth: List[Person]
    ) -> List[Tuple[int, int]]:
        """Finds pairs of the 'same' trajectories in both files

        This function iterated over all trajectories in truth as long as there are trajectories left
        in either truth or test and and tries to find the associated trajectory. For this it compares
        all each truth_trajectory with all left test trajectories, when trimmed to a common start and end
        frame. If the best match has an error lower than the current threshold, the two trajectories
        are a match and the trajectories are not evaluated anymore. If the best match has still a
        difference higher than the current theshold, it is cached.

        Each time every queued truth_trajectory has been tested, the threshold for the maximum allowed
        difference is increased. This ensures that a test trajectory is matches with the best fitting
        truth trajectory, not with the first fitting one.

        :param test: List of Persons in the test trc (is changed; give a deep copy)
        :type test: List[Person]
        :param truth: List of Persons in the truth trc
        :type truth: List[Person]
        :return: List of tuples with the indices of the pairs in their respective list (truth_idx, test_idx)
        :rtype: List[Tuple[int,int]]
        """
        result = []
        truth_queue: queue.SimpleQueue[Person] = queue.SimpleQueue()
        for person in truth:
            truth_queue.put(person)

        min_diff = MIN_DIFF_START
        cache = {}
        queue_size = truth_queue.qsize()
        i = 0
        while (not truth_queue.empty()) and (not (len(test) == 0)):
            # One iteration through the queue/truth candidates?
            i += 1
            if i > queue_size:
                queue_size = truth_queue.qsize()
                min_diff += DIFF_STEP
                i = 1

            truth_person = truth_queue.get()
            best_fit_diff = MAX_DIFF
            done = False

            # First test cached match; most of the time this already is the real one
            old_match = cache.get(truth_person)
            if old_match != None:
                if old_match[1] < min_diff:
                    if old_match[0] in test:
                        result.append((truth_person.id, old_match[0].id))
                        test.remove(old_match[0])
                        done = True
                        continue
                    elif (
                        old_match[1] == -1
                    ):  # Was this trajctory already closest to a deleted/already matched person last time?
                        continue
                    else:
                        cache.update({truth_person: (None, -1)})
                else:
                    truth_queue.put(truth_person)
                    continue

            for test_person in test:
                # Calculate difference with common start frame
                # Zip in compare_persons ends at common end frame
                if (
                    test_person.last_frame < truth_person.first_frame
                    or test_person.first_frame > truth_person.last_frame
                ):
                    continue
                if test_person.first_frame > truth_person.first_frame:
                    frame_diff = test_person.first_frame - truth_person.first_frame
                    diff = calc_diff(
                        test_person.points, truth_person.points[frame_diff:]
                    )
                elif test_person.first_frame < truth_person.first_frame:
                    frame_diff = truth_person.first_frame - test_person.first_frame
                    diff = calc_diff(
                        test_person.points[frame_diff:], truth_person.points
                    )
                else:
                    diff = calc_diff(test_person.points, truth_person.points)

                if diff < min_diff:
                    result.append((truth_person.id, test_person.id))
                    test.remove(test_person)
                    done = True
                    break
                elif diff < best_fit_diff:
                    best_fit_diff = diff
                    cache.update({truth_person: (test_person, diff)})
            if not done:
                if best_fit_diff >= MAX_DIFF:
                    self.output.append(
                        f"The person {truth_person.id + 1} in the truth file has no counterpart in test!"
                    )
                    continue
                truth_queue.put(truth_person)

            # TODO mitschreiben fuer die grobe Statistik
        if self.check_counterpart:
            for person in test:
                self.output.append(
                    f"The person {person.id +1} in the test file has no counterpart in truth!"
                )
            while not truth_queue.empty():
                self.output.append(
                    f"The person {truth_queue.get().id +1} in the truth file has no counterpart in test!"
                )
        return result

    def run(self):
        truth = parse_trc(self.truth_filename)
        test = parse_trc(self.test_filename)
        indices = self.associate_trajectories(deepcopy(test), truth)
        indices = sorted(indices, key=lambda x: x[0])
        self.compare_files(test, truth, indices)

        self.output.append(
            "\nThe mean difference in point coordinates is: "
            + str(mean(self.point_diffs))
        )
        self.output.append(
            "The median difference in point coordinates is: "
            + str(median(self.point_diffs))
        )
        point_diffs = list(filter(lambda x: x != 0, self.point_diffs))
        if len(point_diffs) > 0:
            self.output.append(
                "The mean difference with 0s filtered out is: " + str(mean(point_diffs))
            )
            self.output.append(
                "The median with 0s filtered out is: " + str(median(point_diffs))
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
