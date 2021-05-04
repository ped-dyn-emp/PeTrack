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

#include <QApplication>
#include <QToolButton>
#include <QVBoxLayout>
#include <QPixmap>
#include <QSlider>
#include <QStyle>
#include <QLineEdit>
#include <QLabel>
#include <QIntValidator>

#include "player.h"
#include "animation.h"
#include "petrack.h"
#include "control.h"
#include "pMessageBox.h"


Player::Player(Animation *anim, QWidget *parent) : QWidget(parent)
{
    int size = style()->pixelMetric(QStyle::PM_ToolBarIconSize);
    QSize iconSize(size, size);

    //play forward Button
    mPlayForwardButton = new QToolButton;
    mPlayForwardButton->setIcon(QPixmap(":/playF"));
    mPlayForwardButton->setIconSize(iconSize);
    connect(mPlayForwardButton, &QToolButton::clicked,this, [&](){this->play(PlayerState::FORWARD);});

    //play backward Button
    mPlayBackwardButton = new QToolButton;
    mPlayBackwardButton->setIcon(QPixmap(":/playB"));
    mPlayBackwardButton->setIconSize(iconSize);
    connect(mPlayBackwardButton, &QToolButton::clicked, this, [&](){this->play(PlayerState::BACKWARD);});

    //frame forward Button
    mFrameForwardButton = new QToolButton;
    mFrameForwardButton->setAutoRepeat(true);
    mFrameForwardButton->setAutoRepeatDelay(400);   // before repetition starts
    mFrameForwardButton->setAutoRepeatInterval(1000./DEFAULT_FPS); // war: 40 // for 1000 ms / 25 fps
    mFrameForwardButton->setIcon(QPixmap(":/skipF"));
    mFrameForwardButton->setIconSize(iconSize);
    connect(mFrameForwardButton,SIGNAL(clicked()),this,SLOT(frameForward()));

    //frame backward Button
    mFrameBackwardButton = new QToolButton;
    mFrameBackwardButton->setAutoRepeat(true);
    mFrameBackwardButton->setAutoRepeatDelay(400);   // before repetition starts
    mFrameBackwardButton->setAutoRepeatInterval(1000./DEFAULT_FPS); // war: 40 // for 1000 ms / 25 fps
    mFrameBackwardButton->setIcon(QPixmap(":/skipB"));
    mFrameBackwardButton->setIconSize(iconSize);
    connect(mFrameBackwardButton,SIGNAL(clicked()),this,SLOT(frameBackward()));

    //pause button;
    mPauseButton = new QToolButton;
    mPauseButton->setIcon(QPixmap(":/pause"));
    mPauseButton->setIconSize(iconSize);
    connect(mPauseButton,SIGNAL(clicked()),this,SLOT(pause()));

    //rec button;
    mRecButton = new QToolButton;
    mRecButton->setIcon(QPixmap(":/record"));
    mRecButton->setIconSize(iconSize);
    connect(mRecButton,SIGNAL(clicked()),this,SLOT(recStream()));
    mRec = false;

    //slider
    mSlider = new QSlider(Qt::Horizontal);
    mSlider->setTickPosition(QSlider::TicksAbove);
    mSlider->setMinimumWidth(100);
    connect(mSlider,SIGNAL(valueChanged(int)),this,SLOT(skipToFrame(int)));

    //frame number
    QFont f("Courier", 12, QFont::Bold); //Times Helvetica, Normal
    mFrameNumValidator = new QIntValidator(0, 999999, this);
    mFrameInNumValidator = new QIntValidator(0, 999999, this);
    mFrameOutNumValidator = new QIntValidator(0, 999999, this);

    mFrameInNum = new QLineEdit("");
    mFrameInNum->setMaxLength(6);
    mFrameInNum->setMaximumWidth(75);
    mFrameInNum->setAlignment(Qt::AlignRight);
    mFrameInNum->setValidator(mFrameInNumValidator);
    mFrameInNum->setFont(f);
    connect(mFrameInNum,SIGNAL(editingFinished()),this,SLOT(update()));

    mFrameOutNum = new QLineEdit("");
    mFrameOutNum->setMaxLength(8);
    mFrameOutNum->setMaximumWidth(75);
    mFrameOutNum->setAlignment(Qt::AlignRight);
    mFrameOutNum->setValidator(mFrameOutNumValidator);
    mFrameOutNum->setFont(f);
    connect(mFrameOutNum,SIGNAL(editingFinished()),this,SLOT(update()));

    mFrameNum = new QLineEdit("0");
    mFrameNum->setMaxLength(8); // bedeutet maxminal 1,1 stunden
    mFrameNum->setMaximumWidth(75); //5*sz.width() //62
    mFrameNum->setAlignment(Qt::AlignRight);
    mFrameNum->setValidator(mFrameNumValidator);
    mFrameNum->setFont(f);
    connect(mFrameNum,SIGNAL(editingFinished()),this,SLOT(skipToFrame()));

    //frame number
    mFpsNum = new QLineEdit(QString::number(DEFAULT_FPS));
    mFpsNum->setMaxLength(8); // bedeutet maxminal 999,99
    mFpsNum->setMaximumWidth(62); //5*sz.width()
    mFpsNum->setAlignment(Qt::AlignRight);
    mFpsNumValidator = new QDoubleValidator(0.0, 999.99, 2, this);
    mFpsNum->setValidator(mFpsNumValidator);
    mFpsNum->setFont(f);
    connect(mFpsNum,SIGNAL(editingFinished()),this,SLOT(setFPS()));

    QFont f2("Courier", 12, QFont::Normal); //Times Helvetica, Normal
    mAtLabel = new QLabel("@");
    mAtLabel->setFont(f2);

    mSourceInLabel = new QLabel("In:");
    mSourceInLabel->setFont(f2);

    mSourceOutLabel = new QLabel("Out:");
    mSourceOutLabel->setFont(f2);

    mFpsLabel = new QLabel("fps");
    mFpsLabel->setFont(f2);
    // default value
    mPlayerSpeedLimited = false;

    //player layout
    mPlayerLayout = new QHBoxLayout();
    mPlayerLayout->addWidget(mPlayBackwardButton);
    mPlayerLayout->addWidget(mFrameBackwardButton);
    mPlayerLayout->addWidget(mPauseButton);
    mPlayerLayout->addWidget(mFrameForwardButton);
    mPlayerLayout->addWidget(mPlayForwardButton);
    mPlayerLayout->addWidget(mRecButton);
    mPlayerLayout->addWidget(mSourceInLabel);
    mPlayerLayout->addWidget(mFrameInNum);
    mPlayerLayout->addWidget(mSourceOutLabel);
    mPlayerLayout->addWidget(mFrameOutNum);
    mPlayerLayout->addWidget(mSlider);
    mPlayerLayout->addWidget(mFrameNum);
    mPlayerLayout->addWidget(mAtLabel);
    mPlayerLayout->addWidget(mFpsNum);
    mPlayerLayout->addWidget(mFpsLabel);
    mPlayerLayout->setMargin(0);

    mMainWindow = (class Petrack*) parent;

    setLayout(mPlayerLayout);

    setAnim(anim);

}

