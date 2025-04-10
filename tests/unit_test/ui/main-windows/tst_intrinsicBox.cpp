/*
 * PeTrack - Software for tracking pedestrians movement in videos
 * Copyright (C) 2025 Forschungszentrum Jülich GmbH, IAS-7
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
#include "extrCalibration.h"
#include "extrinsicBox.h"
#include "intrinsicBox.h"
#include "personStorage.h"
#include "petrack.h"
#include "ui_intrinsicBox.h"
#include "util.h"

#include <QDomElement>
#include <QSignalSpy>
#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/trompeloeil.hpp>


bool matEq(const cv::Mat &lhs, const cv::Mat &rhs)
{
    return cv::countNonZero(lhs != rhs) == 0;
}

constexpr double SERIALIZATION_MARGIN = 0.01;

TEST_CASE("IntrinsicCameraParams have value semantics")
{
    IntrinsicCameraParams testParams;
    testParams.setFx(1);
    testParams.setFy(1);
    testParams.setCx(0);
    testParams.setCy(0);
    CHECK(matEq(testParams.cameraMatrix, cv::Mat::eye(cv::Size(3, 3), CV_64F)));
    CHECK(matEq(testParams.distortionCoeffs, cv::Mat::zeros(1, 14, CV_32F)));
    CHECK(qIsNaN(testParams.reprojectionError));
    testParams.setFx(1000);
    testParams.setR2(2000);

    SECTION("Copy Constructor")
    {
        IntrinsicCameraParams copy{testParams};
        copy.setFy(-1);
        copy.setR4(-1);
        CHECK(copy.getFx() == 1000);
        CHECK(copy.getR2() == 2000);
        CHECK(testParams.getFy() == 1);
        CHECK(testParams.getR4() == 0);
    }

    SECTION("Copy Assignment")
    {
        IntrinsicCameraParams copy;
        copy = testParams;
        copy.setFy(-1);
        copy.setR4(-1);
        CHECK(copy.getFx() == 1000);
        CHECK(copy.getR2() == 2000);
        CHECK(testParams.getFy() == 1);
        CHECK(testParams.getR4() == 0);
    }
}

TEST_CASE("IntrinsicCameraParams values")
{
    AutoCalib         autoCalib;
    CalibFilter       filterCalib;
    Petrack           petrack{"Unkown"};
    Autosave          autosave(petrack);
    PersonStorage     storage(petrack, autosave);
    ExtrCalibration   calib(storage);
    ExtrinsicBox      extrBox(nullptr, calib);
    Ui::IntrinsicBox *ui      = new Ui::IntrinsicBox(); // ownership transferred to IntrinsicBox
    auto              intrBox = IntrinsicBox(nullptr, ui, autoCalib, filterCalib, extrBox, []() {});

    QSignalSpy spy{&intrBox, &IntrinsicBox::paramsChanged};
    CHECK(spy.isValid());
    IntrinsicModelsParameters params;
    WHEN("I set the values of the IntrinsicParameters (via the setter)")
    {
        params.oldModelParams.cameraMatrix = (cv::Mat_<double>(3, 3) << 800, 2, 3, 4, 900, 6, 7, 8, 9);
        // values need to be in valid range -5 to 5
        params.oldModelParams.distortionCoeffs =
            (cv::Mat_<float>(1, 14) << -4.5, -4, -3.5, -3, -2.5, -2, -1.5, -1, 1, 1.5, 2, 2.5, 3, 3.5);

        params.oldModelParams.reprojectionError = 42;

        params.extModelParams.cameraMatrix = (cv::Mat_<double>(3, 3) << 600, 2, 3, 4, 800, 6, 7, 8, 9);
        // values need to be in valid range -5 to 5
        params.extModelParams.distortionCoeffs =
            (cv::Mat_<float>(1, 14) << 4.5, 4, 3.5, 3, 2.5, 2, 1.5, 1, -1, -1.5, -2, -2.5, -3, -3.5);
        params.extModelParams.reprojectionError = 187;

        intrBox.setIntrinsicCameraParams(params);
        THEN("The params are changed in the params-struct")
        {
            auto newParams = intrBox.getBothIntrinsicCameraParams();
            CHECK(newParams.oldModelParams == params.oldModelParams);
            CHECK(newParams.extModelParams == params.extModelParams);
        }

        THEN("The extended model params are shown in the ui")
        {
            CHECK(ui->fx->value() == Catch::Approx(params.extModelParams.getFx()));
            CHECK(ui->fy->value() == Catch::Approx(params.extModelParams.getFy()));
            CHECK(ui->cx->value() == Catch::Approx(params.extModelParams.getCx()));
            CHECK(ui->cy->value() == Catch::Approx(params.extModelParams.getCy()));

            CHECK(ui->r2->value() == Catch::Approx(static_cast<double>(params.extModelParams.getR2())));
            CHECK(ui->r4->value() == Catch::Approx(static_cast<double>(params.extModelParams.getR4())));
            CHECK(ui->tx->value() == Catch::Approx(static_cast<double>(params.extModelParams.getTx())));
            CHECK(ui->ty->value() == Catch::Approx(static_cast<double>(params.extModelParams.getTy())));
            CHECK(ui->r6->value() == Catch::Approx(static_cast<double>(params.extModelParams.getR6())));
            CHECK(ui->k4->value() == Catch::Approx(static_cast<double>(params.extModelParams.getK4())));
            CHECK(ui->k5->value() == Catch::Approx(static_cast<double>(params.extModelParams.getK5())));
            CHECK(ui->k6->value() == Catch::Approx(static_cast<double>(params.extModelParams.getK6())));
            CHECK(ui->s1->value() == Catch::Approx(static_cast<double>(params.extModelParams.getS1())));
            CHECK(ui->s2->value() == Catch::Approx(static_cast<double>(params.extModelParams.getS2())));
            CHECK(ui->s3->value() == Catch::Approx(static_cast<double>(params.extModelParams.getS3())));
            CHECK(ui->s4->value() == Catch::Approx(static_cast<double>(params.extModelParams.getS4())));
            CHECK(ui->taux->value() == Catch::Approx(static_cast<double>(params.extModelParams.getTaux())));
            CHECK(ui->tauy->value() == Catch::Approx(static_cast<double>(params.extModelParams.getTauy())));
            CHECK(
                ui->intrError->text().toDouble() ==
                Catch::Approx(static_cast<double>(params.extModelParams.reprojectionError)));
        }
        GIVEN("Some parameters are out of range")
        {
            params.extModelParams.setS4(156468);
            THEN("An exception is thrown")
            {
                CHECK_THROWS_AS(intrBox.setIntrinsicCameraParams(params), std::domain_error);
            }
        }

        THEN("The 'paramsChanged'-signal is fired exactly once")
        {
            CHECK(spy.count() == 1);
        }

        AND_WHEN("I press the extended model checkbox")
        {
            ui->extModelCheckBox->setChecked(false);

            THEN("The old model parameters are shown in the UI")
            {
                CHECK(ui->fx->value() == Catch::Approx(params.oldModelParams.getFx()));
                CHECK(ui->fy->value() == Catch::Approx(params.oldModelParams.getFy()));
                CHECK(ui->cx->value() == Catch::Approx(params.oldModelParams.getCx()));
                CHECK(ui->cy->value() == Catch::Approx(params.oldModelParams.getCy()));

                CHECK(ui->r2->value() == Catch::Approx(static_cast<double>(params.oldModelParams.getR2())));
                CHECK(ui->r4->value() == Catch::Approx(static_cast<double>(params.oldModelParams.getR4())));
                CHECK(ui->tx->value() == Catch::Approx(static_cast<double>(params.oldModelParams.getTx())));
                CHECK(ui->ty->value() == Catch::Approx(static_cast<double>(params.oldModelParams.getTy())));
                CHECK(ui->r6->value() == Catch::Approx(static_cast<double>(params.oldModelParams.getR6())));
                CHECK(ui->k4->value() == Catch::Approx(static_cast<double>(params.oldModelParams.getK4())));
                CHECK(ui->k5->value() == Catch::Approx(static_cast<double>(params.oldModelParams.getK5())));
                CHECK(ui->k6->value() == Catch::Approx(static_cast<double>(params.oldModelParams.getK6())));
                CHECK(ui->s1->value() == Catch::Approx(static_cast<double>(params.oldModelParams.getS1())));
                CHECK(ui->s2->value() == Catch::Approx(static_cast<double>(params.oldModelParams.getS2())));
                CHECK(ui->s3->value() == Catch::Approx(static_cast<double>(params.oldModelParams.getS3())));
                CHECK(ui->s4->value() == Catch::Approx(static_cast<double>(params.oldModelParams.getS4())));
                CHECK(ui->taux->value() == Catch::Approx(static_cast<double>(params.oldModelParams.getTaux())));
                CHECK(ui->tauy->value() == Catch::Approx(static_cast<double>(params.oldModelParams.getTauy())));
                CHECK(
                    ui->intrError->text().toDouble() ==
                    Catch::Approx(static_cast<double>(params.oldModelParams.reprojectionError)));
            }
        }
    }
}

TEST_CASE("IntrinsicBox: reading/writing xml")
{
    AutoCalib         autoCalib;
    CalibFilter       filterCalib;
    Petrack           petrack{"Unkown"};
    Autosave          autosave(petrack);
    PersonStorage     storage(petrack, autosave);
    ExtrCalibration   calib(storage);
    ExtrinsicBox      extrBox(nullptr, calib);
    Ui::IntrinsicBox *ui      = new Ui::IntrinsicBox(); // ownership transferred to IntrinsicBox
    auto              intrBox = IntrinsicBox(nullptr, ui, autoCalib, filterCalib, extrBox, []() {});

    // spliting, because changing some of these also changes the intrinsic params
    WHEN("We change non-calculated values")
    {
        ui->apply->setChecked(!ui->apply->isChecked());
        ui->boardSizeX->setValue(ui->boardSizeX->value() + 1);
        ui->boardSizeY->setValue(ui->boardSizeY->value() + 1);
        ui->squareSize->setValue(ui->squareSize->value() + 1);
        ui->quadAspectRatio->setChecked(!ui->quadAspectRatio->isChecked());
        ui->fixCenter->setChecked(!ui->fixCenter->isChecked());
        ui->tangDist->setChecked(!ui->tangDist->isChecked());
        ui->extModelCheckBox->setChecked(!ui->extModelCheckBox->isChecked());
        // only calibrating or calling this function applies these changes to the pet file
        intrBox.setCalibSettings();

        AND_WHEN("We save the widget into a pet-file")
        {
            QDomDocument doc;
            auto         root = doc.createElement("PETRACK");
            root.setAttribute("VERSION", "0.9.2");
            doc.appendChild(root);
            QDomElement elem = doc.createElement("CALIBRATION");
            root.appendChild(elem);
            intrBox.setXml(elem);
            THEN("We can read in the values later on")
            {
                AutoCalib         newAutoCalib;
                CalibFilter       newFilterCalib;
                Ui::IntrinsicBox *newUi = new Ui::IntrinsicBox(); // ownership transferred to IntrinsicBox
                auto newIntrBox         = IntrinsicBox(nullptr, newUi, newAutoCalib, newFilterCalib, extrBox, []() {});

                for(auto subElem = elem.firstChildElement(); !subElem.isNull(); subElem = subElem.nextSiblingElement())
                {
                    newIntrBox.getXml(subElem);
                }


                INFO(nodeToString(elem).toStdString());
                CHECK(newUi->apply->isChecked() == ui->apply->isChecked());
                CHECK(newUi->boardSizeX->value() == ui->boardSizeX->value());
                CHECK(newUi->boardSizeY->value() == ui->boardSizeY->value());
                CHECK(newUi->squareSize->value() == Catch::Approx(ui->squareSize->value()));
                CHECK(newUi->quadAspectRatio->isChecked() == ui->quadAspectRatio->isChecked());
                CHECK(newUi->fixCenter->isChecked() == ui->fixCenter->isChecked());
                CHECK(newUi->tangDist->isChecked() == ui->tangDist->isChecked());
                CHECK(newUi->extModelCheckBox->isChecked() == ui->extModelCheckBox->isChecked());

                AND_THEN("parameters are correctly disabled")
                {
                    CHECK(!newUi->cx->isEnabled());
                    CHECK(!newUi->cy->isEnabled());
                    CHECK(!newUi->fy->isEnabled());
                    CHECK(!newUi->tx->isEnabled());
                    CHECK(!newUi->ty->isEnabled());
                    CHECK(!newUi->k4->isEnabled());
                    CHECK(!newUi->k5->isEnabled());
                    CHECK(!newUi->k6->isEnabled());
                    CHECK(!newUi->s1->isEnabled());
                    CHECK(!newUi->s2->isEnabled());
                    CHECK(!newUi->s3->isEnabled());
                    CHECK(!newUi->s4->isEnabled());
                    CHECK(!newUi->taux->isEnabled());
                    CHECK(!newUi->tauy->isEnabled());
                }
            }
        }
    }
    WHEN("We change calibration options checkboxes")
    {
        ui->quadAspectRatio->setChecked(!ui->quadAspectRatio->isChecked());
        ui->fixCenter->setChecked(!ui->fixCenter->isChecked());
        ui->tangDist->setChecked(!ui->tangDist->isChecked());

        AND_WHEN("We save the widget into a pet-file")
        {
            QDomDocument doc;
            auto         root = doc.createElement("PETRACK");
            root.setAttribute("VERSION", "0.9.2");
            doc.appendChild(root);
            QDomElement elem = doc.createElement("CALIBRATION");
            root.appendChild(elem);
            intrBox.setXml(elem);
            // options only change when you calibrate again
            THEN("The options aren't actually saved in the pet-file")
            {
                AutoCalib         newAutoCalib;
                CalibFilter       newFilterCalib;
                Ui::IntrinsicBox *newUi = new Ui::IntrinsicBox(); // ownership transferred to IntrinsicBox
                auto newIntrBox         = IntrinsicBox(nullptr, newUi, newAutoCalib, newFilterCalib, extrBox, []() {});

                for(auto subElem = elem.firstChildElement(); !subElem.isNull(); subElem = subElem.nextSiblingElement())
                {
                    newIntrBox.getXml(subElem);
                }
                INFO(nodeToString(elem).toStdString());
                CHECK(newUi->quadAspectRatio->isChecked() == !ui->quadAspectRatio->isChecked());
                CHECK(newUi->fixCenter->isChecked() == !ui->fixCenter->isChecked());
                CHECK(newUi->tangDist->isChecked() == !ui->tangDist->isChecked());
            }
        }
    }
    WHEN("We change calculated values")
    {
        // set different parameters:
        IntrinsicModelsParameters params;
        params.oldModelParams.cameraMatrix = (cv::Mat_<double>(3, 3) << 800, 0, 5, 0, 900, 7, 0, 0, 1);
        // values need to be in valid range -5 to 5
        params.oldModelParams.distortionCoeffs =
            (cv::Mat_<float>(1, 14) << -4.5, -4, -3.5, -3, -2.5, -2, -1.5, -1, 1, 1.5, 2, 2.5, 3, 3.5);
        params.oldModelParams.reprojectionError = 0.5;

        params.extModelParams.cameraMatrix = (cv::Mat_<double>(3, 3) << 600, 0, 5, 0, 800, 7, 0, 0, 1);
        // values need to be in valid range -5 to 5
        params.extModelParams.distortionCoeffs =
            (cv::Mat_<float>(1, 14) << 4.5, 4, 3.5, 3, 2.5, 2, 1.5, 1, -1, -1.5, -2, -2.5, -3, -3.5);
        params.extModelParams.reprojectionError = 78.5;
        intrBox.setIntrinsicCameraParams(params);

        AND_WHEN("We save the widget into a pet-file")
        {
            QDomDocument doc;
            auto         root = doc.createElement("PETRACK");
            root.setAttribute("VERSION", "0.10.1");
            doc.appendChild(root);
            QDomElement elem = doc.createElement("CALIBRATION");
            root.appendChild(elem);
            intrBox.setXml(elem);
            THEN("We can read in the values later on")
            {
                AutoCalib         newAutoCalib;
                CalibFilter       newFilterCalib;
                Ui::IntrinsicBox *newUi = new Ui::IntrinsicBox(); // ownership transferred to IntrinsicBox
                auto newIntrBox         = IntrinsicBox(nullptr, newUi, newAutoCalib, newFilterCalib, extrBox, []() {});

                for(auto subElem = elem.firstChildElement(); !subElem.isNull(); subElem = subElem.nextSiblingElement())
                {
                    newIntrBox.getXml(subElem);
                }

                INFO(nodeToString(elem).toStdString());
                auto originalParams   = intrBox.getBothIntrinsicCameraParams();
                auto serializedParams = newIntrBox.getBothIntrinsicCameraParams();
                CHECK(
                    cv::norm(
                        originalParams.oldModelParams.cameraMatrix - serializedParams.oldModelParams.cameraMatrix) ==
                    Catch::Approx(0).margin(SERIALIZATION_MARGIN));
                CHECK(
                    cv::norm(
                        originalParams.oldModelParams.distortionCoeffs -
                        serializedParams.oldModelParams.distortionCoeffs) ==
                    Catch::Approx(0).margin(SERIALIZATION_MARGIN));

                CHECK(
                    originalParams.oldModelParams.reprojectionError ==
                    Catch::Approx(serializedParams.oldModelParams.reprojectionError));

                // check extended model parameters
                CHECK(
                    cv::norm(
                        originalParams.extModelParams.cameraMatrix - serializedParams.extModelParams.cameraMatrix) ==
                    Catch::Approx(0).margin(SERIALIZATION_MARGIN));
                CHECK(
                    cv::norm(
                        originalParams.extModelParams.distortionCoeffs -
                        serializedParams.extModelParams.distortionCoeffs) ==
                    Catch::Approx(0).margin(SERIALIZATION_MARGIN));

                if(qIsNaN(originalParams.extModelParams.reprojectionError))
                {
                    CHECK(qIsNaN(serializedParams.extModelParams.reprojectionError));
                }
                else
                {
                    CHECK(
                        originalParams.extModelParams.reprojectionError ==
                        Catch::Approx(serializedParams.extModelParams.reprojectionError));
                }
            }
        }
    }
}

TEST_CASE("ImageSizeChanged")
{
    AutoCalib       autoCalib;
    CalibFilter     filterCalib;
    Petrack         petrack{"Unkown"};
    Autosave        autosave(petrack);
    PersonStorage   storage(petrack, autosave);
    ExtrCalibration calib(storage);
    ExtrinsicBox    extrBox(nullptr, calib);
    auto           *ui      = new Ui::IntrinsicBox(); // ownership transferred to IntrinsicBox
    auto            intrBox = IntrinsicBox(nullptr, ui, autoCalib, filterCalib, extrBox, []() {});
    constexpr int   width   = 1280;
    constexpr int   height  = 720;

    WHEN("the image size changes")
    {
        AND_GIVEN("fix center is set")
        {
            ui->fixCenter->setChecked(true);
            intrBox.imageSizeChanged(width, height, 0);

            double cxFixed = (width - 1) / 2.;
            double cyFixed = (height - 1) / 2.;
            THEN("cx and cy are set to the new image center")
            {
                IntrinsicCameraParams params = intrBox.getIntrinsicCameraParams();
                CHECK(params.getCx() == Catch::Approx(cxFixed));
                CHECK(params.getCy() == Catch::Approx(cyFixed));
                CHECK(ui->cx->value() == Catch::Approx(cxFixed));
                CHECK(ui->cy->value() == Catch::Approx(cyFixed));
            }
            AND_WHEN("I switch calibration models")
            {
                ui->extModelCheckBox->setChecked(false);
                IntrinsicCameraParams params = intrBox.getIntrinsicCameraParams();
                THEN("cx and cy are updated in ui and data accordingly")
                {
                    CHECK(params.getCx() == Catch::Approx(cxFixed));
                    CHECK(params.getCy() == Catch::Approx(cyFixed));
                    CHECK(ui->cx->value() == Catch::Approx(cxFixed));
                    CHECK(ui->cy->value() == Catch::Approx(cyFixed));
                }
            }
        }

        AND_GIVEN("fix center is not set **and** the border is changed")
        {
            ui->fixCenter->setChecked(false);
            intrBox.imageSizeChanged(width, height, 0);

            const auto cx = ui->cx->value();
            const auto cy = ui->cy->value();

            // change border by +20
            intrBox.imageSizeChanged(width + 20, height + 20, 20);

            THEN("cx and cy are updated accordingly")
            {
                CHECK(ui->cx->value() == Catch::Approx(cx + 20));
                CHECK(ui->cy->value() == Catch::Approx(cy + 20));
            }
        }
    }
}
