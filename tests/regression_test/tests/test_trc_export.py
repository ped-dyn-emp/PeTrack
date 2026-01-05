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


def test_trc_export(pytestconfig):
    petrack_path = pytestconfig.getoption("path")
    project = "../../../demo/03_tracking/06_tracking_with_trajectories.pet"
    trc_file = Path("../../../demo/03_tracking/06_exported_trajectories.trc")

    export_file = trc_file.with_name(trc_file.stem + "_export.trc")

    subprocess.run(
        [
            petrack_path,
            "-project",
            project,
            "-autosavetracker",
            export_file.resolve(),
            "-platform",
            "offscreen",
        ],
        check=True,
    )

    assert export_file.exists(), f"Missing export file: {export_file}"

    # Compare line by line
    with trc_file.open("r") as f1, export_file.open("r") as f2:
        for i, (line1, line2) in enumerate(zip(f1, f2), start=1):
            if line1 != line2:
                raise AssertionError(
                    f"Mismatch in {trc_file.name} vs {export_file.name} "
                    f"at line {i}:\n"
                    f"  truth : {line1.strip()}\n"
                    f"  export: {line2.strip()}"
                )

        # Check if one file has extra lines
        rest1 = f1.readlines()
        rest2 = f2.readlines()
        if rest1 or rest2:
            raise AssertionError(
                f"File length mismatch in {trc_file.name} vs {export_file.name}:\n"
                f"  extra in truth : {len(rest1)} lines\n"
                f"  extra in export: {len(rest2)} lines"
            )
