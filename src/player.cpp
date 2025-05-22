/*
 * PeTrack - Software for tracking pedestrians movement in videos
 * Copyright (C) 2025 Forschungszentrum Jülich GmbH, IAS-7
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

#include "player.h"

#include "animation.h"
#include "control.h"
#include "logger.h"
#include "pMessageBox.h"
#include "pSlider.h"
#include "petrack.h"

#include <QApplication>
#include <QIntValidator>
#include <QLabel>
#include <QLineEdit>
#include <QPixmap>
#include <QSlider>
#include <QStyle>
#include <QToolButton>
#include <QVBoxLayout>
#include <algorithm>
#include <qtimer.h>

Player::Player(Animation *anim, QWidget *parent) : QWidget(parent)
{
    int   size = style()->pixelMetric(QStyle::PM_ToolBarIconSize);
    QSize iconSize(size, size);

    // play forward Button
    mPlayForwardButton = new QToolButton;
    mPlayForwardButton->setIcon(QPixmap(":/playF"));
    mPlayForwardButton->setIconSize(iconSize);
    connect(mPlayForwardButton, &QToolButton::clicked, this, [&]() { this->play(PlayerState::FORWARD); });

    // play backward Button
    mPlayBackwardButton = new QToolButton;
    mPlayBackwardButton->setIcon(QPixmap(":/playB"));
    mPlayBackwardButton->setIconSize(iconSize);
    connect(mPlayBackwardButton, &QToolButton::clicked, this, [&]() { this->play(PlayerState::BACKWARD); });

    // frame forward Button
    mFrameForwardButton = new QToolButton;
    mFrameForwardButton->setAutoRepeat(true);
    mFrameForwardButton->setAutoRepeatDelay(400);                    // before repetition starts
    mFrameForwardButton->setAutoRepeatInterval(1000. / DEFAULT_FPS); // war: 40 // for 1000 ms / 25 fps
    mFrameForwardButton->setIcon(QPixmap(":/skipF"));
    mFrameForwardButton->setIconSize(iconSize);
    connect(mFrameForwardButton, &QToolButton::clicked, this, &Player::frameForward);

    // frame backward Button
    mFrameBackwardButton = new QToolButton;
    mFrameBackwardButton->setAutoRepeat(true);
    mFrameBackwardButton->setAutoRepeatDelay(400);                    // before repetition starts
    mFrameBackwardButton->setAutoRepeatInterval(1000. / DEFAULT_FPS); // war: 40 // for 1000 ms / 25 fps
    mFrameBackwardButton->setIcon(QPixmap(":/skipB"));
    mFrameBackwardButton->setIconSize(iconSize);
    connect(mFrameBackwardButton, &QToolButton::clicked, this, &Player::frameBackward);

    // pause button;
    mPauseButton = new QToolButton;
    mPauseButton->setIcon(QPixmap(":/pause"));
    mPauseButton->setIconSize(iconSize);
    connect(mPauseButton, &QToolButton::clicked, this, &Player::pause);

    // slider
    mSlider = new PSlider;
    mSlider->setOrientation(Qt::Horizontal);
    mSlider->setTickPosition(QSlider::TicksAbove);
    mSlider->setMinimumWidth(100);
    mSlider->setThrottleInterval(100);
    connect(mSlider, &PSlider::throttledValueChanged, this, [this](int val) { skipToFrame(val); });

    // frame number
    QFont f("Courier", 12, QFont::Bold); // Times Helvetica, Normal
    mFrameNumValidator    = new QIntValidator(0, 999999, this);
    mFrameInNumValidator  = new QIntValidator(0, 999999, this);
    mFrameOutNumValidator = new QIntValidator(0, 999999, this);

    mFrameInNum = new QLineEdit("");
    mFrameInNum->setMaxLength(6);
    mFrameInNum->setMaximumWidth(75);
    mFrameInNum->setAlignment(Qt::AlignRight);
    mFrameInNum->setValidator(mFrameInNumValidator);
    mFrameInNum->setFont(f);
    connect(mFrameInNum, &QLineEdit::editingFinished, this, &Player::update);

    mFrameOutNum = new QLineEdit("");
    mFrameOutNum->setMaxLength(8);
    mFrameOutNum->setMaximumWidth(75);
    mFrameOutNum->setAlignment(Qt::AlignRight);
    mFrameOutNum->setValidator(mFrameOutNumValidator);
    mFrameOutNum->setFont(f);
    connect(mFrameOutNum, &QLineEdit::editingFinished, this, &Player::update);

    mFrameNum = new QLineEdit("0");
    mFrameNum->setMaxLength(8);     // bedeutet maxminal 1,1 stunden
    mFrameNum->setMaximumWidth(75); // 5*sz.width() //62
    mFrameNum->setAlignment(Qt::AlignRight);
    mFrameNum->setValidator(mFrameNumValidator);
    mFrameNum->setFont(f);
    connect(mFrameNum, &QLineEdit::editingFinished, this, static_cast<bool (Player::*)()>(&Player::skipToFrame));

    // frame number
    mFpsNum = new QLineEdit(QString::number(DEFAULT_FPS));
    mFpsNum->setMaxLength(8);     // bedeutet maxminal 999,99
    mFpsNum->setMaximumWidth(62); // 5*sz.width()
    mFpsNum->setAlignment(Qt::AlignRight);
    mFpsNumValidator = new QDoubleValidator(0.0, 999.99, 2, this);
    mFpsNum->setValidator(mFpsNumValidator);
    mFpsNum->setFont(f);
    mFpsNum->setToolTip("Current playback fps");
    connect(mFpsNum, &QLineEdit::editingFinished, this, [this] { setPlaybackFPS(mFpsNum->text().toDouble()); });

    QFont f2("Courier", 12, QFont::Normal); // Times Helvetica, Normal
    mAtLabel = new QLabel("@");
    mAtLabel->setFont(f2);

    mSourceInLabel = new QLabel("In:");
    mSourceInLabel->setFont(f2);

    mSourceOutLabel = new QLabel("Out:");
    mSourceOutLabel->setFont(f2);

    mFpsLabel = new QLabel("fps");
    mFpsLabel->setFont(f2);
    mFpsLabel->setToolTip("Current playback fps");
    // default value
    mPlayerSpeedLimited = false;

    // player layout
    mPlayerLayout = new QHBoxLayout();
    mPlayerLayout->addWidget(mPlayBackwardButton);
    mPlayerLayout->addWidget(mFrameBackwardButton);
    mPlayerLayout->addWidget(mPauseButton);
    mPlayerLayout->addWidget(mFrameForwardButton);
    mPlayerLayout->addWidget(mPlayForwardButton);
    mPlayerLayout->addWidget(mSourceInLabel);
    mPlayerLayout->addWidget(mFrameInNum);
    mPlayerLayout->addWidget(mSourceOutLabel);
    mPlayerLayout->addWidget(mFrameOutNum);
    mPlayerLayout->addWidget(mSlider);
    mPlayerLayout->addWidget(mFrameNum);
    mPlayerLayout->addWidget(mAtLabel);
    mPlayerLayout->addWidget(mFpsNum);
    mPlayerLayout->addWidget(mFpsLabel);
    mPlayerLayout->setContentsMargins(0, 0, 0, 0);

    mMainWindow = (class Petrack *) parent;

    setLayout(mPlayerLayout);

    setAnim(anim);
}


void Player::setPlaybackFPS(double fps) // default: double fps=-1.
{
    if(fps != -1)
    {
        mFpsNum->setText(QString::number(fps));
        mFrameForwardButton->setAutoRepeatInterval(1000. / fps);  // for 1000 ms / 25 fps
        mFrameBackwardButton->setAutoRepeatInterval(1000. / fps); // for 1000 ms / 25 fps
    }
    mAnimation->setPlaybackFPS(mFpsNum->text().toDouble());
}
void Player::setPlayerSpeedLimited(bool fixed)
{
    mPlayerSpeedLimited = fixed;
}

void Player::togglePlayerSpeedLimited()
{
    setPlayerSpeedLimited(!mPlayerSpeedLimited);
}
bool Player::getPlayerSpeedLimited() const
{
    return mPlayerSpeedLimited;
}

void Player::setPlayerSpeedFixed(bool fixed)
{
    mPlayerSpeedFixed = fixed;
}

bool Player::getPlayerSpeedFixed() const
{
    return mPlayerSpeedFixed;
}

void Player::setLooping(bool looping)
{
    mLooping = looping;
}

void Player::setSpeedRelativeToRealtime(double factor)
{
    setPlaybackFPS(mAnimation->getSequenceFPS() * factor);
}

void Player::setAnim(Animation *anim)
{
    if(anim)
    {
        pause();
        mAnimation = anim;
        int max    = anim->getNumFrames() > 1 ? anim->getNumFrames() - 1 : 0;
        setSliderMax(max);
        mFrameNumValidator->setTop(max);
        mFrameInNumValidator->setTop(anim->getSourceOutFrameNum());
        mFrameOutNumValidator->setTop(anim->getMaxFrames());
    }
}

bool Player::getPaused() const
{
    return mState == PlayerState::PAUSE;
}


void Player::setSliderMax(int max)
{
    mSlider->setMaximum(max);
}

/**
 * @brief Processes and displays the image mImg (set in forward() or backward())
 *
 * Heavy lifting is in Petrack::updateImage(). This method itself handles
 * recording and updating the value of the video-slider.
 *
 * @return Boolean indicating if an frame was processed and displayed
 */
