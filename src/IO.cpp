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

#include "IO.h"

#include "logger.h"
#include "moCapPerson.h"
#include "pMessageBox.h"
#include "skeletonTree.h"
#include "skeletonTreeFactory.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <QTextStream>
#include <opencv2/opencv.hpp>

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
std::variant<std::unordered_map<int, float>, std::string> IO::readHeightFile(const QString &heightFileName)
{
    if(!heightFileName.isEmpty())
    {
        // Import heights from txt-file
        if(heightFileName.endsWith(".txt", Qt::CaseInsensitive))
        {
            QFile heightFile(heightFileName);
            if(!heightFile.open(QIODevice::ReadOnly | QIODevice::Text))
            {
                return "Could not open " + heightFileName.toStdString();
            }

            std::unordered_map<int, float> markerHeights;

            QTextStream in(&heightFile);
            bool        readHeader           = false;
            float       conversionFactorToCM = 1.0F;

            while(!in.atEnd())
            {
                QString line = in.readLine();
                // Process header/comment line
                if(line.startsWith("#", Qt::CaseInsensitive))
                {
                    if(!readHeader)
                    {
                        const QString &headerline = line;
                        if(headerline.contains("z/cm"))
                        {
                            conversionFactorToCM = 1.0F;
                        }
                        else if(headerline.contains("z/m"))
                        {
                            conversionFactorToCM = 100.0F;
                        }
                        readHeader = true;
                    }
                    continue;
                }

                // read line with format: [id height]
                if(auto splitLine = line.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts); splitLine.size() == 2)
                {
                    bool markerConverted = true;
                    int  markerID        = splitLine[0].toInt(&markerConverted);
                    if(!markerConverted)
                    {
                        return "Marker needs to be an integer value, but is " + splitLine[0].toStdString();
                    }
                    bool  heightConverted = true;
                    float height          = splitLine[1].toFloat(&heightConverted) * conversionFactorToCM;

                    if(!heightConverted || height <= 0)
                    {
                        return "Height needs to be a positive numerical value, but is " + splitLine[1].toStdString();
                    }

                    if(auto inserted = markerHeights.insert(std::make_pair(markerID, height)); !inserted.second)
                    {
                        return "Duplicate entry for markerID = " + std::to_string(markerID) + ".";
                    }
                }
                else if(splitLine.empty())
                {
                    // just ignore empty lines
                    continue;
                }
                else
                {
                    return "Line should contain exactly 2 values: id height. But it contains " +
                           std::to_string(splitLine.size()) + " entries.";
                }
            }
            heightFile.close();
            return markerHeights;
        }

        return "Cannot load " + heightFileName.toStdString() +
               " maybe because of wrong file extension. Needs to be .txt.";
    }
    return "No file provided.";
}

/**
 * @brief Delegates the input c3d to the correct method for given system.
 *
 * This method calls the correct IO method for the MoCap system.
 * Currently only XSENS is supported.
 *
 * @param storage mMoCapStorage of petrack
 * @param metadata new MoCapPersonMetadata
 */
void IO::readMoCapC3D(MoCapStorage &storage, const MoCapPersonMetadata &metadata)
{
    MoCapPerson        person;
    const std::string &filename = metadata.getFilepath();
    MoCapSystem        fp       = metadata.getSystem();
    person.setMetadata(metadata);

    ezc3d::c3d c3d;
    try
    {
        c3d = ezc3d::c3d{filename};
    }
    catch(const std::invalid_argument &e)
    {
        std::stringstream ss;
        ss << "Error while reading C3D File " << filename << ": " << e.what() << '\n';
        PCritical(nullptr, "Error: Cannot load C3D File", ss.str().c_str());
        return;
    }

    size_t firstFrame = c3d.header().firstFrame();
    person.setFileTimeOffset(-static_cast<double>(firstFrame) / person.getMetadata().getSamplerate());

    // getImagePoint takes points in cm -> cm as target unit
    const std::string unit             = c3d.parameters().group("POINT").parameter("UNITS").valuesAsString()[0];
    double            conversionFactor = 1e-1; // default, since XSens uses this
    if(unit == "mm")
    {
        conversionFactor = 1e-1;
    }
    else if(unit == "cm")
    {
        conversionFactor = 1.0;
    }
    else if(unit == "m")
    {
        conversionFactor = 100.0;
    }

    const auto c3dToPoint3f = [conversionFactor](const ezc3d::DataNS::Points3dNS::Point &point)
    {
        return cv::Point3f{
                   static_cast<float>(point.x()), static_cast<float>(point.y()), static_cast<float>(point.z())} *
               conversionFactor;
    };

    switch(fp)
    {
        case XSensC3D:
            readSkeletonC3D_XSENS(c3d, person, c3dToPoint3f);
            break;
        case END:
            break; // So clang doesn't say it isn't handled
    }

    storage.addPerson(person);
}

/**
 * @brief Reads a XSens c3d and extracts the skeletons
 *
 * See the official MVN_User_Manual and/or the definition of XSensStruct for details
 * on which points are which.
 *
 * @param c3d[in] c3d-oject corresponding to the XSens c3d-File
 * @param person[out] the person which the skeletons are added to
 * @param c3dToPoint3f[in] function which converts a c3d point to a cv::Point3f in cm
 */
