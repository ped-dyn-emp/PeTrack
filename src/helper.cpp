/*
 * PeTrack - Software for tracking pedestrians movement in videos
 * Copyright (C) 2010-2022 Forschungszentrum Jülich GmbH,
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
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "helper.h"

#include <opencv2/opencv.hpp>

QString proFileName; ///< Path to the project (.pet) file; used for saving relative paths via getFileList and
                     ///< getExistingFile
QString commandLineOptionsString = QObject::tr(
    "<p><code>petrack [-help|-?] [[-project] project.pet] </code><br>"
    "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<code>[-sequence imageSequenceOrVideo]</code><br>"
    "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<code>[-autoSave|-autosave "
    "imgFldOrVideo|proj.pet|trackerFile]</code></code><br>"
    "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<code>[-autoTrack|-autotrack trackerFile]</code><br>"
    "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<code>[-autoReadMarkerID|-autoreadmarkerid "
    "markerIdFile]</code><br>"
    "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<code>[-autoReadHeight|-autoreadheight "
    "heightFile]</code><br>"
    "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<code>[-autoPlay|-autoplay trackerFile]</code><br>"
    "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<code>[-autoExportView|-autoexportview "
    "outputFile]</code></p>"
    "<dl><dt><kbd>-help|-?</kbd></dt><dd>shows help information for command line options</dd>"
    "<dt><kbd>-project</kbd></dt><dd>optional option to set project file; otherwise the argument without option flag "
    "is used as project file</dd>"
    "<dt><kbd>-sequence imageSequenceOrVideo</kbd></dt><dd>loads image sequence or video; option overwrites "
    "<kbd>SRC</kbd> attribute in project file</dd>"
    "<dt><kbd>-autoSave|-autosave imgFldOrVideo|proj.pet|trackerFile</kbd></dt><dd>if the argument ends with "
    "<kbd>pet</kbd>, a project file will be written to <kbd>proj.pet</kbd> at the end; if the argument ends with "
    "<kbd>txt,dat </kbd>or<kbd> trav</kbd>, the trajectories will be written in a format according to the suffix of "
    "<kbd>trackerFile</kbd>; otherwise <kbd>imgFldOrVideo</kbd> is the folder to store the image sequence or a name of "
    "a video file for the direct export; in all cases <kbd>PeTrack</kbd> ends after finishing the work</dd>"
    "<dt><kbd>-autoTrack|-autotrack trackerFile</kbd></dt><dd>calculates automatically the trajectories of marked "
    "pedestrians and stores the result to <kbd>trackerFile</kbd></dd>"
    "<dt><kbd>-autoReadMarkerID|-autoreadmarkerid markerFile</kbd></dt><dd> automatically reads the "
    "<kbd>txt-file</kbd> including personID and markerID and applies the markerIDs to the corresponding person. If "
    "-autoTrack is not used, saving trackerFiles using -autoSaveTracker is recommended.</dd>"
    "<dt><kbd>-autoReadHeight|-autoreadheight heightFile</kbd></dt><dd> automatically reads the <kbd>trackerFile</kbd> "
    "including markerID and individual height and applies the heights to the corresponding person</dd>"
    "<dt><kbd>-autoPlay|-autoplay trackerFile</kbd></dt><dd>plays the video or image sequence and stores the "
    "trajectories to <kbd>trackerFile</kbd></dd>"
    "<dt><kbd>-autoExportView|-autoexportview outputFile</kbd></dt><dd>exports the view, e.g., the undistorted video "
    "or the video with trajectories, to <kbd>outputFile</kbd></dd></dl>"
    "<p>Example:<br>To generate trajectories from a single image sequence starting with <kbd>frame000.jpg</kbd>"
    "with settings stored in the project file <kbd>project.pet</kbd>, export tracker file <kbd>trackerFile</kbd>"
    "and exit with saving the project to <kbd>project.pet</kbd> again:</p>"
    "<p><code>petrack.exe -project project.pet -sequence frame000.jpg</code><br>"
    "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<code>-autoTrack trackerFile -autoSave "
    "project.pet</code></p>");


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
        std::cout << "Error: " << channels << " channels are not supported!" << std::endl;
    }
}

#ifndef STEREO_DISABLED
void copyToQImage(QImage *qImg, IplImage *iplImg)
{
    Mat tempMat = cvarrToMat(iplImg);
    copyToQImage(qImg, tempMat);
}
#endif

#ifndef STEREO_DISABLED
void showImg(QLabel *l, IplImage *i)
{
    QImage qImg(i->width, i->height, QImage::Format_RGB32);
    copyToQImage(&qImg, i);
    l->setPixmap(QPixmap::fromImage(qImg));
    l->show();
}
#endif

/**
@brief get roi: copies roi to rect by setting values of roi to correct values inside rect

no copy of data, only new header which allows to access rect
rect wird veraendert, roi nicht

@param[in] img Mat &img
@param[in] const QRect &roi
@param[in] Rect &rect
@return img(rect)
*/
cv::Mat getRoi(cv::Mat &img, const QRect &roi, cv::Rect &rect, bool evenPixelNumber)
{
    rect.x      = roi.x();
    rect.y      = roi.y();
    rect.width  = roi.width();
    rect.height = roi.height();
    if(evenPixelNumber)
    {
        // roi.width and roi.height must be even
        if(rect.width % 2 > 0)
        {
            --rect.width;
        }
        if(rect.height % 2 > 0)
        {
            --rect.height;
        }
    }
    // roi.width and roi.height must be >=0
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
        rect.width -= (rect.x + rect.width - img.cols);
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
        rect.height -= (rect.y + rect.height - img.rows);
    }

    return img(rect);
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
