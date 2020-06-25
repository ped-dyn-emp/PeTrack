/*

Class Animation
First setup: Ricardo Martin Brualla, 01.02.2007
Later: Maik Boltes

This class will hold an animation in form of a video or a sequence of images.
In case of a video, you can only get the next frame. If it is a sequence of images,
you can select the frame you want to get.
Appart from that, there is a static function that converts IplImages to QPixmaps, so 
they can be represented in QT.

*/

#ifndef ANIMATION_H
#define ANIMATION_H

#include <QWidget>
#include <QPair>
#include <QPixmap>
#include <QSize>
#include <QImage>
#include <QStringList>
#include <QTime>


#include "opencv2/opencv.hpp"

#include "filter.h"
#include "helper.h"
#include "petrack.h"
#ifdef STEREO
#include "pgrAviFile.h"
#else
#include "stereoAviFile.h"
#endif

#define DEFAULT_FPS 25

class Petrack;

class Animation{

public:

    // Constructor & Destructor
    Animation(QWidget *wParent);
    ~Animation();
 
    // Returns a pointer to the next frame of the animation 
//    IplImage* getNextFrame();
    cv::Mat getNextFrame();
 
    // Returns a pointer to the previous frame of the animation
//    IplImage* getPreviousFrame();
    cv::Mat getPreviousFrame();
 
    // Returns a pointer to the frame at the index index
//    IplImage* getFrameAtIndex(int index);
    cv::Mat getFrameAtIndex(int index);
 
    // Returns a pointer to the frame at the position position
    // positions is a double between 0 and 1 that indicates the position in the animation
//    IplImage* getFrameAtPos(double position);
    cv::Mat getFrameAtPos(double position);

    // Returns a pointer to the current frame 
//    IplImage* getCurrentFrame();
    cv::Mat getCurrentFrame();

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
    int getMaxFrames();
 
    // Returns the index of the current frame  
    int getCurrentFrameNum();

    // Returns the speedup to accelerate the video speed
    int getSpeedup();

    // Sets the sourceIn/Out frame numbers
    void updateSourceInFrameNum(int in=-1);
    void updateSourceOutFrameNum(int out=-1);

    // Returns the sourceIn/Out frame numbers
    int getSourceInFrameNum();
    int getSourceOutFrameNum();
 
    // Returns the filename of the current frame  
    QString getCurrentFileName();
 
    // Returns the FPS of the current animation if it is a video
    double getFPS();

    void setFPS(double fps);
 
    // Returns the size of the original frames (could made bigger after filtering)
    QSize getSize();
 
    // free's all the data in animation
    void free();

    void reset();
 
    bool isVideo();
    bool isStereoVideo();
    bool isImageSequence();
    bool isCameraLiveStream();

#ifndef STEREO_DISABLED
    enum Camera getCamera();
    void setCamera(enum Camera);
#endif
    int getFirstFrameSec();
    int getFirstFrameMicroSec();

    QString getFileBase();
    QFileInfo getFileInfo();

#ifndef STEREO_DISABLED
    // used to get access of both frames only with calibStereoFilter
#ifdef STEREO
    PgrAviFile *getCaptureStereo();
#else
    StereoAviFile *getCaptureStereo();
#endif
#endif

private:

    Petrack *mMainWindow;
    // name, info of the video or sequence
    QString mFileBase;
    QString mFileSuffix;
    QFileInfo mFileInfo;

    // Indicate if the current animation is a video or a photo or a stereo video
    bool mVideo, mImgSeq, mStereo, mCameraLiveStream;

#ifndef STEREO_DISABLED
    // indicates which camera is used for stereo video
    enum Camera mCamera;
 #endif

    // Pointer that will be used by the animation and that will be returned in the public functions 
//    IplImage *mImage;
    cv::Mat mImage;
 
    // Size of the frame 
    QSize mSize;
 
    // Number of frames in the whole animation 
    int mMaxFrames;
 
    // Number of frames per second in video
    double mFps;

    // the time (seconds since 1.1.1970 0 uhr, microseconds (.000001s)) when the first frame was recorded (bumblebee .time file)
    int mFirstSec;
    int mFirstMicroSec;

    // Index of the current opened frame 
    int mCurrentFrame;

    // Speed up to accelerate the video speed
    int mSpeedup;

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
    // Returns a pointer to the frame at index index in the serie 
//    IplImage* getFramePhoto(int index);
    cv::Mat getFramePhoto(int index);

    // Gets Size and Frame number information of the recently open animation
    // It is thought to be called once just at the opening of an animation
    bool getInfoPhoto();

    // Free's the photo series data
    void freePhoto();

    // Sets the sourceIn/Out frame numbers
    void setSourceInFrameNum(int in=-1);
    void setSourceOutFrameNum(int out=-1);

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
#ifndef STEREO_DISABLED
    bool openAnimationStereoVideo(int fileNumber, IplImage* stereoImgLeft, IplImage* stereoImgRight);
#endif
    bool openAnimationStereoVideo(int fileNumber, cv::Mat &stereoImgLeft, cv::Mat &stereoImgRight);

    // like above for the first time with new filename
    bool openAnimationStereoVideo(QString fileName);

    // Implementation of the openAnimation function for videos
    // Opens an animation from a video file 
    bool openAnimationVideo(QString fileName);
 
    // Implementation of getFrameAtIndex for videos
    // Returns a pointer to the frame at index index in the video 
//    IplImage* getFrameVideo(int index);
    cv::Mat getFrameVideo(int index);
 
    // Gets Size and Frame number information of the recently open animation
    // It is thought to be called once just at the opening of an animation
    bool getInfoVideo(QString fileName);

    bool getCameraInfo();
 
    // Free's the video data 
    void freeVideo();


    // Capture structure from OpenCV 3/4
    cv::VideoCapture mVideoCapture;


#ifndef STEREO_DISABLED
    // Capture structure from pgrAviFile for Stereo Videos
#ifdef STEREO
    PgrAviFile *mCaptureStereo;
#else
    StereoAviFile *mCaptureStereo;
#endif
    // stereo image allocated in animation to use space for a sequence of stereo files
//    IplImage* mStereoImgLeft;
//    IplImage* mStereoImgRight;

    cv::Mat mStereoImgLeft;
    cv::Mat mStereoImgRight;

    // A list with all the filenames of the stereo video series
    QStringList mStereoVideoFilesList;
 #endif
};

#endif
