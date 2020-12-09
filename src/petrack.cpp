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

#include <QtWidgets>
#include <QtOpenGL>
#include <QSignalMapper>

// Added for Qt5 support
#include <QtPrintSupport/QPrinter>
#include <QtPrintSupport/QPrintDialog>

#include "stereoItem.h"
#include "stereoWidget.h"
#include "colorMarkerItem.h"
#include "multiColorMarkerItem.h"
#include "colorRangeWidget.h"
#include "colorMarkerWidget.h"
#include "codeMarkerWidget.h"
#include "multiColorMarkerWidget.h"
#include "petrack.h"
#include "control.h"
#include "view.h"
#include "gridItem.h"
#include "imageItem.h"
#include "logoItem.h"
#include "animation.h"
#include "player.h"
#include "calibFilter.h"
#include "autoCalib.h"
#include "recognition.h"
#include "trackerItem.h"
#include "backgroundItem.h"
#include "tracker.h"
#include "trackerReal.h"
#include "cmath"
#ifdef AVI
#include "aviFile.h"
#else
#include "aviFileWriter.h"
#endif
#include "person.h"

#include <time.h>

#include <cstdlib>

#include <iomanip>

#include "opencv2/opencv.hpp"

// Zeitausgabe aktivieren
//#define TIME_MEASUREMENT

//temp muss spaeter herausgenommen werden,
// dient dazu, in anderen dateien um schnell ueber cw->temp1->value() an einstellbare daten zu kommen
Control *cw;

int Petrack::trcVersion = 0;

using namespace::cv;
using namespace std;

// Reihenfolge des anlegens der objekte ist sehr wichtig
Petrack::Petrack()
{
    QIcon icon;
    icon.addFile(":/icon"); // about
    //     icon.addFile(":/icon_small");
    //     icon.addFile(":/icon_smaller");
    icon.addFile(":/icon_smallest"); // window title bar
    setWindowIcon(icon); // QIcon(":/icon")fuer about und icon in windows titel bar top left
    mHeadSize = -1;
    mCmPerPixel = -1;
    mScene = NULL;
    mTracker = NULL;
    mTrackerReal = NULL; // damit beim zeichnen von control mit analysePlot nicht auf einen feheler laeuft
    mStatusLabelFPS = NULL;
    mStatusPosRealHeight = NULL;
    mStatusLabelPosReal = NULL;
    mImageItem = NULL;
    mRecognitionChanged = true;
    mTrackChanged = true;
    mCoordItem = NULL;
    mImage = NULL;
//    mIplImg = NULL;
//    mIplImgFiltered = NULL;
    setLoading(true);
    //     mPrevIplImgFiltered = NULL;
    //     mPrevFrame = -1;

    setAcceptDrops(true);

//    grabKeyboard(); // nimmt alle Keyboard Eingaben entgegen, eintragen in Text-Felder dann nicht mehr moeglich
//    grabMouse(); // nimmt alle MouseEvents eintgegen, Doppelklick etc. dann nicht mehr moeglich

    int space = 2;
    //    setAccessibleName("PeTrack");

    mBrightContrastFilter.disable();
    //     mContrastFilter.disable();
    //     mBrightFilter.disable();
    mBorderFilter.disable();
    mSwapFilter.disable();
    mBackgroundFilter.disable();
    mStereoContext = NULL;
    mCalibFilter = new CalibFilter; // schoener waere erst zu erzeugen, wenn video geladen wird, da sonst bei stereo erst normealer und dann stereo objekt erzeugt wird
    mCalibFilter->disable();        // aber control widget greift schon bei erzeugung auf alle objekte zur einstellung zurueck
    //mCalibFilter.disable(); //enable();

    //mImageBorderSize=0;

    mControlWidget = new Control(this);
    mControlWidget->setFixedWidth(mControlWidget->geometry().width());
    mControlWidget->setMinimumHeight(mControlWidget->geometry().height());
    mControlWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    //setFixedSize(mControlWidget->geometry().width(),mControlWidget->geometry().height());
    cw =  mControlWidget; // muss spaeter geloescht werden

    mStereoWidget = new StereoWidget(this);
    mStereoWidget->setWindowFlags(Qt::Window);
    mStereoWidget->setWindowTitle("Stereo parameter");
    //mStereoWidget->show();

    mColorRangeWidget = new ColorRangeWidget(this);
    mColorRangeWidget->setWindowFlags(Qt::Window);
    mColorRangeWidget->setWindowTitle("Color range");

    mColorMarkerWidget = new ColorMarkerWidget(this);
    mColorMarkerWidget->setWindowFlags(Qt::Window);
    mColorMarkerWidget->setWindowTitle("Color marker parameter");
    //mColorMarkerWidget->show();

    mCodeMarkerWidget = new CodeMarkerWidget(this);
    mCodeMarkerWidget->setWindowFlags(Qt::Window);
    mCodeMarkerWidget->setWindowTitle("Code marker parameter");
    //mCodeMarkerWidget->show();


    mMultiColorMarkerWidget = new MultiColorMarkerWidget(this);
    mMultiColorMarkerWidget->setWindowFlags(Qt::Window);
    mMultiColorMarkerWidget->setWindowTitle("MultiColor marker parameter");
    //mColorMarkerWidget->show();

    mScene = new QGraphicsScene(this);

    mImageItem = new ImageItem(this); // durch uebergabe von scene wird indirekt ein scene->addItem() aufgerufen
    mControlWidget->setScene(mScene);

    // wird in controll.cpp behandelt!!!
    //     if (mControlWidget->filterBright->checkState() == Qt::Checked)
    //         mBrightFilter.enable();
    //     else //Qt::Unchecked
    //         mBrightFilter.disable();
    //     if (mControlWidget->filterContrast->checkState() == Qt::Checked)
    //         mContrastFilter.enable();
    //     else //Qt::Unchecked
    //         mContrastFilter.disable();
    //     if (mControlWidget->filterContrast->checkState() == Qt::Checked)
    //         mContrastFilter.enable();
    //     else //Qt::Unchecked
    //         mContrastFilter.disable();

    mAnimation = new Animation(this);

    //     mImage = new QImage; // wird erst bei neuer sequenz erstellt, da dort groesse bekannt,
    //      zudem kann so ueber mImage==NULL beginn abgefragt werden
    //     // hier koennte ein Einfuehrungsbild mit logo etc eingeblendet werden
    //     mImage->load(":/logo");
    //     //C:/Dokumente und Einstellungen/Maik Boltes/Eigene Dateien/diss/logo/logo.png
    //     mImageItem->setLogo(mImage);
    //     // so wurde gruene recogniton area zurueckgesetzt:
    //     //mImageItem->setImage(mImage);
    mLogoItem = new LogoItem(this); // durch uebergabe von scene wird indirekt ein scene->addItem() aufgerufen
    mLogoItem->setZValue(6); // groesser heisst weiter oben

    mExtrCalibration.setMainWindow(this);
    //mExtCalibration = new ExtrCalibration(this); // muss vor CoordItem (und GridItem) stehen damit man auf useExtCalib abfragen kann

    mGridItem = new GridItem(this);
    mGridItem->setZValue(2.5); // durch uebergabe von scene wird indirekt ein scene->addItem() aufgerufen

    mCoordItem = new CoordItem(this);
    mCoordItem->setZValue(3); // groesser heisst weiter oben
    mImageItem->setCoordItem(mCoordItem);

    mTrackingRoiItem = new TrackingRoiItem(this);
    mTrackingRoiItem->setZValue(4); // groesser heisst weiter oben

    mRecognitionRoiItem = new RecognitionRoiItem(this);
    mRecognitionRoiItem->setZValue(5); // groesser heisst weiter oben


    mViewWidget = new ViewWidget(this);
    mView = mViewWidget->view();
    //mScene->setSceneRect(0, 0, 0, 0);
    mView->setScene(mScene);
    connect(mView, SIGNAL(mouseDoubleClick()), this, SLOT(openSequence()));
    connect(mView, SIGNAL(mouseShiftDoubleClick(QPointF)), this, SLOT(addManualTrackPointOnlyVisible(QPointF))); //const QPoint &pos funktionierte nicht
    connect(mView, SIGNAL(mouseShiftControlDoubleClick(QPointF)), this, SLOT(splitTrackPerson(QPointF))); //const QPoint &pos funktionierte nicht
    connect(mView, SIGNAL(mouseControlDoubleClick(QPointF)), this, SLOT(addManualTrackPoint(QPointF))); //const QPoint &pos funktionierte nicht
    connect(mView, SIGNAL(mouseRightDoubleClick(QPointF, int)), this, SLOT(deleteTrackPoint(QPointF, int))); //const QPoint &pos funktionierte nicht
    connect(mView, SIGNAL(mouseMiddleDoubleClick(int)), this, SLOT(deleteTrackPointAll(int))); //const QPoint &pos funktionierte nicht
    connect(mView, SIGNAL(mouseShiftWheel(int)), this, SLOT(skipToFrameWheel(int)));
    //connect(mView, SIGNAL(mouseRightClick(QPointF)), this, SLOT(showContextMenu(QPointF)));

    mPlayerWidget = new Player(mAnimation, this);

    QVBoxLayout *vLayout = new QVBoxLayout;
    vLayout->setSpacing(space);
    vLayout->addWidget(mViewWidget);
    vLayout->addWidget(mPlayerWidget);

    QVBoxLayout *vControlLayout = new QVBoxLayout;
    //vControlLayout->setSpacing(space);
    vControlLayout->addWidget(mControlWidget);

    //---------------------------

    mTracker = new Tracker(this);
    mTrackerReal = new TrackerReal(this);
    mTrackerItem = new TrackerItem(this, mTracker);
    mTrackerItem->setZValue(5); // groesser heisst weiter oben

    mControlWidget->getColorPlot()->setTracker(mTracker);
#ifdef QWT
    mControlWidget->getAnalysePlot()->setTrackerReal(mTrackerReal);
#endif

    //---------------------------

    mStereoItem = new StereoItem(this);
    mStereoItem->setZValue(2);// groesser heisst weiter oben
    mStereoItem->setVisible(false);

    //---------------------------

    mColorMarkerItem = new ColorMarkerItem(this);
    mColorMarkerItem->setZValue(2);// groesser heisst weiter oben
    mColorMarkerItem->setVisible(false);
    //---------------------------

    mCodeMarkerItem = new CodeMarkerItem(this);
    mCodeMarkerItem->setZValue(2);// groesser heisst weiter oben
    mCodeMarkerItem->setVisible(false);
    //---------------------------

    mMultiColorMarkerItem = new MultiColorMarkerItem(this);
    mMultiColorMarkerItem->setZValue(2);// groesser heisst weiter oben
    mMultiColorMarkerItem->setVisible(false);

    //---------------------------

    mBackgroundItem = new BackgroundItem(this);
    mBackgroundItem->setZValue(2.2); // um so groesser um so hoeher um so eher zu sehen
    mBackgroundItem->setVisible(false);

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

    //---------------------------

    mCentralLayout = new QHBoxLayout;
    mCentralLayout->setMargin(space);
    mCentralWidget = new QFrame;
    mCentralWidget->setFrameStyle(QFrame::Sunken | QFrame::StyledPanel);
    mCentralWidget->setLayout(mCentralLayout);
    setCentralWidget(mCentralWidget);
    mCentralLayout->addLayout(vLayout);
    mCentralLayout->addLayout(vControlLayout);
    //mCentralLayout->addWidget(mControlWidget); //, 0, Qt::AlignTop
    setWindowTitle(tr("PeTrack"));

    //---------------------------
    mAutoCalib.setMainWindow(this);
    //---------------------------

    createActions();
    createMenus();
    //createToolBars();
    createStatusBar();

    mSeqFileName = QDir::currentPath(); //fuer allerersten Aufruf des Programms
    readSettings();

    saveXml(mDefaultSettings); //noch nicht noetig, da eh noch nicht fkt
//    mDefaultSettings = QString(QDir::tempPath() + "/default.pet");
//    saveProject(mDefaultSettings); // save default settings for later restore

    //#include <time.h>

    mShowFPS = 0;
    //    mLastTime = -1;

    mTrcFileName = "";

    // initialer Aufruf, damit beim reinen Laden einer Videodatei die Defaultwerte in control genommen werden zum Setzen
    setHeadSize();

    // um im background subtraction filter das hoehenbild zu beruecksichtigen
    mBackgroundFilter.setStereoContext(&mStereoContext);

    mAutoBackTrack = true; // ist der default, dann wenn in XML-Datei nicht drin steht
    mAutoTrackOptimizeColor = false; // ist der default, dann wenn in XML-Datei nicht drin steht

//    resetUI();

    setLoading(false);
}
Petrack::~Petrack()
{
    delete mImage;
    delete cw;
    // hier muessten weitere stehen insb die im konstruktor erzeugt werden
    // aber da petrack nur vernichtet wird, wenn programm beendet wird, kann man sich das auch schenken
}

void Petrack::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls())
        event->acceptProposedAction();
}
void Petrack::dropEvent(QDropEvent *event)
{
    if (event->mimeData()->hasUrls())
    {
        if (event->mimeData()->urls().first().toLocalFile().right(4) == ".pet")
            openProject(event->mimeData()->urls().first().toLocalFile());
        else if (event->mimeData()->urls().first().toLocalFile().right(4) == ".trc")
            importTracker(event->mimeData()->urls().first().toLocalFile());
        else
            openSequence(event->mimeData()->urls().first().toLocalFile());
        event->acceptProposedAction();
    }
}

//void Petrack::contextMenuEvent(QContextMenuEvent *event){
//        QMenu menu(this);
//        menu.addAction(mDelPastAct);
//        menu.addAction(mDelFutureAct);
//        menu.addAction(mDelAllRoiAct);
//        menu.exec(event->globalPos());
//}

void Petrack::updateSceneRect()
{
    double iW=0, iH=0, bS=0;

    //     // must be set explicitly, because the bounding box of the whole scene raises automatically but not gets smaller
    //     cout << "Image: "<< boundingRect().x() << " " << boundingRect().width() <<endl;
    //     cout << "Image: "<< sceneBoundingRect().x() << " " << sceneBoundingRect().width() <<endl;
    //     cout << "Image: "<< mScene->itemsBoundingRect().x() << " " << mScene->itemsBoundingRect().width() <<endl;
    //     mScene->setSceneRect(mScene->itemsBoundingRect()); // klappte nicht so gut, warum?

    if (mImage)
    {
        iW = mImage->width();
        iH = mImage->height();
        bS = getImageBorderSize();
    }

    if (mControlWidget->getCalibCoordShow())
    {
        double scale=mControlWidget->getCalibCoordScale()/10.;
        double tX=mControlWidget->getCalibCoordTransX()/10.;
        double tY=mControlWidget->getCalibCoordTransY()/10.;

        // setzen der bounding box der scene
        // Faktor 1.1 dient dazu, dass auch Zahl "1" bei coord gut in sichtbaren Bereich passt
        double xMin = (tX-1.1*scale < -bS) ? tX-1.1*scale : -bS;
        double yMin = (tY-1.1*scale < -bS) ? tY-1.1*scale : -bS;
        double xMax = (tX+1.1*scale > iW-bS) ? tX+1.1*scale : iW-bS;
        double yMax = (tY+1.1*scale > iH-bS) ? tY+1.1*scale : iH-bS;
        mScene->setSceneRect(xMin, yMin, xMax-xMin, yMax-yMin);
    }
    else
        mScene->setSceneRect(-bS, -bS, iW, iH);
}

void Petrack::openXml(QDomDocument &doc, bool openSeq)
{

    QDomElement root = doc.firstChildElement("PETRACK");
    QString seq;
    int frame = -1, sourceFrameIn = -1, sourceFrameOut = -1;
    double fps = DEFAULT_FPS;
    int onlyPeopleNr = 1;
    QString onlyPeopleNrList = "1";
    int zoom = 250, rotate = 0, hScroll = 0, vScroll = 0;
    enum Camera cam = cameraUnset;
    setLoading(true);
    for(QDomElement elem = root.firstChildElement(); !elem.isNull(); elem = elem.nextSiblingElement())
    {
        if (elem.tagName() == "MAIN")
        {
            if (elem.hasAttribute("SRC"))
            {
                seq = elem.attribute("SRC");
                QString tmpSeq = getExistingFile(seq, mProFileName);
                if (tmpSeq != "")
                    seq = tmpSeq;
//                if (openSeq && (seq != ""))
//                    openSequence(seq); // wenn leer, dann kommt abfrage hoch, welche datei; abbrechen, wenn aktuelle gewuenscht

            }
            if (elem.hasAttribute("STATUS_HEIGHT"))
            {
                if (mStatusPosRealHeight) // null kann eigentlich nicht vorkommen, da im constructor von petrack erzeugt wird
                    mStatusPosRealHeight->setValue(elem.attribute("STATUS_HEIGHT").toDouble());
            }
        }
        else if (elem.tagName() == "STEREO")
        {
            mStereoWidget->getXml(elem);
        }
        else if (elem.tagName() == "COLOR_MARKER")
        {
            mColorMarkerWidget->getXml(elem);
        }
        else if (elem.tagName() == "CODE_MARKER")
        {
            mCodeMarkerWidget->getXml(elem);
        }
        else if (elem.tagName() == "MULTI_COLOR_MARKER")
        {
            mMultiColorMarkerWidget->getXml(elem);
        }
        else if (elem.tagName() == "CONTROL")
        {
            mControlWidget->getXml(elem);
            QDomElement tmpElem = (elem.firstChildElement("TRACKING")).firstChildElement("PATH");
            if (tmpElem.hasAttribute("ONLY_PEOPLE_NR"))
                onlyPeopleNr = tmpElem.attribute("ONLY_PEOPLE_NR").toInt();
            if (tmpElem.hasAttribute("ONLY_PEOPLE_NR_LIST"))
                onlyPeopleNrList = tmpElem.attribute("ONLY_PEOPLE_NR_LIST");
        }
        else if (elem.tagName() == "PLAYER")
        {
            if (elem.hasAttribute("FRAME"))
            {
                frame = elem.attribute("FRAME").toInt();
            }
            if (elem.hasAttribute("FPS"))
            {
                //mAnimation->setFPS(elem.attribute("FPS").toDouble());
                fps = elem.attribute("FPS").toDouble();
            }
            if (elem.hasAttribute("SOURCE_FRAME_IN"))
            {
                sourceFrameIn = elem.attribute("SOURCE_FRAME_IN").toInt();
            }
            if (elem.hasAttribute("SOURCE_FRAME_OUT"))
            {
                sourceFrameOut = elem.attribute("SOURCE_FRAME_OUT").toInt();
            }
            if (elem.hasAttribute("PLAYER_SPEED_FIXED"))
            {
                mPlayerWidget->setPlayerSpeedLimited(elem.attribute("PLAYER_SPEED_FIXED").toInt());
            }
        }
        else if (elem.tagName() == "VIEW")
        {
            if (elem.hasAttribute("ANTIALIAS"))
            {
                mAntialiasAct->setChecked(elem.attribute("ANTIALIAS").toInt() == Qt::Checked);
            }
            if (elem.hasAttribute("OPENGL"))
            {
                mOpenGLAct->setChecked(elem.attribute("OPENGL").toInt() == Qt::Checked);
            }
            if (elem.hasAttribute("SAVE_TRANSFORMED"))
            {
                mCropZoomViewAct->setChecked(elem.attribute("SAVE_TRANSFORMED") == Qt::Checked);
            }
            if (elem.hasAttribute("TRANSFORMATION"))
            {
                QString matStr = elem.attribute("TRANSFORMATION");
                QTextStream in(&matStr);
                in >> zoom >> rotate >> hScroll >> vScroll;
                //elem.attribute("TRANSFORMATION").sscanf("%f %f %f %f %f %f", &m11, &m12, &m21, &m22, &dx, &dy);
                //mView->setMatrix(QMatrix(m11, m12, m21, m22, dx, dy));
                //mView->centerOn(m21, m22);
            }
            if (elem.hasAttribute("CAMERA"))
            {
                cam = (enum Camera) elem.attribute("CAMERA").toInt();
            }
            if (elem.hasAttribute("HIDE_CONTROLS"))
            {
                mHideControlsAct->setChecked(elem.attribute("HIDE_CONTROLS").toInt() == Qt::Checked);
            }
        } else if (elem.tagName() == "AUTO_TRACK")
        {
            if (elem.hasAttribute("BACK_TRACK"))
            {
                mAutoBackTrack = elem.attribute("BACK_TRACK").toInt();
            }
            if (elem.hasAttribute("OPTIMZE_COLOR"))
            {
                mAutoTrackOptimizeColor = elem.attribute("OPTIMZE_COLOR").toInt();
            }
        }
        else
            debout << "Unknown PETRACK tag " << elem.tagName() << endl;
    }
    // open koennte am schluss passieren, dann wuerde nicht erst unveraendertes bild angezeigt,
    // dafuer koennte es aber sein, dass werte zb bei fx nicht einstellbar sind!

    if (openSeq && (seq != ""))
        openSequence(seq); // wenn leer, dann kommt abfrage hoch, welche datei; abbrechen, wenn aktuelle gewuenscht

    mViewWidget->setZoomLevel(zoom);
    mViewWidget->setRotateLevel(rotate);
    mView->horizontalScrollBar()->setValue(hScroll);
    mView->verticalScrollBar()->setValue(vScroll);

    bool loaded = false;
    if (!mBackgroundFilter.getFilename().isEmpty())
    {
        if (!(loaded = mBackgroundFilter.load(mBackgroundFilter.getFilename())))
            debout << "Error: loading background file " << mBackgroundFilter.getFilename() << "!" << endl;
    }

    mPlayerWidget->setFrameInNum( sourceFrameIn == -1 ? mAnimation->getSourceInFrameNum() : sourceFrameIn );
    mPlayerWidget->setFrameOutNum( sourceFrameOut == -1 ? mAnimation->getSourceOutFrameNum() : sourceFrameOut );
    mPlayerWidget->update();

    if (frame != -1)
    {
        if (mControlWidget->filterBg->isChecked() && !loaded)// mit dem anfangs geladenen bild wurde bereits faelschlicherweise bg bestimmt
            mBackgroundFilter.reset(); // erst nach dem springen zu einem frame background bestimmen
        mPlayerWidget->skipToFrame(frame); // hier wird updateImage ausgefuehrt
    }
    else if (loaded)
        updateImage();

    // nicht schon in control, sonst loescht opensequence wieder tracker
    if (mTrcFileName != "")
    {
        // vorher loeschen aller trajektorien, da sonst nach start im ersten bild
        // mgl zwei trackpoints
        // beim haendischen importieren sind weiterhin parallele trajektorien moeglich (warnung wird ausgegeben)
        frame = 0; // default
        if ((mTracker->largestLastFrame() >= frame) && (mTracker->smallestFirstFrame() <= frame))
        {
            mTracker->clear();
            mTracker->reset();
            //mTracker->init();
        }
        importTracker(mTrcFileName);
    }

    mControlWidget->trackShowOnlyNr->setValue(onlyPeopleNr);
    mControlWidget->trackShowOnlyNrList->setText(onlyPeopleNrList);

    if (cam == cameraLeft)
        mCameraLeftViewAct->setChecked(true);
    else if (cam == cameraRight)
        mCameraRightViewAct->setChecked(true);
    setCamera();
    mPlayerWidget->setFPS(fps); // erst spaet setzen, damit Wert den des geladenen Videos ueberschreiben kann
    setLoading(false);

}

