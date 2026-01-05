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

#include "helper.h"

#include "logger.h"

#include <algorithm>
#include <opencv2/opencv.hpp>
#include <regex>

QString proFileName; ///< Path to the project (.pet) file; used for saving relative paths via getFileList and
///< getExistingFile
const QString commandLineOptionsString = []() -> QString
{
    const std::vector<std::pair<QString, QString>> options{
        {"-sequence imageSequenceOrVideo",
         "loads image sequence or video; option overwrites <kbd>SRC</kbd> attribute in project file"},
        {"-autoSave|-autosave imgFldOrVideo|proj.pet|trackerFile",
         "if the argument ends with <kbd>pet</kbd>, a project file will be written to <kbd>proj.pet</kbd> at the end; "
         "if the argument ends with <kbd>txt,dat </kbd>or<kbd> trav</kbd>, the trajectories will be written in a "
         "format according to the suffix of <kbd>trackerFile</kbd>; otherwise <kbd>imgFldOrVideo</kbd> is the folder "
         "to store the image sequence or a name of a video file for the direct export; in all cases <kbd>PeTrack</kbd> "
         "ends after finishing the work"},
        {"-autoTrack|-autotrack trackerFile",
         "calculates automatically the trajectories of marked pedestrians and stores the result to "
         "<kbd>trackerFile</kbd>"},
        {"-autoReadMarkerID|-autoreadmarkerid markerIdFile",
         "automatically reads the <kbd>txt-file</kbd> including personID and markerID and applies the markerIDs to the "
         "corresponding person. If -autoTrack is not used, saving trackerFiles using -autoSaveTracker is recommended."},
        {"-autoReadHeight|-autoreadheight heightFile",
         "automatically reads the <kbd>trackerFile</kbd> "
         "including markerID and individual height and applies the heights to the corresponding person"},
        {"-autoPlay|-autoplay trackerFile",
         "plays the video or image sequence and stores the "
         "trajectories to <kbd>trackerFile</kbd>"},
        {"-autoExportView|-autoexportview outputFile",
         "exports the view, e.g., the undistorted video "
         "or the video with trajectories, to <kbd>outputFile</kbd>"},
        {"-autoIntrinsic | -autointrinsic calibDir",
         "performs intrinsic calibration with the files in <kbd>calibDir</kbd>. Saving the pet-file with "
         "<kbd>-autoSave</kbd> is recommended, since else the calculated parameters will be lost."}};

    // help and project are supposed to be on the same line as petrack
    // therefore they are handled separately
    QString petrackCall{"<p><code>petrack [-help|-?] [[-project] project.pet]</code><br>"};
    QString allExplanations{"<dl><dt><kbd>-help|-?</kbd></dt><dd>shows help information for command line "
                            "options</dd><dt><kbd>-project</kbd></dt><dd>optional option to set project file; "
                            "otherwise the argument without option flag is used as project file</dd>"};
    for(const auto &[command, explanation] : options)
    {
        petrackCall +=
            QString{"&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<code>[%1]</code><br>"}.arg(command);

        allExplanations += QString{"<dt><kbd>%1</kbd></dt><dd>%2</dd>"}.arg(command, explanation);
    }
    petrackCall += QString{"</p>"};
    allExplanations += QString{"</dl>"};

    const QString example{
        "<p>Example:<br>To generate trajectories from a single image sequence starting with <kbd>frame000.jpg</kbd>"
        "with settings stored in the project file <kbd>project.pet</kbd>, export tracker file <kbd>trackerFile</kbd>"
        "and exit with saving the project to <kbd>project.pet</kbd> again:</p>"
        "<p><code>petrack.exe -project project.pet -sequence frame000.jpg</code><br>"
        "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<code>-autoTrack trackerFile -autoSave "
        "project.pet</code></p>"};

    return petrackCall + allExplanations + example;
}();


void copyToQImage(QImage &qImg, cv::Mat &img) // war static functin in animatioln class
{
    if(qImg.height() != img.rows || qImg.width() != img.cols) //! qImg || ( or qImg has no object behind pointer
    {
        qImg = QImage(QSize(img.cols, img.rows), QImage::Format_RGB888);
    }

    int channels = img.channels();

    if(channels == 3)
    {
        // Needs Qt 5.14 for QImage::Format_BGR888 (saves the color transformation into RGB888)
        qImg = QImage(
                   (const unsigned char *) (img.data),
                   img.cols,
                   img.rows,
                   static_cast<int>(img.step),
                   QImage::Format_BGR888)
                   .copy();
    }
    else if(channels == 1)
    {
        qImg = QImage(
                   static_cast<const uchar *>(img.data),
                   img.cols,
                   img.rows,
                   static_cast<int>(img.step),
                   QImage::Format_Grayscale8)
                   .copy();
    }
    else
    {
        SPDLOG_ERROR("{} channels are not supported!", channels);
    }
}


