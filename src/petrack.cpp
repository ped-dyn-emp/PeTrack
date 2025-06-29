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

#include <QMutex>
#include <QSignalMapper>
#include <QtOpenGL>
#include <QtOpenGLWidgets/QOpenGLWidget>
#include <QtWidgets>

// Added for Qt5 support
#include "YOLOMarkerWidget.h"
#include "aboutDialog.h"
#include "alignmentGridBox.h"
#include "analysePlot.h"
#include "animation.h"
#include "annotationGroupItem.h"
#include "annotationGroupWidget.h"
#include "autoCalib.h"
#include "autosaveSettings.h"
#include "backgroundItem.h"
#include "calibFilter.h"
#include "codeMarkerWidget.h"
#include "colorMarkerItem.h"
#include "colorMarkerWidget.h"
#include "colorRangeWidget.h"
#include "control.h"
#include "coordItem.h"
#include "coordinateSystemBox.h"
#include "editMoCapDialog.h"
#include "extrinsicBox.h"
#include "filterBeforeBox.h"
#include "gridItem.h"
#include "imageItem.h"
#include "importHelper.h"
#include "intrinsicBox.h"
#include "keybindingDialog.h"
#include "logger.h"
#include "logoItem.h"
#include "moCapItem.h"
#include "multiColorMarkerItem.h"
#include "multiColorMarkerWidget.h"
#include "openMoCapDialog.h"
#include "pIO.h"
#include "pMessageBox.h"
#include "person.h"
#include "petrack.h"
#include "player.h"
#include "recognition.h"
#include "roiItem.h"
#include "stereoItem.h"
#include "stereoWidget.h"
#include "tracker.h"
#include "trackerItem.h"
#include "trackerReal.h"
#include "view.h"
#include "worldImageCorrespondence.h"

#include <QtPrintSupport/QPrintDialog>
#include <QtPrintSupport/QPrinter>
#include <cmath>
#include <ctime>
#include <deque>
#include <iomanip>
#include <opencv2/opencv.hpp>


int Petrack::trcVersion = 0;

// Reihenfolge des anlegens der objekte ist sehr wichtig
Petrack::Petrack(QString petrackVersion) :
    mExtrCalibration(mPersonStorage),
    mPetrackVersion(std::move(petrackVersion)),
    mAuthors(IO::readAuthors(QCoreApplication::applicationDirPath() + "/.zenodo.json"))
{
    QIcon icon;
    icon.addFile(":/icon");          // about
    icon.addFile(":/icon_smallest"); // window title bar
    setWindowIcon(icon);
    mHeadSize            = -1;
    mCmPerPixel          = -1;
    mScene               = nullptr;
    mTracker             = nullptr;
    mTrackerReal         = nullptr; // damit beim zeichnen von control mit analysePlot nicht auf einen feheler laeuft
    mStatusLabelFPS      = nullptr;
    mStatusPosRealHeight = nullptr;
    mStatusLabelPosReal  = nullptr;
    mImageItem           = nullptr;
    mRecognitionChanged  = true;
    mTrackChanged        = true;
    mCoordItem           = nullptr;
    mImage               = nullptr;
    setLoading(true);

    setAcceptDrops(true);

    int space = 2;

    mBrightContrastFilter.disable();
    mBorderFilter.disable();
    mSwapFilter.disable();
    mBackgroundFilter.disable();
    mStereoContext = nullptr;
    mCalibFilter.disable();
    mScene = new QGraphicsScene(this);

    mTrackingRoiItem = new RoiItem(this, Qt::blue);
    connect(mTrackingRoiItem, &RoiItem::changed, this, [=]() { this->setTrackChanged(true); });
    mTrackingRoiItem->setZValue(4); // groesser heisst weiter oben

    mRecognitionRoiItem = new RoiItem(this, Qt::green);
    connect(mRecognitionRoiItem, &RoiItem::changed, this, [=]() { this->setRecognitionChanged(true); });
    mRecognitionRoiItem->setZValue(5); // groesser heisst weiter oben


    // setup control

    auto updateImageCallback = [this]()
    {
        if(!isLoading())
        {
            updateImage();
        }
    };

    auto updateStatusPos = [this]() { setStatusPosReal(); };
    auto updateHeadSize  = [this]() { setHeadSize(); };
    auto getBorderSize   = [this]() { return getImageBorderSize(); };

    auto *filterBeforeBox = new FilterBeforeBox(
        nullptr, // reparented when added to layout
        *getBackgroundFilter(),
        *getBrightContrastFilter(),
        *getBorderFilter(),
        *getSwapFilter(),
        updateImageCallback);

    auto *extrinsicBox = new ExtrinsicBox(this, *getExtrCalibration());
    auto *intrinsicBox = new IntrinsicBox(this, *getAutoCalib(), *getCalibFilter(), *extrinsicBox, updateImageCallback);
    mImageItem         = new ImageItem(this, nullptr);
    auto *coordSysBox  = new CoordinateSystemBox(
        this,
        updateStatusPos,
        updateHeadSize,
        getBorderSize,
        *intrinsicBox,
        *extrinsicBox,
        *mImageItem,
        mExtrCalibration);
    auto *gridBox = new AlignmentGridBox(this);

    mControlWidget = new Control(
        *this,
        *mScene,
        mReco,
        *mTrackingRoiItem,
        *mRecognitionRoiItem,
        mMissingFrames,
        filterBeforeBox,
        intrinsicBox,
        extrinsicBox,
        coordSysBox,
        gridBox);

    connect(mImageItem, &ImageItem::imageChanged, mControlWidget, &Control::imageSizeChanged);

    // end setup control

    mWorldImageCorrespondence = &mControlWidget->getWorldImageCorrespondence();

    mStereoWidget = new StereoWidget(this);
    mStereoWidget->setWindowFlags(Qt::Window);
    mStereoWidget->setWindowTitle("Stereo parameter");

    mColorRangeWidget = new ColorRangeWidget(this);
    mColorRangeWidget->setWindowFlags(Qt::Window);
    mColorRangeWidget->setWindowTitle("Color range");

    mColorMarkerWidget = new ColorMarkerWidget(this);
    mColorMarkerWidget->setWindowFlags(Qt::Window);
    mColorMarkerWidget->setWindowTitle("Color marker parameter");

    mCodeMarkerWidget = new CodeMarkerWidget(this, mReco.getCodeMarkerOptions(), nullptr);
    mCodeMarkerWidget->setWindowFlags(Qt::Window);
    mCodeMarkerWidget->setWindowTitle("Code marker parameter");


    mMultiColorMarkerWidget = new MultiColorMarkerWidget(this);
    mMultiColorMarkerWidget->setWindowFlags(Qt::Window);
    mMultiColorMarkerWidget->setWindowTitle("MultiColor marker parameter");

    mYOLOMarkerWidget = new YOLOMarkerWidget(this, nullptr);

    mLogoItem = new LogoItem(this); // durch uebergabe von scene wird indirekt ein scene->addItem() aufgerufen
    mLogoItem->setZValue(6);        // groesser heisst weiter oben

    mExtrCalibration.setMainWindow(this);

    mGridItem = new GridItem(this, nullptr, coordSysBox, gridBox);
    mGridItem->setZValue(2.5); // durch uebergabe von scene wird indirekt ein scene->addItem() aufgerufen

    mCoordItem = new CoordItem(this, nullptr, coordSysBox);
    mCoordItem->setZValue(3); // groesser heisst weiter oben

    mViewWidget = new ViewWidget(this);
    mView       = mViewWidget->view();

    // Remove border from view (purple glitch at top left)
    mView->setFrameShape(QFrame::NoFrame);

    mView->setScene(mScene);
    connect(mView, &GraphicsView::mouseShiftDoubleClick, this, &Petrack::addManualTrackPointOnlyVisible);
    connect(mView, &GraphicsView::mouseShiftControlDoubleClick, this, &Petrack::splitTrackPerson);
    connect(mView, &GraphicsView::mouseControlDoubleClick, this, &Petrack::addOrMoveManualTrackPoint);
    connect(mView, &GraphicsView::mouseRightDoubleClick, this, &Petrack::deleteTrackPoint);
    connect(mView, &GraphicsView::mouseShiftWheel, this, &Petrack::skipToFrameWheel);
    connect(mView, &GraphicsView::mouseCtrlAltDoubleClick, this, &Petrack::skipToFrameFromTrajectory);
    connect(mView, &GraphicsView::mouseAltMoved, this, &Petrack::moveTrackPoint);
    connect(mView, &GraphicsView::mouseAltPressed, this, &Petrack::selectPersonForMoveTrackPoint);
    connect(mView, &GraphicsView::altReleased, this, &Petrack::releaseTrackPoint);
    connect(mView, &GraphicsView::mouseAltReleased, this, &Petrack::releaseTrackPoint);
    connect(mView, &GraphicsView::mouseCtrlWheel, this, &Petrack::scrollShowOnly);

    mLogWindow = new LogWindow(this, nullptr);
    mLogWindow->setWindowFlags(Qt::Window);
    mLogWindow->setWindowTitle("Log");

    mGroupingWidget = new AnnotationGroupWidget(mGroupManager, mAnimation, this);
    mGroupingWidget->setWindowFlags(Qt::Window);
    mGroupingWidget->setWindowTitle("Annotation Groups");
    connect(&mGroupManager, &AnnotationGroupManager::trajectoryAssignmentChanged, [this]() { this->updateImage(); });
    connect(&mGroupManager, &AnnotationGroupManager::visualizationParameterChanged, [this]() { this->updateImage(); });
    connect(&mGroupManager, &AnnotationGroupManager::groupsChanged, [this]() { this->updateImage(); });

    mPlayerWidget = new Player(&mAnimation, this);

    QVBoxLayout *vLayout = new QVBoxLayout;
    vLayout->setSpacing(space);
    vLayout->addWidget(mViewWidget);
    vLayout->addWidget(mPlayerWidget);

    //---------------------------

    mTracker     = new Tracker(this, mPersonStorage);
    mTrackerReal = new TrackerReal(this, mPersonStorage);
    mTrackerItem = new TrackerItem(this, mPersonStorage);
    mTrackerItem->setZValue(5); // groesser heisst weiter oben

    mActionGroupItem = new AnnotationGroupItem(mGroupManager, *this, mPersonStorage, mAnimation);
    mActionGroupItem->setZValue(6);

    mControlWidget->getColorPlot()->setPersonStorage(&mPersonStorage);
#ifdef QWT
    mControlWidget->getAnalysePlot()->setTrackerReal(mTrackerReal);
#endif

    //---------------------------

    mStereoItem = new StereoItem(this);
    mStereoItem->setZValue(2); // groesser heisst weiter oben
    mStereoItem->setVisible(false);

    //---------------------------

    mColorMarkerItem = new ColorMarkerItem(this);
    mColorMarkerItem->setZValue(2); // groesser heisst weiter oben
    mColorMarkerItem->setVisible(false);
    //---------------------------

    mCodeMarkerItem = new CodeMarkerItem(this, mReco.getCodeMarkerOptions());
    mCodeMarkerItem->setZValue(2); // groesser heisst weiter oben
    mCodeMarkerItem->setVisible(false);
    //---------------------------

    mMultiColorMarkerItem = new MultiColorMarkerItem(this);
    mMultiColorMarkerItem->setZValue(2); // groesser heisst weiter oben
    mMultiColorMarkerItem->setVisible(false);

    //---------------------------

    mBackgroundItem = new BackgroundItem(this, nullptr, *filterBeforeBox);
    mBackgroundItem->setZValue(2.2); // um so groesser um so hoeher um so eher zu sehen
    mBackgroundItem->setVisible(false);

    //---------------------------

    mMoCapItem = new MoCapItem(*this, mAnimation, mMoCapController);
    mMoCapItem->setZValue(3); // um so groesser um so hoeher um so eher zu sehen

    /// Add Items
    mScene->addItem(mImageItem);
    mScene->addItem(mLogoItem);
    mScene->addItem(mGridItem);
    mScene->addItem(mCoordItem);
    mScene->addItem(mTrackingRoiItem);
    mScene->addItem(mRecognitionRoiItem);
    mScene->addItem(mTrackerItem);
    mScene->addItem(mActionGroupItem);
    mScene->addItem(mStereoItem);
    mScene->addItem(mColorMarkerItem);
    mScene->addItem(mCodeMarkerItem);
    mScene->addItem(mMultiColorMarkerItem);
    mScene->addItem(mBackgroundItem);
    mScene->addItem(mMoCapItem);

    //---------------------------

    mCentralLayout = new QHBoxLayout;
    mCentralLayout->setContentsMargins(space, space, space, space);
    mCentralWidget = new QFrame;
    mCentralWidget->setFrameStyle(QFrame::Sunken | QFrame::StyledPanel);
    mCentralWidget->setLayout(mCentralLayout);
    setCentralWidget(mCentralWidget);
    mSplitter = new QSplitter(this);
    // create playAndView-Widget to wrap layout, since QSplitter only accepts widgets
    QWidget *playAndView = new QWidget(this);
    playAndView->setLayout(vLayout);
    mSplitter->addWidget(playAndView);

    mSplitter->addWidget(mControlWidget);

    mSplitter->setStretchFactor(0, 1);
    mSplitter->setStretchFactor(1, 0);

    mCentralLayout->addWidget(mSplitter);


    setWindowTitle(tr("PeTrack"));

    //---------------------------
    mAutoCalib.setMainWindow(this);
    //---------------------------

    createActions();
    createMenus();
    createStatusBar();

    auto *exportShortCut = new QShortcut{QKeySequence("Shift+e"), this};
    connect(exportShortCut, &QShortcut::activated, this, [=]() { exportTracker(); });


    auto *toggleOnlineTracking = new QShortcut{QKeySequence("Shift+t"), this};
    connect(toggleOnlineTracking, &QShortcut::activated, this, [=]() { mControlWidget->toggleOnlineTracking(); });

    auto *toggleRecognition = new QShortcut{QKeySequence("Shift+r"), this};
    connect(toggleRecognition, &QShortcut::activated, this, [=]() { mControlWidget->toggleRecognition(); });

    auto *toggleShowOnly = new QShortcut{QKeySequence("Shift+a"), this};
    connect(toggleShowOnly, &QShortcut::activated, this, [=]() { mControlWidget->toggleShowOnly(); });

    auto *goToStart = new QShortcut{QKeySequence("Shift+s"), this};
    connect(goToStart, &QShortcut::activated, this, [=]() { mControlWidget->onTrackGotoStartNrClicked(); });

    // TODO delete once we get Options to be value only (i.e. no Pointer/Ref anymore)
    mReco.getCodeMarkerOptions().setControlWidget(mControlWidget);
    mReco.getCodeMarkerOptions().setCodeMarkerItem(mCodeMarkerItem);

    mSeqFileName = QDir::currentPath(); // fuer allerersten Aufruf des Programms
    readSettings();

    saveXml(mDefaultSettings); // noch nicht noetig, da eh noch nicht fkt

    mShowFPS = 0;

    mTrcFileName = "";

    // initialer Aufruf, damit beim reinen Laden einer Videodatei die Defaultwerte in control genommen werden zum Setzen
    setHeadSize();

    // um im background subtraction filter das hoehenbild zu beruecksichtigen
    mBackgroundFilter.setStereoContext(&mStereoContext);

    setLoading(false);
}
Petrack::~Petrack()
{
    delete mImage;
    // hier muessten weitere stehen insb die im konstruktor erzeugt werden
    // aber da petrack nur vernichtet wird, wenn programm beendet wird, kann man sich das auch schenken
}

void Petrack::dragEnterEvent(QDragEnterEvent *event)
{
    if(event->mimeData()->hasUrls())
    {
        event->acceptProposedAction();
    }
}

/**
 * @brief Accepts dropped .pet, .trc and media files
 *
 * Opens the project for a .pet. Imports the trajectories for a .trc
 * and tries to open the sequence for any other kind of file.
 *
 * @param event
 */
void Petrack::dropEvent(QDropEvent *event)
{
    if(event->mimeData()->hasUrls())
    {
        if(event->mimeData()->urls().first().toLocalFile().endsWith(".pet", Qt::CaseInsensitive))
        {
            openProject(event->mimeData()->urls().first().toLocalFile());
        }
        else if(event->mimeData()->urls().first().toLocalFile().endsWith(".trc", Qt::CaseInsensitive))
        {
            importTracker(event->mimeData()->urls().first().toLocalFile());
        }
        else
        {
            openSequence(event->mimeData()->urls().first().toLocalFile());
        }
        event->acceptProposedAction();
    }
}

void Petrack::updateSceneRect()
{
    double iW = 0, iH = 0, bS = 0;

    if(mImage && !mImage->isNull())
    {
        iW = mImage->width();
        iH = mImage->height();
        bS = getImageBorderSize();
    }
    else
    {
        mScene->setSceneRect(mScene->itemsBoundingRect());
        return;
    }

    if(mControlWidget->getCalibCoordShow())
    {
        double scale = mControlWidget->getCalibCoordScale() / 10.;
        auto   t     = mControlWidget->getCalibCoord2DTrans() / 10.;

        // setzen der bounding box der scene
        // Faktor 1.1 dient dazu, dass auch Zahl "1" bei coord gut in sichtbaren Bereich passt
        double xMin = (t.x() - 1.1 * scale < -bS) ? t.x() - 1.1 * scale : -bS;
        double yMin = (t.y() - 1.1 * scale < -bS) ? t.y() - 1.1 * scale : -bS;
        double xMax = (t.x() + 1.1 * scale > iW - bS) ? t.x() + 1.1 * scale : iW - bS;
        double yMax = (t.y() + 1.1 * scale > iH - bS) ? t.y() + 1.1 * scale : iH - bS;
        mScene->setSceneRect(xMin, yMin, xMax - xMin, yMax - yMin);
    }
    else
    {
        mScene->setSceneRect(-bS, -bS, iW, iH);
    }
}

/**
 * @brief Loads the content of a .pet file into petrack
 *
 * @param doc the DOM of the .pet file
 * @param openSeq true, if sequence given in doc should be opened
 */
