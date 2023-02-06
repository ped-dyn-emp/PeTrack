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
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "control.h"
#include "imageItem.h"
#include "ui_control.h"
#include "util.h"
#include "view.h"

#include <QGraphicsScene>
#include <QSignalSpy>
#include <QTestEventList>
#include <catch2/catch.hpp>
#include <iostream>
#include <memory>


// TODO Connect und Disconnect vom Event (lässt sich an und abschalten)
// TODO Weitere Testcases überlegen

void checkSelectedColor(
    Control *con,
    bool     invHue,
    int      toHue,
    int      fromHue,
    int      toSat,
    int      fromSat,
    int      toVal,
    int      fromVal,
    int      mapX,
    int      mapW,
    int      mapY,
    int      mapH)
{
    REQUIRE(con->getColorPlot()->getMapItem()->getActMapInvHue() == invHue);

    QColor toColor   = con->getColorPlot()->getMapItem()->getActMapToColor();
    QColor fromColor = con->getColorPlot()->getMapItem()->getActMapFromColor();

    // Hue
    REQUIRE(toColor.hue() == Approx(toHue).margin(1));
    REQUIRE(fromColor.hue() == Approx(fromHue).margin(1));

    // Saturation
    REQUIRE(toColor.saturation() == Approx(toSat).margin(1));
    REQUIRE(fromColor.saturation() == Approx(fromSat).margin(1));

    // Value
    REQUIRE(toColor.value() == Approx(toVal).margin(1));
    REQUIRE(fromColor.value() == Approx(fromVal).margin(1));

    // Sliders
    REQUIRE(con->getMapX() == Approx(mapX).margin(2));
    REQUIRE(con->getMapW() == Approx(mapW).margin(2));
    REQUIRE(con->getMapY() == Approx(mapY).margin(2));
    REQUIRE(con->getMapH() == Approx(mapH).margin(2));
}


