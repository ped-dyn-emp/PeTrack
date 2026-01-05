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

from pytest import approx
import subprocess
import xml.etree.ElementTree as ET
import numpy as np
import math
from dataclasses import dataclass
from numpy import r_


# NOTE: Does not test all options; ie. QuadAspectRatio, Fix Center, Ext. Model, ...

def extract_parameters(node):
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
    taux = float(node.get("TAUX", "nan"))
    tauy = float(node.get("TAUY", "nan"))
    tx = float(node.get("TX", "nan"))
    ty = float(node.get("TY", "nan"))
    # comparison will fail, if this is NaN
    reprojection_error = float(node.get("ReprError", "nan"))
    if math.isnan(reprojection_error):
        reprojection_error = float("-inf")
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
            taux,
            tauy,
            tx,
            ty,
            reprojection_error,
        ]
    )


def read_intrinsics(tree: ET.ElementTree) -> np.ndarray:
    # NOTE: needs non-nan reprojection error

    oldModelNode = tree.find("./CONTROL/CALIBRATION/INTRINSIC_PARAMETERS/OLD_MODEL")

    extModelNode = tree.find("./CONTROL/CALIBRATION/INTRINSIC_PARAMETERS/EXT_MODEL")

    if oldModelNode is None or extModelNode is None:
        raise RuntimeError("Invalid pet-File! No Calibration node")

    node = tree.find("./CONTROL/CALIBRATION/INTRINSIC_PARAMETERS")
    tang_dist = float(node.get("TANG_DIST", "nan"))
    fix_center = float(node.get("FIX_CENTER", "nan"))
    quad_aspect = float(node.get("QUAD_ASPECT_RATIO", "nan"))

    oldModelParams = extract_parameters(oldModelNode)
    extModelParams = extract_parameters(extModelNode)
    config = np.array([tang_dist, fix_center, quad_aspect])
    return np.concatenate([oldModelParams, extModelParams, config])


@dataclass
class Extrinsic3DParams:
    rot: np.ndarray
    trans: np.ndarray
    swap: np.ndarray
    manual_trans: np.ndarray


def read_3d_extrinsics(tree: ET.ElementTree) -> Extrinsic3DParams:
    node = tree.find("./CONTROL/CALIBRATION/EXTRINSIC_PARAMETERS")
    if node is None:
        raise RuntimeError("Invalid pet-File! No Calibration node")

    coord_dim = int(node.get("COORD_DIMENSION", "nan"))
    if coord_dim != 0:
        raise RuntimeError("Project is 2D project, but only 3D project are compared")

    # axis_len = float(node.get("COORD3D_AXIS_LEN", "nan"))
    swap_x = int(node.get("COORD3D_SWAP_X", "nan"))
    swap_y = int(node.get("COORD3D_SWAP_Y", "nan"))
    swap_z = int(node.get("COORD3D_SWAP_Z", "nan"))
    manual_trans_x = float(node.get("COORD3D_TRANS_X", "nan"))
    manual_trans_y = float(node.get("COORD3D_TRANS_Y", "nan"))
    manual_trans_z = float(node.get("COORD3D_TRANS_Z", "nan"))
    # calib_file = float(node.get("EXTERNAL_CALIB_FILE", "nan"))
    rot1 = float(node.get("EXTR_ROT_1", "nan"))
    rot2 = float(node.get("EXTR_ROT_2", "nan"))
    rot3 = float(node.get("EXTR_ROT_3", "nan"))
    trans1 = float(node.get("EXTR_TRANS_1", "nan"))
    trans2 = float(node.get("EXTR_TRANS_2", "nan"))
    trans3 = float(node.get("EXTR_TRANS_3", "nan"))

    return Extrinsic3DParams(
        r_[rot1, rot2, rot3],
        r_[trans1, trans2, trans3],
        r_[swap_x, swap_y, swap_z],
        r_[manual_trans_x, manual_trans_y, manual_trans_z],
    )


def compare_extrinsic_calib(test_pet: ET.ElementTree, truth_pet: ET.ElementTree):
    test_calib = read_3d_extrinsics(test_pet)
    truth_calib = read_3d_extrinsics(truth_pet)

    assert test_calib.rot == approx(truth_calib.rot, abs=0.1)
    assert test_calib.trans == approx(truth_calib.trans, abs=0.1)
    assert test_calib.manual_trans == approx(truth_calib.manual_trans, abs=0.1)
    assert (test_calib.swap == truth_calib.swap).all()


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


def test_load_old_project_old_model(pytestconfig):
    petrack_path = pytestconfig.getoption("path")
    project = "../data/01_old_intrinsic_oldModel.pet"
    real_intrinsic = "../data/01_old_intrinsic_oldModel_truth.pet"
    output = "../data/calibTest.pet"  # same for other test, cannot be run concurrently

    # run autocalib on demo project
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

    test_pet = ET.parse(output)
    truth_pet = ET.parse(real_intrinsic)

    compare_intrinsic_calib(test_pet, truth_pet)


def test_load_old_project_old_model_no_flag(pytestconfig):
    petrack_path = pytestconfig.getoption("path")
    project = "../data/01_old_intrinsic_oldModel_noFlag.pet"
    real_intrinsic = "../data/01_old_intrinsic_oldModel_noFlag_truth.pet"
    output = "../data/calibTest.pet"  # same for other test, cannot be run concurrently

    # run autocalib on demo project
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

    test_pet = ET.parse(output)
    truth_pet = ET.parse(real_intrinsic)

    compare_intrinsic_calib(test_pet, truth_pet)


def test_load_old_project_ext_model(pytestconfig):
    petrack_path = pytestconfig.getoption("path")
    project = "../data/01_old_intrinsic_extModel.pet"
    real_intrinsic = "../data/01_old_intrinsic_extModel_truth.pet"
    output = "../data/calibTest.pet"  # same for other test, cannot be run concurrently

    # run autocalib on demo project
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

    test_pet = ET.parse(output)
    truth_pet = ET.parse(real_intrinsic)

    compare_intrinsic_calib(test_pet, truth_pet)


def test_load_old_project_ext_model_no_flag(pytestconfig):
    petrack_path = pytestconfig.getoption("path")
    project = "../data/01_old_intrinsic_extModel_noFlag.pet"
    real_intrinsic = "../data/01_old_intrinsic_extModel_noFlag_truth.pet"
    output = "../data/calibTest.pet"  # same for other test, cannot be run concurrently

    # run autocalib on demo project
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

    test_pet = ET.parse(output)
    truth_pet = ET.parse(real_intrinsic)

    compare_intrinsic_calib(test_pet, truth_pet)


def test_extr_calib(pytestconfig):
    petrack_path = pytestconfig.getoption("path")
    project = "../../../demo/01_calibration/01_intrinsic.pet"
    real_extrinsic = "../../../demo/01_calibration/02_extrinsic.pet"
    extrinsic_file = "../../../demo/01_calibration/after.3dc"
    output = (
        "../data/extrCalibTest.pet"  # same for other test, cannot be run concurrently
    )

    # run autocalib on demo project
    subprocess.run(
        [
            petrack_path,
            "-project",
            project,
            "-autoExtrinsic",
            extrinsic_file,
            "-autosave",
            output,
            "-platform",
            "offscreen",
        ],
        check=True,
    )

    test_pet = ET.parse(output)
    truth_pet = ET.parse(real_extrinsic)

    compare_extrinsic_calib(test_pet, truth_pet)
