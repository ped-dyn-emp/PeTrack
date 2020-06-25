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

#include <QWidget>
#include <QPair>
#include <QPixmap>
#include <QSize>
#include <QImage>
#include <QStringList>
#include <QRegExp>
#include <QFileInfo>
#include <QDir>
#include <QList>
#include <QTime>
#include <QThread>
#include <QMessageBox>

#include <string>
#include <fstream>
#include <sstream>
#include <cstring>
#include <cstdlib>
#include <iomanip>


#include "opencv2/opencv.hpp"

#include "animation.h"

using namespace::cv;
using namespace std;

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
    mSpeedup = 1;
    mMaxFrames = -1;
    mSourceInFrame = -1;
    mSourceOutFrame = -1;
    mFps = -1;
    mFirstSec = -1;
    mFirstMicroSec = -1;
    ////mCapture = NULL;
#ifndef STEREO_DISABLED
    mCaptureStereo = NULL;
#endif
//    mImage = NULL;

//    mStereoImgLeft = NULL;
//    mStereoImgRight = NULL;


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

// Returns a pointer to the next frame of the animation 
Mat Animation::getNextFrame()
{
    return getFrameAtIndex(mCurrentFrame+mSpeedup);
}

// Returns a pointer to the previous frame of the animation
Mat Animation::getPreviousFrame()
{
    return getFrameAtIndex(mCurrentFrame-mSpeedup);
}

// Returns a pointer to the frame at the index index
Mat Animation::getFrameAtIndex(int index)
{
    if (mCameraLiveStream)
        return getFrameVideo(index);
    // geaendert: We make sure first the index is valid.
    //            If not, it will be set the first or the last index
    if (index<getSourceInFrameNum()){
        return Mat();
        //index=0;
    }
    if (index>getSourceOutFrameNum()){
        return Mat();
        //index = getNumFrames()-1;
    }
    // Call the own methods to get it done
    if (mVideo)
        return getFrameVideo(index);
    if (mImgSeq)
        return getFramePhoto(index);
    return Mat();
}

// Returns a pointer to the frame at the position position
// positions is a double between 0 and 1 that indicates the position in the animation
Mat Animation::getFrameAtPos(double position)
{
    return getFrameAtIndex((int) (getSourceInFrameNum()+(position*(getNumFrames()-1))));
}

// Returns a pointer to the current frame 
Mat Animation::getCurrentFrame()
{
    return mImage;
}
// Returns the speedup to accelerate the video speed
int Animation::getSpeedup()
{
    return mSpeedup;
}

// reads the .time file of bumblebee xb3 experiments
// returns, if timefile could be read
bool Animation::openTimeFile(QString &timeFileName)
{
    QFile file(timeFileName);
    int fframe , fcycSec=-1, fcycCount=-1, fcycOffset, fsec=-1, fmicroSec=-1, fbufIndex, fbufSeqNum, fseqNum; // fuer erstes frame
    int lframe=-1 , lcycSec=-1, lcycCount=-1, lcycOffset, lsec=-1, lmicroSec=-1, lbufIndex, lbufSeqNum, lseqNum; // fuer vorangegangenen frame
    int frame=-1 , cycSec, cycCount, cycOffset, sec, microSec, bufIndex, bufSeqNum, seqNum;
    int dum, add=0;
    double fps, dif, difMin=1000., difMax=-1000.;
    int minFrame=-1, maxFrame=-1;

    debout << "Found corresponding time file " << timeFileName << endl;

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        debout << "  Error: Cannot open existing time file " << timeFileName << ":" << endl << "  " << file.errorString() << endl;
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
                fframe=frame; fcycSec=cycSec; fcycCount=cycCount; fcycOffset=cycOffset; fsec=sec; fmicroSec=microSec; fbufIndex=bufIndex; fbufSeqNum=bufSeqNum; fseqNum=seqNum;
                dt.setTime_t(sec);
                debout << "  Recording starts at " << dt.toString("dd.MM.yyyy hh:mm:ss.");
                cout << setw(6) << setfill('0') << microSec << endl;
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
            lframe=frame; lcycSec=cycSec; lcycCount=cycCount; lcycOffset=cycOffset; lsec=sec; lmicroSec=microSec; lbufIndex=bufIndex; lbufSeqNum=bufSeqNum; lseqNum=seqNum;
            in >> dum;
        }
        dt.setTime_t(sec);
        //fps = frame/((sec-fsec)+0.000001*microSec-0.000001*fmicroSec); // nicht frame+1 !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        fps = frame/((cycSec-fcycSec+add)+(cycCount-fcycCount)/8000.); //((sec-fsec)+0.000001*microSec-0.000001*fmicroSec);
        debout << "  Recording ends   at " << dt.toString("dd.MM.yyyy hh:mm:ss."); // entspricht nicht der in statusbar angezeigten zeit des letzten frames, da in statusbar fps und frameanzahl herangezogen wird (genauer)!!!
        cout << setw(6) << setfill('0') << microSec << endl;
        debout << "  Fps with "<< frame+1 <<" frames: " << fps << " (min "<<1./difMax<<" at frame "<<maxFrame<<" to "<<maxFrame+1<<", max "<<1./difMin<<" at frame "<<minFrame<<" to "<<minFrame+1<<")" << endl;
        setFPS(fps);
        mStereo = true;
        mSourceOutFrame = frame; //mNumFrames = frame+1;

        file.close();
        return true;
    }
}