void Petrack::openProject(QString fileName, bool openSeq) // default fileName="", openSequence = true
{
    if (!QFileInfo(mProFileName).isDir()) // a project is already loaded
        if (!maybeSave())
            return;
    // if no destination file or folder is given
    if (fileName.isEmpty())
        fileName = QFileDialog::getOpenFileName(this, tr("Select project file"), QFileInfo(mProFileName).path(),
                                                tr("PeTrack project file (*.pet);;All files (*.*)"));
    // folgende zeilen nicht noetig, da dies nicht in pet-datei steht!
    //     else
    //         fileName = getExistingFile(fileName); // ueberprueft die ;-getrennten Dateinamen auf existenz und gibt ersten zurueck - interessant fuer relativen und absoluten pfad

    if (!fileName.isEmpty())
    {
        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly))
        {
            QMessageBox::critical(this, tr("PeTrack"), tr("Cannot open %1:\n%2.").arg(fileName).arg(file.errorString()));
            return;
        }
        resetSettings();
        QDomDocument doc("PETRACK"); // eigentlich Pfad zu Beschreibungsdatei fuer Dateiaufbau
        if (!doc.setContent(&file))
        {
            QMessageBox::critical(this, tr("PeTrack"), tr("Cannot read content from %1.").arg(fileName));
            file.close();
            return;
        }

//        resetUI();
        debout << "open " << fileName << endl;
        file.close();
        mProFileName = fileName;

        QDomElement root = doc.firstChildElement("PETRACK");
        if (root.hasAttribute("VERSION"))
            if (root.attribute("VERSION") != VERSION)
            {
                QMessageBox::warning(this, tr("PeTrack"), tr("Reading %1:\nDifferent version numbers %2 (application) and %3 (file) may cause problems.").arg(fileName).arg(VERSION).arg(root.attribute("VERSION")));
                //tr("Cannot read content from %1\nbecause of different version numbers\n%2 (application) and %3 (file).").arg(fileName).arg(VERSION).arg(root.attribute("VERSION")));
                //return;
            }
        openXml(doc, openSeq);
        updateWindowTitle();
    }
}

