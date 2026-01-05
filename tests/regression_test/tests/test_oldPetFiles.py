#
# PeTrack - Software for tracking pedestrians movement in videos
# Copyright (C) 2026 Forschungszentrum Jülich GmbH, IAS-7
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
import pytest
import subprocess
import xml.etree.ElementTree as ET
from test_pet import compare_xml


def pytest_addoption(parser):
    parser.addoption("--path", action="store", default="../../../build/petrack.exe")


@pytest.fixture(
    params=[
        "2006",
        "2008",
        "2009",
        "2012",
        "2013",
        "2014",
        "2015",
        "2016",
        "2017",
        "2018",
        "2019"
    ],
    scope="session",
)
def load_old_pet_files(request, pytestconfig):
    petrack_path = pytestconfig.getoption("path")

    """Does load old pet files and expects them not to crash."""

    project = "../data/old_pet_files/" + request.param + ".pet"
    output = "test_oldPet.pet"
    empty_pet = "../../../demo/00_files/00_empty.pet"
    subprocess.run(
        [
            petrack_path,
            "-project",
            project,
            "-autosave",
            output,
            "-platform",
            "offscreen",
        ],
        check=True,
    )
    yield output, empty_pet


def test_old_pet_files(load_old_pet_files):
    # TODO check if values are correct
    test_pet = ET.parse(load_old_pet_files[0])
    truth_pet = ET.parse(load_old_pet_files[1])

    # comparing with empty pet file should fail when pet file is loaded correctly
    with pytest.raises(AssertionError):
        compare_xml(test_pet, truth_pet)