bool Player::updateImage()
{
    if(mImg.empty())
    {
        pause();
        return false;
    }
    bool successful = mMainWindow->updateImage(mImg);

    mSlider->setValue(
        mAnimation->getCurrentFrameNum()); //(1000*mAnimation->getCurrentFrameNum())/mAnimation->getNumFrames());
    mFrameNum->setText(QString().number(mAnimation->getCurrentFrameNum()));

    return successful;
}

bool Player::forward()
{
    qApp->processEvents();

    bool should_be_last_frame = mAnimation->getCurrentFrameNum() == mAnimation->getSourceOutFrameNum();

    mImg = mAnimation->getNextFrame();
    // check if animation is broken somewhere in the video
    if(mImg.empty())
    {
        if(!should_be_last_frame)
        {
            SPDLOG_WARN("video unexpected finished.");
        }
    }

    return updateImage();
}

bool Player::backward()
{
    qApp->processEvents();
    mImg = mAnimation->getPreviousFrame();
    return updateImage();
}


/**
 * @brief Sets the state of the video player
 *
 * @see PlayerState
 * @param state
 */
void Player::play(PlayerState state)
{
    if(mState == PlayerState::PAUSE)
    {
        mState = state;
        playVideo();
    }
    else
    {
        mState = state;
    }
}

