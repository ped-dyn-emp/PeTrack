/*
 * PeTrack - Software for tracking pedestrians movement in videos
 * Copyright (C) 2026 Forschungszentrum Jülich GmbH, IAS-7
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
#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>


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
            Catch::Approx(0).margin(VEC_MARGIN));

        REQUIRE(
            cv::norm(calib->camToWorldRotation(cv::Vec3d(1, 2, 3)) - cv::Vec3d(1, 2, 3)) ==
            Catch::Approx(0).margin(VEC_MARGIN));
    }

    SECTION("Rotated around z-axis")
    {
        // rotate 90 degrees
        doc.setContent(testConfig.arg("0", "0", QString::number(PI / 2)));
        control->getXml(doc.documentElement(), QString("0.10.0"));
        REQUIRE(
            cv::norm(calib->camToWorldRotation(cv::Vec3d(1, 0, 0)) - cv::Vec3d(0, -1, 0)) ==
            Catch::Approx(0).margin(VEC_MARGIN));

        REQUIRE(
            cv::norm(calib->camToWorldRotation(cv::Vec3d(0, 1, 0)) - cv::Vec3d(1, 0, 0)) ==
            Catch::Approx(0).margin(VEC_MARGIN));

        REQUIRE(
            cv::norm(calib->camToWorldRotation(cv::Vec3d(1, 2, 3)) - cv::Vec3d(2, -1, 3)) ==
            Catch::Approx(0).margin(VEC_MARGIN));

        REQUIRE(
            cv::norm(calib->camToWorldRotation(cv::Vec3d(-5, 5, -2)) - cv::Vec3d(5, 5, -2)) ==
            Catch::Approx(0).margin(VEC_MARGIN));

        // negative rotation
        doc.setContent(testConfig.arg("0", "0", QString::number(-PI)));
        control->getXml(doc.documentElement(), QString("0.10.0"));
        REQUIRE(
            cv::norm(calib->camToWorldRotation(cv::Vec3d(1, 0, 0)) - cv::Vec3d(-1, 0, 0)) ==
            Catch::Approx(0).margin(VEC_MARGIN));

        REQUIRE(
            cv::norm(calib->camToWorldRotation(cv::Vec3d(1, 2, -3)) - cv::Vec3d(-1, -2, -3)) ==
            Catch::Approx(0).margin(VEC_MARGIN));
    }

    SECTION("Wild rotation")
    {
        // vector (1, 1, 1) with length pi/2
        doc.setContent(testConfig.arg("0.9067", "0.9067", "0.9067"));
        control->getXml(doc.documentElement(), QString("0.10.0"));

        REQUIRE(
            cv::norm(calib->camToWorldRotation(cv::Vec3d(1, 1, 1)) - cv::Vec3d(1, 1, 1)) ==
            Catch::Approx(0).margin(VEC_MARGIN));

        REQUIRE(
            cv::norm(calib->camToWorldRotation(cv::Vec3d(1, 0, 0)) - cv::Vec3d(0.33, -0.24, 0.91)) ==
            Catch::Approx(0).margin(VEC_MARGIN));

        REQUIRE(
            cv::norm(calib->camToWorldRotation(cv::Vec3d(1, 2, 3)) - cv::Vec3d(1.42, 3.15, 1.42)) ==
            Catch::Approx(0).margin(VEC_MARGIN));

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
                Catch::Approx(0).margin(VEC_MARGIN));

            REQUIRE(
                cv::norm(calib->camToWorldRotation(cv::Vec3d(1, 2, 3)) - cv::Vec3d(1.42, 3.15, 1.42)) ==
                Catch::Approx(0).margin(VEC_MARGIN));
        }
    }

    SECTION("Another Wild Rotation")
    {
        doc.setContent(testConfig.arg("0.5", "-2", "1.1"));
        control->getXml(doc.documentElement(), QString("0.10.0"));

        REQUIRE(
            cv::norm(calib->camToWorldRotation(cv::Vec3d(1, 1, 1)) - cv::Vec3d(0.2, -0.63, -1.6)) ==
            Catch::Approx(0).margin(VEC_MARGIN));

        REQUIRE(
            cv::norm(calib->camToWorldRotation(cv::Vec3d(1, -2, 3)) - cv::Vec3d(1.69, -3.33, 0.27)) ==
            Catch::Approx(0).margin(VEC_MARGIN));
    }
}

TEST_CASE("src/extrCalibration/loadExtrCalibFile", "[extrCalibration]")
{
    Petrack petrack{"Unknown"};
    auto   *calib = petrack.getExtrCalibration();
    QFile   testExtrCalib("testExtrCalib.txt");
    GIVEN("I have a .3dc file with whitespaces and tabs at the end of a line")
    {
        QString text = QString(
            "-200 100 0 2175.49 877.495  \n"
            "-200 100 200 2374.94 456.997\t\n"
            "0 100 0 2181.07 1564.47\n"
            "0 100 200 2390.02 1591.7\n"
            "200 100 0 2188.53 2256.36\n"
            "200 100 200 2401.54 2738.47\n");

        if(testExtrCalib.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            QTextStream out(&testExtrCalib);
            out << text;
        }
        testExtrCalib.close();
        THEN("The points should be correctly loaded anyways")
        {
            calib->setExtrCalibFile("testExtrCalib.txt");
            calib->loadExtrCalibFile();
            auto points = calib->get3DList();
            CHECK(points.size() == 6);
        }
        std::remove("testExtrCalib.txt");
    }
}
