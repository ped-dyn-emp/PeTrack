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

#ifndef PLAYER_H
#define PLAYER_H

#include <QTemporaryFile>
#include <QWidget>
#include <opencv2/opencv.hpp>

class QLabel;
class PSlider;
class QToolButton;
class QHBoxLayout;
class QLineEdit;
class QIntValidator;
class QDoubleValidator;

class Animation;
class Petrack;

enum class PlayerState
{
    FORWARD,
    BACKWARD,
    PAUSE
};

/**
 * @brief The Player class controls the playing and processing of the Animation/Sequence
 *
 * The Player controls the playback and recordings of the Animation.
 * It does not hold the sequence itself. That is done in Animation. However, the
 * actual processing of the Animation is tied to it being played since the player
 * runs Petrack::updateImage() as part of its play-loop. So processing means
 * playing. But processing can be turned off by the user, so playing does not
 * neccessarily mean processing.
 */
class Player : public QWidget
{
    Q_OBJECT
public:
    Player(Animation *anim, QWidget *parent = nullptr);
    void setAnim(Animation *anim);
    bool getPaused() const;
    void setSpeedRelativeToRealtime(double factor);
    void togglePlayPause();
    void jumpSeconds(double seconds);
    void queueJumpSeconds(double seconds);
    void togglePlayerSpeedLimited();
    void setPlayerSpeedLimited(bool fixed);
    bool getPlayerSpeedLimited() const;
    bool getPlayerSpeedFixed() const;
    void setFrameInNum(int in = -1.);
    void setFrameOutNum(int out = -1.);
    int  getFrameInNum();
    int  getFrameOutNum();
    int  getPos();
    bool getLooping() const;

public slots:
    bool frameForward();
    bool frameBackward();
    void pause();
    bool skipToFrame(int f, bool pauseBefore = true);
    bool skipToFrame();
    void update();
    void setPlaybackFPS(double fps = -1.);
    void setPlayerSpeedFixed(bool fixed);
    void setLooping(bool looping);
    void play(PlayerState state);

private:
    void setSliderMax(int max);
    bool updateImage();
    bool forward();
    bool backward();
    void playVideo();

    Animation     *mAnimation;
    QTemporaryFile mTmpFile;
    PlayerState    mState         = PlayerState::PAUSE;
    PlayerState    mPrevDirection = PlayerState::PAUSE;
    bool           mPlayerSpeedLimited;
    bool           mPlayerSpeedFixed   = false;
    bool           mLooping            = false;
    double         mPendingJumpSeconds = 0;

    // GUI
    QToolButton   *mFrameForwardButton, *mFrameBackwardButton, *mPlayForwardButton, *mPlayBackwardButton, *mPauseButton;
    cv::Mat        mImg;
    QHBoxLayout   *mPlayerLayout;
    Petrack       *mMainWindow;
    PSlider       *mSlider;
    QLineEdit     *mFrameNum;
    QLineEdit     *mFrameInNum;
    QLineEdit     *mFrameOutNum;
    QLabel        *mSourceInLabel;
    QLabel        *mSourceOutLabel;
    QIntValidator *mFrameNumValidator;
    QIntValidator *mFrameInNumValidator;
    QIntValidator *mFrameOutNumValidator;
};

#endif
