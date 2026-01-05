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

import subprocess
from pathlib import Path


def test_export_nonempty(pytestconfig):
    # had problems with empty exports; this is a regression test for the fix
    petrack_path = pytestconfig.getoption("path")
    ground_truth = "../data/markerJapan_truth.txt"
    output = "../data/trc_loading_file.txt"
    project = "../data/load_trc_file.pet"

    # open and save (only saving would write last opened video as SRC)
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

    assert len(Path(output).read_text().splitlines()) == len(
        Path(ground_truth).read_text().splitlines()
    )
