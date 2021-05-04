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
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

/*

Class Animation
Edited by: Ricardo Martin Brualla, 01.02.2007
           Maik Boltes, 05.02.2007

This class will hold an animation in form of a video or a sequence of images.
In case of a video, you can only get the next frame. If it is a sequence of images,
you can select the frame you want to get.
Appart from that, there is a static function that converts IplImages to QImage, so 
they can be represented in QT.

*/
#include "animation.h"

#include <QWidget>
#include <QSize>
#include <QStringList>
#include <QRegExp>
#include <QFileInfo>
#include <QDir>
#include <QTime>

#include <sstream>
#include <iomanip>

#include <opencv2/opencv.hpp>

#include "pMessageBox.h"
#include "filter.h"
#include "helper.h"
#include "petrack.h"

/**********************************************************************/
/* Constructors & Destructors                                        **/
/**********************************************************************/

Animation::Animation(QWidget *wParent)
{
    mMainWindow = (class Petrack*) wParent;
    mVideo = false;
    mStereo = false;
    mImgSeq = false;
    mCameraLiveStream = false;
    mCurrentFrame = -1;
    mMaxFrames = -1;
    mSourceInFrame = -1;
    mSourceOutFrame = -1;
    mFps = -1;
    mFirstSec = -1;
    mFirstMicroSec = -1;
    ////mCapture = NULL;
#ifndef STEREO_DISABLED
    mCaptureStereo = nullptr;
#endif
    mTimeFileLoaded = false;
    // Set image size
    mSize.setHeight(0);
    mSize.setWidth(0);
}

Animation::~Animation()
{
    if (mImgSeq)
        freePhoto();
    if (mVideo || mCameraLiveStream)
        freeVideo();

    mSourceInFrame = -1;
    mSourceOutFrame = -1;
}

/**********************************************************************/
/* Common Implementation of Video and Sequence of Images             **/
/**********************************************************************/

/// Returns the next frame of the animation
cv::Mat Animation::getNextFrame()
{
    return getFrameAtIndex(mCurrentFrame+1);
}

/// Returns the previous frame of the animation
cv::Mat Animation::getPreviousFrame()
{
    return getFrameAtIndex(mCurrentFrame-1);
}

/// Returns the frame at the index index
cv::Mat Animation::getFrameAtIndex(int index)
{
    if (mCameraLiveStream)
        return getFrameVideo(index);
    // geaendert: We make sure first the index is valid.
    //            If not, it will be set the first or the last index
    if (index<getSourceInFrameNum()){
        return cv::Mat();
    }
    if (index>getSourceOutFrameNum()){
        return cv::Mat();
    }
    // Call the own methods to get it done
    if (mVideo)
        return getFrameVideo(index);
    if (mImgSeq)
        return getFramePhoto(index);
    return cv::Mat();
}

/**
 * @brief Returns the frame at the position position
 *
 * @param position double between 0 and 1 that indicates the position in the animation
 * @return the frame at the indicated position
 */
cv::Mat Animation::getFrameAtPos(double position)
{
    return getFrameAtIndex((int) (getSourceInFrameNum()+(position*(getNumFrames()-1))));
}

/// Returns the current frame
cv::Mat Animation::getCurrentFrame()
{
    return mImage;
}

/**
 * @brief Skips (grabs) the given number of frames (default 1)
 *
 * @param num Number of frames to skip.
 */
void Animation::skipFrame(int num)
{
    if(mVideoCapture.isOpened())
    {
        int lastFrameNum = getSourceOutFrameNum();
        for(int i = 0; i < num && mCurrentFrame < lastFrameNum; ++i)
        {
            mVideoCapture.grab();
            mCurrentFrame += 1;
            mMainWindow->updateShowFPS(true);
        }
    }
}

/**
 * @brief reads the .time file of bumblebee xb3 experiments
 *
 * @param timeFileName
 * @return if timefile could be read
 */
