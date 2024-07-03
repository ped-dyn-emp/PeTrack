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
#include "control.h"
#include "extrCalibration.h"
#include "petrack.h"

#include <QDomDocument>
#include <catch2/catch.hpp>


// use margin for absolute difference, as epsilon would be relative which is useless when comparing to 0
constexpr float VEC_MARGIN = 0.01f;

TEST_CASE("src/extrCalibration/camToWorldRotation", "[extrCalibration]")
{
    Petrack  petrack{"Unknown"};
    auto    *calib   = petrack.getExtrCalibration();
    Control *control = petrack.getControlWidget();

    const QString testConfig{
        R"(<CONTROL>
                <CALIBRATION>
                    <EXTRINSIC_PARAMETERS COORD3D_SWAP_Y="0" COORD3D_SWAP_Z="0" EXTR_ROT_1="%1" EXTR_ROT_2="%2" EXTR_ROT_3="%3" EXTR_TRANS_1="0" EXTR_TRANS_2="0" EXTR_TRANS_3="0" />
                </CALIBRATION>
            </CONTROL>)"};

    QDomDocument doc;
    doc.setContent(testConfig.arg("0", "0", "0"));
    control->getXml(doc.documentElement(), QString("0.10.0"));

    SECTION("Identity Coordinate System")
    {
        REQUIRE(
            cv::norm(calib->camToWorldRotation(cv::Vec3d(1, 0, 0)) - cv::Vec3d(1, 0, 0)) ==
            Approx(0).margin(VEC_MARGIN));

        REQUIRE(
            cv::norm(calib->camToWorldRotation(cv::Vec3d(1, 2, 3)) - cv::Vec3d(1, 2, 3)) ==
            Approx(0).margin(VEC_MARGIN));
    }

    SECTION("Rotated around z-axis")
    {
        // rotate 90 degrees
        doc.setContent(testConfig.arg("0", "0", QString::number(PI / 2)));
        control->getXml(doc.documentElement(), QString("0.10.0"));
        REQUIRE(
            cv::norm(calib->camToWorldRotation(cv::Vec3d(1, 0, 0)) - cv::Vec3d(0, -1, 0)) ==
            Approx(0).margin(VEC_MARGIN));

        REQUIRE(
            cv::norm(calib->camToWorldRotation(cv::Vec3d(0, 1, 0)) - cv::Vec3d(1, 0, 0)) ==
            Approx(0).margin(VEC_MARGIN));

        REQUIRE(
            cv::norm(calib->camToWorldRotation(cv::Vec3d(1, 2, 3)) - cv::Vec3d(2, -1, 3)) ==
            Approx(0).margin(VEC_MARGIN));

        REQUIRE(
            cv::norm(calib->camToWorldRotation(cv::Vec3d(-5, 5, -2)) - cv::Vec3d(5, 5, -2)) ==
            Approx(0).margin(VEC_MARGIN));

        // negative rotation
        doc.setContent(testConfig.arg("0", "0", QString::number(-PI)));
        control->getXml(doc.documentElement(), QString("0.10.0"));
        REQUIRE(
            cv::norm(calib->camToWorldRotation(cv::Vec3d(1, 0, 0)) - cv::Vec3d(-1, 0, 0)) ==
            Approx(0).margin(VEC_MARGIN));

        REQUIRE(
            cv::norm(calib->camToWorldRotation(cv::Vec3d(1, 2, -3)) - cv::Vec3d(-1, -2, -3)) ==
            Approx(0).margin(VEC_MARGIN));
    }

    SECTION("Wild rotation")
    {
        // vector (1, 1, 1) with length pi/2
        doc.setContent(testConfig.arg("0.9067", "0.9067", "0.9067"));
        control->getXml(doc.documentElement(), QString("0.10.0"));

        REQUIRE(
            cv::norm(calib->camToWorldRotation(cv::Vec3d(1, 1, 1)) - cv::Vec3d(1, 1, 1)) ==
            Approx(0).margin(VEC_MARGIN));

        REQUIRE(
            cv::norm(calib->camToWorldRotation(cv::Vec3d(1, 0, 0)) - cv::Vec3d(0.33, -0.24, 0.91)) ==
            Approx(0).margin(VEC_MARGIN));

        REQUIRE(
            cv::norm(calib->camToWorldRotation(cv::Vec3d(1, 2, 3)) - cv::Vec3d(1.42, 3.15, 1.42)) ==
            Approx(0).margin(VEC_MARGIN));

        SECTION("Translation should not matter")
        {
            const QString testConfig{
                R"(<CONTROL>
                        <CALIBRATION>
                            <EXTRINSIC_PARAMETERS EXTR_ROT_1="%1" EXTR_ROT_2="%2" EXTR_ROT_3="%3" EXTR_TRANS_1="10" EXTR_TRANS_2="-20" EXTR_TRANS_3="-500" />
                        </CALIBRATION>
                    </CONTROL>)"};
            doc.setContent(testConfig.arg("0.9067", "0.9067", "0.9067"));
            control->getXml(doc.documentElement(), QString("0.10.0"));
            REQUIRE(
                cv::norm(calib->camToWorldRotation(cv::Vec3d(1, 0, 0)) - cv::Vec3d(0.33, -0.24, 0.91)) ==
                Approx(0).margin(VEC_MARGIN));

            REQUIRE(
                cv::norm(calib->camToWorldRotation(cv::Vec3d(1, 2, 3)) - cv::Vec3d(1.42, 3.15, 1.42)) ==
                Approx(0).margin(VEC_MARGIN));
        }
    }

    SECTION("Another Wild Rotation")
    {
        doc.setContent(testConfig.arg("0.5", "-2", "1.1"));
        control->getXml(doc.documentElement(), QString("0.10.0"));

        REQUIRE(
            cv::norm(calib->camToWorldRotation(cv::Vec3d(1, 1, 1)) - cv::Vec3d(0.2, -0.63, -1.6)) ==
            Approx(0).margin(VEC_MARGIN));

        REQUIRE(
            cv::norm(calib->camToWorldRotation(cv::Vec3d(1, -2, 3)) - cv::Vec3d(1.69, -3.33, 0.27)) ==
            Approx(0).margin(VEC_MARGIN));
    }
}