void Petrack::saveXml(QDomDocument &doc)
{
    QDomElement elem;

    QDomElement root = doc.createElement("PETRACK");
    root.setAttribute("VERSION", VERSION);
    doc.appendChild(root);

    // main settings (window size, status hight)
    elem = doc.createElement("MAIN");
    QString seq = "";
    if (mImage)
        seq = getFileList(mSeqFileName, mProFileName);

    elem.setAttribute("SRC", seq);
    elem.setAttribute("STATUS_HEIGHT", mStatusPosRealHeight->value());
    root.appendChild(elem);

    // control settings (right control widget)
    elem = doc.createElement("CONTROL");
    mControlWidget->setXml(elem);
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
    //         QMatrix mat = mView->matrix();
    //         QPointF center = (mView->sceneRect()).center();
    // kein rect oder so gefunden, was den sichtbaren bereich in viewport angibt
    //        elem.setAttribute("TRANSFORMATION", QString("%1 %2 %3 %4 %5 %6").arg(mat.m11()).arg(mat.m12()).arg(mat.m21()).arg(mat.m22()).arg(mat.dx()).arg(mat.dy()));
    elem.setAttribute("TRANSFORMATION", QString("%1 %2 %3 %4").arg(mViewWidget->getZoomLevel()).arg(mViewWidget->getRotateLevel()).arg(mView->horizontalScrollBar()->value()).arg(mView->verticalScrollBar()->value()));
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

// rueckgabewert zeigt an, ob gesichert werden konnte
bool Petrack::saveSameProject()
{
    return saveProject(mProFileName);
}

// rueckgabewert zeigt an, ob gesichert werden konnte
bool Petrack::saveProject(QString fileName) // default fileName=""
{
    // if no destination file or folder is given
    if (fileName.isEmpty() || QFileInfo(mProFileName).isDir())
    {
        //QFileInfo(mProFileName).path()
        fileName = QFileDialog::getSaveFileName(this, tr("Select project file"), mProFileName,
                                                tr("PeTrack project file (*.pet);;All files (*.*)"));
    }

    if (!fileName.isEmpty())
    {
        QDomDocument doc("PETRACK"); // eigentlich Pfad zu Beschreibungsdatei fuer Dateiaufbau
        saveXml(doc);

        // file output
        QByteArray byteArray;
        QXmlStreamWriter xmlStream (&byteArray);
        xmlStream.setAutoFormatting(true);
        xmlStream.setAutoFormattingIndent(4);

        xmlStream.writeStartDocument();
        xmlStream.writeDTD("<!DOCTYPE PETRACK>");

        QDomElement element = doc.documentElement();
        writeXmlElement(xmlStream, element);

        xmlStream.writeEndDocument();

        QFile file(fileName);
        if (!file.open(QFile::WriteOnly | QFile::Truncate | QFile::Text))
        {
            QMessageBox::critical(this, tr("PeTrack"), tr("Cannot save %1:\n%2.").arg(fileName).arg(file.errorString()));
            file.close();
            return false;
        }
        file.write(byteArray);
        file.close(); // also flushes the file

        statusBar()->showMessage(tr("Saved project to %1.").arg(fileName), 5000);
        debout << "save project to " << fileName << endl;

        mProFileName = fileName;
        updateWindowTitle();
        return true;
    }
    else
        return false;
}

void Petrack::writeXmlElement(QXmlStreamWriter& xmlStream, QDomElement element)
{
    xmlStream.writeStartElement(element.tagName());

    QVector<QString> attribute_names;
    const QDomNamedNodeMap attributes = element.attributes();
    for(int i = 0; i < attributes.size(); ++i){
        attribute_names.push_back(attributes.item(i).toAttr().name());
    }

    //TODO: check if sorting of elements fits our needs
    std::stable_sort(attribute_names.begin(), attribute_names.end()); // for a canonical XML

    // Wants this macro instead of range-based for loop
    foreach(QString name, attribute_names){
        QDomAttr attr = element.attributeNode(name);
        xmlStream.writeAttribute(attr.name(), attr.value());
    }

    // order of child nodes is defined at creation
    if(element.hasChildNodes()){
        const QDomNodeList children = element.childNodes();
        for(int i = 0; i < children.size(); ++i){
            writeXmlElement(xmlStream, children.at(i).toElement());
        }
    }

    xmlStream.writeEndElement();
}

void Petrack::openCameraLiveStream(int camID)
{
    if (camID == -1)
    {
        // if more than one camera connected show to choose
        //camID = selectedID;
        debout << "No camera ID delivered: Set CameraID to 0 (default Camera)" << endl;
        camID = 0; // default

    }
    if (!mAnimation->openCameraStream(camID))
    {
        QMessageBox::critical(this, tr("PeTrack"), tr("Cannot start Camera Livestream."));
        return;
    }
    mSeqFileName = "camera live stream";
    debout << "open " << mSeqFileName << " (" << mAnimation->getNumFrames() << " frames; " << mAnimation->getFPS() << " fps; " << mAnimation->getSize().width() << "x" << mAnimation->getSize().height() << " pixel)" << endl; //size
    updateSequence();
    updateWindowTitle();
    mPlayerWidget->setFPS(mAnimation->getFPS());
    if (mOpenGLAct->isChecked())
        mLogoItem->fadeOut(1);
    else
        mLogoItem->fadeOut(50);
    updateCoord();

    mPlayerWidget->play(PlayerState::FORWARD);
}

void Petrack::openSequence(QString fileName) // default fileName = ""
{
    if (fileName.isEmpty())
        fileName = QFileDialog::getOpenFileName(this, tr("Open video or image sequence"), QFileInfo(mSeqFileName).path(),
                                                tr("All supported types (*.avi *.mpg *.mts *.m2t *.m2ts *.wmv *.mp4 *.mov *.mxf *.bmp *.dib *.jpeg *.jpg *.jpe *.png *.pbm *.pgm *.ppm *.sr *.ras *.tiff *.tif *.exr *.jp2);;Video (*.avi *.mpg *.mts *.m2ts *.m2t *.wmv *.mov *.mp4 *.mxf);;Images (*.bmp *.dib *.jpeg *.jpg *.jpe *.png *.pbm *.pgm *.ppm *.sr *.ras *.tiff *.tif *.exr *.jp2);;Windows bitmaps (*.bmp *.dib);;JPEG (*.jpeg *.jpg *.jpe);;Portable network graphics (*.png);;Portable image format (*.pbm *.pgm *.ppm);;Sun rasters (*.sr *.ras);;TIFF (*.tiff *.tif);;OpenEXR HDR (*.exr);;JPEG 2000 (*.jp2);;All files (*.*)"));
    // in control
    //     else
    //         fileName = getExistingFile(fileName); // ueberprueft die ;-getrennten Dateinamen auf existenz und gibt ersten zurueck - interessant fuer relativen und absoluten pfad
    if (!fileName.isEmpty())
    {
        if (!mAnimation->openAnimation(fileName))
        {
            QMessageBox::critical(this, tr("PeTrack"), tr("Cannot load %1.").arg(fileName));
            return;
        }

        //updateSourceInOutFrames();
        mCameraGroupView->setEnabled(mAnimation->isStereoVideo());
        mCameraMenu->setEnabled(mAnimation->isStereoVideo());

#ifdef STEREO
        if (mAnimation->isStereoVideo())
        {
            if (mStereoContext)
                delete mStereoContext;
            mStereoContext = new pet::StereoContext(this);
        }

        bool lastIsStereoVideo = mAnimation->isStereoVideo();
        if (mCalibFilter == NULL || (mAnimation->isStereoVideo() != lastIsStereoVideo))
        {
            bool lastCalibFilterEnabled = false;
            if (mCalibFilter != NULL)
            {
                lastCalibFilterEnabled = mCalibFilter->getEnabled();
                delete mCalibFilter;
            }
            if (mAnimation->isStereoVideo())
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
        debout << "open " << mSeqFileName << " (" << mAnimation->getNumFrames() << " frames; " << mAnimation->getFPS() << " fps; " << mAnimation->getSize().width() << "x" << mAnimation->getSize().height() << " pixel)" << endl; //size
        updateSequence();
        updateWindowTitle();
        mPlayerWidget->setFPS(mAnimation->getFPS());
        if (mOpenGLAct->isChecked())
            mLogoItem->fadeOut(1);
        else
            mLogoItem->fadeOut(50);
        updateCoord();
    }
}

void Petrack::updateWindowTitle()
{
    QString title;
    QSize size = mAnimation->getSize();

    if (QFileInfo(mProFileName).isDir())
        title = tr("PeTrack (v") + VERSION + tr("): ");
    else
    {
        title = tr("PeTrack (v") + VERSION + tr("): ") + QFileInfo(mProFileName).fileName();
        if (mAnimation->isVideo() || mAnimation->isImageSequence())
            title +=  "; ";
    }
    if (mAnimation->isVideo())
        title += "sequence: " + mAnimation->getCurrentFileName() + tr(" (%1").arg(mAnimation->getNumFrames()) + tr(" frames; %1x%2").arg(size.width()).arg(size.height()) + " pixel)"; //size getFileBase()+".avi"
      //title += "sequence: " + mAnimation->getCurrentFileName() + tr(" (%1").arg(mAnimation->getNumFrames()) + tr(" frames; %1").arg(mAnimation->getFPS()) + tr(" fps; %1x%2").arg(size.width()).arg(size.height()) + " pixel)"; //size getFileBase()+".avi"
    else if (mAnimation->isImageSequence())
        title += "sequence: " + mAnimation->getCurrentFileName() + tr(" ... (%1").arg(mAnimation->getNumFrames()) + tr(" frames; %1x%2").arg(size.width()).arg(size.height()) + " pixel)"; //size getFileBase()
      //title += "sequence: " + mAnimation->getCurrentFileName() + tr(" ... (%1").arg(mAnimation->getNumFrames()) + tr(" frames; %1").arg(mAnimation->getFPS()) + tr(" fps; %1x%2").arg(size.width()).arg(size.height()) + " pixel)"; //size getFileBase()
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
// kann die geladene einzelbildsequnz oder video in video oder einzelbildsequenz speichern
// saveSequ zeigt an, ob einzelbildsequenz oder video gespeichert werden soll,
// wenn dest nicht angegeben ist, ansonsten wird dateiendung von dest als hinweis auf format genommen
void Petrack::saveSequence(bool saveVideo, bool saveView, QString dest) // default saveView= false, dest=""
{
    static QString lastDir;
//    bool autoSave = false;

    // if no destination file or folder is given
    if (dest.isEmpty())
    {
        if (lastDir.isEmpty() && !mSeqFileName.isEmpty())
            lastDir = QFileInfo(mSeqFileName).path();

        if (saveVideo)
        {
            dest = QFileDialog::getSaveFileName(this, tr("Select video file"), lastDir,
                                                tr("Video (*.avi);;All files (*.*)"));  //? *.mpg *.mpeg
        }
        else
        {
            if (saveView)
                dest = QFileDialog::getExistingDirectory(this, tr("Select directory to save view sequence"),
                                                         lastDir, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
            else
                dest = QFileDialog::getExistingDirectory(this, tr("Select directory to save image sequence"),
                                                         lastDir, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
        }
    }
    else // uebergabe von saveVideo spielt keine roll mehr, sondern wird hier analysiert anhand von Dateiendung
    {
//        autoSave = true; // command line option
        if (dest.right(4) == ".avi")
            saveVideo = true;
        else
            saveVideo = false;
    }

    if (!dest.isEmpty() && mImage)
    {
        int rest = mAnimation->getNumFrames()-1;
        int numLength = 1;
        int memPos = mPlayerWidget->getPos();
        QString fileName = "";
#ifdef AVI
        AviFile aviFile;
#else
        AviFileWriter aviFile;
#endif
        //CvVideoWriter* videoWriter;
        //using namespace cv; oder cv::VideoWriter *vw;
        bool formatIsSaveAble = false;
        bool saveRet;
        QImage *viewImage = NULL; // =NULL, amit keine Warnung
        QPainter *painter = NULL; // =NULL, amit keine Warnung
        int progEnd = /*mAnimation->getNumFrames()-1*/mAnimation->getSourceOutFrameNum()-mPlayerWidget->getPos(); // nur wenn nicht an anfang gesprungen wird:-mPlayerWidget->getPos()
        Mat iplImgFilteredBGR;
        bool writeFrameRet = false;
        bool convert8To24bit = false;
        int mult;


        if (saveVideo)
        {
            //            //
            //            // Retreive the number of compressors.
            //            //
            //            int iNumCompressors = aviFile.enumerateCompressors(mIplImgFiltered->height, mIplImgFiltered->width, 8*mIplImgFiltered->nChannels, NULL, 0);
            //
            //            if(iNumCompressors)
            //            {
            //                ICINFO* picinfo = new ICINFO[iNumCompressors];
            //                COMPVARS* comp = new COMPVARS[iNumCompressors];
            //                char *c;
            //
            //                //
            //                // Retreive information about these compressors.
            //                //
            //                aviFile.enumerateCompressors(mIplImgFiltered->height, mIplImgFiltered->width, 8*mIplImgFiltered->nChannels, picinfo, iNumCompressors);
            //
            //                for (int i=0; i<iNumCompressors; i++)
            //                {
            //                    c = (char*) &(picinfo[i].fccHandler);
            //                    debout << c[0] << c[1] << c[2] << c[3] << " " << (char *) picinfo[i].szName << endl;
            //                    comp[i].cbSize = picinfo[i].dwSize;
            //                    //comp[i].lQ = 100.; //picinfo[i].;
            //                    comp[i].dwFlags = picinfo[i].dwFlags;
            //                    comp[i].fccHandler = picinfo[i].fccHandler;
            //                    comp[i].fccType = picinfo[i].fccType;
            //                    //comp[i].cbState = ;
            //                    //comp[i].lpState = ;
            //                }
            //
            //                //
            //                // Set the compressor to the first one found.
            //                //
            //                //aviFile.setCompressor(comp);
            //
            //                delete [] picinfo;
            //            }

            if (saveView)
            {
                if (mCropZoomViewAct->isChecked())
                    viewImage = new QImage(mView->viewport()->width(), mView->viewport()->height(), QImage::Format_RGB32);
                else
                    viewImage = new QImage((int) mScene->width(), (int) mScene->height(), QImage::Format_RGB32);
                painter = new QPainter();
            }
            //CvSize size;
            //size.width =  mIplImgFiltered->width;
            //size.height = mIplImgFiltered->height;
            if (convert8To24bit)
                mult = 3;
            else
                mult = 1;

            //debout << "width: " << mIplImgFiltered->width << " height: " << mIplImgFiltered->height << endl;
            //debout << "width: " << mView->viewport()->width() << " height: " << mView->viewport()->height() << endl;
            //debout << "width: " << mScene->width() << " height: " << mScene->height() << endl;
            //debout << "channels: " << mIplImgFiltered->nChannels << endl;
//            debout << "viewImage: " << viewImage->depth() << endl;

            bool ok = false;

            if( saveView )
                ok = aviFile.open(dest.toStdString().c_str(), viewImage->width(), viewImage->height(), viewImage->depth(), mAnimation->getFPS());
            else
                ok = aviFile.open(dest.toStdString().c_str(), mImg.cols, mImg.rows, mult*8*mImg.channels(), mAnimation->getFPS());
            ////vw = new cv::VideoWriter;
            ////vw->open(dest.toStdString().c_str(), CV_FOURCC_DEFAULT, mAnimation->getFPS(), size);
            //// -1 muss unter linux durch CV_FOURCC('P','I','M','1') ist mpeg1 oder CV_FOURCC('M','J','P','G') ersetzt werden oder CV_FOURCC_DEFAULT
            //// CV_FOURCC('D','I','B',' ') == 0 bedeutet rgb unkomprimiert - siehe http://opencvlibrary.sourceforge.net/VideoCodecs
            //if (autoSave)
            //    videoWriter = cvCreateVideoWriter(dest.toStdString().c_str(), CV_FOURCC_DEFAULT, mAnimation->getFPS(), size); //, int is_color=1
            //else
            //    videoWriter = cvCreateVideoWriter(dest.toStdString().c_str(), 0, mAnimation->getFPS(), size); //, int is_color=1
            //// if codec is not supported or no codec is selected
            //if (!videoWriter)
            //    return;
            if (!ok)
            {
                debout << "Error: opening AVI file: " << dest.toStdString().c_str() << endl;
                return;
            }
        }

        //         mPlayerWidget->skipToFrame(0); // koennte man auch weglassen, dann wurde von aktueller pos film geschrieben!!!

        if (!saveVideo)
        {
            if (saveView)
            {
                if (mCropZoomViewAct->isChecked())
                    viewImage = new QImage(mView->viewport()->width(), mView->viewport()->height(), QImage::Format_RGB32);
                else
                    viewImage = new QImage((int) mScene->width(), (int) mScene->height(), QImage::Format_RGB32);
                painter = new QPainter();
            }

            // test, if fileformat is supported
            if (mAnimation->isVideo())
            {
                // calculate string length of sequence number
                while ((rest/=10) > 0)
                    numLength++;
                fileName = (dest + "/" + mAnimation->getFileBase() + "%1.png").arg(mPlayerWidget->getPos(), numLength, 10, QChar('0'));
            }
            else
                fileName = dest + "/" + mAnimation->getCurrentFileName();

            if (saveView)
            {
                painter->begin(viewImage);
                if (mCropZoomViewAct->isChecked())
                    mView->render(painter);
                else
                    mScene->render(painter);
                painter->end();

                if (viewImage->save(fileName)) //, const char * format = 0 (format wird aus dateinamen geholt), int quality = -1 default normal (0..100)
                {
                    formatIsSaveAble = true;
                    mPlayerWidget->frameForward();
                }
            }
            else if (mImage->save(fileName)) //, const char * format = 0 (format wird aus dateinamen geholt), int quality = -1 default normal (0..100)
            {
                formatIsSaveAble = true;
                mPlayerWidget->frameForward();
            }
        }
        else if ((mImgFiltered.channels() == 1)  /*&& convert8To24bit*/)
        {
            Size size;
            size.width = mImgFiltered.cols;
            size.height = mImgFiltered.rows;
            iplImgFilteredBGR.create(size, CV_8UC3);
        }

        QProgressDialog progress("", "Abort save", 0, progEnd, this);
        progress.setWindowModality(Qt::WindowModal); // blocks main window

        if (saveVideo)
        {
            if (saveView)
                progress.setLabelText("Save video view...");
            else
                progress.setLabelText("Save video...");
        }else
        {
            if (saveView)
                progress.setLabelText("Save view sequence...");
            else
                progress.setLabelText("Save image sequence...");
        }


        do
        {
            progress.setValue(mPlayerWidget->getPos()-memPos); // -mempos nur, wenn nicht an den anfang gesprungen wird
            qApp->processEvents();
            if (progress.wasCanceled())
                break;

            if (saveVideo)
            {
                // video sequence
                if (saveView)
                {
                    painter->begin(viewImage);
                    if (mCropZoomViewAct->isChecked())
                        mView->render(painter);
                    else
                        mScene->render(painter);
                    painter->end();
                }
//                debout << "nChannels: " << mIplImgFiltered->nChannels << endl;
                if ((mImgFiltered.channels() == 1) /* && convert8To24bit*/)
                {
                    //cvCvtColor(mIplImgFiltered, iplImgFilteredBGR, CV_GRAY2BGR);
                    cvtColor(mImg, iplImgFilteredBGR, COLOR_GRAY2BGR);
                    if( saveView )
                        writeFrameRet = aviFile.appendFrame((const unsigned char*) viewImage->bits(), true); // 2. param besagt, ob vertikal gespiegel werden soll
                    else
                        writeFrameRet = aviFile.appendFrame((const unsigned char*) iplImgFilteredBGR.data, true); // 2. param besagt, ob vertikal gespiegel werden soll
                }
                else
                {
                    if( saveView ){
//                        debout << "." << endl;
                        writeFrameRet = aviFile.appendFrame((const unsigned char*) viewImage->bits(), true); // 2. param besagt, ob vertikal gespiegel werden soll
                    }else{
                        writeFrameRet = aviFile.appendFrame((const unsigned char*) mImg.data, true); // 2. param besagt, ob vertikal gespiegel werden soll
                    }
                }

                if (!writeFrameRet)
                {
                    progress.setValue(progEnd);
                    QMessageBox::critical(this, tr("PeTrack"), tr("Cannot save %1 maybe because of wrong file extension or unsupported codec.").arg(dest));
                    break;
                }
            }
            else
            {
                // single frame sequence
                if (saveView)
                {
                    painter->begin(viewImage);
                    if (mCropZoomViewAct->isChecked())
                        mView->render(painter);
                    else
                        mScene->render(painter);
                    painter->end();
                }
                if (mAnimation->isVideo())
                {
                    fileName = (dest + "/" + mAnimation->getFileBase() + "%1.png").arg(mPlayerWidget->getPos(), numLength, 10, QChar('0'));
                    if (saveView)
                        saveRet = viewImage->save(fileName);
                    else
                        saveRet = mImage->save(fileName);
                }
                else if (formatIsSaveAble)
                {
                    fileName = dest + "/" + mAnimation->getCurrentFileName();
                    if (saveView)
                        saveRet = viewImage->save(fileName);
                    else
                        saveRet = mImage->save(fileName);
                }
                else
                {
                    fileName = dest + "/" + QFileInfo(mAnimation->getCurrentFileName()).completeBaseName() + ".png";
                    if (saveView)
                        saveRet = viewImage->save(fileName);
                    else
                        saveRet = mImage->save(fileName, "PNG"); //, int quality = -1 default normal (0..100)
                }
                if (!saveRet)
                {
                    progress.setValue(progEnd);
                    QMessageBox::critical(this, tr("PeTrack"), tr("Cannot save %1.").arg(fileName));
                    break;
                }
            }
        }
        while (mPlayerWidget->frameForward());

        if (!saveVideo && saveView)
        {
            delete viewImage;
            delete painter;
        }

        // bei abbruch koennen es auch mPlayerWidget->getPos() frames sein, die bisher geschrieben wurden
        //-memPos nur, wenn nicht an den anfang gesprungen wird
        debout << "wrote " << mPlayerWidget->getPos()+1-memPos << " of " << mAnimation->getNumFrames() << " frames." << endl;
        progress.setValue(progEnd);

        if (saveVideo)
        {
            aviFile.close();
            //cvReleaseVideoWriter(&videoWriter);
//            if (iplImgFilteredBGR)
//                cvReleaseImage(&iplImgFilteredBGR);
        }

        mPlayerWidget->skipToFrame(memPos);
        lastDir = dest;
    }
}

void Petrack::saveView(QString dest) //default = ""
{
    static QString lastFile;

    if(mImage)
    {
        // if no destination file or folder is given
        if (dest.isEmpty())
        {
            if (lastFile.isEmpty() && !mSeqFileName.isEmpty())
                lastFile = QFileInfo(mSeqFileName).path();

            // alle unetrstuetzen fileformate erhaelt man mit
            // QImageReader::supportedImageFormats() and QImageWriter::supportedImageFormats()
            // gif muss nicht dabei sein, dazu muss qt mit -qt-gif uebersetzt worden sein
            dest = QFileDialog::getSaveFileName(this, tr("Select image file"), lastFile,
                                                tr("PDF (*.pdf);;Postscript (*.ps *.eps);;Windows bitmaps (*.bmp);;JPEG (*.jpeg *.jpg);;Portable network graphics (*.png);;Portable image format (*.pbm *.pgm *.ppm);;X11 Bitmap or Pixmap (*.xbm *.xpm);;Pixel Images (*.bmp *.jpeg *.jpg *.png *.pbm *.pgm *.ppm *.xbm *.xpm);;All supported types (*pdf *ps *.eps *.bmp *.jpeg *.jpg *.png *.pbm *.pgm *.ppm *.xbm *.xpm);;All files (*.*)"));
        }

        if (!dest.isEmpty())
        {
            if (dest.right(4) == ".pdf" || dest.right(3) == ".ps" || dest.right(4) == ".eps")
            {
                QPrinter printer(QPrinter::ScreenResolution); // HighResolution?
                printer.setColorMode(QPrinter::Color);
                //printer.setPageSize(QPrinter::A4);
                //printer.setOutputFormat(QPrinter::PdfFormat); //QPrinter::PostScriptFormat automatisch durch dateiendung
                printer.setOutputFileName(dest);
                QPainter painter(&printer);
                if (mCropZoomViewAct->isChecked())
                    mView->render(&painter);
                else
                    mScene->render(&painter);
            }
            else
            {
                // schwarzer rand links und unten?!
                QImage *img;
                if (mCropZoomViewAct->isChecked())
                    img = new QImage(mView->viewport()->width(), mView->viewport()->height(), QImage::Format_RGB32);
                else
                    img = new QImage((int) mScene->width(), (int) mScene->height(), QImage::Format_RGB32);
                QPainter painter(img);
                if (mCropZoomViewAct->isChecked())
                    mView->render(&painter);
                else
                    mScene->render(&painter);
                painter.end();
                if (!img->save(dest)) //, "PNG"
                    QMessageBox::critical(this, tr("PeTrack"), tr("Cannot save %1 maybe because of wrong file extension.").arg(dest));
                delete img;
            }
            lastFile = dest;
        }
    }
}

void Petrack::saveImage(QString dest) //default = ""
{
    static QString lastFile;

    if(mImage)
    {
        // if no destination file or folder is given
        if (dest.isEmpty())
        {
            if (lastFile.isEmpty() && !mSeqFileName.isEmpty())
                lastFile = QFileInfo(mSeqFileName).path();


            // alle unetrstuetzen fileformate erhaelt man mit
            // QImageReader::supportedImageFormats() and QImageWriter::supportedImageFormats()
            // gif muss nict dabei sein, dazu muss qt mit -qt-gif uebesetz worden sein
            dest = QFileDialog::getSaveFileName(this, tr("Select image file"), lastFile,
                                                tr("PDF (*.pdf);;Postscript (*.ps *.eps);;Windows bitmaps (*.bmp);;JPEG (*.jpeg *.jpg);;Portable network graphics (*.png);;Portable image format (*.pbm *.pgm *.ppm);;X11 Bitmap or Pixmap (*.xbm *.xpm);;Pixel Images (*.bmp *.jpeg *.jpg *.png *.pbm *.pgm *.ppm *.xbm *.xpm);;All supported types (*pdf *ps *.eps *.bmp *.jpeg *.jpg *.png *.pbm *.pgm *.ppm *.xbm *.xpm);;All files (*.*)"));
        }

        if (!dest.isEmpty())
        {
            if (dest.right(4) == ".pdf" || dest.right(3) == ".ps" || dest.right(4) == ".eps")
            {

                QPrinter printer(QPrinter::ScreenResolution); // HighResolution?
                printer.setColorMode(QPrinter::Color);
                //printer.setPageSize(QPrinter::A4);
                //printer.setOutputFormat(QPrinter::PdfFormat); //QPrinter::PostScriptFormat automatisch durch dateiendung
                printer.setOutputFileName(dest);
                QPainter painter(&printer);
                QRect rect = painter.viewport();
                QSize size = mImage->size();
                size.scale(rect.size(), Qt::KeepAspectRatio);
                painter.setViewport(rect.x(), rect.y(), size.width(), size.height());
                painter.setWindow(mImage->rect());
                painter.drawImage(0, 0, *mImage);
            }
            else
            {
                if (!mImage->save(dest)) //, "PNG"
                    QMessageBox::critical(this, tr("PeTrack"), tr("Cannot save %1 maybe because of wrong file extension.").arg(dest));
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
        QPrinter printer(QPrinter::ScreenResolution); //ScreenResolution, HighResolution// liefert zu hause: QWin32PrintEngine::initialize: GetPrinter failed ()
        printer.setPageSize(QPageSize{QPageSize::PageSizeId::A4});
        QPrintDialog dialog(&printer, this);
        //printer.setOutputFormat(QPrinter::PostScriptFormat);
        if (dialog.exec()) {
            QPainter painter(&printer);
            mView->render(&painter);

            //             QRect rect = painter.viewport();
            //             QSize size = imageWidget->pixmap()->size();
            //             size.scale(rect.size(), Qt::KeepAspectRatio);
            //             painter.setViewport(rect.x(), rect.y(), size.width(), size.height());
            //             painter.setWindow(imageWidget->pixmap()->rect());
            //             painter.drawPixmap(0, 0, *imageWidget->pixmap());
            // // print the upper half of the viewport into the lower.
            //  // half of the page.
            //  QRect viewport = view.viewport()->rect();
            //  view.render(&painter,
            //              QRectF(0, printer.height() / 2,
            //                     printer.width(), printer.height() / 2),
            //              viewport.adjusted(0, 0, 0, -viewport.height() / 2));
        }
    }
    else
        QMessageBox::critical(this, tr("PeTrack"), tr("Nothing to print!"));
}

void Petrack::resetSettings()
{
    mAnimation->reset();
    openXml(mDefaultSettings);
//    resetUI();
}

void Petrack::about()
{
    //People/Pedestrian/Person tracking
    QMessageBox::about(this, tr("About PeTrack"),
                       tr("<p><b>PeTrack</b> - Pedestrian tracking<br>" //&nbsp;&nbsp;&nbsp;&nbsp;
                          "Version " VERSION "<br>"
                          "(Build " COMPILE_DATE " " COMPILE_TIME ")</p>"
                          "<p>by Maik Boltes, Daniel Salden<br>"
                          "&copy; Forschungszentrum Juelich GmbH</p>"));
    //©  &copy; &#169; (c)
    // folgender befehl wird erst nach About-Fenster ausgefuehrt und ersetzt Icon im Hauptfenster
    //QApplication::activeWindow()->setWindowIcon(QIcon(":/icon"));
    // siehe auch Petrack::Petrack()
}


void Petrack::commandLineOptions()
{
    //QTextBrowser(this);
    //static QMessageBox* mb = NULL;
    //if (mb == NULL)
    //{
    QMessageBox* mb = new QMessageBox(this); //QMessageBox mb(this);
    mb->setAttribute(Qt::WA_DeleteOnClose);
    mb->setWindowTitle(tr("Command line options"));
    //mb->setWindowModality(Qt::NonModal);
    mb->setModal(false); // if you want it non-modal
    mb->setText(commandLineOptionsString);
    mb->setStandardButtons(QMessageBox::Ok);
    //}

    //"<style>spantab {padding-left: 4em; margin-left:5em}</style><span class=\"tab\"></span>"
    //"<p>Only to convert a video <kbd>video.avi</kbd> with settings stored in a petrack project file <kbd>project.pet</kbd> to an image sequence to be stored in the directory <kbd>folder</kbd> call:<br>"
    //"<code>petrack.exe -project project.pet </code><br>"
    //"&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<code>-sequence video.avi </code><br>"
    //"&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<code>-autoSave folder</code></p>"
    //To generate trajectories from a single image sequence frame000.jpg to frame345.jpg
    //with settings stored in project.pet, export tracker file tracker_file and exit
    //with saving the project to project.pet again:
    //petrack.exe -project project.pet -sequence frame000.jpg -autoTrack tracker_file -autoSave project.pet
    //For more information (especially key bindings) see http://www.fz-juelich.de/jsc/petrack/.
    //Normal usage is interactive usage.



    mb->show(); //mb->exec(); //mb->open();

    //    QMessageBox::about(this, tr("Command line options"), tr(
//"Beside the space bar all bindings only affect inside the image.<p>"
//"Space bar<br>blavbla"));

//Usage: petrack [-help|-?] [[-project] project.pet] [-sequence image_sequence_or_video] [-autoSave|-autosave image_folder_or_video|project.pet|tracker_file] [-autoTrack|-autotrack tracker_file] [-autoPlay|-autoplay tracker_file]

//Usage:
//    ------
//    petrack [-help|-?] [[-project] project.pet] [-sequence image_sequence_or_video] [-autoSave|-autosave image_folder_or_video|project.pet|tracker_file] [-autoTrack|-autotrack tracker_file] [-autoPlay|-autoplay tracker_file]

//    -help|-? shows help information for command line options
//    -project optional option to set project file to project.pet; otherwise the argument without option flag is used as project file
//    -sequence loads image sequence or video image_sequence_or_video; option overwrites SRC attribute in project file
//    -autoSave|-autosave if the argument ends with pet, a project file will be written to proj.pet at the end;
//        if the argument ends with txt, dat or trav, the trajectories will be written in a format according to the suffix of tracker_file;
//        otherwise image_folder_or_video is the folder to store the image sequence or a name of a video file for the direct export;
//        in all cases PeTrack ends after finishing the work
//    -autoTrack|-autotrack calculates automatically the trajectories of marked pedestrians and stores the result to tracker_file
//    -autoPlay|-autoplay plays the video or image sequence and stores the trajectories to tracker_file

}

void Petrack::keyBindings()
{
    QMessageBox* mb = new QMessageBox(this);
    mb->setAttribute(Qt::WA_DeleteOnClose);
    mb->setWindowTitle(tr("Command line options"));
    mb->setModal(false); // if you want it non-modal
    mb->setText(tr(
"<p>Beside the space bar all bindings only affect inside the image.</p>"
"<dl><dt><kbd>Space bar</kbd></dt><dd>toggles between pause and last play direction</dd>"
"<dt><kbd>Mouse scroll wheel</kbd></dt><dd>zooms in and out to or from the pixel of the image at the position of the mouse pointer</dd>"
"<dt><kbd>Shift + mouse scroll wheel</kbd></dt><dd>plays forwards or backwards frame by frame</dd>"
"<dt><kbd>Holding left mouse button</kbd></dt><dd>moves image</dd>"
"<dt><kbd>Arrows up/down</kbd></dt><dd>zoom in/out</dd>"
"<dt><kbd>Arrows left/right</kbd></dt><dd>frame back/forward</dd>"
"<dt><kbd>Double-click left mouse button</kbd></dt><dd>opens video or image sequence</dd>"
"<dt><kbd>Ctrl + double-click left mouse button</kbd></dt><dd>inserts new or moves near trackpoint</dd>"
"<dt><kbd>Ctrl + Shift + double-click left mouse button</kbd></dt><dd>splits near trackpoint before actual frame</dd>"
"<dt><kbd>Ctrl + double-click right mouse button</kbd></dt><dd>deletes a trajectory of a near trackpoint</dd>"
"<dt><kbd>Shift + double-click right mouse button</kbd></dt><dd>deletes the past part of a trajectory of a near trackpoint</dd>"
"<dt><kbd>Alt + double-click right mouse button</kbd></dt><dd>deletes the future part of a trajectory of a near trackpoint</dd>"
"<dt><kbd>Ctrl + double-click middle mouse button</kbd></dt><dd>deletes all trajectories</dd>"
"<dt><kbd>Shift + double-click middle mouse button</kbd></dt><dd>deletes the past part of all trajectories</dd>"
"<dt><kbd>Alt + double-click middle mouse button</kbd></dt><dd>deletes the future part of all trajectories</dd>"
"<dt><kbd>Shift + t</kbd></dt><dd>toggles tracking online calculation</dd>"
"<dt><kbd>Shift + double-click left mouse button</kbd></dt><dd>inserts new or moves near trackpoint and enables showing only the modified trajectory</dd></dl>"
"<p>Further key bindings you will find next to the entries of the menus.</p>"
                    ));

//"Beside the space bar all bindings only affect inside the image.<p>"
//"Space bar<br>"
//"    toggles between pause and last play direction<br>"
//"Mouse scroll wheel<br>"
//"    zooms in and out to or from the pixel of the image at the position of the mouse pointer<br>"
//"Shift + mouse scroll wheel<br>"
//"    plays forwards or backwards<br>"
//"Holding left mouse button<br>"
//"    moves image<br>"
//"Arrows<br>"
//"    moves image<br>"
//"Double-click left mouse button<br>"
//"    opens video or image sequence<br>"
//"Ctrl + double-click left mouse button<br>"
//"    inserts new or moves near trackpoint<br>"
//"Ctrl + double-click right mouse button<br>"
//"    deletes a trajectory of a near trackpoint<br>"
//"Shift + double-click right mouse button<br>"
//"    deletes the past part of a trajectory of a near trackpoint<br>"
//"Alt + double-click right mouse button<br>"
//"    deletes the future part of a trajectory of a near trackpoint<br>"
//"Ctrl + double-click middle mouse button<br>"
//"    deletes all trajectories<br>"
//"Shift + double-click middle mouse button<br>"
//"    deletes the past part of all trajectories<br>"
//"Alt + double-click middle mouse button<br>"
//"    deletes the future part of all trajectories<br>"
//"Shift + t<br>"
//"    toggles tracking online calculation<br>"
//"Shift + double-click left mouse button<br>"
//"    inserts new or moves near trackpoint and enables showing only the modified trajectory<p>"
//"Further key bindings you will find next to the entries of the menus."

    mb->setStandardButtons(QMessageBox::Ok);
    mb->show();
    //QMessageBox::about(NULL, tr("Key bindings"), tr(
}

void Petrack::onlineHelp()
{
    static QUrl url("http://www.fz-juelich.de/jsc/petrack/");
    if (!(QDesktopServices::openUrl(url)))
        QMessageBox::critical(this, tr("PeTrack"), tr("Cannot open external browser<br>with url ") + url.toString() + "!");
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
    bool ok;
    QFont font = QFontDialog::getFont(&ok, this->font(), this);
    if (ok) {
        this->setFont(font); // font is set to the font the user selected
    } else {
        // the user canceled the dialog; font is set to the initial
        // value, in this case Times, 12.
    }
    //static QFont font("Courier", 10, QFont::Normal); //Times Helvetica, Normal Bold // QFont("Times", 12)
    //this->setFont(font);
}

void Petrack::showHideControlWidget()
{
    // show | hide Control
    mViewWidget->hideControls(mHideControlsAct->isChecked());
}

void Petrack::setCamera()
{
#ifndef STEREO_DISABLED
    if (mAnimation)
    {
        if (mCameraLeftViewAct->isChecked())
        {
            if ((mAnimation->getCamera()) != cameraLeft)
                mAnimation->setCamera(cameraLeft); // war: hier wird direkt bei Umstellung neu gelesen
            else
                return;
        }
        else if (mCameraRightViewAct->isChecked())
        {
            if ((mAnimation->getCamera()) != cameraRight)
                mAnimation->setCamera(cameraRight); // war: hier wird direkt bei Umstellung neu gelesen
            else
                return;
        }
        else // kann eigentlich nicht vorkommen
        {
            mAnimation->setCamera(cameraUnset);
            return;
        }
        updateImage(mAnimation->getFrameAtIndex(mAnimation->getCurrentFrameNum())); // wird nur aufgerufen, wenn left / right sich geaendert hat
        //mPlayerWidget->updateImage();
        //mPlayerWidget->skipToFrame(mPlayerWidget->getPos()); // machtpasue!!
        //updateImage(true); // nur dies aufrufen, wenn nicht links rechts gleichzeitig gehalten wird
    }

#endif
}

void Petrack::createActions()
{
    mOpenSeqAct = new QAction(tr("&Open Sequence"), this);
    mOpenSeqAct->setShortcut(tr("Ctrl+Shift+O"));
    connect(mOpenSeqAct, SIGNAL(triggered()), this, SLOT(openSequence()));

    mOpenCameraAct = new QAction(tr("Open Camera Stream"), this);
    //mOpenCameraAct->setShortcut(tr("Ctrl+C")); // because of some reason it is sometimes fired with Ctrl+LeftMouseButton ==> so disabled (it's also not really needed)
    connect(mOpenCameraAct, SIGNAL(triggered()), this, SLOT(openCameraLiveStream()));

    mSaveSeqVidAct = new QAction(tr("Save Video"), this);
    //mSaveSeqVidAct->setShortcut(tr("Ctrl+E"));
    mSaveSeqVidAct->setEnabled(false);
    connect(mSaveSeqVidAct, SIGNAL(triggered()), this, SLOT(saveVideo()));

    mSaveSeqVidViewAct = new QAction(tr("Save Video View"), this);
    mSaveSeqVidViewAct->setShortcut(tr("Ctrl+E"));
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
    connect(mSavePrAct, SIGNAL(triggered()), this, SLOT(saveProject()));

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
    connect(mHideControlsAct, SIGNAL(changed()),this, SLOT(showHideControlWidget()));

    mCropZoomViewAct = new QAction(tr("&Transform while saving"), this); //Crop and zoom while saving
    mCropZoomViewAct->setCheckable(true);

    mOpenGLAct = new QAction(tr("Open&GL"), this);
    mOpenGLAct->setShortcut(tr("Ctrl+G"));
    mOpenGLAct->setCheckable(true);
    connect(mOpenGLAct, SIGNAL(triggered()), this, SLOT(opengl()));

    mResetAct = new QAction(tr("&Reset"), this);
    mResetAct->setShortcut(tr("Ctrl+R"));
    connect(mResetAct, SIGNAL(triggered()), this, SLOT(reset()));

    mFitViewAct = new QAction(tr("Fit in window"), this); // Resize to window; fit in view; show all; in fenster einpassen
    mFitViewAct->setShortcut(tr("Ctrl+0"));
    connect(mFitViewAct, SIGNAL(triggered()), this, SLOT(fitInView()));

    mFitROIAct = new QAction(tr("Fit in region of interest"), this); // Resize ROI to window; fit in view;
    mFitROIAct->setShortcut(tr("Ctrl+1"));
    connect(mFitROIAct, SIGNAL(triggered()), this, SLOT(fitInROI()));

    mCameraGroupView = new QActionGroup(this);
    //mCameraGroupView->addAction(mCameraLeftViewAct);
    //mCameraGroupView->addAction(mCameraRightViewAct);
    mCameraLeftViewAct = new QAction(tr("&Left"), mCameraGroupView);
    mCameraLeftViewAct->setShortcut(tr("Ctrl++Shift+L"));
    mCameraLeftViewAct->setCheckable(true);
    connect(mCameraLeftViewAct, SIGNAL(triggered()), this, SLOT(setCamera()));
    mCameraRightViewAct = new QAction(tr("&Right"), mCameraGroupView);
    mCameraRightViewAct->setShortcut(tr("Ctrl++Shift+R"));
    mCameraRightViewAct->setCheckable(true);
    connect(mCameraRightViewAct, SIGNAL(triggered()), this, SLOT(setCamera()));
    mCameraRightViewAct->setChecked(true); // right wird als default genommen, da reference image in triclops auch right ist // erste trj wurden mit left gerechnet

    mLimitPlaybackSpeed = new QAction(tr("&Limit playback speed"));
    // Not checkable like Fix since this is also controlled through clicking on FPS and syncing currently would be bothersome
    connect(mLimitPlaybackSpeed, &QAction::triggered, mPlayerWidget, [&](){mPlayerWidget->setPlayerSpeedLimited(!mPlayerWidget->getPlayerSpeedLimited());});
    mFixPlaybackSpeed = new QAction(tr("&Fix playback speed"));
    mFixPlaybackSpeed->setCheckable(true);
    connect(mFixPlaybackSpeed, &QAction::toggled, mPlayerWidget, &Player::setPlayerSpeedFixed);
    mSetToRealtime = new QAction(tr("&Realtime"));
    connect(mSetToRealtime, &QAction::triggered, mPlayerWidget, [&](){mPlayerWidget->setSpeedRelativeToRealtime(1.0);});
    mSetTo2p00 = new QAction(tr("&x2"));
    connect(mSetTo2p00, &QAction::triggered, mPlayerWidget, [&](){mPlayerWidget->setSpeedRelativeToRealtime(2.0);});
    mSetTo1p75 = new QAction(tr("&x1.75"));
    connect(mSetTo1p75, &QAction::triggered, mPlayerWidget, [&](){mPlayerWidget->setSpeedRelativeToRealtime(1.75);});
    mSetTo1p50 = new QAction(tr("&x1.5"));
    connect(mSetTo1p50, &QAction::triggered, mPlayerWidget, [&](){mPlayerWidget->setSpeedRelativeToRealtime(1.5);});
    mSetTo1p25 = new QAction(tr("&x1.25"));
    connect(mSetTo1p25, &QAction::triggered, mPlayerWidget, [&](){mPlayerWidget->setSpeedRelativeToRealtime(1.25);});
    mSetTo0p75 = new QAction(tr("&x0.75"));
    connect(mSetTo0p75, &QAction::triggered, mPlayerWidget, [&](){mPlayerWidget->setSpeedRelativeToRealtime(0.75);});
    mSetTo0p50 = new QAction(tr("&x0.5"));
    connect(mSetTo0p50, &QAction::triggered, mPlayerWidget, [&](){mPlayerWidget->setSpeedRelativeToRealtime(0.5);});
    mSetTo0p25 = new QAction(tr("&x0.25"));
    connect(mSetTo0p25, &QAction::triggered, mPlayerWidget, [&](){mPlayerWidget->setSpeedRelativeToRealtime(0.25);});

    mPlayerLooping = new QAction(tr("&Loop"));
    mPlayerLooping->setCheckable(true);
    connect(mPlayerLooping, &QAction::triggered, mPlayerWidget, &Player::setLooping);
    // -------------------------------------------------------------------------------------------------------

    QSignalMapper* signalMapper = new QSignalMapper(this);

    mDelPastAct = new QAction(tr("&Past part of all trj."), this);
    connect(mDelPastAct, SIGNAL(triggered()), signalMapper, SLOT(map()));
    signalMapper->setMapping(mDelPastAct, -1);
    connect(signalMapper, SIGNAL(mapped(int)), this, SLOT(deleteTrackPointAll(int))); // -1

    mDelFutureAct = new QAction(tr("&Future part of all trj."), this);
    connect(mDelFutureAct, SIGNAL(triggered()), signalMapper, SLOT(map()));
    signalMapper->setMapping(mDelFutureAct, 1);
    connect(signalMapper, SIGNAL(mapped(int)), this, SLOT(deleteTrackPointAll(int))); // 1

    mDelAllRoiAct = new QAction(tr("&Trj. moving through ROI"), this);
    connect(mDelAllRoiAct, SIGNAL(triggered()), this, SLOT(deleteTrackPointROI()));
    mDelPartRoiAct = new QAction(tr("Part of Trj. inside &ROI"), this);
    connect(mDelPartRoiAct, SIGNAL(triggered()), this, SLOT(deleteTrackPointInsideROI()));

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

void Petrack::createMenus()
{
    mFileMenu = new QMenu(tr("&File"), this);
    mFileMenu->addAction(mOpenPrAct);
    mFileMenu->addAction(mSaveAct);
    mFileMenu->addAction(mSavePrAct);
    mFileMenu->addSeparator();
    mFileMenu->addAction(mOpenSeqAct);
    mFileMenu->addAction(mOpenCameraAct);
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

void Petrack::createStatusBar()
{
    QFont f("Courier", 12, QFont::Bold); //Times Helvetica, Normal
    statusBar()->setMaximumHeight(28);
    statusBar()->showMessage(tr("Ready"));
    statusBar()->addPermanentWidget(mStatusLabelStereo = new QLabel(" "));
    statusBar()->addPermanentWidget(mStatusLabelTime = new QLabel(" "));
    statusBar()->addPermanentWidget(mStatusLabelFPS = new QLabel(" "));
    statusBar()->addPermanentWidget(mStatusPosRealHeight = new QDoubleSpinBox());
    connect(mStatusPosRealHeight, SIGNAL(valueChanged(double)), this, SLOT(setStatusPosReal()));

    //mStatusPosRealHeight->setEnabled(false); // temporaer, damit nichts eingetragen werden kann!!!!!!
    statusBar()->addPermanentWidget(mStatusLabelPosReal = new QLabel(" "));
    statusBar()->addPermanentWidget(mStatusLabelPos = new QLabel(" "));
    statusBar()->addPermanentWidget(mStatusLabelColor = new QLabel(" "));
    mStatusLabelStereo->setFont(f);
    mStatusLabelStereo->setMinimumWidth(200);
    mStatusLabelTime->setFont(f);
    mStatusLabelTime->setMinimumWidth(200);
    //mStatusLabelTime->setAlignment(Qt::AlignRight); ??? verschiebt text nach oben???!!!
    mStatusLabelFPS->setFont(f);
    mStatusLabelFPS->setMinimumWidth(80);    
    mStatusLabelFPS->setAutoFillBackground(true);
    mStatusLabelFPS->setToolTip("Click to adapt play rate to fps rate");
    //mStatusLabelFPS->setFrameStyle(QFrame::VLine);
    mStatusPosRealHeight->setRange(-999.9, 9999.9); // in cm
    mStatusPosRealHeight->setDecimals(1);
    mStatusPosRealHeight->setFont(f);
    mStatusLabelPosReal->setFont(f);
    mStatusLabelPosReal->setMinimumWidth(340);
    //     mStatusLabelPosReal->setFrameStyle(QFrame::Sunken|QFrame::Panel);
    mStatusLabelPos->setFont(f);
    mStatusLabelPos->setMinimumWidth(100);
    //mStatusLabelPos->setFrameStyle(QFrame::VLine|QFrame::Sunken);
    // debout << mStatusLabelPos->frameWidth() <<endl; ???? warum 0, daher wird auch sunken, vline nicht richtig gesetzt!!!!
    mStatusLabelColor->setFont(f);
    mStatusLabelColor->setMinimumWidth(90);
    mStatusLabelColor->setAutoFillBackground(true);
}

void Petrack::resetUI()
{
    ///ToDo:
    ///
    /// Reset all UI elements to default settings
    /// Noetig damit alle UI Elemente, welche in der neu geladenen Projekt-Datei z.B. noch nicht vorhanden sind, auf sinnvolle Werte gesetzt werden.
    /// Anderenfalls kommt es evtl. beim nacheinander laden verschiedener Projekte zu einem Programmabsturz
    ///
    return;
    /// sequence , height
    mSeqFileName = "";
    mAnimation->free();
    if( mImage )
    {
        mImage->fill(QColor::fromRgb(255,255,255));
        mImageItem->setImage(mImage);
        mLogoItem->show();
    }
    mStatusPosRealHeight->setValue(180.0);
    /// calibration params
    /// image filters
    mControlWidget->filterBrightContrast->setCheckState(Qt::Unchecked);
    mControlWidget->filterBrightParam->setValue(0);
    mControlWidget->filterContrastParam->setValue(0);

    mControlWidget->filterBorder->setCheckState(Qt::Unchecked);
    mControlWidget->filterBorderParamSize->setValue(0);
    QColor defaultBorder = Qt::black;
    getBorderFilter()->getBorderColR()->setValue(defaultBorder.red());
    getBorderFilter()->getBorderColG()->setValue(defaultBorder.green());
    getBorderFilter()->getBorderColB()->setValue(defaultBorder.blue());

    mControlWidget->filterBg->setCheckState(Qt::Unchecked);
    mControlWidget->filterBgDeleteNumber->setValue(3);
    mControlWidget->filterBgDeleteTrj->setCheckState(Qt::Checked);
    mControlWidget->filterBgShow->setCheckState(Qt::Unchecked);
    mControlWidget->filterBgUpdate->setCheckState(Qt::Unchecked);
    getBackgroundFilter()->setFilename(NULL);
    getBackgroundFilter()->reset();

    mControlWidget->filterSwap->setCheckState(Qt::Unchecked);
    mControlWidget->filterSwapH->setCheckState(Qt::Unchecked);
    mControlWidget->filterSwapV->setCheckState(Qt::Unchecked);

    /// intrinsic params
    mControlWidget->apply->setCheckState(Qt::Unchecked);
    mControlWidget->fx->setValue(1000);
    mControlWidget->fy->setValue(1000);
    mControlWidget->cx->setValue(960);
    mControlWidget->cx->setMinimum(0);
    mControlWidget->cx->setMaximum(mControlWidget->cx->value()*2);
    mControlWidget->cy->setValue(540);
    mControlWidget->cx->setMinimum(0);
    mControlWidget->cy->setMaximum(mControlWidget->cy->value()*2);
    mControlWidget->r2->setValue(0);
    mControlWidget->r4->setValue(0);
    mControlWidget->r6->setValue(0);
    mControlWidget->tx->setValue(0);
    mControlWidget->ty->setValue(0);
    //mControlWidget->k4->setValue(0);
    //mControlWidget->k5->setValue(0);
    //mControlWidget->k6->setValue(0);
    mControlWidget->quadAspectRatio->setCheckState(Qt::Unchecked);
    mControlWidget->fixCenter->setCheckState(Qt::Unchecked);
    mControlWidget->tangDist->setCheckState(Qt::Checked);
    getAutoCalib()->setBoardSizeX(6);
    getAutoCalib()->setBoardSizeY(8);
    getAutoCalib()->setSquareSize(5.0);
    getAutoCalib()->setCalibFiles(QStringList());

    /// extrinsic params
    mControlWidget->trans1->setValue(0);
    mControlWidget->trans2->setValue(0);
    mControlWidget->trans3->setValue(-500);
    mControlWidget->rot1->setValue(0);
    mControlWidget->rot2->setValue(0);
    mControlWidget->rot3->setValue(0);
    getExtrCalibration()->setExtrCalibFile(NULL);

    /// coord system
    mControlWidget->coordShow->setCheckState(Qt::Unchecked);
    mControlWidget->coordFix->setCheckState(Qt::Unchecked);
    mControlWidget->coordTab->setCurrentIndex(0);
    // 3D
    mControlWidget->coord3DTransX->setValue(0);
    mControlWidget->coord3DTransY->setValue(0);
    mControlWidget->coord3DTransZ->setValue(0);
    mControlWidget->coord3DAxeLen->setValue(200);
    mControlWidget->coord3DSwapX->setCheckState(Qt::Unchecked);
    mControlWidget->coord3DSwapY->setCheckState(Qt::Unchecked);
    mControlWidget->coord3DSwapZ->setCheckState(Qt::Unchecked);
    mControlWidget->extCalibPointsShow->setCheckState(Qt::Unchecked);
    mControlWidget->extVanishPointsShow->setCheckState(Qt::Unchecked);
    // 2D
    mControlWidget->coordTransX->setValue(0);
    mControlWidget->coordTransY->setValue(0);
    mControlWidget->coordRotate->setValue(0);
    mControlWidget->coordScale->setValue(100);
    mControlWidget->coordAltitude->setValue(535);
    mControlWidget->coordUnit->setValue(100);
    mControlWidget->coordUseIntrinsic->setCheckState(Qt::Unchecked);

    /// alignment grid
    mControlWidget->gridShow->setCheckState(Qt::Unchecked);
    mControlWidget->gridFix->setCheckState(Qt::Unchecked);
    mControlWidget->gridTab->setCurrentIndex(0);
    // 3D
    mControlWidget->grid3DTransX->setValue(0);
    mControlWidget->grid3DTransY->setValue(0);
    mControlWidget->grid3DTransZ->setValue(0);
    mControlWidget->grid3DResolution->setValue(100);
    // 2D
    mControlWidget->gridTransX->setValue(0);
    mControlWidget->gridTransY->setValue(0);
    mControlWidget->gridRotate->setValue(0);
    mControlWidget->gridScale->setValue(100);

    ///
    /// recognition params
    ///
    mControlWidget->performRecognition->setCheckState(Qt::Unchecked);
    mControlWidget->recoStep->setValue(1);

    // region of interest
    mControlWidget->roiFix->setCheckState(Qt::Unchecked);
    mControlWidget->roiShow->setCheckState(Qt::Unchecked);
    getRecoRoiItem()->setRect(0,0,0,0);

    // marker
    mControlWidget->recoMethod->setCurrentIndex(5);
    mControlWidget->markerBrightness->setValue(100);
    mControlWidget->markerIgnoreWithout->setCheckState(Qt::Checked);

    // size and color
    mControlWidget->recoShowColor->setCheckState(Qt::Checked);
    mControlWidget->recoAutoWB->setCheckState(Qt::Checked);
    mControlWidget->recoColorX->setCurrentIndex(0);
    mControlWidget->recoColorY->setCurrentIndex(1);
    mControlWidget->recoColorZ->setValue(255);
    mControlWidget->recoGreyLevel->setValue(50);
    mControlWidget->recoSymbolSize->setValue(10);

    // map
    mControlWidget->recoColorModel->setCurrentIndex(0);
    mControlWidget->colorPlot->getMapItem()->delMaps();
    mControlWidget->mapNr->setValue(0);
    mControlWidget->mapNr->setMinimum(0);
    mControlWidget->mapNr->setMaximum(0);
    mControlWidget->mapX->setValue(0);
    mControlWidget->mapY->setValue(0);
    mControlWidget->mapW->setValue(0);
    mControlWidget->mapH->setValue(0);
    mControlWidget->mapColor->setCheckState(Qt::Checked);
    mControlWidget->mapHeight->setValue(180);
//    mControlWidget->colorPlot->replot();
    mControlWidget->mapDefaultHeight->setValue(180);

    ///
    /// tracking params
    ///
    mControlWidget->trackOnlineCalc->setCheckState(Qt::Checked);
    mControlWidget->trackRepeat->setCheckState(Qt::Checked);
    mControlWidget->trackRepeatQual->setValue(50);
    mControlWidget->trackExtrapolation->setCheckState(Qt::Checked);
    mControlWidget->trackMerge->setCheckState(Qt::Unchecked);
    mControlWidget->trackOnlyVisible->setCheckState(Qt::Checked);
    getTrackRoiItem()->setRect(0,0,0,0);

    // export options
    mControlWidget->trackMissingFrames->setCheckState(Qt::Checked);
    mControlWidget->trackRecalcHeight->setCheckState(Qt::Checked);
    mControlWidget->trackAlternateHeight->setCheckState(Qt::Unchecked);
    mControlWidget->exportElimTp->setCheckState(Qt::Unchecked);
    mControlWidget->exportElimTrj->setCheckState(Qt::Unchecked);
    mControlWidget->exportSmooth->setCheckState(Qt::Checked);
    mControlWidget->exportViewDir->setCheckState(Qt::Unchecked);
    mControlWidget->exportAngleOfView->setCheckState(Qt::Unchecked);
    mControlWidget->exportUseM->setCheckState(Qt::Unchecked);
    mControlWidget->exportComment->setCheckState(Qt::Unchecked);
    mControlWidget->exportMarkerID->setCheckState(Qt::Unchecked);

    // test options
    mControlWidget->testEqual->setCheckState(Qt::Checked);
    mControlWidget->testVelocity->setCheckState(Qt::Checked);
    mControlWidget->testInside->setCheckState(Qt::Checked);
    mControlWidget->testLength->setCheckState(Qt::Checked);
    setTrackFileName(NULL);

    // search region
    mControlWidget->trackRegionScale->setValue(16);
    mControlWidget->trackRegionLevels->setValue(3);
    mControlWidget->trackErrorExponent->setValue(0);
    mControlWidget->trackShowSearchSize->setCheckState(Qt::Unchecked);

    // path params
    mControlWidget->trackShow->setCheckState(Qt::Checked);
    mControlWidget->trackFix->setCheckState(Qt::Unchecked);

    mControlWidget->trackShowOnlyVisible->setCheckState(Qt::Unchecked);
    mControlWidget->trackShowOnly->setCheckState(Qt::Unchecked);
    mControlWidget->trackShowOnlyNr->setValue(1);
    mControlWidget->trackShowOnlyList->setCheckState(Qt::Unchecked);
    mControlWidget->trackShowOnlyNrList->setEnabled(false);
    mControlWidget->trackShowOnlyListButton->setEnabled(false);

    mControlWidget->trackShowCurrentPoint->setCheckState(Qt::Checked);
    mControlWidget->trackShowPoints->setCheckState(Qt::Unchecked);
    mControlWidget->trackShowPath->setCheckState(Qt::Checked);
    mControlWidget->trackShowColColor->setCheckState(Qt::Checked);
    mControlWidget->trackShowColorMarker->setCheckState(Qt::Checked);
    mControlWidget->trackShowNumber->setCheckState(Qt::Checked);
    mControlWidget->trackShowGroundPosition->setCheckState(Qt::Unchecked);
    mControlWidget->trackShowGroundPath->setCheckState(Qt::Unchecked);

    mControlWidget->setTrackPathColor(Qt::red);
    mControlWidget->setTrackGroundPathColor(Qt::green);
    mControlWidget->trackHeadSized->setCheckState(Qt::Checked);
    mControlWidget->trackCurrentPointSize->setValue(60);
    mControlWidget->trackPointSize->setValue(7);
    mControlWidget->trackPathWidth->setValue(2);
    mControlWidget->trackColColorSize->setValue(11);
    mControlWidget->trackColorMarkerSize->setValue(14);
    mControlWidget->trackNumberSize->setValue(14);
    mControlWidget->trackGroundPositionSize->setValue(1);
    mControlWidget->trackGroundPathSize->setValue(1);

    mControlWidget->trackShowPointsColored->setCheckState(Qt::Checked);
    mControlWidget->trackNumberBold->setCheckState(Qt::Checked);
    mControlWidget->trackShowBefore->setValue(15);
    mControlWidget->trackShowAfter->setValue(15);

    /// analysis params
    mControlWidget->anaMissingFrames->setCheckState(Qt::Checked);
    mControlWidget->anaMarkAct->setCheckState(Qt::Unchecked);
    mControlWidget->anaStep->setValue(25);
    mControlWidget->anaConsiderX->setCheckState(Qt::Unchecked);
    mControlWidget->anaConsiderY->setCheckState(Qt::Checked);
    mControlWidget->anaConsiderAbs->setCheckState(Qt::Unchecked);
    mControlWidget->anaConsiderRev->setCheckState(Qt::Unchecked);
    mControlWidget->showVoronoiCells->setCheckState(Qt::Unchecked);

//    updateImage();

}

void Petrack::setStatusStereo(float x, float y, float z)
{
    if (mStatusLabelStereo)
    {
        if (z<0)
            mStatusLabelStereo->setText(QString("x= novalue  y= novalue  z= novalue  "));
        else
            mStatusLabelStereo->setText(QString("x=%1cm  y=%2cm  z=%3cm  ").arg(x, 6, 'f', 1).arg(y, 6, 'f', 1).arg(z, 6, 'f', 1));
    }
}

void Petrack::setStatusTime()
{
    if (mStatusLabelTime)
        mStatusLabelTime->setText(mAnimation->getTimeString());
}

void Petrack::setStatusFPS()
{
    if (mStatusLabelFPS)
    {
        mStatusLabelFPS->setText(QString("%1fps  ").arg(mShowFPS, 5, 'f', 1));

        QPalette pal = mStatusLabelFPS->palette(); // static moeglich?
        QColor color;//(qRed(col), qGreen(col), qBlue(col));

        double diff = mShowFPS-mAnimation->getFPS();
        int opacity = mPlayerWidget->getPlayerSpeedLimited() ? 128 : 20;

        if( diff < -6 ) // very slow ==> red
            color.setRgb(200,0,0,opacity);//mStatusLabelFPS->setStyleSheet("background-color: rgba(200,0,0,20);");// border-style: outset; border-width: 1px; border-color: yellow;");//, qRgba(250,0,0,50));//Qt::yellow);
        else if( diff < -2 ) // better ==> yellow
            color.setRgb(200,200,0,opacity);//mStatusLabelFPS->setStyleSheet("background-color: rgba(200,200,0,20);");// border-style: outset; border-width: 1px; border-color: yellow;");//, qRgba(250,0,0,50));//Qt::yellow);
        else if (diff > -2) // nearly ok ==> green
            color.setRgb(0,200,0,opacity);//mStatusLabelFPS->setStyleSheet("background-color: rgba(0,200,0,20);");// border-style: outset; border-width: 1px; border-color: red;");//, qRgba(250,0,0,50));//Qt::yellow);

        pal.setColor(QPalette::Window, color);
//        pal.setColor(QPalette::WindowText,mPlayerWidget->getPlayerSpeedFixed() ? QColor(100,100,100) : QColor(0,0,0));
        //        pal.setBrush(QPalette::WindowText, mPlayerWidget->getPlayerSpeedFixed() ? QBrush(QColor(50,50,50),Qt::Dense2Pattern) : QBrush(QColor(0,0,0),Qt::SolidPattern));

        mStatusLabelFPS->setPalette(pal);
    }
}
void Petrack::setShowFPS(double fps)
{
    if ((fps == 0.) || (mShowFPS == 0))
        mShowFPS = fps;
    else
        mShowFPS = mShowFPS*.9+fps*.1; // glaetten durch Hinzunahme des alten Wertes
    setStatusFPS();
}

/**
 * @brief Updates the FPS shown to the User
 *
 * This method calculates the FPS by remembering how long
 * it has been since it was called last time. If skipped is
 * true, it doesn't directly updat the FPS since 2
 * skipped frames have essentially a time delay of 0 between
 * them, which would make calculations wonky.
 *
 * @param skipped True, if this is a skipped frame; default false
 */
void Petrack::updateShowFPS(bool skipped) {
    static QElapsedTimer lastTime;
    static int skippedFrames = 0;

    if(skipped){
        skippedFrames++;
        return;
    }

    if (mPlayerWidget->getPaused())
    {
        setShowFPS(0.);
        lastTime.invalidate();
    }
    else
    {
        if (lastTime.isValid())
        {
            if (lastTime.elapsed() > 0)
            {
                int numFrames = skippedFrames > 0 ? skippedFrames+1 : 1;
                setShowFPS(numFrames*1000./lastTime.elapsed());
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
    if (mImageItem)
        setStatusPosReal(mImageItem->getPosReal(mMousePosOnImage, getStatusPosRealHeight()));
}
void Petrack::setStatusPosReal(const QPointF &pos) // pos in cm
{
    if (mStatusLabelPosReal)
    {
        QChar deg(0xB0);
        QString labelText = QString(" cm from ground:%1cm,%2cm,%3").arg(pos.x(), 6, 'f', 1).arg(pos.y(), 6, 'f', 1).arg(
                                    getImageItem()->getAngleToGround(mMousePosOnImage.x(), mMousePosOnImage.y(), getStatusPosRealHeight()), 5, 'f', 1);
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
    if ((qRed(col)+qGreen(col)+qBlue(col))/3 < 128)
        mStatusLabelColor->setText(QString("<font color=\"#ffffff\">&nbsp;%1</font>").arg(s));
    else
        mStatusLabelColor->setText(QString("<font color=\"#000000\">&nbsp;%1</font>").arg(s));

    //     mStatusLabelColor->setBackgroundRole(QPalette::Text);
    QPalette pal = mStatusLabelColor->palette(); // static moeglich?
    QColor color(qRed(col), qGreen(col), qBlue(col));

    pal.setColor(QPalette::Window, color);
    mStatusLabelColor->setPalette(pal);

    mControlWidget->getColorPlot()->setCursor(color);
    mControlWidget->getColorPlot()->replot();

    //debout << "QColor: " << color << " height: " << mControlWidget->getColorPlot()->map(color) << endl;
}
void Petrack::setStatusColor()
{
    QPointF pos = getMousePosOnImage();
    if ((int)(pos.x())<mImage->width() && (int)(pos.y())<mImage->height())
    {
        QPoint pos2((int)(pos.x()), (int)(pos.y())); // .toPoint() ???
        setStatusColor(mImage->pixel(pos2));
    }
    //     else
    //         setStatusColor(qRgb(255, 255, 255));
}

double Petrack::getStatusPosRealHeight()
{
    if (mStatusPosRealHeight)
        return mStatusPosRealHeight->value();
    else
        return 0.;
}

void Petrack::readSettings()
{
    QSettings settings("Forschungszentrum Juelich GmbH", "PeTrack by Maik Boltes, Daniel Salden");
    QPoint pos = settings.value("pos", QPoint(200, 200)).toPoint();
    QSize size = settings.value("size", QSize(400, 400)).toSize();
    mAntialiasAct->setChecked(settings.value("antialias", false).toBool());
    mOpenGLAct->setChecked(settings.value("opengl", false).toBool());
    mSeqFileName = settings.value("seqFileName", QDir::currentPath()).toString();
    mProFileName = settings.value("proFilePath", QDir::currentPath()).toString();
    // nicht ganz sauber, da so immer schon zu anfang in calib file list etwas drin steht und somit auto ausgefuehrt werden kann
    // wird aber beim ersten openCalib... ueberschrieben
    mAutoCalib.addCalibFile(settings.value("calibFile", QDir::currentPath()).toString());
    resize(size);
    move(pos);
    antialias();
    opengl();
}

void Petrack::writeSettings()
{
    QSettings settings("Forschungszentrum Juelich GmbH", "PeTrack by Maik Boltes, Daniel Salden");
    settings.setValue("pos", pos());
    settings.setValue("size", size());
    settings.setValue("antialias", mAntialiasAct->isChecked());
    settings.setValue("opengl", mOpenGLAct->isChecked());
    settings.setValue("seqFileName", mSeqFileName);
    settings.setValue("proFilePath", QFileInfo(mProFileName).path()); // nur path, damit bei saveCurrentProject
    if (!mAutoCalib.isEmptyCalibFiles()) //!mCalibFiles.isEmpty()
        settings.setValue("calibFile", mAutoCalib.getCalibFile(0));
}

bool Petrack::maybeSave()
{
    int ret = QMessageBox::warning(this, tr("PeTrack"),
                                   tr("Do you want to save\n"
                                      "the current project?\n"
                                      "Be sure to save trajectories, background\n"
                                      "and 3D calibration point separately!"),
                                   QMessageBox::Yes | QMessageBox::Default,
                                   QMessageBox::No,
                                   QMessageBox::Cancel | QMessageBox::Escape);

    if (ret == QMessageBox::Yes)
    {
        if (saveSameProject())
            return true;
        else
            return false;
    }
    else if (ret == QMessageBox::Cancel)
        return false;
    else
        return true;
}

void Petrack::closeEvent(QCloseEvent *event)
{

    if (maybeSave())
    {
        writeSettings();
        event->accept();
    }
    else
        event->ignore();

}

void Petrack::setMousePosOnImage(QPointF pos)
{
    if (mImage)
    {
        mMousePosOnImage = pos;
        // real coordinate
        setStatusPosReal(mImageItem->getPosReal(pos, getStatusPosRealHeight()));

        // pixel coordinate
        //s.sprintf("%4dx%4d", event->pos().x(), event->pos().y());
        //QTextStream(&s) << event->pos().x() << "x" << event->pos().y();
        QPoint pos1((int)(pos.x())+1, (int)(pos.y())+1);
        setStatusPos(pos1);  //QString("%1x%2").arg((int)event->pos().x()+1, 4).arg((int)event->pos().y()+1, 4)
        //== event->scenePos().x()
        // Koordinaten auf dem Bildschirm: event->screenPos().x(), event->screenPos().y()
        
        // pixel color
        QPoint pos2((int)(pos.x()), (int)(pos.y()));
        setStatusColor(mImage->pixel(pos2));//(mImage->toImage()).pixel(pos... wenn pixmap

        //        if ((mStereoContext) && mStereoContext->getDisparity())
        //        {
        //        }
    }
}

void Petrack::keyPressEvent(QKeyEvent * event)
{
    switch (event->key()) {
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
//        debout << "test Key_D" << endl;
        break;
    default:
        ;
    }
//    QMainWindow::keyPressEvent(event);
}

void Petrack::mousePressEvent(QMouseEvent *event)
{
    // mouse click in fps status label ?
    if( event->pos().x() >= mStatusLabelFPS->pos().x() && event->pos().x() <= mStatusLabelFPS->pos().x()+mStatusLabelFPS->width() )
    {
        mPlayerWidget->togglePlayerSpeedLimited();
        setStatusFPS();
    }
}

// update control widget, if image size changed (especially because of changing border)
void Petrack::updateControlImage(Mat &img)
{

    // auch moeglich hoehe und breite von bild stat border veraenderungen zu checken
    static int lastBorderSize = -1;

    if( isLoading() )
        lastBorderSize = -1;

    int diffBorderSize=0;
    int iW = img.cols;
    int iH = img.rows;

    // wird auch nochmal in ImageWidget gemacht, aber ist hier frueher noetig
    double cX = mControlWidget->getCalibCxValue(); // merken, da min/max value verandernkann wenn aus dem rahmen
    double cY = mControlWidget->getCalibCyValue();

    mControlWidget->setCalibCxMin(0  /*iW/2.-50.*/);
    mControlWidget->setCalibCxMax(iW /*iW/2.+50.*/);
    mControlWidget->setCalibCyMin(0  /*iH/2.-50.*/);
    mControlWidget->setCalibCyMax(iH /*iH/2.+50.*/);

    if (mControlWidget->fixCenter->checkState() == Qt::Checked)
    {
        mControlWidget->setCalibCxValue((iW-1)/2.);
        mControlWidget->setCalibCyValue((iH-1)/2.);
    }
    else
    {
        if (lastBorderSize != -1)
            diffBorderSize = getImageBorderSize()-lastBorderSize;
        lastBorderSize = getImageBorderSize();

        mControlWidget->setCalibCxValue(cX+diffBorderSize);
        mControlWidget->setCalibCyValue(cY+diffBorderSize);
    }
}

void Petrack::importTracker(QString dest) //default = ""
{
    static QString lastFile;

    if (lastFile == "")
        lastFile = mTrcFileName;

    // if no destination file or folder is given
    if (dest.isEmpty())
    {
        dest = QFileDialog::getOpenFileName(this, tr("Select file for importing tracking pathes"), lastFile,
                                            tr("PeTrack tracker (*.trc *.txt);;All files (*.*)"));
    }
    // in control
    //     else
    //         dest = getExistingFile(dest); // ueberprueft die ;-getrennten Dateinamen auf existenz und gibt ersten zurueck - interessant fuer relativen und absoluten pfad

    if (!dest.isEmpty())
    {
        if (dest.right(4) == ".trc")
        {
            QFile file(dest);
            int i, sz;

            if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
            {
                // errorstring ist je nach Betriebssystem in einer anderen Sprache!!!!
                QMessageBox::critical(this, tr("PeTrack"), tr("Cannot open %1:\n%2").arg(dest).arg(file.errorString()));
                return;
            }

            setTrackChanged(true);// flag changes of track parameters
            mTracker->reset();

            QTextStream in(&file);
            TrackPerson tp;
            QString comment;

            bool ok; // shows if int stands in first line - that was in the first version of trc file
            QString firstLine = in.readLine();
            sz = firstLine.toInt(&ok);
            if (!ok)
            {
                if (firstLine.contains("version 4",Qt::CaseInsensitive))
                {
                    trcVersion = 4;
                }
                else if (firstLine.contains("version 3",Qt::CaseInsensitive))
                {
                    trcVersion = 3;
                }
                else if(firstLine.contains("version 2",Qt::CaseInsensitive))
                {
                    trcVersion = 2;
                }
                else
                {
                    debout << "Error: wrong header while reading TRC file." << endl;
                    sz = 0;
                }
                in >> sz;
            }
            else
                trcVersion = 1;

            if ((sz > 0) && (mTracker->size() != 0))
                debout << "Warning: Overlapping trajectories will be joined not until tracking adds new trackpoints." << endl;
            for (i = 0; i < sz; ++i)
            {
                if( trcVersion == 2)
                {
                    in >> tp;
                }else if( trcVersion >= 3)
                {

//                    in.skipWhiteSpace(); // skip white spaces for reading the comment line without this the reading makes some problems
//                    // Kommentarzeile lesen
//                    comment = in.readLine();
//                    // Kommentarzeichen entfernen - wird beim exportieren wieder hinzugefuegt
//                    if( comment.startsWith("#"))
//                    {
//                        comment = comment.remove(0,2);
//                    }
                    in >> tp;
//                    tp.setComment(comment);

                }
                mTracker->append(tp);
                tp.clear();  // loeschen, sonst immer weitere pfade angehangen werden
            }

            mControlWidget->trackNumberAll->setText(QString("%1").arg(mTracker->size()));
            mControlWidget->trackShowOnlyNr->setMaximum(MAX(mTracker->size(),1));
            mControlWidget->trackNumberVisible->setText(QString("%1").arg(mTracker->visible(mAnimation->getCurrentFrameNum())));
            mControlWidget->colorPlot->replot();
            file.close();
            debout << "import " << dest << " (" << sz << " person(s), file version " << trcVersion << ")" << endl;
            mTrcFileName = dest; // fuer Project-File, dann koennte track path direkt mitgeladen werden, wenn er noch da ist
        }else
        if (dest.right(4) == ".txt") // 3D Koordinaten als Tracking-Daten importieren Zeilenformat: Personennr, Framenr, x, y, z
        {
            QMessageBox::warning(this,tr("PeTrack"), tr("Are you sure you want to import 3D data from TXT-File? You have to make sure that the coordinate system now is exactly at the same position and orientation than at export time!"));

            QFile file(dest);
            // size of person list
            int sz = 0;

            if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
            {
                // errorstring ist je nach Betriebssystem in einer anderen Sprache!!!!
                QMessageBox::critical(this, tr("PeTrack"), tr("Cannot open %1:\n%2").arg(dest).arg(file.errorString()));
                return;
            }

            setTrackChanged(true);// flag changes of track parameters
            mTracker->reset();

            //mTrackerReal->reset(); ???

            QTextStream in(&file);
            TrackPerson tp;
            TrackPoint tPoint;
            Point2f p2d;

            QString line;
            QString headerline;
            bool exec_once_flag = false;
            double conversionFactorTo_cm = 1.0;
            int personNr = -1, frameNr = -1, current_personNr = 1;
            float x, y ,z;

            while( 1 )
            {

                // Falls Datei am Ende letzte Person abspeichern und Lese-Schleife beenden
                if( in.atEnd() )
                {
                    tp.setLastFrame(frameNr);
                    mTracker->append(tp);
                    ++sz;
                    tp.clear();
                    break;
                }

                line = in.readLine();

                // Kommentare ueberlesen
                if( line.startsWith("#",Qt::CaseInsensitive) )
                {
                    headerline = line;
                    continue;
                }

                // Test-Ausgabe
                //debout << line << endl;

                if ((!exec_once_flag) && (!headerline.contains("cm")))
                {
                    conversionFactorTo_cm = 100.0;
                    exec_once_flag = true;
                    QMessageBox::warning(this,tr("PeTrack"), tr("PeTrack will interpret position data as unit [m]. No header with [cm] found."));
                }

                QTextStream stream(&line);

                // Zeile als Stream einlesen Format: [id frame x y z]
                stream >> personNr
                       >> frameNr
                       >> x
                       >> y
                       >> z;

                // convert data to cm
                x = x * conversionFactorTo_cm;
                y = y * conversionFactorTo_cm;
                z = z * conversionFactorTo_cm;

                // 3-dimensionale Berechnung/Anzeige des Punktes
                if( mControlWidget->getCalibCoordDimension() == 0 )
                {
                    p2d = mExtrCalibration.getImagePoint(Point3f(x,y,z));
                }
                // 2-dimensionale Berechnung/Anzeige des Punktes
                else
                {
                    QPointF pos = mImageItem->getPosImage(QPointF(x,y),z);
                    p2d.x = pos.x();
                    p2d.y = pos.y();
                }

                tPoint = TrackPoint(Vec2F(p2d.x,p2d.y),100);
                tPoint.setSp(x,y,-mControlWidget->getCalibExtrTrans3()-z); // fuer den Abstand zur Kamera in z-Richtung wie bei einer Stereokamera
                //tPoint.setSp(x,y,z); // Eigentlich bisher nur fuer Stereodaten verwendet

                // Neue ID ? ==> letzte Person beendet ==> abspeichern
                if( personNr > current_personNr )
                {
                    mTracker->append(tp);
                    ++sz;
                    current_personNr++;
                    tp.clear();
                }

                // TrackPerson leer ? ==> Neue TrackPerson erstellen
                if ( tp.isEmpty() )
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

                // Test-Ausgabe
                //debout << "Person: " << personNr << " at Frame: " << frameNr << " at 2D-Position: (" << p2d.x << ", " << p2d.y << ") and 3D-Position: (" << x << ", " << y << ", " << z << ")" << endl;

            }

            mControlWidget->trackNumberAll->setText(QString("%1").arg(mTracker->size()));
            mControlWidget->trackShowOnlyNr->setMaximum(MAX(mTracker->size(),1));
            mControlWidget->trackNumberVisible->setText(QString("%1").arg(mTracker->visible(mAnimation->getCurrentFrameNum())));
            mControlWidget->colorPlot->replot();
            file.close();
            debout << "import " << dest << " (" << sz << " person(s) )" << endl;
            mTrcFileName = dest; // fuer Project-File, dann koennte track path direkt mitgeladen werden, wenn er noch da ist
        }
        else
        {
            QMessageBox::critical(this, tr("PeTrack"), tr("Cannot load %1 maybe because of wrong file extension.").arg(dest));
        }
        lastFile = dest;
    }
}

void Petrack::testTracker()
{
    static int idx=0; // index in Fehlerliste, die als letztes angesprungen wurde
    QList<int> pers, frame;

    mTracker->checkPlausibility(pers, frame,
                                mControlWidget->testEqual->isChecked(),
                                mControlWidget->testVelocity->isChecked(),
                                mControlWidget->testInside->isChecked(),
                                mControlWidget->testLength->isChecked());
    if (pers.length()<=idx)
        idx=0;
    if (pers.length()>idx)
    {
        mControlWidget->trackShowOnly->setCheckState(Qt::Checked);
        mControlWidget->trackShowOnlyNr->setValue(pers[idx]);
//        mControlWidget->trackShowOnlyNr->setValue(pers[idx]);
        mPlayerWidget->skipToFrame(frame[idx]); // QString().number(mAnimation->getCurrentFrameNum())
        //mControlWidget->trackGotoNr->click();

        ++idx;
    }

}

int Petrack::calculateRealTracker()
{
    bool autoCorrectOnlyExport = (mControlWidget->getRecoMethod() == 5) && // multicolor
            mMultiColorMarkerWidget->autoCorrect->isChecked() &&
            mMultiColorMarkerWidget->autoCorrectOnlyExport->isChecked();
    //int anz = mTrackerReal->calculate(mTracker, mImageItem, mControlWidget->getColorPlot(), getImageBorderSize(), mControlWidget->anaMissingFrames->checkState());
    int anz = mTrackerReal->calculate(mTracker, mImageItem, mControlWidget->getColorPlot(), getImageBorderSize(),
                            mControlWidget->anaMissingFrames->checkState(), //mControlWidget->trackMissingFrames->checkState(),
                            mStereoWidget->stereoUseForExport->isChecked(),
                            mControlWidget->trackAlternateHeight->checkState(), mControlWidget->coordAltitude->value(), mStereoWidget->stereoUseCalibrationCenter->isChecked(),
                            mControlWidget->exportElimTp->isChecked(), mControlWidget->exportElimTrj->isChecked(), mControlWidget->exportSmooth->isChecked(),
                            mControlWidget->exportViewDir->isChecked(), mControlWidget->exportAngleOfView->isChecked(), mControlWidget->exportMarkerID->isChecked(),
                            autoCorrectOnlyExport);

    mTrackerReal->calcMinMax();
    return anz;
}


// Veraltet wird in extCalibration gemacht!
//void Petrack::load3DCalibPoints(QString src) // default = ""
//{

//    bool all_ok = true;

//    vector<Point3f> list3D;
//    vector<Point2f> list2D;

//    if( !src.isEmpty() )
//    {
//        if( src.right(4) == ".3dc" )
//        {
//            //mExtrCalibration->setExtrCalibFile(src);
//            QFile file(src);
//            if( !file.open(QIODevice::ReadOnly | QIODevice::Text) )
//            {
//                QMessageBox::critical(this, tr("PeTrack"), tr("Error: Cannot open %1:\n%2.").arg(src).arg(file.errorString()));
//                return;
//            }

//            debout << "reading 3D calibration data from " << src << "..." << endl;

//            QTextStream in(&file);
//            QString line;
//            int lineCount = 0;
//            QStringList values;

//            line = in.readLine();
//            lineCount++;
//            int sz_3d = line.toInt(&all_ok);
//            if( !all_ok ){
//                QMessageBox::critical(this, tr("PeTrack"), tr("Error: First line of 3D point file should only contain the number of listed 3D-Points."));
//                return;
//            }

//            bool withImageData = false;
//            double coords[5];
//            while( !in.atEnd() )
//            {
//                line = in.readLine();

//                lineCount++;
//                // comment lines
//                if( line.startsWith("#") || line.startsWith(";;") || line.startsWith("//") || line.startsWith("!") )
//                    continue;

//                //QStream stream(line);

//                //stream >> x_3d >> y_3d << z_3d
//                //       >> x_2d << y_2d;

//                values = line.split(" ");
//                //cout << line << " (" << values.size() << ")" << endl;
//                if( lineCount == 2 && values.size() == 5 )
//                {
//                    withImageData = true;
//                }
//                if( values.size() == 3 || values.size() == 5 )
//                {
//                    //debout << values.at(0) << ", " << values.at(1) << ", " << values.at(2) << endl;
//                    for(int i=0; i<3; i++)
//                    {
//                        coords[i] = values.at(i).toDouble(&all_ok);
//                        if(!all_ok)
//                        {
//                            QMessageBox::critical(this, tr("PeTrack"), tr("Error: Reading Float-Value in line %1 column %2 crashed: %3 (File: %4)").arg(lineCount).arg(i).arg(values.at(i)).arg(src));
//                            return;
//                        }
//                    }
//                    list3D.push_back( Point3f( coords[0], coords[1], coords[2] ) );
//                    if (values.size() == 5)
//                    {
//                        if( !withImageData ){
//                            QMessageBox::critical(this, tr("PeTrack"), tr("Error: Unsupported File Format in: %1 (Only 3D-Points or 3D-Points and 2D-Points, but no mix)").arg(src));
//                            return;
//                        }
//                        for(int i=3; i<5; i++)
//                        {
//                            coords[i] = values.at(i).toDouble(&all_ok);
//                            if(!all_ok)
//                            {
//                                QMessageBox::critical(this, tr("PeTrack"), tr("Error: Reading Float-Value in line %1 column %2 crashed: %3 (File: %4)").arg(lineCount).arg(i).arg(values.at(i)).arg(src));
//                                return;
//                            }
//                        }
//                        list2D.push_back( Point2f( coords[3], coords[4] ) );

//                    }else if( withImageData )
//                    {
//                        QMessageBox::critical(this, tr("PeTrack"), tr("Error: Unsupported File Format in: %1 (Only 3D-Points or 3D-Points and 2D-Points, but no mix)").arg(src));
//                        return;
//                    }
//                }else
//                {
//                    QMessageBox::critical(this, tr("PeTrack"), tr("Error: Unsupported File Format in: %1").arg(src));
//                    return;
//                }


//            }
//            file.close();
//            all_ok = true;
//            int sz_2d = 0;
//            if( !mTracker || mTracker->size() < 4 )
//            {
//                if( !withImageData )
//                {
//                    QMessageBox::critical(this, tr("PeTrack"), tr("Error: At minimum four 3D calibration points needed for 3D calibration."));
//                }
//            }else
//            {
//                sz_2d = mTracker->size();
//                list2D.clear();

//                if( sz_2d != sz_3d ){
//                    if( !withImageData )
//                    {
//                        QMessageBox::critical(this, tr("PeTrack"), tr("Count of 2D-Points (%1) and 3D-Points (%2) disagree").arg(sz_2d).arg(sz_3d));
//                        all_ok = false;
//                    }
//                }
//                //debout << "Marked 2D-Image-Points: " << endl;
//                if( all_ok )
//                {
//                    for(int i = 0; i < sz_2d; i++)
//                    {
//                        //debout << "[" << i << "]: (" << mTracker->at(i).at(0).x() << ", " << mTracker->at(i).at(0).y() << ")" << endl;
//                        // Info: Tracker->TrackPerson->TrackPoint->Vec2F
//                        list2D.push_back(Point2f(mTracker->at(i).at(0).x(),mTracker->at(i).at(0).y()));
//                    }
//                }
//            }


//            //            mExtrCalibration->set3DList(list3D);
//            //            mExtrCalibration->set2DList(list2D);
//            //debout << "3D.x " << "3D.y " << "3D.z " << "2D.x " << "2D.y " << endl;
//            for( int i=0; i<sz_3d; i++ )
//            {
//                //                debout << ((Point3f) mExtrCalibration->get3DList().at(i)).x << " "
//                //                       << ((Point3f) mExtrCalibration->get3DList().at(i)).y << " "
//                //                       << ((Point3f) mExtrCalibration->get3DList().at(i)).z << " "
//                //                       << ((Point2f) mExtrCalibration->get2DList().at(i)).x << " "
//                //                       << ((Point2f) mExtrCalibration->get2DList().at(i)).y << endl;
//            }

//            if( !withImageData )
//            {
//                // Save corresponding image-points,
//                QFile file(src);
//                int i;

//                if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
//                {
//                    QMessageBox::critical(this, tr("PeTrack"), tr("Cannot open %1:\n%2.").arg(src).arg(file.errorString()));
//                    return;
//                }

//                QTextStream out(&file);

//                out << sz_3d << endl;
//                for (i = 0; i < sz_3d; ++i)
//                {
//                    out << list3D.at(i).x << " " << list3D.at(i).y << " " << list3D.at(i).z << " " << list2D.at(i).x << " " << list2D.at(i).y << endl;
//                }

//                file.close();

//            }

//            //            mExtrCalibration->calibExtrinsicParams();
//        }else // is 3dc-File ?
//        {
//            QMessageBox::critical(this, tr("PeTrack"), tr("File format is not supported for 3D calibration input files: %1 (supported: .3dc ").arg(src.right(4)));
//            return;
//        }
//    }else // src empty ?
//    {
//        QMessageBox::critical(this, tr("PeTrack"), tr("Cannot open 3D calibration file: file is missing."));
//        return;
//    }
//}

void Petrack::exportTracker(QString dest) //default = ""
{
    static QString lastFile;

    if (lastFile == "")
        lastFile = mTrcFileName;

    if (mTracker)
    {
        // if no destination file or folder is given
        if (dest.isEmpty())
        {
            //             if (lastFile.isEmpty() && !mSeqFileName.isEmpty())
            //                 lastFile = QFileInfo(mSeqFileName).path();
            //getSaveFileName(  ==> Wird keine Dateiendung angegeben wird automatisch die erste aus dem Filter angehangen
            //QUrl url = QFileDialog::getSaveFileUrl(this,
            //                                    tr("Select file for exporting tracking pathes"),
            //                                    lastFile,
            //                                    tr("Tracker (*.*);;Petrack tracker (*.trc);;Text (*.txt);;Text for gnuplot(*.dat);;XML Travisto (*.trav);;All supported types (*.txt *.trc *.dat *.trav *.);;All files (*.*)"));
            //dest = url.toLocalFile();//path();//toString();

            QFileDialog fileDialog(this,
                                   tr("Select file for exporting tracking pathes"),
                                   lastFile,
                                   tr("Tracker (*.*);;Petrack tracker (*.trc);;Text (*.txt);;Text for gnuplot(*.dat);;XML Travisto (*.trav);;All supported types (*.txt *.trc *.dat *.trav *.);;All files (*.*)"));
            fileDialog.setAcceptMode(QFileDialog::AcceptSave);
            fileDialog.setFileMode(QFileDialog::AnyFile);
            fileDialog.setDefaultSuffix("");
            //fileDialog.setOption();

//            foreach (QString filter, fileDialog.mimeTypeFilters()) {
//                debout << "MIME-Typ Filter: " << filter << endl;

//            }
//            foreach (QString filter, fileDialog.nameFilters()) {
//                debout << "Name Filter: " << filter << endl;

//            }

            if( fileDialog.exec() ){
                dest = fileDialog.selectedFiles().at(0);
//                debout << "selectedFiles" << endl;
            }
            //dest = QFileDialog::getSaveFileName(this,
            //                                    tr("Select file for exporting tracking pathes"),
            //                                    lastFile,
            //                                    tr("Tracker (*.*);;Petrack tracker (*.trc);;Text (*.txt);;Text for gnuplot(*.dat);;XML Travisto (*.trav);;All supported types (*.txt *.trc *.dat *.trav *.);;All files (*.*)"));

       }

        if (!dest.isEmpty())
        {
            QList<int> pers, frame;
            bool autoCorrectOnlyExport = (mControlWidget->getRecoMethod() == 5) && // multicolor
                    mMultiColorMarkerWidget->autoCorrect->isChecked() &&
                    mMultiColorMarkerWidget->autoCorrectOnlyExport->isChecked();

//            debout << "dest: " << dest << " suffix: " << dest.right(4) << endl;

            if (dest.right(4) == ".trc")
            {
#ifdef TIME_MEASUREMENT
                double time1 = 0.0, tstart;
                tstart = clock();
#endif
                //QFile file(dest);
                QTemporaryFile file;
                int i;

                if (!file.open()/*!file.open(QIODevice::WriteOnly | QIODevice::Text)*/)
                {
                    QMessageBox::critical(this, tr("PeTrack"), tr("Cannot open %1:\n%2.").arg(dest).arg(file.errorString()));
                    return;
                }
                QProgressDialog progress("Export TRC-File",NULL,0,mTracker->size()+1,this->window());
                progress.setWindowTitle("Export .trc-File");
                progress.setWindowModality(Qt::WindowModal);
                progress.setVisible(true);
                progress.setValue(0);
                progress.setLabelText(QString("Export tracking data ..."));

                qApp->processEvents();

                //if (mControlWidget->exportComment->isChecked())
                    trcVersion = 4;
                //else
                //    trcVersion = 2;

                debout << "export tracking data to " << dest << " (" << mTracker->size() << " person(s), file version " << trcVersion << ")..." << std::endl;
                QTextStream out(&file);

                out << "version " << trcVersion << Qt::endl;
                out << mTracker->size() << Qt::endl;
                for (i = 0; i < mTracker->size(); ++i)
                {
                    qApp->processEvents();
                    progress.setLabelText(QString("Export person %1 of %2 ...").arg(i+1).arg(mTracker->size()));
                    progress.setValue(i+1);
                    //if(mControlWidget->exportComment->isChecked())
                    //    out << "# " << (*mTracker)[i].comment() << endl;
                    out << (*mTracker)[i] << Qt::endl;
                }
                file.flush();
                file.close();
#ifdef TIME_MEASUREMENT
                time1 += clock() - tstart;
                time1 = time1/CLOCKS_PER_SEC;
                cout << "  time(writing) = " << time1 << " sec." << endl;

                time1 = 0.0;
                tstart = clock();
#endif
                progress.setLabelText(QString("Save file ..."));
                qApp->processEvents();

                if (QFile::exists(dest))
                    QFile::remove(dest);

                if( !file.copy(dest) )
                    QMessageBox::critical(this, tr("PeTrack"),
                                                   tr("Could not export tracking data.\n"
                                                      "Please try again!"),
                                                   QMessageBox::Ok);
                else
                    statusBar()->showMessage(tr("Saved tracking data to %1.").arg(dest), 5000);

                // kurz Alternative zu oben aber schlechter lesbar
                //while(!file.copy(dest)) QFile::remove(dest);

                progress.setValue(mTracker->size()+1);

                std::cout << " finished " << std::endl;

#ifdef TIME_MEASUREMENT
                time1 += clock() - tstart;
                time1 = time1/CLOCKS_PER_SEC;
                cout << "  time(copying) = " << time1 << " sec." << endl;

//                time1 = 0.0;
//                tstart = clock();
#endif
//                mTracker->checkPlausibility(pers, frame,
//                                            mControlWidget->testEqual->isChecked(),
//                                            mControlWidget->testVelocity->isChecked(),
//                                            mControlWidget->testInside->isChecked(),
//                                            mControlWidget->testLength->isChecked());
#ifdef TIME_MEASUREMENT
//                time1 += clock() - tstart;
//                time1 = time1/CLOCKS_PER_SEC;
//                cout << "  time(checkPlausibility) = " << time1 << " sec." << endl;
#endif
                mTrcFileName = dest; // fuer Project-File, dann koennte track path direkt mitgeladen werden, wenn er noch da ist
            }
            else if (dest.right(4) == ".txt")
            {
                QTemporaryFile file;

                if (!file.open()) //!file.open(QIODevice::WriteOnly | QIODevice::Text))
                {
                    QMessageBox::critical(this, tr("PeTrack"), tr("Cannot open %1:\n%2.").arg(dest).arg(file.errorString()));
                    return;
                }

                debout << "export tracking data to " << dest << " (" << mTracker->size() << " person(s))..." << endl;

#ifdef TIME_MEASUREMENT
                double time1 = 0.0, tstart;
                tstart = clock();
#endif
                // recalcHeight true, wenn personenhoehe ueber trackpoints neu berechnet werden soll (z.b. um waehrend play mehrfachberuecksichtigung von punkten auszuschliessen, aenderungen in altitude neu in berechnung einfliessen zu lassen)
                if (mControlWidget->trackRecalcHeight->checkState())
                {
                    if ( mControlWidget->getCalibCoordDimension() == 0 ) // 3D
                        ;//Nothing to be done because z already the right height
                    else // 2D
                        mTracker->recalcHeight(mControlWidget->coordAltitude->value());
                }
#ifdef TIME_MEASUREMENT
                time1 += clock() - tstart;
                time1 = time1/CLOCKS_PER_SEC;
                cout << "  time(recalcHeight) = " << time1 << " sec." << endl;

                time1 = 0.0;
                tstart = clock();
#endif                  
                mTrackerReal->calculate(mTracker, mImageItem, mControlWidget->getColorPlot(), getImageBorderSize(),
                                        mControlWidget->trackMissingFrames->checkState(),
                                        mStereoWidget->stereoUseForExport->isChecked(),
                                        mControlWidget->trackAlternateHeight->checkState(), mControlWidget->coordAltitude->value(), mStereoWidget->stereoUseCalibrationCenter->isChecked(),
                                        mControlWidget->exportElimTp->isChecked(), mControlWidget->exportElimTrj->isChecked(), mControlWidget->exportSmooth->isChecked(),
                                        mControlWidget->exportViewDir->isChecked(), mControlWidget->exportAngleOfView->isChecked(), mControlWidget->exportMarkerID->isChecked(),
                                        autoCorrectOnlyExport);
#ifdef TIME_MEASUREMENT
                time1 += clock() - tstart;
                time1 = time1/CLOCKS_PER_SEC;
                cout << "  time(calculate) = " << time1 << " sec." << endl;

                time1 = 0.0;
                tstart = clock();
#endif

                QTextStream out(&file);

                out << "# PeTrack project: "     << QFileInfo(getProFileName()).fileName() << Qt::endl;
                out << "# raw trajectory file: " << QFileInfo(getTrackFileName()).fileName() << Qt::endl;
                out << "# framerate: "           << mAnimation->getFPS() << " fps" << Qt::endl;

                if (mControlWidget->exportComment->isChecked())
                {
                    out << "# personal information:" << Qt::endl;
                    out << "# ID| Comment" << Qt::endl;

                    // std out
                    std::cout << std::endl << "Printing comment table..." << std::endl << std::endl;
                    std::cout << "ID  | Comment" << std::endl;
                    std::cout << "----|----------------" << std::endl;

                    for(int i=0;i<mTracker->size();++i)
                    {
                        auto commentSplit = mTracker->at(i).comment().split("\n", Qt::KeepEmptyParts);
                        out << "#" << qSetFieldWidth(3) << (i+1) << qSetFieldWidth(0) << "|" << commentSplit.at(0) << Qt::endl;
                        std::cout  << setw(4) << (i+1) << "|" << commentSplit.at(0) << std::endl;

                        commentSplit.pop_front();
                        for (const auto& line : commentSplit)
                        {
                            out << "#" << qSetFieldWidth(3) << " " << qSetFieldWidth(0) << "|" << line << Qt::endl;
                            std::cout << "    |" << line << std::endl;
                        }
                    }
                }
                mTrackerReal->exportTxt(out,
                                        mControlWidget->trackAlternateHeight->checkState(),
                                        mStereoWidget->stereoUseForExport->isChecked(),
                                        mControlWidget->exportViewDir->isChecked(),
                                        mControlWidget->exportAngleOfView->isChecked(),
                                        mControlWidget->exportUseM->isChecked(),
                                        mControlWidget->exportMarkerID->isChecked());
                //out << *mTrackerReal;
                file.flush();
                file.close();

                if (QFile::exists(dest))
                    QFile::remove(dest);

                if( !file.copy(dest) )
                    QMessageBox::critical(this, tr("PeTrack"),
                                                   tr("Could not export tracking data.\n"
                                                      "Please try again!"),
                                                   QMessageBox::Ok);
                else
                    statusBar()->showMessage(tr("Saved tracking data to %1.").arg(dest), 5000);

                std::cout << " finished" << std::endl;

#ifdef TIME_MEASUREMENT
                time1 += clock() - tstart;
                time1 = time1/CLOCKS_PER_SEC;
                cout << "  time(export) = " << time1 << " sec." << endl;

//                time1 = 0.0;
//                tstart = clock();
#endif
//                mTracker->checkPlausibility(pers, frame,
//                                            mControlWidget->testEqual->isChecked(),
//                                            mControlWidget->testVelocity->isChecked(),
//                                            mControlWidget->testInside->isChecked(),
//                                            mControlWidget->testLength->isChecked());
#ifdef TIME_MEASUREMENT
//                time1 += clock() - tstart;
//                time1 = time1/CLOCKS_PER_SEC;
//                cout << "  time(checkPlausibility) = " << time1 << " sec." << endl;
#endif
            }
            else if (dest.right(4) == ".dat")
            {
                //                // war: immer wenn txt exportiert wird, wird auch fuer dat fuer excel exportiert
                //                QString destDat(dest.left(dest.length()-4)+".dat");
                //                QFile fileDat(destDat);
                //                if (!fileDat.open(QIODevice::WriteOnly | QIODevice::Text))
                //                {
                //                    QMessageBox::critical(this, tr("PeTrack"), tr("Cannot open %1:\n%2.").arg(destDat).arg(fileDat.errorString()));
                //                    return;
                //                }

                QTemporaryFile fileDat;

                if (!fileDat.open()) //!fileDat.open(QIODevice::WriteOnly | QIODevice::Text))
                {
                    QMessageBox::critical(this, tr("PeTrack"), tr("Cannot open %1:\n%2.").arg(dest).arg(fileDat.errorString()));
                    return;
                }
                // recalcHeight true, wenn personenhoehe ueber trackpoints neu berechnet werden soll (z.b. um waehrend play mehrfachberuecksichtigung von punkten auszuschliessen, aenderungen in altitude neu in berechnung einfliessen zu lassen)
                if (mControlWidget->trackRecalcHeight->checkState())
                {
                    mTracker->recalcHeight(mControlWidget->coordAltitude->value());
                }
                mTrackerReal->calculate(mTracker, mImageItem, mControlWidget->getColorPlot(), getImageBorderSize(),
                                        mControlWidget->trackMissingFrames->checkState(),
                                        mStereoWidget->stereoUseForExport->isChecked(),
                                        mControlWidget->trackAlternateHeight->checkState(), mControlWidget->coordAltitude->value(), mStereoWidget->stereoUseCalibrationCenter->isChecked(),
                                        mControlWidget->exportElimTp->isChecked(), mControlWidget->exportElimTrj->isChecked(), mControlWidget->exportSmooth->isChecked(),
                                        mControlWidget->exportViewDir->isChecked(), mControlWidget->exportAngleOfView->isChecked(), mControlWidget->exportMarkerID->isChecked(),
                                        autoCorrectOnlyExport);

                debout << "export tracking data to " << dest << " (" << mTracker->size() << " person(s))..." << std::endl;
                QTextStream outDat(&fileDat);
                mTrackerReal->exportDat(outDat, mControlWidget->trackAlternateHeight->checkState(), mStereoWidget->stereoUseForExport->isChecked());
                fileDat.flush();
                fileDat.close();

                if (QFile::exists(dest))
                    QFile::remove(dest);

                if( !fileDat.copy(dest) )
                    QMessageBox::critical(this, tr("PeTrack"),
                                                   tr("Could not export tracking data.\n"
                                                      "Please try again!"),
                                                   QMessageBox::Ok);
                else
                    statusBar()->showMessage(tr("Saved tracking data to %1.").arg(dest), 5000);

                std::cout << " finished" << std::endl;


//                mTracker->checkPlausibility(pers, frame,
//                                            mControlWidget->testEqual->isChecked(),
//                                            mControlWidget->testVelocity->isChecked(),
//                                            mControlWidget->testInside->isChecked(),
//                                            mControlWidget->testLength->isChecked());

                //
                //                // immer wenn txt exportiert wird, wird auch fuer visu-software xml nach ulrich kemlow exportiert
                //                QString destXml(dest.left(dest.length()-4)+".trav");
                //                QFile fileXml(destXml);
                //                if (!fileXml.open(QIODevice::WriteOnly | QIODevice::Text))
                //                {
                //                    QMessageBox::critical(this, tr("PeTrack"), tr("Cannot open %1:\n%2.").arg(destXml).arg(fileXml.errorString()));
                //                    return;
                //                }
                //                debout << "export tracking data to " << destXml << " (" << mTracker->size() << " person(s))..." << endl;
                //                // already done: mTrackerReal->calculate(mTracker, mImageItem, mControlWidget->getColorPlot(), getImageBorderSize(), mControlWidget->trackMissingFrames->checkState());
                //                QTextStream outXml(&fileXml);
                //                outXml << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" <<endl;
                //                outXml << "<trajectoriesDataset>" <<endl;
                //                outXml << "    <header>" <<endl;
                //                outXml << "        <roomCaption>PeTrack: " << mAnimation->getFileBase() << "</roomCaption>" << endl;
                //                outXml << "        <roomID>0</roomID>" <<endl;
                //                outXml << "        <agents>" << mTracker->size() << "</agents>" << endl;
                //                outXml << "        <frameRate>" << mAnimation->getFPS() << "</frameRate> <!--per second-->" << endl;
                //                // outXml << "        <timeStep>" << 1000./mAnimation->getFPS() << "</timeStep>   <!-- millisecond-->" << endl; inverse von
                //                outXml << "        <timeFirstFrame sec=\"" << mAnimation->getFirstFrameSec() << "\" microsec=\"" << mAnimation->getFirstFrameMicroSec()
                //                       << "\"/> <!-- " << mAnimation->getTimeString(0) << " -->" << endl;
                //                outXml << "    </header>" <<endl<<endl;
                //
                //                mTrackerReal->exportXml(outXml, mControlWidget->trackAlternateHeight->checkState(), mStereoWidget->stereoUseForExport->isChecked());
                //
                //                outXml << "</trajectoriesDataset>" <<endl;
                //                fileXml.close();
                //                cout << " finished" << endl;
            }
            else if (dest.right(5) == ".trav")
            {
                // recalcHeight true, wenn personenhoehe ueber trackpoints neu berechnet werden soll (z.b. um waehrend play mehrfachberuecksichtigung von punkten auszuschliessen, aenderungen in altitude neu in berechnung einfliessen zu lassen)
                if (mControlWidget->trackRecalcHeight->checkState())
                {
                    mTracker->recalcHeight(mControlWidget->coordAltitude->value());
                }

                mTrackerReal->calculate(mTracker, mImageItem, mControlWidget->getColorPlot(), getImageBorderSize(),
                                        mControlWidget->trackMissingFrames->checkState(),
                                        mStereoWidget->stereoUseForExport->isChecked(),
                                        mControlWidget->trackAlternateHeight->checkState(), mControlWidget->coordAltitude->value(), mStereoWidget->stereoUseCalibrationCenter->isChecked(),
                                        mControlWidget->exportElimTp->isChecked(), mControlWidget->exportElimTrj->isChecked(), mControlWidget->exportSmooth->isChecked(),
                                        mControlWidget->exportViewDir->isChecked(), mControlWidget->exportAngleOfView->isChecked(), mControlWidget->exportMarkerID->isChecked(),
                                        autoCorrectOnlyExport);
//                mTrackerReal->calculate(mTracker, mImageItem, mControlWidget->getColorPlot(), getImageBorderSize(),
//                                        mControlWidget->trackMissingFrames->checkState(),
//                                        mStereoWidget->stereoUseForExport->isChecked(),
//                                        mControlWidget->trackAlternateHeight->checkState(), mControlWidget->coordAltitude->value(), mStereoWidget->stereoUseCalibrationCenter->isChecked(),
//                                        mControlWidget->exportElimTp->isChecked(), mControlWidget->exportElimTrj->isChecked(), mControlWidget->exportSmooth->isChecked());

                QTemporaryFile fileXml;
                if (!fileXml.open()) //!fileXml.open(QIODevice::WriteOnly | QIODevice::Text))
                {
                    QMessageBox::critical(this, tr("PeTrack"), tr("Cannot open %1:\n%2.").arg(dest).arg(fileXml.errorString()));
                    return;
                }
                debout << "export tracking data to " << dest << " (" << mTracker->size() << " person(s))..." << std::endl;
                // already done: mTrackerReal->calculate(mTracker, mImageItem, mControlWidget->getColorPlot(), getImageBorderSize(), mControlWidget->trackMissingFrames->checkState());
                QTextStream outXml(&fileXml);
                outXml << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << Qt::endl;
                outXml << "<trajectoriesDataset>" << Qt::endl;
                outXml << "    <header version=\"1.0\">" << Qt::endl;
                outXml << "        <roomCaption>PeTrack: " << mAnimation->getFileBase() << "</roomCaption>" << Qt::endl;
                outXml << "        <roomID>0</roomID>" << Qt::endl;
                outXml << "        <agents>" << mTracker->size() << "</agents>" << Qt::endl;
                outXml << "        <frameRate>" << mAnimation->getFPS() << "</frameRate> <!--per second-->" << Qt::endl;
                // outXml << "        <timeStep>" << 1000./mAnimation->getFPS() << "</timeStep>   <!-- millisecond-->" << endl; inverse von
                outXml << "        <timeFirstFrame sec=\"" << mAnimation->getFirstFrameSec() << "\" microsec=\"" << mAnimation->getFirstFrameMicroSec()
                       << "\"/> <!-- " << mAnimation->getTimeString(0) << " -->" << Qt::endl;
                outXml << "    </header>" << Qt::endl<< Qt::endl;

                mTrackerReal->exportXml(outXml, mControlWidget->trackAlternateHeight->checkState(), mStereoWidget->stereoUseForExport->isChecked());

                outXml << "</trajectoriesDataset>" << Qt::endl;
                fileXml.flush();
                fileXml.close();

                if (QFile::exists(dest))
                    QFile::remove(dest);

                if( !fileXml.copy(dest) )
                    QMessageBox::critical(this, tr("PeTrack"),
                                                   tr("Could not export tracking data.\n"
                                                      "Please try again!"),
                                                   QMessageBox::Ok);
                else
                    statusBar()->showMessage(tr("Saved tracking data to %1.").arg(dest), 5000);

                cout << " finished" << endl;
            }
            else
            { // wenn keine Dateiendung, dann wird trc und txt herausgeschrieben
                exportTracker(dest + ".trc");
                exportTracker(dest + ".txt");
            }
            //             else
            //             {
            //                 QMessageBox::critical(this, tr("PeTrack"), tr("Cannot save %1 maybe because of wrong file extension.").arg(dest));
            //             }
            lastFile = dest;
        }
    }
}

// fuer anschliessende groessenberechnung
void Petrack::playAll()
{
    int memPos = mPlayerWidget->getPos();
    int progVal = 0;

    QProgressDialog progress("Playing whole sequence...", "Abort playing", 0, mAnimation->getNumFrames(), this);
    progress.setWindowModality(Qt::WindowModal); // blocks main window

    // vorwaertslaufen ab aktueller Stelle und trackOnlineCalc zum tracken nutzen
    do
    {
        progress.setValue(++progVal); //mPlayerWidget->getPos()
        qApp->processEvents();
        if (progress.wasCanceled())
            break;
    }
    while (mPlayerWidget->frameForward());

    mPlayerWidget->skipToFrame(memPos);
}

void Petrack::trackAll()
{
    int memPos = mPlayerWidget->getPos();
    int progVal = 0;
    enum Qt::CheckState memCheckState = mControlWidget->trackOnlineCalc->checkState();
    enum Qt::CheckState memRecoState = mControlWidget->performRecognition->checkState();

    mControlWidget->trackOnlineCalc->setCheckState(Qt::Checked);
    mControlWidget->performRecognition->setCheckState(Qt::Checked);

    QProgressDialog progress("Tracking pedestrians through all frames...", "Abort tracking", 0, 2*mAnimation->getNumFrames()-memPos, this);
    progress.setWindowModality(Qt::WindowModal); // blocks main window

    //    mTracker()->init(); // wenn vorherige Daten weggenommen werden sollen

    // mPlayerWidget->skipToFrame(0); //man koennte an den Anfang springen

    // vorwaertslaufen ab aktueller Stelle und trackOnlineCalc zum tracken nutzen
    do
    {
        progress.setValue(++progVal); //mPlayerWidget->getPos()
        qApp->processEvents();
        if (progress.wasCanceled())
            break;
    }
    while (mPlayerWidget->frameForward());

    if (mAutoBackTrack)
    {
        // zuruecksprinegn an die stelle, wo der letzte trackPath nicht vollstaendig
        // etwas spaeter, da erste punkte in reco path meist nur ellipse ohne markererkennung
        mControlWidget->trackOnlineCalc->setCheckState(Qt::Unchecked);
        mPlayerWidget->skipToFrame(mTracker->largestFirstFrame()+5);
        mControlWidget->trackOnlineCalc->setCheckState(Qt::Checked);
        //progVal = 2*mAnimation->getNumFrames()-memPos-mPlayerWidget->getPos();
        progVal += mAnimation->getNumFrames()-mPlayerWidget->getPos();
        progress.setValue(progVal); //mPlayerWidget->getPos()

        // recognition abstellen, bis an die stelle, wo trackAll begann
        // UEBERPRUEFEN, OB TRACKPATH NICHT RECOGNITION PUNKTE UEBERSCHREIBT!!!!!!!!!!
        // repeate und repaetQual koennte temporaer umgestellt werden
        mControlWidget->performRecognition->setCheckState(Qt::Unchecked);

        // rueckwaertslaufen
        do
        {
            if (progVal+1 < 2*mAnimation->getNumFrames()-memPos)
                progress.setValue(++progVal); //mPlayerWidget->getPos()
            qApp->processEvents();
            if (progress.wasCanceled())
                break;
            if (mPlayerWidget->getPos() == memPos+1)
                mControlWidget->performRecognition->setCheckState(Qt::Checked);
        }
        while (mPlayerWidget->frameBackward());

        // bei abbruch koennen es auch mPlayerWidget->getPos() frames sein, die bisher geschrieben wurden
        //         debout << "wrote " << mPlayerWidget->getPos()+1 << " of " << mAnimation->getNumFrames() << " frames." << endl;
        progress.setValue(2*mAnimation->getNumFrames()-memPos);
    }

    if (mAutoTrackOptimizeColor)
        mTracker->optimizeColor();

    mControlWidget->performRecognition->setCheckState(memRecoState);
    mControlWidget->trackOnlineCalc->setCheckState(Qt::Unchecked);
    mPlayerWidget->skipToFrame(memPos);
    mControlWidget->trackOnlineCalc->setCheckState(memCheckState);
}

// default: (QPointF *pos=NULL, int pers=-1, int frame=-1);
int Petrack::winSize(QPointF *pos, int pers, int frame, int level)
{
    // default of mControlWidget->trackRegionScale->value() is 16, so that
    // a factor of 1.6 of the headsize is used
    if (level == -1)
        level = mControlWidget->trackRegionLevels->value();
    return (int)((getHeadSize(pos, pers, frame) / pow(2.,level)) * (mControlWidget->trackRegionScale->value() / 10.));
}

void Petrack::updateImage(bool imageChanged) // default = false (only true for new animation frame)
{

#ifdef TIME_MEASUREMENT
    // die reine Ausgabe  folgender Zeile kostet 1-2 Millisekunden
    //        "==========: "
    debout << "go  update: " << getElapsedTime() <<endl;
#endif

    static int lastRecoFrame = -10000;
    static bool borderChangedForTracking = false;

    //static QTime updateTime; // ergibt die gleichen werte wie benutzte native systemroutinben!!!

    // need semaphore to guarrantee that updateImage only called once
    // updateValue of control automatically calls updateImage!!!
    static QSemaphore semaphore(1);
    if (!mImg.empty() && mImage && semaphore.tryAcquire())
    {

        int frameNum = mAnimation->getCurrentFrameNum();

        setStatusTime();

        updateShowFPS();

        mImgFiltered = mImg;

        // have to store because evaluation sets the filter parameter to unchanged
        bool brightContrastChanged = mBrightContrastFilter.changed();
        bool swapChanged = mSwapFilter.changed();
        //         bool brightChanged = mBrightFilter.changed();
        //         bool contrastChanged = mContrastFilter.changed();
        bool borderChanged = mBorderFilter.changed();
        bool calibChanged = mCalibFilter->changed();

        // speicherverwaltung wird komplett von filtern ueberneommen

        // Filter anwenden, Reihenfolge wichtig - Rechenintensive moeglichst frueh
        // fkt so nur mit kopierenden filtern
#ifdef TIME_MEASUREMENT
    //        "==========: "
    debout << "vor filter: " << getElapsedTime() <<endl;
#endif

//        debout << "swap:    " << mIplImgFiltered << " " << mIplImg << endl;
        if (imageChanged || swapChanged)
            mImgFiltered = mSwapFilter.apply(mImgFiltered);
        else
            mImgFiltered = mSwapFilter.getLastResult();

//        debout << "b-c:     " << mIplImgFiltered << " " << mIplImg << endl;
        if (imageChanged || swapChanged || brightContrastChanged)
            mImgFiltered = mBrightContrastFilter.apply(mImgFiltered);
        else
            mImgFiltered = mBrightContrastFilter.getLastResult();

//        debout << "border:  " << mIplImgFiltered << " " << mIplImg << endl;
         if (imageChanged || swapChanged || brightContrastChanged || borderChanged)
            mImgFiltered = mBorderFilter.apply(mImgFiltered); // mIplImg
        else
            mImgFiltered = mBorderFilter.getLastResult();

#ifdef TIME_MEASUREMENT
    //        "==========: "
    debout << "nch filter: " << getElapsedTime() <<endl;
#endif
        if (borderChanged)
            updateControlImage(mImgFiltered);
#ifndef STEREO_DISABLED
        if (imageChanged || swapChanged || brightContrastChanged || borderChanged || calibChanged)
        {
            if (mStereoContext)
                mStereoContext->init(mImgFiltered);
        }
#endif

#ifdef TIME_MEASUREMENT
    //        "==========: "
    debout << "vor  calib: " << getElapsedTime() <<endl;
#endif
        if (imageChanged || swapChanged || brightContrastChanged || borderChanged || calibChanged)
            mImgFiltered = mCalibFilter->apply(mImgFiltered);
        else
            mImgFiltered = mCalibFilter->getLastResult();

#ifdef TIME_MEASUREMENT
    //        "==========: "
    debout << "nach calib: " << getElapsedTime() <<endl;
#endif

        if (brightContrastChanged || swapChanged || borderChanged || calibChanged)
        {
            // abfrage hinzugenommen, damit beim laden von .pet bg-file angegeben werden kann fuer mehrere versuche und beim nachladen von versuch nicht bg geloescht wird
            if (mBackgroundFilter.getFilename() != "")
                debout << "Warning: No background reset, because of explicit loaded background image!" <<endl;
            else
                mBackgroundFilter.reset(); // alle gesammelten hintergrundinfos werden verworfen und bg.changed auf true gesetzt
        }

//        debout << "bg:      " << mIplImgFiltered << " " << mIplImg << endl;
        if (imageChanged || mBackgroundFilter.changed())
            mImgFiltered = mBackgroundFilter.apply(mImgFiltered);
        else
            mImgFiltered = mBackgroundFilter.getLastResult();

#ifdef TIME_MEASUREMENT
    //        "==========: "
    debout << "nach    bg: " << getElapsedTime() <<endl;
#endif

        // delete track list, if intrinsic param have changed
        if (calibChanged && mTracker->size() > 0) //mCalibFilter.getEnabled() &&
        {
            //CvSize size;
            //size.width = mIplImgFiltered->width;
            //size.height = mIplImgFiltered->height;
            //mTracker->init(size);

            // Evtl. nicht Tracker loeschen sondern entsprechend der neuen Calibration verschieben?!?!?
            mTracker->clear();
            mTracker->reset();
            if( !isLoading() )
                debout << "Warning: deleted all tracking pathes because intrinsic parameters have changed." << endl;
        }
        else
        {
#ifndef STEREO_DISABLED
            // calculate position in 3D space and height of person for "old" trackPoints, if checked "even"
            if (mStereoContext && mStereoWidget->stereoUseForHeightEver->isChecked() && mStereoWidget->stereoUseForHeight->isChecked())
            {
                // buildt disparity picture if it should be used for height detection
                mStereoContext->getDisparity();

                mTracker->calcPosition(frameNum);
            }
#endif
        }
        if (borderChanged)
            borderChangedForTracking = true;
        // tracking vor recognition, da dann neu gefundene punkte mit getrackten bereits ueberprueft werden koennen
        if ((trackChanged() || imageChanged)  && (mControlWidget->trackOnlineCalc->checkState() == Qt::Checked)) // borderChanged ???
        {
            // Rect for tracking area
            QRect roi(myRound(mTrackingRoiItem->rect().x()+getImageBorderSize()),
                       myRound(mTrackingRoiItem->rect().y()+getImageBorderSize()),
                       myRound(mTrackingRoiItem->rect().width()),
                       myRound(mTrackingRoiItem->rect().height()));

//            mTracker->resize(Size(roi.width(),roi.height()));

            if (borderChangedForTracking)
            {
                Size size;
                size.width = mImgFiltered.cols;
                size.height = mImgFiltered.rows;
                mTracker->resize(size);

                mTrackingRoiItem->checkRect();
            }
#ifndef STEREO_DISABLED
            // buildt disparity picture if it should be used for height detection
            if (mStereoContext && mStereoWidget->stereoUseForHeight->isChecked())
                mStereoContext->getDisparity();
#endif

            Rect rect;
            getRoi(mImgFiltered, roi, rect);
            // Ignore all tracking points outside of rect

            //if (mPrevIplImgFiltered) // wenn ein vorheriges bild vorliegt
            // mPrevIplImgFiltered == NULL zeigt an, dass neue bildfolge && mPrevFrame == -1 ebenso
            // winSize(), wurde mal uebergeben
#ifdef TIME_MEASUREMENT
            debout << "vor  track: " << getElapsedTime() <<endl;
#endif
//            debout << "test" << endl;
            int anz = mTracker->track(mImgFiltered, rect, frameNum,
                                      mControlWidget->trackRepeat->isChecked(),
                                      mControlWidget->trackRepeatQual->value(), getImageBorderSize(),
                                      mControlWidget->trackRegionLevels->value(), getOnlyVisible());
#ifdef TIME_MEASUREMENT
            debout << "nach track: " << getElapsedTime() <<endl;
#endif
//            debout << "track anz: " << anz << endl;
            mControlWidget->trackNumberNow->setText(QString("%1").arg(anz));
            mTrackChanged = false;
            borderChangedForTracking = false;
        }
        else
            mControlWidget->trackNumberNow->setText(QString("0"));
        // hier muesste fuer ameisen etc allgemeinABC.getPosList(...)
        //#static IplImage *tempImg = NULL;

//        debout << "reco:    " << mIplImgFiltered << " " << mIplImg << endl;
//        debout << "imageChanged: " << imageChanged << " swapChanged: " << swapChanged << " b-c-Changed: " << brightContrastChanged << " borderChanged: "<< borderChanged << " calibChanged: " << calibChanged << " recognitionChanged: " << recognitionChanged() << endl;
        if (((((lastRecoFrame+mControlWidget->recoStep->value()) <= frameNum) ||
              ((lastRecoFrame-mControlWidget->recoStep->value()) >= frameNum)) &&
             imageChanged) || mAnimation->isCameraLiveStream() || swapChanged || brightContrastChanged || borderChanged || calibChanged || recognitionChanged())
        {
#ifndef STEREO_DISABLED
            // buildt disparity picture if it should be used for height detection or recognition
            if (mStereoContext && (mStereoWidget->stereoUseForHeight->isChecked() || mStereoWidget->stereoUseForReco->isChecked()))
                mStereoContext->getDisparity(); // wird nicht neu berechnet, wenn vor tracking schon berechnet wurde
#endif
            if (borderChanged)
                mRecognitionRoiItem->checkRect();
            //#cvReleaseImage(&tempImg);
            //#tempImg = cvCloneImage(mIplImgFiltered); // must be deleted, if marker in ebene darueber gemalt werden!!!!!
            //             QPointF p = mRecognitionRoiItem->mapToItem(mImageItem, QPointF(0.,0.)); nicht noetig da drag nun selber gemacht wird
//            debout << "start reco" << endl;
            if (mControlWidget->performRecognition->checkState() == Qt::Checked)
            {
                QRect rect(myRound(mRecognitionRoiItem->rect().x()+getImageBorderSize()),
                           myRound(mRecognitionRoiItem->rect().y()+getImageBorderSize()),
                           myRound(mRecognitionRoiItem->rect().width()),
                           myRound(mRecognitionRoiItem->rect().height()));
                QList<TrackPoint> persList;
//                bool markerLess = true;
                int recoMethod = mControlWidget->getRecoMethod();
#ifdef TIME_MEASUREMENT
                //        "==========: "
                debout << "vor   reco: " << getElapsedTime() <<endl;
#endif
                if ((recoMethod == 0) || (recoMethod == 1) || (recoMethod == 3) || (recoMethod == 4) || (recoMethod == 5) || (recoMethod == 6)) //else
                { // 0 == Kaserne, 1 == Hermes, 2 == Ohne, 3 == Color, 4 == Japan, 5 == MultiColor, 6 == CodeMarker
                    // persList == crossList
                    //                    getMarkerPos(mIplImgFiltered, rect, &persList, mControlWidget->markerBrightness->value(),
                    //                                getImageBorderSize(), (mControlWidget->markerIgnoreWithout->checkState() == Qt::Checked),
                    //                                (mControlWidget->recoAutoWB->checkState() == Qt::Checked), getBackgroundFilter(), recoMethod); //#tempImg
                    getMarkerPos(mImgFiltered, rect, &persList, mControlWidget, getImageBorderSize(), getBackgroundFilter());
//                    markerLess = false;
//                    debout << "Testausgabe persList: [Frame " << frameNum << "] " << endl;
//                    for (int i = 0; i < persList.size(); ++i)
//                    {
//                        cout << persList.at(i) << endl;
//                    }
                }
#ifndef STEREO_DISABLED
                if (mStereoContext && mStereoWidget->stereoUseForReco->isChecked())
                {
                    PersonList pl;
                    pl.calcPersonPos(mImgFiltered, rect, persList, mStereoContext, getBackgroundFilter(), markerLess);
                }
#endif
#ifdef TIME_MEASUREMENT
                //        "==========: "
                debout << "nach  reco: " << getElapsedTime() <<endl;
#endif
                mTracker->addPoints(persList, frameNum);
//                debout << "reco anz: " << persList.size() << endl;
                // folgendes lieber im Anschluss, ggf beim exportieren oder statt test direkt del:
                if (mStereoContext && mStereoWidget->stereoUseForReco->isChecked())
                    mTracker->purge(frameNum); // bereinigen wenn weniger als 0.2 recognition und nur getrackt

                mControlWidget->recoNumberNow->setText(QString("%1").arg(persList.size()));
                mRecognitionChanged = false;
                //     QString s("#%1%2%3"); // static moeglich?
                //     s = s.arg(qRed(col), 2, 16, QChar('0')).arg(qGreen(col), 2, 16, QChar('0')).arg(qBlue(col), 2, 16, QChar('0'));
                //     if ((qRed(col)+qGreen(col)+qBlue(col))/3 < 128)
                //         mStatusLabelColor->setText(QString("<font color=\"#ffffff\">&nbsp;%1</font>").arg(s));
                //     else
                //         mStatusLabelColor->setText(QString("<font color=\"#000000\">&nbsp;%1</font>").arg(s));

                //                 mControlWidget->getColorPlot()->updateTracker(); // oder nur wenn tab offen oder wenn sich mtracker geaendert hat???
                if (false) // hier muss Abfage hin ob kasernen marker genutzt wird
                    mControlWidget->getColorPlot()->replot(); // oder nur wenn tab offen oder wenn sich mtracker geaendert hat???
            }
            else
                mControlWidget->recoNumberNow->setText(QString("0"));
            lastRecoFrame = frameNum;
//            debout << "end reco" << endl;

        }
        else
            mControlWidget->recoNumberNow->setText(QString("0"));

        mControlWidget->trackNumberAll->setText(QString("%1").arg(mTracker->size())); // kann sich durch reco und tracker aendern
        mControlWidget->trackShowOnlyNr->setMaximum(MAX(mTracker->size(),1)); // kann sich durch reco und tracker aendern
        mControlWidget->trackNumberVisible->setText(QString("%1").arg(mTracker->visible(frameNum))); // kann sich durch reco und tracker aendern

        // in anzuzeigendes Bild kopieren
        // erst hier wird die bildgroesse von mimage an filteredimg mit border angepasst
        copyToQImage(*mImage, mImgFiltered);

        if (borderChanged)
            mImageItem->setImage(mImage);
        else
        {
            getScene()->update(); //repaint();
            // update pixel color (because image pixel moves)
            setStatusColor();
        }

#ifdef QWT
        mControlWidget->getAnalysePlot()->setActFrame(frameNum);
        if (mControlWidget->anaMarkAct->isChecked())
        {
            mControlWidget->getAnalysePlot()->replot();
        }
#endif

        semaphore.release();
    }
#ifdef TIME_MEASUREMENT
    //        "==========: "
    debout << "stp update: " << getElapsedTime() <<endl;
#endif
//    debout << " ############ out ############## " << mIplImgFiltered << " " << mIplImg << endl;

//    debout << "mIplImage ok ? " << (mIplImg != NULL) << " +++ mIplImageFiltered ok ? " << (mIplImgFiltered != NULL) << endl;
}
void Petrack::updateImage(const Mat &img)
{
    mImg = img;
//    namedWindow("Test");
//    imshow("Test",mImg);
//    waitKey();

    updateImage(true);
}

//void Petrack::updateImage(IplImage *iplImg)
//{
//    //     static int prevFrame = -1; // only 1 instanze allowed!!!!!!

//    //if (iplImg != mIplImg)
//    //{
//    //cvReleaseImage(&mIplImg); wird nun in animation gemacht
//    mIplImg = iplImg;
//    //}

//    //     //if ( online calculation || calc all)
//    //     if (mIplImgFiltered) // != if mIplImgFiltered != NULL a picture was shown before
//    //     {
//    //         if (mPrevIplImgFiltered)
//    //             cvReleaseImage(&mPrevIplImgFiltered);
//    //         mPrevIplImgFiltered = cvCreateImage(cvGetSize(mIplImgFiltered),8,3);
//    //     }
//    //     mPrevFrame = prevFrame;
//    //     prevFrame = mAnimation->getCurrentFrameNum();
////    debout << "test" << endl;
//    updateImage(true);
////    debout << "test" << endl;

//}

void Petrack::updateSequence()
{
    QImage *oldImage = mImage;

    //cvReleaseImage(&mIplImg); darf niiiiiie gemacht werden - ewig nach fehler gesucht - kommt bei filern durcheinander!!!
    QSize size = mAnimation->getSize();
    size.setWidth(size.width()+2*getImageBorderSize());// border is inside the mImage!
    size.setHeight(size.height()+2*getImageBorderSize());
    mImage = new QImage(size, QImage::Format_RGB888);//32); //wird in updateImage gemacht



    //     cvReleaseImage(&mPrevIplImgFiltered); // vorheriges bild ; NULL zeigt an, dass neue bildfolge
    //     mPrevFrame = -1;

    // set roi for recognition if image size changes or roi is zero
    //in oldImage steckt border drin, mIplImg->height zeigt noch auf altes ursprungsbild
    // mRecognitionRoiItem->rect().width() != 0 && oldImage == NULL wenn projektdatei eingelesen wird!!!!!
    if ((mRecognitionRoiItem->rect().width() == 0 ) || // default while initialization, after that >= MIN_SIZE
            (oldImage && ((oldImage->width() != mImage->width()) || (oldImage->height() != mImage->height()))))
        mRecognitionRoiItem->setRect(-getImageBorderSize(), -getImageBorderSize(), mImage->width(), mImage->height());
    if ((mTrackingRoiItem->rect().width() == 0 ) ||
            (oldImage && ((oldImage->width() != mImage->width()) || (oldImage->height() != mImage->height()))))
        mTrackingRoiItem->setRect(-getImageBorderSize(), -getImageBorderSize(), mImage->width(), mImage->height());

    Size size2;
    size2.width = mTrackingRoiItem->rect().width();
    size2.height = mTrackingRoiItem->rect().height();
    mTracker->init(size2);

    mPlayerWidget->setAnim(mAnimation);
//    debout << "test" << endl;
    mPlayerWidget->skipToFrame(0);
//    debout << "test" << endl;
    mImageItem->setImage(mImage);//wird in updateImage gemacht
//    debout << "test" << endl;
    delete oldImage;
//  debout << "test" << endl;
    mSaveSeqVidAct->setEnabled(true);
    mSaveSeqVidViewAct->setEnabled(true);
    mSaveSeqImgAct->setEnabled(true);
    mSaveSeqViewAct->setEnabled(true);
    mSaveImageAct->setEnabled(true);
    mSaveViewAct->setEnabled(true);
    mPrintAct->setEnabled(true);
    mResetSettingsAct->setEnabled(true);
}


//// calculate background for backgroud subtraction
//// WIRD NICHT MEHR BENOETIGT, DA HINTERGRUNDMODELL IM BETRIEB AKTUALISIERT WIRD!!!
//void Petrack::calcBackground()
//{
//    int memPos = mPlayerWidget->getPos();
//    if (memPos != -1) // video loaded
//    {
//        int progVal = 0;
//        int from = mControlWidget->filterBgFrom->value(); // from - frame where background begins
//        int num  = mControlWidget->filterBgNum->value(); // num  - frame number for averaging because of stabilization

//        if (from < mAnimation->getNumFrames())
//        {
//            QProgressDialog progress("Calculating background...", "Abort calculation", 0, num, this);
//            progress.setWindowModality(Qt::WindowModal); // blocks main window
//            mPlayerWidget->skipToFrame(from);

//            cvNamedWindow("BG", 1);
//            cvNamedWindow("FG", 1);
//            CvBGStatModel* bg_model = cvCreateGaussianBGModel(getIplImageFiltered());
//            //CvBGStatModel* bg_model = cvCreateFGDStatModel(getIplImageFiltered());

//            while ((++progVal < num) && (mPlayerWidget->frameForward()))
//            {
//                cvUpdateBGStatModel(getIplImageFiltered(), bg_model);
//                cvShowImage("BG", bg_model->background);
//                cvShowImage("FG", bg_model->foreground);

//                progress.setValue(progVal);
//                qApp->processEvents();
//                if (progress.wasCanceled())
//                    break;
//            }

//            cvReleaseBGStatModel(&bg_model);

//            mPlayerWidget->skipToFrame(memPos); // ruecksprung zur urspruenglichen Position
//        }
//        else
//        {
//            QMessageBox::critical(this, tr("PeTrack"), tr("from has to be smaller than %1.").arg(mAnimation->getNumFrames()));
//            return;
//        }
//    }
//}

// gibt cm pro pixel zurueck
// wert wir nur neu bestimmt, wenn auch kopfgroesse berechnet wird
double Petrack::getCmPerPixel()
{
    return mCmPerPixel;
}
// die Groesse des kreises des durchschnittlichen Kopfdurchmessers, der ganzen kopf umfasst in Pixel
// annahmen: 21cm avg kopflaenge, mapDefaultheight genommen statt: 173cm avg koerpergroesse mann / frau mit Schuhen (180cm waere nur Mann)
// hS ==-1 als default besagt, dass mHeadSize neu berechnet statt gesetzt werden soll
// WENN HEADSIZE NEU BERECHNET WIRD WIRD AUTOMATISCH AUCH CMPERPIXEL MITBERECHNET
void Petrack::setHeadSize(double hS)
{
    if (hS == -1)
    {
        mCmPerPixel = getImageItem()->getCmPerPixel();
        //debout << mCmPerPixel <<endl;
        mHeadSize = (HEAD_SIZE*mControlWidget->coordAltitude->value() / (mControlWidget->coordAltitude->value()-mControlWidget->mapDefaultHeight->value())) /
                mCmPerPixel;
    }
    else
        mHeadSize = hS;
}
// gibt Kopfgroesse zurueck
// default: (QPointF *pos=NULL, int pers=-1, int frame=-1)
double Petrack::getHeadSize(QPointF *pos, int pers, int frame)
{
    double z, h;

    if ((pers >= 0) && (pers < mTracker->size()) && mTracker->at(pers).trackPointExist(frame))
    {
        if( mControlWidget->getCalibCoordDimension() == 0 )
        {

            //debout << "getHeadSize: " << pers << " " << frame << endl;
            //debout << mTracker->at(pers).trackPointAt(frame) << endl;

//            debout << "mTracker->at(pers).height(): " << mTracker->at(pers).height() << endl;
//            debout << "mControlWidget->mapDefaultHeight->value(): " << mControlWidget->mapDefaultHeight->value() << endl;
//            debout << "mControlWidget->mapHeight->value(): " << mControlWidget->mapHeight->value() << endl;
            int diff;
            Point3f p3d = getExtrCalibration()->get3DPoint(Point2f(mTracker->at(pers).trackPointAt(frame).x(),
                                                            mTracker->at(pers).trackPointAt(frame).y()),
                                                           mControlWidget->mapDefaultHeight->value());
                                                           //mTracker->at(pers).height()); // mStatusPosRealHeight->value());

//            debout << "Track position: (" << mTracker->at(pers).trackPointAt(frame).x() << ", " << mTracker->at(pers).trackPointAt(frame).y() << ")" << endl;
//            debout << "P3D: (" << p3d.x << ", " << p3d.y << ", " << p3d.z << ")" << endl;

            //debout << "3D Punkt: x: " << p3d.x << ", y: " << p3d.y << ", z: " << p3d.z << endl;

            Point2f p3d_x1 = getExtrCalibration()->getImagePoint( Point3f(p3d.x+HEAD_SIZE*0.5, p3d.y, p3d.z) );
            Point2f p3d_x2 = getExtrCalibration()->getImagePoint( Point3f(p3d.x-HEAD_SIZE*0.5, p3d.y, p3d.z) );
            Point2f p3d_y1 = getExtrCalibration()->getImagePoint( Point3f(p3d.x, p3d.y+HEAD_SIZE*0.5, p3d.z) );
            Point2f p3d_y2 = getExtrCalibration()->getImagePoint( Point3f(p3d.x, p3d.y-HEAD_SIZE*0.5, p3d.z) );

            diff = (int) max(sqrt(pow(p3d_x2.x-p3d_x1.x,2)+pow(p3d_x2.y-p3d_x1.y,2)),
                           sqrt(pow(p3d_y2.x-p3d_y1.x,2)+pow(p3d_y2.y-p3d_y1.y,2)));
//                    max(abs(getExtrCalibration()->getImagePoint(Point3f(p3d.x+HEAD_SIZE*0.5, p3d.y,               p3d.z)).x-
//                                 getExtrCalibration()->getImagePoint(Point3f(p3d.x-HEAD_SIZE*0.5, p3d.y,               p3d.z)).x)
//                                ,abs(getExtrCalibration()->getImagePoint(Point3f(p3d.x,           p3d.y+HEAD_SIZE*0.5, p3d.z)).y-
//                                 getExtrCalibration()->getImagePoint(Point3f(p3d.x,               p3d.y-HEAD_SIZE*0.5, p3d.z)).y));
//            debout << "HeadSize: " << diff << endl;
            return diff;// < 8 ? 8 : diff;
        }else
        {
            z = mTracker->at(pers).trackPointAt(frame).sp().z();
            h = mTracker->at(pers).height();
            if (z > 0)
                return (HEAD_SIZE*mControlWidget->coordAltitude->value() / z) / getImageItem()->getCmPerPixel();
            else if (h > MIN_HEIGHT)
                return (HEAD_SIZE*mControlWidget->coordAltitude->value() / (mControlWidget->coordAltitude->value()-h)) / getImageItem()->getCmPerPixel();
            else
                return mHeadSize;
        }
    }

    if (pos != NULL)
        return mHeadSize; // muss noch aus density map gelesen werden!!!
    else //(pos == NULL) && (pers == -1)
        return mHeadSize;

}

/**
 * @brief Petrack::getOnlyVisible Returns trajectories which should be evaluated
 *
 * If "only for visible people" is checked, then only people visible via
 * "show only people" (single person) or "show only people list"(multiple persons)
 * are going to be evaluated. If "only for visible people" is not checked,
 * everyone gets evaluated, not only the ones selected for visibility.
 *
 * @return all trajectories which should be evaluated; empty when all should be evaluated
 */
QSet<int> Petrack::getOnlyVisible()
{
    if  ((mControlWidget->trackOnlyVisible->checkState() == Qt::Checked) && (mControlWidget->trackShowOnly->checkState() == Qt::Checked || mControlWidget->trackShowOnlyList->checkState() == Qt::Checked))
    {
//        int maxPed = mTracker->size();
        if( mControlWidget->trackShowOnlyList->checkState() == Qt::Checked)
        {
            QStringList list = mControlWidget->trackShowOnlyNrList->text().split(",", Qt::SkipEmptyParts);
            QSet<int> onlyVisible;
            foreach(QString s, list)
            {
                bool ok = false;
                int nr = s.toInt(&ok);
                if(ok/* && nr <= maxPed && nr > 0*/) // einzelne ID
                    onlyVisible.insert(nr-1);
                else // error or IDs range (e.g. 1-3, 6-10, etc.)
                {
                    QStringList range = s.split("-", Qt::SkipEmptyParts);
                    int last,first = range[0].toInt(&ok);

                    if(ok/* && first <= maxPed && nr > 0*/)
                    {
                        last = range[1].toInt(&ok);
                        if(ok/* && last <= maxPed && nr > 0*/)
                        {
                            if(first>last)
                                std::swap(first,last);

                            for(int i=first;i<=last;i++)
                                onlyVisible.insert(i-1);
                        }
                    }

                }
                if(!ok)
                    debout << "Warning: error while reading showOnlyVisible list from input line!" << endl;
            }
            return onlyVisible;//QSet<int>() << mControlWidget->trackShowOnlyNr->text().split(",").toSet(); //in anzeige wird ab 1 gezaehlt, in datenstruktur ab 0

        }else // if(ControlWidget->trackShowOnly->checkState() == Qt::Checked) //
        {
            QSet<int> onlyVisible;
            onlyVisible.insert(mControlWidget->trackShowOnlyNr->value()-1);
            return onlyVisible;
        }
    }else
        return QSet<int>();
}

void Petrack::addManualTrackPointOnlyVisible(QPointF pos)
{
    int pers = addManualTrackPoint(pos)+1;
    if (pers == 0)
        pers = mTracker->size()+1;
    pers = mControlWidget->trackShowOnlyNr->maximum();
    mControlWidget->trackShowOnlyNr->setValue(pers);
//    mControlWidget->trackShowOnlyNr->setText(QString::number(pers));
    mControlWidget->trackShowOnly->setChecked(true);
}

void Petrack::updateControlWidget()
{
    mControlWidget->trackNumberAll->setText(QString("%1").arg(mTracker->size()));
    mControlWidget->trackShowOnlyNr->setMaximum(MAX(mTracker->size(),1));
    mControlWidget->trackNumberVisible->setText(QString("%1").arg(mTracker->visible(mAnimation->getCurrentFrameNum())));
}

//void Petrack::showContextMenu(QPointF pos)
//{
//    QMenu menu(this);
//    menu.addAction(mDelPastAct);
//    menu.addAction(mDelFutureAct);
//    menu.addAction(mDelAllRoiAct);
//    menu.exec(pos.toPoint());
//}

void Petrack::splitTrackPerson(QPointF pos)
{
    mTracker->splitPersonAt((Vec2F) pos, mAnimation->getCurrentFrameNum(), getOnlyVisible());
    updateControlWidget();
}

int Petrack::addManualTrackPoint(QPointF pos) //const QPoint &pos
{
    int pers = -1;
    TrackPoint tP((Vec2F) pos, 110); // 110 ist ueber 100 (hoechste Qualitaetsstufe) und wird nach einfuegen auf 100 gesetzt
    // so kann aber ein punkt immer angepasst werden
    mTracker->addPoint(tP, mAnimation->getCurrentFrameNum(), getOnlyVisible(), &pers);
    updateControlWidget();
    return pers;
}

// direction zeigt an, ob bis zum aktuellen (-1), ab dem aktuellen (1) oder ganzer trackpath (0)
// loeschen von Trackpoints einer Trajektorie
void Petrack::deleteTrackPoint(QPointF pos, int direction) //const QPoint &pos
{
    mTracker->delPoint((Vec2F) pos, direction, mAnimation->getCurrentFrameNum(), getOnlyVisible());
    updateControlWidget();
}
void Petrack::editTrackPersonComment(QPointF pos)
{

    mTracker->editTrackPersonComment((Vec2F) pos, mAnimation->getCurrentFrameNum(), getOnlyVisible());
    updateControlWidget();
}
void Petrack::setTrackPersonHeight(QPointF pos)
{
    mTracker->setTrackPersonHeight((Vec2F) pos, mAnimation->getCurrentFrameNum(), getOnlyVisible());
    updateControlWidget();
}
void Petrack::resetTrackPersonHeight(QPointF pos)
{
    mTracker->resetTrackPersonHeight((Vec2F) pos, mAnimation->getCurrentFrameNum(), getOnlyVisible());
    updateControlWidget();
}

// direction zeigt an, ob bis zum aktuellen (-1), ab dem aktuellen (1) oder ganzer trackpath (0)
// loeschen von Trackpoints aller Trajektorien
void Petrack::deleteTrackPointAll(int direction) //const QPoint &pos
{
    mTracker->delPointAll(direction, mAnimation->getCurrentFrameNum());
    updateControlWidget();
}

void Petrack::deleteTrackPointROI()
{
    mTracker->delPointROI();
    updateControlWidget();
    mScene->update();
}

void Petrack::deleteTrackPointInsideROI()
{
    mTracker->delPointInsideROI();
    updateControlWidget();
    mScene->update();
}
void Petrack::updateSourceInOutFrames()
{
    mPlayerWidget->setFrameInNum(mAnimation->getSourceInFrameNum());
    mPlayerWidget->setFrameOutNum(mAnimation->getSourceOutFrameNum());    
}

// delta gibt menge an Umdrehungen und richtung an
void Petrack::skipToFrameWheel(int delta)
{
    mPlayerWidget->skipToFrame(mPlayerWidget->getPos()+delta);
}

#include "moc_petrack.cpp"