void Petrack::openXml(QDomDocument &doc, bool openSeq)
{
    mMissingFrames.reset();
    bool                      missingFramesExecuted = false;
    std::vector<MissingFrame> missingFrames{};

    QDomElement root = doc.firstChildElement("PETRACK");
    QString     seq;
    int         frame = -1, sourceFrameIn = -1, sourceFrameOut = -1;
    double      playbackFps      = DEFAULT_FPS;
    double      sequenceFps      = DEFAULT_FPS;
    double      oldFps           = -1;
    int         onlyPeopleNr     = 1;
    QString     onlyPeopleNrList = "1";
    int         zoom = 250, rotate = 0, hScroll = 0, vScroll = 0;
    enum Camera cam = cameraUnset;
    setLoading(true);
    auto petVersion = root.attribute("VERSION");

    for(QDomElement elem = root.firstChildElement(); !elem.isNull(); elem = elem.nextSiblingElement())
    {
        if(elem.tagName() == "MAIN")
        {
            if(elem.hasAttribute("SRC"))
            {
                seq            = elem.attribute("SRC");
                QString tmpSeq = getExistingFile(seq, mProFileName);
                if(tmpSeq != "")
                {
                    seq = tmpSeq;
                }

                // will show undistorted image until calibration is loaded as well
                // but image changes maximum values e.g. for cx; need to set this first to ensure that the
                // correct values can be read in
                mSeqFileName = seq;
                if(openSeq)
                {
                    if(seq != "")
                    {
                        openSequence(seq);
                    }
                    else
                    {
                        mAnimation.reset();
                        mImg         = cv::Mat();
                        mImgFiltered = cv::Mat();
                        delete mImage;
                        mImage = nullptr;
                        updateSequence();
                        mLogoItem->ensureVisible();
                        mLogoItem->fadeIn();
                    }
                }
            }
            loadDoubleValue(elem, "STATUS_HEIGHT", mStatusPosRealHeight);
        }
        else if(elem.tagName() == "STEREO")
        {
            mStereoWidget->getXml(elem);
        }
        else if(elem.tagName() == "COLOR_MARKER")
        {
            mColorMarkerWidget->getXml(elem);
        }
        else if(elem.tagName() == "CODE_MARKER")
        {
            mCodeMarkerWidget->getXml(elem);
        }
        else if(elem.tagName() == "MULTI_COLOR_MARKER")
        {
            mMultiColorMarkerWidget->getXml(elem);
        }
        else if(elem.tagName() == "YOLO_MARKER")
        {
            mYOLOMarkerWidget->getXML(elem);
        }
        else if(elem.tagName() == "MOCAP")
        {
            mMoCapController.getXml(elem);
        }
        else if(elem.tagName() == "CONTROL")
        {
            mControlWidget->getXml(elem, petVersion);
            QDomElement tmpElem = (elem.firstChildElement("TRACKING")).firstChildElement("PATH");
            onlyPeopleNr        = readInt(tmpElem, "ONLY_PEOPLE_NR", 1);
            onlyPeopleNrList    = readQString(tmpElem, "ONLY_PEOPLE_NR_LIST", "");
        }
        else if(elem.tagName() == "EXTR_CALIBRATION")
        {
            mExtrCalibration.getXml(elem);
        }
        else if(elem.tagName() == "PLAYER")
        {
            frame = readInt(elem, "FRAME", -1);
            // handle old projects (prior to 0.10.2), which don't differentiate between sequence fps and playback fps
            oldFps         = readDouble(elem, "FPS", -1);
            sequenceFps    = readDouble(elem, "SEQUENCE_FPS", -1);
            playbackFps    = readDouble(elem, "PLAYBACK_FPS", -1);
            sourceFrameIn  = readInt(elem, "SOURCE_FRAME_IN", -1);
            sourceFrameOut = readInt(elem, "SOURCE_FRAME_OUT", -1);
            mPlayerWidget->setPlayerSpeedLimited(readBool(elem, "PLAYER_SPEED_FIXED", false));
        }
        else if(elem.tagName() == "VIEW")
        {
            loadBoolValue(elem, "ANTIALIAS", mAntialiasAct, false);
            loadBoolValue(elem, "OPENGL", mOpenGLAct, false);
            loadBoolValue(elem, "SAVE_TRANSFORMED", mCropZoomViewAct, false);
            QString     matStr = readQString(elem, "TRANSFORMATION");
            QTextStream in(&matStr);
            in >> zoom >> rotate >> hScroll >> vScroll;

            cam = (enum Camera) readInt(elem, "CAMERA", static_cast<int>(cameraUnset));
            loadBoolValue(elem, "HIDE_CONTROLS", mHideControlsAct, false);
        }
        else if(elem.tagName() == "AUTO_TRACK")
        {
            mAutoBackTrack          = readBool(elem, "BACK_TRACK", true);
            mAutoTrackOptimizeColor = readBool(elem, "OPTIMIZE_COLOR", false);
        }
        else if(elem.tagName() == "MISSING_FRAMES")
        {
            if((elem.hasAttribute("executed")) && (elem.attribute("executed").toInt() == 1))
            {
                missingFramesExecuted = true;
                auto node             = elem.firstChildElement("FRAME");
                for(; !node.isNull(); node = node.nextSiblingElement("FRAME"))
                {
                    size_t num   = readInt(node, "NUM_FRAME");
                    int    count = readInt(node, "NUM_MISSING");
                    missingFrames.push_back(MissingFrame{num, count});
                }
            }
        }
        else
        {
            SPDLOG_ERROR("Unknown PETRACK tag {}", elem.tagName());
        }
    }

    mMissingFrames.setExecuted(missingFramesExecuted);
    mMissingFrames.setMissingFrames(missingFrames);

    mViewWidget->setZoomLevel(zoom);
    mViewWidget->setRotateLevel(rotate);
    mView->horizontalScrollBar()->setValue(hScroll);
    mView->verticalScrollBar()->setValue(vScroll);

    bool loaded = false;
    if(!mBackgroundFilter.getFilename().isEmpty())
    {
        if(!(loaded = mBackgroundFilter.load(mBackgroundFilter.getFilename())))
        {
            SPDLOG_ERROR("Error: loading background file {}!", mBackgroundFilter.getFilename());
        }
    }

    mPlayerWidget->setFrameInNum(sourceFrameIn == -1 ? mAnimation.getSourceInFrameNum() : sourceFrameIn);
    mPlayerWidget->setFrameOutNum(sourceFrameOut == -1 ? mAnimation.getSourceOutFrameNum() : sourceFrameOut);
    mPlayerWidget->update();

    if(frame != -1)
    {
        // used first loaded image to determine bg, should not have happened
        if(mControlWidget->isFilterBgChecked() && !loaded)
        {
            // reset background and first skip to selected frame
            mBackgroundFilter.reset();
        }
        if(frame == mAnimation.getCurrentFrameNum())
        {
            //  needed such that e.g. the calibration is marked as known
            //  else, the first updateImage call would be after loading the trajectories
            //  and they would be immediately deleted, because the intrinsic is new
            //  skipToFrame only calls it if we are not already there
            updateImage();
        }
        // will call updateImage and update bg
        mPlayerWidget->skipToFrame(frame);
    }
    else if(loaded)
    {
        updateImage();
    }

    // nicht schon in control, sonst loescht opensequence wieder tracker
    if(mTrcFileName != "")
    {
        // vorher loeschen aller trajektorien, da sonst nach start im ersten bild
        // mgl zwei trackpoints
        // beim haendischen importieren sind weiterhin parallele trajektorien moeglich (warnung wird ausgegeben)
        frame = 0; // default
        if((mPersonStorage.largestLastFrame() >= frame) && (getPersonStorage().smallestFirstFrame() <= frame))
        {
            mPersonStorage.clear();
            mTracker->reset();
        }
        importTracker(mTrcFileName);
    }

    mControlWidget->setTrackShowOnlyNr(onlyPeopleNr);
    mControlWidget->trackShowOnlyNrList()->setText(onlyPeopleNrList);

    if(cam == cameraLeft)
    {
        mCameraLeftViewAct->setChecked(true);
    }
    else if(cam == cameraRight)
    {
        mCameraRightViewAct->setChecked(true);
    }
    setCamera();
    if(oldFps != -1)
    {
        setFPS(oldFps);
    }
    else
    {
        mPlayerWidget->setPlaybackFPS(playbackFps);
        setSequenceFPS(sequenceFps); // set here to override the fps from the default fps of the sequence
    }

    updateImage(); // needed to undistort, draw border, etc. for first display
    setLoading(false);
}

/**
 * Open a petrack project.
 * When mProFileName is set and a file, this method will ask to save the project before opening the new one.
 * If it is not a file, the path is used as directory for the filedialog.
 *
 * If the given filename is empty, a dialog opens that asks for a file to open.
 *
 * @param fileName the filename to open
 * @param openSeq  true, if sequence given in project should be opened
 */
void Petrack::openProject(QString fileName, bool openSeq) // default fileName="", openSequence = true
{
    if(QFileInfo(mProFileName).isFile()) // a project is already loaded
    {
        if(!maybeSave())
        {
            return;
        }
    }
    else if(!QFileInfo(mProFileName).isDir())
    {
        SPDLOG_INFO("mProFileName neither file nor directory - resetting");
        mProFileName = "";
    }
    // if no destination file or folder is given
    if(fileName.isEmpty())
    {
        fileName = QFileDialog::getOpenFileName(
            this,
            tr("Select project file"),
            QFileInfo(mProFileName).path(),
            tr("PeTrack project file (*.pet);;All files (*.*)"));
    }

    if(Autosave::autosaveExists(fileName) && fileName != mProFileName)
    {
        auto ret = PQuestion(this, "Autosave detected", "An autosave was detected.\nDo you want to load the Autosave?");
        if(ret == PMessageBox::StandardButton::Yes)
        {
            setProFileName(fileName);
            mAutosave.loadAutosave();
            return;
        }
    }

    if(!fileName.isEmpty())
    {
        QFile file(fileName);
        if(!file.open(QIODevice::ReadOnly))
        {
            PCritical(this, tr("PeTrack"), tr("Cannot open %1:\n%2.").arg(fileName, file.errorString()));
            return;
        }

        QDomDocument oldSettings;
        QString      oldProFilename = mProFileName;
        saveXml(oldSettings);

        resetSettings();
        QDomDocument doc("PETRACK"); // eigentlich Pfad zu Beschreibungsdatei fuer Dateiaufbau
        if(!doc.setContent(&file))
        {
            PCritical(this, tr("PeTrack"), tr("Cannot read content from %1.").arg(fileName));
            file.close();
            return;
        }

        SPDLOG_INFO("open: {}", fileName);
        file.close();
        setProFileName(fileName);

        QDomElement root = doc.firstChildElement("PETRACK");
        if(root.hasAttribute("VERSION"))
        {
            if(root.attribute("VERSION") != mPetrackVersion)
            {
                PWarning(
                    this,
                    tr("PeTrack"),
                    tr("Reading %1:\nDifferent version numbers %2 (application) and %3 (file) may cause problems.")
                        .arg(fileName, mPetrackVersion, root.attribute("VERSION")));
            }
        }
        try
        {
            openXml(doc, openSeq);
        }
        catch(std::domain_error &e)
        {
            // reset to settings before trying to load new file
            openXml(oldSettings);
            setProFileName(oldProFilename);

            auto errorStr = QString{"Error during reading of pet file:\n%1"}.arg(e.what());
            PCritical(this, "Could not read pet-file", errorStr);
            return;
        }
        mLastTrackerExport = mTrcFileName;
        updateWindowTitle();
    }
}

void Petrack::saveXml(QDomDocument &doc)
{
    QDomElement elem;

    QDomElement root = doc.createElement("PETRACK");
    root.setAttribute("VERSION", mPetrackVersion);
    doc.appendChild(root);

    // main settings (window size, status hight)
    elem        = doc.createElement("MAIN");
    QString seq = getFileList(mSeqFileName, mProFileName);

    elem.setAttribute("SRC", seq);
    elem.setAttribute("STATUS_HEIGHT", mStatusPosRealHeight->value());
    root.appendChild(elem);

    // settings for stereo
    elem = doc.createElement("STEREO");
    mStereoWidget->setXml(elem);
    root.appendChild(elem);

    // settings for color marker
    elem = doc.createElement("COLOR_MARKER");
    mColorMarkerWidget->setXml(elem);
    root.appendChild(elem);

    // settings for code marker
    elem = doc.createElement("CODE_MARKER");
    mCodeMarkerWidget->setXml(elem);
    root.appendChild(elem);

    // settings for YOLO marker
    elem = doc.createElement("YOLO_MARKER");
    mYOLOMarkerWidget->setXML(elem);
    root.appendChild(elem);

    // control settings (right control widget)
    elem = doc.createElement("CONTROL");
    mControlWidget->setXml(elem);
    root.appendChild(elem);

    // settings for multicolor marker
    elem = doc.createElement("MULTI_COLOR_MARKER");
    mMultiColorMarkerWidget->setXml(elem);
    root.appendChild(elem);

    // Reprojection error extrinsic calib
    elem = doc.createElement("EXTR_CALIBRATION");
    mExtrCalibration.setXml(elem);
    root.appendChild(elem);

    // settings for MoCap-Visualization
    elem = doc.createElement("MOCAP");
    mMoCapController.setXml(elem);
    root.appendChild(elem);

    // player settings (which frame, frame range)
    elem = doc.createElement("PLAYER");
    elem.setAttribute("FRAME", mPlayerWidget->getPos()); // == mAnimation.getCurrentFrameNum()
    elem.setAttribute("SEQUENCE_FPS", mAnimation.getSequenceFPS());
    elem.setAttribute("PLAYBACK_FPS", mAnimation.getPlaybackFPS());
    elem.setAttribute("SOURCE_FRAME_IN", mPlayerWidget->getFrameInNum());
    elem.setAttribute("SOURCE_FRAME_OUT", mPlayerWidget->getFrameOutNum());
    elem.setAttribute("PLAYER_SPEED_FIXED", mPlayerWidget->getPlayerSpeedLimited());

    root.appendChild(elem);

    // view settings (zoom, rotate, alias, opengl)
    elem = doc.createElement("VIEW");
    elem.setAttribute("ANTIALIAS", mAntialiasAct->isChecked());
    elem.setAttribute("OPENGL", mOpenGLAct->isChecked());
    elem.setAttribute("SAVE_TRANSFORMED", mCropZoomViewAct->isChecked());
    elem.setAttribute(
        "TRANSFORMATION",
        QString("%1 %2 %3 %4")
            .arg(mViewWidget->getZoomLevel())
            .arg(mViewWidget->getRotateLevel())
            .arg(mView->horizontalScrollBar()->value())
            .arg(mView->verticalScrollBar()->value()));
    elem.setAttribute("CAMERA", mAnimation.getCamera());
    elem.setAttribute("HIDE_CONTROLS", mHideControlsAct->isChecked());
    root.appendChild(elem);

    // auto track settings
    elem = doc.createElement("AUTO_TRACK");
    elem.setAttribute("BACK_TRACK", mAutoBackTrack);
    elem.setAttribute("OPTIMZE_COLOR", mAutoTrackOptimizeColor);
    root.appendChild(elem);

    elem = doc.createElement("MISSING_FRAMES");
    elem.setAttribute("executed", mMissingFrames.isExecuted());
    for(const auto &missingFrame : mMissingFrames.getMissingFrames())
    {
        auto frame = doc.createElement("FRAME");
        frame.setAttribute("NUM_FRAME", static_cast<int>(missingFrame.mNumber));
        frame.setAttribute("NUM_MISSING", missingFrame.mCount);
        elem.appendChild(frame);
    }
    root.appendChild(elem);
}

/// rueckgabewert zeigt an, ob gesichert werden konnte
bool Petrack::saveSameProject()
{
    return saveProject(mProFileName);
}

/**
 * Ask the user to provide a filename to save project into
 * @return true if the saving process was done successfully
 */
bool Petrack::saveProjectAs()
{
    // empty filename will trigger file selection box
    return saveProject("");
}

/**
 * Save the project to the given filename
 * @param fileName the name of the file to save the project into
 * @return true if the saving process was done successfully
 */
bool Petrack::saveProject(QString fileName) // default fileName=""
{
    if(fileName.isEmpty())
    {
        fileName = QFileDialog::getSaveFileName(
            this, tr("Select project file"), mProFileName, tr("PeTrack project file (*.pet);;All files (*.*)"));
    }

    // selection was cancelled
    if(fileName.isEmpty())
    {
        return false;
    }

    setProFileName(fileName);
    QDomDocument doc("PETRACK"); // eigentlich Pfad zu Beschreibungsdatei fuer Dateiaufbau
    saveXml(doc);

    // file output
    QByteArray       byteArray;
    QXmlStreamWriter xmlStream(&byteArray);
    xmlStream.setAutoFormatting(true);
    xmlStream.setAutoFormattingIndent(4);

    xmlStream.writeStartDocument();
    xmlStream.writeDTD("<!DOCTYPE PETRACK>");

    QDomElement element = doc.documentElement();
    writeXmlElement(xmlStream, element);

    xmlStream.writeEndDocument();

    QFile file(fileName);
    if(!file.open(QFile::WriteOnly | QFile::Truncate | QFile::Text))
    {
        PCritical(this, tr("PeTrack"), tr("Cannot save %1:\n%2.").arg(fileName, file.errorString()));
        file.close();
        return false;
    }
    file.write(byteArray);
    file.close(); // also flushes the file

    statusBar()->showMessage(tr("Saved project to %1.").arg(fileName), 5000);
    SPDLOG_INFO("save project to {}", fileName);

    updateWindowTitle();
    return true;
}

void Petrack::writeXmlElement(QXmlStreamWriter &xmlStream, QDomElement element)
{
    xmlStream.writeStartElement(element.tagName());

    QVector<QString>       attribute_names;
    const QDomNamedNodeMap attributes = element.attributes();
    for(int i = 0; i < attributes.size(); ++i)
    {
        attribute_names.push_back(attributes.item(i).toAttr().name());
    }

    // TODO: check if sorting of elements fits our needs
    std::stable_sort(attribute_names.begin(), attribute_names.end()); // for a canonical XML

    // Wants this macro instead of range-based for loop
    foreach(QString name, attribute_names)
    {
        QDomAttr attr = element.attributeNode(name);
        xmlStream.writeAttribute(attr.name(), attr.value());
    }

    // order of child nodes is defined at creation
    if(element.hasChildNodes())
    {
        const QDomNodeList children = element.childNodes();
        for(int i = 0; i < children.size(); ++i)
        {
            writeXmlElement(xmlStream, children.at(i).toElement());
        }
    }

    xmlStream.writeEndElement();
}

/**
 * @brief Opens camera livestream from cam with camID
 * @param camID id of camera to use (defaults to 0)
 */
void Petrack::openCameraLiveStream(int camID /* =-1*/)
{
    if(camID == -1)
    {
        // if more than one camera connected show to choose
        // camID = selectedID;
        SPDLOG_INFO("No camera ID delivered: Set CameraID to 0 (default Camera)");
        camID = 0; // default
    }
    if(!mAnimation.openCameraStream(camID))
    {
        PCritical(this, tr("PeTrack"), tr("Cannot start Camera Livestream."));
        return;
    }
    mSeqFileName = "camera live stream";
    SPDLOG_INFO(
        "open {} ({} frames; {} fps; {} x {} pixel)",
        mSeqFileName,
        mAnimation.getNumFrames(),
        mAnimation.getSequenceFPS(),
        mAnimation.getSize().width(),
        mAnimation.getSize().height());
    updateSequence();
    updateWindowTitle();
    setFPS(mAnimation.getSequenceFPS());
    mLogoItem->fadeOut();

    mPlayerWidget->play(PlayerState::FORWARD);
}

