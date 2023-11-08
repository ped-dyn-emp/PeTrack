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
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef ANIMATION_H
#define ANIMATION_H

#include <QFileInfo>
#include <QImage>
#include <QPair>
#include <QPixmap>
#include <QSize>
#include <QStringList>
#include <QTime>
#include <QWidget>
#include <opencv2/opencv.hpp>

#ifdef STEREO
#include "pgrAviFile.h"
#else
#include "stereoAviFile.h"
#endif

inline constexpr int DEFAULT_FPS = 25;

class Petrack;

/**
 * @brief The Animation class manages the sequence
 *
 * The Animation class manages the sequence. Currently supported
 * are video files of different formats (via OpenCV/FFMPEG),
 * image sequences and camera livestreams. Every access to metadata
 * (FPS, resolution, etc.) and to the sequene itself is managed by
 * this class.
 */
class Animation
{
public:
    // Constructor & Destructor
    Animation(QWidget *wParent);
    ~Animation();

    // Returns the next frame of the animation
    cv::Mat getNextFrame();

    // Returns the previous frame of the animation
    cv::Mat getPreviousFrame();

    // Returns the frame at the index index
    cv::Mat getFrameAtIndex(int index);

    // Returns the frame at the position position
    // positions is a double between 0 and 1 that indicates the position in the animation
    cv::Mat getFrameAtPos(double position);

    // Returns the current frame
    cv::Mat getCurrentFrame();

    void skipFrame(int num = 1);

    // Return String with current time
    QString getTimeString(int frame = -1);

    // reads the .time file of bumblebee xb3 experiments
    // returns, if timefile could be read
    bool openTimeFile(QString &timeFileName);

    // Opens an animation given the filename of a video or an image.
    // Returns true if the animation was open succesfully
    bool openAnimation(QString fileName);

    // Opens a live stream of the camera with id ID
    bool openCameraStream(int camID);

    // Returns the number of frames in the current animation
    int getNumFrames();

    // Returns the maximum number of frames in the source file
    int getMaxFrames() const;

    // Returns the index of the current frame
    int getCurrentFrameNum() const;

    // Sets the sourceIn/Out frame numbers
    void updateSourceInFrameNum(int in = -1);
    void updateSourceOutFrameNum(int out = -1);

    // Returns the sourceIn/Out frame numbers
    int getSourceInFrameNum() const;
    int getSourceOutFrameNum() const;

    // Returns the filename of the current frame
    QString getCurrentFileName();

    // Returns the FPS of the current animation if it is a video
    double getFPS();
    double getOriginalFPS() const;

    void setFPS(double fps);

    // Returns the size of the original frames (could made bigger after filtering)
    QSize getSize();

    // free's all the data in animation
    void free();

    void reset();

    bool isVideo() const;
    bool isStereoVideo() const;
    bool isImageSequence() const;
    bool isCameraLiveStream() const;

    enum Camera getCamera();
    void        setCamera(enum Camera);
    int         getFirstFrameSec() const;
    int         getFirstFrameMicroSec() const;

    QString   getFileBase();
    QFileInfo getFileInfo();

    // used to get access of both frames only with calibStereoFilter
#ifdef STEREO
    PgrAviFile *getCaptureStereo();
#else
    StereoAviFile *getCaptureStereo();
#endif

private:
    Petrack *mMainWindow;
    // name, info of the video or sequence
    QString   mFileBase;
    QString   mFileSuffix;
    QFileInfo mFileInfo;

    // Indicate if the current animation is a video or a photo or a stereo video
    bool mVideo, mImgSeq, mStereo, mCameraLiveStream;

    // Image that will be used by the animation and that will be returned in the public functions
    cv::Mat mImage;

    // Size of the frame
    QSize mSize;

    // Number of frames in the whole animation
    int mMaxFrames;

    // Number of frames per second in video
    double mFps;
    double mOriginalFps;

    // the time (seconds since 1.1.1970 0 uhr, microseconds (.000001s)) when the first frame was recorded (bumblebee
    // .time file)
    int mFirstSec;
    int mFirstMicroSec;

    // Index of the current opened frame
    int mCurrentFrame;

    // Index of sourceIn/Out frame
    int mSourceInFrame;
    int mSourceOutFrame;

    // file number of stereo files from hermes experiments
    int mCurrentStereoFileNumber;

    // indicates, if time file is loaded inside the open sequence
    bool mTimeFileLoaded;

    /******************************************/
    /***  Sequence of photos implementation ***/
    /******************************************/

    // Methods

    // Implementation of the openAnimation function for photo series
    // Opens an animation made of photos
    bool openAnimationPhoto(QString fileName);

    // Implementation of getFrameAtIndex for photo series
    // Returns the frame at index index in the serie
    cv::Mat getFramePhoto(int index);

    // Gets Size and Frame number information of the recently open animation
    // It is thought to be called once just at the opening of an animation
    bool getInfoPhoto();

    // Free's the photo series data
    void freePhoto();

    // Sets the sourceIn/Out frame numbers
    void setSourceInFrameNum(int in = -1);
    void setSourceOutFrameNum(int out = -1);

    // Variables

    // A list with all the filenames of the series
    QStringList mImgFilesList;

    /******************************************/
    /***  Video implementation              ***/
    /******************************************/

    // Implementation of the openAnimation function for stereo videos
    // Opens an animation from a sequence of stereo video files
    // fileNumber indicates the number of the successive files splited while writing
    // nur bei einer ganz neuen sequenz ist stereoImgBuffer != 0
    bool openAnimationStereoVideo(int fileNumber, cv::Mat &stereoImgLeft, cv::Mat &stereoImgRight);

    // like above for the first time with new filename
    bool openAnimationStereoVideo(QString fileName);

    // Implementation of the openAnimation function for videos
    // Opens an animation from a video file
    bool openAnimationVideo(QString fileName);

    // Implementation of getFrameAtIndex for videos
    // Returns the frame at index index in the video
    cv::Mat getFrameVideo(int index);

    // Gets Size and Frame number information of the recently open animation
    // It is thought to be called once just at the opening of an animation
    bool getInfoVideo(QString fileName);

    bool getCameraInfo();

    // Free's the video data
    void freeVideo();


    // Capture structure from OpenCV 3/4
    cv::VideoCapture mVideoCapture;


    // Capture structure from pgrAviFile for Stereo Videos
#ifdef STEREO
    PgrAviFile *mCaptureStereo;
#else
    StereoAviFile *mCaptureStereo;
#endif
    // stereo image allocated in animation to use space for a sequence of stereo files
    cv::Mat mStereoImgLeft;
    cv::Mat mStereoImgRight;

    // A list with all the filenames of the stereo video series
    QStringList mStereoVideoFilesList;
};

#endif
