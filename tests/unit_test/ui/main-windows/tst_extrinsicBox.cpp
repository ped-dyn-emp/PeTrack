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

#include "extrCalibration.h"
#include "extrinsicBox.h"
#include "personStorage.h"
#include "petrack.h"
#include "ui_extrinsicBox.h"
#include "util.h"

#include <QDomElement>
#include <catch2/catch.hpp>

TEST_CASE("ExtrinsicBox: display of values in UI")
{
    Petrack         petrack{"Unkown"};
    Autosave        autosave(petrack);
    PersonStorage   storage(petrack, autosave);
    ExtrCalibration calib(storage);
    Ui::extr       *ui = new Ui::extr();
    ExtrinsicBox    extrBox(nullptr, ui, calib);

    ExtrinsicParameters params;
    params.rot1   = 0.5;
    params.rot2   = 0.2;
    params.rot3   = -0.5;
    params.trans1 = 234;
    params.trans2 = 420;
    params.trans3 = 699;

    WHEN("We change the values of the extrinsic parameters")
    {
        extrBox.setExtrinsicParameters(params);

        THEN("The values are displayed accordingly")
        {
            CHECK(ui->rot1->value() == params.rot1);
            CHECK(ui->rot2->value() == params.rot2);
            CHECK(ui->rot3->value() == params.rot3);
            CHECK(ui->trans1->value() == params.trans1);
            CHECK(ui->trans2->value() == params.trans2);
            CHECK(ui->trans3->value() == params.trans3);
        }
    }

    WHEN("We change the values in the UI")
    {
        ui->rot1->setValue(params.rot1);
        ui->rot2->setValue(params.rot2);
        ui->rot3->setValue(params.rot3);
        ui->trans1->setValue(params.trans1);
        ui->trans2->setValue(params.trans2);
        ui->trans3->setValue(params.trans3);

        THEN("The changes are propagated accordingly")
        {
            CHECK(extrBox.getExtrinsicParameters() == params);
        }
    }
}

TEST_CASE("ExtrinsicBox: reading/writing xml")
{
    Petrack         petrack{"Unkown"};
    Autosave        autosave(petrack);
    PersonStorage   storage(petrack, autosave);
    ExtrCalibration calib(storage);
    ExtrinsicBox    extrBox(nullptr, calib);

    WHEN("We change the extrinsic parameters")
    {
        ExtrinsicParameters params;
        params.rot1   = 0.5;
        params.rot2   = 0.2;
        params.rot3   = -0.5;
        params.trans1 = 234;
        params.trans2 = 420;
        params.trans3 = 699;

        extrBox.setExtrinsicParameters(params);
        INFO(extrBox.getExtrinsicParameters().trans1);
        AND_WHEN("We save that state into an xml-document")
        {
            QDomDocument doc;
            QDomElement  elem = doc.createElement("EXTRINSIC_PARAMETERS");
            extrBox.setXml(elem);

            THEN("We can read that state in again")
            {
                Petrack         petrack{"Unkown"};
                Autosave        autosave(petrack);
                PersonStorage   storage(petrack, autosave);
                ExtrCalibration calib(storage);
                ExtrinsicBox    extrBox(nullptr, calib);

                extrBox.getXml(elem);
                INFO(nodeToString(elem));
                CHECK(extrBox.getExtrinsicParameters() == params);
            }
        }
    }
}
