/*
 * PeTrack - Software for tracking pedestrians movement in videos
 * Copyright (C) 2010-2020 Forschungszentrum Jülich GmbH,
 * Maik Boltes, Juliane Adrian, Ricardo Martin Brualla, Arne Graf, Paul Häger, Daniel Hillebrand,
 * Deniz Kilic, Paul Lieberenz, Daniel Salden, Tobias Schrödter, Ann Katrin Seemann
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

#include <catch2/catch.hpp>
#include <fstream>
#include <ostream>

#include "IO.h"

TEST_CASE("src/IO", "[tracking][io]")
{
    SECTION("readHeightFile")
    {
        SECTION("broken input")
        {
            SECTION("could not open file")
            {
                auto errorMessage = IO::readHeightFile("not_exisiting_file.txt");
                REQUIRE(std::holds_alternative<std::string>(errorMessage));
                std::string errorAsString = std::get<std::string>(errorMessage);
                REQUIRE_THAT(errorAsString, Catch::Matchers::Equals("Could not open not_exisiting_file.txt"));
            }

            SECTION("wrong file extension")
            {
                auto errorMessage = IO::readHeightFile("wrong.extension");
                REQUIRE(std::holds_alternative<std::string>(errorMessage));
                std::string errorAsString = std::get<std::string>(errorMessage);
                REQUIRE_THAT(errorAsString, Catch::Matchers::Equals(
                        "Cannot load wrong.extension maybe because of wrong file extension. Needs to be .txt."));
            }

            SECTION("heightFileName empty")
            {
                auto errorMessage = IO::readHeightFile("");
                REQUIRE(std::holds_alternative<std::string>(errorMessage));
                std::string errorAsString = std::get<std::string>(errorMessage);
                REQUIRE_THAT(errorAsString, Catch::Matchers::Equals("No file provided."));
            }

            std::string heigtFileName{"heights.txt"};
            std::ofstream heightFile(heigtFileName);
            heightFile << "# id z/cm" << std::endl;

            SECTION("Duplicate entry in file")
            {
                heightFile << "987 184" << std::endl;
                heightFile << "987 184" << std::endl;
                heightFile.close();

                auto errorMessage = IO::readHeightFile(QString::fromStdString(heigtFileName));
                REQUIRE(std::holds_alternative<std::string>(errorMessage));
                std::string errorAsString = std::get<std::string>(errorMessage);
                REQUIRE_THAT(errorAsString, Catch::Matchers::Equals("Duplicate entry for markerID = 987."));
            }

            SECTION("Wrong inputs")
            {
                SECTION("marker")
                {
                    SECTION("floating point value")
                    {
                        heightFile << "234.234 184" << std::endl;
                        heightFile.close();

                        auto errorMessage = IO::readHeightFile(QString::fromStdString(heigtFileName));
                        REQUIRE(std::holds_alternative<std::string>(errorMessage));
                        std::string errorAsString = std::get<std::string>(errorMessage);
                        REQUIRE_THAT(errorAsString,
                                     Catch::Matchers::Equals("Marker needs to be an integer value, but is 234.234"));
                    }

                    SECTION("arbitrary string")
                    {
                        heightFile << "WRONG 184" << std::endl;
                        heightFile.close();

                        auto errorMessage = IO::readHeightFile(QString::fromStdString(heigtFileName));
                        REQUIRE(std::holds_alternative<std::string>(errorMessage));
                        std::string errorAsString = std::get<std::string>(errorMessage);
                        REQUIRE_THAT(errorAsString,
                                     Catch::Matchers::Equals("Marker needs to be an integer value, but is WRONG"));
                    }
                }

                SECTION("height")
                {
                    SECTION("not floating point")
                    {
                        heightFile << "123 WRONG" << std::endl;
                        heightFile.close();

                        auto errorMessage = IO::readHeightFile(QString::fromStdString(heigtFileName));
                        REQUIRE(std::holds_alternative<std::string>(errorMessage));
                        std::string errorAsString = std::get<std::string>(errorMessage);
                        REQUIRE_THAT(errorAsString, Catch::Matchers::Equals(
                                "Height needs to be a positive numerical value, but is WRONG"));
                    }

                    SECTION("not positive")
                    {
                        heightFile << "123 0" << std::endl;
                        heightFile.close();

                        auto errorMessage = IO::readHeightFile(QString::fromStdString(heigtFileName));
                        REQUIRE(std::holds_alternative<std::string>(errorMessage));
                        std::string errorAsString = std::get<std::string>(errorMessage);
                        REQUIRE_THAT(errorAsString, Catch::Matchers::Equals(
                                "Height needs to be a positive numerical value, but is 0"));
                    }
                }

                SECTION("too many columns")
                {
                    heightFile << "987 184 123 WRONG" << std::endl;
                    heightFile.close();

                    auto errorMessage = IO::readHeightFile(QString::fromStdString(heigtFileName));
                    REQUIRE(std::holds_alternative<std::string>(errorMessage));
                    std::string errorAsString = std::get<std::string>(errorMessage);
                    REQUIRE_THAT(errorAsString, Catch::Matchers::Equals(
                            "Line should contain exactly 2 values: id height. But it contains 4 entries."));
                }

                SECTION("too few columns")
                {
                    heightFile << "987" << std::endl;
                    heightFile.close();

                    auto errorMessage = IO::readHeightFile(QString::fromStdString(heigtFileName));
                    REQUIRE(std::holds_alternative<std::string>(errorMessage));
                    std::string errorAsString = std::get<std::string>(errorMessage);
                    REQUIRE_THAT(errorAsString, Catch::Matchers::Equals(
                            "Line should contain exactly 2 values: id height. But it contains 1 entries."));
                }
            }

            std::remove(heigtFileName.c_str());
        }


        SECTION("correct input")
        {
            std::string heigtFileName{"heights.txt"};
            std::ofstream heightFile(heigtFileName);

            SECTION("file empty")
            {
                heightFile.close();
                auto ret = IO::readHeightFile(QString::fromStdString(heigtFileName));
                REQUIRE(std::holds_alternative<std::unordered_map<int, float>>(ret));
                std::unordered_map<int, float> markerHeights = std::get<std::unordered_map<int, float>>(ret);
                REQUIRE(markerHeights.empty());
            }

            std::unordered_map<int, float> referenceValuesMap{
                    {987, 184},   //NOLINT
                    {988, 179},   //NOLINT
                    {989, 177.5}, //NOLINT
                    {990, 154.5}, //NOLINT
                    {991, 177},   //NOLINT
                    {992, 177},   //NOLINT
                    {993, 175.5}, //NOLINT
                    {994, 194},   //NOLINT
                    {995, 177},   //NOLINT
                    {996, 167},   //NOLINT
                    {997, 174},   //NOLINT
                    {998, 190},   //NOLINT
                    {999, 174}    //NOLINT
            };
            std::vector<std::pair<int, float>> referenceValuesVec;
            referenceValuesVec.assign(std::begin(referenceValuesMap), std::end(referenceValuesMap));

            SECTION("no comments")
            {
                std::for_each(std::begin(referenceValuesMap), std::end(referenceValuesMap),
                              [&heightFile](const std::pair<int, float> & element)
                              {
                                heightFile << element.first << " " << element.second << std::endl;
                              });
                heightFile.close();

                auto ret = IO::readHeightFile(QString::fromStdString(heigtFileName));
                REQUIRE(std::holds_alternative<std::unordered_map<int, float>>(ret));
                std::unordered_map<int, float> markerHeights = std::get<std::unordered_map<int, float>>(ret);
                std::vector<std::pair<int, float>> markerHeightsVec;
                markerHeightsVec.assign(std::begin(markerHeights), std::end(markerHeights));
                CHECK_THAT(markerHeightsVec, Catch::UnorderedEquals(referenceValuesVec));
            }


            SECTION("z-coordinate in cm")
            {
                heightFile << "# id z/cm" << std::endl;
                std::for_each(std::begin(referenceValuesMap), std::end(referenceValuesMap),
                              [&heightFile](const std::pair<int, float> & element)
                              {
                                  heightFile << element.first << " " << element.second << std::endl;
                              });
                heightFile.close();

                auto ret = IO::readHeightFile(QString::fromStdString(heigtFileName));
                REQUIRE(std::holds_alternative<std::unordered_map<int, float>>(ret));
                std::unordered_map<int, float> markerHeights = std::get<std::unordered_map<int, float>>(ret);
                std::vector<std::pair<int, float>> markerHeightsVec;
                markerHeightsVec.assign(std::begin(markerHeights), std::end(markerHeights));
                CHECK_THAT(markerHeightsVec, Catch::UnorderedEquals(referenceValuesVec));
            }

            SECTION("z-coordinate in m")
            {
                heightFile << "# id z/m" << std::endl;
                std::for_each(std::begin(referenceValuesMap), std::end(referenceValuesMap),
                              [&heightFile](const std::pair<int, float> & element)
                              {
                                  heightFile << element.first << " " << element.second/100.F << std::endl;
                              });
                heightFile.close();

                auto ret = IO::readHeightFile(QString::fromStdString(heigtFileName));
                REQUIRE(std::holds_alternative<std::unordered_map<int, float>>(ret));
                std::unordered_map<int, float> markerHeights = std::get<std::unordered_map<int, float>>(ret);
                std::vector<std::pair<int, float>> markerHeightsVec;
                markerHeightsVec.assign(std::begin(markerHeights), std::end(markerHeights));

                CHECK_THAT(markerHeightsVec, Catch::UnorderedEquals(referenceValuesVec));
            }

            SECTION("z-coordinate unit not specified")
            {
                heightFile << "# id z/not_specifies" << std::endl;
                std::for_each(std::begin(referenceValuesMap), std::end(referenceValuesMap),
                              [&heightFile](const std::pair<int, float> & element)
                              {
                                  heightFile << element.first << " " << element.second << std::endl;
                              });
                heightFile.close();

                auto ret = IO::readHeightFile(QString::fromStdString(heigtFileName));
                REQUIRE(std::holds_alternative<std::unordered_map<int, float>>(ret));
                std::unordered_map<int, float> markerHeights = std::get<std::unordered_map<int, float>>(ret);
                std::vector<std::pair<int, float>> markerHeightsVec;
                markerHeightsVec.assign(std::begin(markerHeights), std::end(markerHeights));
                CHECK_THAT(markerHeightsVec, Catch::UnorderedEquals(referenceValuesVec));
            }

            std::remove(heigtFileName.c_str());

        }
    }

    SECTION("readMarkerIDFile")
    {
        SECTION("broken input")
        {
            SECTION("could not open file")
            {
                auto errorMessage = IO::readMarkerIDFile("not_exisiting_file.txt");
                REQUIRE(std::holds_alternative<std::string>(errorMessage));
                std::string errorAsString = std::get<std::string>(errorMessage);
                REQUIRE_THAT(errorAsString, Catch::Matchers::Equals("Could not open not_exisiting_file.txt"));
            }

            SECTION("wrong file extension")
            {
                auto errorMessage = IO::readMarkerIDFile("wrong.extension");
                REQUIRE(std::holds_alternative<std::string>(errorMessage));
                std::string errorAsString = std::get<std::string>(errorMessage);
                REQUIRE_THAT(errorAsString, Catch::Matchers::Equals(
                        "Cannot load wrong.extension maybe because of wrong file extension. Needs to be .txt."));
            }

            SECTION("markerFileName empty")
            {
                auto errorMessage = IO::readMarkerIDFile("");
                REQUIRE(std::holds_alternative<std::string>(errorMessage));
                std::string errorAsString = std::get<std::string>(errorMessage);
                REQUIRE_THAT(errorAsString, Catch::Matchers::Equals("No file provided."));
            }

            std::string markerFileName{"marker.txt"};
            std::ofstream markerFile(markerFileName);
            markerFile << "# id markerID" << std::endl;

            SECTION("Duplicate entry in file")
            {
                markerFile << "1 4356" << std::endl;
                markerFile << "1 4356" << std::endl;
                markerFile.close();

                auto errorMessage = IO::readMarkerIDFile(QString::fromStdString(markerFileName));
                REQUIRE(std::holds_alternative<std::string>(errorMessage));
                std::string errorAsString = std::get<std::string>(errorMessage);
                REQUIRE_THAT(errorAsString, Catch::Matchers::Equals("Duplicate entry for personID = 1."));
            }

            SECTION("Wrong inputs")
            {
                SECTION("personID")
                {
                    SECTION("floating point value")
                    {
                        markerFile << "234.234 4356" << std::endl;
                        markerFile.close();

                        auto errorMessage = IO::readMarkerIDFile(QString::fromStdString(markerFileName));
                        REQUIRE(std::holds_alternative<std::string>(errorMessage));
                        std::string errorAsString = std::get<std::string>(errorMessage);
                        REQUIRE_THAT(errorAsString,
                                     Catch::Matchers::Equals("PersonID needs to be an integer value, but is 234.234"));
                    }

                    SECTION("arbitrary string")
                    {
                        markerFile << "WRONG 4356" << std::endl;
                        markerFile.close();

                        auto errorMessage = IO::readMarkerIDFile(QString::fromStdString(markerFileName));
                        REQUIRE(std::holds_alternative<std::string>(errorMessage));
                        std::string errorAsString = std::get<std::string>(errorMessage);
                        REQUIRE_THAT(errorAsString,
                                     Catch::Matchers::Equals("PersonID needs to be an integer value, but is WRONG"));
                    }
                }

                SECTION("markerID")
                {
                    SECTION("floating point value")
                    {
                        markerFile << "1 43.56" << std::endl;
                        markerFile.close();

                        auto errorMessage = IO::readMarkerIDFile(QString::fromStdString(markerFileName));
                        REQUIRE(std::holds_alternative<std::string>(errorMessage));
                        std::string errorAsString = std::get<std::string>(errorMessage);
                        REQUIRE_THAT(errorAsString,
                                     Catch::Matchers::Equals("MarkerID needs to be an integer value, but is 43.56"));
                    }

                    SECTION("arbitrary string")
                    {
                        markerFile << "1 WRONG" << std::endl;
                        markerFile.close();

                        auto errorMessage = IO::readMarkerIDFile(QString::fromStdString(markerFileName));
                        REQUIRE(std::holds_alternative<std::string>(errorMessage));
                        std::string errorAsString = std::get<std::string>(errorMessage);
                        REQUIRE_THAT(errorAsString,
                                     Catch::Matchers::Equals("MarkerID needs to be an integer value, but is WRONG"));
                    }
                }

                SECTION("too many columns")
                {
                    markerFile << "1 4356 123 WRONG" << std::endl;
                    markerFile.close();

                    auto errorMessage = IO::readMarkerIDFile(QString::fromStdString(markerFileName));
                    REQUIRE(std::holds_alternative<std::string>(errorMessage));
                    std::string errorAsString = std::get<std::string>(errorMessage);
                    REQUIRE_THAT(errorAsString, Catch::Matchers::Equals(
                            "Line should contain exactly 2 values: personID markerID. But it contains 4 entries."));
                }

                SECTION("too few columns")
                {
                    markerFile << "2" << std::endl;
                    markerFile.close();

                    auto errorMessage = IO::readMarkerIDFile(QString::fromStdString(markerFileName));
                    REQUIRE(std::holds_alternative<std::string>(errorMessage));
                    std::string errorAsString = std::get<std::string>(errorMessage);
                    REQUIRE_THAT(errorAsString, Catch::Matchers::Equals(
                            "Line should contain exactly 2 values: personID markerID. But it contains 1 entries."));
                }
            }

            std::remove(markerFileName.c_str());
        }

        SECTION("correct input")
        {
            std::string markerFileName{"marker.txt"};
            std::ofstream markerFile(markerFileName);

            SECTION("file empty")
            {
                markerFile.close();
                auto ret = IO::readMarkerIDFile(QString::fromStdString(markerFileName));
                REQUIRE(std::holds_alternative<std::unordered_map<int, int>>(ret));
                auto markerHeights = std::get<std::unordered_map<int, int>>(ret);
                REQUIRE(markerHeights.empty());
            }

            std::unordered_map<int, int> referenceValuesMap{
                    {1, 987},   //NOLINT
                    {2, 988},   //NOLINT
                    {3, 989},   //NOLINT
                    {4, 990},   //NOLINT
                    {5, 991},   //NOLINT
                    {6, 992},   //NOLINT
                    {7, 993},   //NOLINT
                    {8, 994},   //NOLINT
                    {9, 995},   //NOLINT
                    {10, 996},  //NOLINT
                    {11, 997},  //NOLINT
                    {12, 998},  //NOLINT
                    {14, 999}   //NOLINT
            };
            std::vector<std::pair<int, int>> referenceValuesVec;
            referenceValuesVec.assign(std::begin(referenceValuesMap), std::end(referenceValuesMap));

            SECTION("no comments")
            {
                for (const auto & [personID, markerID] : referenceValuesMap)
                {
                    markerFile << personID << " " << markerID << std::endl;
                }
                markerFile.close();

                auto ret = IO::readMarkerIDFile(QString::fromStdString(markerFileName));
                REQUIRE(std::holds_alternative<std::unordered_map<int, int>>(ret));
                auto markerIDs = std::get<std::unordered_map<int, int>>(ret);
                std::vector<std::pair<int, int>> markerHeightsVec;
                markerHeightsVec.assign(std::begin(markerIDs), std::end(markerIDs));
                CHECK_THAT(markerHeightsVec, Catch::UnorderedEquals(referenceValuesVec));
            }

            SECTION("with comments")
            {
                markerFile << "# this is a comment at the beginning" << std::endl;
                for (const auto & [personID, markerID] : referenceValuesMap)
                {
                    markerFile << personID << " " << markerID << std::endl;
                    markerFile << "# this is a comment in the middle" << std::endl;
                }
                markerFile << "# this is a comment at the end" << std::endl;
                markerFile.close();

                auto ret = IO::readMarkerIDFile(QString::fromStdString(markerFileName));
                REQUIRE(std::holds_alternative<std::unordered_map<int, int>>(ret));
                auto markerIDs = std::get<std::unordered_map<int, int>>(ret);
                std::vector<std::pair<int, int>> markerHeightsVec;
                markerHeightsVec.assign(std::begin(markerIDs), std::end(markerIDs));
                CHECK_THAT(markerHeightsVec, Catch::UnorderedEquals(referenceValuesVec));
            }

            std::remove(markerFileName.c_str());
        }
    }

}