void Player::setFPS(double fps) // default: double fps=-1.
{
    if (fps != -1)
    {
        mFpsNum->setText(QString::number(fps));
        mFrameForwardButton->setAutoRepeatInterval(1000./fps); // for 1000 ms / 25 fps
        mFrameBackwardButton->setAutoRepeatInterval(1000./fps); // for 1000 ms / 25 fps
    }
    mAnimation->setFPS(mFpsNum->text().toDouble());

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

void Player::setLooping(bool looping)
{
    mLooping = looping;
}

void Player::setSpeedRelativeToRealtime(double factor)
{
    setFPS(mAnimation->getOriginalFPS()*factor);
}

void Player::setAnim(Animation *anim)
{
    if (anim)
    {
        pause();
        mSliderSet = false;
        mAnimation = anim;
        int max = anim->getNumFrames()>1 ? anim->getNumFrames()-1 : 0;
        setSliderMax(max);
        mFrameNumValidator->setTop(max);
        mFrameInNumValidator->setTop(anim->getSourceOutFrameNum());
        mFrameOutNumValidator->setTop(anim->getMaxFrames());
    }
}

bool Player::getPaused()
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
    if (mImg.empty())
    {
        pause();
        return false;
    }
    qApp->processEvents();
#ifdef TIME_MEASUREMENT
    double time1 = 0.0, tstart;
    tstart = clock();
#endif
    mMainWindow->updateImage(mImg);
    if (mRec)
    {
        mAviFile.appendFrame((const unsigned char*) mImg.data, true);
    }
#ifdef TIME_MEASUREMENT
    time1 += clock() - tstart;
    time1 = time1/CLOCKS_PER_SEC;
    cout << "  time(update image) = " << time1 << " sec." << endl;
#endif
    // mSliderSet is needed because setValue will call skiptoframe because of connect slot
    mSliderSet = true;
    mSlider->setValue(mAnimation->getCurrentFrameNum()); //(1000*mAnimation->getCurrentFrameNum())/mAnimation->getNumFrames());
    mFrameNum->setText(QString().number(mAnimation->getCurrentFrameNum()));
    mSliderSet = false; // reset setSlider here because if value doesnt change than it would not be reset by skiptoframe
    qApp->processEvents(); // to allow event while playing

    return true;
}

