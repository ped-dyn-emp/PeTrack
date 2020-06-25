#include <catch2/catch.hpp>

#include <iostream>
#include "control.h"

#include <QSignalSpy>
#include <QTestEventList>
#include <QGraphicsScene>
#include "imageItem.h"

#include "view.h"


// TODO Connect und Disconnect vom Event (lässt sich an und abschalten)
// TODO Weitere Testcases überlegen

void checkSelectedColor(Control* con, bool invHue, int toHue, int fromHue,
                        int toSat, int fromSat, int toVal, int fromVal,
                        int mapX, int mapW, int mapY, int mapH){

    REQUIRE(con->getColorPlot()->getMapItem()->getActMapInvHue() == invHue);

    QColor toColor = con->getColorPlot()->getMapItem()->getActMapToColor();
    QColor fromColor = con->getColorPlot()->getMapItem()->getActMapFromColor();

    // Hue
    REQUIRE(toColor.hue() == toHue);
    REQUIRE(fromColor.hue() == fromHue);

    // Saturation
    REQUIRE(toColor.saturation() == toSat);
    REQUIRE(fromColor.saturation() == fromSat);

    // Value
    REQUIRE(toColor.value() == toVal);
    REQUIRE(fromColor.value() == fromVal);

    // Sliders
    REQUIRE(con->mapX->value() == mapX);
    REQUIRE(con->mapW->value() == mapW);
    REQUIRE(con->mapY->value() == mapY);
    REQUIRE(con->mapH->value() == mapH);
}


SCENARIO("I open PeTrack with a red image", "[ui][config]")
{
    Petrack pet {};
    cv::Mat redTestImage {cv::Size(50, 50), CV_8UC3, cv::Scalar(179,255,200)};
    cv::cvtColor(redTestImage, redTestImage, cv::COLOR_HSV2BGR);
    // NOTE Gibt es eine bessere Methode, als das Bild erst zu speichern?
    cv::imwrite("TESTBILD_DELETE_ME.png", redTestImage);
    pet.openSequence("TESTBILD_DELETE_ME.png");
    QPoint viewCenter = pet.getView()->viewport()->geometry().center();

    QPointer<Control> con = pet.getControlWidget();

    GIVEN("I click the colorPickerButton") {
        QTestEventList eventList;
        QPushButton* colorPickerButton = con->findChild<QPushButton*>("colorPickerButton");
        eventList.addMouseClick(Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier);
        eventList.simulate(colorPickerButton);

        THEN("The Button is checked") {
            REQUIRE(colorPickerButton->isChecked());
        }

        AND_GIVEN("I shift+click on one point of the (red) image"){
            eventList.clear();
            eventList.addMouseClick(Qt::MouseButton::LeftButton, Qt::KeyboardModifier::ShiftModifier, viewCenter);

            QSignalSpy setColorSpy{pet.getView(), SIGNAL(setColorEvent(QPoint, GraphicsView*))};
            REQUIRE(setColorSpy.isValid());
            setColorSpy.wait(20);
            eventList.simulate(pet.getView()->viewport());
            REQUIRE(setColorSpy.count() == 1);

            THEN("This pixel and its sourroundings get selected as new color"){
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
                int mapW= 350;
                // y = 2 * fromSaturation = 500
                int mapY = 500;
                // h = toSaturation - fromSaturation = 5
                int mapH = 5;

                checkSelectedColor(con, invHue, toHue, fromHue, toSat, fromSat, toVal, fromVal, mapX, mapW, mapY, mapH);

                AND_GIVEN("Afterwards I left-click on a slightly different red pixel"){
                    cv::Mat differentRedTestImage {cv::Size(50, 50), CV_8UC3, cv::Scalar(170,235,220)};
                    cv::cvtColor(differentRedTestImage, differentRedTestImage, cv::COLOR_HSV2BGR);
                    pet.updateImage(differentRedTestImage);

                    eventList.clear();
                    eventList.addMouseClick(Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, viewCenter);
                    eventList.simulate(pet.getView()->viewport());

                    THEN("The selected color is expanded accordingly"){
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

                        checkSelectedColor(con, invHue, toHue, fromHue, toSat, fromSat, toVal, fromVal, mapX, mapW, mapY, mapH);
                    }
                }
            }
        }

        AND_GIVEN("I shift+click on a red-isch/magenta pixel (Hue=340)"){
            cv::Mat magentaTestImage {cv::Size(50, 50), CV_8UC3, cv::Scalar(170,235,220)};
            cv::cvtColor(magentaTestImage, magentaTestImage, cv::COLOR_HSV2BGR);
            pet.updateImage(magentaTestImage);

            QTestEventList eventList;
            eventList.addMouseClick(Qt::MouseButton::LeftButton, Qt::KeyboardModifier::ShiftModifier, viewCenter);
            eventList.simulate(pet.getView()->viewport());

            REQUIRE(!con->getColorPlot()->getMapItem()->getActMapInvHue());

            AND_GIVEN("Afterwards I click on an red image (Hue=10)"){
                cv::Mat otherSideRedImage {cv::Size(50, 50), CV_8UC3, cv::Scalar(5,235,220)};
                cv::cvtColor(otherSideRedImage, otherSideRedImage, cv::COLOR_HSV2BGR);
                pet.updateImage(otherSideRedImage);

                QTestEventList eventList;
                eventList.addMouseClick(Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, viewCenter);
                eventList.simulate(pet.getView()->viewport());

                THEN("Inverse hue gets checked"){ /*Lieber color changed accordingly und dementsprechend weiterarbeiten?*/
                    REQUIRE(con->getColorPlot()->getMapItem()->getActMapInvHue());

                }
            }
            AND_GIVEN("Afterwards I click on an pink Image (Hue=320)"){
                cv::Mat pinkTestImage {cv::Size(50, 50), CV_8UC3, cv::Scalar(160,235,220)};
                cv::cvtColor(pinkTestImage, pinkTestImage, cv::COLOR_HSV2BGR);
                pet.updateImage(pinkTestImage);

                QTestEventList eventList;
                eventList.addMouseClick(Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, viewCenter);
                eventList.simulate(pet.getView()->viewport());

                THEN("I still do not use inverse Hue"){
                    REQUIRE(!con->getColorPlot()->getMapItem()->getActMapInvHue());
                }
            }
        }
        AND_GIVEN("I shift+click on one specific pixel"){
            cv::Mat newTestImage {cv::Size(50, 50), CV_8UC3, cv::Scalar(50,235,220)};
            // set middle Pixel (clicked pixel) to other Value
            newTestImage.at<cv::Vec3b>(cv::Point(25,25)) = cv::Vec3b(100,255,255);
            cv::cvtColor(newTestImage, newTestImage, cv::COLOR_HSV2BGR);
            pet.updateImage(newTestImage);

            QTestEventList eventList;
            QPointF pointOnScene = pet.getImageItem()->mapToScene(QPoint(25, 25));
            QPoint pointOnViewport = pet.getView()->mapFromScene(pointOnScene);
            eventList.addMouseClick(Qt::MouseButton::LeftButton, Qt::KeyboardModifier::ShiftModifier,
                                    pointOnViewport);
            eventList.simulate(pet.getView()->viewport());

            THEN("I get a color selection fitting this single pixel"){
                bool invHue = false;
                // Upper Bound Hue 200 + 5 -> 105 Lower Bound: 200 - 5 -> 95
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