bool Animation::openTimeFile(QString &timeFileName)
{
    QFile file(timeFileName);
    int fcycSec=-1, fcycCount=-1; // fuer erstes frame
    int lframe=-1 , lcycSec=-1, lcycCount=-1; // fuer vorangegangenen frame
    int frame=-1 , cycSec, cycCount, cycOffset, sec, microSec, bufIndex, bufSeqNum, seqNum;
    int dum, add=0;
    double fps, dif, difMin=1000., difMax=-1000.;
    int minFrame=-1, maxFrame=-1;

    debout << "Found corresponding time file " << timeFileName << std::endl;

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        debout << "  Error: Cannot open existing time file " << timeFileName << ":" << std::endl << "  " << file.errorString() << std::endl;
        return false;
    }
    else
    {
        QTextStream in(&file);
        QDateTime dt;
        in >> dum; // einlesen des ersten Wertes eines Zeile, um leerzeilen trotz nicht-Dateiende zu erkennen!
        while (!in.atEnd())
        {
            frame = dum;
            in >> cycSec >> cycCount >> cycOffset >> sec >> microSec >> bufIndex >> bufSeqNum >> seqNum;
            if (frame == 0)
            {
                fcycSec=cycSec; fcycCount=cycCount;
                dt.setTime_t(sec);
                debout << "  Recording starts at " << dt.toString("dd.MM.yyyy hh:mm:ss.");
                std::cout << std::setw(6) << std::setfill('0') << microSec << std::endl;
                mFirstSec = sec;
                mFirstMicroSec = microSec;
            }
            else
            {
                // minimale maximale Zeitabstaende zwischen frames bestimmen - cyc... sind genauere zeiten vom bus
                if (lcycSec > cycSec) // wrap around after 128s
                {
                    dif = (cycSec-lcycSec+128)+(cycCount-lcycCount)/8000.;
                    add += 128;
                }
                else
                    dif = (cycSec-lcycSec)+(cycCount-lcycCount)/8000.;
                if (dif > difMax)
                {
                    difMax = dif;
                    maxFrame = lframe;
                }
                if (dif < difMin)
                {
                    difMin = dif;
                    minFrame = lframe;
                }
            }
            lframe=frame; lcycSec=cycSec; lcycCount=cycCount;
            in >> dum;
        }
        dt.setTime_t(sec);
        fps = frame/((cycSec-fcycSec+add)+(cycCount-fcycCount)/8000.);
        debout << "  Recording ends   at " << dt.toString("dd.MM.yyyy hh:mm:ss."); // entspricht nicht der in statusbar angezeigten zeit des letzten frames, da in statusbar fps und frameanzahl herangezogen wird (genauer)!!!
        std::cout << std::setw(6) << std::setfill('0') << microSec << std::endl;
        debout << "  Fps with "<< frame+1 <<" frames: " << fps << " (min "<<1./difMax<<" at frame "<<maxFrame<<" to "<<maxFrame+1<<", max "<<1./difMin<<" at frame "<<minFrame<<" to "<<minFrame+1<<")" << std::endl;
        setFPS(fps);
        mStereo = true;
        mSourceOutFrame = frame; //mNumFrames = frame+1;

        file.close();
        return true;
    }
}

/// returns -1, if not set by time file
int Animation::getFirstFrameSec() const
{
    return mFirstSec;
}
int Animation::getFirstFrameMicroSec() const
{
    return mFirstMicroSec;
}

/**
 * @brief Get actual time of the given frame
 *
 * @param frame frame for which to determine time; -1 means current frame
 * @return string with real time at given frame
 */
QString Animation::getTimeString(int frame)
{
    if (frame == -1)
        frame = getCurrentFrameNum();

    if (mFirstSec != -1) // stereo video from arena with 16 fps
    {
        QDateTime dt;
        int deziMilliSec = frame%16*625 + mFirstMicroSec/100; // 625 = 10000/16
        dt.setTime_t(mFirstSec + frame/16 + deziMilliSec/10000);
        return (dt.toString("dd.MM.yyyy hh:mm:ss")+".%1").arg(deziMilliSec%10000, 4, 10, QChar('0'));
    }
    else
    {
        int sec = (int) (frame/mFps);
        QTime t(0,0,0);
        t = t.addSecs(sec);
        return (t.toString("hh:mm:ss")+".%1").arg(myRound(((frame/mFps)-sec)*10000), 4, 10, QChar('0'));
    }
}

/**
 * @brief Opens an animation given the filename of a video or an image
 * @param fileName
 * @return true if the animation was open succesfully
 */
bool Animation::openAnimation(QString fileName)
{

    QFileInfo fileInfo(fileName);
    // First of all : does the file exist?
    if (fileInfo.exists())
    {
        bool openRet = false;
        QString timeFileName = fileName.left(fileName.lastIndexOf("cam")+4)+".time";
        QFileInfo fileTimeInfo(timeFileName);
        if (fileTimeInfo.exists())
        {
            mTimeFileLoaded = openTimeFile(timeFileName);
        }
        // Modify it to open other video extensions!
        //if ((fileName.right(4).toLower() == ".avi") ||
        //    (fileName.right(4).toLower() == ".mov") ||
        //    (fileName.right(4).toLower() == ".mts") ||
        //    (fileName.right(5).toLower() == ".m2ts") ||
        //    (fileName.right(4).toLower() == ".wmv") ||
        //    (fileName.right(4).toLower() == ".mp4") )
        //{
        // now all videos will be try to open with OpenCV
        openRet = openAnimationPhoto(fileName);
        //}
        // If it is not a video, then is a photo :-)
        //else
        if (openRet == false) // es konnte keine Bildsequenz geladen werden
            openRet = openAnimationVideo(fileName);
        if (mTimeFileLoaded && !openRet)
            debout << "Warning: New loaded time file do not correspond to untouched sequence, because new sequence was not loadable!" << std::endl;
        mTimeFileLoaded = false; // reset for new open stereo video sequence
        return openRet;
    }
    else 
        return false;
}