void Petrack::openSequence(QString fileName) // default fileName = ""
{
    if(fileName.isEmpty())
    {
        fileName = QFileDialog::getOpenFileName(
            this,
            tr("Open video or image sequence"),
            QFileInfo(mSeqFileName).path(),
            tr("All supported types (*.avi *.mpg *.mts *.m2t *.m2ts *.wmv *.mp4 *.mov *.mxf *.bmp *.dib *.jpeg *.jpg "
               "*.jpe *.png *.pbm *.pgm *.ppm *.sr *.ras *.tiff *.tif *.exr *.jp2);;Video (*.avi *.mpg *.mts *.m2ts "
               "*.m2t *.wmv *.mov *.mp4 *.mxf);;Images (*.bmp *.dib *.jpeg *.jpg *.jpe *.png *.pbm *.pgm *.ppm *.sr "
               "*.ras *.tiff *.tif *.exr *.jp2);;Windows bitmaps (*.bmp *.dib);;JPEG (*.jpeg *.jpg *.jpe);;Portable "
               "network graphics (*.png);;Portable image format (*.pbm *.pgm *.ppm);;Sun rasters (*.sr *.ras);;TIFF "
               "(*.tiff *.tif);;OpenEXR HDR (*.exr);;JPEG 2000 (*.jp2);;All files (*.*)"));
    }
    if(!fileName.isEmpty())
    {
        if(!mAnimation.openAnimation(fileName))
        {
            PCritical(this, tr("PeTrack"), tr("Cannot load %1.").arg(fileName));
            return;
        }

        mCameraGroupView->setEnabled(mAnimation.isStereoVideo());
        mCameraMenu->setEnabled(mAnimation.isStereoVideo());

        if(mAnimation.isStereoVideo())
        {
            if(mStereoContext)
                delete mStereoContext;
            mStereoContext = new pet::StereoContext(this);
        }


        mSeqFileName = fileName;
        SPDLOG_INFO(
            "open {} ({} frames; {} fps; {} x {} pixel)",
            mSeqFileName,
            mAnimation.getNumFrames(),
            mAnimation.getSequenceFPS(),
            mAnimation.getSize().width(),
            mAnimation.getSize().height());
        updateSequence();
        updateWindowTitle();
        setFPS(mAnimation.getSequenceFPS());
        mLogoItem->fadeOut();
        mMissingFrames.reset();
    }
}

void Petrack::openMoCapFile()
{
    OpenMoCapDialog dialog(this, mMoCapController);
    dialog.exec();
}

void Petrack::editMoCapSettings()
{
    auto *dialog = new EditMoCapDialog(this, mMoCapStorage, [this]() { mScene->update(); });
    dialog->show();
}

void Petrack::updateWindowTitle()
{
    QString title;
    QSize   size = mAnimation.getSize();

    if(QFileInfo(mProFileName).isDir())
    {
        title = tr("PeTrack (v") + mPetrackVersion + tr("): ");
    }
    else
    {
        title = tr("PeTrack (v") + mPetrackVersion + tr("): ") + QFileInfo(mProFileName).fileName();
        if(mAnimation.isVideo() || mAnimation.isImageSequence())
        {
            title += "; ";
        }
    }
    if(mAnimation.isVideo())
    {
        title += "sequence: " + mAnimation.getCurrentFileName() + tr(" (%1").arg(mAnimation.getNumFrames()) +
                 tr(" frames; %1x%2").arg(size.width()).arg(size.height()) + " pixel)";
    }
    else if(mAnimation.isImageSequence())
    {
        title += "sequence: " + mAnimation.getCurrentFileName() + tr(" ... (%1").arg(mAnimation.getNumFrames()) +
                 tr(" frames; %1x%2").arg(size.width()).arg(size.height()) + " pixel)";
    }
    if(mAnimation.isVideo() || mAnimation.isImageSequence())
    {
        title += tr(" Sequence FPS: %1").arg(mAnimation.getSequenceFPS());
    }
    setWindowTitle(title);
}

void Petrack::exportVideo()
{
    exportSequence(true, false);
}
void Petrack::exportVideoView()
{
    exportSequence(true, true);
}
void Petrack::exportImageSequence()
{
    exportSequence(false, false);
}
void Petrack::exportViewSequence()
{
    exportSequence(false, true);
}


/**
 * @brief Exports current sequence as avi-file or image sequence
 *
 * Exports the loaded image sequence or video from current frame on till the end.
 * One can export the sequence as is or one can export the view shown in PeTrack.
 *
 * @param exportVideo true, if wanting to export a video. Ignored when dest isn't empty
 * @param exportView true, if sequence should be exported as shown in PeTrack (with trajectories etc.)
 * @param dest destination file; if empty, the user chooses via a dialog
 */
