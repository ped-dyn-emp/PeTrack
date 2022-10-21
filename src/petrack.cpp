/*
 * PeTrack - Software for tracking pedestrians movement in videos
 * Copyright (C) 2010-2022 Forschungszentrum Jülich GmbH,
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

#include <QSignalMapper>
#include <QtOpenGL>
#include <QtWidgets>

// Added for Qt5 support
#include "aboutDialog.h"
#include "animation.h"
#include "autoCalib.h"
#include "backgroundItem.h"
#include "calibFilter.h"
#include "codeMarkerWidget.h"
#include "colorMarkerItem.h"
#include "colorMarkerWidget.h"
#include "colorRangeWidget.h"
#include "control.h"
#include "gridItem.h"
#include "helper.h"
#include "imageItem.h"
#include "logoItem.h"
#include "moCapItem.h"
#include "multiColorMarkerItem.h"
#include "multiColorMarkerWidget.h"
#include "openMoCapDialog.h"
#include "pMessageBox.h"
#include "petrack.h"
#include "player.h"
#include "roiItem.h"
#include "stereoItem.h"
#include "stereoWidget.h"
#include "tracker.h"
#include "trackerItem.h"
#include "trackerReal.h"
#include "view.h"

#include <QtPrintSupport/QPrintDialog>
#include <QtPrintSupport/QPrinter>

#ifdef AVI
#include "aviFile.h"
#else
#include "aviFileWriter.h"
#endif
#include "IO.h"
#include "person.h"

#include <cmath>
#include <ctime>
#include <iomanip>
#include <opencv2/opencv.hpp>

int Petrack::trcVersion = 0;

// Reihenfolge des anlegens der objekte ist sehr wichtig
Petrack::Petrack() :
    mExtrCalibration(mPersonStorage),
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
    mCalibFilter   = new CalibFilter; // schoener waere erst zu erzeugen, wenn video geladen wird, da sonst bei stereo
                                      // erst normealer und dann stereo objekt erzeugt wird
    mCalibFilter->disable(); // aber control widget greift schon bei erzeugung auf alle objekte zur einstellung zurueck

    mScene = new QGraphicsScene(this);

    mTrackingRoiItem = new RoiItem(this, Qt::blue);
    connect(mTrackingRoiItem, &RoiItem::changed, this, [=]() { this->setTrackChanged(true); });
    mTrackingRoiItem->setZValue(4); // groesser heisst weiter oben

    mRecognitionRoiItem = new RoiItem(this, Qt::green);
    connect(mRecognitionRoiItem, &RoiItem::changed, this, [=]() { this->setRecognitionChanged(true); });
    mRecognitionRoiItem->setZValue(5); // groesser heisst weiter oben

    mControlWidget = new Control(*this, *mScene, mReco, *mTrackingRoiItem, *mRecognitionRoiItem);

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

    mImageItem = new ImageItem(this); // durch uebergabe von scene wird indirekt ein scene->addItem() aufgerufen


    mAnimation = new Animation(this);

    mLogoItem = new LogoItem(this); // durch uebergabe von scene wird indirekt ein scene->addItem() aufgerufen
    mLogoItem->setZValue(6);        // groesser heisst weiter oben

    mExtrCalibration.setMainWindow(this);

    mGridItem = new GridItem(this);
    mGridItem->setZValue(2.5); // durch uebergabe von scene wird indirekt ein scene->addItem() aufgerufen

    mCoordItem = new CoordItem(this);
    mCoordItem->setZValue(3); // groesser heisst weiter oben
    mImageItem->setCoordItem(mCoordItem);

    mViewWidget = new ViewWidget(this);
    mView       = mViewWidget->view();
    mView->setScene(mScene);
    connect(mView, &GraphicsView::mouseDoubleClick, this, [this]() { this->openSequence(); });
    connect(mView, &GraphicsView::mouseShiftDoubleClick, this, &Petrack::addManualTrackPointOnlyVisible);
    connect(mView, &GraphicsView::mouseShiftControlDoubleClick, this, &Petrack::splitTrackPerson);
    connect(mView, &GraphicsView::mouseControlDoubleClick, this, &Petrack::addOrMoveManualTrackPoint);
    connect(mView, &GraphicsView::mouseRightDoubleClick, this, &Petrack::deleteTrackPoint);
    connect(mView, &GraphicsView::mouseMiddleDoubleClick, this, &Petrack::deleteTrackPointAll);
    connect(mView, &GraphicsView::mouseShiftWheel, this, &Petrack::skipToFrameWheel);
    connect(mView, &GraphicsView::mouseAltDoubleClick, this, &Petrack::skipToFrameFromTrajectory);
    connect(mView, &GraphicsView::mouseAltMoved, this, &Petrack::moveTrackPoint);
    connect(mView, &GraphicsView::mouseAltPressed, this, &Petrack::selectPersonForMoveTrackPoint);
    connect(mView, &GraphicsView::altReleased, this, &Petrack::releaseTrackPoint);
    connect(mView, &GraphicsView::mouseAltReleased, this, &Petrack::releaseTrackPoint);
    connect(mView, &GraphicsView::mouseCtrlWheel, this, &Petrack::scrollShowOnly);

    mPlayerWidget = new Player(mAnimation, this);

    QVBoxLayout *vLayout = new QVBoxLayout;
    vLayout->setSpacing(space);
    vLayout->addWidget(mViewWidget);
    vLayout->addWidget(mPlayerWidget);

    //---------------------------

    mTracker     = new Tracker(this, mPersonStorage);
    mTrackerReal = new TrackerReal(this, mPersonStorage);
    mTrackerItem = new TrackerItem(this, mPersonStorage);
    mTrackerItem->setZValue(5); // groesser heisst weiter oben

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

    mBackgroundItem = new BackgroundItem(this);
    mBackgroundItem->setZValue(2.2); // um so groesser um so hoeher um so eher zu sehen
    mBackgroundItem->setVisible(false);

    //---------------------------

    mMoCapItem = new MoCapItem(*this, *mAnimation, mMoCapController);
    mMoCapItem->setZValue(3); // um so groesser um so hoeher um so eher zu sehen

    /// Add Items
    mScene->addItem(mImageItem);
    mScene->addItem(mLogoItem);
    mScene->addItem(mGridItem);
    mScene->addItem(mCoordItem);
    mScene->addItem(mTrackingRoiItem);
    mScene->addItem(mRecognitionRoiItem);
    mScene->addItem(mTrackerItem);
    mScene->addItem(mStereoItem);
    mScene->addItem(mColorMarkerItem);
    mScene->addItem(mCodeMarkerItem);
    mScene->addItem(mMultiColorMarkerItem);
    mScene->addItem(mBackgroundItem);
    mScene->addItem(mMoCapItem);

    //---------------------------

    mCentralLayout = new QHBoxLayout;
    mCentralLayout->setMargin(space);
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

    auto *exportShortCut = new QShortcut{QKeySequence("Ctrl+e"), this};
    connect(exportShortCut, &QShortcut::activated, this, [=]() { exportTracker(); });

    auto *toggleOnlineTracking = new QShortcut{QKeySequence("Shift+t"), this};
    connect(toggleOnlineTracking, &QShortcut::activated, this, [=]() { mControlWidget->toggleOnlineTracking(); });

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

    mAutoBackTrack          = true;  // ist der default, dann wenn in XML-Datei nicht drin steht
    mAutoTrackOptimizeColor = false; // ist der default, dann wenn in XML-Datei nicht drin steht

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
        if(event->mimeData()->urls().first().toLocalFile().right(4) == ".pet")
        {
            openProject(event->mimeData()->urls().first().toLocalFile());
        }
        else if(event->mimeData()->urls().first().toLocalFile().right(4) == ".trc")
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

    if(mImage)
    {
        iW = mImage->width();
        iH = mImage->height();
        bS = getImageBorderSize();
    }

    if(mControlWidget->getCalibCoordShow())
    {
        double scale = mControlWidget->getCalibCoordScale() / 10.;
        double tX    = mControlWidget->getCalibCoordTransX() / 10.;
        double tY    = mControlWidget->getCalibCoordTransY() / 10.;

        // setzen der bounding box der scene
        // Faktor 1.1 dient dazu, dass auch Zahl "1" bei coord gut in sichtbaren Bereich passt
        double xMin = (tX - 1.1 * scale < -bS) ? tX - 1.1 * scale : -bS;
        double yMin = (tY - 1.1 * scale < -bS) ? tY - 1.1 * scale : -bS;
        double xMax = (tX + 1.1 * scale > iW - bS) ? tX + 1.1 * scale : iW - bS;
        double yMax = (tY + 1.1 * scale > iH - bS) ? tY + 1.1 * scale : iH - bS;
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
    QDomElement root = doc.firstChildElement("PETRACK");
    QString     seq;
    int         frame = -1, sourceFrameIn = -1, sourceFrameOut = -1;
    double      fps              = DEFAULT_FPS;
    int         onlyPeopleNr     = 1;
    QString     onlyPeopleNrList = "1";
    int         zoom = 250, rotate = 0, hScroll = 0, vScroll = 0;
    enum Camera cam = cameraUnset;
    setLoading(true);
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
            }
            if(elem.hasAttribute("STATUS_HEIGHT"))
            {
                if(mStatusPosRealHeight) // null kann eigentlich nicht vorkommen, da im constructor von petrack erzeugt
                                         // wird
                {
                    mStatusPosRealHeight->setValue(elem.attribute("STATUS_HEIGHT").toDouble());
                }
            }
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
        else if(elem.tagName() == "MOCAP")
        {
            mMoCapController.getXml(elem);
        }
        else if(elem.tagName() == "CONTROL")
        {
            mControlWidget->getXml(elem);
            QDomElement tmpElem = (elem.firstChildElement("TRACKING")).firstChildElement("PATH");
            if(tmpElem.hasAttribute("ONLY_PEOPLE_NR"))
            {
                onlyPeopleNr = tmpElem.attribute("ONLY_PEOPLE_NR").toInt();
            }
            if(tmpElem.hasAttribute("ONLY_PEOPLE_NR_LIST"))
            {
                onlyPeopleNrList = tmpElem.attribute("ONLY_PEOPLE_NR_LIST");
            }
        }
        else if(elem.tagName() == "EXTR_CALIBRATION")
        {
            mExtrCalibration.getXml(elem);
        }
        else if(elem.tagName() == "PLAYER")
        {
            if(elem.hasAttribute("FRAME"))
            {
                frame = elem.attribute("FRAME").toInt();
            }
            if(elem.hasAttribute("FPS"))
            {
                fps = elem.attribute("FPS").toDouble();
            }
            if(elem.hasAttribute("SOURCE_FRAME_IN"))
            {
                sourceFrameIn = elem.attribute("SOURCE_FRAME_IN").toInt();
            }
            if(elem.hasAttribute("SOURCE_FRAME_OUT"))
            {
                sourceFrameOut = elem.attribute("SOURCE_FRAME_OUT").toInt();
            }
            if(elem.hasAttribute("PLAYER_SPEED_FIXED"))
            {
                mPlayerWidget->setPlayerSpeedLimited(elem.attribute("PLAYER_SPEED_FIXED").toInt());
            }
        }
        else if(elem.tagName() == "VIEW")
        {
            if(elem.hasAttribute("ANTIALIAS"))
            {
                mAntialiasAct->setChecked(elem.attribute("ANTIALIAS").toInt() == Qt::Checked);
            }
            if(elem.hasAttribute("OPENGL"))
            {
                mOpenGLAct->setChecked(elem.attribute("OPENGL").toInt() == Qt::Checked);
            }
            if(elem.hasAttribute("SAVE_TRANSFORMED"))
            {
                mCropZoomViewAct->setChecked(elem.attribute("SAVE_TRANSFORMED") == Qt::Checked);
            }
            if(elem.hasAttribute("TRANSFORMATION"))
            {
                QString     matStr = elem.attribute("TRANSFORMATION");
                QTextStream in(&matStr);
                in >> zoom >> rotate >> hScroll >> vScroll;
            }
            if(elem.hasAttribute("CAMERA"))
            {
                cam = (enum Camera) elem.attribute("CAMERA").toInt();
            }
            if(elem.hasAttribute("HIDE_CONTROLS"))
            {
                mHideControlsAct->setChecked(elem.attribute("HIDE_CONTROLS").toInt() == Qt::Checked);
            }
        }
        else if(elem.tagName() == "AUTO_TRACK")
        {
            if(elem.hasAttribute("BACK_TRACK"))
            {
                mAutoBackTrack = elem.attribute("BACK_TRACK").toInt();
            }
            if(elem.hasAttribute("OPTIMZE_COLOR"))
            {
                mAutoTrackOptimizeColor = elem.attribute("OPTIMZE_COLOR").toInt();
            }
        }
        else
        {
            debout << "Unknown PETRACK tag " << elem.tagName() << std::endl;
        }
    }
    // open koennte am schluss passieren, dann wuerde nicht erst unveraendertes bild angezeigt,
    // dafuer koennte es aber sein, dass werte zb bei fx nicht einstellbar sind!
    mSeqFileName = seq;
    if(openSeq && (seq != ""))
    {
        openSequence(seq); // wenn leer, dann kommt abfrage hoch, welche datei; abbrechen, wenn aktuelle gewuenscht
    }

    mViewWidget->setZoomLevel(zoom);
    mViewWidget->setRotateLevel(rotate);
    mView->horizontalScrollBar()->setValue(hScroll);
    mView->verticalScrollBar()->setValue(vScroll);

    bool loaded = false;
    if(!mBackgroundFilter.getFilename().isEmpty())
    {
        if(!(loaded = mBackgroundFilter.load(mBackgroundFilter.getFilename())))
        {
            debout << "Error: loading background file " << mBackgroundFilter.getFilename() << "!" << std::endl;
        }
    }

    mPlayerWidget->setFrameInNum(sourceFrameIn == -1 ? mAnimation->getSourceInFrameNum() : sourceFrameIn);
    mPlayerWidget->setFrameOutNum(sourceFrameOut == -1 ? mAnimation->getSourceOutFrameNum() : sourceFrameOut);
    mPlayerWidget->update();

    if(frame != -1)
    {
        if(mControlWidget->isFilterBgChecked() &&
           !loaded) // mit dem anfangs geladenen bild wurde bereits faelschlicherweise bg bestimmt
        {
            mBackgroundFilter.reset(); // erst nach dem springen zu einem frame background bestimmen
        }
        mPlayerWidget->skipToFrame(frame); // hier wird updateImage ausgefuehrt
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
    mPlayerWidget->setFPS(fps); // erst spaet setzen, damit Wert den des geladenen Videos ueberschreiben kann
    setLoading(false);
}

void Petrack::openProject(QString fileName, bool openSeq) // default fileName="", openSequence = true
{
    if(!QFileInfo(mProFileName).isDir()) // a project is already loaded
    {
        if(!maybeSave())
        {
            return;
        }
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
        resetSettings();
        QDomDocument doc("PETRACK"); // eigentlich Pfad zu Beschreibungsdatei fuer Dateiaufbau
        if(!doc.setContent(&file))
        {
            PCritical(this, tr("PeTrack"), tr("Cannot read content from %1.").arg(fileName));
            file.close();
            return;
        }

        debout << "open " << fileName << std::endl;
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
        openXml(doc, openSeq);
        mLastTrackerExport = mTrcFileName;

        if(newerThanVersion(QString("0.9.0"), root.attribute("VERSION")))
        {
            PWarning(
                this,
                tr("PeTrack"),
                tr("You are using a project version lower than 0.9: Therefore, the extended intrinsic calibration "
                   "model is disabled."));
            mControlWidget->setExtModelChecked(false);
        }
        else if(newerThanVersion(QString("0.9.2"), root.attribute("VERSION")))
        {
            // only checking one parameter because if the ext. model is used all parameters are not equal to zero
            if(mControlWidget->getCalibS1() == 0.)
            {
                mControlWidget->setExtModelChecked(false);
            }
            else
            {
                mControlWidget->setExtModelChecked(true);
            }
        }

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

    // control settings (right control widget)
    elem = doc.createElement("CONTROL");
    mControlWidget->setXml(elem);
    root.appendChild(elem);

    // Reprojection error extrinsic calib
    elem = doc.createElement("EXTR_CALIBRATION");
    mExtrCalibration.setXml(elem);
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

    // settings for multicolor marker
    elem = doc.createElement("MULTI_COLOR_MARKER");
    mMultiColorMarkerWidget->setXml(elem);
    root.appendChild(elem);

    // settings for MoCap-Visualization
    elem = doc.createElement("MOCAP");
    mMoCapController.setXml(elem);
    root.appendChild(elem);

    // player settings (which frame, frame range)
    elem = doc.createElement("PLAYER");
    elem.setAttribute("FRAME", mPlayerWidget->getPos()); // == mAnimation->getCurrentFrameNum()
    elem.setAttribute("FPS", mAnimation->getFPS());
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
#ifndef STEREO_DISABLED
    elem.setAttribute("CAMERA", mAnimation->getCamera());
#else
    elem.setAttribute("CAMERA", cameraUnset);
#endif
    elem.setAttribute("HIDE_CONTROLS", mHideControlsAct->isChecked());
    root.appendChild(elem);

    // auto track settings
    elem = doc.createElement("AUTO_TRACK");
    elem.setAttribute("BACK_TRACK", mAutoBackTrack);
    elem.setAttribute("OPTIMZE_COLOR", mAutoTrackOptimizeColor);
    root.appendChild(elem);
}

/// rueckgabewert zeigt an, ob gesichert werden konnte
bool Petrack::saveSameProject()
{
    return saveProject(mProFileName);
}

bool Petrack::saveProjectAs()
{
    auto fileName = QFileDialog::getSaveFileName(
        this, tr("Select project file"), mProFileName, tr("PeTrack project file (*.pet);;All files (*.*)"));
    return saveProject(fileName);
}

/// rueckgabewert zeigt an, ob gesichert werden konnte
bool Petrack::saveProject(QString fileName) // default fileName=""
{
    // if no destination file or folder is given
    if(fileName.isEmpty() && QFileInfo(mProFileName).isDir())
    {
        fileName = QFileDialog::getSaveFileName(
            this, tr("Select project file"), mProFileName, tr("PeTrack project file (*.pet);;All files (*.*)"));
    }

    if(!fileName.isEmpty())
    {
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
        debout << "save project to " << fileName << std::endl;

        updateWindowTitle();
        return true;
    }
    else
    {
        return false;
    }
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
        debout << "No camera ID delivered: Set CameraID to 0 (default Camera)" << std::endl;
        camID = 0; // default
    }
    if(!mAnimation->openCameraStream(camID))
    {
        PCritical(this, tr("PeTrack"), tr("Cannot start Camera Livestream."));
        return;
    }
    mSeqFileName = "camera live stream";
    debout << "open " << mSeqFileName << " (" << mAnimation->getNumFrames() << " frames; " << mAnimation->getFPS()
           << " fps; " << mAnimation->getSize().width() << "x" << mAnimation->getSize().height() << " pixel)"
           << std::endl; // size
    updateSequence();
    updateWindowTitle();
    mPlayerWidget->setFPS(mAnimation->getFPS());
    if(mOpenGLAct->isChecked())
    {
        mLogoItem->fadeOut(1);
    }
    else
    {
        mLogoItem->fadeOut(50);
    }
    updateCoord();

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
        if(!mAnimation->openAnimation(fileName))
        {
            PCritical(this, tr("PeTrack"), tr("Cannot load %1.").arg(fileName));
            return;
        }

        mCameraGroupView->setEnabled(mAnimation->isStereoVideo());
        mCameraMenu->setEnabled(mAnimation->isStereoVideo());

#ifdef STEREO
        if(mAnimation->isStereoVideo())
        {
            if(mStereoContext)
                delete mStereoContext;
            mStereoContext = new pet::StereoContext(this);
        }

        bool lastIsStereoVideo = mAnimation->isStereoVideo();
        if(mCalibFilter == NULL || (mAnimation->isStereoVideo() != lastIsStereoVideo))
        {
            bool lastCalibFilterEnabled = false;
            if(mCalibFilter != NULL)
            {
                lastCalibFilterEnabled = mCalibFilter->getEnabled();
                delete mCalibFilter;
            }
            if(mAnimation->isStereoVideo())
            {
                mCalibFilter = new CalibStereoFilter;
                ((CalibStereoFilter *) mCalibFilter)->setStereoContext(mStereoContext);
            }
            else
                mCalibFilter = new CalibFilter;
            mCalibFilter->setEnabled(lastCalibFilterEnabled);
        }
#endif
        mSeqFileName = fileName;
        debout << "open " << mSeqFileName << " (" << mAnimation->getNumFrames() << " frames; " << mAnimation->getFPS()
               << " fps; " << mAnimation->getSize().width() << "x" << mAnimation->getSize().height() << " pixel)"
               << std::endl; // size
        updateSequence();
        updateWindowTitle();
        mPlayerWidget->setFPS(mAnimation->getFPS());
        if(mOpenGLAct->isChecked())
        {
            mLogoItem->fadeOut(1);
        }
        else
        {
            mLogoItem->fadeOut(50);
        }
        updateCoord();
    }
}

void Petrack::openMoCapFile()
{
    OpenMoCapDialog dialog(this, mMoCapController);
    dialog.exec();
}

void Petrack::updateWindowTitle()
{
    QString title;
    QSize   size = mAnimation->getSize();

    if(QFileInfo(mProFileName).isDir())
    {
        title = tr("PeTrack (v") + mPetrackVersion + tr("): ");
    }
    else
    {
        title = tr("PeTrack (v") + mPetrackVersion + tr("): ") + QFileInfo(mProFileName).fileName();
        if(mAnimation->isVideo() || mAnimation->isImageSequence())
        {
            title += "; ";
        }
    }
    if(mAnimation->isVideo())
    {
        title += "sequence: " + mAnimation->getCurrentFileName() + tr(" (%1").arg(mAnimation->getNumFrames()) +
                 tr(" frames; %1x%2").arg(size.width()).arg(size.height()) + " pixel)";
    }
    else if(mAnimation->isImageSequence())
    {
        title += "sequence: " + mAnimation->getCurrentFileName() + tr(" ... (%1").arg(mAnimation->getNumFrames()) +
                 tr(" frames; %1x%2").arg(size.width()).arg(size.height()) + " pixel)";
    }
    setWindowTitle(title);
}

void Petrack::saveVideo()
{
    saveSequence(true, false);
}
void Petrack::saveVideoView()
{
    saveSequence(true, true);
}
void Petrack::saveImageSequence()
{
    saveSequence(false, false);
}
void Petrack::saveViewSequence()
{
    saveSequence(false, true);
}


/**
 * @brief Saves current sequence as avi-file or image sequence
 *
 * Saves the loaded image sequence or video from current frame on till the end.
 * One can save the sequence as is or one can save the view shown in PeTrack.
 *
 * @param saveVideo true, if wanting to save a video. Ignored when dest isn't empty
 * @param saveView true, if sequence should be saved as shown in PeTrack (with trajectories etc.)
 * @param dest destination file; if empty, the user chooses via a dialog
 */