/// Opens a camera livestream
bool Animation::openCameraStream(int camID)
{
   if( !mVideoCapture.open(camID) )
   {
            return false;
   }else
   {
        // Destroy anything that was before
        free();
        // Set new video & photo labels
        mStereo = false;
        mVideo = true;
        mImgSeq = false;
        mCameraLiveStream = true;

        mCurrentFrame = -1;
        // Get the information of the animation
        if (!getCameraInfo())
             return false;

   }
   return true;
}

/// Returns the number of frames in the current animation
int Animation::getNumFrames()
{
    if (mVideo || mImgSeq || mStereo)
        return getSourceOutFrameNum()-getSourceInFrameNum()+1;//return mNumFrames;
    return 0;
}

int Animation::getMaxFrames() const
{
    if (mVideo || mImgSeq)
        return mMaxFrames;
    return 0;
}

/// Returns the index of the current frame
int Animation::getCurrentFrameNum() const
{
    return mCurrentFrame;
}
void Animation::updateSourceInFrameNum(int in)
{
    mSourceInFrame = in;
}

void Animation::setSourceInFrameNum(int in)
{
    mSourceInFrame = in;
    mMainWindow->updateSourceInOutFrames();
}
/// Returns the sourceIn frame number
int Animation::getSourceInFrameNum() const
{
    return mSourceInFrame;
}
void Animation::updateSourceOutFrameNum(int out)
{
    mSourceOutFrame = out;
}

void Animation::setSourceOutFrameNum(int out)
{
    mSourceOutFrame = out;
    mMainWindow->updateSourceInOutFrames();
}
/// Returns the sourceOut frame number
int Animation::getSourceOutFrameNum() const
{
    return mSourceOutFrame;
}

/// Returns the filename of the current frame
QString Animation::getCurrentFileName()
{
    if (mCameraLiveStream)
        return QString("camera live stream");

    if (!mImgFilesList.isEmpty() && mImgSeq)
    {
        if(mCurrentFrame < getSourceInFrameNum() || mCurrentFrame > getSourceOutFrameNum())
            return QFileInfo(mImgFilesList.at(getSourceInFrameNum())).fileName();
        else
            return QFileInfo(mImgFilesList.at(mCurrentFrame)).fileName(); // to cut path
    }
    else
        return mFileBase + "." + mFileSuffix; //".avi"; //QString()
}

/// Returns the FPS of the current animation
double Animation::getFPS()
{
    if (mCameraLiveStream)
    {
        if (mVideoCapture.get(cv::CAP_PROP_FPS))
            setFPS(mVideoCapture.get(cv::CAP_PROP_FPS));
    }
    if (mVideo)
        return mFps;
    else if (mImgSeq)
    {
        if (mFps == -1)
            return DEFAULT_FPS; // we assume images from pal video (germany)
        else
            return mFps; // if we found a corresponding .time file for the bumblebee xb3
    }
    else
        return -1;
}

double Animation::getOriginalFPS() const
{
    return mOriginalFps;
}

// -2 shows, that it is called by widget
void Animation::setFPS(double fps)
{
    mFps = fps;
}

/// Returns the size of the original frames (could made bigger after filtering)
QSize Animation::getSize()
{
    if (mVideo || mImgSeq)
        return mSize;
    else
    {
        mSize.setHeight(0);
        mSize.setWidth(0);
        return mSize;
    }
}

/// free's all the data in animation
void Animation::free()
{
    if (mImgSeq)
        freePhoto();
    if (mVideo || mCameraLiveStream)
        freeVideo();

    mSourceInFrame = -1;
    mSourceOutFrame = -1;
}

void Animation::reset()
{
    free();

    mVideo = false;
    mStereo = false;
    mImgSeq = false;
    mCameraLiveStream = false;
    mCurrentFrame = -1;
    mMaxFrames = -1;
    mSourceInFrame = -1;
    mSourceOutFrame = -1;
    mFps = -1;
    mFirstSec = -1;
    mFirstMicroSec = -1;
#ifndef STEREO_DISABLED
    mCaptureStereo = nullptr;
#endif


    mTimeFileLoaded = false;
    // Set image size
    mSize.setHeight(0);
    mSize.setWidth(0);
}

/// filename without sequence number and suffix : image0001-left.png => image-left
QString Animation::getFileBase()
{
    return mFileBase;
}

QFileInfo Animation::getFileInfo()
{
    return mFileInfo;
}

bool Animation::isVideo() const
{
    return mVideo;
}

bool Animation::isStereoVideo() const
{
    return mStereo;
}