bool Player::forward()
{
    qApp->processEvents();
#ifdef TIME_MEASUREMENT
    double time1 = 0.0, tstart;
    tstart = clock();
#endif

    bool should_be_last_frame = mAnimation->getCurrentFrameNum() == mAnimation->getSourceOutFrameNum();

    mImg = mAnimation->getNextFrame();
    // check if animation is broken somewhere in the video
    if (mImg.empty())
    {
        if (!should_be_last_frame)
        {
            debout << "Warning: video unexpected finished." << std::endl;
        }
    }
#ifdef TIME_MEASUREMENT
    time1 += clock() - tstart;
    time1 = time1/CLOCKS_PER_SEC;
    cout << "  time(load frame) = " << time1 << " sec." << endl;
#endif
    return updateImage();
}

bool Player::backward()
{
    qApp->processEvents();
#ifdef TIME_MEASUREMENT
    double time1 = 0.0, tstart;
    tstart = clock();
#endif
    mImg = mAnimation->getPreviousFrame();
#ifdef TIME_MEASUREMENT
    time1 += clock() - tstart;
    time1 = time1/CLOCKS_PER_SEC;
    cout << "  time(load frame) = " << time1 << " sec." << endl;
#endif
    return updateImage();
}


/**
 * @brief Sets the state of the video player
 *
 * @see PlayerState
 * @param state
 */
