#ifndef HELPER_H
#define HELPER_H

#include <opencv2/opencv.hpp>
#include "QFileInfo"

extern QString commandLineOptionsString;

// debug Beispiel - keines besonders gut
//qDebug() << "Widget" << this << "at position" << this->pos();
//Q_ASSERT(1);
//dumpObjectTree();
//dumpObjectInfo();
// stefans ansatz steht in svt_system (nicht besser); noetig: #include "imagehlp.h"
// this ausserhalb von objekt nicht verfuegbar: #define debout cout << typeid(this).name() << "." << __func__ << " in " << __FILE__ << " line " << __LINE__ << ": "

// gleiche Variable wie QT benutzt, es gibt auch noch QT_NO_DEBUG and QT_NO_WARNING_OUTPUT
#ifdef QT_NO_DEBUG_OUTPUT
    #define debout //
#else
    #define debout std::cout << __func__ << " in " << __FILE__ << " line " << __LINE__ << ": "
#endif

#include <iostream>

#include <QString>
inline std::ostream& operator<<(std::ostream& s, const QString& t)
{
    s << t.toStdString();
    return s;
}

#define VERSION "0.8"
#define COMPILE_TIME __TIME__
#define COMPILE_DATE __DATE__

#ifndef MIN
#define	MIN(a, b) ((a)<(b)?(a):(b))
#endif
#ifndef MAX
#define	MAX(a, b) ((a)>(b)?(a):(b))
#endif

#ifndef PI
#define PI 3.141592654
#endif

// SHARING MAY ALSO WORK:
//    > IplImage* frame = cvQueryFrame(mCamera);
//    > QImage qtImage = QImage(QSize(frame->width, frame->height), QImage::Format_RGB888);
//    > cvImage = cvCreateImageHeader(cvSize(frame->width, frame->height), 8, 3);
//    > cvImage->imageData = (char *)qtImage.bits();
// Static function that converts an iplImg in a qImg.
// If the images are not the same size, a new qImg will be created with the same size as the iplImg.
#include <QImage>
#ifndef STEREO_DISABLED
#include "cxcore.h"
void copyToQImage(QImage *qImg, IplImage *iplImg); // war static functin in animatioln class
#endif
void copyToQImage(QImage *qImg, cv::Mat &img);

#include <QLabel>
#ifndef STEREO_DISABLED
void showImg(QLabel *l, IplImage *i);
#endif

cv::Mat getRoi(cv::Mat &img, const QRect &roi, cv::Rect &rect, bool evenPixelNumber = true);

inline double getMedianOf3(double a, double b, double c)
{
    if (a<b)
    {
        if (b<c)
            return b; // a b c
        else if (c<b && c<a)
            return a; // c a b
        else
            return c; // a c b
    }
    else // b<=a
    {
        if (a<c)
            return a; // b a c
        else if (c<a && c<b)
            return b; // c b a
        else
            return c; // b c a
    }
}

// // round double to int
// inline int round(double d)
// {
//   return (int) (d<0 ? d-.5 : d+.5);
// }
// da obige Fkt manchmal schon existiert:
// d darf keine seiteneffekte haben!!!
// myround genommen, da round manchmal in math.h existiert, aber nicht immer
#define myRound(d) ((int) ((d)<0 ? (d)-.5 : (d)+.5))
#define myClip(d, min, max) (((d) < min) ? (min) : (((d) > max) ? (max) : (d)))

// get image color from 3-channel-images with values 0..255
#define getR(img,x,y) ((int)*(uchar*)((img)->imageData + (img)->widthStep*(y) + (img)->nChannels*(x) + 2))
#define getG(img,x,y) ((int)*(uchar*)((img)->imageData + (img)->widthStep*(y) + (img)->nChannels*(x) + 1))
#define getB(img,x,y) ((int)*(uchar*)((img)->imageData + (img)->widthStep*(y) + (img)->nChannels*(x)))

//#define getRGB(img,x,y) ((QRgb) QColor(img.at<Vec3b>(y,x)[2],img.at<Vec3b>(y,x)[1],img.at<Vec3b>(y,x)[0]).rgb());
// get image grey value from grey-images with values 0..255 (may be also 3 channels???)
#define getGrey(img,x,y) ((int)*(uchar*)((img)->imageData + (img)->widthStep*(y) + (x)))
//#define getGreyS(img,x,y) ((int) img.at<int>(y,x))