bool Animation::isImageSequence() const
{
    return mImgSeq;
}

bool Animation::isCameraLiveStream() const
{
    return mCameraLiveStream;
}

#ifndef STEREO_DISABLED
enum Camera Animation::getCamera()
{
    if (mCaptureStereo != nullptr)
        return mCaptureStereo->getCamera();
    else
        return cameraUnset;
}

void Animation::setCamera(enum Camera c)
{
    if (mCaptureStereo != nullptr)
        return mCaptureStereo->setCamera(c);
    //else   //keine Warnung, damit bei Projekt ohne direkt geladenem Video aber setzen von Stereo-Einstellungen keine Warnung ausgegeben wird
    //    debout << "Warning: Setting camera is only allowed for loaded stereo videos!" << endl;
}
#endif
/**********************************************************************/
/* Sequence of Images implementation                                 **/
/**********************************************************************/

bool Animation::openAnimationPhoto(QString fileName)
{
    // check, if cv can open one animation file

    cv::Mat tempMat;
    tempMat = cv::imread(fileName.toStdString(),cv::IMREAD_UNCHANGED);//, IMREAD_UNCHANGED);//CV_LOAD_IMAGE_UNCHANGED);

    // Check for invalid input
    if(! tempMat.data )
    {
        return false;
    }
    if( tempMat.channels() == 4 ){
        std::cout << "Warning: PNG-Alpha channel will be ignored." << std::endl;
        tempMat = cv::imread(fileName.toStdString(),cv::IMREAD_COLOR);
    }

   // Destroy anything that was before
    free();
    // Set new video & photo labels
    mVideo = false;
    mStereo = false;
    mImgSeq = true;
    mCameraLiveStream = false;

    // Accessing to file information and directory information
    mFileInfo = QFileInfo(fileName);
    QDir dir = mFileInfo.dir();
    // Get all files in the same directory
    QList<QFileInfo> fileList = dir.entryInfoList(QDir::Files,QDir::Name);

    // series1_0002-left => split into series1_|0002|-left
    // regexp is greedy - from left to right try to get the most characters
    QRegExp regExp("(?:[0-9]*)([^0-9]*)$"); //(?: ) zum ignorieren
    QString front, back;
    int frontLen, frontLenList;
    if ((frontLen = regExp.indexIn(mFileInfo.completeBaseName())) > -1)
    {
        front = mFileInfo.completeBaseName().left(frontLen);
        back = regExp.cap(1);
    }
    else // does not match regExp at all
        return false;
    mFileBase = front + back; // completeBaseName to cut suffix and sequence number

    // Create a new image files list 
    mImgFilesList.clear();
    for (int i = 0; i < fileList.size(); i++)
    {
        if ((frontLenList = regExp.indexIn(fileList.at(i).completeBaseName())) > -1)
        {
            // Is the file in the series?
            if ((fileList.at(i).completeBaseName().left(frontLenList) == front) &&
                (regExp.cap(1) == back) &&
                (fileList.at(i).suffix() == mFileInfo.suffix()))
            {
                mImgFilesList << fileList.at(i).filePath();
            }
        }
    }

    // Get the information of the animation
    if (!getInfoPhoto())
        return false;
    // Set the current frame to  -1 (shows, that no frame is already loaded)
    mCurrentFrame = -1;

    return true;
}

/**
 * @brief Implementation of getFrameAtIndex for photo series
 * @param index
 * @return the frame at given index in the series
 */
cv::Mat Animation::getFramePhoto(int index)
{
    if ((index != mCurrentFrame) || mImage.empty())
    {
        // Check if the index is valid
        if(index < getSourceInFrameNum() || index > getSourceOutFrameNum())
            return cv::Mat();

        mImage = cv::imread(mImgFilesList.at(index).toStdString(),cv::IMREAD_UNCHANGED);//CV_LOAD_IMAGE_UNCHANGED);

        // Check for invalid input
        if( mImage.empty() )                              // Check for invalid input
        {
            debout <<  "Could not open or find the image" << std::endl ;
            return cv::Mat();
        }

        if( mImage.channels() == 4 )
        {
            mImage = imread(mImgFilesList.at(index).toStdString(),cv::IMREAD_COLOR);
        }
        // Check image size of each frame
        if( (mSize.width() > 0 && mSize.height() > 0) && (mImage.cols != mSize.width() || mImage.rows != mSize.height()) )
        {
            debout << "Could not load image: image size differs in image sequence" << std::endl;
            debout << "Please ensure to have images of the same size!" << std::endl;

            PCritical(nullptr,"An error has occurred!",
                                  QString("The size of the images in the selected image sequence does not agree."
                                          "<br /><br />[0]:  %1  (%2x%3 pixel)<br />[%4]:   %5  (%6x%7 pixel)")
                                  .arg(mImgFilesList.at(0)).arg(mSize.width()).arg(mSize.height()).arg(index)
                                  .arg(mImgFilesList.at(index)).arg(mImage.cols).arg(mImage.rows));

            return cv::Mat();
        }

        // We set the current frame index
        mCurrentFrame = index;
    }
    return mImage;
}