SCENARIO("I open PeTrack with a red image", "[ui][config]")
{
    Petrack pet{"Unknown"};
    cv::Mat redTestImage{cv::Size(50, 50), CV_8UC3, cv::Scalar(179, 255, 200)};
    cv::cvtColor(redTestImage, redTestImage, cv::COLOR_HSV2BGR);
    // NOTE Gibt es eine bessere Methode, als das Bild erst zu speichern?
    cv::imwrite("TESTBILD_DELETE_ME.png", redTestImage);
    pet.openSequence("TESTBILD_DELETE_ME.png");
    QPoint viewCenter = pet.getView()->viewport()->geometry().center();

    QPointer<Control> con = pet.getControlWidget();

    GIVEN("I click the colorPickerButton")
    {
        QTestEventList eventList;
        QPushButton   *colorPickerButton = con->findChild<QPushButton *>("colorPickerButton");
        eventList.addMouseClick(Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier);
        eventList.simulate(colorPickerButton);

        THEN("The Button is checked")
        {
            REQUIRE(colorPickerButton->isChecked());
        }

        AND_GIVEN("I shift+click on one point of the (red) image")
        {
            eventList.clear();
            eventList.addMouseClick(Qt::MouseButton::LeftButton, Qt::KeyboardModifier::ShiftModifier, viewCenter);
            eventList.addMouseClick(Qt::MouseButton::LeftButton, Qt::KeyboardModifier::ShiftModifier, viewCenter);

            QSignalSpy setColorSpy{pet.getView(), &GraphicsView::setColorEvent};
            REQUIRE(setColorSpy.isValid());
            setColorSpy.wait(20);
            eventList.simulate(pet.getView()->viewport());
            REQUIRE(setColorSpy.count() == 2);

            THEN("This pixel and its sourroundings get selected as new color")
            {
                // Red, so we need Inverse Hue
                bool invHue = true;

                // Hue Upper Bound: 358(179) + 5 (current Buffer) -> 363 -> 3
                // Hue Lower Bound: 358(179) - 5 (current Buffer) -> 353
                int toHue = 3, fromHue = 353;

                // Saturation
                // Upper Bound: 255 + 5 -> 255
                // Lower Bound: 255 - 5 -> 250
                int toSat = 255, fromSat = 250;

                // Value
                // Upper Bound: 200 + 5 -> 205
                // Lower Bound: 200 - 5 -> 195
                int toVal = 205, fromVal = 195;

                // Sliders
                // x = 2 * 3 = 6
                int mapX = 6;
                // w = |toHue - fromHue| = 350
                int mapW = 350;
                // y = 2 * fromSaturation = 500
                int mapY = 500;
                // h = toSaturation - fromSaturation = 5
                int mapH = 5;

                checkSelectedColor(con, invHue, toHue, fromHue, toSat, fromSat, toVal, fromVal, mapX, mapW, mapY, mapH);

                AND_GIVEN("Afterwards I left-click on a slightly different red pixel")
                {
                    cv::Mat differentRedTestImage{cv::Size(50, 50), CV_8UC3, cv::Scalar(170, 235, 220)};
                    cv::cvtColor(differentRedTestImage, differentRedTestImage, cv::COLOR_HSV2BGR);
                    pet.updateImage(differentRedTestImage);

                    eventList.clear();
                    eventList.addMouseClick(Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, viewCenter);
                    eventList.simulate(pet.getView()->viewport());

                    THEN("The selected color is expanded accordingly")
                    {
                        // new lower bound on Hue is 340 - 10 -> 330
                        fromHue = 330;
                        // 330 (fromHue) - 3 (toHue) = 327
                        mapW = 327;
                        // new lower bound on Saturation is 235 - 10 -> 225
                        fromSat = 225;
                        // 255 (toSat) - 225 (fromSat) = 30
                        mapH = 30;
                        // 225 * 2 = 450
                        mapY = 450;
                        // new upper bound on Value is 220 + 10 -> 230
                        toVal = 230;

                        checkSelectedColor(
                            con, invHue, toHue, fromHue, toSat, fromSat, toVal, fromVal, mapX, mapW, mapY, mapH);
                    }
                }
            }
        }

        AND_GIVEN("I shift+click on a red-isch/magenta pixel (Hue=340)")
        {
            cv::Mat magentaTestImage{cv::Size(50, 50), CV_8UC3, cv::Scalar(170, 235, 220)};
            cv::cvtColor(magentaTestImage, magentaTestImage, cv::COLOR_HSV2BGR);
            pet.updateImage(magentaTestImage);

            QTestEventList eventList;
            eventList.addMouseClick(Qt::MouseButton::LeftButton, Qt::KeyboardModifier::ShiftModifier, viewCenter);
            eventList.addMouseClick(Qt::MouseButton::LeftButton, Qt::KeyboardModifier::ShiftModifier, viewCenter);
            eventList.simulate(pet.getView()->viewport());

            REQUIRE(!con->getColorPlot()->getMapItem()->getActMapInvHue());

            AND_GIVEN("Afterwards I click on an red image (Hue=10)")
            {
                cv::Mat otherSideRedImage{cv::Size(50, 50), CV_8UC3, cv::Scalar(5, 235, 220)};
                cv::cvtColor(otherSideRedImage, otherSideRedImage, cv::COLOR_HSV2BGR);
                pet.updateImage(otherSideRedImage);

                QTestEventList eventList;
                eventList.addMouseClick(Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, viewCenter);
                eventList.simulate(pet.getView()->viewport());

                THEN("Inverse hue gets checked")
                { /*Lieber color changed accordingly und dementsprechend weiterarbeiten?*/
                    REQUIRE(con->getColorPlot()->getMapItem()->getActMapInvHue());
                }
            }
            AND_GIVEN("Afterwards I click on an pink Image (Hue=320)")
            {
                cv::Mat pinkTestImage{cv::Size(50, 50), CV_8UC3, cv::Scalar(160, 235, 220)};
                cv::cvtColor(pinkTestImage, pinkTestImage, cv::COLOR_HSV2BGR);
                pet.updateImage(pinkTestImage);

                QTestEventList eventList;
                eventList.addMouseMove(viewCenter);
                eventList.addMouseClick(Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, viewCenter);
                eventList.simulate(pet.getView()->viewport());

                THEN("I still do not use inverse Hue")
                {
                    REQUIRE(!con->getColorPlot()->getMapItem()->getActMapInvHue());
                }
            }
        }
        AND_GIVEN("I shift+click on one specific pixel")
        {
            cv::Mat newTestImage{cv::Size(50, 50), CV_8UC3, cv::Scalar(50, 235, 220)};
            // set middle Pixel (clicked pixel) to other Value
            newTestImage.at<cv::Vec3b>(cv::Point(25, 25)) = cv::Vec3b(100, 255, 255);
            cv::cvtColor(newTestImage, newTestImage, cv::COLOR_HSV2BGR);
            pet.updateImage(newTestImage);
            QMouseEvent event{QEvent::HoverMove, QPoint(25, 25), Qt::NoButton, Qt::NoButton, Qt::NoModifier};

            QPointF pointOnScene    = pet.getImageItem()->mapToScene(QPoint(25, 25));
            QPoint  pointOnViewport = pet.getView()->mapFromScene(pointOnScene);

            // Only after a click the hover event setting mMousePosOnImage gets fired. Focus?
            eventList.addMouseClick(Qt::MouseButton::LeftButton, Qt::KeyboardModifier::ShiftModifier, pointOnViewport);
            eventList.addMouseClick(Qt::MouseButton::LeftButton, Qt::KeyboardModifier::ShiftModifier, pointOnViewport);
            eventList.simulate(pet.getView()->viewport());

            THEN("I get a color selection fitting this single pixel")
            {
                bool invHue = false;
                // Upper Bound Hue 200 + 5 -> 205 Lower Bound: 200 - 5 -> 195
                int fromHue = 195, toHue = 205;
                // mapW = tH-fH = 10
                int mapW = 10;
                // mapX = 2*fH = 2 * 195 = 390
                int mapX = 390;
                // Upper bound saturation 255 + 5 -> 255 Lower bound 255 - 5 -> 250
                int fromSat = 250, toSat = 255;
                // map H = 255 - 250 = 5
                int mapH = 5;
                // mapY = 2 * 250 = 500
                int mapY = 500;
                // Upper bound value 255 + 5 -> 255 Lower bound 255 - 5 -> 250
                int fromVal = 250, toVal = 255;

                checkSelectedColor(con, invHue, toHue, fromHue, toSat, fromSat, toVal, fromVal, mapX, mapW, mapY, mapH);
            }
        }
    } // Clicked Color Picker Button
} // Scenario