/**
 * @brief Quasi MainLoop: Plays video in accordance to set frame rate
 *
 * This method is (indirectly) initiating calls to Player::updateImage
 * and thus controls processing and display of video frames. The user has
 * the option to limit playback speed, which is enforced here as well.
 *
 * The method is left, when the video is paused and reentered, when playing
 * gets started again.
 */
void Player::playVideo()
{
    static QElapsedTimer timer;
    long long int        overtime = 0;

    while(mState != PlayerState::PAUSE)
    {
        // slow down the player speed for extrem fast video sequences (Jiayue China or 16fps cam99 basigo grid video)
        if(mPlayerSpeedLimited || mPlayerSpeedFixed)
        {
            auto supposedDiff = static_cast<long long int>(1'000 / mAnimation->getPlaybackFPS());

            if(timer.isValid())
            {
                if(mPlayerSpeedFixed && mState == PlayerState::FORWARD)
                {
                    overtime = std::max(0LL, overtime + (timer.elapsed() - supposedDiff));
                    if(overtime >= supposedDiff)
                    {
                        mAnimation->skipFrame(static_cast<int>(overtime / supposedDiff));
                        overtime %= supposedDiff;
                    }
                }

                while(!timer.hasExpired(supposedDiff))
                {
                    qApp->processEvents();
                }
            }
            timer.start();
        }
        else
        {
            timer.invalidate();
        }


        switch(mState)
        {
            case PlayerState::FORWARD:
                mImg = mAnimation->getNextFrame();
                break;
            case PlayerState::BACKWARD:
                mImg = mAnimation->getPreviousFrame();
                break;
            case PlayerState::PAUSE:
                break;
        }

        int currentFrame = mAnimation->getCurrentFrameNum(); // Fetch current frame directly from mAnimation
        if(!updateImage())
        {
            mState = PlayerState::PAUSE;
            if(currentFrame != 0 && currentFrame != mAnimation->getSourceOutFrameNum())
            {
                SPDLOG_WARN("video unexpectedly finished.");
            }
        }
        else
        {
            if(mLooping && mMainWindow->getControlWidget()->isOnlineTrackingChecked())
            {
                PWarning(
                    this,
                    "Error: No tracking while looping",
                    "Looping and tracking are incompatible. Please disable one first.");
                mState = PlayerState::PAUSE;
                break;
            }
            else if(mLooping)
            {
                if(mState == PlayerState::FORWARD && currentFrame == mAnimation->getSourceOutFrameNum())
                {
                    mImg = mAnimation->getFrameAtIndex(mAnimation->getSourceInFrameNum());
                    mAnimation->setCurrentFrameNum(mAnimation->getSourceInFrameNum());
                }
                else if(mState == PlayerState::BACKWARD && currentFrame == mAnimation->getSourceInFrameNum())
                {
                    mImg = mAnimation->getFrameAtIndex(mAnimation->getSourceOutFrameNum());
                    mAnimation->setCurrentFrameNum(mAnimation->getSourceOutFrameNum());
                }
            }
        }
    }

    timer.invalidate();
}

bool Player::frameForward()
{
    pause();
    return forward();
}
bool Player::frameBackward()
{
    pause();
    return backward();
}

void Player::pause()
{
    mState = PlayerState::PAUSE;
    mMainWindow->setShowFPS(0.);
}

/**
 * @brief Toggles pause/play for use via spacebar
 */
void Player::togglePlayPause()
{
    static PlayerState lastState;

    if(mState != PlayerState::PAUSE)
    {
        lastState = mState;
        pause();
    }
    else if(updateImage()) // only play the video again if there are no pending image updates
    {
        play(lastState);
    }
}

bool Player::skipToFrame(int f, bool pauseBefore) // [0..mAnimation->getNumFrames()-1]
{
    if(f == mAnimation->getCurrentFrameNum())
    {
        return false;
    }
    if(pauseBefore)
    {
        pause();
    }
    mImg = mAnimation->getFrameAtIndex(f);
    return updateImage();
}

bool Player::skipToFrame() // [0..mAnimation->getNumFrames()-1]
{
    if(mFrameNum->text().toInt() < getFrameInNum())
    {
        mFrameNum->setText(QString::number(getFrameInNum()));
    }
    if(mFrameNum->text().toInt() > getFrameOutNum())
    {
        mFrameNum->setText(QString::number(getFrameOutNum()));
    }

    return skipToFrame(mFrameNum->text().toInt());
}

void Player::jumpSeconds(double seconds)
{
    int curFrame = getPos();
    // calculate next frame
    int jumpWidth = static_cast<int>(seconds * mAnimation->getSequenceFPS());
    int nextFrame = curFrame + jumpWidth;
    nextFrame     = std::clamp(nextFrame, getFrameInNum(), getFrameOutNum());

    // go to next frame
    skipToFrame(nextFrame, false);
}


void Player::queueJumpSeconds(double seconds)
{
    static QTimer jumpTimer;
    if(!jumpTimer.isActive())
    {
        jumpTimer.setSingleShot(true);
        connect(
            &jumpTimer,
            &QTimer::timeout,
            [this]()
            {
                jumpSeconds(mPendingJumpSeconds);
                mPendingJumpSeconds = 0;
            });
    }
    mPendingJumpSeconds += seconds;
    jumpTimer.start(300);
}

/**
 * @brief Properly updates FrameInNum and FrameOutNum
 */
void Player::update()
{
    if constexpr(true || !mMainWindow->isLoading())
    {
        if(mFrameNum->text().toInt() < mFrameInNum->text().toInt())
        {
            mFrameNum->setText(mFrameInNum->text());
            skipToFrame(mFrameNum->text().toInt());
        }
        if(mFrameNum->text().toInt() > mFrameOutNum->text().toInt())
        {
            mFrameNum->setText(mFrameOutNum->text());
            skipToFrame(mFrameNum->text().toInt());
        }
        mSlider->setMinimum(getFrameInNum());
        mSlider->setMaximum(getFrameOutNum());

        mFrameInNumValidator->setTop(getFrameOutNum() - 1);
        mFrameNumValidator->setBottom(getFrameInNum());
        mFrameNumValidator->setTop(getFrameOutNum());

        mAnimation->updateSourceInFrameNum(mFrameInNum->text().toInt());
        mAnimation->updateSourceOutFrameNum(mFrameOutNum->text().toInt());

        mMainWindow->updateWindowTitle();
    }
}

int Player::getFrameInNum()
{
    if(mFrameInNum->text() == "")
    {
        return -1;
    }
    return mFrameInNum->text().toInt();
}
void Player::setFrameInNum(int in)
{
    if(in == -1)
    {
        in = 0;
    }

    mFrameInNum->setText(QString::number(in));

    mFrameInNumValidator->setTop(getFrameOutNum() - 1);
    mFrameNumValidator->setBottom(getFrameInNum());
    mFrameNumValidator->setTop(getFrameOutNum());
}
int Player::getFrameOutNum()
{
    if(mFrameOutNum->text() == "")
    {
        return -1;
    }

    return mFrameOutNum->text().toInt();
}
void Player::setFrameOutNum(int out)
{
    if(out == -1)
    {
        out = mAnimation->getMaxFrames() - 1;
    }

    mFrameOutNum->setText(QString::number(out));

    mFrameInNumValidator->setTop(/*out*/ getFrameOutNum() - 1);
    mFrameNumValidator->setBottom(getFrameInNum());
    mFrameNumValidator->setTop(/*out*/ getFrameOutNum());
}

int Player::getPos()
{
    return mAnimation->getCurrentFrameNum();
}

#include "moc_player.cpp"
