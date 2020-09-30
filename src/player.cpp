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

//#define TIME_MEASUREMENT

Player::Player(Animation *anim, QWidget *parent) : QWidget(parent)
{
    int size = style()->pixelMetric(QStyle::PM_ToolBarIconSize);
    QSize iconSize(size, size);

    //play forward Button
    mPlayForwardButton = new QToolButton;
    mPlayForwardButton->setIcon(QPixmap(":/playF"));
    mPlayForwardButton->setIconSize(iconSize);
    connect(mPlayForwardButton,SIGNAL(clicked()),this,SLOT(playForward()));

    //play backward Button
    mPlayBackwardButton = new QToolButton;
    mPlayBackwardButton->setIcon(QPixmap(":/playB"));
    mPlayBackwardButton->setIconSize(iconSize);
    connect(mPlayBackwardButton,SIGNAL(clicked()),this,SLOT(playBackward()));

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
    //mSlider->setRange(0, anim->getNumFrames()>1 ? anim->getNumFrames()-1 : 0); 0 ist default unf setAnim macht rest
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
//     QSize sz = mFrameNum->minimumSizeHint(); //mFrameNum->minimumSizeHint(); // mFrameNum->sizeHint();
    mFrameNum->setMaximumWidth(75); //5*sz.width() //62
    mFrameNum->setAlignment(Qt::AlignRight);
//     mFrameNum->setInputMask ("99999");
    mFrameNum->setValidator(mFrameNumValidator);
    mFrameNum->setFont(f);
    connect(mFrameNum,SIGNAL(editingFinished()),this,SLOT(skipToFrame()));

    //frame number
    mFpsNum = new QLineEdit(QString::number(DEFAULT_FPS));
    mFpsNum->setMaxLength(8); // bedeutet maxminal 999,99
//     QSize sz = mFrameNum->minimumSizeHint(); //mFrameNum->minimumSizeHint(); // mFrameNum->sizeHint();
    mFpsNum->setMaximumWidth(62); //5*sz.width()
    mFpsNum->setAlignment(Qt::AlignRight);
//     mFrameNum->setInputMask ("99999");
    mFpsNumValidator = new QDoubleValidator(0.0, 999.99, 2, this);
    mFpsNum->setValidator(mFpsNumValidator);
    mFpsNum->setFont(f);
    connect(mFpsNum,SIGNAL(editingFinished()),this,SLOT(setFPS()));
//     mFpsLabel = new QLabel();
//     mFpsLabel->setFont(f2);

    //static QLabel atLabel("@");
    QFont f2("Courier", 12, QFont::Normal); //Times Helvetica, Normal
    mAtLabel = new QLabel("@");
    //mAtLabel->setText();
    mAtLabel->setFont(f2);

    mSourceInLabel = new QLabel("In:");
    mSourceInLabel->setFont(f2);

    mSourceOutLabel = new QLabel("Out:");
    mSourceOutLabel->setFont(f2);

    //static QLabel fpsLabel("fps");
    mFpsLabel = new QLabel("fps");
    //mFpsLabel->setText("fps");
    mFpsLabel->setFont(f2);
    // default value
    mPlayerSpeedFixed = false;

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
void Player::setPlayerSpeedFixed(bool fixed)
{
    mPlayerSpeedFixed = fixed;
}

void Player::togglePlayerSpeedFixed()
{
    setPlayerSpeedFixed(!mPlayerSpeedFixed);
}
bool Player::getPlayerSpeedFixed()
{
    return mPlayerSpeedFixed;
}

void Player::fixSpeedRelativeToRealtime(double factor)
{
    setFPS(mAnimation->getOriginalFPS()*factor);
    setPlayerSpeedFixed(true);
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
//        mFpsLabel->setText(QString("@ %1 fps").arg(anim->getFPS()));
//        mIplImg = NULL; // release?
    }
}

bool Player::getPaused()
{
    return (!mPlayF && !mPlayB);
}

void Player::setSliderMax(int max)
{
    mSlider->setMaximum(max);
}