/**
 * Create an opencv Rect from a given QRect
 * This method creates an opencv Rect by respecting the given QRect dimensions and a given image matrix (for its maximum
 * size) i. e. if an x or y value is less than zero, the whole rect will be moved such that the value is equal to zero.
 * Correspondingly, if the width or height is greater than the given image, the rectangle will be trimmed to the maximum
 * size.
 *
 *
 * @param roi QRect of which a cv::Rect should be made
 * @param img the image for cutting width and height
 * @param evenPixelNumber if {@code true}, the rect will be cut to the next lesser even width and height.
 * @return a cv::Rect matching the position of the QRect by respecting the given image and evenPixelNumber parameter.
 */
cv::Rect qRectToCvRect(const QRect &roi, const cv::Mat &img, bool evenPixelNumber)
{
    cv::Rect rect(roi.x(), roi.y(), roi.width(), roi.height());

    // trim to image dimensions
    if(rect.x < 0)
    {
        rect.width += rect.x;
        rect.x = 0;
    }
    else if(rect.x > img.cols)
    {
        rect.width = 0;
        rect.x     = img.cols;
    }
    if(rect.x + rect.width > img.cols)
    {
        rect.width = img.cols - rect.x;
    }
    if(rect.y < 0)
    {
        rect.height += rect.y;
        rect.y = 0;
    }
    else if(rect.y > img.rows)
    {
        rect.height = 0;
        rect.y      = img.rows;
    }
    if(rect.y + rect.height > img.rows)
    {
        rect.height = img.rows - rect.y;
    }

    rect.height = std::max(0, rect.height);
    rect.width  = std::max(0, rect.width);

    if(evenPixelNumber)
    {
        rect.width -= rect.width % 2;
        rect.height -= rect.height % 2;
    }

    return rect;
}

#include <iostream>
#include <vector>
/**
 * @brief Compares two PeTrack version strings and returns if the first version string is newer than the second one
 *
 * @param q1: first PeTrack version string
 * @param q2: second PeTrack version string
 * @throws std::invalid_argument Thrown if one of the input strings is not in the right format
 * @return boolean, whether the first version is higher than the second one
 */
bool newerThanVersion(const QString &q1, const QString &q2)
{
    QStringList      version1 = q1.split(QLatin1Char('.'));
    QStringList      version2 = q2.split(QLatin1Char('.'));
    std::vector<int> version1_parts;
    std::vector<int> version2_parts;
    constexpr int    amountOfVersionParts = 3;

    for(const auto &versionPart : version1)
    {
        bool ok;
        int  part = versionPart.toInt(&ok, 10);
        if(ok)
        {
            version1_parts.push_back(part);
        }
        else
        {
            throw std::invalid_argument("Invalid PeTrack version string: Version is non-numeric!");
        }
    }
    for(const auto &versionPart : version2)
    {
        bool ok;
        int  part = versionPart.toInt(&ok, 10);
        if(ok)
        {
            version2_parts.push_back(part);
        }
        else
        {
            throw std::invalid_argument("Invalid PeTrack version string: Version is non-numeric!");
        }
    }
    if(!(version1_parts.size() == amountOfVersionParts && version2_parts.size() == amountOfVersionParts))
    {
        // special case: PATCH can be omissed; is then assumed to be zero
        if(version1_parts.size() == 2)
        {
            version1_parts.push_back(0);
        }
        if(version2_parts.size() == 2)
        {
            version2_parts.push_back(0);
        }

        // check if after addition of patch it is valid
        if(!(version1_parts.size() == amountOfVersionParts && version2_parts.size() == amountOfVersionParts))
        {
            throw std::invalid_argument("Invalid PeTrack version string: Amount of version parts is wrong!");
        }
    }
    for(int i = 0; i < amountOfVersionParts; ++i)
    {
        if(version1_parts[i] > version2_parts[i])
        {
            return true;
        }
        else if(version1_parts[i] < version2_parts[i])
        {
            return false;
        }
    }
    return false;
}

std::set<int> splitCompactString(const std::string &input)
{
    // string should be numbers or ranges separated by comma (and optionally whitespace
    const std::regex inputRegex(R"(\s*(\d+\s*(-\s*\d+)?)(,\s*\d+\s*(-\s*\d+)?)*\s*)");

    if(!std::regex_match(input, inputRegex))
    {
        throw std::invalid_argument("Invalid input string");
    }

    std::set<int> result;
    std::string   noSpace;
    std::remove_copy_if(input.begin(), input.end(), std::back_inserter(noSpace), ::isspace);
    std::stringstream ss(noSpace);
    std::string       segment;

    while(std::getline(ss, segment, ','))
    {
        if(segment.find('-') != std::string::npos)
        {
            std::vector<int>  rangeBounds;
            std::stringstream rangeStream(segment);
            std::string       range;

            while(std::getline(rangeStream, range, '-'))
            {
                rangeBounds.push_back(std::stoi(range));
            }

            if(rangeBounds.size() == 2)
            {
                int start = rangeBounds[0];
                int end   = rangeBounds[1];
                if(end < start)
                {
                    int tmp = start;
                    start   = end;
                    end     = tmp;
                }
                for(int i = start; i <= end; ++i)
                {
                    result.insert(i);
                }
            }
        }
        else
        {
            result.insert(std::stoi(segment));
        }
    }

    return result;
}