void Petrack::saveSequence(bool saveVideo, bool saveView, QString dest) // default saveView= false, dest=""
{
    static QString lastDir;
    //    bool autoSave = false;

    // if no destination file or folder is given
    if(dest.isEmpty())
    {
        if(lastDir.isEmpty() && !mSeqFileName.isEmpty())
        {
            lastDir = QFileInfo(mSeqFileName).path();
        }

        if(saveVideo)
        {
            dest = QFileDialog::getSaveFileName(
                this,
                tr("Select video file"),
                lastDir,
                tr("Video (*.avi);;All files (*.*)")); //? *.mpg *.mpeg
        }
        else
        {
            if(saveView)
            {
                dest = QFileDialog::getExistingDirectory(
                    this,
                    tr("Select directory to save view sequence"),
                    lastDir,
                    QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
            }
            else
            {
                dest = QFileDialog::getExistingDirectory(
                    this,
                    tr("Select directory to save image sequence"),
                    lastDir,
                    QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
            }
        }
    }
    else // uebergabe von saveVideo spielt keine roll mehr, sondern wird hier analysiert anhand von Dateiendung
    {
        //        autoSave = true; // command line option
        if(dest.right(4) == ".avi")
        {
            saveVideo = true;
        }
        else
        {
            saveVideo = false;
        }
    }

    if(!dest.isEmpty() && mImage)
    {
        int     rest      = mAnimation->getNumFrames() - 1;
        int     numLength = 1;
        int     memPos    = mPlayerWidget->getPos();
        QString fileName  = "";
#ifdef AVI
        AviFile aviFile;
#else
        AviFileWriter aviFile;
#endif
        bool      formatIsSaveAble = false;
        bool      saveRet;
        QImage   *viewImage = nullptr;
        QPainter *painter   = nullptr;
        int       progEnd   = mAnimation->getSourceOutFrameNum() -
                      mPlayerWidget->getPos(); // nur wenn nicht an anfang gesprungen wird:-mPlayerWidget->getPos()
        cv::Mat iplImgFilteredBGR;
        bool    writeFrameRet   = false;
        bool    convert8To24bit = false;
        int     mult;


        if(saveVideo)
        {
            if(saveView)
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
            if(convert8To24bit)
            {
                mult = 3;
            }
            else
            {
                mult = 1;
            }

            bool ok = false;

            if(saveView)
            {
                ok = aviFile.open(
                    dest.toStdString().c_str(),
                    viewImage->width(),
                    viewImage->height(),
                    viewImage->depth(),
                    mAnimation->getFPS());
            }
            else
            {
                ok = aviFile.open(
                    dest.toStdString().c_str(), mImg.cols, mImg.rows, mult * 8 * mImg.channels(), mAnimation->getFPS());
            }

            if(!ok)
            {
                debout << "Error: opening AVI file: " << dest.toStdString().c_str() << std::endl;
                return;
            }
        }

        if(!saveVideo)
        {
            if(saveView)
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
            if(mAnimation->isVideo())
            {
                // calculate string length of sequence number
                while((rest /= 10) > 0)
                {
                    numLength++;
                }
                fileName = (dest + "/" + mAnimation->getFileBase() + "%1.png")
                               .arg(mPlayerWidget->getPos(), numLength, 10, QChar('0'));
            }
            else
            {
                fileName = dest + "/" + mAnimation->getCurrentFileName();
            }

            if(saveView)
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

        QProgressDialog progress("", "Abort save", 0, progEnd, this);
        progress.setWindowModality(Qt::WindowModal); // blocks main window

        if(saveVideo)
        {
            if(saveView)
            {
                progress.setLabelText("Save video view...");
            }
            else
            {
                progress.setLabelText("Save video...");
            }
        }
        else
        {
            if(saveView)
            {
                progress.setLabelText("Save view sequence...");
            }
            else
            {
                progress.setLabelText("Save image sequence...");
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

            if(saveVideo)
            {
                // video sequence
                if(saveView)
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
                if((mImgFiltered.channels() == 1) /* && convert8To24bit*/)
                {
                    cv::cvtColor(mImg, iplImgFilteredBGR, cv::COLOR_GRAY2BGR);
                    if(saveView)
                    {
                        writeFrameRet = aviFile.appendFrame(
                            (const unsigned char *) viewImage->bits(),
                            true); // 2. param besagt, ob vertikal gespiegel werden soll
                    }
                    else
                    {
                        writeFrameRet = aviFile.appendFrame(
                            (const unsigned char *) iplImgFilteredBGR.data,
                            true); // 2. param besagt, ob vertikal gespiegel werden soll
                    }
                }
                else
                {
                    if(saveView)
                    {
                        writeFrameRet = aviFile.appendFrame(
                            (const unsigned char *) viewImage->bits(),
                            true); // 2. param besagt, ob vertikal gespiegel werden soll
                    }
                    else
                    {
                        writeFrameRet = aviFile.appendFrame(
                            (const unsigned char *) mImg.data,
                            true); // 2. param besagt, ob vertikal gespiegel werden soll
                    }
                }

                if(!writeFrameRet)
                {
                    progress.setValue(progEnd);
                    PCritical(
                        this,
                        tr("PeTrack"),
                        tr("Cannot save %1 maybe because of wrong file extension or unsupported codec.").arg(dest));
                    break;
                }
            }
            else
            {
                // single frame sequence
                if(saveView)
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
                if(mAnimation->isVideo())
                {
                    fileName = (dest + "/" + mAnimation->getFileBase() + "%1.png")
                                   .arg(mPlayerWidget->getPos(), numLength, 10, QChar('0'));
                    if(saveView)
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
                    fileName = dest + "/" + mAnimation->getCurrentFileName();
                    if(saveView)
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
                    fileName = dest + "/" + QFileInfo(mAnimation->getCurrentFileName()).completeBaseName() + ".png";
                    if(saveView)
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
                    PCritical(this, tr("PeTrack"), tr("Cannot save %1.").arg(fileName));
                    break;
                }
            }
        } while(mPlayerWidget->frameForward());

        if(!saveVideo && saveView)
        {
            delete viewImage;
            delete painter;
        }

        // bei abbruch koennen es auch mPlayerWidget->getPos() frames sein, die bisher geschrieben wurden
        //-memPos nur, wenn nicht an den anfang gesprungen wird
        debout << "wrote " << mPlayerWidget->getPos() + 1 - memPos << " of " << mAnimation->getNumFrames() << " frames."
               << std::endl;
        progress.setValue(progEnd);

        if(saveVideo)
        {
            aviFile.close();
        }

        mPlayerWidget->skipToFrame(memPos);
        lastDir = dest;
    }
}

/**
 * @brief Saves the current View, including visualizations, in a file (.g. pdf)
 *
 * @param dest name of the saved file; if empty, a dialogue for the user opens
 */
void Petrack::saveView(QString dest) // default = ""
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
            if(dest.right(4) == ".pdf" || dest.right(3) == ".ps" || dest.right(4) == ".eps")
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

void Petrack::saveImage(QString dest) // default = ""
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
            if(dest.right(4) == ".pdf" || dest.right(3) == ".ps" || dest.right(4) == ".eps")
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
    mAnimation->reset();
    openXml(mDefaultSettings, false);
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
    PMessageBox *mb = new PMessageBox{this, tr("Command line options"), commandLineOptionsString, QIcon()};
    mb->setModal(false);
    mb->setAttribute(Qt::WA_DeleteOnClose);
    mb->show();
}

void Petrack::keyBindings()
{
    const QString out = tr(
        "<p>Beside the space bar all bindings only affect inside the image.</p>"
        "<dl><dt><kbd>Space bar</kbd></dt><dd>toggles between pause and last play direction</dd>"
        "<dt><kbd>Mouse scroll wheel</kbd></dt><dd>zooms in and out to or from the pixel of the image at the position "
        "of the mouse pointer</dd>"
        "<dt><kbd>Shift + mouse scroll wheel</kbd></dt><dd>plays forwards or backwards frame by frame</dd>"
        "<dt><kbd>Holding left mouse button</kbd></dt><dd>moves image</dd>"
        "<dt><kbd>Arrows up/down</kbd></dt><dd>zoom in/out</dd>"
        "<dt><kbd>Arrows left/right</kbd></dt><dd>frame back/forward</dd>"
        "<dt><kbd>Double-click left mouse button</kbd></dt><dd>opens video or image sequence</dd>"
        "<dt><kbd>Ctrl + double-click left mouse button</kbd></dt><dd>inserts new or moves near trackpoint</dd>"
        "<dt><kbd>Ctrl + Shift + double-click left mouse button</kbd></dt><dd>splits near trackpoint before actual "
        "frame</dd>"
        "<dt><kbd>Ctrl + double-click right mouse button</kbd></dt><dd>deletes a trajectory of a near trackpoint</dd>"
        "<dt><kbd>Shift + double-click right mouse button</kbd></dt><dd>deletes the past part of a trajectory of a "
        "near trackpoint</dd>"
        "<dt><kbd>Alt + double-click right mouse button</kbd></dt><dd>deletes the future part of a trajectory of a "
        "near trackpoint</dd>"
        "<dt><kbd>Ctrl + double-click middle mouse button</kbd></dt><dd>deletes all trajectories</dd>"
        "<dt><kbd>Shift + double-click middle mouse button</kbd></dt><dd>deletes the past part of all trajectories</dd>"
        "<dt><kbd>Alt + double-click middle mouse button</kbd></dt><dd>deletes the future part of all trajectories</dd>"
        "<dt><kbd>Shift + t</kbd></dt><dd>toggles tracking online calculation</dd>"
        "<dt><kbd>Shift + double-click left mouse button</kbd></dt><dd>inserts new or moves near trackpoint and "
        "enables showing only the modified trajectory</dd>"
        "<dt><kbd>Alt + double-click left mouse button</kbd></dt><dd>jumps to frame of trackpoint under cursor</dd>"
        "<dt><kbd>Alt + holding left mouse button</kbd></dt><dd>moves trackpoint under cursor</dd>"
        "<dt><kbd>Ctrl + E</kbd></dt><dd>export trajectories</dd>"
        "<dt><kbd>Ctrl + mouse scroll wheel</kbd></dt><dd>change the displayed person (if show only people "
        "enabled)</dd></dl>"
        "<p>Further key bindings you will find next to the entries of the menus.</p>");

    PMessageBox *mb = new PMessageBox(this, tr("Key Bindings"), out, QIcon());
    mb->setAttribute(Qt::WA_DeleteOnClose);
    mb->setModal(false);

    mb->show();
}

void Petrack::onlineHelp()
{
    static QUrl url("https://jugit.fz-juelich.de/ped-dyn-emp/petrack/-/wikis/home");
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
    mView->setViewport(mOpenGLAct->isChecked() ? new QGLWidget(QGLFormat(QGL::SampleBuffers)) : new QWidget);

    // alten freigeben wegen new????
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

void Petrack::setCamera()
{
#ifndef STEREO_DISABLED
    if(mAnimation)
    {
        if(mCameraLeftViewAct->isChecked())
        {
            if((mAnimation->getCamera()) != cameraLeft)
                mAnimation->setCamera(cameraLeft); // war: hier wird direkt bei Umstellung neu gelesen
            else
                return;
        }
        else if(mCameraRightViewAct->isChecked())
        {
            if((mAnimation->getCamera()) != cameraRight)
                mAnimation->setCamera(cameraRight); // war: hier wird direkt bei Umstellung neu gelesen
            else
                return;
        }
        else // kann eigentlich nicht vorkommen
        {
            mAnimation->setCamera(cameraUnset);
            return;
        }
        updateImage(mAnimation->getFrameAtIndex(
            mAnimation->getCurrentFrameNum())); // wird nur aufgerufen, wenn left / right sich geaendert hat
        // mPlayerWidget->updateImage();
        // mPlayerWidget->skipToFrame(mPlayerWidget->getPos()); // machtpasue!!
        // updateImage(true); // nur dies aufrufen, wenn nicht links rechts gleichzeitig gehalten wird
    }

#endif
}

/**
 * @brief Helper function to create Actions for the menu bar
 * @see Petrack::createMenus()
 */
void Petrack::createActions()
{
    mOpenSeqAct = new QAction(tr("&Open Sequence"), this);
    mOpenSeqAct->setShortcut(tr("Ctrl+Shift+O"));
    connect(mOpenSeqAct, SIGNAL(triggered()), this, SLOT(openSequence()));

    mOpenCameraAct = new QAction(tr("Open Camera Stream"), this);
    // mOpenCameraAct->setShortcut(tr("Ctrl+C")); // because of some reason it is sometimes fired with
    // Ctrl+LeftMouseButton ==> so disabled (it's also not really needed)
    connect(mOpenCameraAct, SIGNAL(triggered()), this, SLOT(openCameraLiveStream()));

    mOpenMoCapAct = new QAction(tr("Manage MoCap Files"), this);
    connect(mOpenMoCapAct, &QAction::triggered, this, &Petrack::openMoCapFile);

    mSaveSeqVidAct = new QAction(tr("Save Video"), this);
    mSaveSeqVidAct->setEnabled(false);
    connect(mSaveSeqVidAct, SIGNAL(triggered()), this, SLOT(saveVideo()));

    mSaveSeqVidViewAct = new QAction(tr("Save Video View"), this);
    mSaveSeqVidViewAct->setEnabled(false);
    connect(mSaveSeqVidViewAct, SIGNAL(triggered()), this, SLOT(saveVideoView()));

    mSaveSeqImgAct = new QAction(tr("Save Image S&equence"), this);
    mSaveSeqImgAct->setShortcut(tr("Ctrl+F"));
    mSaveSeqImgAct->setEnabled(false);
    connect(mSaveSeqImgAct, SIGNAL(triggered()), this, SLOT(saveImageSequence()));

    mSaveSeqViewAct = new QAction(tr("Save View S&equence"), this);
    //    mSaveSeqViewAct->setShortcut(tr("Ctrl+F"));
    mSaveSeqViewAct->setEnabled(false);
    connect(mSaveSeqViewAct, SIGNAL(triggered()), this, SLOT(saveViewSequence()));

    mOpenPrAct = new QAction(tr("&Open Project"), this);
    mOpenPrAct->setShortcut(tr("Ctrl+O"));
    connect(mOpenPrAct, SIGNAL(triggered()), this, SLOT(openProject()));

    mSavePrAct = new QAction(tr("&Save Project As"), this);
    mSavePrAct->setShortcut(tr("Ctrl+Shift+S"));
    connect(mSavePrAct, &QAction::triggered, this, &Petrack::saveProjectAs);

    mSaveAct = new QAction(tr("&Save Project"), this);
    mSaveAct->setShortcut(tr("Ctrl+S"));
    connect(mSaveAct, SIGNAL(triggered()), this, SLOT(saveSameProject()));

    mSaveImageAct = new QAction(tr("&Save Image"), this);
    mSaveImageAct->setShortcut(tr("Ctrl+I"));
    mSaveImageAct->setEnabled(false);
    connect(mSaveImageAct, SIGNAL(triggered()), this, SLOT(saveImage()));

    mSaveViewAct = new QAction(tr("&Save View"), this);
    mSaveViewAct->setShortcut(tr("Ctrl+V"));
    mSaveViewAct->setEnabled(false);
    connect(mSaveViewAct, SIGNAL(triggered()), this, SLOT(saveView()));

    mPrintAct = new QAction(tr("&Print"), this);
    mPrintAct->setShortcut(tr("Ctrl+P"));
    mPrintAct->setEnabled(false);
    connect(mPrintAct, SIGNAL(triggered()), this, SLOT(print()));

    mResetSettingsAct = new QAction(tr("&Reset Settings"), this);
    //    mResetSettingsAct->setShortcut(tr("Ctrl+R"));
    mResetSettingsAct->setEnabled(false); // da es noch nicht fehlerfrei funktioniert
    connect(mResetSettingsAct, SIGNAL(triggered()), this, SLOT(resetSettings()));

    mExitAct = new QAction(tr("E&xit"), this);
    mExitAct->setShortcut(tr("Ctrl+Q"));
    connect(mExitAct, SIGNAL(triggered()), this, SLOT(close()));

    mAntialiasAct = new QAction(tr("&Antialias"), this);
    mAntialiasAct->setShortcut(tr("Ctrl+A"));
    mAntialiasAct->setCheckable(true);
    connect(mAntialiasAct, SIGNAL(triggered()), this, SLOT(antialias()));

    mFontAct = new QAction(tr("&Font"), this);
    connect(mFontAct, SIGNAL(triggered()), this, SLOT(setGlobalFont()));

    mHideControlsAct = new QAction(tr("&Hide controls"), this);
    mHideControlsAct->setShortcut(tr("Ctrl+H"));
    mHideControlsAct->setCheckable(true);
    connect(mHideControlsAct, SIGNAL(triggered()), this, SLOT(showHideControlWidget()));
    connect(mHideControlsAct, SIGNAL(changed()), this, SLOT(showHideControlWidget()));

    mCropZoomViewAct = new QAction(tr("&Transform while saving"), this); // Crop and zoom while saving
    mCropZoomViewAct->setCheckable(true);

    mOpenGLAct = new QAction(tr("Open&GL"), this);
    mOpenGLAct->setShortcut(tr("Ctrl+G"));
    mOpenGLAct->setCheckable(true);
    connect(mOpenGLAct, SIGNAL(triggered()), this, SLOT(opengl()));

    mResetAct = new QAction(tr("&Reset"), this);
    mResetAct->setShortcut(tr("Ctrl+R"));
    connect(mResetAct, SIGNAL(triggered()), this, SLOT(reset()));

    mFitViewAct =
        new QAction(tr("Fit in window"), this); // Resize to window; fit in view; show all; in fenster einpassen
    mFitViewAct->setShortcut(tr("Ctrl+0"));
    connect(mFitViewAct, SIGNAL(triggered()), this, SLOT(fitInView()));

    mFitROIAct = new QAction(tr("Fit in region of interest"), this); // Resize ROI to window; fit in view;
    mFitROIAct->setShortcut(tr("Ctrl+1"));
    connect(mFitROIAct, SIGNAL(triggered()), this, SLOT(fitInROI()));

    mCameraGroupView = new QActionGroup(this);
    // mCameraGroupView->addAction(mCameraLeftViewAct);
    // mCameraGroupView->addAction(mCameraRightViewAct);
    mCameraLeftViewAct = new QAction(tr("&Left"), mCameraGroupView);
    mCameraLeftViewAct->setShortcut(tr("Ctrl++Shift+L"));
    mCameraLeftViewAct->setCheckable(true);
    connect(mCameraLeftViewAct, SIGNAL(triggered()), this, SLOT(setCamera()));
    mCameraRightViewAct = new QAction(tr("&Right"), mCameraGroupView);
    mCameraRightViewAct->setShortcut(tr("Ctrl++Shift+R"));
    mCameraRightViewAct->setCheckable(true);
    connect(mCameraRightViewAct, SIGNAL(triggered()), this, SLOT(setCamera()));
    mCameraRightViewAct->setChecked(true); // right wird als default genommen, da reference image in triclops auch right
                                           // ist // erste trj wurden mit left gerechnet

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


    mDelPastAct = new QAction(tr("&Past part of all trj."), this);
    connect(
        mDelPastAct,
        &QAction::triggered,
        this,
        [this]() { this->deleteTrackPointAll(PersonStorage::Direction::Previous); });

    mDelFutureAct = new QAction(tr("&Future part of all trj."), this);
    connect(
        mDelFutureAct,
        &QAction::triggered,
        this,
        [this]() { this->deleteTrackPointAll(PersonStorage::Direction::Following); });

    mDelAllRoiAct = new QAction(tr("&Trj. moving through ROI"), this);
    connect(mDelAllRoiAct, &QAction::triggered, this, &Petrack::deleteTrackPointROI);
    mDelPartRoiAct = new QAction(tr("Part of Trj. inside &ROI"), this);
    connect(mDelPartRoiAct, &QAction::triggered, this, &Petrack::deleteTrackPointInsideROI);

    // -------------------------------------------------------------------------------------------------------

    mCommandAct = new QAction(tr("&Command line options"), this);
    connect(mCommandAct, SIGNAL(triggered()), this, SLOT(commandLineOptions()));

    mKeyAct = new QAction(tr("&Key bindings"), this);
    connect(mKeyAct, SIGNAL(triggered()), this, SLOT(keyBindings()));

    mAboutAct = new QAction(tr("&About"), this);
    connect(mAboutAct, SIGNAL(triggered()), this, SLOT(about()));

    mOnlineHelpAct = new QAction(tr("Online &Help"), this);
    mOnlineHelpAct->setShortcut(tr("Ctrl+H"));
    connect(mOnlineHelpAct, SIGNAL(triggered()), this, SLOT(onlineHelp()));
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
    mFileMenu->addAction(mOpenMoCapAct);
    mFileMenu->addAction(mSaveSeqVidAct);
    mFileMenu->addAction(mSaveSeqVidViewAct);
    mFileMenu->addAction(mSaveImageAct);
    mFileMenu->addAction(mSaveSeqImgAct);
    mFileMenu->addAction(mSaveViewAct);
    mFileMenu->addAction(mSaveSeqViewAct);
    mFileMenu->addAction(mPrintAct);
    mFileMenu->addSeparator();
    mFileMenu->addAction(mResetSettingsAct);
    mFileMenu->addSeparator();
    mFileMenu->addAction(mExitAct);

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

    mDeleteMenu = new QMenu(tr("&Delete"), this);
    mDeleteMenu->addAction(mDelPastAct);
    mDeleteMenu->addAction(mDelFutureAct);
    mDeleteMenu->addAction(mDelAllRoiAct);
    mDeleteMenu->addAction(mDelPartRoiAct);

    mHelpMenu = new QMenu(tr("&Help"), this);
    mHelpMenu->addAction(mCommandAct);
    mHelpMenu->addAction(mKeyAct);
    mHelpMenu->addAction(mAboutAct);
    mHelpMenu->addAction(mOnlineHelpAct);

    menuBar()->addMenu(mFileMenu);
    menuBar()->addMenu(mViewMenu);
    menuBar()->addMenu(mDeleteMenu);
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
    connect(mStatusPosRealHeight, SIGNAL(valueChanged(double)), this, SLOT(setStatusPosReal()));

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
    ///  Noetig damit alle UI Elemente, welche in der neu geladenen Projekt-Datei z.B. noch nicht vorhanden sind, auf
    ///  sinnvolle Werte gesetzt werden. Anderenfalls kommt es evtl. beim nacheinander laden verschiedener Projekte zu
    ///  einem Programmabsturz
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
        mStatusLabelTime->setText(mAnimation->getTimeString());
    }
}

void Petrack::setStatusFPS()
{
    if(mStatusLabelFPS)
    {
        mStatusLabelFPS->setText(QString("%1fps  ").arg(mShowFPS, 5, 'f', 1));

        QPalette pal = mStatusLabelFPS->palette(); // static moeglich?
        QColor   color;

        double diff    = mShowFPS - mAnimation->getFPS();
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
        setStatusPosReal(mImageItem->getPosReal(mMousePosOnImage, getStatusPosRealHeight()));
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
                                    getImageItem()->getAngleToGround(
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
 * @brief Reads (and applies) settings form platform-independent persistent storage
 *
 * The saved size and position of the application window get reconstructed. As well as
 * the options about antialiasing and the usage of OpenGL.
 * mSeqFileName and mProFileName get set, so the "Open Project" and "Open Sequence"
 * dialogues start at correct folder. The old project/sequence is NOT actually loaded.
 */
void Petrack::readSettings()
{
    QSettings settings("Forschungszentrum Juelich GmbH", "PeTrack by Maik Boltes, Daniel Salden");
    QPoint    pos  = settings.value("pos", QPoint(200, 200)).toPoint();
    QSize     size = settings.value("size", QSize(400, 400)).toSize();
    mAntialiasAct->setChecked(settings.value("antialias", false).toBool());
    mOpenGLAct->setChecked(settings.value("opengl", false).toBool());
    mSeqFileName = settings.value("seqFileName", QDir::currentPath()).toString();
    setProFileName(settings.value("proFilePath", QDir::currentPath()).toString());
    // nicht ganz sauber, da so immer schon zu anfang in calib file list etwas drin steht und somit auto ausgefuehrt
    // werden kann wird aber beim ersten openCalib... ueberschrieben
    mAutoCalib.addCalibFile(settings.value("calibFile", QDir::currentPath()).toString());
    resize(size);
    move(pos);
    antialias();
    opengl();
    mSplitter->restoreState(settings.value("controlSplitterSizes").toByteArray());
}

/**
 * @brief Writes persistent setting.
 * @see Petrack::readSettings
 */
void Petrack::writeSettings()
{
    QSettings settings("Forschungszentrum Juelich GmbH", "PeTrack by Maik Boltes, Daniel Salden");
    settings.setValue("pos", pos());
    settings.setValue("size", size());
    settings.setValue("antialias", mAntialiasAct->isChecked());
    settings.setValue("opengl", mOpenGLAct->isChecked());
    settings.setValue("seqFileName", mSeqFileName);
    settings.setValue("proFilePath", QFileInfo(mProFileName).path()); // nur path, damit bei saveCurrentProject
    if(!mAutoCalib.isEmptyCalibFiles())                               //! mCalibFiles.isEmpty()
    {
        settings.setValue("calibFile", mAutoCalib.getCalibFile(0));
    }
    settings.setValue("controlSplitterSizes", mSplitter->saveState());
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
        setStatusPosReal(mImageItem->getPosReal(pos, getStatusPosRealHeight()));

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
        case Qt::Key_Down:
            mViewWidget->zoomOut(1);
            break;
        case Qt::Key_Up:
            mViewWidget->zoomIn(1);
            break;
        case Qt::Key_Space:
            // space wird von buttons, wenn focus drauf ist als Aktivierung vorher abgegriffen und nicht durchgereicht
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
    int iW             = img.cols;
    int iH             = img.rows;

    // wird auch nochmal in ImageWidget gemacht, aber ist hier frueher noetig
    double cX = mControlWidget->getCalibCx(); // merken, da min/max value verandernkann wenn aus dem rahmen
    double cY = mControlWidget->getCalibCy();

    mControlWidget->setCalibCxMin(0 /*iW/2.-50.*/);
    mControlWidget->setCalibCxMax(iW /*iW/2.+50.*/);
    mControlWidget->setCalibCyMin(0 /*iH/2.-50.*/);
    mControlWidget->setCalibCyMax(iH /*iH/2.+50.*/);

    if(mControlWidget->isFixCenterChecked())
    {
        mControlWidget->setCalibCx((iW - 1) / 2.);
        mControlWidget->setCalibCy((iH - 1) / 2.);
    }
    else
    {
        if(lastBorderSize != -1)
        {
            diffBorderSize = getImageBorderSize() - lastBorderSize;
        }
        lastBorderSize = getImageBorderSize();

        mControlWidget->setCalibCx(cX + diffBorderSize);
        mControlWidget->setCalibCy(cY + diffBorderSize);
    }
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
        if(dest.right(4) == ".trc")
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
            TrackPerson tp;
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
                    debout << "Error: wrong header while reading TRC file." << std::endl;
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
                debout << "Warning: Overlapping trajectories will be joined not until tracking adds new trackpoints."
                       << std::endl;
            }
            for(i = 0; i < sz; ++i)
            {
                if(trcVersion == 2)
                {
                    in >> tp;
                }
                else if(trcVersion >= 3)
                {
                    in >> tp;
                }
                mPersonStorage.addPerson(tp);
                tp.clear(); // loeschen, sonst immer weitere pfade angehangen werden
            }

            mControlWidget->setTrackNumberAll(QString("%1").arg(mPersonStorage.nbPersons()));
            mControlWidget->setTrackShowOnlyNr(static_cast<int>(MAX(mPersonStorage.nbPersons(), 1)));
            mControlWidget->setTrackNumberVisible(
                QString("%1").arg(mPersonStorage.visible(mAnimation->getCurrentFrameNum())));
            mControlWidget->replotColorplot();
            file.close();
            debout << "import " << dest << " (" << sz << " person(s), file version " << trcVersion << ")" << std::endl;
            mTrcFileName =
                dest; // fuer Project-File, dann koennte track path direkt mitgeladen werden, wenn er noch da ist
        }
        else if(dest.right(4) == ".txt") // 3D Koordinaten als Tracking-Daten importieren Zeilenformat: Personennr,
                                         // Framenr, x, y, z
        {
            PWarning(
                this,
                tr("PeTrack"),
                tr("Are you sure you want to import 3D data from TXT-File? You have to make sure that the coordinate "
                   "system now is exactly at the same position and orientation than at export time!"));

            QFile file(dest);
            // size of person list
            int sz = 0;

            if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
            {
                // errorstring ist je nach Betriebssystem in einer anderen Sprache!!!!
                PCritical(this, tr("PeTrack"), tr("Cannot open %1:\n%2").arg(dest).arg(file.errorString()));
                return;
            }

            setTrackChanged(true); // flag changes of track parameters
            mTracker->reset();

            QTextStream in(&file);
            TrackPerson tp;
            TrackPoint  tPoint;
            cv::Point2f p2d;

            QString line;
            QString headerline;
            bool    exec_once_flag        = false;
            double  conversionFactorTo_cm = 1.0;
            int     personNr = -1, frameNr = -1, current_personNr = 1;
            float   x, y, z;

            while(1)
            {
                // Falls Datei am Ende letzte Person abspeichern und Lese-Schleife beenden
                if(in.atEnd())
                {
                    tp.setLastFrame(frameNr);
                    mPersonStorage.addPerson(tp);
                    ++sz;
                    tp.clear();
                    break;
                }

                line = in.readLine();

                // Kommentare ueberlesen
                if(line.startsWith("#", Qt::CaseInsensitive))
                {
                    headerline = line;
                    continue;
                }

                if((!exec_once_flag) && (!headerline.contains("cm")))
                {
                    conversionFactorTo_cm = 100.0;
                    exec_once_flag        = true;
                    PWarning(
                        this,
                        tr("PeTrack"),
                        tr("PeTrack will interpret position data as unit [m]. No header with [cm] found."));
                }

                QTextStream stream(&line);

                // Zeile als Stream einlesen Format: [id frame x y z]
                stream >> personNr >> frameNr >> x >> y >> z;

                // convert data to cm
                x = x * conversionFactorTo_cm;
                y = y * conversionFactorTo_cm;
                z = z * conversionFactorTo_cm;

                // 3-dimensionale Berechnung/Anzeige des Punktes
                if(mControlWidget->getCalibCoordDimension() == 0)
                {
                    p2d = mExtrCalibration.getImagePoint(cv::Point3f(x, y, z));
                }
                // 2-dimensionale Berechnung/Anzeige des Punktes
                else
                {
                    QPointF pos = mImageItem->getPosImage(QPointF(x, y), z);
                    p2d.x       = pos.x();
                    p2d.y       = pos.y();
                }

                tPoint = TrackPoint(Vec2F(p2d.x, p2d.y), 100);
                tPoint.setSp(
                    x,
                    y,
                    -mControlWidget->getCalibExtrTrans3() -
                        z); // fuer den Abstand zur Kamera in z-Richtung wie bei einer Stereokamera

                // Neue ID ? ==> letzte Person beendet ==> abspeichern
                if(personNr > current_personNr)
                {
                    mPersonStorage.addPerson(tp);
                    ++sz;
                    current_personNr++;
                    tp.clear();
                }

                // TrackPerson leer ? ==> Neue TrackPerson erstellen
                if(tp.isEmpty())
                {
                    tp = TrackPerson(personNr, frameNr, tPoint);
                    tp.setFirstFrame(frameNr);
                    tp.setHeight(z);
                }
                // TrackPoint an TrackPerson anhaengen
                else
                {
                    tp.setLastFrame(frameNr);
                    tp.append(tPoint);
                }
            }

            mControlWidget->setTrackNumberAll(QString("%1").arg(mPersonStorage.nbPersons()));
            mControlWidget->setTrackShowOnlyNr(static_cast<int>(MAX(mPersonStorage.nbPersons(), 1)));
            mControlWidget->setTrackNumberVisible(
                QString("%1").arg(mPersonStorage.visible(mAnimation->getCurrentFrameNum())));
            mControlWidget->replotColorplot();
            file.close();
            debout << "import " << dest << " (" << sz << " person(s) )" << std::endl;
            mTrcFileName =
                dest; // fuer Project-File, dann koennte track path direkt mitgeladen werden, wenn er noch da ist
        }
        else
        {
            PCritical(this, tr("PeTrack"), tr("Cannot load %1 maybe because of wrong file extension.").arg(dest));
        }
        lastFile = dest;
    }
}

void Petrack::testTracker()
{
    static int idx = 0; // index in Fehlerliste, die als letztes angesprungen wurde
    QList<int> pers, frame;

    mPersonStorage.checkPlausibility(
        pers,
        frame,
        mControlWidget->isTestEqualChecked(),
        mControlWidget->isTestVelocityChecked(),
        mControlWidget->isTestInsideChecked(),
        mControlWidget->isTestLengthChecked());
    if(pers.length() <= idx)
    {
        idx = 0;
    }
    if(pers.length() > idx)
    {
        mControlWidget->setTrackShowOnly(Qt::Checked);
        mControlWidget->setTrackShowOnlyNr(pers[idx]);
        mPlayerWidget->skipToFrame(frame[idx]);
        ++idx;
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
        mImageItem,
        mControlWidget->getColorPlot(),
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
    if(mTracker)
    {
        // if no destination file or folder is given
        if(dest.isEmpty())
        {
            QFileDialog fileDialog(
                this,
                tr("Select file for exporting tracking pathes"),
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
        }

        if(!dest.isEmpty())
        {
            QList<int> pers, frame;
            bool autoCorrectOnlyExport = (mReco.getRecoMethod() == reco::RecognitionMethod::MultiColor) && // multicolor
                                         mMultiColorMarkerWidget->autoCorrect->isChecked() &&
                                         mMultiColorMarkerWidget->autoCorrectOnlyExport->isChecked();

            if(dest.right(4) == ".trc")
            {
#ifdef TIME_MEASUREMENT
                double time1 = 0.0, tstart;
                tstart       = clock();
#endif
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

                debout << "export tracking data to " << dest << " (" << mPersonStorage.nbPersons()
                       << " person(s), file version " << trcVersion << ")..." << std::endl;
                QTextStream out(&file);

                out << "version " << trcVersion << Qt::endl;
                out << mPersonStorage.nbPersons() << Qt::endl;
                const auto &persons = mPersonStorage.getPersons();
                for(size_t i = 0; i < persons.size(); ++i)
                {
                    qApp->processEvents();
                    progress.setLabelText(
                        QString("Export person %1 of %2 ...").arg(i + 1).arg(mPersonStorage.nbPersons()));
                    progress.setValue(static_cast<int>(i + 1));
                    out << persons[i] << Qt::endl;
                }
                file.flush();
                file.close();
#ifdef TIME_MEASUREMENT
                time1 += clock() - tstart;
                time1 = time1 / CLOCKS_PER_SEC;
                cout << "  time(writing) = " << time1 << " sec." << endl;

                time1  = 0.0;
                tstart = clock();
#endif
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

                std::cout << " finished " << std::endl;
                mAutosave.resetTrackPersonCounter();

#ifdef TIME_MEASUREMENT
                time1 += clock() - tstart;
                time1 = time1 / CLOCKS_PER_SEC;
                cout << "  time(copying) = " << time1 << " sec." << endl;

//                time1 = 0.0;
//                tstart = clock();
#endif

#ifdef TIME_MEASUREMENT
//                time1 += clock() - tstart;
//                time1 = time1/CLOCKS_PER_SEC;
//                cout << "  time(checkPlausibility) = " << time1 << " sec." << endl;
#endif
                mTrcFileName =
                    dest; // fuer Project-File, dann koennte track path direkt mitgeladen werden, wenn er noch da ist
            }
            else if(dest.right(4) == ".txt")
            {
                QTemporaryFile file;

                if(!file.open())
                {
                    PCritical(this, tr("PeTrack"), tr("Cannot open %1:\n%2.").arg(dest).arg(file.errorString()));
                    return;
                }

                debout << "export tracking data to " << dest << " (" << mPersonStorage.nbPersons() << " person(s))..."
                       << std::endl;

#ifdef TIME_MEASUREMENT
                double time1 = 0.0, tstart;
                tstart       = clock();
#endif
                // recalcHeight true, wenn personenhoehe ueber trackpoints neu berechnet werden soll (z.b. um waehrend
                // play mehrfachberuecksichtigung von punkten auszuschliessen, aenderungen in altitude neu in berechnung
                // einfliessen zu lassen)
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
#ifdef TIME_MEASUREMENT
                time1 += clock() - tstart;
                time1 = time1 / CLOCKS_PER_SEC;
                cout << "  time(recalcHeight) = " << time1 << " sec." << endl;

                time1  = 0.0;
                tstart = clock();
#endif
                mTrackerReal->calculate(
                    this,
                    mTracker,
                    mImageItem,
                    mControlWidget->getColorPlot(),
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
#ifdef TIME_MEASUREMENT
                time1 += clock() - tstart;
                time1 = time1 / CLOCKS_PER_SEC;
                cout << "  time(calculate) = " << time1 << " sec." << endl;

                time1  = 0.0;
                tstart = clock();
#endif

                QTextStream out(&file);

                out << "# PeTrack project: " << QFileInfo(getProFileName()).fileName() << Qt::endl;
                out << "# raw trajectory file: " << QFileInfo(getTrackFileName()).fileName() << Qt::endl;
                out << "# framerate: " << mAnimation->getFPS() << " fps" << Qt::endl;

                if(mControlWidget->isExportCommentChecked())
                {
                    out << "# personal information:" << Qt::endl;
                    out << "# ID| Comment" << Qt::endl;

                    // std out
                    std::cout << std::endl << "Printing comment table..." << std::endl << std::endl;
                    std::cout << "ID  | Comment" << std::endl;
                    std::cout << "----|----------------" << std::endl;

                    for(int i = 0; i < static_cast<int>(mPersonStorage.nbPersons()); ++i)
                    {
                        auto commentSplit = mPersonStorage.at(i).comment().split("\n", Qt::KeepEmptyParts);
                        out << "#" << qSetFieldWidth(3) << (i + 1) << qSetFieldWidth(0) << "|" << commentSplit.at(0)
                            << Qt::endl;
                        std::cout << std::setw(4) << (i + 1) << "|" << commentSplit.at(0) << std::endl;

                        commentSplit.pop_front();
                        for(const QString &line : commentSplit)
                        {
                            out << "#" << qSetFieldWidth(3) << " " << qSetFieldWidth(0) << "|" << line << Qt::endl;
                            std::cout << "    |" << line << std::endl;
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

                std::cout << " finished" << std::endl;

#ifdef TIME_MEASUREMENT
                time1 += clock() - tstart;
                time1 = time1 / CLOCKS_PER_SEC;
                cout << "  time(export) = " << time1 << " sec." << endl;

//                time1 = 0.0;
//                tstart = clock();
#endif

#ifdef TIME_MEASUREMENT
//                time1 += clock() - tstart;
//                time1 = time1/CLOCKS_PER_SEC;
//                cout << "  time(checkPlausibility) = " << time1 << " sec." << endl;
#endif
            }
            else if(dest.right(4) == ".dat")
            {
                QTemporaryFile fileDat;

                if(!fileDat.open()) //! fileDat.open(QIODevice::WriteOnly | QIODevice::Text))
                {
                    PCritical(this, tr("PeTrack"), tr("Cannot open %1:\n%2.").arg(dest).arg(fileDat.errorString()));
                    return;
                }
                // recalcHeight true, wenn personenhoehe ueber trackpoints neu berechnet werden soll (z.b. um waehrend
                // play mehrfachberuecksichtigung von punkten auszuschliessen, aenderungen in altitude neu in berechnung
                // einfliessen zu lassen)
                if(mControlWidget->isTrackRecalcHeightChecked())
                {
                    mPersonStorage.recalcHeight(mControlWidget->getCameraAltitude());
                }
                mTrackerReal->calculate(
                    this,
                    mTracker,
                    mImageItem,
                    mControlWidget->getColorPlot(),
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

                debout << "export tracking data to " << dest << " (" << mPersonStorage.nbPersons() << " person(s))..."
                       << std::endl;
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

                std::cout << " finished" << std::endl;
            }
            else if(dest.right(5) == ".trav")
            {
                // recalcHeight true, wenn personenhoehe ueber trackpoints neu berechnet werden soll (z.b. um waehrend
                // play mehrfachberuecksichtigung von punkten auszuschliessen, aenderungen in altitude neu in berechnung
                // einfliessen zu lassen)
                if(mControlWidget->isTrackRecalcHeightChecked())
                {
                    mPersonStorage.recalcHeight(mControlWidget->getCameraAltitude());
                }

                mTrackerReal->calculate(
                    this,
                    mTracker,
                    mImageItem,
                    mControlWidget->getColorPlot(),
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
                debout << "export tracking data to " << dest << " (" << mPersonStorage.nbPersons() << " person(s))..."
                       << std::endl;
                // already done: mTrackerReal->calculate(mTracker, mImageItem, mControlWidget->getColorPlot(),
                // getImageBorderSize(), mControlWidget->isTrackMissingFramesChecked());
                QTextStream outXml(&fileXml);
                outXml << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << Qt::endl;
                outXml << "<trajectoriesDataset>" << Qt::endl;
                outXml << "    <header version=\"1.0\">" << Qt::endl;
                outXml << "        <roomCaption>PeTrack: " << mAnimation->getFileBase() << "</roomCaption>" << Qt::endl;
                outXml << "        <roomID>0</roomID>" << Qt::endl;
                outXml << "        <agents>" << mPersonStorage.nbPersons() << "</agents>" << Qt::endl;
                outXml << "        <frameRate>" << mAnimation->getFPS() << "</frameRate> <!--per second-->" << Qt::endl;
                // outXml << "        <timeStep>" << 1000./mAnimation->getFPS() << "</timeStep>   <!-- millisecond-->"
                // << endl; inverse von
                outXml << "        <timeFirstFrame sec=\"" << mAnimation->getFirstFrameSec() << "\" microsec=\""
                       << mAnimation->getFirstFrameMicroSec() << "\"/> <!-- " << mAnimation->getTimeString(0) << " -->"
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

                std::cout << " finished" << std::endl;
            }
            else
            { // wenn keine Dateiendung, dann wird trc und txt herausgeschrieben
                exportTracker(dest + ".trc");
                exportTracker(dest + ".txt");
            }
            mLastTrackerExport = dest;
        }
    }
}

// fuer anschliessende groessenberechnung
void Petrack::playAll()
{
    int memPos  = mPlayerWidget->getPos();
    int progVal = 0;

    QProgressDialog progress("Playing whole sequence...", "Abort playing", 0, mAnimation->getNumFrames(), this);
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
        2 * mAnimation->getNumFrames() - memPos,
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
        // progVal = 2*mAnimation->getNumFrames()-memPos-mPlayerWidget->getPos();
        progVal += mAnimation->getNumFrames() - mPlayerWidget->getPos();
        progress.setValue(progVal); // mPlayerWidget->getPos()

        // recognition abstellen, bis an die stelle, wo trackAll begann
        // UEBERPRUEFEN, OB TRACKPATH NICHT RECOGNITION PUNKTE UEBERSCHREIBT!!!!!!!!!!
        // repeate und repaetQual koennte temporaer umgestellt werden
        mControlWidget->setPerformRecognitionChecked(false);

        // rueckwaertslaufen
        do
        {
            if(progVal + 1 < 2 * mAnimation->getNumFrames() - memPos)
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
        progress.setValue(2 * mAnimation->getNumFrames() - memPos);
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

void Petrack::updateImage(bool imageChanged) // default = false (only true for new animation frame)
{
#ifdef TIME_MEASUREMENT
    // die reine Ausgabe  folgender Zeile kostet 1-2 Millisekunden
    //        "==========: "
    debout << "go  update: " << getElapsedTime() << endl;
#endif

    mCodeMarkerItem->resetSavedMarkers();

    static int  lastRecoFrame            = -10000;
    static bool borderChangedForTracking = false;

    // need semaphore to guarrantee that updateImage only called once
    // updateValue of control automatically calls updateImage!!!
    static QSemaphore semaphore(1);
    if(!mImg.empty() && mImage && semaphore.tryAcquire())
    {
        int frameNum = mAnimation->getCurrentFrameNum();

        setStatusTime();

        updateShowFPS();

        mImgFiltered = mImg;

        // have to store because evaluation sets the filter parameter to unchanged
        bool brightContrastChanged = mBrightContrastFilter.changed();
        bool swapChanged           = mSwapFilter.changed();
        bool borderChanged         = mBorderFilter.changed();
        bool calibChanged          = mCalibFilter->changed();

        // speicherverwaltung wird komplett von filtern ueberneommen

        // Filter anwenden, Reihenfolge wichtig - Rechenintensive moeglichst frueh
        // fkt so nur mit kopierenden filtern
#ifdef TIME_MEASUREMENT
        //        "==========: "
        debout << "vor filter: " << getElapsedTime() << endl;
#endif

        if(imageChanged || swapChanged)
        {
            mImgFiltered = mSwapFilter.apply(mImgFiltered);
        }
        else
        {
            mImgFiltered = mSwapFilter.getLastResult();
        }

        if(imageChanged || swapChanged || brightContrastChanged)
        {
            mImgFiltered = mBrightContrastFilter.apply(mImgFiltered);
        }
        else
        {
            mImgFiltered = mBrightContrastFilter.getLastResult();
        }

        if(imageChanged || swapChanged || brightContrastChanged || borderChanged)
        {
            mImgFiltered = mBorderFilter.apply(mImgFiltered); // mIplImg
        }
        else
        {
            mImgFiltered = mBorderFilter.getLastResult();
        }

#ifdef TIME_MEASUREMENT
        //        "==========: "
        debout << "nch filter: " << getElapsedTime() << endl;
#endif
        if(borderChanged)
        {
            updateControlImage(mImgFiltered);
        }
#ifndef STEREO_DISABLED
        if(imageChanged || swapChanged || brightContrastChanged || borderChanged || calibChanged)
        {
            if(mStereoContext)
                mStereoContext->init(mImgFiltered);
        }
#endif

#ifdef TIME_MEASUREMENT
        //        "==========: "
        debout << "vor  calib: " << getElapsedTime() << endl;
#endif
        if(imageChanged || swapChanged || brightContrastChanged || borderChanged || calibChanged)
        {
            mImgFiltered = mCalibFilter->apply(mImgFiltered);
        }
        else
        {
            mImgFiltered = mCalibFilter->getLastResult();
        }

#ifdef TIME_MEASUREMENT
        //        "==========: "
        debout << "nach calib: " << getElapsedTime() << endl;
#endif

        if(brightContrastChanged || swapChanged || borderChanged || calibChanged)
        {
            // abfrage hinzugenommen, damit beim laden von .pet bg-file angegeben werden kann fuer mehrere versuche und
            // beim nachladen von versuch nicht bg geloescht wird
            if(mBackgroundFilter.getFilename() != "")
            {
                debout << "Warning: No background reset, because of explicit loaded background image!" << std::endl;
            }
            else
            {
                mBackgroundFilter
                    .reset(); // alle gesammelten hintergrundinfos werden verworfen und bg.changed auf true gesetzt
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

#ifdef TIME_MEASUREMENT
        //        "==========: "
        debout << "nach    bg: " << getElapsedTime() << endl;
#endif

        // delete track list, if intrinsic param have changed
        if(calibChanged && mPersonStorage.nbPersons() > 0) // mCalibFilter.getEnabled() &&
        {
            // Evtl. nicht Tracker loeschen sondern entsprechend der neuen Calibration verschieben?!?!?
            mPersonStorage.clear();
            mTracker->reset();
            if(!isLoading())
            {
                debout << "Warning: deleted all tracking pathes because intrinsic parameters have changed."
                       << std::endl;
            }
        }
        else
        {
#ifndef STEREO_DISABLED
            // calculate position in 3D space and height of person for "old" trackPoints, if checked "even"
            if(mStereoContext && mStereoWidget->stereoUseForHeightEver->isChecked() &&
               mStereoWidget->stereoUseForHeight->isChecked())
            {
                // buildt disparity picture if it should be used for height detection
                mStereoContext->getDisparity();

                mPersonStorage.calcPosition(frameNum);
            }
#endif
        }
        if(borderChanged)
        {
            borderChangedForTracking = true;
        }
        // tracking vor recognition, da dann neu gefundene punkte mit getrackten bereits ueberprueft werden koennen
        if((trackChanged() || imageChanged) && (mControlWidget->isOnlineTrackingChecked())) // borderChanged ???
        {
            // Rect for tracking area
            QRect roi(
                myRound(mTrackingRoiItem->rect().x() + getImageBorderSize()),
                myRound(mTrackingRoiItem->rect().y() + getImageBorderSize()),
                myRound(mTrackingRoiItem->rect().width()),
                myRound(mTrackingRoiItem->rect().height()));

            if(borderChangedForTracking)
            {
                cv::Size size;
                size.width  = mImgFiltered.cols;
                size.height = mImgFiltered.rows;
                mTracker->resize(size);

                mTrackingRoiItem->restoreSize();
            }
#ifndef STEREO_DISABLED
            // buildt disparity picture if it should be used for height detection
            if(mStereoContext && mStereoWidget->stereoUseForHeight->isChecked())
                mStereoContext->getDisparity();
#endif

            cv::Rect rect;
            getRoi(mImgFiltered, roi, rect);
            // Ignore all tracking points outside of rect

            // if (mPrevIplImgFiltered) // wenn ein vorheriges bild vorliegt
            //  mPrevIplImgFiltered == NULL zeigt an, dass neue bildfolge && mPrevFrame == -1 ebenso
            //  winSize(), wurde mal uebergeben
#ifdef TIME_MEASUREMENT
            debout << "vor  track: " << getElapsedTime() << endl;
#endif
            //            debout << "test" << endl;
            int anz = mTracker->track(
                mImgFiltered,
                rect,
                frameNum,
                mControlWidget->isTrackRepeatChecked(),
                mControlWidget->getTrackRepeatQual(),
                getImageBorderSize(),
                mReco.getRecoMethod(),
                mControlWidget->getTrackRegionLevels(),
                getPedestriansToTrack());
#ifdef TIME_MEASUREMENT
            debout << "nach track: " << getElapsedTime() << endl;
#endif
            mControlWidget->setTrackNumberNow(QString("%1").arg(anz));
            mTrackChanged            = false;
            borderChangedForTracking = false;
        }
        else
        {
            mControlWidget->setTrackNumberNow(QString("0"));
        }
        // hier muesste fuer ameisen etc allgemeinABC.getPosList(...)

        if(((((lastRecoFrame + mControlWidget->getRecoStep()) <= frameNum) ||
             ((lastRecoFrame - mControlWidget->getRecoStep()) >= frameNum)) &&
            imageChanged) ||
           mAnimation->isCameraLiveStream() || swapChanged || brightContrastChanged || borderChanged || calibChanged ||
           recognitionChanged())
        {
#ifndef STEREO_DISABLED
            // buildt disparity picture if it should be used for height detection or recognition
            if(mStereoContext &&
               (mStereoWidget->stereoUseForHeight->isChecked() || mStereoWidget->stereoUseForReco->isChecked()))
                mStereoContext->getDisparity(); // wird nicht neu berechnet, wenn vor tracking schon berechnet wurde
#endif
            if(borderChanged)
            {
                mRecognitionRoiItem->restoreSize();
            }

            if(mControlWidget->isPerformRecognitionChecked())
            {
                QRect rect(
                    myRound(mRecognitionRoiItem->rect().x() + getImageBorderSize()),
                    myRound(mRecognitionRoiItem->rect().y() + getImageBorderSize()),
                    myRound(mRecognitionRoiItem->rect().width()),
                    myRound(mRecognitionRoiItem->rect().height()));
                QList<TrackPoint> persList;
                auto              recoMethod = mReco.getRecoMethod();
#ifdef TIME_MEASUREMENT
                //        "==========: "
                debout << "vor   reco: " << getElapsedTime() << endl;
#endif
                if((recoMethod == reco::RecognitionMethod::Casern) || (recoMethod == reco::RecognitionMethod::Hermes) ||
                   (recoMethod == reco::RecognitionMethod::Color) || (recoMethod == reco::RecognitionMethod::Japan) ||
                   (recoMethod == reco::RecognitionMethod::MultiColor) ||
                   (recoMethod == reco::RecognitionMethod::Code)) // else
                {
                    persList = mReco.getMarkerPos(
                        mImgFiltered, rect, mControlWidget, getImageBorderSize(), getBackgroundFilter());
                }
#ifndef STEREO_DISABLED
                if(mStereoContext && mStereoWidget->stereoUseForReco->isChecked())
                {
                    PersonList pl;
                    pl.calcPersonPos(mImgFiltered, rect, &persList, mStereoContext, getBackgroundFilter(), markerLess);
                }
#endif
#ifdef TIME_MEASUREMENT
                //        "==========: "
                debout << "nach  reco: " << getElapsedTime() << endl;
#endif
                mPersonStorage.addPoints(persList, frameNum, mReco.getRecoMethod());

                // folgendes lieber im Anschluss, ggf beim exportieren oder statt test direkt del:
                if(mStereoContext && mStereoWidget->stereoUseForReco->isChecked())
                {
                    mPersonStorage.purge(frameNum); // bereinigen wenn weniger als 0.2 recognition und nur getrackt
                }

                mControlWidget->setRecoNumberNow(QString("%1").arg(persList.size()));
                mRecognitionChanged = false;

                if(false) // hier muss Abfage hin ob kasernen marker genutzt wird
                {
                    mControlWidget->getColorPlot()
                        ->replot(); // oder nur wenn tab offen oder wenn sich mtracker geaendert hat???
                }
            }
            else
            {
                mControlWidget->setRecoNumberNow(QString("0"));
            }
            lastRecoFrame = frameNum;
        }
        else
        {
            mControlWidget->setRecoNumberNow(QString("0"));
        }

        mControlWidget->setTrackNumberAll(
            QString("%1").arg(mPersonStorage.nbPersons())); // kann sich durch reco und tracker aendern
        mControlWidget->setTrackShowOnlyNrMaximum(
            static_cast<int>(MAX(mPersonStorage.nbPersons(), 1))); // kann sich durch reco und tracker aendern
        mControlWidget->setTrackNumberVisible(
            QString("%1").arg(mPersonStorage.visible(frameNum))); // kann sich durch reco und tracker aendern

        // in anzuzeigendes Bild kopieren
        // erst hier wird die bildgroesse von mimage an filteredimg mit border angepasst
        copyToQImage(*mImage, mImgFiltered);

        if(borderChanged)
        {
            mImageItem->setImage(mImage);
        }
        else
        {
            getScene()->update(); // repaint();
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

        semaphore.release();
    }
#ifdef TIME_MEASUREMENT
    //        "==========: "
    debout << "stp update: " << getElapsedTime() << endl;
#endif
}
void Petrack::updateImage(const cv::Mat &img)
{
    mImg = img;
    updateImage(true);
}


void Petrack::updateSequence()
{
    QImage *oldImage = mImage;

    QSize size = mAnimation->getSize();
    size.setWidth(size.width() + 2 * getImageBorderSize()); // border is inside the mImage!
    size.setHeight(size.height() + 2 * getImageBorderSize());
    mImage = new QImage(size, QImage::Format_RGB888); // 32); //wird in updateImage gemacht

    // set roi for recognition if image size changes or roi is zero
    // in oldImage steckt border drin, mIplImg->height zeigt noch auf altes ursprungsbild
    // mRecognitionRoiItem->rect().width() != 0 && oldImage == NULL wenn projektdatei eingelesen wird!!!!!
    if((mRecognitionRoiItem->rect().width() == 0) || // default while initialization, after that >= MIN_SIZE
       (oldImage && ((oldImage->width() != mImage->width()) || (oldImage->height() != mImage->height()))))
    {
        mRecognitionRoiItem->setRect(-getImageBorderSize(), -getImageBorderSize(), mImage->width(), mImage->height());
    }
    if((mTrackingRoiItem->rect().width() == 0) ||
       (oldImage && ((oldImage->width() != mImage->width()) || (oldImage->height() != mImage->height()))))
    {
        mTrackingRoiItem->setRect(-getImageBorderSize(), -getImageBorderSize(), mImage->width(), mImage->height());
    }

    cv::Size size2;
    size2.width  = mTrackingRoiItem->rect().width();
    size2.height = mTrackingRoiItem->rect().height();
    mTracker->init(size2);

    mPlayerWidget->setAnim(mAnimation);
    mPlayerWidget->skipToFrame(0);
    mImageItem->setImage(mImage); // wird in updateImage gemacht
    delete oldImage;
    mSaveSeqVidAct->setEnabled(true);
    mSaveSeqVidViewAct->setEnabled(true);
    mSaveSeqImgAct->setEnabled(true);
    mSaveSeqViewAct->setEnabled(true);
    mSaveImageAct->setEnabled(true);
    mSaveViewAct->setEnabled(true);
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
        mCmPerPixel = getImageItem()->getCmPerPixel();
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
                return (HEAD_SIZE * mControlWidget->getCameraAltitude() / z) / getImageItem()->getCmPerPixel();
            }
            else if(h > MIN_HEIGHT)
            {
                return (HEAD_SIZE * mControlWidget->getCameraAltitude() / (mControlWidget->getCameraAltitude() - h)) /
                       getImageItem()->getCmPerPixel();
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
 * The given text will be split on ',' and then each element will be checked if it is a range. Ranges are marked with
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
    mControlWidget->setTrackShowOnlyNr(static_cast<int>(MAX(mPersonStorage.nbPersons(), 1)));
    mControlWidget->setTrackNumberVisible(QString("%1").arg(mPersonStorage.visible(mAnimation->getCurrentFrameNum())));
}

void Petrack::splitTrackPerson(QPointF pos)
{
    mPersonStorage.splitPersonAt((Vec2F) pos, mAnimation->getCurrentFrameNum(), getPedestrianUserSelection());
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
        tP, mAnimation->getCurrentFrameNum(), getPedestrianUserSelection(), mReco.getRecoMethod(), &pers);
    updateControlWidget();
    return pers;
}

// direction zeigt an, ob bis zum aktuellen (-1), ab dem aktuellen (1) oder ganzer trackpath (0)
// loeschen von Trackpoints einer Trajektorie
void Petrack::deleteTrackPoint(QPointF pos, int direction) // const QPoint &pos
{
    mPersonStorage.delPoint((Vec2F) pos, direction, mAnimation->getCurrentFrameNum(), getPedestrianUserSelection());
    updateControlWidget();
}
void Petrack::editTrackPersonComment(QPointF pos)
{
    mPersonStorage.editTrackPersonComment((Vec2F) pos, mAnimation->getCurrentFrameNum(), getPedestrianUserSelection());
    updateControlWidget();
}
void Petrack::setTrackPersonHeight(QPointF pos)
{
    mPersonStorage.setTrackPersonHeight((Vec2F) pos, mAnimation->getCurrentFrameNum(), getPedestrianUserSelection());
    updateControlWidget();
}
void Petrack::resetTrackPersonHeight(QPointF pos)
{
    mPersonStorage.resetTrackPersonHeight((Vec2F) pos, mAnimation->getCurrentFrameNum(), getPedestrianUserSelection());
    updateControlWidget();
}

/**
 * @brief Delete the following, previous or whole trajectory of **all** trajectories
 * @param direction previous, following or whole
 */
void Petrack::deleteTrackPointAll(PersonStorage::Direction direction) // const QPoint &pos
{
    mPersonStorage.delPointAll(direction, mAnimation->getCurrentFrameNum());
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
    auto successfullySelected =
        mManualTrackPointMover.selectTrackPoint(pos, mPersonStorage, getPedestrianUserSelection(), range);

    if(successfullySelected)
    {
        setCursor(QCursor{Qt::CursorShape::DragMoveCursor});
    }
}

void Petrack::releaseTrackPoint()
{
    mManualTrackPointMover.setTrackPoint();
    mAutosave.trackPersonModified();
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
    mPlayerWidget->setFrameInNum(mAnimation->getSourceInFrameNum());
    mPlayerWidget->setFrameOutNum(mAnimation->getSourceOutFrameNum());
}

// delta gibt menge an Umdrehungen und richtung an
void Petrack::skipToFrameWheel(int delta)
{
    mPlayerWidget->skipToFrame(mPlayerWidget->getPos() + delta);
}

void Petrack::skipToFrameFromTrajectory(QPointF pos)
{
    auto       peds      = getPedestrianUserSelection();
    const auto before    = mControlWidget->getTrackShowBefore();
    const auto after     = mControlWidget->getTrackShowAfter();
    const auto currFrame = mPlayerWidget->getPos();
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

void Petrack::setPeTrackVersion(const std::string &petrackVersion)
{
    mPetrackVersion = QString::fromStdString(petrackVersion);
    updateWindowTitle();
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
    const std::string &compileTimeStamp,
    const std::string &compilerID,
    const std::string &compilerVersion)
{
    mCompileDate     = QString::fromStdString(compileTimeStamp);
    mCompilerID      = QString::fromStdString(compilerID);
    mCompilerVersion = QString::fromStdString(compilerVersion);
}

#include "moc_petrack.cpp"