// returns -1, if not set by time file
int Animation::getFirstFrameSec()
{
    return mFirstSec;
}
int Animation::getFirstFrameMicroSec()
{
    return mFirstMicroSec;
}

// get actual Time
// fuer default: frame = -1 wird aktueller frame genommen
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

// Opens an animation given the filename of a video or an image.
// Returns true if the animation was open succesfully
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
        //openRet = openAnimationVideo(fileName);
        openRet = openAnimationPhoto(fileName);
        //}
        // If it is not a video, then is a photo :-)
        //else
        if (openRet == false) // es konnte keine Bildsequenz geladen werden
            openRet = openAnimationVideo(fileName);
        if (mTimeFileLoaded && !openRet)
            debout << "Warning: New loaded time file do not correspond to untouched sequence, because new sequence was not loadable!" << endl;
        mTimeFileLoaded = false; // reset for new open stereo video sequence
        return openRet;
    }
    else 
        return false;
}
// Opens a camera livestream
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

// Returns the number of frames in the current animation  
int Animation::getNumFrames()
{
    if (mVideo || mImgSeq || mStereo)
        return getSourceOutFrameNum()-getSourceInFrameNum()+1;//return mNumFrames;
    return 0;
}

int Animation::getMaxFrames()
{
    if (mVideo || mImgSeq)
        return mMaxFrames;
    return 0;
}

// Returns the index of the current frame  
int Animation::getCurrentFrameNum()
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
// Returns the sourceIn frame number
int Animation::getSourceInFrameNum()
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
// Returns the sourceOut frame number
int Animation::getSourceOutFrameNum()
{
    return mSourceOutFrame;
}

// Returns the filename of the current frame  
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

