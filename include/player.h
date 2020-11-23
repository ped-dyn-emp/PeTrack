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

#ifndef PLAYER_H
#define PLAYER_H

#include <QWidget>
#include <QTemporaryFile>

#ifdef AVI
#include "aviFile.h"
#else
#include "aviFileWriter.h"
#endif

#include "opencv2/opencv.hpp"

class QLabel;
class QSlider;
class QToolButton;
class QHBoxLayout;
class QLineEdit;
class QIntValidator;
class QDoubleValidator;

class Animation;
class Petrack;

enum class PlayerState{
    FORWARD,
    BACKWARD,
    PAUSE
};

class Player : public QWidget
{
    Q_OBJECT
public:
    Player(Animation *anim, QWidget *parent = 0);
    void setAnim(Animation *anim);
    bool getPaused();
    void setSpeedRelativeToRealtime(double factor);

public slots:
    bool frameForward();
    bool frameBackward();
    void recStream();
    void pause();
    void togglePlayPause();
    bool skipTo(int proMil);
    bool skipToFrame(int f);
    bool skipToFrame();
    void update();
    void setFPS(double fps=-1.);
    void togglePlayerSpeedLimited();
    void setPlayerSpeedLimited(bool fixed);
    bool getPlayerSpeedLimited();
    void setPlayerSpeedFixed(bool fixed);
    void setFrameInNum(int in=-1.);
    void setFrameOutNum(int out=-1.);
    int getFrameInNum();
    int getFrameOutNum();
    int getPos();
    void play(PlayerState state);


private:
//     void paintEvent(QPaintEvent *event);
    void setSliderMax(int max);
    bool updateImage();
    bool forward();
    bool backward();
    void playVideo();

    Animation *mAnimation;    
    QTemporaryFile mTmpFile;
    PlayerState mState = PlayerState::PAUSE;
    bool mPlayerSpeedLimited;
    bool mPlayerSpeedFixed = false;
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
