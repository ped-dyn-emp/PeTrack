import copy

from pytest import approx
from Person_txt import Person, Point
import pytest

# TODO de-import time
import time


class TestTrackerTxt:
    EPSILON = 15e-3  # Angabe in Meter
    error_message = []
    MAX_DIFF = 99999999

    def parse_txt(self, filename):
        content = []

        with open(filename) as file:
            person_id = -1
            person = 0
            for line in file:

                # Lines starting with # part of the header no data
                if line.startswith('#'):
                    if line.startswith('# id frame x/cm'):
                        self.EPSILON = self.EPSILON * 100
                    continue

                values = line.split(' ')
                if values[0] == person_id:
                    person.add_point(Point(values[1], values[2], values[3], values[4]))
                else:
                    if person_id != -1:
                        content.append(person)
                    person = Person(values[0])
                    person.add_point(Point(values[1], values[2], values[3], values[4]))
                    person_id = values[0]
            content.append(person)  # letzte Person hinzufuegen
        return content

    def compare(self, iteration_list, truth_file, test_file):
        for truth_index, test_index in iteration_list:
            person_truth = truth_file[truth_index]
            person_test = test_file[test_index]
            if person_test != person_truth:
                self.detailed_compare(person_test, person_truth)

    def detailed_compare(self, person_test, person_truth):

        # Making sure point comparison begins at the same Frame
        # Side effect that not all Points are remembered anymore by the person, but that does not matter
        # Since they are only used here in this comparison
        first_truth_frame = int(person_truth.points[0].frame)
        first_test_frame = int(person_test.points[0].frame)

        last_test_frame = int(person_test.points[-1].frame)
        last_truth_frame = int(person_truth.points[-1].frame)

        if last_test_frame != last_truth_frame:
            self.error_message.extend(("Different last frame for ", str(person_test), "(test) and "
                                       , str(person_truth), "(truth)\nLast frame in truth file: ",
                                       str(last_truth_frame),
                                       "\nLast frame in test file: ", str(last_test_frame) + "\n\n"))

        if first_test_frame != first_truth_frame:
            self.error_message.extend(("Different first frame for ", str(person_test), "(test) and ",
                                       str(person_truth), "(truth)\nFirst frame in truth file: ",
                                       str(first_truth_frame),
                                       "\nFirst frame in test file: ", str(first_test_frame), "\n\n"))

            if first_test_frame < first_truth_frame:
                diff = first_truth_frame - first_test_frame
                person_test.points = person_test.points[diff:]
            elif first_test_frame > first_truth_frame:
                diff = first_test_frame - first_truth_frame
                person_truth.points = person_truth.points[diff:]

        for point_truth, point_test in zip(person_truth.points, person_test.points):
            if point_truth.frame != point_test.frame:
                self.error_message.append("Frames are numbered wrong\n\n")

            if point_truth.coordinates != approx(point_test.coordinates, abs=self.EPSILON):
                self.error_message.extend(("Diverging coordinates at ", point_test.__str__(), " in the test file.\n",
                                           "At ", point_truth.__str__(), " in accepted truth.\nFor person ",
                                           str(person_test),
                                           " in test file and person  ", str(person_truth), " in truth file \n\n"))

    def test_e2e_via_txt(self, petrack_on_testdata):
        test_file = self.parse_txt(petrack_on_testdata[0] + ".txt")
        truth_file = self.parse_txt(petrack_on_testdata[1] + ".txt")

        self.error_message = []
        before = time.time()
        truth_file_copy = copy.deepcopy(truth_file)
        test_file_copy = copy.deepcopy(test_file)
        iteration_list = self.get_list_indices(truth_file_copy, test_file_copy)
        iteration_list.sort(key=lambda indices: indices[0])  # sort by truth person id
        after = time.time()
        # print(iteration_list)
        self.compare(iteration_list, truth_file, test_file)

        print("Duration for comparison: " + str(after - before))
        if self.error_message:
            pytest.fail(''.join(self.error_message), False)

    def get_list_indices(self, truth_file, test_file):
        list_for_iteration = []
        '''
        Each true trajectory is mapped to a counterpart in the tst file for which the diff (see calc_diff)
        is less than a threshold
        The easy way would be to just map each true trajectory to the best fitting test trajectory, but the best for
        one true trajectory could be an even better fit for another'''
        threshold = 1e-3
        STEP_WIDTH = 0.1
        cache = {}
        while threshold < self.MAX_DIFF and len(truth_file) != 0:
            for truth_person in reversed(truth_file):
                # See if cached value is still relevant and if yes, just work with that
                if truth_person in cache.keys():
                    match = cache.get(truth_person)
                    if match[0] in test_file:
                        if match[1] < threshold:
                            list_for_iteration.append((int(truth_person.person_id) - 1, int(match[0].person_id) - 1))
                            truth_file.remove(truth_person)
                            test_file.remove(match[0])
                            continue
                        else:
                            continue

                match = -1
                min_diff = -1
                for test_person in test_file:
                    diff = self.calc_diff(truth_person, test_person)
                    if min_diff == -1:
                        min_diff = diff
                        match = test_person
                    elif diff < min_diff and diff < self.MAX_DIFF:
                        min_diff = diff
                        match = test_person
                if match != -1 and min_diff < threshold:
                    list_for_iteration.append((int(truth_person.person_id) - 1, int(match.person_id) - 1))
                    truth_file.remove(truth_person)
                    test_file.remove(match)
                    # print(min_diff)
                elif match == -1:
                    self.error_message += "Person with ID " + str(truth_person.person_id) \
                                          + " in truth file has no counterpart in test file\n\n"
                    truth_file.remove(truth_person)
                else:
                    cache.update({truth_person: (match, min_diff)})
            threshold += STEP_WIDTH

        # Check if someone is in test_file, who wasn't paired wih someone in the truth_file
        test_ids = range(1, len(test_file) + 1)
        for test_id in test_ids:
            match = False
            for id_tuple in list_for_iteration:
                if test_id == id_tuple[1] + 1:
                    match = True
                    break
            if not match:
                self.error_message += "Person with ID " + str(test_id) \
                                      + " in test file doesn't have a counterpart in truth file\n\n"

        return list_for_iteration

    def calc_diff(self, person_truth, person_test):
        first_truth_frame = int(person_truth.points[0].frame)
        first_test_frame = int(person_test.points[0].frame)

        if first_test_frame < first_truth_frame:
            frame_diff = first_truth_frame - first_test_frame
            diff = self.diff_loop(person_truth.points, person_test.points[frame_diff:])
        elif first_test_frame > first_truth_frame:
            frame_diff = first_test_frame - first_truth_frame
            diff = self.diff_loop(person_truth.points[frame_diff:], person_test.points)
        else:
            diff = self.diff_loop(person_truth.points, person_test.points)
        return diff

    def diff_loop(self, points_truth, points_test):
        diff = 0
        number_common_points = 0
        for truth_point, test_point in zip(points_truth, points_test):
            number_common_points += 1
            diff += abs(truth_point.x - test_point.x)
            diff += abs(truth_point.y - test_point.y)
            diff += abs(truth_point.z - test_point.z)
        if number_common_points != 0:
            return diff / number_common_points
        else:
            return self.MAX_DIFF

# tester = TestTrackerTxt()
# tester.test_e2e_via_txt(("markerJapan_test", "markerJapan_truth"))