// Returns the FPS of the current animation
double Animation::getFPS()
{
    if (mCameraLiveStream)
    {
        if (mVideoCapture.get(CAP_PROP_FPS))
            setFPS(mVideoCapture.get(CAP_PROP_FPS));
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

// -2 shows, that it is called by widget
void Animation::setFPS(double fps)
{
    mFps = fps;
}

// Returns the size of the original frames (could made bigger after filtering)
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

// free's all the data in animation
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
    mSpeedup = 1;
    mMaxFrames = -1;
    mSourceInFrame = -1;
    mSourceOutFrame = -1;
    mFps = -1;
    mFirstSec = -1;
    mFirstMicroSec = -1;
    ////mCapture = NULL;
#ifndef STEREO_DISABLED
    mCaptureStereo = NULL;
#endif


    mTimeFileLoaded = false;
    // Set image size
    mSize.setHeight(0);
    mSize.setWidth(0);
}

// void Animation::setFilter(Filter *newFilter){
//     mFilter = newFilter;
// }

// IplImage* Aniion::applyFilter(IplImage *image){
//     if(mFilter==0)return image;
//     else return mFilter->apply(image); //mFilter->setOnCopy(false); geht nicht
// }

// filename without sequence number and suffix : image0001-left.png => image-left
QString Animation::getFileBase()
{
    return mFileBase;
}

QFileInfo Animation::getFileInfo()
{
    return mFileInfo;
}

bool Animation::isVideo()
{
    return mVideo;
}

bool Animation::isStereoVideo()
{
    return mStereo;
}

bool Animation::isImageSequence()
{
    return mImgSeq;
}

bool Animation::isCameraLiveStream()
{
    return mCameraLiveStream;
}

#ifndef STEREO_DISABLED
enum Camera Animation::getCamera()
{
    if (mCaptureStereo != NULL)
        return mCaptureStereo->getCamera();
    else
        return cameraUnset;
}

void Animation::setCamera(enum Camera c)
{
    if (mCaptureStereo != NULL)
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

    // OLD
    //if (!cvLoadImage((fileName.toStdString()).c_str(), -1))
    //    return false;

    Mat tempMat;
    tempMat = imread(fileName.toStdString(),IMREAD_UNCHANGED);//, IMREAD_UNCHANGED);//CV_LOAD_IMAGE_UNCHANGED);

    if(! tempMat.data )                              // Check for invalid input
    {
        //debout <<  "Could not open or find the image" << std::endl ;
        return false;
    }
//    namedWindow("Test-View");
//    imshow("Test-View",tempMat);
//    waitKey();
//    debout << "fileName: " << fileName.toStdString() << " channels: " << tempMat.channels() << " depth: " << tempMat.depth() << endl;
    if( tempMat.channels() == 4 ){
        cout << "Warning: PNG-Alpha channel will be ignored." << endl;
//        tempMat.release();
        tempMat = imread(fileName.toStdString(),IMREAD_COLOR);
    }



    //if( !mVideoCapture.isOpened() || mVideoCapture.get(CV_CAP_PROP_FRAME_COUNT) > 1)
    //   return false;

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

//    for(int i=0;i<fileList.size();i++)
//    {
//        debout << "File[" << i << "]: " <<fileList.at(i).completeBaseName() << " suffix: " << fileList.at(i).suffix() << endl;
//    }

    // series1_0002-left => split into series1_|0002|-left
    // regexp is greedy - from left to right try to get the most characters
    QRegExp regExp("(?:[0-9]*)([^0-9]*)$"); //(?: ) zum ignorieren
    QString front, back;
    int frontLen, frontLenList;
    if ((frontLen = regExp.indexIn(mFileInfo.completeBaseName())) > -1)
    {
        front = mFileInfo.completeBaseName().left(frontLen);
        back = regExp.cap(1);
        //debout << mFileInfo.completeBaseName() << ": " << frontLen << " " << front << " " << back << endl;
    }
    else // does not match regExp at all
        return false;
    mFileBase = front + back; // completeBaseName to cut suffix and sequence number

    // nun wird aus Einzelbildern video erzeugt
//    //untersuchen, ob stereo sequence
//    if (mFileInfo.completeBaseName().contains("right")) //, !Qt::CaseSensitive // macht das ganze komplizierter; auch deutsche Variante nicht erlaubt
//    {
//        QString fileNameLeft = mFileInfo.path() + "/" + mFileInfo.completeBaseName().replace("right", "left") + "." + mFileInfo.completeSuffix();
//        if (QFileInfo(fileNameLeft).exists())
//            mStereo = true;
//    }
//    else if (fileInfo.completeBaseName().contains("left"))
//    {
//        QString fileNameLeft = mFileInfo.path() + "/" + mFileInfo.completeBaseName().replace("left", "right") + "." + mFileInfo.completeSuffix();
//        if (QFileInfo(fileNameLeft).exists())
//            mStereo = true;
//    }
//    if (mStereo)
//        debout << "Image sequence is interpreted as stereo sequence!" << endl;

    // Create a new image files list 
    mImgFilesList.clear();
    for (int i = 0; i < fileList.size(); i++)
    {
        if ((frontLenList = regExp.indexIn(fileList.at(i).completeBaseName())) > -1)
        {
            // Is the file in the series? 
            // debout << "|" << fileList.at(i).completeBaseName().left(frontLenList) << "|" << " == " << "|" << front << "|" << endl
            //       << "|" << regExp.cap(1) << "|" << " == " << "|" << back << "|" << endl
            //       << "|" << fileList.at(i).suffix() << "|" << " == " << "|" << mFileInfo.suffix() << "|" << endl;
            if ((fileList.at(i).completeBaseName().left(frontLenList) == front) &&
                (regExp.cap(1) == back) &&
                (fileList.at(i).suffix() == mFileInfo.suffix()))
            {
                mImgFilesList << fileList.at(i).filePath();
            }
        }
    }

//     QRegExp regExpNumAtEnd("[0-9]*$");
////     QRegExp regExpNoNumAtEnd("[^0-9]*$");
//     // Get the name of the serie; series1_0002-left.png => series1_0002-left
//     QString fileBaseName = mFileInfo.completeBaseName();
//     // Get the string after enumeration; series1_0002-left => series1_0002 (attach == -left)
////     QString attach = fileBaseName.right(fileBaseName.size()-regexpNoNumAtEnd.indexIn(fileBaseName));
//     QString fileBaseName = fileBaseName.left(regexpNoNumAtEnd.indexIn(fileBaseName));
//     // series1_0002 => series1_
//     mFileBase = fileBaseName.left(regexpNumAtEnd.indexIn(fileBaseName)); //getSeriesName(fileBaseName);
//     // Create a new image files list 
//     mImgFilesList.clear();
//     for (int i = 0; i < fileList.size(); i++)
//         // Is the file in the series?
//         //sonst wuerde bei w0 auch wert0 oder bei reiner zahlenkolonne alle genommen werden
//         //if(fileList.at(i).fileName().left(mFileBase.length())==mFileBase)
//         if((fileList.at(i).completeBaseName().left(regexpNumAtEnd.indexIn(fileList.at(i).completeBaseName())) == mFileBase) &&
//            (fileList.at(i).suffix() == mFileInfo.suffix()))
//             mImgFilesList << fileList.at(i).filePath();

    // Get the information of the animation
    if (!getInfoPhoto())
        return false;
    // Set the current frame to  -1 (shows, that no frame is already loaded)
    mCurrentFrame = -1;

    return true;
}

// Implementation of getFrameAtIndex for photo series
// Returns a pointer to the frame at index index in the serie 
Mat Animation::getFramePhoto(int index)
{
    if ((index != mCurrentFrame) || mImage.empty())
    {
        // Check if the index is valid
        if(index < getSourceInFrameNum() || index > getSourceOutFrameNum())
            return Mat();
        // Load the image at a temporary IplImage structure
        // We need to do this because a loaded IplImage cannot be accessed directly
        // = cvLoadImage((mImgFilesList.at(index).toStdString()).c_str(), -1);

        /// See: http://docs.opencv.org/3.0-beta/modules/imgcodecs/doc/reading_and_writing_images.html#imread
//        Mat tempMat;

//        tempMat
        mImage = imread(mImgFilesList.at(index).toStdString(),IMREAD_UNCHANGED);//CV_LOAD_IMAGE_UNCHANGED);

        if( mImage.empty() )                              // Check for invalid input
        {
            debout <<  "Could not open or find the image" << std::endl ;
            return Mat();
        }

        if( mImage.channels() == 4 )
        {
            mImage = imread(mImgFilesList.at(index).toStdString(),IMREAD_COLOR);
        }
        // Check image size of each frame
        //debout << "mat.cols: " << tempMat.cols << " : " << mSize.width() << " tempMat.rows: " << tempMat.rows << " : " << mSize.height() << endl;
        if( (mSize.width() > 0 && mSize.height() > 0) && (mImage.cols != mSize.width() || mImage.rows != mSize.height()) )
        {
            debout << "Could not load image: image size differs in image sequence" << endl;
            debout << "Please ensure to have images of the same size!" << endl;

            QMessageBox::critical(NULL,"An error has occurred!",
                                  QString("The size of the images in the selected image sequence does not agree."
                                          "<br /><br />[0]:  %1  (%2x%3 pixel)<br />[%4]:   %5  (%6x%7 pixel)")
                                  .arg(mImgFilesList.at(0)).arg(mSize.width()).arg(mSize.height()).arg(index)
                                  .arg(mImgFilesList.at(index)).arg(mImage.cols).arg(mImage.rows));

            return Mat();
        }


        //debout << "[" << mImgFilesList.at(index).toStdString() << "]: Channels: " << tempMat.channels() << endl;

        ///
        //debout << "mVideoCapture >> tempMat" << endl;
        //mVideoCapture.open(mImgFilesList.at(index).toStdString());
        //mVideoCapture >> tempMat;
        //debout << "Size: " << tempMat.cols << "x" << tempMat.rows << " channels: " << tempMat.channels() << " depth: " << tempMat.depth() << endl;
        //IplImage* tempImg = cvCreateImage(cvSize(tempMat.cols,tempMat.rows), 8, 3);
//        IplImage* tempImg = cvCreateImage(cvSize(tempMat.cols,tempMat.rows), 8, tempMat.channels());
//        tempImg->imageData = (char *) tempMat.data;
//        debout << "it works!" << endl;
//        cvNamedWindow("Test");
//        cvShowImage("Test",tempImg);
//        cvWaitKey();
//        // Is it created?
//        if (tempImg == NULL)
//            return NULL;
        // delete first old data
        //cvReleaseImage(&mImage); darf nicht freigegeben werden -> absturz, aber speicher wird woanders freigegeben, keine speicheranhaeufung
        // We create and copy the frame to our IplImage pointer
//        cvReleaseImage(&mImage); // lieber vorher loeschen statt kopieren da ggf borderfilter bild groesser amcht?!
//        mImage = cvCloneImage(tempImg);
        // naechsten beiden zeilen entsprechen oberer:
        //mImage = cvCreateImage(cvGetSize(mTempImg), 8, 3);
        //cvCopy(mTempImg, mImage, 0);
        // We release the temporary IplImage
//        cvReleaseImage(&tempImg);
        // We set the current frame index
        mCurrentFrame = index;
        // Return the pointer
    }
    return mImage; //applyFilter(mImage);
}

// Gets Size and Frame number information of the recently open animation
// It is thought to be called once just at the opening of an animation
bool Animation::getInfoPhoto()
{
    bool rc = false;
    Mat tempImg;
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
            debout << "Error: Only 1 or 3 channels (you are using "<< tempImg.channels() <<") and 8 bpp (you are using "<< tempImg.depth() <<") are supported!" << endl;
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
//        cvReleaseImage(&mImage);
        mImage = Mat();
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

// Implementation of the openAnimation function for videos
// Opens an animation from a video file
bool Animation::openAnimationVideo(QString fileName)
{
    ///CvCapture *capture = NULL;

    // Create the capture device
    ///capture = cvCreateFileCapture(fileName.toStdString().c_str());

    if( !mVideoCapture.open(fileName.toStdString().c_str()) )
        return false;

    ////if (capture != NULL)
    if(mVideoCapture.isOpened())
    {
        ////int width  = (int) cvGetCaptureProperty(capture, CV_CAP_PROP_FRAME_WIDTH);
        ////int height = (int) cvGetCaptureProperty(capture, CV_CAP_PROP_FRAME_HEIGHT);
        ////int fps    = (int) cvGetCaptureProperty(capture, CV_CAP_PROP_FPS);
        int width  = (int) mVideoCapture.get(CAP_PROP_FRAME_WIDTH);
        int height = (int) mVideoCapture.get(CAP_PROP_FRAME_HEIGHT);
        int fps    = (int) mVideoCapture.get(CAP_PROP_FPS);
        //int fourcc = (int) cvGetCaptureProperty(capture, CV_CAP_PROP_FOURCC);
        //int frameCount = (int) cvGetCaptureProperty(capture, CV_CAP_PROP_FRAME_COUNT);
        //int propFormat = (int) cvGetCaptureProperty(capture, CV_CAP_PROP_FORMAT);
        //int convertRgb = (int) cvGetCaptureProperty(capture, CV_CAP_PROP_CONVERT_RGB);
        //debout << width << " " << height << " " << fps << " " << fourcc << " "<< frameCount<< " "<< propFormat<< " "<< convertRgb <<endl;
        // stereoVideo:  1280 960 16 0 640 0 0
        // JSC_Boltes_Daenemark_DTU_Experimente\Exercise\exam1: 720 480 29 1685288548 3359 0 0
//        debout << "w: " << width << " h: " << height << " fps: " << fps << endl;
        if ((width == 1280) && (height == 960) && (fps == 16)) // dann gehe ich von einem stereo video der BBX3 aus!!!
        {
            ////cvReleaseCapture(&capture);
            ////capture = NULL;
            mVideoCapture.release();
            mStereo = true;
        }
    }
    // Check if it was created succesfully
    //debout << "Versuch folgende Datei zu oeffnen " << fileName.toStdString().c_str() <<endl;
    ////if(capture == NULL)
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
        debout << "Stereo videos temporary not supported!" << endl;
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
        ////mCapture = capture;
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

// Implementation of getFrameAtIndex for videos
// Returns a pointer to the frame at index index in the video 
Mat Animation::getFrameVideo(int index)
{

    if (mCameraLiveStream)
    {
//        IplImage* tempImg;
//        Mat tempMat;
        if (mVideoCapture.read(mImage/*tempMat*/) )
        {
//            tempImg = cvCreateImage(cvSize(tempMat.cols,tempMat.rows), 8, 3);
//            tempImg->imageData = (char *) tempMat.data;

            if (mImage.empty())//tempImg == NULL)
                return Mat();

//            cvReleaseImage(&mImage);
//            mImage = cvCloneImage(tempImg);
//            cvReleaseImage(&tempImg);
            mCurrentFrame++;
        }
        return mImage;
    }

    if ((index != mCurrentFrame) || mImage.empty())
    {
        // Check if we have a valid index
        if (index < getSourceInFrameNum() || (index > getSourceOutFrameNum() && getSourceOutFrameNum() != -1) )
            return Mat();
        if (mVideo && !mStereo)
        {
            // Check if we have a valid capture device
            ////if (mCapture == NULL)
            if (!mVideoCapture.isOpened())
                return Mat();
            // Now we need to see if it is necessary to seek for the frame or if we can use
            // directly the cvQueryFrame function
            // This is tested since the seek function takes a lot of time!
            if (index == getSourceInFrameNum() || mCurrentFrame+1 != index)
            {
                // OLD
                ////cvSetCaptureProperty(mCapture, CV_CAP_PROP_POS_FRAMES, index);
                // NEW
                if( !mVideoCapture.set(CAP_PROP_POS_FRAMES, index) )
                {
                    debout << "Error: video file does not support skipping." << endl;
                    return Mat();
                }
            }
            // Query the frame
            // = cvQueryFrame(mCapture);
//            IplImage* tempImg;
//            Mat tempMat;
            if( mVideoCapture.read(mImage))//tempMat) )
            {
//                tempImg = cvCreateImage(cvSize(tempMat.cols,tempMat.rows), 8, 3);
//                tempImg->imageData = (char *) tempMat.data;

                if (mImage.empty())//tempImg == NULL)
                    return Mat();
            }else
            {
                debout << "Warning: number of frames in the video seems to be incorrect. Frame[" << index << "] is not loadable! Set number of Frames to " << index << "." << endl;
                mSourceOutFrame = index-1;
                setSourceOutFrameNum(mSourceOutFrame);
                //mNumFrames = index - getSourceInFrameNum();
                return Mat();
            }
            // --------------------------------------------------------- diese zwei zeilen

//            cvReleaseImage(&mImage);
//            mImage = cvCloneImage(tempImg);
//            cvReleaseImage(&tempImg);
            //if (mImage) debout << "mImage: " << mImage << " " << (void *)mImage->imageData <<endl;
            //if (tempImg) debout << "tempImg: " << tempImg << " " << (void *)tempImg->imageData <<endl;

// ---------------------------- ersetzen seit 17.07.2012 folgenden Bereich ---- von hier
//            // We create and copy the frame to our IplImage pointer
//            // es musste mal nicht geswapped/gespiegelt werden um x achse
//#if (CV_MAJOR_VERSION < 2)
//            if (tempImg->origin == 0) // pixel begin on top left
//            {
//                //if (mImage)
//                    //cvCopy(tempImg, mImage);  // geht nicht, da border das bild groesser machen kann
//                if (mImage)
//                {
//                    cvReleaseImage(&mImage);
//                    mImage = NULL;
//                }
//                mImage = cvCloneImage(tempImg); // cvCreateImage(cvGetSize(tempImg), 8, 3);
//            }
//            else // tempImg->origin == 1 begin on bottom left (Windows bitmaps style)
//#endif
//            {
//                if (mImage)
//                {
//                    cvReleaseImage(&mImage);
//                    mImage = NULL;
//                }
//                mImage = cvCreateImage(cvGetSize(tempImg), 8, 3);
//                // folgende beiden zeilen duerften effizienter sein, aber fuerhte zum absturz im 2. durchlauf
//                //if (!mImage) // beim allerersten Aufruf
//                //    mImage = cvCreateImage(cvGetSize(tempImg), 8, 3);
//                char *dataIn;
//                char *dataOut = mImage->imageData - 1;
//                int lineLen = tempImg->width*tempImg->nChannels;

//                for (int i = mImage->height-1; i >= 0 ; --i)
//                {
//                    dataIn  = tempImg->imageData + i*lineLen - 1; // to swap data we go from bottom to top
//                    for (int j = 0; j < mImage->width; ++j)
//                    {
//                        //cout << i << " " << j << endl;
//                        *(++dataOut) = *(++dataIn);
//                        *(++dataOut) = *(++dataIn);
//                        *(++dataOut) = *(++dataIn);
//                    }
//                }
//            }
// --------------------------------------------- bis hier
            // from now on the image is indexed from top left on windows and linux
            //cvCopy(tempImg, mImage, NULL);

            /*
           ATTENTION! The temp image cannot be released if we are capturing from a video.
           If you try to, you will get a very long to find bug ;-)
           (See that we do it when capturing photos!)
           //We release the temporary IplImage
           cvReleaseImage(&tempImg);
//        You must make capture = 0 before releasing it!
//        capture = cvQueryFrame(camera);
//        [...]
//        capture = 0;
//        cvReleaseImage( &capture );

           */
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

// Gets Size and Frame number information of the recently open animation
// It is thought to be called once just at the opening of an animation
bool Animation::getInfoVideo(QString fileName)
{
    // Set the size of the frames
    // We will grab the information from a frame of the animation
    //mNumFrames = 1000;
    setSourceInFrameNum(0);
    Mat tempImg = getFrameVideo(getSourceInFrameNum());
    if (tempImg.empty())
    {
        debout << "Error: No frame could be retrieved from the capture during getInfoVideo." << endl;
        return false;
    }
    // 1 or 3 channel and 8 bit per pixel
    if (!(((tempImg.channels() == 1) || (tempImg.channels() == 3)) && (tempImg.depth() == CV_8U)))
    {
        debout << "Warning: Only 1 or 3 channels (you are using "<< tempImg.channels() <<") and 8 bpp (you are using "<< tempImg.depth() <<") are supported!" << endl;
    }
    // Set the size in the QSize structure
    mSize.setHeight(tempImg.rows);
    mSize.setWidth(tempImg.cols);
    // Release the temporary frame
    // We get the FPS number with the cvGetCaptureProperty function
    // Note that this doesn't work if no frame has already retrieved!!
    ////setFPS(cvGetCaptureProperty(mCapture,CV_CAP_PROP_FPS));
    if (mVideoCapture.get(CAP_PROP_FPS))
        setFPS(mVideoCapture.get(CAP_PROP_FPS));
 
    // Since the Xine implementation is kaputt, we have to do a hack to get the length of the video
    // We will do two cases :
    //  - In Linux will hack it through avinfo
    //  - In Windows will trust the VfW implementation of OpenCV

//    //Linux Hack!
//#ifdef LINUX
//    // We will call avinfo to get the duration of the video in seconds.
//    // Avinfo returns the number of seconds rounded down to the next integer.
//    // The frames of the last second won't be used, since we don't know how many frames are in the last second.

//    // Build the command string
//    // We'll call avinfo so it prints its output to avinfo.tmp

//    QString command = "avinfo ";
//    command += fileName;
//    command += " >avinfo.tmp";
//    char buffer[256];
//    strcpy(buffer,command.toStdString().c_str());
//    // System needs a const char *, that's why we need the char buffer[256]
//    system(buffer);
//    // After avinfo being done, we read avinfo.tmp
//    ifstream dataFile;
//    dataFile.open("avinfo.tmp");
//    char dummy;
//    QString file;
//    while (dataFile >> dummy)
//        file += dummy;
//    dataFile.close();
//    // We can delete the temporary file right now!
//    strcpy(buffer, "rm avinfo.tmp");
//    system(buffer);
//    // With a RegExp evaluator, we find the spot with the duration
//    // It's format is (hh:mm:ss)
//    QRegExp regExp("\\d\\d:\\d\\d:\\d\\d");
//    int ind = regExp.indexIn(file);
//    // If the is no such place in the file, we don't know what to do
//    if(ind == -1)
//    {
//        debout << "The number of frames could not be determined. Strange behaviour!" << endl;
//        setSourceOutFrameNum(-1);
//        return false;
//    }
//    QString duration = file.mid(ind,8);
//    // Now we read the hours, minutes and seconds
//    int hours, minutes, seconds;
//    istringstream iss(duration.toStdString());
//    iss >> hours >> dummy >> minutes >> dummy >> seconds;
//    // And we finally calculate the number of frames
//    mMaxFrames = (int) (((60*hours + minutes)*60 + seconds)*getFPS());
//    ////mNumFrames = (int) cvGetCaptureProperty(mCapture,CV_CAP_PROP_FRAME_COUNT);
//    mMaxFrames = (int) mVideoCapture.get(CV_CAP_PROP_FRAME_COUNT);
//#endif

    // detect the used video codec
    int fourCC = static_cast<int>( mVideoCapture.get(CAP_PROP_FOURCC) );
    char FOURCC[] = {(char)( fourCC & 0XFF) ,
                     (char)((fourCC & 0XFF00) >> 8),
                     (char)((fourCC & 0XFF0000) >> 16),
                     (char)((fourCC & 0XFF000000) >> 24),
                                                        0};
    //debout << "used video codec: " << FOURCC << endl;

    ////mNumFrames = (int) cvGetCaptureProperty(mCapture,CV_CAP_PROP_FRAME_COUNT);
    mMaxFrames = (int) mVideoCapture.get(CAP_PROP_FRAME_COUNT); //mNumFrame
    mSourceOutFrame = mMaxFrames-1;
    //debout << "OpenCV FRAME_COUNT: " << mMaxFrames << endl;

    // Set videocapture to the last frame if CV_CAP_PROP_POS_FRAMES is supported by used video codec
    if( mVideoCapture.set(CAP_PROP_POS_FRAMES, mMaxFrames) > 0 )
    {
        // Set videoCapture to the really correct last frame
        mVideoCapture.set(CAP_PROP_POS_FRAMES, mVideoCapture.get(CAP_PROP_POS_FRAMES)-1);


        // Check if mNumFrames agrees with last readable frame
        if( mMaxFrames != (mVideoCapture.get(CAP_PROP_POS_FRAMES)+1) )
        {
            debout << "Warning: number of frames detected by OpenCV library (" << mMaxFrames << ") seems to be incorrect! (set to estimated value: "<< (mVideoCapture.get(CAP_PROP_POS_FRAMES)+1) << ") [video codec: " << FOURCC << "]" << endl;
            mMaxFrames = mVideoCapture.get(CAP_PROP_POS_FRAMES)+1;
            mSourceOutFrame = mMaxFrames-1;
        }

        // Check if the last frame of the video is readable/OK?
        Mat frame;
        while( !mVideoCapture.read(frame) ){
            mVideoCapture.set(CAP_PROP_POS_FRAMES,mVideoCapture.get(CAP_PROP_POS_FRAMES)-2);
            mMaxFrames = mVideoCapture.get(CAP_PROP_POS_FRAMES)+1;
            if( mVideoCapture.get(CAP_PROP_POS_FRAMES) < 0 ){
                cerr << "Warning: video file seems to be broken!" << endl;
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
        debout << "Error: Incorrect number ("<<getNumFrames()<<") of frames. Setting it to "<<defaultFrames<<"." << endl;
        mSourceOutFrame = getSourceInFrameNum()+defaultFrames;
        //mNumFrames = defaultFrames;
    }
    if (getNumFrames() > maxFrames)// = 6h * 60mins * 60 secs * 25 frames
    {
        debout << "Warning: Number of frames ("<<getNumFrames()<<") seems to be incorrect. Setting it to "<<defaultFrames<<"." << endl;
        mSourceOutFrame = getSourceInFrameNum()+defaultFrames;
    }
    setSourceOutFrameNum(mSourceOutFrame);
    return true;

//     // Print some info about the video..
//     debout << "The video (" << fileName.toStdString() << ") is a " << getSize().width() << "X" << getSize().height() << " with " << getFPS() << "fps and has " << getNumFrames() << " frames." << endl;
}

bool Animation::getCameraInfo()
{
    Mat tempImg = getFrameVideo(0);
    if (tempImg.empty())
    {
        debout << "Error: No frame could be retrieved from the capture during getInfoVideo." << endl;
        return false;
    }
    // 1 or 3 channel and 8 bit per pixel
    if (!(((tempImg.channels() == 1) || (tempImg.channels() == 3)) && (tempImg.depth() == CV_8U)))
    {
        debout << "Warning: Only 1 or 3 channels (you are using "<< tempImg.channels() <<") and 8 bpp (you are using "<< tempImg.depth() <<") are supported!" << endl;
    }
    // Set the size in the QSize structure
    mSize.setHeight(tempImg.rows);
    mSize.setWidth(tempImg.cols);
    // Release the temporary frame
    // We get the FPS number with the cvGetCaptureProperty function
    // Note that this doesn't work if no frame has already retrieved!!
    ////setFPS(cvGetCaptureProperty(mCapture,CV_CAP_PROP_FPS));
    if (mVideoCapture.get(CAP_PROP_FPS))
        setFPS(mVideoCapture.get(CAP_PROP_FPS));

    return true;
}

// Free's the video data 
void Animation::freeVideo()
{
    // Release the capture device
    ////if (mCapture != NULL)
    ////{
    ////    cvReleaseCapture(&mCapture);
    ////    mCapture = NULL;
    ////}
    /// Close VideoCapturer
    if (mVideoCapture.isOpened())
    {
        mVideoCapture.release();
    }
    // Release the image pointer
    if (!mImage.empty())
    {
//        cvReleaseImage(&mImage);
        mImage = Mat();
    }

}