void Player::play(PlayerState state){
    if(mState == PlayerState::PAUSE){
        mState = state;
        playVideo();
    }else{
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
void Player::playVideo(){
    static QElapsedTimer timer;
    int currentFrame = mAnimation->getCurrentFrameNum();
    long long int overtime = 0;

    while(mState != PlayerState::PAUSE){
        // slow down the player speed for extrem fast video sequences (Jiayue China or 16fps cam99 basigo grid video)
        if (mPlayerSpeedLimited || mPlayerSpeedFixed)
        {
            auto supposedDiff = static_cast<long long int>(1'000/mAnimation->getFPS());

            if(timer.isValid()){
                if(mPlayerSpeedFixed && mState == PlayerState::FORWARD){
                    overtime = std::max(0LL, overtime + (timer.elapsed() - supposedDiff));
                    if(overtime >= supposedDiff){
                        mAnimation->skipFrame(static_cast<int>(overtime / supposedDiff));
                        overtime = overtime % supposedDiff;
                        currentFrame = std::min(mAnimation->getCurrentFrameNum() + 1, mAnimation->getSourceOutFrameNum());
                    }
                }

                while(!timer.hasExpired(supposedDiff)){
                    qApp->processEvents();
                }
            }
            timer.start();
        }else{
            timer.invalidate();
        }


        switch(mState){
        case PlayerState::FORWARD:
            mImg = mAnimation->getFrameAtIndex(currentFrame);
            currentFrame++;
            break;
        case PlayerState::BACKWARD:
            mImg = mAnimation->getFrameAtIndex(currentFrame);
            currentFrame--;
            break;
        case PlayerState::PAUSE:
            break;
        }

        if(!updateImage()){
            mState = PlayerState::PAUSE;
            if( mAnimation->getCurrentFrameNum() != 0 && mAnimation->getCurrentFrameNum() != mAnimation->getSourceOutFrameNum()){
                debout << "Warning: video unexpectedly finished." << std::endl;
            }
        }else{
            if( mLooping && mMainWindow->getControlWidget()->trackOnlineCalc->checkState() == Qt::Checked)
            {
                PWarning(this, "Error: No tracking while looping", "Looping and tracking are incompatible. Please disable one first.");
                mState = PlayerState::PAUSE;
                break;
            }else if(mLooping)
            {
                if(mState == PlayerState::FORWARD && mAnimation->getCurrentFrameNum() == mAnimation->getSourceOutFrameNum())
                {
                    currentFrame = 0;
                }else if(mState == PlayerState::BACKWARD && mAnimation->getCurrentFrameNum() == 0)
                {
                    currentFrame = mAnimation->getSourceOutFrameNum();
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

    if (mState != PlayerState::PAUSE)
    {
        lastState = mState;
        pause();
    }
    else
    {
        play(lastState);
    }
}

/**
 * @brief Toggles recording and saving of recording
 *
 * If already recording, the method stops the recording and saves it to
 * a user-given file. If recording hasn't started, this method starts it.
 *
 * Actual recording happens in Player::updateImage()
 */
void Player::recStream()
{
    if (mAnimation->isCameraLiveStream() || mAnimation->isVideo() || mAnimation->isImageSequence() || mAnimation->isStereoVideo())
    {
        // video temp path/file name
        QString videoTmp = QDir::tempPath()+"/petrack-video-record.avi";

        if (mRec) //stop recording and save recorded stream to disk
        {
            mRec = false;
            mRecButton->setIcon(QPixmap(":/record"));

            mAviFile.close();

            QString dest;

            QFileDialog fileDialog(this,
                                   tr("Select file for saving video output"),
                                   nullptr,
                                   tr("Video (*.*);;AVI-File (*.avi);;All supported types (*.avi *.mp4);;All files (*.*)"));
            fileDialog.setAcceptMode(QFileDialog::AcceptSave);
            fileDialog.setFileMode(QFileDialog::AnyFile);
            fileDialog.setDefaultSuffix("");

            if( fileDialog.exec() )
                dest = fileDialog.selectedFiles().at(0);

            if (dest == nullptr)
                return;

            if (QFile::exists(dest))
                QFile::remove(dest);

            QProgressDialog progress("Save Video File",nullptr,0,2,mMainWindow);
            progress.setWindowTitle("Save Video File");
            progress.setWindowModality(Qt::WindowModal);
            progress.setVisible(true);
            progress.setValue(0);

            progress.setLabelText(QString("save video ..."));

            qApp->processEvents();
            progress.setValue(1);

            if (!QFile(videoTmp).copy(dest))
            {
                PCritical(this, tr("PeTrack"),
                                            tr("Error: Could not save video file!"));
            }else
            {
                mMainWindow->statusBar()->showMessage(tr("Saved video file to %1.").arg(dest), 5000);
                if (!QFile(videoTmp).remove())
                debout << "Could not remove tmp-file: " << videoTmp << std::endl;

                progress.setValue(2);
            }

        }else // open video writer
        {
            if (mAviFile.open(videoTmp.toStdString().c_str(), mImg.cols, mImg.rows, 8*mImg.channels(), mAnimation->getFPS()))
            {
                mRec = true;
                mRecButton->setIcon(QPixmap(":/stop-record"));
            }else
            {
                debout << "error: could not open video output file!" << std::endl;
            }
        }
    }
}

bool Player::skipTo(int proMil) // proMil = [0..1000]
{
    if (mSliderSet)
    {
        return false;
    }
    pause();
    mImg = mAnimation->getFrameAtPos(proMil/1000.0); // value between 0..1
    return updateImage();
}

bool Player::skipToFrame(int f) // [0..mAnimation->getNumFrames()-1]
{
    if (mSliderSet)
    {
        return false;
    }
    pause();
    mImg = mAnimation->getFrameAtIndex(f);
    return updateImage();
}

bool Player::skipToFrame() // [0..mAnimation->getNumFrames()-1]
{
       if (mFrameNum->text().toInt() < getFrameInNum())
           mFrameNum->setText(QString::number(getFrameInNum()));
       if (mFrameNum->text().toInt() > getFrameOutNum())
           mFrameNum->setText(QString::number(getFrameOutNum()));

    return skipToFrame(mFrameNum->text().toInt());
}

/**
 * @brief Properly updates FrameInNum and FrameOutNum
 */
void Player::update()
{
    if (true || !mMainWindow->isLoading())
    {

        if( mFrameNum->text().toInt() < mFrameInNum->text().toInt() )
        {
            mFrameNum->setText(mFrameInNum->text());
            skipToFrame(mFrameNum->text().toInt());
        }
        if( mFrameNum->text().toInt() > mFrameOutNum->text().toInt() )
        {
            mFrameNum->setText(mFrameOutNum->text());
            skipToFrame(mFrameNum->text().toInt());
        }
        mSlider->setMinimum(getFrameInNum());
        mSlider->setMaximum(getFrameOutNum());

        mFrameInNumValidator->setTop(getFrameOutNum()-1);
        mFrameNumValidator->setBottom(getFrameInNum());
        mFrameNumValidator->setTop(getFrameOutNum());

        mAnimation->updateSourceInFrameNum(mFrameInNum->text().toInt());
        mAnimation->updateSourceOutFrameNum(mFrameOutNum->text().toInt());

        mMainWindow->updateWindowTitle();
    }
}

int Player::getFrameInNum()
{
    if (mFrameInNum->text() == "")
        return -1;
    return mFrameInNum->text().toInt();
}
void Player::setFrameInNum(int in)
{
    if (in == -1)
        in = 0;

    mFrameInNum->setText(QString::number(in));

    mFrameInNumValidator->setTop(getFrameOutNum()-1);
    mFrameNumValidator->setBottom(getFrameInNum());
    mFrameNumValidator->setTop(getFrameOutNum());
}
int Player::getFrameOutNum()
{
    if (mFrameOutNum->text() == "")
        return -1;

    return mFrameOutNum->text().toInt();
}
void Player::setFrameOutNum(int out)
{
    if (out == -1)
        out = mAnimation->getMaxFrames()-1;

    mFrameOutNum->setText(QString::number(out));

    mFrameInNumValidator->setTop(/*out*/getFrameOutNum()-1);
    mFrameNumValidator->setBottom(getFrameInNum());
    mFrameNumValidator->setTop(/*out*/getFrameOutNum());
}

int Player::getPos()
{
    return mAnimation->getCurrentFrameNum();
}

#include "moc_player.cpp"
