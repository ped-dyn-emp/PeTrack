#
# PeTrack - Software for tracking pedestrians movement in videos
# Copyright (C) 2023 Forschungszentrum Jülich GmbH, IAS-7
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

import subprocess
from pytest import approx
import xml.etree.ElementTree as ET


def compare_xml(test: ET.ElementTree, truth: ET.ElementTree):
    for test_elem, truth_elem in zip(test.iter(), truth.iter()):
        for test_attrib, truth_attrib in zip(
            test_elem.attrib.items(), truth_elem.attrib.items()
        ):
            assert test_attrib[0] == truth_attrib[0]

            if test_attrib[0] == "SOURCE_FRAME_IN":
                # in 00_empty.pet = -1; but -1 gets read as 0, i.e. beginning of video
                # need extra comparison for this
                assert int(test_attrib[1]) == 0 and int(truth_attrib[1]) == -1, f"Failed comparison for SOURCE_FRAME_IN"
                continue

            try:
                # treat all numeric values as float for comparison
                test_value = float(test_attrib[1])
                truth_value = float(truth_attrib[1])
                assert test_value == approx(
                    truth_value, abs=0.0001
                ), f"Failed comparion on {test_elem.tag}/{test_attrib[0]}"
            except:
                assert (
                    test_attrib[1] == truth_attrib[1]
                ), f"Failed comparion on {test_elem.tag}/{test_attrib[0]}"


def test_defaultPet(pytestconfig):
    petrack_path = pytestconfig.getoption("path")
    ground_truth = "../../../demo/00_files/00_empty.pet"
    output = "../data/defaultPetTest.pet"

    # open and save (only saving would write last opened video as SRC)
    subprocess.run(
        [
            petrack_path,
            "-project",
            ground_truth,
            "-autosave",
            output,
            "-platform",
            "offscreen",
        ],
        check=True,
    )

    test_pet = ET.parse(output)
    truth_pet = ET.parse(ground_truth)
    compare_xml(test_pet, truth_pet)