/**
 * @brief Gets Size and Frame number information of the recently open animation
 *
 * This methods gets size and length in frames of the current animation. It
 * should be called just once. Afterwards the saved values can be used.
 *
 * @return If data could be succesfully read
 */
bool Animation::getInfoPhoto()
{
    bool rc = false;
    cv::Mat tempImg;
    // Set the number of frames
    mMaxFrames = mImgFilesList.size();
    setSourceInFrameNum(0);
    setSourceOutFrameNum(mMaxFrames-1);
    // Set the size of the frames
    // For that we need to retrieve a frame!
    tempImg = getFramePhoto(getSourceInFrameNum());
    if (!tempImg.empty()) // if frame is loadable
    {
        // 1 or 3 channel and 8 bit per pixel
        if (!(((tempImg.channels() == 1) || (tempImg.channels() == 3)) && (tempImg.depth() == CV_8U)))
        {
            debout << "Error: Only 1 or 3 channels (you are using "<< tempImg.channels() <<") and 8 bpp (you are using "<< tempImg.depth() <<") are supported!" << std::endl;
        }
        else
        {
            mSize.setHeight(tempImg.rows);
            mSize.setWidth(tempImg.cols);
            rc = true;
        }
    }
    return rc;
}


void Animation::freePhoto()
{
    // Release the image pointer
    if (!mImage.empty())
    {
        mImage = cv::Mat();
    }
    // Clear the list of filenames in the serie
    mImgFilesList.clear();
    // Reset size
    mSize.setHeight(0);
    mSize.setWidth(0);
}

/**********************************************************************/
/* Video implementation                                              **/
/**********************************************************************/

#ifndef STEREO_DISABLED
// used to get access of both frames only with calibStereoFilter
#ifdef STEREO
PgrAviFile *Animation::getCaptureStereo()
#else
StereoAviFile *Animation::getCaptureStereo()
#endif
{
    return mCaptureStereo;
}
#endif

