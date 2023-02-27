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

from pytest import approx
import subprocess
import xml.etree.ElementTree as ET
import numpy as np

# NOTE: Does not test all options; ie. QuadAspectRatio, Fix Center, Ext. Model, ...


def read_intrinsics(tree: ET.ElementTree) -> np.ndarray:
    # NOTE: needs non-nan reprojection error

    node = tree.find("./CONTROL/CALIBRATION/INTRINSIC_PARAMETERS")
    if node is None:
        raise RuntimeError("Invalid pet-File! No Calibration node")

    cx = float(node.get("CX", "nan"))
    cy = float(node.get("CY", "nan"))
    fx = float(node.get("FX", "nan"))
    fy = float(node.get("FY", "nan"))
    k4 = float(node.get("K4", "nan"))
    k5 = float(node.get("K5", "nan"))
    k6 = float(node.get("K6", "nan"))
    r2 = float(node.get("R2", "nan"))
    r4 = float(node.get("R4", "nan"))
    r6 = float(node.get("R6", "nan"))
    s1 = float(node.get("S1", "nan"))
    s2 = float(node.get("S2", "nan"))
    s3 = float(node.get("S3", "nan"))
    s4 = float(node.get("S4", "nan"))
    tang_dist = float(node.get("TANG_DIST", "nan"))
    taux = float(node.get("TAUX", "nan"))
    tauy = float(node.get("TAUY", "nan"))
    tx = float(node.get("TX", "nan"))
    ty = float(node.get("TY", "nan"))

    # comparison will fail, if this is NaN
    reprojection_error = float(node.get("ReprError", "nan"))
    return np.array(
        [
            cx,
            cy,
            fx,
            fy,
            k4,
            k5,
            k6,
            r2,
            r4,
            r6,
            s1,
            s2,
            s3,
            s4,
            tang_dist,
            taux,
            tauy,
            tx,
            ty,
            reprojection_error,
        ]
    )


def compare_intrinsic_calib(test_pet: ET.ElementTree, truth_pet: ET.ElementTree):
    test_calib_params = read_intrinsics(test_pet)
    truth_calib_params = read_intrinsics(truth_pet)

    # high margin, but calibration itself should be tested by OpenCV itself
    # this is just to see that we did calibrate
    assert test_calib_params == approx(truth_calib_params, abs=0.1)


def test_autoCalib_default_options(pytestconfig):
    petrack_path = pytestconfig.getoption("path")
    project = "../../../demo/00_files/00_empty.pet"
    real_intrinsic = "../../../demo/01_calibration/01_intrinsic.pet"
    intrinsic_dir = "../../../demo/00_files/calibration/intrinsic"
    output = "../data/calibTest.pet"

    # run autocalib on demo project
    subprocess.run(
        [
            petrack_path,
            "-project",
            project,
            "-autoIntrinsic",
            intrinsic_dir,
            "-autosave",
            output,
            "-platform",
            "offscreen",
        ],
        check=True,
    )

    test_pet = ET.parse(output)
    truth_pet = ET.parse(real_intrinsic)

    compare_intrinsic_calib(test_pet, truth_pet)


def test_autoCalib_old_model(pytestconfig):
    petrack_path = pytestconfig.getoption("path")
    project = "../data/00_empty.pet"
    real_intrinsic = "../data/01_intrinsic.pet"
    intrinsic_dir = "../../../demo/00_files/calibration/intrinsic"
    output = "../data/calibTest.pet" # same for other test, cannot be run concurrently

    # run autocalib on demo project
    subprocess.run(
        [
            petrack_path,
            "-project",
            project,
            "-autoIntrinsic",
            intrinsic_dir,
            "-autosave",
            output,
            "-platform",
            "offscreen",
        ],
        check=True,
    )

    test_pet = ET.parse(output)
    truth_pet = ET.parse(real_intrinsic)

    compare_intrinsic_calib(test_pet, truth_pet)
