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

#ifndef HELPER_H
#define HELPER_H

#include <QString>
#include <opencv2/core/mat.hpp>
#include <opencv2/core/types.hpp>
#include <set>
#include <string>
#include <vector>

class QImage;
class QRect;
class QColor;
class QTextStream;

extern const QString commandLineOptionsString;
extern QString       proFileName; ///< Path to the project (.pet) file; defined in helper.cpp

constexpr const char *file_name(const char *path)
{
    const char *file = path;
    while(*path)
    {
        const char current = *path;
        ++path;
        if(current == '/' || current == '\\')
        {
            file = path;
        }
    }
    return file;
}


#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif
#ifndef MAX
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif

inline constexpr double PI = 3.141592654;

// SHARING MAY ALSO WORK:
//    > IplImage* frame = cvQueryFrame(mCamera);
//    > QImage qtImage = QImage(QSize(frame->width, frame->height), QImage::Format_RGB888);
//    > cvImage = cvCreateImageHeader(cvSize(frame->width, frame->height), 8, 3);
//    > cvImage->imageData = (char *)qtImage.bits();
// Static function that converts an iplImg in a qImg.
// If the images are not the same size, a new qImg will be created with the same size as the iplImg.
void copyToQImage(QImage &qImg, cv::Mat &img);

cv::Rect qRectToCvRect(const QRect &roi, const cv::Mat &img, bool evenPixelNumber = true);
cv::Mat  getRoi(cv::Mat &img, const QRect &roi, cv::Rect &rect, bool evenPixelNumber = true);

inline double getMedianOf3(double a, double b, double c)
{
    if(a < b)
    {
        if(b < c)
        {
            return b; // a b c
        }
        else if(c < b && c < a)
        {
            return a; // c a b
        }
        else
        {
            return c; // a c b
        }
    }
    else // b<=a
    {
        if(a < c)
        {
            return a; // b a c
        }
        else if(c < a && c < b)
        {
            return b; // c b a
        }
        else
        {
            return c; // b c a
        }
    }
}

// d darf keine seiteneffekte haben!!!
// myround genommen, da round manchmal in math.h existiert, aber nicht immer
#define myRound(d)          (static_cast<int>(((d) < 0 ? (d) - .5 : (d) + .5)))
#define myClip(d, min, max) (((d) < min) ? (min) : (((d) > max) ? (max) : (d)))

// get image color from 3-channel-images with values 0..255
#define getR(img, x, y) ((int) *(uchar *) ((img)->imageData + (img)->widthStep * (y) + (img)->nChannels * (x) + 2))
#define getG(img, x, y) ((int) *(uchar *) ((img)->imageData + (img)->widthStep * (y) + (img)->nChannels * (x) + 1))
#define getB(img, x, y) ((int) *(uchar *) ((img)->imageData + (img)->widthStep * (y) + (img)->nChannels * (x)))

// get image grey value from grey-images with values 0..255 (may be also 3 channels???)
#define getGrey(img, x, y) ((int) *(uchar *) ((img)->imageData + (img)->widthStep * (y) + (x)))

cv::Scalar qcolor2scalar(QColor color);

QColor scalar2qcolor(cv::Scalar color);

QColor getValue(const cv::Mat &img, int x, int y);

std::ostream &operator<<(std::ostream &s, const QColor &col);


QTextStream &operator<<(QTextStream &s, const QColor &col);


QTextStream &operator>>(QTextStream &s, QColor &col);

/**
 * @brief checks the ;-separated file names for existence and returns the first
 *
 * Interesting for working with absolute <em>and</em> relative paths.
 *
 * @param fileList
 * @param relToFileName
 * @return first file to exist
 */
QString getExistingFile(const QString &fileList, const QString &relToFileName = proFileName);

QString getFileList(const QString &file, const QString &originOfRelativePath = proFileName);

clock_t getElapsedTime();
bool    newerThanVersion(const QString &q1, const QString &q2);

/**
 * Computes the median of the values in a given vector.
 * @tparam T any sortable type
 * @param data vector for which the median should be computed
 * @return median of all values in data
 */
template <typename T>
T computeMedian(std::vector<T> data)
{
    if((data.size() % 2) != 0U)
    {
        std::nth_element(data.begin(), data.begin() + data.size() / 2, data.end());
        return data[data.size() / 2];
    }

    std::nth_element(data.begin(), data.begin() + data.size() / 2, data.end());
    std::nth_element(data.begin(), data.begin() + (data.size() - 1) / 2, data.end());
    return 0.5 * (data[data.size() / 2] + data[(data.size() - 1) / 2]);
}


/**
 * Utility function to split a compact string of integers (containin intervals written by a-b) into a full list of
 * integers. Spaces are completely ignored
 * @param input The string input e. g. "1, 3-6, 7, 10"
 * @return a list of integers e. g. [1, 3, 4, 5, 6, 7, 10]
 */
std::set<int> splitCompactString(const std::string &input);

#endif