SCENARIO("Open PeTrack check defaults", "[ui][config]")
{
    Petrack pet{"Unknown"};
    REQUIRE(pet.getRecognizer().getRecoMethod() == reco::RecognitionMethod::MultiColor);
}


TEST_CASE("Loading from and saving to XML node", "[config]")
{
    Petrack      pet{"0.9.1"};
    Control     *control = pet.getControlWidget();
    QDomDocument doc;
    QDomElement  save = doc.createElement("CONTROL");
    control->setXml(save);

    SECTION("PATH")
    {
        QDomElement pathNode = save.elementsByTagName("PATH").at(0).toElement();
        REQUIRE(!pathNode.isNull());
        SECTION("ONLY_PEOPLE_NR_LIST")
        {
            REQUIRE(pathNode.hasAttribute("ONLY_PEOPLE_NR_LIST"));

            // IMPORTANT: reading ONLY_PEOPLE_NR is done in petrack.cpp, as the trajectories need to be loaded
            // before! Therefore using save from petrack, not Control
            QDomDocument doc;
            pet.saveXml(doc);
            const QString testValue{"101-987"};
            doc.elementsByTagName("PATH").at(0).toElement().setAttribute("ONLY_PEOPLE_NR_LIST", testValue);
            pet.openXml(doc, false);
            REQUIRE(control->trackShowOnlyNrList()->text() == testValue);
        }
    }
}

SCENARIO("Change the show only people list", "[ui][config][tracking][path]")
{
    Petrack  pet{"Unknown"};
    Control *control = pet.getControlWidget();
    control->setTrackShowOnlyListChecked(true);

    GIVEN("No filter given")
    {
        REQUIRE(control->isTrackShowOnlyListChecked());
        REQUIRE(control->trackShowOnlyNrList()->text().isEmpty());

        WHEN("Enter valid filter (single values)")
        {
            QSet<size_t> enteredIDs{1, 4, 6, 7};
            QSet<size_t> expectedIDs;
            for(auto id : enteredIDs)
            {
                expectedIDs.insert(id - 1);
            }

            std::stringstream input;
            std::copy(enteredIDs.begin(), enteredIDs.end(), std::ostream_iterator<size_t>(input, ","));

            QTest::keyClicks(control->trackShowOnlyNrList(), input.str().c_str(), Qt::NoModifier, 50);
            auto receivedIDs = pet.getPedestrianUserSelection();

            THEN("the entered ids should be returned")
            {
                REQUIRE(expectedIDs == receivedIDs);
                REQUIRE(control->trackShowOnlyNrList()->styleSheet().isEmpty());
            }
        }

        WHEN("Enter valid filter (range)")
        {
            QSet<size_t> enteredIDs{1, 2, 3, 4};
            QSet<size_t> expectedIDs;
            for(auto id : enteredIDs)
            {
                expectedIDs.insert(id - 1);
            }

            QString input("1-4");

            QSet<size_t> receivedIDs;
            QTest::keyClicks(control->trackShowOnlyNrList(), input, Qt::NoModifier, 50);
            receivedIDs = pet.getPedestrianUserSelection();

            THEN("the entered ids should be returned")
            {
                REQUIRE(expectedIDs == receivedIDs);
                REQUIRE(control->trackShowOnlyNrList()->styleSheet().isEmpty());
            }
        }

        WHEN("Enter invalid filter")
        {
            QString input("1-");

            THEN("the border should be red")
            {
                QTest::keyClicks(control->trackShowOnlyNrList(), input, Qt::NoModifier, 50);
                auto receivedIDs = pet.getPedestrianUserSelection();
                REQUIRE(receivedIDs.isEmpty());
                REQUIRE(control->trackShowOnlyNrList()->styleSheet().contains("border: 1px solid red"));
            }
        }

        WHEN("Enter filter (element-wise)")
        {
            QString input("1, 3,4,10, 20-40, 50-45, 100");
            THEN("no exception should be thrown")
            {
                for(auto character : input.toStdString())
                {
                    QTest::keyClick(control->trackShowOnlyNrList(), character, Qt::NoModifier, 10);

                    QSet<size_t> receivedIDs = pet.getPedestrianUserSelection();
                    if(receivedIDs.isEmpty())
                    {
                        REQUIRE(control->trackShowOnlyNrList()->styleSheet().contains("border: 1px solid red"));
                    }
                    else
                    {
                        REQUIRE(control->trackShowOnlyNrList()->styleSheet().isEmpty());
                    }
                }
            }
        }
    }
}
