#
# PeTrack - Software for tracking pedestrians movement in videos
# Copyright (C) 2010-2022 Forschungszentrum Jülich GmbH,
# Maik Boltes, Juliane Adrian, Ricardo Martin Brualla, Arne Graf, Paul Häger, Daniel Hillebrand,
# Deniz Kilic, Paul Lieberenz, Daniel Salden, Tobias Schrödter, Ann Katrin Seemann
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


def pytest_addoption(parser):
    parser.addoption("--path", action="store", default="../../../build/petrack.exe")




# "codeMarker"
@pytest.fixture(params=["markerCasern", "multicolor", "markerJapan", "multiColorMarkerWithAruco", "multiColorMarkerWithAruco_dictMip36h12", "blackdotMarker"], scope='session')
def petrack_on_testdata(request, pytestconfig):
    petrack_path = pytestconfig.getoption("path")

    """Does execute PeTrack on the test data to generate the trc and txt Files"""
    # TODO an letzendliche Projektstruktur anpassen
    test_path = "../data/" + request.param + "_test"
    truth_path = "../data/" + request.param + "_truth"
    project = "../data/" + request.param + ".pet"
    output = test_path
    subprocess.run([petrack_path, "-project", project, "-autotrack", output, "-platform", "offscreen"], check=True)

    yield test_path, truth_path


