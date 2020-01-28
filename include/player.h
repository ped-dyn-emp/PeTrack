#ifndef PLAYER_H
#define PLAYER_H

#include <QWidget>
#include <QTemporaryFile>

#ifdef AVI
#include "aviFile.h"
#else
#include "aviFileWriter.h"
#endif

#include "opencv.hpp"

class QLabel;
class QSlider;
class QToolButton;
class QHBoxLayout;
class QLineEdit;
class QIntValidator;
class QDoubleValidator;

class Animation;
class Petrack;

class Player : public QWidget
{
    Q_OBJECT
public:
    Player(Animation *anim, QWidget *parent = 0);
    void setAnim(Animation *anim);
    bool getPaused();

public slots:
    bool frameForward();
    bool frameBackward();
    void playForward();
    void playBackward();
    void recStream();
    void pause();
    void togglePlayPause();
    bool skipTo(int proMil);
    bool skipToFrame(int f);
    bool skipToFrame();
    void update();
    void setFPS(double fps=-1.);
    void togglePlayerSpeedFixed();
    void setPlayerSpeedFixed(bool fixed);
    bool getPlayerSpeedFixed();
    void setFrameInNum(int in=-1.);
    void setFrameOutNum(int out=-1.);
    int getFrameInNum();
    int getFrameOutNum();
    int getPos();
    bool done();


private:
//     void paintEvent(QPaintEvent *event);
    void setSliderMax(int max);
    bool updateImage();
    bool forward();
    bool backward();

    Animation *mAnimation;    
    QTemporaryFile mTmpFile;
    bool mPlayF;
    bool mPlayB;
    bool mPlayerSpeedFixed;
    bool mSliderSet;
    bool mRec;

#ifdef AVI
    AviFile mAviFile;
#else
    AviFileWriter mAviFile;
#endif
 
    //GUI 
    QToolButton *mFrameForwardButton, *mFrameBackwardButton, *mPlayForwardButton, *mPlayBackwardButton, *mPauseButton;
    QToolButton *mRecButton;
//    IplImage *mIplImg;
    cv::Mat mImg;
    QHBoxLayout *mPlayerLayout;
    Petrack *mMainWindow;
    QSlider *mSlider;
    QLineEdit *mFrameNum;
    QLineEdit *mFrameInNum;
    QLineEdit *mFrameOutNum;
    QLineEdit *mFpsNum;
    QLabel *mAtLabel;
    QLabel *mSourceInLabel;
    QLabel *mSourceOutLabel;
    QLabel *mFpsLabel;
    QIntValidator *mFrameNumValidator;
    QIntValidator *mFrameInNumValidator;
    QIntValidator *mFrameOutNumValidator;
    QDoubleValidator *mFpsNumValidator;
};

#endif