// also cvcamPlayAVI() is available!!!!!
#ifndef STEREO_DISABLED
// Implementation of the openAnimation function for videos
// Opens an animation from a sequence of stereo video file
// Stereo data from Arena/Messe (hermes) experiments
// fileNumber indicates the number of the successive files splited while writing
bool Animation::openAnimationStereoVideo(int fileNumber, IplImage *stereoImgLeft, IplImage *stereoImgRight)
{
#ifdef STEREO
    PgrAviFile *captureStereo = new PgrAviFile;
#else
    StereoAviFile *captureStereo = new StereoAviFile;
#endif

    if ((fileNumber < mStereoVideoFilesList.length()) &&
        (captureStereo->open(mStereoVideoFilesList[fileNumber].toStdString().c_str(), stereoImgLeft, stereoImgRight)))
    {
        // wird nun schon vorher abgefragt: vor mTimeFileLoaded war mFps == 16 because time file must be loaded before ; && (myRound(mFps) == 16)
        if (!((captureStereo->m_iRows == 960) && (captureStereo->m_iCols == 1280) && (captureStereo->m_iBPP == 16)))
        {
            debout << "Error: Only stereo videos from Hermes experiments with 1280x960 pixel, 16 bits per pixel anf 16 frames per second are supported!" << endl;
            delete captureStereo;
            return false;
        }
        if (mCaptureStereo)
        {
            captureStereo->setCamera(mCaptureStereo->getCamera());
            mCaptureStereo->close();
            delete mCaptureStereo;
        }
        mCurrentStereoFileNumber = fileNumber;
        mCaptureStereo = captureStereo;
        return true;
    }
    else
    {
        delete captureStereo;
        return false;
    }
}
bool Animation::openAnimationStereoVideo(int fileNumber, Mat &stereoImgLeft, Mat &stereoImgRight)
{
    IplImage *tempStereoImgLeft, *tempStereoImgRight;
    Size size;
    size.width = stereoImgLeft.cols;
    size.height = stereoImgRight.rows;
    tempStereoImgLeft  = cvCreateImage(size, 8, 1); // new unsigned char[2*1280*960]; // only stereo from bumblebee xb3 supported!
    tempStereoImgRight = cvCreateImage(size, 8, 1); // new unsigned char[2*1280*960]; // only stereo from bumblebee xb3 supported!
//        tempStereoImgLeft.create(size,CV_8UC1);
//        tempStereoImgRight.create(size,CV_8UC1);
    bool ret = openAnimationStereoVideo(fileNumber, tempStereoImgLeft, tempStereoImgRight);

    if(ret)
    {

        mStereoImgLeft  = cvarrToMat(tempStereoImgLeft);
        mStereoImgRight = cvarrToMat(tempStereoImgRight);

    }else
    {
        cvReleaseImage(&tempStereoImgLeft);
        cvReleaseImage(&tempStereoImgRight);
    }


    return ret;
}
#endif
#ifndef STEREO_DISABLED
// like above for the first time with new filename
bool Animation::openAnimationStereoVideo(QString fileName)
{
    bool ret = false;
//    IplImage *tempStereoImgLeft, *tempStereoImgRight;
//    Mat tempStereoImgLeft,tempStereoImgRight;
    // Accessing to file information and directory information
    QFileInfo fileInfo(fileName);
    QDir dir = fileInfo.dir();
    QStringList filters;
    //QString firstFileName = (fileName.left(fileName.lastIndexOf("cam")+5) + "%1.avi").arg(fileNumber, 4, 10, QChar('0'));
    filters << (fileInfo.fileName()).left((fileInfo.fileName()).lastIndexOf("cam")+5) + "????.avi"; //"*.cpp" << "*.cxx" << "*.cc";
    //dir.setNameFilters(filters);
    QStringList tmpList = mStereoVideoFilesList;
    mStereoVideoFilesList = dir.entryList(filters);
    for (int i = 0; i < mStereoVideoFilesList.size(); i++)
        mStereoVideoFilesList[i] = fileInfo.path() + "/" + mStereoVideoFilesList[i];

    //debout << "getNumFrames: " << getNumFrames() << " mStereoVideoFilesList.len: " << mStereoVideoFilesList.length() << endl;
    if (((getNumFrames()-1/*mNumFrames-1*/) / 640) == (mStereoVideoFilesList.length()-1))
    {
        Size size;
        size.width = 1280;
        size.height = 960;
        mStereoImgLeft.create(size,CV_8UC1);
        mStereoImgRight.create(size,CV_8UC1);
//        tempStereoImgLeft  = cvCreateImage(size, 8, 1); // new unsigned char[2*1280*960]; // only stereo from bumblebee xb3 supported!
//        tempStereoImgRight = cvCreateImage(size, 8, 1); // new unsigned char[2*1280*960]; // only stereo from bumblebee xb3 supported!
//        tempStereoImgLeft.create(size,CV_8UC1);
//        tempStereoImgRight.create(size,CV_8UC1);
        ret = openAnimationStereoVideo(0, mStereoImgLeft, mStereoImgRight);
    }
    //debout << " ret: " << ret << endl;
    if (ret)
    {
//        if (mStereoImgLeft)
//        {
//            cvReleaseImage(&mStereoImgLeft);
//            mStereoImgLeft = NULL;
//        }
//        if (mStereoImgRight)
//        {
//            cvReleaseImage(&mStereoImgRight);
//            mStereoImgRight = NULL;
//        }
//        mStereoImgLeft  = cvarrToMat(tempStereoImgLeft);
//        mStereoImgRight = cvarrToMat(tempStereoImgRight);

    }
    else
    {
        mStereoVideoFilesList = tmpList;
//        tempStereoImgLeft = NULL;
//        tempStereoImgRight = NULL;
    }

    return ret;
}
#endif

/// Implementation of the openAnimation function for videos
/// Opens an animation from a video file
bool Animation::openAnimationVideo(QString fileName)
{
    if( !mVideoCapture.open(fileName.toStdString().c_str()) )
        return false;

    if(mVideoCapture.isOpened())
    {
        int width  = (int) mVideoCapture.get(cv::CAP_PROP_FRAME_WIDTH);
        int height = (int) mVideoCapture.get(cv::CAP_PROP_FRAME_HEIGHT);
        int fps    = (int) mVideoCapture.get(cv::CAP_PROP_FPS);
        if ((width == 1280) && (height == 960) && (fps == 16)) // dann gehe ich von einem stereo video der BBX3 aus!!!
        {
            mVideoCapture.release();
            mStereo = true;
        }
    }
    // Check if it was created succesfully
    if(!mVideoCapture.isOpened())
    {
#if  STEREO && not STEREO_DISABLED


         // untersuchen, ob Stereodaten von Arena/Messe-Versuchen
        if (mTimeFileLoaded && openAnimationStereoVideo(fileName))
        {
            // Destroy anything that was before
            //free();

            // Set new video & photo labels

            mVideo = true;
            mImgSeq = false;
            mCameraLiveStream = false;
            //mCurrentFrame = -1;
            // Get the information of the animation
//            if (!getInfoVideo(fileName))
//                return false;

            // Set the size in the QSize structure
            mSize.setHeight(mCaptureStereo->m_iRows);
            mSize.setWidth(mCaptureStereo->m_iCols);
            // mFps = mCaptureStereo->m_frameRate; == 0!!!!
        }
        else
            return false;
#else
        debout << "Stereo videos temporary not supported!" << std::endl;
        return false;

#endif
    }
    else
    {
        // Destroy anything that was before
        free();
#ifndef STEREO_DISABLED
        if (mCaptureStereo)
        {
            mCaptureStereo->close();
            delete mCaptureStereo;
        }
#endif
        // Set new video & photo labels
        mStereo = false;
        mVideo = true;
        mImgSeq = false;
        mCameraLiveStream = false;

        mCurrentFrame = -1;
        // Get the information of the animation
        if (!getInfoVideo(fileName))
            return false;
        // Set the current frame to -1 (shows, that no frame is already loaded)
    }

    mFileInfo = QFileInfo(fileName);
    mFileBase = mFileInfo.completeBaseName();
    mFileSuffix = mFileInfo.suffix();
    mCurrentFrame = -1; // Set the current frame to -1 (shows, that no frame is already loaded)

    return true;
}