bool Player::updateImage()
{
    if (mImg.empty())
    {
        pause();
        //cerr<<"No valid image to update"<<endl;
//        mMainWindow->updateImage(Mat(mImg.rows,mImg.cols,CV_8UC3,Scalar(0,0,0)));
        return false;
    }
    qApp->processEvents();
#ifdef TIME_MEASUREMENT
    double time1 = 0.0, tstart;
    tstart = clock();
#endif
//    debout << "test" << endl;
    mMainWindow->updateImage(mImg);
//    debout << "test" << endl;
    if (mRec)
    {
        mAviFile.appendFrame((const unsigned char*) mImg.data, true);
        //mAviWriter.append(mIplImg);
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

    // slow down the player speed for extrem fast video sequences (Jiayue China or 16fps cam99 basigo grid video)
    if (mPlayerSpeedFixed)
    {
        auto supposedDiff = static_cast<long long int>(1'000/mAnimation->getFPS());
        static QElapsedTimer timer;
        if(timer.isValid()){
            while(!timer.hasExpired(supposedDiff)){
                qApp->processEvents();
            }
        }else{
            timer.start();
        }

        timer.start();
        done();
    }else
    {
        done();
    }
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

    mImg = mAnimation->getNextFrame();//cvarrToMat(mAnimation->getNextFrame());
    // check if animation is broken somewhere in the video
    if (mImg.empty())
    {
        //debout << "mIplImg == NULL " << mAnimation->getCurrentFrameNum() << endl;
        if (!should_be_last_frame)
        {
            debout << "Warning: video unexpected finished." << endl;
            //mSlider->setMaximum(mAnimation->getCurrentFrameNum());
            //mMainWindow->updateWindowTitle();
        }else
        {
            //debout << "Animation reached its end!" << endl;
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
    mImg = mAnimation->getPreviousFrame();//cvarrToMat(mAnimation->getPreviousFrame());
#ifdef TIME_MEASUREMENT
    time1 += clock() - tstart;
    time1 = time1/CLOCKS_PER_SEC;
    cout << "  time(load frame) = " << time1 << " sec." << endl;
#endif
    return updateImage();
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

void Player::playForward()
{
    if (mPlayF)
        return;
    if (mPlayB)
    {
        pause();
        playForward(); //pause();
        return;
    }
    pause();
    mPlayF = true;
    forward();
}

void Player::playBackward()
{
    if (mPlayB)
        return;
    if (mPlayF)
    {
        pause();
        playBackward(); //pause();
        return;
    }
    pause();
    mPlayB = true;
    backward();
}

void Player::pause()
{
    mPlayF = false;
    mPlayB = false;
    mMainWindow->setShowFPS(0.);
}

void Player::togglePlayPause()
{
    static bool lastPlayF = true; // muss entgegen dem in setAnim() gesetzt werden von playF

    if (mPlayF || mPlayB)
    {
        lastPlayF = mPlayF;
        pause();
    }
    else if (lastPlayF)
        playForward();
    else
        playBackward();
}

bool Player::done()
{
    qApp->processEvents();
    if (mPlayF)
        return forward();
    if (mPlayB)
        return backward();
    return false;
}
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
                                   NULL,
                                   tr("Video (*.*);;AVI-File (*.avi);;All supported types (*.avi *.mp4);;All files (*.*)"));
            fileDialog.setAcceptMode(QFileDialog::AcceptSave);
            fileDialog.setFileMode(QFileDialog::AnyFile);
            fileDialog.setDefaultSuffix("");

            if( fileDialog.exec() )
                dest = fileDialog.selectedFiles().at(0);

            if (dest == NULL)
                return;

            if (QFile::exists(dest))
                QFile::remove(dest);

            QProgressDialog progress("Save Video File",NULL,0,2,mMainWindow);
            progress.setWindowTitle("Save Video File");
            progress.setWindowModality(Qt::WindowModal);
            progress.setVisible(true);
            progress.setValue(0);

            progress.setLabelText(QString("save video ..."));

            qApp->processEvents();
            progress.setValue(1);

            if (!QFile(videoTmp).copy(dest))
            {
                QMessageBox::critical(this, tr("PeTrack"),
                                            tr("Error: Could not save video file!"),
                                            QMessageBox::Ok);
            }else
            {
                mMainWindow->statusBar()->showMessage(tr("Saved video file to %1.").arg(dest), 5000);
                if (!QFile(videoTmp).remove())
                debout << "Could not remove tmp-file: " << videoTmp << endl;

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
                debout << "error: could not open video output file!" << endl;
            }
        }
    }
}

bool Player::skipTo(int proMil) // proMil = [0..1000]
{
    if (mSliderSet)
    {
//         mSliderSet = false;
        return false;
    }
    pause();
    mImg = mAnimation->getFrameAtPos(proMil/1000.0);//cvarrToMat(mAnimation->getFrameAtPos(proMil/1000.0)); // value between 0..1
    return updateImage();
}

bool Player::skipToFrame(int f) // [0..mAnimation->getNumFrames()-1]
{
    if (mSliderSet)
    {
//        mSliderSet = false;
        return false;
    }
    pause();
//    debout << "test" << endl;
    mImg = mAnimation->getFrameAtIndex(f);//cvarrToMat(mAnimation->getFrameAtIndex(f));
//    debout << "test" << endl;
    return updateImage();
}

bool Player::skipToFrame() // [0..mAnimation->getNumFrames()-1]
{
//     if (mSliderSet)
//     {
// //         mSliderSet = false;
//         return false;
//     }
    //debout << "num: " << mFrameNum->text().toInt() << endl;
       if (mFrameNum->text().toInt() < getFrameInNum())
           mFrameNum->setText(QString::number(getFrameInNum()));
       if (mFrameNum->text().toInt() > getFrameOutNum())
           mFrameNum->setText(QString::number(getFrameOutNum()));
//     pause();
//     mIplImg = mAnimation->getFrameAtIndex(mFrameNum->text().toInt());
//     return updateImage();

    return skipToFrame(mFrameNum->text().toInt());
}

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

    //update();
}

int Player::getPos()
{
    return mAnimation->getCurrentFrameNum();
}

#include "moc_player.cpp"