void Petrack::exportSequence(bool exportVideo, bool exportView, QString dest) // default exportView= false, dest=""
{
    static QString lastDir;
    // if sequenceFps is not set, show an error message
    if(exportVideo && mAnimation.getSequenceFPS() <= 0)
    {
        PCritical(
            this,
            tr("PeTrack"),
            tr("Sequence FPS is not set.\nCheck your configuration and make sure to set sequence FPS."));
        return;
    }

    // if no destination file or folder is given
    if(dest.isEmpty())
    {
        if(lastDir.isEmpty() && !mSeqFileName.isEmpty())
        {
            lastDir = QFileInfo(mSeqFileName).path();
        }

        if(exportVideo)
        {
            dest = QFileDialog::getSaveFileName(
                this,
                tr("Select video file"),
                lastDir,
                tr("Video (*.mp4 *.avi);;All files (*.*)")); //? *.mpg *.mpeg
        }
        else
        {
            if(exportView)
            {
                dest = QFileDialog::getExistingDirectory(
                    this, tr("Select folder"), lastDir, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
            }
            else
            {
                dest = QFileDialog::getExistingDirectory(
                    this, tr("Select folder"), lastDir, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
            }
        }
    }
    auto extension = dest.right(4).toLower();

    int fourcc = -1;
    if(extension == ".mp4")
    {
        fourcc      = cv::VideoWriter::fourcc('m', 'p', '4', 'v');
        exportVideo = true;
    }
    else if(extension == ".avi")
    {
        fourcc      = cv::VideoWriter::fourcc('M', 'J', 'P', 'G');
        exportVideo = true;
    }
    else
    {
        exportVideo = false;
    }


    if(!dest.isEmpty() && mImage)
    {
        int       rest             = mAnimation.getNumFrames() - 1;
        int       numLength        = 1;
        int       memPos           = mPlayerWidget->getPos();
        QString   fileName         = "";
        bool      formatIsSaveAble = false;
        bool      saveRet;
        QImage   *viewImage = nullptr;
        QPainter *painter   = nullptr;
        int       progEnd   = mAnimation.getSourceOutFrameNum() -
                      mPlayerWidget->getPos(); // nur wenn nicht an anfang gesprungen wird:-mPlayerWidget->getPos()
        cv::Mat         iplImgFilteredBGR;
        bool            writeFrameRet = false;
        cv::VideoWriter outputVideo;

        if(exportVideo)
        {
            if(exportView)
            {
                if(mCropZoomViewAct->isChecked())
                {
                    viewImage =
                        new QImage(mView->viewport()->width(), mView->viewport()->height(), QImage::Format_RGB32);
                }
                else
                {
                    viewImage = new QImage((int) mScene->width(), (int) mScene->height(), QImage::Format_RGB32);
                }
                painter = new QPainter();
            }

            if(exportView)
            {
                outputVideo = cv::VideoWriter(
                    dest.toStdString(),
                    fourcc,
                    mAnimation.getSequenceFPS(),
                    cv::Size(viewImage->width(), viewImage->height()));
            }
            else
            {
                bool colored = (mImg.channels() > 1);
                outputVideo  = cv::VideoWriter(
                    dest.toStdString(),
                    fourcc,
                    mAnimation.getSequenceFPS(),
                    cv::Size(mImgFiltered.cols, mImgFiltered.rows),
                    colored);
            }
        }

        if(!exportVideo)
        {
            if(exportView)
            {
                if(mCropZoomViewAct->isChecked())
                {
                    viewImage =
                        new QImage(mView->viewport()->width(), mView->viewport()->height(), QImage::Format_RGB32);
                }
                else
                {
                    viewImage = new QImage((int) mScene->width(), (int) mScene->height(), QImage::Format_RGB32);
                }
                painter = new QPainter();
            }

            // test, if fileformat is supported
            if(mAnimation.isVideo())
            {
                // calculate string length of sequence number
                while((rest /= 10) > 0)
                {
                    numLength++;
                }
                fileName = (dest + "/" + mAnimation.getFileBase() + "%1.png")
                               .arg(mPlayerWidget->getPos(), numLength, 10, QChar('0'));
            }
            else
            {
                fileName = dest + "/" + mAnimation.getCurrentFileName();
            }

            if(exportView)
            {
                painter->begin(viewImage);
                if(mCropZoomViewAct->isChecked())
                {
                    mView->render(painter);
                }
                else
                {
                    mScene->render(painter);
                }
                painter->end();

                if(viewImage->save(fileName)) //, const char * format = 0 (format wird aus dateinamen geholt), int
                                              // quality = -1 default normal (0..100)
                {
                    formatIsSaveAble = true;
                    mPlayerWidget->frameForward();
                }
            }
            else if(mImage->save(fileName)) //, const char * format = 0 (format wird aus dateinamen geholt), int quality
                                            //= -1 default normal (0..100)
            {
                formatIsSaveAble = true;
                mPlayerWidget->frameForward();
            }
        }
        else if((mImgFiltered.channels() == 1) /*&& convert8To24bit*/)
        {
            cv::Size size;
            size.width  = mImgFiltered.cols;
            size.height = mImgFiltered.rows;
            iplImgFilteredBGR.create(size, CV_8UC3);
        }

        QProgressDialog progress("", "Abort export", 0, progEnd, this);
        progress.setWindowModality(Qt::WindowModal); // blocks main window

        if(exportVideo)
        {
            if(exportView)
            {
                progress.setLabelText("Export view video...");
            }
            else
            {
                progress.setLabelText("Export video...");
            }
        }
        else
        {
            if(exportView)
            {
                progress.setLabelText("Export view sequence...");
            }
            else
            {
                progress.setLabelText("Export image sequence...");
            }
        }

        do
        {
            progress.setValue(
                mPlayerWidget->getPos() - memPos); // -mempos nur, wenn nicht an den anfang gesprungen wird
            qApp->processEvents();
            if(progress.wasCanceled())
            {
                break;
            }

            if(exportVideo)
            {
                // video sequence
                if(exportView)
                {
                    painter->begin(viewImage);
                    if(mCropZoomViewAct->isChecked())
                    {
                        mView->render(painter);
                    }
                    else
                    {
                        mScene->render(painter);
                    }
                    painter->end();
                }

                if(exportView)
                {
                    cv::Mat frame(
                        viewImage->height(),
                        viewImage->width(),
                        CV_8UC4,
                        (unsigned char *) viewImage->bits(),
                        viewImage->bytesPerLine());
                    cv::cvtColor(frame, frame, cv::COLOR_RGBA2RGB); // need for right image interpretation
                    outputVideo.write(frame);
                    writeFrameRet = true;
                }
                else
                {
                    cv::Mat frame = mImgFiltered.clone();
                    outputVideo.write(frame);
                    writeFrameRet = true;
                }

                if(!writeFrameRet)
                {
                    progress.setValue(progEnd);
                    PCritical(
                        this,
                        tr("PeTrack"),
                        tr("Cannot export %1 maybe because of wrong file extension or unsupported codec.").arg(dest));
                    break;
                }
            }
            else
            {
                // single frame sequence
                if(exportView)
                {
                    painter->begin(viewImage);
                    if(mCropZoomViewAct->isChecked())
                    {
                        mView->render(painter);
                    }
                    else
                    {
                        mScene->render(painter);
                    }
                    painter->end();
                }
                if(mAnimation.isVideo())
                {
                    fileName = (dest + "/" + mAnimation.getFileBase() + "%1.png")
                                   .arg(mPlayerWidget->getPos(), numLength, 10, QChar('0'));
                    if(exportView)
                    {
                        saveRet = viewImage->save(fileName);
                    }
                    else
                    {
                        saveRet = mImage->save(fileName);
                    }
                }
                else if(formatIsSaveAble)
                {
                    fileName = dest + "/" + mAnimation.getCurrentFileName();
                    if(exportView)
                    {
                        saveRet = viewImage->save(fileName);
                    }
                    else
                    {
                        saveRet = mImage->save(fileName);
                    }
                }
                else
                {
                    fileName = dest + "/" + QFileInfo(mAnimation.getCurrentFileName()).completeBaseName() + ".png";
                    if(exportView)
                    {
                        saveRet = viewImage->save(fileName);
                    }
                    else
                    {
                        saveRet = mImage->save(fileName, "PNG"); //, int quality = -1 default normal (0..100)
                    }
                }
                if(!saveRet)
                {
                    progress.setValue(progEnd);
                    PCritical(this, tr("PeTrack"), tr("Cannot export %1.").arg(fileName));
                    break;
                }
            }
        } while(mPlayerWidget->frameForward());

        if(!exportVideo && exportView)
        {
            delete viewImage;
            delete painter;
        }

        // bei abbruch koennen es auch mPlayerWidget->getPos() frames sein, die bisher geschrieben wurden
        //-memPos nur, wenn nicht an den anfang gesprungen wird
        SPDLOG_INFO("wrote {} of {} frames.", mPlayerWidget->getPos() + 1 - memPos, mAnimation.getNumFrames());
        progress.setValue(progEnd);

        if(exportVideo)
        {
            outputVideo.release();
        }

        mPlayerWidget->skipToFrame(memPos);
        lastDir = dest;
    }
}

/**
 * @brief Saves the current View, including visualizations, in a file (e.g. pdf)
 *
 * @param dest name of the saved file; if empty, a dialogue for the user opens
 */
void Petrack::exportView(QString dest) // default = ""
{
    static QString lastFile;

    if(mImage)
    {
        // if no destination file or folder is given
        if(dest.isEmpty())
        {
            if(lastFile.isEmpty() && !mSeqFileName.isEmpty())
            {
                lastFile = QFileInfo(mSeqFileName).path();
            }

            // alle unetrstuetzen fileformate erhaelt man mit
            // QImageReader::supportedImageFormats() and QImageWriter::supportedImageFormats()
            // gif muss nicht dabei sein, dazu muss qt mit -qt-gif uebersetzt worden sein
            dest = QFileDialog::getSaveFileName(
                this,
                tr("Select image file"),
                lastFile,
                tr("PDF (*.pdf);;Postscript (*.ps *.eps);;Windows bitmaps (*.bmp);;JPEG (*.jpeg *.jpg);;Portable "
                   "network graphics (*.png);;Portable image format (*.pbm *.pgm *.ppm);;X11 Bitmap or Pixmap (*.xbm "
                   "*.xpm);;Pixel Images (*.bmp *.jpeg *.jpg *.png *.pbm *.pgm *.ppm *.xbm *.xpm);;All supported types "
                   "(*pdf *ps *.eps *.bmp *.jpeg *.jpg *.png *.pbm *.pgm *.ppm *.xbm *.xpm);;All files (*.*)"));
        }

        if(!dest.isEmpty())
        {
            if(dest.endsWith(".pdf", Qt::CaseInsensitive) || dest.endsWith(".ps", Qt::CaseInsensitive) ||
               dest.endsWith(".eps", Qt::CaseInsensitive))
            {
                QPdfWriter pdfWriter(dest);
                pdfWriter.setPageMargins({0, 0, 0, 0});
                QPageSize pageSize{mImage->size()};
                pdfWriter.setPageSize(pageSize);
                QPainter painter(&pdfWriter);

                if(mCropZoomViewAct->isChecked())
                {
                    mView->render(&painter);
                }
                else
                {
                    mScene->render(&painter);
                }
            }
            else
            {
                // schwarzer rand links und unten?!
                QImage *img;
                if(mCropZoomViewAct->isChecked())
                {
                    img = new QImage(mView->viewport()->width(), mView->viewport()->height(), QImage::Format_RGB32);
                }
                else
                {
                    img = new QImage((int) mScene->width(), (int) mScene->height(), QImage::Format_RGB32);
                }
                QPainter painter(img);
                if(mCropZoomViewAct->isChecked())
                {
                    mView->render(&painter);
                }
                else
                {
                    mScene->render(&painter);
                }
                painter.end();
                if(!img->save(dest)) //, "PNG"
                {
                    PCritical(
                        this, tr("PeTrack"), tr("Cannot save %1 maybe because of wrong file extension.").arg(dest));
                }
                delete img;
            }
            lastFile = dest;
        }
    }
}

void Petrack::exportImage(QString dest) // default = ""
{
    static QString lastFile;

    if(mImage)
    {
        // if no destination file or folder is given
        if(dest.isEmpty())
        {
            if(lastFile.isEmpty() && !mSeqFileName.isEmpty())
            {
                lastFile = QFileInfo(mSeqFileName).path();
            }


            // alle unetrstuetzen fileformate erhaelt man mit
            // QImageReader::supportedImageFormats() and QImageWriter::supportedImageFormats()
            // gif muss nict dabei sein, dazu muss qt mit -qt-gif uebesetz worden sein
            dest = QFileDialog::getSaveFileName(
                this,
                tr("Select image file"),
                lastFile,
                tr("PDF (*.pdf);;Postscript (*.ps *.eps);;Windows bitmaps (*.bmp);;JPEG (*.jpeg *.jpg);;Portable "
                   "network graphics (*.png);;Portable image format (*.pbm *.pgm *.ppm);;X11 Bitmap or Pixmap (*.xbm "
                   "*.xpm);;Pixel Images (*.bmp *.jpeg *.jpg *.png *.pbm *.pgm *.ppm *.xbm *.xpm);;All supported types "
                   "(*pdf *ps *.eps *.bmp *.jpeg *.jpg *.png *.pbm *.pgm *.ppm *.xbm *.xpm);;All files (*.*)"));
        }

        if(!dest.isEmpty())
        {
            if(dest.endsWith(".pdf", Qt::CaseInsensitive) || dest.endsWith(".ps", Qt::CaseInsensitive) ||
               dest.endsWith(".eps", Qt::CaseInsensitive))
            {
                QPdfWriter pdfWriter(dest);
                pdfWriter.setPageMargins({0, 0, 0, 0});
                QPageSize pageSize{mImage->size()};
                pdfWriter.setPageSize(pageSize);
                QPainter painter(&pdfWriter);

                QRect rect = painter.viewport();
                QSize size = mImage->size();
                size.scale(rect.size(), Qt::KeepAspectRatio);
                painter.setViewport(rect.x(), rect.y(), size.width(), size.height());
                painter.setWindow(mImage->rect());
                painter.drawImage(0, 0, *mImage);
            }
            else
            {
                if(!mImage->save(dest)) //, "PNG"
                {
                    PCritical(
                        this, tr("PeTrack"), tr("Cannot save %1 maybe because of wrong file extension.").arg(dest));
                }
            }
            lastFile = dest;
        }
    }
}

void Petrack::print()
{
    if(mImage)
    {
        // HighResolution font zu gross! und laengere laufzeit und eher overflow
        // aber so pixelig und keine schoenen linien
        QPrinter printer(QPrinter::ScreenResolution); // ScreenResolution, HighResolution// liefert zu hause:
        // QWin32PrintEngine::initialize: GetPrinter failed ()
        printer.setPageSize(QPageSize{QPageSize::PageSizeId::A4});
        QPrintDialog dialog(&printer, this);
        if(dialog.exec())
        {
            QPainter painter(&printer);
            mView->render(&painter);
        }
    }
    else
    {
        PCritical(this, tr("PeTrack"), tr("Nothing to print!"));
    }
}

void Petrack::resetSettings()
{
    mAnimation.reset();
    openXml(mDefaultSettings, false);
    mControlWidget->resetCorrection();
}

void Petrack::setFPS(double fps)
{
    mAnimation.setSequenceFPS(fps);
    mPlayerWidget->setPlaybackFPS(fps);
    updateWindowTitle();
}

void Petrack::setSequenceFPS(double fps)
{
    mAnimation.setSequenceFPS(fps);
    updateWindowTitle();
}

void Petrack::setSequenceFPSDialog()
{
    bool   ok;
    double fps = QInputDialog::getDouble(
        this,
        tr("Set Sequence FPS"),
        tr("Set native FPS of sequence/video (not playback speed):"),
        mAnimation.getSequenceFPS(),
        0,
        100000,
        2,
        &ok);
    if(ok)
    {
        setFPS(fps);
    }
}

void Petrack::about()
{
    auto about = new AboutDialog(
        this,
        mPetrackVersion,
        mGitCommitID,
        mGitCommitDate,
        mGitCommitBranch,
        mCompilerID,
        mCompilerVersion,
        mCompileDate,
        mAuthors);
    about->show();
}


void Petrack::commandLineOptions()
{
    PMessageBox *mb = new PMessageBox{
        this,
        tr("Command line options"),
        commandLineOptionsString,
        QIcon(),
        QString(),
        PMessageBox::StandardButton::Yes};
    mb->setModal(false);
    mb->setAttribute(Qt::WA_DeleteOnClose);
    mb->show();
}

void Petrack::keyBindings()
{
    KeyBindingGroup traj_creation{
        "trajectory creation/manipulation",
        {{"Ctrl + double-click left mouse button", "inserts new or moves trajectory point"},
         {"Shift + double-click left mouse button",
          "inserts new or moves near trajectory point\nand "
          "enables showing only the modified trajectory "},
         {"Alt + holding left mouse button", "moves trajectory point under cursor"},
         {"Ctrl + shift + double-click left mouse button", "splits trajectory before current frame"}}};

    KeyBindingGroup traj_deletion{
        "trajectory deletion",
        {{"Ctrl + double-click right mouse button", "deletes a trajectory"},
         {"Shift + double-click right mouse button", "deletes the past part of a trajectory"},
         {"Alt + double-click right mouse button", "deletes the future part of a trajectory"}}};

    KeyBindingGroup video_navigation{
        "video navigation",
        {{"Space bar", "toggles between pause and last play direction"},
         {"Ctrl + Alt + double-click left mouse button", "jumps to frame of trajectory point under cursor"},
         {"Shift + s", "jumps to start of selected trajectory"},
         {"Mouse scroll wheel",
          "zooms in and out to or from the pixel of the image\nat the position of the mouse pointer"},
         {"Shift + mouse scroll wheel", "plays forwards or backwards frame by frame"},
         {"Holding left mouse button", "moves image"},
         {"Arrow left/Arrow right", "frame back/forward"},
         {"j/l", "jump back/forward"}

        }};

    KeyBindingGroup general{
        "general",
        {{"Ctrl + z", "Undo the last manual action on\ntrajectories (e.g. moving point)"},
         {"Ctrl + Shift + z", "Redo the last action that was reverted\nvia Ctrl + z"},
         {"Shift + t", "toggles tracking online calculation"},
         {"Shift + r", "toggles recognition"},
         {"Shift + e", "export trajectories"},
         {"Shift + a", "toggles \"show only\" or \"show only list\""},
         {"Arrow up/Arrow down", "change the displayed person\n(if show only people enabled)"},
         {"Ctrl + mouse scroll wheel", "change the displayed person\n(if show only people enabled)"}}};

    auto *diag = new KeybindingDialog(this, {general, traj_creation, traj_deletion, video_navigation});
    diag->show();
}

void Petrack::onlineHelp()
{
    static QUrl url("https://go.fzj.de/petrack-docs");
    if(!(QDesktopServices::openUrl(url)))
    {
        PCritical(this, tr("PeTrack"), tr("Cannot open external browser<br>with url ") + url.toString() + "!");
    }
}

void Petrack::antialias()
{
    mView->setRenderHint(QPainter::Antialiasing, mAntialiasAct->isChecked());
}
void Petrack::opengl()
{
    mView->setViewport(mOpenGLAct->isChecked() ? new QOpenGLWidget(mView) : new QWidget);
    // need full viewport update for fade out animation of LogoItem to work
    mView->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
}
void Petrack::reset()
{
    mViewWidget->resetView();
}
void Petrack::fitInView()
{
    mViewWidget->fitInView();
}
void Petrack::fitInROI()
{
    mViewWidget->fitInROI(getRecoRoiItem()->rect());
    // what about trackingRoi???
}
void Petrack::setGlobalFont()
{
    bool  ok;
    QFont font = QFontDialog::getFont(&ok, this->font(), this);
    if(ok)
    {
        this->setFont(font); // font is set to the font the user selected
    }
    else
    {
        // the user canceled the dialog; font is set to the initial
        // value, in this case Times, 12.
    }
}

void Petrack::showHideControlWidget()
{
    // show | hide Control
    mViewWidget->hideControls(mHideControlsAct->isChecked());
}
void Petrack::showLogWindow()
{
    mLogWindow->show();
}
void Petrack::showGroupAnnotationWindow()
{
    mGroupingWidget->show();
}

void Petrack::setCamera()
{
    if(mCameraLeftViewAct->isChecked())
    {
        if((mAnimation.getCamera()) != cameraLeft)
            mAnimation.setCamera(cameraLeft); // war: hier wird direkt bei Umstellung neu gelesen
        else
            return;
    }
    else if(mCameraRightViewAct->isChecked())
    {
        if((mAnimation.getCamera()) != cameraRight)
            mAnimation.setCamera(cameraRight); // war: hier wird direkt bei Umstellung neu gelesen
        else
            return;
    }
    else // kann eigentlich nicht vorkommen
    {
        mAnimation.setCamera(cameraUnset);
        return;
    }
    updateImage(mAnimation.getFrameAtIndex(
        mAnimation.getCurrentFrameNum())); // wird nur aufgerufen, wenn left / right sich geaendert hat
}

/**
 * @brief Helper function to create Actions for the menu bar
 * @see Petrack::createMenus()
 */
void Petrack::createActions()
{
    mOpenSeqAct = new QAction(tr("&Open Sequence"), this);
    mOpenSeqAct->setShortcut(tr("Ctrl+Shift+O"));
    connect(mOpenSeqAct, &QAction::triggered, this, [this] { openSequence(); });

    mOpenCameraAct = new QAction(tr("Open Camera Stream"), this);
    // mOpenCameraAct->setShortcut(tr("Ctrl+C")); // because of some reason it is sometimes fired with
    // Ctrl+LeftMouseButton ==> so disabled (it's also not really needed)
    connect(mOpenCameraAct, &QAction::triggered, this, &Petrack::openCameraLiveStream);

    mOpenMoCapAct = new QAction(tr("Manage MoCap Files"), this);
    connect(mOpenMoCapAct, &QAction::triggered, this, &Petrack::openMoCapFile);

    mEditMoCapAct = new QAction(tr("Edit MoCap Settings"), this);
    connect(mEditMoCapAct, &QAction::triggered, this, &Petrack::editMoCapSettings);

    mExportSeqVidAct = new QAction(tr("Export Video"), this);
    mExportSeqVidAct->setEnabled(false);
    connect(mExportSeqVidAct, &QAction::triggered, this, &Petrack::exportVideo);

    mExportSeqVidViewAct = new QAction(tr("Export View Video"), this);
    mExportSeqVidViewAct->setEnabled(false);
    connect(mExportSeqVidViewAct, &QAction::triggered, this, &Petrack::exportVideoView);

    mExportSeqImgAct = new QAction(tr("Export Image Sequence"), this);
    mExportSeqImgAct->setShortcut(tr("Ctrl+F"));
    mExportSeqImgAct->setEnabled(false);
    connect(mExportSeqImgAct, &QAction::triggered, this, &Petrack::exportImageSequence);

    mExportSeqViewAct = new QAction(tr("Export View Sequence"), this);
    //    mSaveSeqViewAct->setShortcut(tr("Ctrl+F"));
    mExportSeqViewAct->setEnabled(false);
    connect(mExportSeqViewAct, &QAction::triggered, this, &Petrack::exportViewSequence);

    mOpenPrAct = new QAction(tr("&Open Project"), this);
    mOpenPrAct->setShortcut(tr("Ctrl+O"));
    connect(mOpenPrAct, &QAction::triggered, this, [this] { openProject(); });

    mSavePrAct = new QAction(tr("&Save Project As"), this);
    mSavePrAct->setShortcut(tr("Ctrl+Shift+S"));
    connect(mSavePrAct, &QAction::triggered, this, &Petrack::saveProjectAs);

    mSaveAct = new QAction(tr("&Save Project"), this);
    mSaveAct->setShortcut(tr("Ctrl+S"));
    connect(mSaveAct, &QAction::triggered, this, &Petrack::saveSameProject);

    mExportImageAct = new QAction(tr("Export Image"), this);
    mExportImageAct->setShortcut(tr("Ctrl+I"));
    mExportImageAct->setEnabled(false);
    connect(mExportImageAct, &QAction::triggered, this, [this] { exportImage(); });

    mExportViewAct = new QAction(tr("Export View"), this);
    mExportViewAct->setShortcut(tr("Ctrl+V"));
    mExportViewAct->setEnabled(false);
    connect(mExportViewAct, &QAction::triggered, this, [this] { exportView(); });

    mPrintAct = new QAction(tr("&Print"), this);
    mPrintAct->setShortcut(tr("Ctrl+P"));
    mPrintAct->setEnabled(false);
    connect(mPrintAct, &QAction::triggered, this, &Petrack::print);

    mResetSettingsAct = new QAction(tr("&Reset Settings"), this);
    //    mResetSettingsAct->setShortcut(tr("Ctrl+R"));
    mResetSettingsAct->setEnabled(false); // da es noch nicht fehlerfrei funktioniert
    connect(mResetSettingsAct, &QAction::triggered, this, &Petrack::resetSettings);

    mAutosaveSettings = new QAction(tr("Autosave Settings"), this);
    connect(mAutosaveSettings, &QAction::triggered, this, &Petrack::openAutosaveSettings);

    mSetSequenceFPSAct = new QAction(tr("Set Sequence FPS"), this);
    mSetSequenceFPSAct->setEnabled(false);
    mSetSequenceFPSAct->setToolTip(tr("Set native FPS of sequence/video (not playback speed)"));
    connect(mSetSequenceFPSAct, &QAction::triggered, this, &Petrack::setSequenceFPSDialog);

    mExitAct = new QAction(tr("Exit"), this);
    mExitAct->setShortcut(tr("Ctrl+Q"));
    connect(mExitAct, &QAction::triggered, this, &Petrack::close);

    mAntialiasAct = new QAction(tr("&Antialias"), this);
    mAntialiasAct->setShortcut(tr("Ctrl+A"));
    mAntialiasAct->setCheckable(true);
    connect(mAntialiasAct, &QAction::triggered, this, &Petrack::antialias);

    mFontAct = new QAction(tr("&Font"), this);
    connect(mFontAct, &QAction::triggered, this, &Petrack::setGlobalFont);

    mHideControlsAct = new QAction(tr("&Hide controls"), this);
    mHideControlsAct->setShortcut(tr("Ctrl+H"));
    mHideControlsAct->setCheckable(true);
    connect(mHideControlsAct, &QAction::triggered, this, &Petrack::showHideControlWidget);
    connect(mHideControlsAct, &QAction::changed, this, &Petrack::showHideControlWidget);

    mShowLogWindowAct = new QAction(tr("&Show log window"), this);
    connect(mShowLogWindowAct, &QAction::triggered, this, &Petrack::showLogWindow);

    mShowGroupAnnotationWindowAct = new QAction(tr("&Show Group Annotation Window"), this);
    connect(mShowGroupAnnotationWindowAct, &QAction::triggered, this, &Petrack::showGroupAnnotationWindow);

    mCropZoomViewAct = new QAction(tr("&Transform while saving"), this); // Crop and zoom while saving
    mCropZoomViewAct->setCheckable(true);

    mOpenGLAct = new QAction(tr("Open&GL"), this);
    mOpenGLAct->setShortcut(tr("Ctrl+G"));
    mOpenGLAct->setCheckable(true);
    connect(mOpenGLAct, &QAction::triggered, this, &Petrack::opengl);

    mResetAct = new QAction(tr("&Reset"), this);
    mResetAct->setShortcut(tr("Ctrl+R"));
    connect(mResetAct, &QAction::triggered, this, &Petrack::reset);

    mFitViewAct =
        new QAction(tr("Fit in window"), this); // Resize to window; fit in view; show all; in fenster einpassen
    mFitViewAct->setShortcut(tr("Ctrl+0"));
    connect(mFitViewAct, &QAction::triggered, this, &Petrack::fitInView);

    mFitROIAct = new QAction(tr("Fit in region of interest"), this); // Resize ROI to window; fit in view;
    mFitROIAct->setShortcut(tr("Ctrl+1"));
    connect(mFitROIAct, &QAction::triggered, this, &Petrack::fitInROI);

    mCameraGroupView = new QActionGroup(this);
    // mCameraGroupView->addAction(mCameraLeftViewAct);
    // mCameraGroupView->addAction(mCameraRightViewAct);
    mCameraLeftViewAct = new QAction(tr("&Left"), mCameraGroupView);
    mCameraLeftViewAct->setShortcut(tr("Ctrl++Shift+L"));
    mCameraLeftViewAct->setCheckable(true);
    connect(mCameraLeftViewAct, &QAction::triggered, this, &Petrack::setCamera);
    mCameraRightViewAct = new QAction(tr("&Right"), mCameraGroupView);
    mCameraRightViewAct->setShortcut(tr("Ctrl++Shift+R"));
    mCameraRightViewAct->setCheckable(true);
    connect(mCameraRightViewAct, &QAction::triggered, this, &Petrack::setCamera);
    mCameraRightViewAct->setChecked(true); // right wird als default genommen, da reference image in triclops auch
                                           // right ist // erste trj wurden mit left gerechnet

    mLimitPlaybackSpeed = new QAction(tr("&Limit playback speed"));
    // Not checkable like Fix since this is also controlled through clicking on FPS and syncing currently would be
    // bothersome
    connect(
        mLimitPlaybackSpeed,
        &QAction::triggered,
        mPlayerWidget,
        [&]() { mPlayerWidget->setPlayerSpeedLimited(!mPlayerWidget->getPlayerSpeedLimited()); });
    mFixPlaybackSpeed = new QAction(tr("&Fix playback speed"));
    mFixPlaybackSpeed->setCheckable(true);
    connect(mFixPlaybackSpeed, &QAction::toggled, mPlayerWidget, &Player::setPlayerSpeedFixed);
    mSetToRealtime = new QAction(tr("&Realtime"));
    connect(
        mSetToRealtime, &QAction::triggered, mPlayerWidget, [&]() { mPlayerWidget->setSpeedRelativeToRealtime(1.0); });
    mSetTo2p00 = new QAction(tr("&x2"));
    connect(mSetTo2p00, &QAction::triggered, mPlayerWidget, [&]() { mPlayerWidget->setSpeedRelativeToRealtime(2.0); });
    mSetTo1p75 = new QAction(tr("&x1.75"));
    connect(mSetTo1p75, &QAction::triggered, mPlayerWidget, [&]() { mPlayerWidget->setSpeedRelativeToRealtime(1.75); });
    mSetTo1p50 = new QAction(tr("&x1.5"));
    connect(mSetTo1p50, &QAction::triggered, mPlayerWidget, [&]() { mPlayerWidget->setSpeedRelativeToRealtime(1.5); });
    mSetTo1p25 = new QAction(tr("&x1.25"));
    connect(mSetTo1p25, &QAction::triggered, mPlayerWidget, [&]() { mPlayerWidget->setSpeedRelativeToRealtime(1.25); });
    mSetTo0p75 = new QAction(tr("&x0.75"));
    connect(mSetTo0p75, &QAction::triggered, mPlayerWidget, [&]() { mPlayerWidget->setSpeedRelativeToRealtime(0.75); });
    mSetTo0p50 = new QAction(tr("&x0.5"));
    connect(mSetTo0p50, &QAction::triggered, mPlayerWidget, [&]() { mPlayerWidget->setSpeedRelativeToRealtime(0.5); });
    mSetTo0p25 = new QAction(tr("&x0.25"));
    connect(mSetTo0p25, &QAction::triggered, mPlayerWidget, [&]() { mPlayerWidget->setSpeedRelativeToRealtime(0.25); });

    mPlayerLooping = new QAction(tr("&Loop"));
    mPlayerLooping->setCheckable(true);
    connect(mPlayerLooping, &QAction::triggered, mPlayerWidget, &Player::setLooping);
    // -------------------------------------------------------------------------------------------------------

    mUndoAct = new QAction(tr("Undo"), this);
    mUndoAct->setShortcut(QKeySequence("Ctrl+z"));
    connect(
        mUndoAct,
        &QAction::triggered,
        this,
        [&]()
        {
            mPersonStorage.undo();
            updateControlWidget();
            getScene()->views().first()->viewport()->repaint();
        });


    mRedoAct = new QAction{"Redo", this};
    mRedoAct->setShortcut(QKeySequence("Ctrl+shift+z"));
    connect(
        mRedoAct,
        &QAction::triggered,
        this,
        [&]()
        {
            mPersonStorage.redo();
            updateControlWidget();
            getScene()->views().first()->viewport()->repaint();
        });

    mDelPastAct = new QAction(tr("Delete &past part of all trj."), this);
    connect(
        mDelPastAct,
        &QAction::triggered,
        this,
        [this]() { this->deleteTrackPointAll(PersonStorage::TrajectorySegment::Previous); });

    mDelFutureAct = new QAction(tr("Delete &future part of all trj."), this);
    connect(
        mDelFutureAct,
        &QAction::triggered,
        this,
        [this]() { this->deleteTrackPointAll(PersonStorage::TrajectorySegment::Following); });

    mDelAllRoiAct = new QAction(tr("Delete &trj. moving through ROI"), this);
    connect(mDelAllRoiAct, &QAction::triggered, this, &Petrack::deleteTrackPointROI);
    mDelPartRoiAct = new QAction(tr("Delete part of trj. inside &ROI"), this);
    connect(mDelPartRoiAct, &QAction::triggered, this, &Petrack::deleteTrackPointInsideROI);

    // -------------------------------------------------------------------------------------------------------

    mCommandAct = new QAction(tr("&Command line options"), this);
    connect(mCommandAct, &QAction::triggered, this, &Petrack::commandLineOptions);

    mKeyAct = new QAction(tr("&Key bindings"), this);
    connect(mKeyAct, &QAction::triggered, this, &Petrack::keyBindings);

    mAboutAct = new QAction(tr("&About"), this);
    connect(mAboutAct, &QAction::triggered, this, &Petrack::about);

    mOnlineHelpAct = new QAction(tr("Online &Help"), this);
    mOnlineHelpAct->setShortcut(tr("F1"));
    connect(mOnlineHelpAct, &QAction::triggered, this, &Petrack::onlineHelp);
}

/**
 * @brief Helper function building menues out of QActions
 * @see Petrack::createActions()
 */
void Petrack::createMenus()
{
    mFileMenu = new QMenu(tr("&File"), this);
    mFileMenu->addAction(mOpenPrAct);
    mFileMenu->addAction(mSaveAct);
    mFileMenu->addAction(mSavePrAct);
    mFileMenu->addSeparator();
    mFileMenu->addAction(mOpenSeqAct);
    mFileMenu->addAction(mOpenCameraAct);
    mFileMenu->addAction(mSetSequenceFPSAct);
    mFileMenu->addAction(mOpenMoCapAct);
    mFileMenu->addAction(mEditMoCapAct);
    mFileMenu->addAction(mExportSeqVidAct);
    mFileMenu->addAction(mExportImageAct);
    mFileMenu->addAction(mExportSeqImgAct);
    mFileMenu->addAction(mExportSeqVidViewAct);
    mFileMenu->addAction(mExportViewAct);
    mFileMenu->addAction(mExportSeqViewAct);
    mFileMenu->addAction(mPrintAct);
    mFileMenu->addSeparator();
    mFileMenu->addAction(mResetSettingsAct);
    mFileMenu->addAction(mAutosaveSettings);
    mFileMenu->addSeparator();
    mFileMenu->addAction(mExitAct);

    mEditMenu = new QMenu(tr("&Edit"), this);
    mEditMenu->addAction(mUndoAct);
    mEditMenu->addAction(mRedoAct);
    mEditMenu->addAction(mDelPastAct);
    mEditMenu->addAction(mDelFutureAct);
    mEditMenu->addAction(mDelAllRoiAct);
    mEditMenu->addAction(mDelPartRoiAct);


    mViewMenu = new QMenu(tr("&View"), this);
    mViewMenu->addAction(mAntialiasAct);
    mViewMenu->addAction(mOpenGLAct);
    mViewMenu->addAction(mCropZoomViewAct);
    mCameraMenu = mViewMenu->addMenu(tr("&Camera"));
    mCameraMenu->addAction(mCameraLeftViewAct);
    mCameraMenu->addAction(mCameraRightViewAct);
    mViewMenu->addAction(mFixPlaybackSpeed);
    mViewMenu->addAction(mLimitPlaybackSpeed);
    mPlaybackSpeedMenu = mViewMenu->addMenu(tr("&Playback speed"));
    mPlaybackSpeedMenu->addAction(mSetToRealtime);
    mPlaybackSpeedMenu->addAction(mSetTo2p00);
    mPlaybackSpeedMenu->addAction(mSetTo1p75);
    mPlaybackSpeedMenu->addAction(mSetTo1p50);
    mPlaybackSpeedMenu->addAction(mSetTo1p25);
    mPlaybackSpeedMenu->addAction(mSetTo0p75);
    mPlaybackSpeedMenu->addAction(mSetTo0p50);
    mPlaybackSpeedMenu->addAction(mSetTo0p25);
    mViewMenu->addAction(mPlayerLooping);
    mViewMenu->addSeparator();
    mViewMenu->addAction(mFitViewAct);
    mViewMenu->addAction(mFitROIAct);
    mViewMenu->addAction(mResetAct);
    mViewMenu->addSeparator();
    mViewMenu->addAction(mFontAct);
    mViewMenu->addSeparator();
    mViewMenu->addAction(mHideControlsAct);
    mViewMenu->addSeparator();
    mViewMenu->addAction(mShowLogWindowAct);
    mViewMenu->addAction(mShowGroupAnnotationWindowAct);


    mHelpMenu = new QMenu(tr("&Help"), this);
    mHelpMenu->addAction(mCommandAct);
    mHelpMenu->addAction(mKeyAct);
    mHelpMenu->addAction(mAboutAct);
    mHelpMenu->addAction(mOnlineHelpAct);

    menuBar()->addMenu(mFileMenu);
    menuBar()->addMenu(mEditMenu);
    menuBar()->addMenu(mViewMenu);
    menuBar()->addMenu(mHelpMenu);

    mCameraMenu->setEnabled(false);
}

/**
 * @brief Helper function to create status bar at the bottom of the window
 */
void Petrack::createStatusBar()
{
    QFont f("Courier", 12, QFont::Bold); // Times Helvetica, Normal
    statusBar()->setMaximumHeight(28);
    statusBar()->showMessage(tr("Ready"));
    statusBar()->addPermanentWidget(mStatusLabelStereo = new QLabel(" "));
    statusBar()->addPermanentWidget(mStatusLabelTime = new QLabel(" "));
    statusBar()->addPermanentWidget(mStatusLabelFPS = new QLabel(" "));
    statusBar()->addPermanentWidget(mStatusPosRealHeight = new QDoubleSpinBox());
    connect(
        mStatusPosRealHeight,
        QOverload<double>::of(&QDoubleSpinBox::valueChanged),
        this,
        static_cast<void (Petrack::*)()>(&Petrack::setStatusPosReal));

    statusBar()->addPermanentWidget(mStatusLabelPosReal = new QLabel(" "));
    statusBar()->addPermanentWidget(mStatusLabelPos = new QLabel(" "));
    statusBar()->addPermanentWidget(mStatusLabelColor = new QLabel(" "));
    mStatusLabelStereo->setFont(f);
    mStatusLabelStereo->setMinimumWidth(200);
    mStatusLabelTime->setFont(f);
    mStatusLabelTime->setMinimumWidth(200);
    mStatusLabelFPS->setFont(f);
    mStatusLabelFPS->setMinimumWidth(80);
    mStatusLabelFPS->setAutoFillBackground(true);
    mStatusLabelFPS->setToolTip("Click to adapt play rate to fps rate");
    mStatusPosRealHeight->setRange(-999.9, 9999.9); // in cm
    mStatusPosRealHeight->setDecimals(1);
    mStatusPosRealHeight->setFont(f);
    mStatusLabelPosReal->setFont(f);
    mStatusLabelPosReal->setMinimumWidth(340);
    mStatusLabelPos->setFont(f);
    mStatusLabelPos->setMinimumWidth(100);
    mStatusLabelColor->setFont(f);
    mStatusLabelColor->setMinimumWidth(90);
    mStatusLabelColor->setAutoFillBackground(true);
}

void Petrack::resetUI()
{
    /// ToDo:
    ///
    ///  Reset all UI elements to default settings
    ///  Noetig damit alle UI Elemente, welche in der neu geladenen Projekt-Datei z.B. noch nicht vorhanden sind,
    ///  auf sinnvolle Werte gesetzt werden. Anderenfalls kommt es evtl. beim nacheinander laden verschiedener
    ///  Projekte zu einem Programmabsturz
    ///
    return;
}

void Petrack::setStatusStereo(float x, float y, float z)
{
    if(mStatusLabelStereo)
    {
        if(z < 0)
        {
            mStatusLabelStereo->setText(QString("x= novalue  y= novalue  z= novalue  "));
        }
        else
        {
            mStatusLabelStereo->setText(
                QString("x=%1cm  y=%2cm  z=%3cm  ").arg(x, 6, 'f', 1).arg(y, 6, 'f', 1).arg(z, 6, 'f', 1));
        }
    }
}

void Petrack::setStatusTime()
{
    if(mStatusLabelTime)
    {
        mStatusLabelTime->setText(mAnimation.getTimeString());
    }
}

void Petrack::setStatusFPS()
{
    if(mStatusLabelFPS)
    {
        mStatusLabelFPS->setText(QString("%1fps  ").arg(mShowFPS, 5, 'f', 1));

        QPalette pal = mStatusLabelFPS->palette(); // static moeglich?
        QColor   color;

        double diff    = mShowFPS - mAnimation.getPlaybackFPS();
        int    opacity = mPlayerWidget->getPlayerSpeedLimited() ? 128 : 20;

        if(diff < -6) // very slow ==> red
        {
            color.setRgb(200, 0, 0, opacity);
        }
        else if(diff < -2) // better ==> yellow
        {
            color.setRgb(200, 200, 0, opacity);
        }
        else if(diff > -2) // nearly ok ==> green
        {
            color.setRgb(0, 200, 0, opacity);
        }

        pal.setColor(QPalette::Window, color);

        mStatusLabelFPS->setPalette(pal);
    }
}
void Petrack::setShowFPS(double fps)
{
    if((fps == 0.) || (mShowFPS == 0))
    {
        mShowFPS = fps;
    }
    else
    {
        mShowFPS = mShowFPS * .9 + fps * .1; // glaetten durch Hinzunahme des alten Wertes
    }
    setStatusFPS();
}

/**
 * @brief Updates the FPS shown to the User
 *
 * This method calculates the FPS by remembering how long
 * it has been since it was called last time. If skipped is
 * true, it doesn't directly update the FPS since 2
 * skipped frames have essentially a time delay of 0 between
 * them, which would make calculations wonky.
 *
 * @param skipped True, if this is a skipped frame; default false
 */
void Petrack::updateShowFPS(bool skipped)
{
    static QElapsedTimer lastTime;
    static int           skippedFrames = 0;

    if(skipped)
    {
        skippedFrames++;
        return;
    }

    if(mPlayerWidget->getPaused())
    {
        setShowFPS(0.);
        lastTime.invalidate();
    }
    else
    {
        if(lastTime.isValid())
        {
            if(lastTime.elapsed() > 0)
            {
                int numFrames = skippedFrames > 0 ? skippedFrames + 1 : 1;
                setShowFPS(numFrames * 1000. / lastTime.elapsed());
                skippedFrames = 0;
            }
        }
        lastTime.start();
    }
}

// ohne neue positionsangabe, sinnvoll, wenn berechnungsweise sich in getPosReal geaendert hat
// gebraucht in control.cpp
void Petrack::setStatusPosReal() // pos in cm
{
    if(mImageItem)
    {
        setStatusPosReal(mWorldImageCorrespondence->getPosReal(mMousePosOnImage, getStatusPosRealHeight()));
    }
}

void Petrack::setStatusPosReal(const QPointF &pos) // pos in cm
{
    if(mStatusLabelPosReal)
    {
        QChar   deg(0xB0);
        QString labelText = QString(" cm from ground:%1cm,%2cm,%3")
                                .arg(pos.x(), 6, 'f', 1)
                                .arg(pos.y(), 6, 'f', 1)
                                .arg(
                                    mWorldImageCorrespondence->getAngleToGround(
                                        mMousePosOnImage.x(), mMousePosOnImage.y(), getStatusPosRealHeight()),
                                    5,
                                    'f',
                                    1);
        labelText.append(deg);
        mStatusLabelPosReal->setText(labelText);
    }
}

void Petrack::setStatusPos(const QPoint &pos) // pos in pixel
{
    mStatusLabelPos->setText(QString("%1x%2").arg(pos.x(), 4).arg(pos.y(), 4));
}

void Petrack::setStatusColor(const QRgb &col)
{
    QString s("#%1%2%3"); // static moeglich?
    s = s.arg(qRed(col), 2, 16, QChar('0')).arg(qGreen(col), 2, 16, QChar('0')).arg(qBlue(col), 2, 16, QChar('0'));
    if((qRed(col) + qGreen(col) + qBlue(col)) / 3 < 128)
    {
        mStatusLabelColor->setText(QString("<font color=\"#ffffff\">&nbsp;%1</font>").arg(s));
    }
    else
    {
        mStatusLabelColor->setText(QString("<font color=\"#000000\">&nbsp;%1</font>").arg(s));
    }

    QPalette pal = mStatusLabelColor->palette(); // static moeglich?
    QColor   color(qRed(col), qGreen(col), qBlue(col));

    pal.setColor(QPalette::Window, color);
    mStatusLabelColor->setPalette(pal);

    mControlWidget->getColorPlot()->setCursor(color);
    mControlWidget->getColorPlot()->replot();
}

void Petrack::setStatusColor()
{
    QPointF pos = getMousePosOnImage();
    if(pos.x() >= 0 && pos.x() < mImage->width() && pos.y() > 0 && pos.y() < mImage->height())
    {
        setStatusColor(mImage->pixel(pos.toPoint()));
    }
}

double Petrack::getStatusPosRealHeight()
{
    if(mStatusPosRealHeight)
    {
        return mStatusPosRealHeight->value();
    }
    else
    {
        return 0.;
    }
}

/**
 * @brief Reads (and applies) settings from platform-independent persistent storage
 *
 * The saved size and position of the application window get reconstructed. As well as
 * the options about antialiasing and the usage of OpenGL.
 * mSeqFileName and mProFileName get set, so the "Open Project" and "Open Sequence"
 * dialogues start at correct folder. The old project/sequence is NOT actually loaded.
 */
void Petrack::readSettings()
{
    const QSettings settings("Forschungszentrum Juelich GmbH", "PeTrack by Maik Boltes, Daniel Salden");
    mAntialiasAct->setChecked(settings.value("antialias", false).toBool());
    mOpenGLAct->setChecked(settings.value("opengl", false).toBool());
    mSeqFileName = settings.value("seqFileName", QDir::currentPath()).toString();
    setProFileName(settings.value("proFilePath", QDir::currentPath()).toString());
    // nicht ganz sauber, da so immer schon zu anfang in calib file list etwas drin steht und somit auto ausgefuehrt
    // werden kann wird aber beim ersten openCalib... ueberschrieben
    mAutoCalib.setLastDir(settings.value("calibFile", QDir::currentPath()).toString());
    auto geometry = settings.value("geometry").toByteArray();
    restoreGeometry(geometry);
    antialias();
    opengl();
    mSplitter->restoreState(settings.value("controlSplitterSizes").toByteArray());
    mAutosave.setPetSaveInterval(settings.value("petSaveInterval", 120).toDouble());
    mAutosave.setChangesTillAutosave(settings.value("changesTillAutosave", 10).toInt());
}

/**
 * @brief Writes persistent setting.
 * @see Petrack::readSettings
 */
void Petrack::writeSettings()
{
    QSettings settings("Forschungszentrum Juelich GmbH", "PeTrack by Maik Boltes, Daniel Salden");
    settings.setValue("geometry", saveGeometry());
    settings.setValue("antialias", mAntialiasAct->isChecked());
    settings.setValue("opengl", mOpenGLAct->isChecked());
    settings.setValue("seqFileName", mSeqFileName);
    settings.setValue("proFilePath", QFileInfo(mProFileName).path()); // nur path, damit bei saveCurrentProject
    if(!mAutoCalib.isEmptyCalibFiles())                               //! mCalibFiles.isEmpty()
    {
        settings.setValue("calibFile", mAutoCalib.getCalibFile(0));
    }
    settings.setValue("controlSplitterSizes", mSplitter->saveState());
    settings.setValue("petSaveInterval", mAutosave.getPetSaveInterval());
    settings.setValue("changesTillAutosave", mAutosave.getChangesTillAutosave());
}

bool Petrack::maybeSave()
{
    int ret = PWarning(
        this,
        tr("PeTrack"),
        tr("Do you want to save "
           "the current project?\n"
           "Be sure to save trajectories, background "
           "and 3D calibration point separately!"),
        PMessageBox::StandardButton::Yes | PMessageBox::StandardButton::No | PMessageBox::StandardButton::Cancel,
        PMessageBox::StandardButton::Yes);

    if(ret == PMessageBox::StandardButton::Yes)
    {
        if(saveSameProject())
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    else if(ret == PMessageBox::StandardButton::Cancel)
    {
        return false;
    }
    else
    {
        return true;
    }
}

void Petrack::closeEvent(QCloseEvent *event)
{
    if(maybeSave())
    {
        writeSettings();
        mAutosave.deleteAutosave();
        mPlayerWidget->pause();
        event->accept();
    }
    else
    {
        event->ignore();
    }
}

/**
 * @brief Sets the mMousePosOnImage member variable and displayed pixel/real coordinates
 *
 * Gets called from ImageItem::hoverMoveEvent() and enables an easy access
 * to the mouse position.
 * @param pos Position of mouse cursor in image pixel coordinates
 */
void Petrack::setMousePosOnImage(QPointF pos)
{
    if(mImage)
    {
        mMousePosOnImage = pos;
        setStatusPosReal(mWorldImageCorrespondence->getPosReal(pos, getStatusPosRealHeight()));

        // pixel coordinate
        QPoint pos1((int) (pos.x()) + 1, (int) (pos.y()) + 1);
        setStatusPos(pos1);

        // pixel color
        setStatusColor();
    }
}

void Petrack::keyPressEvent(QKeyEvent *event)
{
    switch(event->key())
    {
        case Qt::Key_Left:
            mPlayerWidget->frameBackward();
            break;
        case Qt::Key_Right:
            mPlayerWidget->frameForward();
            break;
        case Qt::Key_L:
            mPlayerWidget->queueJumpSeconds(3);
            break;
        case Qt::Key_J:
            mPlayerWidget->queueJumpSeconds(-3);
            break;
        case Qt::Key_Down:
            mControlWidget->setTrackShowOnlyNr(mControlWidget->getTrackShowOnlyNr() - 1);
            break;
        case Qt::Key_Up:
            mControlWidget->setTrackShowOnlyNr(mControlWidget->getTrackShowOnlyNr() + 1);
            break;
        case Qt::Key_Space:
            // space wird von buttons, wenn focus drauf ist als Aktivierung vorher abgegriffen und nicht
            // durchgereicht
            mPlayerWidget->togglePlayPause();
            break;
        case Qt::Key_D:
            break;
        default:;
    }
}

void Petrack::mousePressEvent(QMouseEvent *event)
{
    // mouse click in fps status label ?
    if(event->pos().x() >= mStatusLabelFPS->pos().x() &&
       event->pos().x() <= mStatusLabelFPS->pos().x() + mStatusLabelFPS->width())
    {
        mPlayerWidget->togglePlayerSpeedLimited();
        setStatusFPS();
    }
}

const QString &Petrack::getLastTrackerExport() const
{
    return mLastTrackerExport;
}

void Petrack::setLastTrackerExport(const QString &newLastTrackerExport)
{
    mLastTrackerExport = newLastTrackerExport;
}

/// update control widget, if image size changed (especially because of changing border)
void Petrack::updateControlImage(cv::Mat &img)
{
    // auch moeglich hoehe und breite von bild stat border veraenderungen zu checken
    static int lastBorderSize = -1;

    if(isLoading())
    {
        lastBorderSize = -1;
    }
    int diffBorderSize = 0;
    if(lastBorderSize != -1)
    {
        diffBorderSize = getImageBorderSize() - lastBorderSize;
    }
    lastBorderSize = getImageBorderSize();

    const int imgWidth  = img.cols;
    const int imgHeight = img.rows;

    mControlWidget->imageSizeChanged(imgWidth, imgHeight, diffBorderSize);
}

void Petrack::importTracker(QString dest) // default = ""
{
    static QString lastFile;

    if(lastFile == "")
    {
        lastFile = mTrcFileName;
    }

    // if no destination file or folder is given
    if(dest.isEmpty())
    {
        dest = QFileDialog::getOpenFileName(
            this,
            tr("Select file for importing tracking pathes"),
            lastFile,
            tr("PeTrack tracker (*.trc *.txt);;All files (*.*)"));
    }

    if(!dest.isEmpty())
    {
        if(dest.endsWith(".trc", Qt::CaseInsensitive))
        {
            QFile file(dest);
            int   i, sz;

            if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
            {
                // errorstring ist je nach Betriebssystem in einer anderen Sprache!!!!
                PCritical(this, tr("PeTrack"), tr("Cannot open %1:\n%2").arg(dest).arg(file.errorString()));
                return;
            }

            setTrackChanged(true); // flag changes of track parameters
            mTracker->reset();

            QTextStream in(&file);
            QString     comment;

            bool    ok; // shows if int stands in first line - that was in the first version of trc file
            QString firstLine = in.readLine();
            sz                = firstLine.toInt(&ok);
            if(!ok)
            {
                if(firstLine.contains("version 4", Qt::CaseInsensitive))
                {
                    trcVersion = 4;
                }
                else if(firstLine.contains("version 3", Qt::CaseInsensitive))
                {
                    trcVersion = 3;
                }
                else if(firstLine.contains("version 2", Qt::CaseInsensitive))
                {
                    trcVersion = 2;
                }
                else
                {
                    SPDLOG_ERROR("wrong header while reading TRC file.");
                    QMessageBox::critical(
                        this,
                        tr("PeTrack"),
                        tr("Could not import tracker:\nNot supported trc version in file: %1.").arg(dest));
                    return;
                }
                in >> sz;
            }
            else
            {
                trcVersion = 1;
            }

            if((sz > 0) && (mPersonStorage.nbPersons() != 0))
            {
                SPDLOG_WARN("overlapping trajectories will be joined not until tracking adds new TrackPoints.");
            }
            for(i = 0; i < sz; ++i)
            {
                TrackPerson tp = fromTrc(in);
                mPersonStorage.addPerson(tp);
            }

            mControlWidget->setTrackNumberAll(QString("%1").arg(mPersonStorage.nbPersons()));
            mControlWidget->setTrackShowOnlyNr(static_cast<int>(MAX(mPersonStorage.nbPersons(), 1)));
            mControlWidget->setTrackNumberVisible(
                QString("%1").arg(mPersonStorage.visible(mAnimation.getCurrentFrameNum())));
            mControlWidget->replotColorplot();
            file.close();
            SPDLOG_INFO("import {} ({} person(s), file version {})", dest, sz, trcVersion);
            mTrcFileName =
                dest; // fuer Project-File, dann koennte track path direkt mitgeladen werden, wenn er noch da ist
        }
        else if(dest.endsWith(".txt", Qt::CaseInsensitive)) // 3D Koordinaten als Tracking-Daten importieren
                                                            // Zeilenformat: Personennr, Framenr, x, y, z
        {
            PWarning(
                this,
                tr("PeTrack"),
                tr("Are you sure you want to import 3D data from TXT-File? You have to make sure that the "
                   "coordinate "
                   "system now is exactly at the same position and orientation than at export time!"));

            QFile file(dest);

            int numberImportedPersons = 0;

            if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
            {
                // errorstring ist je nach Betriebssystem in einer anderen Sprache!!!!
                PCritical(this, tr("PeTrack"), tr("Cannot open %1:\n%2").arg(dest).arg(file.errorString()));
                return;
            }

            setTrackChanged(true); // flag changes of track parameters
            mTracker->reset();

            QTextStream in(&file);
            TrackPoint  tPoint;

            QString line;
            bool    unitFound            = false;
            double  conversionFactorToCM = 1.0;

            std::map<int, std::map<int, Vec3F>> personData;
            QString                             headerline;

            while(in.readLineInto(&line))
            {
                if(line.startsWith("#", Qt::CaseInsensitive))
                {
                    headerline = line;
                    continue;
                }

                if((!unitFound) && (!headerline.contains("cm")))
                {
                    conversionFactorToCM = 100.0;
                    unitFound            = true;
                    PWarning(
                        this,
                        tr("PeTrack"),
                        tr("PeTrack will interpret position data as unit [m]. No header with [cm] found."));
                }

                // Read data from line with format: persNr frameNr x y z
                int   personNr = -1, frameNr = -1;
                float x, y, z;

                QTextStream stream(&line);
                stream >> personNr >> frameNr >> x >> y >> z;

                // convert data to cm
                x = x * conversionFactorToCM;
                y = y * conversionFactorToCM;
                z = z * conversionFactorToCM;

                if(personData[personNr].find(frameNr) == personData[personNr].end())
                {
                    personData[personNr][frameNr] = Vec3F(x, y, z);
                }
                else
                {
                    PCritical(
                        this,
                        "Error importing txt file",
                        tr("Could not import the data from the provided txt file, as the data for person %1 in "
                           "frame "
                           "%1 is twice in the txt-file.")
                            .arg(personNr)
                            .arg(frameNr));
                    return;
                }
            }

            for(auto &[persNr, frameData] : personData)
            {
                std::deque<TrackPoint> pixelPoints;
                for(auto &[frameNr, realWorldCoordinates] : frameData)
                {
                    cv::Point2f p2d;

                    if(mControlWidget->getCalibCoordDimension() == 0)
                    {
                        // compute image point from 3d calibration
                        p2d = mExtrCalibration.getImagePoint(
                            cv::Point3f(realWorldCoordinates.x(), realWorldCoordinates.y(), realWorldCoordinates.z()));
                    }
                    else
                    {
                        // compute image point from 2d calibration
                        QPointF pos = mWorldImageCorrespondence->getPosImage(
                            QPointF(realWorldCoordinates.x(), realWorldCoordinates.y()), realWorldCoordinates.z());
                        p2d.x = pos.x();
                        p2d.y = pos.y();
                    }

                    TrackPoint trackPoint(Vec2F(p2d.x, p2d.y), 100);
                    trackPoint.setSp(
                        realWorldCoordinates.x(),
                        realWorldCoordinates.y(),
                        -mControlWidget->getExtrinsicParameters().trans3 -
                            realWorldCoordinates.z()); // distance to camera as with stereo cameras
                    pixelPoints.push_back(trackPoint);
                }

                TrackPerson trackPerson(persNr, frameData.begin()->first, pixelPoints.front());
                trackPerson.setHeight(frameData.begin()->second.z());
                pixelPoints.pop_front();

                for(const auto &trackPoint : pixelPoints)
                {
                    trackPerson.append(trackPoint);
                }
                mPersonStorage.addPerson(trackPerson);
                numberImportedPersons++;
            }

            mControlWidget->setTrackNumberAll(QString("%1").arg(mPersonStorage.nbPersons()));
            mControlWidget->setTrackShowOnlyNr(static_cast<int>(MAX(mPersonStorage.nbPersons(), 1)));
            mControlWidget->setTrackNumberVisible(
                QString("%1").arg(mPersonStorage.visible(mAnimation.getCurrentFrameNum())));
            mControlWidget->replotColorplot();
            file.close();
            SPDLOG_INFO("import {} ({} person(s))", dest, numberImportedPersons);
            mTrcFileName = dest;
        }
        else
        {
            PCritical(this, tr("PeTrack"), tr("Cannot load %1 maybe because of wrong file extension.").arg(dest));
        }
        lastFile = dest;
    }
}

int Petrack::calculateRealTracker()
{
    bool autoCorrectOnlyExport = (mReco.getRecoMethod() == reco::RecognitionMethod::MultiColor) && // multicolor
                                 mMultiColorMarkerWidget->autoCorrect->isChecked() &&
                                 mMultiColorMarkerWidget->autoCorrectOnlyExport->isChecked();
    int anz = mTrackerReal->calculate(
        this,
        mTracker,
        mWorldImageCorrespondence,
        mControlWidget->getColorPlot(),
        mMissingFrames,
        getImageBorderSize(),
        mControlWidget->getAnaMissingFrames(),
        mStereoWidget->stereoUseForExport->isChecked(),
        mControlWidget->getTrackAlternateHeight(),
        mControlWidget->getCameraAltitude(),
        mStereoWidget->stereoUseCalibrationCenter->isChecked(),
        mControlWidget->isExportElimTpChecked(),
        mControlWidget->isExportElimTrjChecked(),
        mControlWidget->isExportSmoothChecked(),
        mControlWidget->isExportViewDirChecked(),
        mControlWidget->isExportAngleOfViewChecked(),
        mControlWidget->isExportMarkerIDChecked(),
        autoCorrectOnlyExport);

    mTrackerReal->calcMinMax();
    return anz;
}


void Petrack::exportTracker(QString dest) // default = ""
{
    try
    {
        if(!mTracker)
        {
            return;
        }


        // if no destination file or folder is given
        if(dest.isEmpty())
        {
            QFileDialog fileDialog(
                this,
                tr("Select file for exporting tracking paths"),
                mLastTrackerExport,
                tr("Tracker (*.*);;Petrack tracker (*.trc);;Text (*.txt);;Text for gnuplot(*.dat);;XML Travisto "
                   "(*.trav);;All supported types (*.txt *.trc *.dat *.trav *.);;All files (*.*)"));
            fileDialog.setAcceptMode(QFileDialog::AcceptSave);
            fileDialog.setFileMode(QFileDialog::AnyFile);
            fileDialog.setDefaultSuffix("");

            if(fileDialog.exec())
            {
                dest = fileDialog.selectedFiles().at(0);
            }

            if(dest.isEmpty())
            {
                return;
            }
        }

        QList<int> pers, frame;
        bool autoCorrectOnlyExport = (mReco.getRecoMethod() == reco::RecognitionMethod::MultiColor) && // multicolor
                                     mMultiColorMarkerWidget->autoCorrect->isChecked() &&
                                     mMultiColorMarkerWidget->autoCorrectOnlyExport->isChecked();

        if(dest.endsWith(".trc", Qt::CaseInsensitive))
        {
            QTemporaryFile file;

            if(!file.open() /*!file.open(QIODevice::WriteOnly | QIODevice::Text)*/)
            {
                PCritical(this, tr("PeTrack"), tr("Cannot open %1:\n%2.").arg(dest).arg(file.errorString()));
                return;
            }
            QProgressDialog progress(
                "Export TRC-File", nullptr, 0, static_cast<int>(mPersonStorage.nbPersons() + 1), this->window());
            progress.setWindowTitle("Export .trc-File");
            progress.setWindowModality(Qt::WindowModal);
            progress.setVisible(true);
            progress.setValue(0);
            progress.setLabelText(QString("Export tracking data ..."));

            qApp->processEvents();

            trcVersion = 4;

            SPDLOG_INFO(
                "export tracking data to {} ({} person(s), file version {})",
                dest,
                mPersonStorage.nbPersons(),
                trcVersion);
            QTextStream out(&file);

            out << "version " << trcVersion << Qt::endl;
            out << mPersonStorage.nbPersons() << Qt::endl;
            const auto &persons = mPersonStorage.getPersons();
            for(size_t i = 0; i < persons.size(); ++i)
            {
                qApp->processEvents();
                progress.setLabelText(QString("Export person %1 of %2 ...").arg(i + 1).arg(mPersonStorage.nbPersons()));
                progress.setValue(static_cast<int>(i + 1));
                out << persons[i] << Qt::endl;
            }
            file.flush();
            file.close();

            progress.setLabelText(QString("Save file ..."));
            qApp->processEvents();

            if(QFile::exists(dest))
            {
                QFile::remove(dest);
            }

            if(!file.copy(dest))
            {
                PCritical(
                    this,
                    tr("PeTrack"),
                    tr("Could not export tracking data.\n"
                       "Please try again!"));
            }
            else
            {
                statusBar()->showMessage(tr("Saved tracking data to %1.").arg(dest), 5000);
            }

            progress.setValue(static_cast<int>(mPersonStorage.nbPersons() + 1));

            SPDLOG_INFO("finished.");
            mAutosave.resetTrackPersonCounter();

            mTrcFileName =
                dest; // fuer Project-File, dann koennte track path direkt mitgeladen werden, wenn er// noch da ist
        }
        else if(dest.endsWith(".txt", Qt::CaseInsensitive))
        {
            QTemporaryFile file;

            if(!file.open())
            {
                PCritical(this, tr("PeTrack"), tr("Cannot open %1:\n%2.").arg(dest).arg(file.errorString()));
                return;
            }

            SPDLOG_INFO("export tracking data to {} ({} person(s))...", dest, mPersonStorage.nbPersons());

            // recalcHeight true, wenn personenhoehe ueber trackpoints neu berechnet werden soll (z.b. um
            // waehrend play mehrfachberuecksichtigung von punkten auszuschliessen, aenderungen in altitude neu
            // in berechnung einfliessen zu lassen)
            if(mControlWidget->isTrackRecalcHeightChecked())
            {
                if(mControlWidget->getCalibCoordDimension() == 0) // 3D
                {
                    ; // Nothing to be done because z already the right height
                }
                else // 2D
                {
                    mPersonStorage.recalcHeight(mControlWidget->getCameraAltitude());
                }
            }

            mTrackerReal->calculate(
                this,
                mTracker,
                mWorldImageCorrespondence,
                mControlWidget->getColorPlot(),
                mMissingFrames,
                getImageBorderSize(),
                mControlWidget->isTrackMissingFramesChecked(),
                mStereoWidget->stereoUseForExport->isChecked(),
                mControlWidget->getTrackAlternateHeight(),
                mControlWidget->getCameraAltitude(),
                mStereoWidget->stereoUseCalibrationCenter->isChecked(),
                mControlWidget->isExportElimTpChecked(),
                mControlWidget->isExportElimTrjChecked(),
                mControlWidget->isExportSmoothChecked(),
                mControlWidget->isExportViewDirChecked(),
                mControlWidget->isExportAngleOfViewChecked(),
                mControlWidget->isExportMarkerIDChecked(),
                autoCorrectOnlyExport);

            QTextStream out(&file);

            out << "# PeTrack project: " << QFileInfo(getProFileName()).fileName() << Qt::endl;
            out << "# raw trajectory file: " << QFileInfo(getTrackFileName()).fileName() << Qt::endl;
            out << "# framerate: " << mAnimation.getSequenceFPS() << " fps" << Qt::endl;

            if(mControlWidget->isExportCommentChecked())
            {
                out << "# personal information:" << Qt::endl;
                out << "# ID| Comment" << Qt::endl;

                // std out
                SPDLOG_INFO("Printing comment table...");
                SPDLOG_INFO("ID  | Comment");
                SPDLOG_INFO("----|----------------");

                for(int i = 0; i < static_cast<int>(mPersonStorage.nbPersons()); ++i)
                {
                    auto commentSplit = mPersonStorage.at(i).comment().split("\n", Qt::KeepEmptyParts);
                    out << "#" << qSetFieldWidth(3) << (i + 1) << qSetFieldWidth(0) << "|" << commentSplit.at(0)
                        << Qt::endl;
                    SPDLOG_INFO("{:04d}|{}", (i + 1), commentSplit.at(0));
                    commentSplit.pop_front();
                    for(const QString &line : commentSplit)
                    {
                        out << "#" << qSetFieldWidth(3) << " " << qSetFieldWidth(0) << "|" << line << Qt::endl;
                        SPDLOG_INFO("    |{}", line);
                    }
                }
            }
            mTrackerReal->exportTxt(
                out,
                mControlWidget->getTrackAlternateHeight(),
                mStereoWidget->stereoUseForExport->isChecked(),
                mControlWidget->isExportViewDirChecked(),
                mControlWidget->isExportAngleOfViewChecked(),
                mControlWidget->isExportUseMeterChecked(),
                mControlWidget->isExportMarkerIDChecked());
            // out << *mTrackerReal;
            file.flush();
            file.close();

            if(QFile::exists(dest))
            {
                QFile::remove(dest);
            }

            if(!file.copy(dest))
            {
                PCritical(
                    this,
                    tr("PeTrack"),
                    tr("Could not export tracking data.\n"
                       "Please try again!"));
            }
            else
            {
                statusBar()->showMessage(tr("Saved tracking data to %1.").arg(dest), 5000);
            }

            SPDLOG_INFO("finished");
        }
        else if(dest.endsWith(".h5", Qt::CaseInsensitive))
        {
            try
            {
                mTrackerReal->calculate(
                    this,
                    mTracker,
                    mWorldImageCorrespondence,
                    mControlWidget->getColorPlot(),
                    mMissingFrames,
                    getImageBorderSize(),
                    mControlWidget->isTrackMissingFramesChecked(),
                    mStereoWidget->stereoUseForExport->isChecked(),
                    mControlWidget->getTrackAlternateHeight(),
                    mControlWidget->getCameraAltitude(),
                    mStereoWidget->stereoUseCalibrationCenter->isChecked(),
                    mControlWidget->isExportElimTpChecked(),
                    mControlWidget->isExportElimTrjChecked(),
                    mControlWidget->isExportSmoothChecked(),
                    mControlWidget->isExportViewDirChecked(),
                    mControlWidget->isExportAngleOfViewChecked(),
                    mControlWidget->isExportMarkerIDChecked(),
                    autoCorrectOnlyExport);
                mTrackerReal->exportHdf5(
                    dest,
                    mAnimation.getSequenceFPS(),
                    mControlWidget->getTrackAlternateHeight(),
                    mStereoWidget->stereoUseForExport->isChecked(),
                    mControlWidget->isExportViewDirChecked(),
                    mControlWidget->isExportAngleOfViewChecked(),
                    mControlWidget->isExportMarkerIDChecked(),
                    mControlWidget->isExportCommentChecked());
            }
            catch(std::runtime_error &e)
            {
                PCritical(this, tr("PeTrack"), tr(e.what()));
            }
        }
        else if(dest.endsWith(".dat", Qt::CaseInsensitive))
        {
            QTemporaryFile fileDat;

            if(!fileDat.open()) //! fileDat.open(QIODevice::WriteOnly | QIODevice::Text))
            {
                PCritical(this, tr("PeTrack"), tr("Cannot open %1:\n%2.").arg(dest).arg(fileDat.errorString()));
                return;
            }
            // recalcHeight true, wenn personenhoehe ueber trackpoints neu berechnet werden soll (z.b. um
            // waehrend play mehrfachberuecksichtigung von punkten auszuschliessen, aenderungen in altitude neu
            // in berechnung einfliessen zu lassen)
            if(mControlWidget->isTrackRecalcHeightChecked())
            {
                mPersonStorage.recalcHeight(mControlWidget->getCameraAltitude());
            }
            mTrackerReal->calculate(
                this,
                mTracker,
                mWorldImageCorrespondence,
                mControlWidget->getColorPlot(),
                mMissingFrames,
                getImageBorderSize(),
                mControlWidget->isTrackMissingFramesChecked(),
                mStereoWidget->stereoUseForExport->isChecked(),
                mControlWidget->getTrackAlternateHeight(),
                mControlWidget->getCameraAltitude(),
                mStereoWidget->stereoUseCalibrationCenter->isChecked(),
                mControlWidget->isExportElimTpChecked(),
                mControlWidget->isExportElimTrjChecked(),
                mControlWidget->isExportSmoothChecked(),
                mControlWidget->isExportViewDirChecked(),
                mControlWidget->isExportAngleOfViewChecked(),
                mControlWidget->isExportMarkerIDChecked(),
                autoCorrectOnlyExport);

            SPDLOG_INFO("export tracking data to {} ({} person(s))...", dest, mPersonStorage.nbPersons());
            QTextStream outDat(&fileDat);
            mTrackerReal->exportDat(
                outDat, mControlWidget->getTrackAlternateHeight(), mStereoWidget->stereoUseForExport->isChecked());
            fileDat.flush();
            fileDat.close();

            if(QFile::exists(dest))
            {
                QFile::remove(dest);
            }

            if(!fileDat.copy(dest))
            {
                PCritical(
                    this,
                    tr("PeTrack"),
                    tr("Could not export tracking data.\n"
                       "Please try again!"));
            }
            else
            {
                statusBar()->showMessage(tr("Saved tracking data to %1.").arg(dest), 5000);
            }

            SPDLOG_INFO("finished");
        }
        else if(dest.endsWith(".trav", Qt::CaseInsensitive))
        {
            // recalcHeight true, wenn personenhoehe ueber trackpoints neu berechnet werden soll (z.b. um
            // waehrend play mehrfachberuecksichtigung von punkten auszuschliessen, aenderungen in altitude neu
            // in berechnung einfliessen zu lassen)
            if(mControlWidget->isTrackRecalcHeightChecked())
            {
                mPersonStorage.recalcHeight(mControlWidget->getCameraAltitude());
            }

            mTrackerReal->calculate(
                this,
                mTracker,
                mWorldImageCorrespondence,
                mControlWidget->getColorPlot(),
                mMissingFrames,
                getImageBorderSize(),
                mControlWidget->isTrackMissingFramesChecked(),
                mStereoWidget->stereoUseForExport->isChecked(),
                mControlWidget->getTrackAlternateHeight(),
                mControlWidget->getCameraAltitude(),
                mStereoWidget->stereoUseCalibrationCenter->isChecked(),
                mControlWidget->isExportElimTpChecked(),
                mControlWidget->isExportElimTrjChecked(),
                mControlWidget->isExportSmoothChecked(),
                mControlWidget->isExportViewDirChecked(),
                mControlWidget->isExportAngleOfViewChecked(),
                mControlWidget->isExportMarkerIDChecked(),
                autoCorrectOnlyExport);

            QTemporaryFile fileXml;
            if(!fileXml.open()) //! fileXml.open(QIODevice::WriteOnly | QIODevice::Text))
            {
                PCritical(this, tr("PeTrack"), tr("Cannot open %1:\n%2.").arg(dest).arg(fileXml.errorString()));
                return;
            }
            SPDLOG_INFO("export tracking data to {} ({} person(s))...", dest, mPersonStorage.nbPersons());
            // already done: mTrackerReal->calculate(mTracker, mImageItem, mControlWidget->getColorPlot(),
            // getImageBorderSize(), mControlWidget->trackMissingFrames->checkState());
            QTextStream outXml(&fileXml);
            outXml << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << Qt::endl;
            outXml << "<trajectoriesDataset>" << Qt::endl;
            outXml << "    <header version=\"1.0\">" << Qt::endl;
            outXml << "        <roomCaption>PeTrack: " << mAnimation.getFileBase() << "</roomCaption>" << Qt::endl;
            outXml << "        <roomID>0</roomID>" << Qt::endl;
            outXml << "        <agents>" << mPersonStorage.nbPersons() << "</agents>" << Qt::endl;
            outXml << "        <frameRate>" << mAnimation.getSequenceFPS() << "</frameRate> <!--per second-->"
                   << Qt::endl;
            // outXml << "        <timeStep>" << 1000./mAnimation.getPlaybackFPS() << "</timeStep>   <!--
            // millisecond-->"
            // << endl; inverse von
            outXml << "        <timeFirstFrame sec=\"" << mAnimation.getFirstFrameSec() << "\" microsec=\""
                   << mAnimation.getFirstFrameMicroSec() << "\"/> <!-- " << mAnimation.getTimeString(0) << " -->"
                   << Qt::endl;
            outXml << "    </header>" << Qt::endl << Qt::endl;

            mTrackerReal->exportXml(
                outXml, mControlWidget->getTrackAlternateHeight(), mStereoWidget->stereoUseForExport->isChecked());

            outXml << "</trajectoriesDataset>" << Qt::endl;
            fileXml.flush();
            fileXml.close();

            if(QFile::exists(dest))
            {
                QFile::remove(dest);
            }

            if(!fileXml.copy(dest))
            {
                PCritical(
                    this,
                    tr("PeTrack"),
                    tr("Could not export tracking data.\n"
                       "Please try again!"));
            }
            else
            {
                statusBar()->showMessage(tr("Saved tracking data to %1.").arg(dest), 5000);
            }

            SPDLOG_INFO("finished");
        }
        else
        { // wenn keine Dateiendung, dann wird trc und txt herausgeschrieben
            exportTracker(dest + ".trc");
            exportTracker(dest + ".txt");
        }
        mLastTrackerExport = dest;
    }
    catch(const std::runtime_error &error)
    {
        PCritical(this, "Failed to export trajectories", error.what());
    }
}

// fuer anschliessende groessenberechnung
void Petrack::playAll()
{
    int memPos  = mPlayerWidget->getPos();
    int progVal = 0;

    QProgressDialog progress("Playing whole sequence...", "Abort playing", 0, mAnimation.getNumFrames(), this);
    progress.setWindowModality(Qt::WindowModal); // blocks main window

    // vorwaertslaufen ab aktueller Stelle und trackOnlineCalc zum tracken nutzen
    do
    {
        progress.setValue(++progVal); // mPlayerWidget->getPos()
        qApp->processEvents();
        if(progress.wasCanceled())
        {
            break;
        }
    } while(mPlayerWidget->frameForward());

    mPlayerWidget->skipToFrame(memPos);
}

/**
 * @brief Activates tracking and reco; calcs through the video (in both ways)
 *
 * This method activates tracking and reco and plays the whole video (from current
 * frame on) till the end. Then, if mAutoBackTrack is set, it jumps back to the
 * largest first frame, i.e. the last time a new person was added/recognized, and
 * tracks backwards till the beginning of the video.
 *
 * The old settings for tracking and reco will be restored. No interaction with the
 * main window is possible for the time of tracking.
 */
void Petrack::trackAll()
{
    int  memPos        = mPlayerWidget->getPos();
    int  progVal       = 0;
    bool memCheckState = mControlWidget->isOnlineTrackingChecked();
    bool memRecoState  = mControlWidget->isPerformRecognitionChecked();

    mControlWidget->setOnlineTrackingChecked(true);
    mControlWidget->setPerformRecognitionChecked(true);

    QProgressDialog progress(
        "Tracking pedestrians through all frames...",
        "Abort tracking",
        0,
        2 * mAnimation.getNumFrames() - memPos,
        this);
    progress.setWindowModality(Qt::WindowModal); // blocks main window

    // vorwaertslaufen ab aktueller Stelle und trackOnlineCalc zum tracken nutzen
    do
    {
        progress.setValue(++progVal); // mPlayerWidget->getPos()
        qApp->processEvents();
        if(progress.wasCanceled())
        {
            break;
        }
    } while(mPlayerWidget->frameForward());

    if(mAutoBackTrack)
    {
        // zuruecksprinegn an die stelle, wo der letzte trackPath nicht vollstaendig
        // etwas spaeter, da erste punkte in reco path meist nur ellipse ohne markererkennung
        mControlWidget->setOnlineTrackingChecked(false);
        mPlayerWidget->skipToFrame(mPersonStorage.largestFirstFrame() + 5);
        mControlWidget->setOnlineTrackingChecked(true);
        // progVal = 2mAnimation.getNumFrames()-memPos-mPlayerWidget->getPos();
        progVal += mAnimation.getNumFrames() - mPlayerWidget->getPos();
        progress.setValue(progVal); // mPlayerWidget->getPos()

        // recognition abstellen, bis an die stelle, wo trackAll begann
        // UEBERPRUEFEN, OB TRACKPATH NICHT RECOGNITION PUNKTE UEBERSCHREIBT!!!!!!!!!!
        // repeate und repaetQual koennte temporaer umgestellt werden
        mControlWidget->setPerformRecognitionChecked(false);

        // rueckwaertslaufen
        do
        {
            if(progVal + 1 < 2 * mAnimation.getNumFrames() - memPos)
            {
                progress.setValue(++progVal); // mPlayerWidget->getPos()
            }
            qApp->processEvents();
            if(progress.wasCanceled())
            {
                break;
            }
            if(mPlayerWidget->getPos() == memPos + 1)
            {
                mControlWidget->setPerformRecognitionChecked(true);
            }
        } while(mPlayerWidget->frameBackward());

        // bei abbruch koennen es auch mPlayerWidget->getPos() frames sein, die bisher geschrieben wurden
        progress.setValue(2 * mAnimation.getNumFrames() - memPos);
    }

    if(mAutoTrackOptimizeColor)
    {
        mPersonStorage.optimizeColor();
    }

    mControlWidget->setPerformRecognitionChecked(memRecoState);
    mControlWidget->setOnlineTrackingChecked(false);
    mPlayerWidget->skipToFrame(memPos);
    mControlWidget->setOnlineTrackingChecked(memCheckState);
}

// default: (QPointF *pos=NULL, int pers=-1, int frame=-1);
int Petrack::winSize(QPointF *pos, int pers, int frame, int level)
{
    // default of mControlWidget->trackRegionScale->value() is 16, so that
    // a factor of 1.6 of the headsize is used
    if(level == -1)
    {
        level = mControlWidget->getTrackRegionLevels();
    }
    return (int) ((getHeadSize(pos, pers, frame) / pow(2., level)) * (mControlWidget->getTrackRegionScale() / 10.));
}

/**
 * Calculates the filtered image based on the values of different filters.
 * They are passed as parameters, because once read, they return false when checking the changed()-method.
 *
 * @param imageChanged bool, if the image has changed since the last usage
 */
void Petrack::getFilteredImage(
    bool imageChanged,
    bool brightContrastFilterChanged,
    bool swapFilterChanged,
    bool borderFilterChanged,
    bool calibFilterChanged)
{
    mImgFiltered = mImg;

    // When applying the filter, the order is important!
    // Computation heavy filter should be applied early.

    if(imageChanged || swapFilterChanged)
    {
        mImgFiltered = mSwapFilter.apply(mImgFiltered);
    }
    else
    {
        mImgFiltered = mSwapFilter.getLastResult();
    }

    if(imageChanged || swapFilterChanged || brightContrastFilterChanged)
    {
        mImgFiltered = mBrightContrastFilter.apply(mImgFiltered);
    }
    else
    {
        mImgFiltered = mBrightContrastFilter.getLastResult();
    }

    if(imageChanged || swapFilterChanged || brightContrastFilterChanged || borderFilterChanged)
    {
        mImgFiltered = mBorderFilter.apply(mImgFiltered);
    }
    else
    {
        mImgFiltered = mBorderFilter.getLastResult();
    }

    if(borderFilterChanged)
    {
        updateControlImage(mImgFiltered);
    }

    if(imageChanged || swapFilterChanged || brightContrastFilterChanged || borderFilterChanged || calibFilterChanged)
    {
        if(mStereoContext)
            mStereoContext->init(mImgFiltered);
    }

    if(imageChanged || swapFilterChanged || brightContrastFilterChanged || borderFilterChanged || calibFilterChanged)
    {
        if(mStereoContext)
        {
            // getRecified rectifies filtered image set in mStereoContext->init()
            mImgFiltered = mStereoContext->getRectified(mAnimation.getCamera());
            mCalibFilter.setChanged(false);
        }
        else
        {
            mImgFiltered = mCalibFilter.apply(mImgFiltered);
        }
    }
    else
    {
        // TODO: need to handle this for the stereo case??
        mImgFiltered = mCalibFilter.getLastResult();
    }

    if(brightContrastFilterChanged || swapFilterChanged || borderFilterChanged || calibFilterChanged)
    {
        // when loading a .pet file, a bg-file may be there.
        // Only delete the bg-filter if no such file is present
        if(mBackgroundFilter.getFilename().isEmpty())
        {
            // delete all background information and set bg.changed() to true.
            mBackgroundFilter.reset();
        }
        else
        {
            SPDLOG_WARN("no background reset, because of explicit loaded background image!");
        }
    }

    if(imageChanged || mBackgroundFilter.changed())
    {
        mImgFiltered = mBackgroundFilter.apply(mImgFiltered);
    }
    else
    {
        mImgFiltered = mBackgroundFilter.getLastResult();
    }
}

void Petrack::resetExistingPoints()
{
    mPersonStorage.clear();
    mTracker->reset();
    if(!isLoading())
    {
        SPDLOG_WARN("deleted all tracking pathes because intrinsic parameters have changed.");
    }
}

/**
 * Perform the tracking step based on the current image and the Tracker's previous feature points
 * @see Tracker::track
 */
void Petrack::performTracking()
{
    // Rect for tracking area
    QRect roi(
        myRound(mTrackingRoiItem->rect().x() + getImageBorderSize()),
        myRound(mTrackingRoiItem->rect().y() + getImageBorderSize()),
        myRound(mTrackingRoiItem->rect().width()),
        myRound(mTrackingRoiItem->rect().height()));

    // build disparity picture if it should be used for height detection
    if(mStereoContext && mStereoWidget->stereoUseForHeight->isChecked())
    {
        mStereoContext->getDisparity();
    }

    cv::Rect rect = qRectToCvRect(roi, mImgFiltered);

    cv::Mat map1 = mCalibFilter.getMap1();
    int     anz  = mTracker->track(
        mImgFiltered,
        rect,
        map1,
        mAnimation.getCurrentFrameNum(),
        mControlWidget->isTrackRepeatChecked(),
        mControlWidget->getTrackRepeatQual(),
        getImageBorderSize(),
        mReco.getRecoMethod(),
        mControlWidget->getTrackRegionLevels(),
        getPedestriansToTrack());

    mControlWidget->setTrackNumberNow(QString("%1").arg(anz));
    mTrackChanged = false;
}

/**
 * Perform the recognition (if enabled by the user) of markers in the current image.
 *
 * All TrackPoints will be added to the personStorage.
 */
void Petrack::performRecognition()
{
    int  frameNum        = mAnimation.getCurrentFrameNum();
    bool isStereoContext = mStereoContext != nullptr;

    // build disparity picture if it should be used for height detection or recognition
    if(isStereoContext &&
       (mStereoWidget->stereoUseForHeight->isChecked() || mStereoWidget->stereoUseForReco->isChecked()))
    {
        // won't recalculate if already done during tracking
        mStereoContext->getDisparity();
    }

    if(mControlWidget->isPerformRecognitionChecked())
    {
        QRect rect(
            myRound(mRecognitionRoiItem->rect().x() + getImageBorderSize()),
            myRound(mRecognitionRoiItem->rect().y() + getImageBorderSize()),
            myRound(mRecognitionRoiItem->rect().width()),
            myRound(mRecognitionRoiItem->rect().height()));
        QList<TrackPoint>     persList;
        [[maybe_unused]] bool markerLess = true;
        auto                  recoMethod = mReco.getRecoMethod();

        if((recoMethod == reco::RecognitionMethod::Casern) || (recoMethod == reco::RecognitionMethod::Hermes) ||
           (recoMethod == reco::RecognitionMethod::Color) || (recoMethod == reco::RecognitionMethod::Japan) ||
           (recoMethod == reco::RecognitionMethod::MultiColor) || (recoMethod == reco::RecognitionMethod::Code) ||
           (recoMethod == reco::RecognitionMethod::MachineLearning))
        {
            persList = mReco.getMarkerPos(
                mImgFiltered,
                rect,
                mControlWidget,
                getImageBorderSize(),
                getBackgroundFilter(),
                mControlWidget->getIntrinsicCameraParams());
            markerLess = false;
        }
        if(isStereoContext && mStereoWidget->stereoUseForReco->isChecked())
        {
            PersonList pl;
            pl.calcPersonPos(mImgFiltered, rect, persList, mStereoContext, getBackgroundFilter(), markerLess);
        }

        mPersonStorage.addPoints(persList, frameNum, mReco.getRecoMethod());

        if(isStereoContext && mStereoWidget->stereoUseForReco->isChecked())
        {
            mPersonStorage.purge(frameNum);
        }

        mControlWidget->setRecoNumberNow(QString("%1").arg(persList.size()));
        mRecognitionChanged = false;
    }
    else
    {
        mControlWidget->setRecoNumberNow(QString("0"));
    }
}

/**
 * Update the image that petrack shows.
 * This will not only change the shown image, but also run tracking and recognition if there are changes in the shown
 * image.
 * Calling this method without an actual change in the shown image may be useful to run distortion or drawing
 * tracking border
 *
 * @param imageChanged specify if the image has actually changed (a new animation frame is shown). Defaults to false
 * @return true if updateImage updated the image successfully immediately, false otherwise (call got queued or failed)
 *
 */
bool Petrack::updateImage(bool imageChanged)
{
    mCodeMarkerItem->resetSavedMarkers();

    static int  lastRecoFrame            = -10000;
    static bool borderChangedForTracking = false;

    // need mutex to guarantee that updateImage only called once at a time
    // updateValue of control automatically calls updateImage!!!
    static QMutex mutex;
    if(!mImg.empty() && mImage)
    {
        if(!mutex.tryLock())
        {
            // add the call to the event queue to make sure no valid update is skipped
            QMetaObject::invokeMethod(
                this, [this, imageChanged] { updateImage(imageChanged); }, Qt::QueuedConnection);
            return false;
        }

        int frameNum = mAnimation.getCurrentFrameNum();

        setStatusTime();

        updateShowFPS();


        // have to store because evaluation sets the filter parameter to unchanged
        bool brightContrastChanged = mBrightContrastFilter.changed();
        bool swapChanged           = mSwapFilter.changed();
        bool borderChanged         = mBorderFilter.changed();
        bool calibChanged          = mCalibFilter.changed();

        getFilteredImage(imageChanged, brightContrastChanged, swapChanged, borderChanged, calibChanged);

        // delete track list, if intrinsic param have changed
        if(calibChanged && mPersonStorage.nbPersons() > 0) // mCalibFilter.getEnabled() &&
        {
            resetExistingPoints();
        }
        else
        {
            // calculate position in 3D space and height of person for "old" trackPoints, if checked "even"
            if(mStereoContext && mStereoWidget->stereoUseForHeightEver->isChecked() &&
               mStereoWidget->stereoUseForHeight->isChecked())
            {
                // build disparity picture if it should be used for height detection
                mStereoContext->getDisparity();

                mPersonStorage.calcPosition(frameNum);
            }
        }
        if(borderChanged)
        {
            borderChangedForTracking = true;
        }
        // tracking before recognition, because new recognized points are checked to match with already tracked ones
        if((trackChanged() || imageChanged) && (mControlWidget->isOnlineTrackingChecked()))
        {
            if(borderChangedForTracking)
            {
                cv::Size size;
                size.width  = mImgFiltered.cols;
                size.height = mImgFiltered.rows;
                mTracker->resize(size);

                mTrackingRoiItem->restoreSize();
            }
            borderChangedForTracking = false;

            performTracking();
        }
        else
        {
            mControlWidget->setTrackNumberNow(QString("0"));
        }

        bool recoFrameCondition =
            ((((lastRecoFrame + mControlWidget->getRecoStep()) <= frameNum) ||
              ((lastRecoFrame - mControlWidget->getRecoStep()) >= frameNum)) &&
             imageChanged);

        if(recoFrameCondition || mAnimation.isCameraLiveStream() || swapChanged || brightContrastChanged ||
           borderChanged || calibChanged || recognitionChanged())
        {
            if(borderChanged)
            {
                mRecognitionRoiItem->restoreSize();
            }
            lastRecoFrame = frameNum;

            performRecognition();
        }
        else
        {
            mControlWidget->setRecoNumberNow(QString("0"));
        }

        // these might change due to reco or tracking
        mControlWidget->setTrackNumberAll(QString("%1").arg(mPersonStorage.nbPersons()));
        mControlWidget->setTrackShowOnlyNrMaximum(static_cast<int>(MAX(mPersonStorage.nbPersons(), 1)));
        mControlWidget->setTrackNumberVisible(QString("%1").arg(mPersonStorage.visible(frameNum)));

        copyToQImage(*mImage, mImgFiltered);

        if(borderChanged)
        {
            mImageItem->setImage(mImage);
        }
        else
        {
            getScene()->views().first()->viewport()->repaint();
            qApp->processEvents();
            // update pixel color (because image pixel moves)
            setStatusColor();
        }

#ifdef QWT
        mControlWidget->getAnalysePlot()->setActFrame(frameNum);
        if(mControlWidget->isAnaMarkActChecked())
        {
            mControlWidget->getAnalysePlot()->replot();
        }
#endif

        mutex.unlock();
        return true;
    }
    return false;
}

bool Petrack::updateImage(const cv::Mat &img)
{
    mImg = img;
    return updateImage(true);
}


void Petrack::updateSequence()
{
    QImage *oldImage = mImage;

    QSize size = mAnimation.getSize();
    if(size != QSize{0, 0})
    {
        size.setWidth(size.width() + 2 * getImageBorderSize()); // border is inside the mImage!
        size.setHeight(size.height() + 2 * getImageBorderSize());
    }
    mImage = new QImage(size, QImage::Format_RGB888);


    if(size == QSize{0, 0})
    {
        // separate handling because border should currently be ignored
        // will be applied when a sequence is loaded in
        mRecognitionRoiItem->setRect(0, 0, 0, 0);
        mTrackingRoiItem->setRect(0, 0, 0, 0);
    }
    else
    {
        const bool imageSizeChanged = oldImage && (oldImage->rect() != mImage->rect());
        // set roi for recognition if image size changes or roi is zero
        if((mRecognitionRoiItem->rect().width() == 0) || imageSizeChanged)
        {
            mRecognitionRoiItem->setRect(
                -getImageBorderSize(), -getImageBorderSize(), mImage->width(), mImage->height());
        }

        if((mTrackingRoiItem->rect().width() == 0) || imageSizeChanged)
        {
            mTrackingRoiItem->setRect(-getImageBorderSize(), -getImageBorderSize(), mImage->width(), mImage->height());
        }
    }

    cv::Size size2;
    size2.width  = mTrackingRoiItem->rect().width();
    size2.height = mTrackingRoiItem->rect().height();
    mTracker->init(size2);

    mPlayerWidget->setAnim(&mAnimation);
    mPlayerWidget->skipToFrame(0);
    mImageItem->setImage(mImage); // wird in updateImage gemacht
    delete oldImage;
    mExportSeqVidAct->setEnabled(true);
    mExportSeqVidViewAct->setEnabled(true);
    mExportSeqImgAct->setEnabled(true);
    mExportSeqViewAct->setEnabled(true);
    mExportImageAct->setEnabled(true);
    mExportViewAct->setEnabled(true);
    mSetSequenceFPSAct->setEnabled(true);
    mPrintAct->setEnabled(true);
    mResetSettingsAct->setEnabled(true);
}


/**
 * @brief Gets cm per pixel. Only recalculates when calculating head size.
 * @return cm per pixel
 */
double Petrack::getCmPerPixel() const
{
    return mCmPerPixel;
}

/**
 * @brief Sets the size of the circle of the average head circumference in pixel.
 *
 * Assumption for default calculation: <br>
 * 21cm avg head length <br>
 * default height of person according to mapDefaultHeigt <br>
 *
 * Default case recalculates mCmPerPixel
 *
 * If headsize get recalulated also mCmPerPixel will be calculated!
 * @see Petrack::getCmPerPixel
 * @param hS new headsize, if hS==-1 mHeadSize will be calculated instead of set
 */
void Petrack::setHeadSize(double hS)
{
    if(hS == -1)
    {
        mCmPerPixel = mWorldImageCorrespondence->getCmPerPixel();
        // debout << mCmPerPixel <<endl;
        mHeadSize = (HEAD_SIZE * mControlWidget->getCameraAltitude() /
                     (mControlWidget->getCameraAltitude() - mControlWidget->getDefaultHeight())) /
                    mCmPerPixel;
    }
    else
    {
        mHeadSize = hS;
    }
}
// gibt Kopfgroesse zurueck
// default: (QPointF *pos=NULL, int pers=-1, int frame=-1)
double Petrack::getHeadSize(QPointF *pos, int pers, int frame)
{
    double z, h;

    if((pers >= 0) && (pers < static_cast<int>(mPersonStorage.nbPersons())) &&
       mPersonStorage.at(pers).trackPointExist(frame))
    {
        if(mControlWidget->getCalibCoordDimension() == 0)
        {
            int         diff;
            cv::Point3f p3d = getExtrCalibration()->get3DPoint(
                cv::Point2f(
                    mPersonStorage.at(pers).trackPointAt(frame).x(), mPersonStorage.at(pers).trackPointAt(frame).y()),
                mControlWidget->getDefaultHeight());

            cv::Point2f p3d_x1 =
                getExtrCalibration()->getImagePoint(cv::Point3f(p3d.x + HEAD_SIZE * 0.5, p3d.y, p3d.z));
            cv::Point2f p3d_x2 =
                getExtrCalibration()->getImagePoint(cv::Point3f(p3d.x - HEAD_SIZE * 0.5, p3d.y, p3d.z));
            cv::Point2f p3d_y1 =
                getExtrCalibration()->getImagePoint(cv::Point3f(p3d.x, p3d.y + HEAD_SIZE * 0.5, p3d.z));
            cv::Point2f p3d_y2 =
                getExtrCalibration()->getImagePoint(cv::Point3f(p3d.x, p3d.y - HEAD_SIZE * 0.5, p3d.z));

            diff = (int) std::max(
                sqrt(pow(p3d_x2.x - p3d_x1.x, 2) + pow(p3d_x2.y - p3d_x1.y, 2)),
                sqrt(pow(p3d_y2.x - p3d_y1.x, 2) + pow(p3d_y2.y - p3d_y1.y, 2)));
            return diff; // < 8 ? 8 : diff;
        }
        else
        {
            z = mPersonStorage.at(pers).trackPointAt(frame).sp().z();
            h = mPersonStorage.at(pers).height();
            if(z > 0)
            {
                return (HEAD_SIZE * mControlWidget->getCameraAltitude() / z) /
                       mWorldImageCorrespondence->getCmPerPixel();
            }
            else if(h > MIN_HEIGHT)
            {
                return (HEAD_SIZE * mControlWidget->getCameraAltitude() / (mControlWidget->getCameraAltitude() - h)) /
                       mWorldImageCorrespondence->getCmPerPixel();
            }
            else
            {
                return mHeadSize;
            }
        }
    }

    if(pos != nullptr)
    {
        return mHeadSize; // muss noch aus density map gelesen werden!!!
    }
    else //(pos == NULL) && (pers == -1)
    {
        return mHeadSize;
    }
}

void Petrack::setProFileName(const QString &fileName)
{
    // don't change project Name to an autosave
    if(mAutosave.isAutosave(fileName) || fileName == mProFileName)
    {
        return;
    }

    // Change project => delete old autosave
    mAutosave.deleteAutosave();
    // NOTE: Use only the global variant in future?
    // global one in helper.h because it is needed to use getFileList and shouldn't depend on Petrack
    proFileName  = fileName;
    mProFileName = fileName;
    updateWindowTitle();
}

const WorldImageCorrespondence &Petrack::getWorldImageCorrespondence()
{
    return *mWorldImageCorrespondence;
}

/**
 * @brief Return the user's selection of pedestrians/trajectories
 *
 * Only people selected via "show only people" (single person) or "show only
 * people list"(multiple persons) are going to be selected.
 *
 * @return All user selected pedestrian (empty for all pedestrians)
 */
QSet<size_t> Petrack::getPedestrianUserSelection()
{
    if(mControlWidget->isTrackShowOnlyChecked())
    {
        QSet<size_t> onlyVisible;
        // subtraction needed as in UI ID start at 1 and internally at 0
        onlyVisible.insert(mControlWidget->getTrackShowOnlyNr() - 1);
        return onlyVisible;
    }
    if(mControlWidget->isTrackShowOnlyListChecked())
    {
        auto enteredIDs = util::splitStringToInt(mControlWidget->trackShowOnlyNrList()->text());
        if(enteredIDs.has_value())
        {
            QSet<size_t> selectedIDs;
            for(auto id : enteredIDs.value())
            {
                // subtraction needed as in UI ID start at 1 and internally at 0
                selectedIDs.insert(id - 1);
            }
            mControlWidget->trackShowOnlyNrList()->setStyleSheet("");
            return selectedIDs;
        }
        else
        {
            mControlWidget->trackShowOnlyNrList()->setStyleSheet("border: 1px solid red");
        }
    }
    return QSet<size_t>();
}

/**
 * @brief Splits the given text to get a set of integers.
 *
 * The given text will be split on ',' and then each element will be checked if it is a range. Ranges are marked
 * with
 * '-' as divider. Only positive integer values are allowed.
 *
 * Examples:
 * '1,5,6' -> (1, 5, 6)
 * '1-5' -> (1, 2, 3, 4, 5)
 *
 * @param input given text
 * @return Set of int in the given text
 */
std::optional<QSet<int>> util::splitStringToInt(const QString &input)
{
    QSet<int> ids;

    for(const auto &id : input.split(",", Qt::SkipEmptyParts))
    {
        bool ok        = false;
        int  enteredID = id.toInt(&ok);
        if(ok && enteredID >= 0) // parse single values
        {
            ids.insert(enteredID);
        }
        else // error or IDs range (e.g. 1-3, 6-10, etc.)
        {
            if(id.startsWith("-"))
            {
                ok = false;
            }
            auto range = id.split("-");
            int  first = range[0].toInt(&ok);
            ok         = ok && range.size() == 2 && !range[1].isEmpty();
            if(ok)
            {
                int last = range[1].toInt(&ok);
                if(ok)
                {
                    if(first > last)
                    {
                        std::swap(first, last);
                    }

                    for(int i = first; i <= last; i++)
                    {
                        ids.insert(i);
                    }
                }
            }
        }
        if(!ok)
        {
            return std::nullopt;
        }
    }
    return ids;
}

/**
 * @brief Checks which pedestrians/trajectories are selected for evaluation.
 *
 * If "only for selected" is checked, then only selected people (@see
 * Petrack::getPedestrianUserSelection()) are going to be tracked, all people
 * otherwise.
 *
 * @return all trajectories which should be evaluated; empty when all should be evaluated
 */
QSet<size_t> Petrack::getPedestriansToTrack()
{
    if(mControlWidget->isTrackOnlySelectedChecked())
    {
        return getPedestrianUserSelection();
    }

    return QSet<size_t>();
}

void Petrack::addManualTrackPointOnlyVisible(const QPointF &pos)
{
    int pers = addOrMoveManualTrackPoint(pos) + 1;
    if(pers == 0)
    {
        pers = static_cast<int>(mPersonStorage.nbPersons()) + 1;
    }
    mControlWidget->setTrackShowOnlyNr(pers);
    mControlWidget->setTrackShowOnly(Qt::Checked);
}

void Petrack::updateControlWidget()
{
    mControlWidget->setTrackNumberAll(QString("%1").arg(mPersonStorage.nbPersons()));
    mControlWidget->setTrackShowOnlyNrMaximum(static_cast<int>(MAX(mPersonStorage.nbPersons(), 1)));
    mControlWidget->setTrackNumberVisible(QString("%1").arg(mPersonStorage.visible(mAnimation.getCurrentFrameNum())));
}

void Petrack::splitTrackPerson(QPointF pos)
{
    mPersonStorage.splitPersonAt((Vec2F) pos, mAnimation.getCurrentFrameNum(), getPedestrianUserSelection());
    updateControlWidget();
}

/**
 * @brief Lets the user add or move a TrackPoint manually
 *
 * There is an check inside addPoint which inhibits adding a point,
 * if only selected trajectories are visualized, since one wouldn't
 * see the newly added TrackPoint.
 *
 * @param pos pixel position of mouse on image
 * @return index of person whose point was moved; -1 if failed or new trajectory is started
 */
int Petrack::addOrMoveManualTrackPoint(const QPointF &pos)
{
    int        pers = -1;
    TrackPoint tP(Vec2F{pos}, 110); // 110 is higher than 100 (max. quality) and gets clamped to 100 after insertion
    // allows replacemet of every point (check for better quality always passes)
    mPersonStorage.addPoint(
        tP, mAnimation.getCurrentFrameNum(), getPedestrianUserSelection(), mReco.getRecoMethod(), &pers);
    updateControlWidget();
    return pers;
}

/**
 * Deletes trajectory based on current frame and selected trajectories.
 * @see PersonStorage::delPoint
 */
void Petrack::deleteTrackPoint(QPointF pos, PersonStorage::TrajectorySegment direction) // const QPoint &pos
{
    mPersonStorage.delPoint((Vec2F) pos, direction, mAnimation.getCurrentFrameNum(), getPedestrianUserSelection());
    updateControlWidget();
}
void Petrack::editTrackPersonComment(QPointF pos)
{
    mPersonStorage.editTrackPersonComment((Vec2F) pos, mAnimation.getCurrentFrameNum(), getPedestrianUserSelection());
    updateControlWidget();
}
void Petrack::setTrackPersonHeight(QPointF pos)
{
    mPersonStorage.setTrackPersonHeight((Vec2F) pos, mAnimation.getCurrentFrameNum(), getPedestrianUserSelection());
    updateControlWidget();
}
void Petrack::resetTrackPersonHeight(QPointF pos)
{
    mPersonStorage.resetTrackPersonHeight((Vec2F) pos, mAnimation.getCurrentFrameNum(), getPedestrianUserSelection());
    updateControlWidget();
}

/**
 * @brief Delete the following, previous or whole trajectory of **all** trajectories
 * @param direction previous, following or whole
 */
void Petrack::deleteTrackPointAll(PersonStorage::TrajectorySegment direction) // const QPoint &pos
{
    mPersonStorage.delPointAll(direction, mAnimation.getCurrentFrameNum());
    updateControlWidget();
}

void Petrack::deleteTrackPointROI()
{
    mPersonStorage.delPointROI();
    updateControlWidget();
    mScene->update();
}

void Petrack::deleteTrackPointInsideROI()
{
    getPersonStorage().delPointInsideROI();
    updateControlWidget();
    mScene->update();
}

void Petrack::moveTrackPoint(QPointF pos)
{
    mManualTrackPointMover.moveTrackPoint(pos, mPersonStorage);
    mScene->update();
}

void Petrack::selectPersonForMoveTrackPoint(QPointF pos)
{
    FrameRange range;
    range.before  = mControlWidget->getTrackShowBefore();
    range.after   = mControlWidget->getTrackShowAfter();
    range.current = mPlayerWidget->getPos();
    if(mControlWidget->isTrackShowComplPathChecked())
    {
        // range covers whole trajectory
        range.before = range.current;
        range.after  = mAnimation.getNumFrames() - range.current;
    }
    auto successfullySelected =
        mManualTrackPointMover.selectTrackPoint(pos, mPersonStorage, getPedestrianUserSelection(), range);

    if(successfullySelected)
    {
        mPersonStorage.onManualAction();
        setCursor(QCursor{Qt::CursorShape::DragMoveCursor});
    }
}

void Petrack::releaseTrackPoint()
{
    mManualTrackPointMover.setTrackPoint();
    setCursor(QCursor{});
}

void Petrack::scrollShowOnly(int delta)
{
    if(delta < 0)
    {
        mControlWidget->setTrackShowOnlyNr(mControlWidget->getTrackShowOnlyNr() - 1);
    }
    else
    {
        mControlWidget->setTrackShowOnlyNr(mControlWidget->getTrackShowOnlyNr() + 1);
    }
}

void Petrack::updateSourceInOutFrames()
{
    mPlayerWidget->setFrameInNum(mAnimation.getSourceInFrameNum());
    mPlayerWidget->setFrameOutNum(mAnimation.getSourceOutFrameNum());
}

// delta gibt menge an Umdrehungen und richtung an
void Petrack::skipToFrameWheel(int delta)
{
    mPlayerWidget->skipToFrame(mPlayerWidget->getPos() + delta);
}

void Petrack::skipToFrameFromTrajectory(QPointF pos)
{
    auto      peds      = getPedestrianUserSelection();
    int       before    = mControlWidget->getTrackShowBefore();
    int       after     = mControlWidget->getTrackShowAfter();
    const int currFrame = mPlayerWidget->getPos();
    if(mControlWidget->isTrackShowComplPathChecked())
    {
        // range covers whole trajectory
        before = currFrame;
        after  = mAnimation.getNumFrames() - currFrame;
    }
    FrameRange frameRange{before, after, currFrame};

    auto res = mPersonStorage.getProximalPersons(pos, peds, frameRange);

    if(res.size() == 1)
    {
        mPlayerWidget->skipToFrame(res.front().frame);
    }
    else if(res.size() > 1)
    {
        PWarning(
            this,
            tr("Too many trajectories"),
            tr("PeTrack can't determine which point you meant. Try selecting fewer trajectories first."));
    }
}

void Petrack::openAutosaveSettings()
{
    AutosaveSettings *autosaveSettings =
        new AutosaveSettings(mAutosave.getPetSaveInterval(), mAutosave.getChangesTillAutosave(), this);
    autosaveSettings->setModal(true);

    if(autosaveSettings->exec() == QDialog::Accepted)
    {
        mAutosave.setPetSaveInterval(autosaveSettings->getPetSaveInterval());
        mAutosave.setChangesTillAutosave(autosaveSettings->getChangesTillAutosave());
    }
}

void Petrack::setGitInformation(
    const std::string &gitCommitID,
    const std::string &gitCommitDate,
    const std::string &gitCommitBranch)
{
    mGitCommitID     = QString::fromStdString(gitCommitID);
    mGitCommitDate   = QString::fromStdString(gitCommitDate);
    mGitCommitBranch = QString::fromStdString(gitCommitBranch);
}

void Petrack::setCompileInformation(
    const std::string &compileOS,
    const std::string &compileTimeStamp,
    const std::string &compilerID,
    const std::string &compilerVersion)
{
    mCompileOS       = QString::fromStdString(compileOS);
    mCompileDate     = QString::fromStdString(compileTimeStamp);
    mCompilerID      = QString::fromStdString(compilerID);
    mCompilerVersion = QString::fromStdString(compilerVersion);
}

#include "moc_petrack.cpp"
