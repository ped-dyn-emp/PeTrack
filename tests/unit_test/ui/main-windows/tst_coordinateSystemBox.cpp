/*
 * PeTrack - Software for tracking pedestrians movement in videos
 * Copyright (C) 2024 Forschungszentrum Jülich GmbH, IAS-7
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "autoCalib.h"
#include "calibFilter.h"
#include "coordinateSystemBox.h"
#include "extrinsicBox.h"
#include "imageItem.h"
#include "intrinsicBox.h"
#include "petrack.h"
#include "qsignalspy.h"
#include "ui_coordinateSystemBox.h"

#include <catch2/catch.hpp>

bool operator==(const CoordPose2D &lhs, const CoordPose2D &rhs)
{
    return lhs.angle == rhs.angle && lhs.position == rhs.position && lhs.scale == rhs.scale && lhs.unit == rhs.unit;
}

TEST_CASE("CoordinateSystemBox: display of values in UI")
{
    Petrack             petrack{"Unkown"};
    Autosave            autosave(petrack);
    PersonStorage       storage(petrack, autosave);
    ExtrCalibration     extrCalib(storage);
    ExtrinsicBox        extrBox(nullptr, extrCalib);
    AutoCalib           autoCalib{};
    CalibFilter         calibFilter;
    IntrinsicBox        intrBox{&petrack, autoCalib, calibFilter, extrBox, []() {}};
    ImageItem           imageItem{&petrack, nullptr};
    auto               *ui = new Ui::CoordinateSystemBox();
    CoordinateSystemBox coordSysBox{
        &petrack, ui, []() {}, []() {}, []() { return 0; }, intrBox, extrBox, imageItem, extrCalib};

    SECTION("Propagation Model to UI / 2D calibration")
    {
        CoordPose2D pose2d;
        pose2d.angle    = 254;
        pose2d.position = {548, 533};
        pose2d.scale    = 1337;
        pose2d.unit     = 42;

        coordSysBox.setCoordPose2D(pose2d);

        CHECK(ui->coordRotate->value() == pose2d.angle);
        CHECK(ui->coordTransX->value() == pose2d.position.x());
        CHECK(ui->coordTransY->value() == pose2d.position.y());
        CHECK(ui->coordScale->value() == pose2d.scale);
        CHECK(ui->coordUnit->value() == pose2d.unit);
    }

    SECTION("Propagation Model to UI / 3D calibration")
    {
        CoordPose3D pose3d;
        pose3d.position = Vec3F{123, 456, 432};
        // swap can only be changed from ui; will not be tested here
        pose3d.swap = {false, true, false};

        coordSysBox.setCoordTrans3D(pose3d.position);
        CHECK(ui->coord3DTransX->value() == pose3d.position.x());
        CHECK(ui->coord3DTransY->value() == pose3d.position.y());
        CHECK(ui->coord3DTransZ->value() == pose3d.position.z());

        coordSysBox.setCoord3DAxeLen(42);
        CHECK(ui->coord3DAxeLen->value() == 42);
    }

    SECTION("Propagation UI to Model / 2D calibration")
    {
        CoordPose2D pose2d;
        pose2d.angle    = 254;
        pose2d.position = {548, 533};
        pose2d.scale    = 1337;
        pose2d.unit     = 42;

        ui->coordRotate->setValue(pose2d.angle);
        ui->coordTransX->setValue(pose2d.position.x());
        ui->coordTransY->setValue(pose2d.position.y());
        ui->coordScale->setValue(pose2d.scale);
        ui->coordUnit->setValue(pose2d.unit);

        CHECK(coordSysBox.getCoordPose2D() == pose2d);

        ui->coordAltitude->setValue(852);
        CHECK(coordSysBox.getCameraAltitude() == 852);

        const bool useIntrinsic = !ui->coordUseIntrinsic->isChecked();
        ui->coordUseIntrinsic->setChecked(useIntrinsic);
        CHECK(coordSysBox.isCoordUseIntrinsicChecked() == useIntrinsic);
    }

    SECTION("Propagation UI to Model / 3D calibration")
    {
        Vec3F trans{123, 456, 432};

        ui->coord3DTransX->setValue(trans.x());
        ui->coord3DTransY->setValue(trans.y());
        ui->coord3DTransZ->setValue(trans.z());
        CHECK(coordSysBox.getCoordTrans3D() == trans);

        SwapAxis swap{false, true, false};
        ui->coord3DSwapX->setChecked(swap.x);
        ui->coord3DSwapY->setChecked(swap.y);
        ui->coord3DSwapZ->setChecked(swap.z);
        CHECK(coordSysBox.getSwap3D() == swap);

        ui->coord3DAxeLen->setValue(852);
        CHECK(coordSysBox.getCoord3DAxeLen() == 852);

        const bool showVanish = !ui->extVanishPointsShow->isChecked();
        ui->extVanishPointsShow->setChecked(showVanish);
        CHECK(coordSysBox.getCalibExtrVanishPointsShow() == showVanish);

        const bool showCalibPoints = !ui->extCalibPointsShow->isChecked();
        ui->extCalibPointsShow->setChecked(showCalibPoints);
        CHECK(coordSysBox.getCalibExtrCalibPointsShow() == showCalibPoints);
    }
}

TEST_CASE("CoordinateSystemBox: UI logic")
{
    Petrack             petrack{"Unkown"};
    Autosave            autosave(petrack);
    PersonStorage       storage(petrack, autosave);
    ExtrCalibration     extrCalib(storage);
    ExtrinsicBox        extrBox(nullptr, extrCalib);
    AutoCalib           autoCalib{};
    CalibFilter         calibFilter;
    IntrinsicBox        intrBox{&petrack, autoCalib, calibFilter, extrBox, []() {}};
    ImageItem           imageItem{&petrack, nullptr};
    auto               *ui                     = new Ui::CoordinateSystemBox();
    int                 updateStatusPosCounter = 0;
    int                 updateHeadSizeCounter  = 0;
    CoordinateSystemBox coordSysBox{
        &petrack,
        ui,
        [&]() { updateStatusPosCounter++; },
        [&]() { updateHeadSizeCounter++; },
        []() { return 0; },
        intrBox,
        extrBox,
        imageItem,
        extrCalib};

    SECTION("Selection of 2D/3D")
    {
        constexpr int index3D = 0;
        constexpr int index2D = 1;
        QSignalSpy    extrChangedSpy(&extrBox, &ExtrinsicBox::enabledChanged);
        // NOTE: default is 3D; enabled
        ui->coordTab->setCurrentIndex(index2D);
        CHECK(extrChangedSpy.count() == 1);

        ui->coordTab->setCurrentIndex(index3D);
        CHECK(extrChangedSpy.count() == 2);
    }
}
