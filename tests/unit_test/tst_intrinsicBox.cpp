/*
 * PeTrack - Software for tracking pedestrians movement in videos
 * Copyright (C) 2023 Forschungszentrum Jülich GmbH, IAS-7
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
 * along with this program.  If not, see <https://cdwww.gnu.org/licenses/>.
 */

#include "autoCalib.h"
#include "calibFilter.h"
#include "intrinsicBox.h"
#include "ui_intrinsicBox.h"
#include "util.h"

#include <QDomElement>
#include <QSignalSpy>
#include <QTextStream>
#include <catch2/catch.hpp>


bool matEq(const cv::Mat &lhs, const cv::Mat &rhs)
{
    return cv::countNonZero(lhs != rhs) == 0;
}

constexpr double SERIALIZATION_MARGIN = 0.01;

TEST_CASE("IntrinsicCameraParams have value semantics")
{
    IntrinsicCameraParams testParams;
    CHECK(matEq(testParams.cameraMatrix, cv::Mat::eye(cv::Size(3, 3), CV_64F)));
    CHECK(matEq(testParams.distortionCoeffs, cv::Mat::zeros(1, 14, CV_32F)));
    CHECK(std::isnan(testParams.reprojectionError));
    testParams.cameraMatrix.at<double>(0, 0) = 1000;
    testParams.distortionCoeffs.at<float>(0) = 2000;

    SECTION("Copy Constructor")
    {
        IntrinsicCameraParams copy{testParams};
        copy.cameraMatrix.at<double>(1, 1)  = -1;
        copy.distortionCoeffs.at<double>(1) = -1;
        CHECK(copy.cameraMatrix.at<double>(0, 0) == 1000);
        CHECK(copy.distortionCoeffs.at<float>(0) == 2000);
        CHECK(testParams.cameraMatrix.at<double>(1, 1) == 1);
        CHECK(testParams.distortionCoeffs.at<float>(1) == 0);
    }

    SECTION("Copy Assignment")
    {
        IntrinsicCameraParams copy;
        copy                                = testParams;
        copy.cameraMatrix.at<double>(1, 1)  = -1;
        copy.distortionCoeffs.at<double>(1) = -1;
        CHECK(copy.cameraMatrix.at<double>(0, 0) == 1000);
        CHECK(copy.distortionCoeffs.at<float>(0) == 2000);
        CHECK(testParams.cameraMatrix.at<double>(1, 1) == 1);
        CHECK(testParams.distortionCoeffs.at<float>(1) == 0);
    }
}

TEST_CASE("IntrinsicCameraParams values")
{
    AutoCalib         autoCalib;
    CalibFilter       filterCalib;
    Ui::IntrinsicBox *ui      = new Ui::IntrinsicBox(); // ownership transferred to IntrinsicBox
    auto              intrBox = IntrinsicBox(nullptr, ui, autoCalib, filterCalib, []() {});

    QSignalSpy spy{&intrBox, &IntrinsicBox::paramsChanged};
    CHECK(spy.isValid());

    WHEN("I set the values of the IntrinsicParameters (via the setter)")
    {
        IntrinsicCameraParams params;
        params.cameraMatrix = (cv::Mat_<double>(3, 3) << 800, 2, 3, 4, 900, 6, 7, 8, 9);
        std::iota(params.distortionCoeffs.begin<float>(), params.distortionCoeffs.end<float>(), -5);
        // values need to be in valid range -5 to 5
        params.distortionCoeffs.at<float>(10) = 1;
        params.distortionCoeffs.at<float>(11) = 2;
        params.distortionCoeffs.at<float>(12) = 3;
        params.distortionCoeffs.at<float>(13) = 4;

        params.reprojectionError = 42;

        intrBox.setIntrinsicCameraParams(params);

        THEN("The params are changed in the params-struct")
        {
            auto newParams = intrBox.getIntrinsicCameraParams();
            CHECK(intrBox.getIntrinsicCameraParams() == params);
        }

        THEN("The params are changed in the ui")
        {
            CHECK(ui->fx->value() == params.cameraMatrix.at<double>(0, 0));
            CHECK(ui->fy->value() == params.cameraMatrix.at<double>(1, 1));
            CHECK(ui->cx->value() == params.cameraMatrix.at<double>(0, 2));
            CHECK(ui->cy->value() == params.cameraMatrix.at<double>(1, 2));

            CHECK(ui->r2->value() == static_cast<double>(params.distortionCoeffs.at<float>(0)));
            CHECK(ui->r4->value() == static_cast<double>(params.distortionCoeffs.at<float>(1)));
            CHECK(ui->tx->value() == static_cast<double>(params.distortionCoeffs.at<float>(2)));
            CHECK(ui->ty->value() == static_cast<double>(params.distortionCoeffs.at<float>(3)));
            CHECK(ui->r6->value() == static_cast<double>(params.distortionCoeffs.at<float>(4)));
            CHECK(ui->k4->value() == static_cast<double>(params.distortionCoeffs.at<float>(5)));
            CHECK(ui->k5->value() == static_cast<double>(params.distortionCoeffs.at<float>(6)));
            CHECK(ui->k6->value() == static_cast<double>(params.distortionCoeffs.at<float>(7)));
            CHECK(ui->s1->value() == static_cast<double>(params.distortionCoeffs.at<float>(8)));
            CHECK(ui->s2->value() == static_cast<double>(params.distortionCoeffs.at<float>(9)));
            CHECK(ui->s3->value() == static_cast<double>(params.distortionCoeffs.at<float>(10)));
            CHECK(ui->s4->value() == static_cast<double>(params.distortionCoeffs.at<float>(11)));
            CHECK(ui->taux->value() == static_cast<double>(params.distortionCoeffs.at<float>(12)));
            CHECK(ui->tauy->value() == static_cast<double>(params.distortionCoeffs.at<float>(13)));
        }

        GIVEN("Some parameters are out of range")
        {
            params.distortionCoeffs.at<float>(12) = 56468;
            auto oldParams                        = intrBox.getIntrinsicCameraParams();
            THEN("The values do not get changed")
            {
                intrBox.setIntrinsicCameraParams(params);
                CHECK(intrBox.getIntrinsicCameraParams() == oldParams);
            }
        }

        THEN("The 'paramsChanged'-signal is fired exactly once")
        {
            CHECK(spy.count() == 1);
        }
    }
}