inline cv::Scalar qcolor2scalar(QColor color)
{
    int r,g,b;
    color.getRgb(&r, &g, &b);
    return cv::Scalar(b,g,r); // swap RGB-->BGR
}

inline QColor scalar2qcolor(cv::Scalar color)
{
    QColor ret;
    ret.setHsv(0,0,color[0]);
    return ret; // swap RGB-->BGR
}

inline QColor getValue(const cv::Mat &img ,int x, int y)
{
    QColor ret;
    cv::Scalar scalar;
    cv::Vec3b val;
    switch(img.channels()){
    case 1:
        scalar = img.at<uchar>(cv::Point(x,y));
        ret =  scalar2qcolor(scalar);
        break;
    case 3:
    case 4:
        val = img.at<cv::Vec3b>(cv::Point(x,y));
        ret.setRgb(val.val[2],val.val[1],val.val[0]);
        break;
    default:
        ;
    }
    return ret;
}
#include <QColor>
#include <QTextStream>
// inline QTextStream& operator<<(QTextStream& s, const QColor& col)
// {
//     //    s << "RGB "<< col.red() << " " << col.green() << " " << col.blue() << " / HSV " << col.hue() << " " << col.saturation() << " " << col.value();
//     s << col.hue() << " " << col.saturation() << " " << col.value();
//     return s;
// } 
// inline ostream& operator<<(ostream& s, const QColor& col)
// {
//     s << "RGB "<< col.red() << " " << col.green() << " " << col.blue() << " / HSV " << col.hue() << " " << col.saturation() << " " << col.value();
//     return s;
// }
inline std::ostream& operator<<(std::ostream& s, const QColor& col)
{
    if (col.isValid())
        s << col.red() << " " << col.green() << " " << col.blue();
    else
        s << -1 << " " << -1 << " " << -1;
    return s;
}
inline QTextStream& operator<<(QTextStream& s, const QColor& col)
{
    if (col.isValid())
        s << col.red() << " " << col.green() << " " << col.blue();
    else
        s << -1 << " " << -1 << " " << -1;
    return s;
}
inline QTextStream& operator>>(QTextStream& s, QColor& col)
{
    int i;
    // leave invalid, if one number is -1
    s >> i;
    if (i != -1)
    {
        col.setRed(i);
        s >> i;
        col.setGreen(i);
        s >> i;
        col.setBlue(i);
    }
    else
        s >> i >> i;
    return s;
}

#include <QFile>
#include <QFileInfo>
#include <QString>
#include <QStringList>
// ueberprueft die ;-getrennten Dateinamen auf existenz und gibt ersten zurueck
// interessant fuer relativen und absoluten pfad
inline QString getExistingFile(const QString &fileList, const QString &relToFileName)
{
    QStringList list;
    list = fileList.split(";", QString::SkipEmptyParts);
    for (int i = 0; i < list.size(); ++i)
    {
        if (QFile(list.at(i)).exists())
            return list.at(i);
        if (QFile(list.at(i).trimmed()).exists())
            return list.at(i).trimmed();
        if (QFile(QFileInfo(relToFileName).absolutePath()+"/"+list.at(i).trimmed()).exists())
            return QFileInfo(relToFileName).absolutePath()+"/"+list.at(i).trimmed();
    }
    return ""; // wenn keine der Dateien existiert
}

#include <QFileInfo>
#include <QDir>
inline QString getFileList(const QString &fileName, const QString &relToFileName)
{
    QString seqAbs      = QFileInfo(fileName).absoluteFilePath();
    QString seqRelToPro = QDir(QFileInfo(relToFileName).absolutePath()).relativeFilePath(seqAbs);

    if (QFileInfo(fileName).isRelative())
    {
        if (fileName == seqRelToPro)
            return fileName+";"+seqAbs;
        else
            return fileName+";"+seqAbs+";"+seqRelToPro;
    }
    else
        return fileName+";"+seqRelToPro;
}

#include "time.h"
inline clock_t getElapsedTime()
{
    static clock_t lastTime = clock();
    static clock_t diffTime; // fuer performance
    diffTime = clock()-lastTime;
    lastTime = clock();
    return diffTime;
}

#endif