/// Implementation of getFrameAtIndex for videos
/// Returns the frame at given index in the video
cv::Mat Animation::getFrameVideo(int index)
{

    if (mCameraLiveStream)
    {
        if (mVideoCapture.read(mImage/*tempMat*/) )
        {
            if (mImage.empty())//tempImg == NULL)
                return cv::Mat();

            mCurrentFrame++;
        }
        return mImage;
    }

    if ((index != mCurrentFrame) || mImage.empty())
    {
        // Check if we have a valid index
        if (index < getSourceInFrameNum() || (index > getSourceOutFrameNum() && getSourceOutFrameNum() != -1) )
            return cv::Mat();
        if (mVideo && !mStereo)
        {
            // Check if we have a valid capture device
            if (!mVideoCapture.isOpened())
                return cv::Mat();
            // Now we need to see if it is necessary to seek for the frame or if we can use
            // directly the cvQueryFrame function
            // This is tested since the seek function takes a lot of time!
            if (index == getSourceInFrameNum() || mCurrentFrame+1 != index)
            {
                if( !mVideoCapture.set(cv::CAP_PROP_POS_FRAMES, index) )
                {
                    debout << "Error: video file does not support skipping." << std::endl;
                    return cv::Mat();
                }
            }
            // Query the frame
            if( mVideoCapture.read(mImage))
            {
                if (mImage.empty())//tempImg == NULL)
                    return cv::Mat();
            }else
            {
                debout << "Warning: number of frames in the video seems to be incorrect. Frame[" << index << "] is not loadable! Set number of Frames to " << index << "." << std::endl;
                mSourceOutFrame = index-1;
                setSourceOutFrameNum(mSourceOutFrame);
                return cv::Mat();
            }
        }
#ifndef STEREO_DISABLED
        else if (mStereo && mCaptureStereo)// stereo video
        {
            if (index/640 != mCurrentStereoFileNumber) // 640 stereo frames in one file
            {
                openAnimationStereoVideo(index/640, mStereoImgLeft, mStereoImgRight);
            }
            mImage = cvarrToMat(mCaptureStereo->readFrame(index - mCurrentStereoFileNumber * 640));
        }
#endif
        // We set the current frame index
        mCurrentFrame = index;
    }
#ifndef STEREO_DISABLED
    else if (mStereo && (index == mCurrentFrame)) // da mgl anderes Bild rechte/links angefordert wird
        mImage = cvarrToMat(mCaptureStereo->readFrame(index - mCurrentStereoFileNumber * 640));
#endif
    // Return the pointer to the IplImage :-)
    return mImage;
}