TEST_CASE("IntrinsicBox: reading/writing xml")
{
    AutoCalib         autoCalib;
    CalibFilter       filterCalib;
    Ui::IntrinsicBox *ui      = new Ui::IntrinsicBox(); // ownership transferred to IntrinsicBox
    auto              intrBox = IntrinsicBox(nullptr, ui, autoCalib, filterCalib, []() {});

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
                Ui::IntrinsicBox *newUi      = new Ui::IntrinsicBox(); // ownership transferred to IntrinsicBox
                auto              newIntrBox = IntrinsicBox(nullptr, newUi, newAutoCalib, newFilterCalib, []() {});

                for(auto subElem = elem.firstChildElement(); !subElem.isNull(); subElem = subElem.nextSiblingElement())
                {
                    newIntrBox.getXml(subElem);
                }


                INFO(nodeToString(elem).toStdString())
                CHECK(newUi->apply->isChecked() == ui->apply->isChecked());
                CHECK(newUi->boardSizeX->value() == ui->boardSizeX->value());
                CHECK(newUi->boardSizeY->value() == ui->boardSizeY->value());
                CHECK(newUi->squareSize->value() == Approx(ui->squareSize->value()));
                CHECK(newUi->quadAspectRatio->isChecked() == ui->quadAspectRatio->isChecked());
                CHECK(newUi->fixCenter->isChecked() == ui->fixCenter->isChecked());
                CHECK(newUi->tangDist->isChecked() == ui->tangDist->isChecked());
                CHECK(newUi->extModelCheckBox->isChecked() == ui->extModelCheckBox->isChecked());
            }
        }
    }

    WHEN("We change calculated values")
    {
        // set different parameters:
        IntrinsicCameraParams params;
        params.cameraMatrix = (cv::Mat_<double>(3, 3) << 800, 0, 5, 0, 900, 7, 0, 0, 1);
        std::iota(params.distortionCoeffs.begin<float>(), params.distortionCoeffs.end<float>(), -5);
        // values need to be in valid range -5 to 5
        params.distortionCoeffs.at<float>(10) = 1;
        params.distortionCoeffs.at<float>(11) = 2;
        params.distortionCoeffs.at<float>(12) = 3;
        params.distortionCoeffs.at<float>(13) = 4;
        params.reprojectionError              = 0.5;
        intrBox.setIntrinsicCameraParams(params);

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
                Ui::IntrinsicBox *newUi      = new Ui::IntrinsicBox(); // ownership transferred to IntrinsicBox
                auto              newIntrBox = IntrinsicBox(nullptr, newUi, newAutoCalib, newFilterCalib, []() {});

                for(auto subElem = elem.firstChildElement(); !subElem.isNull(); subElem = subElem.nextSiblingElement())
                {
                    newIntrBox.getXml(subElem);
                }

                INFO(nodeToString(elem).toStdString())
                auto oldParams = intrBox.getIntrinsicCameraParams();
                auto newParams = newIntrBox.getIntrinsicCameraParams();
                CHECK(
                    cv::norm(oldParams.cameraMatrix - newParams.cameraMatrix) ==
                    Approx(0).margin(SERIALIZATION_MARGIN));
                CHECK(
                    cv::norm(oldParams.distortionCoeffs - newParams.distortionCoeffs) ==
                    Approx(0).margin(SERIALIZATION_MARGIN));

                // NOTE: bug already in master, will fix after this is merged
                // CHECK(oldparams.reprojectionError == Approx(newparams.reprojectionError));
            }
        }
    }
}

