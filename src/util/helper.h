/*
 * PeTrack - Software for tracking pedestrians movement in videos
 * Copyright (C) 2024 Forschungszentrum Jülich GmbH, IAS-7
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

#include <QFileInfo>
#include <QRect>
#include <QString>
#include <opencv2/core/types.hpp>
#include <opencv2/opencv.hpp>

class QCheckBox;

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
#include <QImage>
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
#define myRound(d)          (static_cast<int>(((d) < 0 ? (d) -.5 : (d) + .5)))
#define myClip(d, min, max) (((d) < min) ? (min) : (((d) > max) ? (max) : (d)))

// get image color from 3-channel-images with values 0..255
#define getR(img, x, y) ((int) *(uchar *) ((img)->imageData + (img)->widthStep * (y) + (img)->nChannels * (x) + 2))
#define getG(img, x, y) ((int) *(uchar *) ((img)->imageData + (img)->widthStep * (y) + (img)->nChannels * (x) + 1))
#define getB(img, x, y) ((int) *(uchar *) ((img)->imageData + (img)->widthStep * (y) + (img)->nChannels * (x)))

// get image grey value from grey-images with values 0..255 (may be also 3 channels???)
#define getGrey(img, x, y) ((int) *(uchar *) ((img)->imageData + (img)->widthStep * (y) + (x)))

inline cv::Scalar qcolor2scalar(QColor color)
{
    int r, g, b;
    color.getRgb(&r, &g, &b);
    return cv::Scalar(b, g, r); // swap RGB-->BGR
}

inline QColor scalar2qcolor(cv::Scalar color)
{
    QColor ret;
    ret.setHsv(0, 0, color[0]);
    return ret; // swap RGB-->BGR
}

inline QColor getValue(const cv::Mat &img, int x, int y)
{
    QColor     ret;
    cv::Scalar scalar;
    cv::Vec3b  val;
    switch(img.channels())
    {
        case 1:
            scalar = img.at<uchar>(cv::Point(x, y));
            ret    = scalar2qcolor(scalar);
            break;
        case 3:
        case 4:
            val = img.at<cv::Vec3b>(cv::Point(x, y));
            ret.setRgb(val.val[2], val.val[1], val.val[0]);
            break;
        default:;
    }
    return ret;
}
#include <QColor>
#include <QTextStream>
inline std::ostream &operator<<(std::ostream &s, const QColor &col)
{
    if(col.isValid())
    {
        s << col.red() << " " << col.green() << " " << col.blue();
    }
    else
    {
        s << -1 << " " << -1 << " " << -1;
    }
    return s;
}
inline QTextStream &operator<<(QTextStream &s, const QColor &col)
{
    if(col.isValid())
    {
        s << col.red() << " " << col.green() << " " << col.blue();
    }
    else
    {
        s << -1 << " " << -1 << " " << -1;
    }
    return s;
}
inline QTextStream &operator>>(QTextStream &s, QColor &col)
{
    int i;
    // leave invalid, if one number is -1
    s >> i;
    if(i != -1)
    {
        col.setRed(i);
        s >> i;
        col.setGreen(i);
        s >> i;
        col.setBlue(i);
    }
    else
    {
        s >> i >> i;
    }
    return s;
}

#include <QFile>
#include <QFileInfo>
#include <QString>
#include <QStringList>
/**
 * @brief checks the ;-separated file names for existence and returns the first
 *
 * Interesting for working with absolute <em>and</em> relative paths.
 *
 * @param fileList
 * @param relToFileName
 * @return first file to exist
 */
inline QString getExistingFile(const QString &fileList, const QString &relToFileName = proFileName)
{
    QStringList list;
    list = fileList.split(";", Qt::SkipEmptyParts);
    for(int i = 0; i < list.size(); ++i)
    {
        if(auto f = QFileInfo(list.at(i)); f.exists() && f.isFile())
        {
            return list.at(i);
        }
        if(auto f = QFileInfo(list.at(i).trimmed()); f.exists() && f.isFile())
        {
            return list.at(i).trimmed();
        }
        if(auto f = QFileInfo(QFileInfo(relToFileName).absolutePath() + "/" + list.at(i).trimmed());
           f.exists() && f.isFile())
        {
            return QFileInfo(relToFileName).absolutePath() + "/" + list.at(i).trimmed();
        }
    }
    return ""; // wenn keine der Dateien existiert
}

#include <QDir>
#include <QFileInfo>
inline QString getFileList(const QString &file, const QString &originOfRelativePath = proFileName)
{
    QString absolutePathToFileName = QFileInfo(file).absoluteFilePath();
    QString relativePathToFileNameFromPet =
        QDir(QFileInfo(originOfRelativePath).absolutePath()).relativeFilePath(absolutePathToFileName);

    // for a non-existing file we can not create a filelist
    if(!QFileInfo(file).exists())
    {
        return file;
    }

    if(QFileInfo(file).isRelative())
    {
        if(file == relativePathToFileNameFromPet)
        {
            return file + ";" + absolutePathToFileName;
        }
        else // if file relative to working directory
        {
            return file + ";" + absolutePathToFileName + ";" + relativePathToFileNameFromPet;
        }
    }
    else
    {
        return file + ";" + relativePathToFileNameFromPet;
    }
}

#include <ctime>
inline clock_t getElapsedTime()
{
    static clock_t lastTime = clock();
    static clock_t diffTime; // fuer performance
    diffTime = clock() - lastTime;
    lastTime = clock();
    return diffTime;
}
bool newerThanVersion(const QString &q1, const QString &q2);

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

#include <QDomElement>

template <typename T>
void setValue(T widget, double value)
{
    if(widget->minimum() > value || widget->maximum() < value)
    {
        std::stringstream ss;
        ss << "Value " << value << " for " << widget->objectName().toStdString() << " is out of range from "
           << widget->minimum() << " to " << widget->maximum();
        throw std::domain_error(ss.str());
    }
    widget->setValue(value);
}

void setValue(QCheckBox *widget, bool value);

template <typename T>
void loadValue(const QDomElement &elem, const QString &name, T widget)
{
    if(elem.hasAttribute(name))
    {
        setValue(widget, elem.attribute(name).toDouble());
    }
}

void loadValue(const QDomElement &elem, const QString &name, QCheckBox *widget);

template <typename T>
void setValueBlocked(T widget, double value)
{
    if(widget->minimum() > value || widget->maximum() < value)
    {
        std::stringstream ss;
        ss << "Value " << value << " for " << widget->objectName().toStdString() << " is out of range from "
           << widget->minimum() << " to " << widget->maximum();
        throw std::domain_error(ss.str());
    }
    widget->blockSignals(true);
    widget->setValue(value);
    widget->blockSignals(false);
}

void setValueBlocked(QCheckBox *widget, bool value);


template <typename T>
void loadValueBlocked(const QDomElement &elem, const QString &name, T widget)
{
    if(elem.hasAttribute(name))
    {
        QSignalBlocker blocker(widget);
        setValue(widget, elem.attribute(name).toDouble());
    }
}

void loadValueBlocked(const QDomElement &elem, const QString &name, QCheckBox *widget);

template <typename T>
void checkValueValid(T widget, double value)
{
    if(widget->minimum() > value || widget->maximum() < value)
    {
        std::stringstream ss;
        ss << "Value " << value << " for " << widget->objectName().toStdString() << " is out of range from "
           << widget->minimum() << " to " << widget->maximum();
        throw std::domain_error(ss.str());
    }
}
#endif