/// Gets Size and Frame number information of the recently open animation
/// It is thought to be called once just at the opening of an animation
bool Animation::getInfoVideo(QString /*fileName*/)
{
    // Set the size of the frames
    // We will grab the information from a frame of the animation
    setSourceInFrameNum(0);
    cv::Mat tempImg = getFrameVideo(getSourceInFrameNum());
    if (tempImg.empty())
    {
        debout << "Error: No frame could be retrieved from the capture during getInfoVideo." << std::endl;
        return false;
    }
    // 1 or 3 channel and 8 bit per pixel
    if (!(((tempImg.channels() == 1) || (tempImg.channels() == 3)) && (tempImg.depth() == CV_8U)))
    {
        debout << "Warning: Only 1 or 3 channels (you are using "<< tempImg.channels() <<") and 8 bpp (you are using "<< tempImg.depth() <<") are supported!" << std::endl;
    }
    // Set the size in the QSize structure
    mSize.setHeight(tempImg.rows);
    mSize.setWidth(tempImg.cols);
    // Release the temporary frame
    // We get the FPS number with the cvGetCaptureProperty function
    // Note that this doesn't work if no frame has already retrieved!!
    if (mVideoCapture.get(cv::CAP_PROP_FPS)){
        setFPS(mVideoCapture.get(cv::CAP_PROP_FPS));
        mOriginalFps = mVideoCapture.get(cv::CAP_PROP_FPS);
    }
    // detect the used video codec
    int fourCC = static_cast<int>( mVideoCapture.get(cv::CAP_PROP_FOURCC) );
    char FOURCC[] = {(char)( fourCC & 0XFF) ,
                     (char)((fourCC & 0XFF00) >> 8),
                     (char)((fourCC & 0XFF0000) >> 16),
                     (char)((fourCC & 0XFF000000) >> 24),
                                                        0};
    mMaxFrames = (int) mVideoCapture.get(cv::CAP_PROP_FRAME_COUNT); //mNumFrame
    mSourceOutFrame = mMaxFrames-1;

    // Set videocapture to the last frame if CV_CAP_PROP_POS_FRAMES is supported by used video codec
    if( mVideoCapture.set(cv::CAP_PROP_POS_FRAMES, mMaxFrames) > 0 )
    {
        // Set videoCapture to the really correct last frame
        mVideoCapture.set(cv::CAP_PROP_POS_FRAMES, mVideoCapture.get(cv::CAP_PROP_POS_FRAMES)-1);


        // Check if mNumFrames agrees with last readable frame
        if( mMaxFrames != (mVideoCapture.get(cv::CAP_PROP_POS_FRAMES)+1) )
        {
            debout << "Warning: number of frames detected by OpenCV library (" << mMaxFrames << ") seems to be incorrect! (set to estimated value: "<< (mVideoCapture.get(cv::CAP_PROP_POS_FRAMES)+1) << ") [video codec: " << FOURCC << "]" << std::endl;
            mMaxFrames = mVideoCapture.get(cv::CAP_PROP_POS_FRAMES)+1;
            mSourceOutFrame = mMaxFrames-1;
        }

        // Check if the last frame of the video is readable/OK?
        cv::Mat frame;
        while( !mVideoCapture.read(frame) ){
            mVideoCapture.set(cv::CAP_PROP_POS_FRAMES,mVideoCapture.get(cv::CAP_PROP_POS_FRAMES)-2);
            mMaxFrames = mVideoCapture.get(cv::CAP_PROP_POS_FRAMES)+1;
            if( mVideoCapture.get(cv::CAP_PROP_POS_FRAMES) < 0 ){
                std::cerr << "Warning: video file seems to be broken!" << std::endl;
                mMaxFrames = -1;
                mSourceOutFrame = mMaxFrames-1;
                return false;
            }
        }
    }

    // Since we don't trust OpenCV, another check is never enough :-)
    int defaultFrames = 10000;
    int maxFrames = 9999999; // > 111 h @ 25fps
    if (getNumFrames() <= 0)
    {
        debout << "Error: Incorrect number ("<<getNumFrames()<<") of frames. Setting it to "<<defaultFrames<<"." << std::endl;
        mSourceOutFrame = getSourceInFrameNum()+defaultFrames;
    }
    if (getNumFrames() > maxFrames)// = 6h * 60mins * 60 secs * 25 frames
    {
        debout << "Warning: Number of frames ("<<getNumFrames()<<") seems to be incorrect. Setting it to "<<defaultFrames<<"." << std::endl;
        mSourceOutFrame = getSourceInFrameNum()+defaultFrames;
    }
    setSourceOutFrameNum(mSourceOutFrame);
    return true;
}

bool Animation::getCameraInfo()
{
    cv::Mat tempImg = getFrameVideo(0);
    if (tempImg.empty())
    {
        debout << "Error: No frame could be retrieved from the capture during getInfoVideo." << std::endl;
        return false;
    }
    // 1 or 3 channel and 8 bit per pixel
    if (!(((tempImg.channels() == 1) || (tempImg.channels() == 3)) && (tempImg.depth() == CV_8U)))
    {
        debout << "Warning: Only 1 or 3 channels (you are using "<< tempImg.channels() <<") and 8 bpp (you are using "<< tempImg.depth() <<") are supported!" << std::endl;
    }
    // Set the size in the QSize structure
    mSize.setHeight(tempImg.rows);
    mSize.setWidth(tempImg.cols);
    // Release the temporary frame
    // We get the FPS number with the cvGetCaptureProperty function
    // Note that this doesn't work if no frame has already retrieved!!
    if (mVideoCapture.get(cv::CAP_PROP_FPS)){
        setFPS(mVideoCapture.get(cv::CAP_PROP_FPS));
        mOriginalFps = mVideoCapture.get(cv::CAP_PROP_FPS);
    }



    return true;
}

/// Free's the video data
void Animation::freeVideo()
{
    // Release the capture device
    if (mVideoCapture.isOpened())
    {
        mVideoCapture.release();
    }
    // Release the image pointer
    if (!mImage.empty())
    {
        mImage = cv::Mat();
    }

}
