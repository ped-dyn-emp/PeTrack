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

#include "IO.h"
#include <QFile>
#include <QRegularExpression>
#include <QTextStream>

/**
 * @brief Reads individual heights for markerIDs from file.
 *
 * Reads the individual heights for markerIDS from \p heightFileName. The file consists of:<br>
 * - lines with markerID and height separated by a whitespace. MarkerIDs may only occur once! <br>
 * - header line starting with # containing either \"z/cm\" or \"z/m\" to determine the unit. If none is given \"z/cm\"
 *    is taken as default. Only the first occurrence of \"z/cm\" or \"z/m\" is considered. <br>
 * - comment lines starting with # will be skipped <br>
 *
 * Example: <br>
 * # id z/cm <br>
 * 987 184 <br>
 * 988 179 <br>
 * # 993 175.5 <br>
 *
 * @param heightFileName name of the file containing the height information
 * @return map of markerID to height if parsing was successful, error message otherwise
 */
std::variant<std::unordered_map<int, float>, std::string> IO::readHeightFile(const QString& heightFileName)
{
    if (!heightFileName.isEmpty())
    {
        // Import heights from txt-file
        if (heightFileName.right(4) == ".txt")
        {
            QFile heightFile(heightFileName);
            if (!heightFile.open(QIODevice::ReadOnly | QIODevice::Text))
            {
                return "Could not open " + heightFileName.toStdString();
            }

            std::unordered_map<int, float> markerHeights;

            QTextStream in(&heightFile);
            bool readHeader = false;
            float conversionFactorToCM = 1.0F;

            while (!in.atEnd())
            {
                QString line = in.readLine();
                // Process header/comment line
                if( line.startsWith("#",Qt::CaseInsensitive))
                {
                    if (!readHeader)
                    {
                        const QString& headerline = line;
                        if (headerline.contains("z/cm"))
                        {
                            conversionFactorToCM = 1.0F;
                        }
                        else if (headerline.contains("z/m"))
                        {
                            conversionFactorToCM = 100.0F;
                        }
                        readHeader = true;
                    }
                    continue;
                }

                // read line with format: [id height]
                if (auto splitLine = line.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts); splitLine.size() == 2)
                {
                    bool markerConverted = true;
                    int markerID = splitLine[0].toInt(&markerConverted);
                    if (!markerConverted)
                    {
                        return "Marker needs to be an integer value, but is " + splitLine[0].toStdString();
                    }
                    bool heightConverted = true;
                    float height = splitLine[1].toFloat(&heightConverted) * conversionFactorToCM;

                    if (!heightConverted || height <= 0)
                    {
                        return "Height needs to be a positive numerical value, but is " + splitLine[1].toStdString();
                    }

                    if (auto inserted = markerHeights.insert(std::make_pair(markerID, height)); !inserted.second)
                    {
                        return "Duplicate entry for markerID = " + std::to_string(markerID) + ".";
                    }
                }
                else
                {
                    return "Line should contain exactly 2 values: id height. But it contains "
                            + std::to_string(splitLine.size()) + " entries.";
                }
            }
            heightFile.close();
            return markerHeights;
        }

        return "Cannot load " + heightFileName.toStdString()
               + " maybe because of wrong file extension. Needs to be .txt.";
    }
    return "No file provided.";
}

/**
 * @brief Reads the markerIDs of individual personIDs from a file.
 *
 * Reads the markerIDs for personIDs from \p markerFileName. The file consists of:<br>
 * - lines with markerID and height separated by a whitespace. MarkerIDs may only occur once! <br>
 * - comment lines starting with # will be skipped <br>
 *
 * Example: <br>
 * # id markerID <br>
 * 1 995 <br>
 * 2 999 <br>
 * 3 998 <br>
 * # this and the following line will be ignored <br>
 * # 4 	 994 <br>
 *
 * @param markerFileName name of the file containing the markerID information
 * @return map of personID to markerID if parsing was successful, error message otherwise
 */
std::variant<std::unordered_map<int, int>, std::string> IO::readMarkerIDFile(const QString &markerFileName)
{
    if (!markerFileName.isEmpty())
    {
        // Import heights from txt-file
        if (markerFileName.right(4) == ".txt")
        {
            QFile markerFile(markerFileName);
            if (!markerFile.open(QIODevice::ReadOnly | QIODevice::Text))
            {
                return "Could not open " + markerFileName.toStdString();
            }

            std::unordered_map<int, int> markerIDs;

            QTextStream in(&markerFile);

            while (!in.atEnd())
            {
                QString line = in.readLine();

                // Skip header/comment line
                if( line.startsWith("#",Qt::CaseInsensitive))
                {
                    continue;
                }

                // read line with format: [personID markerID]
                if (auto splitLine = line.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts); splitLine.size() == 2)
                {
                    bool personIDConverted = true;
                    int personID = splitLine[0].toInt(&personIDConverted);
                    if (!personIDConverted)
                    {
                        return "PersonID needs to be an integer value, but is " + splitLine[0].toStdString();
                    }
                    bool markerIDConverted = true;
                    int markerID = splitLine[1].toInt(&markerIDConverted);

                    if (!markerIDConverted )
                    {
                        return "MarkerID needs to be an integer value, but is " + splitLine[1].toStdString();
                    }

                    if (auto inserted = markerIDs.insert(std::make_pair(personID, markerID)); !inserted.second)
                    {
                        return "Duplicate entry for personID = " + std::to_string(personID) + ".";
                    }
                }
                else
                {
                    return "Line should contain exactly 2 values: personID markerID. But it contains "
                           + std::to_string(splitLine.size()) + " entries.";
                }
            }
            markerFile.close();
            return markerIDs;
        }

        return "Cannot load " + markerFileName.toStdString()
               + " maybe because of wrong file extension. Needs to be .txt.";
    }
    return "No file provided.";
}