TEST_CASE("QuadAspectRatio")
{
    AutoCalib   autoCalib;
    CalibFilter filterCalib;
    auto       *ui                       = new Ui::IntrinsicBox(); // ownership transferred to IntrinsicBox
    auto        intrBox                  = IntrinsicBox(nullptr, ui, autoCalib, filterCalib, []() {});
    auto        params                   = intrBox.getIntrinsicCameraParams();
    params.cameraMatrix.at<double>(0, 0) = 600;
    intrBox.setIntrinsicCameraParams(params);

    WHEN("I activate quad aspect ratio")
    {
        ui->quadAspectRatio->setChecked(true);

        THEN("fy equals fx")
        {
            const auto params = intrBox.getIntrinsicCameraParams();
            const auto fx     = params.cameraMatrix.at<double>(0, 0);
            const auto fy     = params.cameraMatrix.at<double>(1, 1);
            CHECK(fx == fy);
        }

        AND_WHEN("I deactivate quad aspect ratio afterwards")
        {
            ui->quadAspectRatio->setChecked(false);

            THEN("fy does not equal fx")
            {
                const auto params = intrBox.getIntrinsicCameraParams();
                const auto fx     = params.cameraMatrix.at<double>(0, 0);
                const auto fy     = params.cameraMatrix.at<double>(1, 1);
                CHECK(fx != fy);
            }
        }
    }
}

TEST_CASE("FixCenter")
{
    AutoCalib   autoCalib;
    CalibFilter filterCalib;
    auto       *ui        = new Ui::IntrinsicBox(); // ownership transferred to IntrinsicBox
    auto        intrBox   = IntrinsicBox(nullptr, ui, autoCalib, filterCalib, []() {});
    auto        oldParams = intrBox.getIntrinsicCameraParams();
    // would be called from ImageItem after setting the image
    constexpr auto width  = 1280;
    constexpr auto height = 720;
    intrBox.imageSizeChanged(width, height, 0);

    WHEN("I activate fix center")
    {
        ui->fixCenter->setChecked(true);

        THEN("cx and cy are set to the image center")
        {
            const auto params = intrBox.getIntrinsicCameraParams();
            const auto cx     = params.cameraMatrix.at<double>(0, 2);
            const auto cy     = params.cameraMatrix.at<double>(1, 2);
            CHECK(cx == Approx((width - 1) / 2.));
            CHECK(cy == Approx((height - 1) / 2.));
        }

        AND_WHEN("I deactivate fix center afterwards")
        {
            ui->fixCenter->setChecked(false);

            THEN("cx and cy have their old values")
            {
                const auto params = intrBox.getIntrinsicCameraParams();
                const auto cx     = params.cameraMatrix.at<double>(0, 2);
                const auto cy     = params.cameraMatrix.at<double>(1, 2);
                CHECK(cx == oldParams.cameraMatrix.at<double>(0, 2));
                CHECK(cy == oldParams.cameraMatrix.at<double>(1, 2));
            }
        }
    }
}

TEST_CASE("ImageSizeChanged")
{
    AutoCalib     autoCalib;
    CalibFilter   filterCalib;
    auto         *ui      = new Ui::IntrinsicBox(); // ownership transferred to IntrinsicBox
    auto          intrBox = IntrinsicBox(nullptr, ui, autoCalib, filterCalib, []() {});
    constexpr int width   = 1280;
    constexpr int height  = 720;

    WHEN("the image size changes")
    {
        intrBox.imageSizeChanged(width, height, 0);

        THEN("Max cx and cy are updated to the image size")
        {
            CHECK(ui->cx->maximum() == Approx(width));
            CHECK(ui->cy->maximum() == Approx(height));
        }
    }

    WHEN("the image size changes")
    {
        AND_GIVEN("fix center is set")
        {
            ui->fixCenter->setChecked(true);
            intrBox.imageSizeChanged(width, height, 0);
            THEN("cx and cy are set to the new image center")
            {
                CHECK(ui->cx->value() == Approx((width - 1) / 2.));
                CHECK(ui->cy->value() == Approx((height - 1) / 2.));
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
                CHECK(ui->cx->value() == Approx(cx + 20));
                CHECK(ui->cy->value() == Approx(cy + 20));
            }
        }
    }
}