void IO::readSkeletonC3D_XSENS(
    const ezc3d::c3d                                                           &c3d,
    MoCapPerson                                                                &person,
    const std::function<cv::Point3f(const ezc3d::DataNS::Points3dNS::Point &)> &c3dToPoint3f)
{
    const auto &frames = c3d.data().frames();

    for(const auto &frame : frames)
    {
        const auto &points = frame.points().points();
        if(points.size() != 87)
        {
            PCritical(nullptr, "Wrong C3D", "You need a C3D-File with joints for visualization in PeTrack.");
            break;
        }

        XSenseStruct skeletonStruct;
        skeletonStruct.mHipR    = c3dToPoint3f(points[78]);
        skeletonStruct.mHipL    = c3dToPoint3f(points[82]);
        skeletonStruct.mRoot    = c3dToPoint3f(points[0]);
        skeletonStruct.mNeck1   = c3dToPoint3f(points[15]);
        skeletonStruct.mNeck2   = c3dToPoint3f(points[69]);
        skeletonStruct.mHeadTop = c3dToPoint3f(points[16]);
        skeletonStruct.mEarR    = c3dToPoint3f(points[17]);
        skeletonStruct.mEarL    = c3dToPoint3f(points[18]);
        skeletonStruct.mShldrR  = c3dToPoint3f(points[71]);
        skeletonStruct.mShldrL  = c3dToPoint3f(points[75]);
        skeletonStruct.mWristR  = c3dToPoint3f(points[73]);
        skeletonStruct.mElbowR  = c3dToPoint3f(points[72]);
        skeletonStruct.mWristL  = c3dToPoint3f(points[77]);
        skeletonStruct.mElbowL  = c3dToPoint3f(points[76]);
        skeletonStruct.mHandR   = c3dToPoint3f(points[32]);
        skeletonStruct.mHandL   = c3dToPoint3f(points[35]);
        skeletonStruct.mKneeR   = c3dToPoint3f(points[79]);
        skeletonStruct.mKneeL   = c3dToPoint3f(points[83]);
        skeletonStruct.mAnkleR  = c3dToPoint3f(points[80]);
        skeletonStruct.mHeelR   = c3dToPoint3f(points[52]);
        skeletonStruct.mToeR    = c3dToPoint3f(points[57]);
        skeletonStruct.mAnkleL  = c3dToPoint3f(points[84]);
        skeletonStruct.mHeelL   = c3dToPoint3f(points[58]);
        skeletonStruct.mToeL    = c3dToPoint3f(points[63]);

        person.addSkeleton(SkeletonTreeFactory::generateTree(skeletonStruct));
    }
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
    if(!markerFileName.isEmpty())
    {
        // Import heights from txt-file
        if(markerFileName.endsWith(".txt", Qt::CaseInsensitive))
        {
            QFile markerFile(markerFileName);
            if(!markerFile.open(QIODevice::ReadOnly | QIODevice::Text))
            {
                return "Could not open " + markerFileName.toStdString();
            }

            std::unordered_map<int, int> markerIDs;

            QTextStream in(&markerFile);

            while(!in.atEnd())
            {
                QString line = in.readLine();

                // Skip header/comment line
                if(line.startsWith("#", Qt::CaseInsensitive))
                {
                    continue;
                }

                // read line with format: [personID markerID]
                if(auto splitLine = line.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts); splitLine.size() == 2)
                {
                    bool personIDConverted = true;
                    int  personID          = splitLine[0].toInt(&personIDConverted);
                    if(!personIDConverted)
                    {
                        return "PersonID needs to be an integer value, but is " + splitLine[0].toStdString();
                    }
                    bool markerIDConverted = true;
                    int  markerID          = splitLine[1].toInt(&markerIDConverted);

                    if(!markerIDConverted)
                    {
                        return "MarkerID needs to be an integer value, but is " + splitLine[1].toStdString();
                    }

                    if(auto inserted = markerIDs.insert(std::make_pair(personID, markerID)); !inserted.second)
                    {
                        return "Duplicate entry for personID = " + std::to_string(personID) + ".";
                    }
                }
                else if(splitLine.empty())
                {
                    // ignore empty lines
                    continue;
                }
                else
                {
                    return "Line should contain exactly 2 values: personID markerID. But it contains " +
                           std::to_string(splitLine.size()) + " entries.";
                }
            }
            markerFile.close();
            return markerIDs;
        }

        return "Cannot load " + markerFileName.toStdString() +
               " maybe because of wrong file extension. Needs to be .txt.";
    }
    return "No file provided.";
}

/**
 * Opens the list of authors in zenodo metadata format (see
 * https://developers.zenodo.org/#representation) for more details.
 *
 * @param authorsFile zenodo metadata file containing the author information
 * @return list of authors
 */
std::vector<std::string> IO::readAuthors(const QString &authorsFile)
{
    QFile file(authorsFile);
    file.open(QIODevice::ReadOnly);

    QJsonParseError parseError{};
    QJsonDocument   jsonDocument = QJsonDocument::fromJson(file.readAll(), &parseError);

    std::vector<std::string> authors;

    if(parseError.error == QJsonParseError::NoError)
    {
        QJsonObject root     = jsonDocument.object();
        auto        creators = root["creators"].toArray();
        for(auto author : creators)
        {
            QJsonObject node = author.toObject();
            authors.push_back(node["name"].toString().toStdString());
        }
    }
    else
    {
        SPDLOG_ERROR("Could not parse author file: {}", parseError.errorString());
    }

    return authors;
}
