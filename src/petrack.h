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

#ifndef PETRACK_H
#define PETRACK_H

#include <QDomDocument>
#include <QKeyEvent>
#include <QMainWindow>
#include <QMouseEvent>
#include <opencv2/opencv.hpp>
#include <optional>

#ifdef STEREO
#include "calibStereoFilter.h"
#include "stereoContext.h"
#endif
#include "animation.h"
#include "annotationGroupManager.h"
#include "autoCalib.h"
#include "autosave.h"
#include "backgroundFilter.h"
#include "borderFilter.h"
#include "brightContrastFilter.h"
#include "calibFilter.h"
#include "extrCalibration.h"
#include "logwindow.h"
#include "manualTrackpointMover.h"
#include "moCapController.h"
#include "moCapPerson.h"
#include "personStorage.h"
#include "swapFilter.h"
#include "trackerReal.h"

class RoiItem;
class RecognitionRoiItem;
class GridItem;
class WorldImageCorrespondence;
class AnnotationGroupWidget;


// durchschnittliche Kopflaenge in cm (Kopf 21x14)
inline constexpr double HEAD_SIZE = 21.;

inline constexpr int AXIS_MARKERS_LENGTH = 10;

class QAction;
class QActionGroup;
class QLabel;
class QMenu;
class QHBoxLayout;
class QImage;
class QString;
class GraphicsView;
class QGraphicsScene;
class QXmlStreamWriter;
class QSplitter;
class QDoubleSpinBox;
class QFrame;

class CoordItem;
class ImageItem;
class LogoItem;
class Control;
class StereoWidget;
class ColorRangeWidget;
class ColorMarkerWidget;
class CodeMarkerWidget;
class MultiColorMarkerWidget;
class YOLOMarkerWidget;
class ViewWidget;
class LogWindow;
class Player;
class TrackerItem;
class StereoItem;
class ColorMarkerItem;
class CodeMarkerItem;
class MultiColorMarkerItem;
class BackgroundItem;
class AnnotationGroupItem;
class Tracker;
class TrackerReal;
class MoCapStorage;
class MoCapItem;

class Petrack : public QMainWindow
{
    Q_OBJECT

public:
    Petrack(QString petrackVersion);
    ~Petrack();
    static int trcVersion; // version numbr for writing TRC-Trajectorie files

protected:
    void closeEvent(QCloseEvent *event);

private slots:
    void exportVideo();
    void exportVideoView();
    void exportImageSequence();
    void exportViewSequence();
    void print();
    void resetSettings();
    void setGlobalFont();
    void showHideControlWidget();


    void setSequenceFPSDialog();
    void antialias();
    void opengl();
    void reset();
    void fitInView();
    void fitInROI();
    void commandLineOptions();
    void keyBindings();
    void showLogWindow();
    void showGroupAnnotationWindow();
    void about();
    void onlineHelp();
    void setCamera();

public slots:
    void openProject(QString fileName = "", bool openSequence = true);
    bool saveSameProject();
    bool saveProjectAs();
    void openSequence(QString fileName = "");
    void openCameraLiveStream(int camID = -1);
    void openMoCapFile();
    void editMoCapSettings();
    void exportView(QString dest = "");
    void exportImage(QString dest = "");
    void setStatusPosReal();
    void addManualTrackPointOnlyVisible(const QPointF &pos);
    void splitTrackPerson(QPointF pos);
    int  addOrMoveManualTrackPoint(const QPointF &pos);
    void deleteTrackPoint(QPointF pos, PersonStorage::TrajectorySegment direction);
    void deleteTrackPointAll(PersonStorage::TrajectorySegment direction);
    void deleteTrackPointROI();
    void deleteTrackPointInsideROI();
    void moveTrackPoint(QPointF pos);
    void selectPersonForMoveTrackPoint(QPointF pos);
    void releaseTrackPoint();
    void skipToFrameWheel(int delta);
    void skipToFrameFromTrajectory(QPointF pos);
    void scrollShowOnly(int delta);

public:
    void         openXml(QDomDocument &doc, bool openSequence = true);
    void         saveXml(QDomDocument &doc);
    void         writeXmlElement(QXmlStreamWriter &xmlStream, QDomElement element);
    bool         saveProject(QString fileName = "");
    void         exportSequence(bool saveVideo, bool saveView = false, QString dest = "");
    void         editTrackPersonComment(QPointF pos);
    void         setTrackPersonHeight(QPointF pos);
    void         resetTrackPersonHeight(QPointF pos);
    void         updateSourceInOutFrames();
    void         setFPS(double fps);
    void         setSequenceFPS(double fps);
    void         updateControlWidget();
    void         dragEnterEvent(QDragEnterEvent *event);
    void         dropEvent(QDropEvent *event);
    void         updateSceneRect();
    double       getStatusPosRealHeight();
    void         setStatusStereo(float x, float y, float z);
    void         setStatusTime();
    void         setStatusFPS();
    void         setShowFPS(double fps);
    void         updateShowFPS(bool skipped = false);
    void         setStatusPosReal(const QPointF &pos);
    void         setStatusPos(const QPoint &pos);
    void         setStatusColor(const QRgb &col);
    void         setStatusColor();
    void         setMousePosOnImage(QPointF pos);
    void         updateControlImage(cv::Mat &img);
    int          calculateRealTracker();
    void         exportTracker(QString dest = "");
    void         importTracker(QString dest = "");
    void         trackAll();
    void         playAll();
    int          winSize(QPointF *pos = nullptr, int pers = -1, int frame = -1, int level = -1);
    bool         updateImage(bool imageChanged = false);
    bool         updateImage(const cv::Mat &img);
    void         updateSequence();
    QSet<size_t> getPedestrianUserSelection();
    QSet<size_t> getPedestriansToTrack();
    double       getCmPerPixel() const;
    void         setHeadSize(double hS = -1);
    double       getHeadSize(QPointF *pos = nullptr, int pers = -1, int frame = -1);

    //------------------------------
    // inline function
    bool isLoading() const { return mLoading; }
    void setLoading(bool b) { mLoading = b; }

    inline pet::StereoContext *getStereoContext() { return mStereoContext; }
    inline QString             getProFileName() { return mProFileName; }

    void setProFileName(const QString &fileName);

public:
    inline QString getTrackFileName() { return mTrcFileName; }
    inline void    setTrackFileName(const QString &fn) { mTrcFileName = fn; }
    inline QString getHeightFileName() { return mHeightFileName; }
    inline void    setHeightFileName(const QString &fn) { mHeightFileName = fn; }
    inline QString getMarkerIDFileName() { return mMarkerIDFileName; }
    inline void    setMarkerIDFileName(const QString &fn) { mMarkerIDFileName = fn; }

    inline Control                *getControlWidget() { return mControlWidget; }
    inline reco::Recognizer       &getRecognizer() { return mReco; }
    inline StereoWidget           *getStereoWidget() { return mStereoWidget; }
    inline ColorRangeWidget       *getColorRangeWidget() { return mColorRangeWidget; }
    inline ColorMarkerWidget      *getColorMarkerWidget() { return mColorMarkerWidget; }
    inline CodeMarkerWidget       *getCodeMarkerWidget() { return mCodeMarkerWidget; }
    inline MultiColorMarkerWidget *getMultiColorMarkerWidget() { return mMultiColorMarkerWidget; }
    inline YOLOMarkerWidget       *getYOLOMarkerWidget() { return mYOLOMarkerWidget; }
    inline LogWindow              *getLogWindow() { return mLogWindow; }
    inline GraphicsView           *getView() { return mView; }
    inline QGraphicsScene         *getScene() { return mScene; }
    inline QImage                 *getImage() { return mImage; }
    inline cv::Mat                 getImg() { return mImg; }
    inline cv::Mat                 getImageFiltered() { return mImgFiltered; }
    inline PersonStorage          &getPersonStorage() { return mPersonStorage; }
    inline const PersonStorage    &getPersonStorage() const { return mPersonStorage; }
    inline Tracker                *getTracker() { return mTracker; }
    inline TrackerReal            *getTrackerReal() { return mTrackerReal; }
    inline ImageItem              *getImageItem() { return mImageItem; }
    inline StereoItem             *getStereoItem() { return mStereoItem; }
    inline ColorMarkerItem        *getColorMarkerItem() { return mColorMarkerItem; }
    inline CodeMarkerItem         *getCodeMarkerItem() { return mCodeMarkerItem; }
    inline MultiColorMarkerItem   *getMultiColorMarkerItem() { return mMultiColorMarkerItem; }
    inline BackgroundItem         *getBackgroundItem() { return mBackgroundItem; }
    inline MoCapItem              *getMoCapItem() { return mMoCapItem; }
    inline RoiItem                *getRecoRoiItem() { return mRecognitionRoiItem; }
    inline RoiItem                *getTrackRoiItem() { return mTrackingRoiItem; }

    inline TrackerItem *getTrackerItem() { return mTrackerItem; }
    inline Animation   *getAnimation() { return &mAnimation; }
    inline Player      *getPlayer() { return mPlayerWidget; }

    inline QPointF getMousePosOnImage() { return mMousePosOnImage; }

    inline void setRecognitionChanged(bool b) { mRecognitionChanged = b; }
    inline bool recognitionChanged() const { return mRecognitionChanged; }

    inline void     setTrackChanged(bool b) { mTrackChanged = b; }
    inline bool     trackChanged() const { return mTrackChanged; }
    inline QAction *getHideControlActor() { return mHideControlsAct; }

    inline CalibFilter *getCalibFilter() { return &mCalibFilter; }

    inline BrightContrastFilter *getBrightContrastFilter() { return &mBrightContrastFilter; }
    inline BorderFilter         *getBorderFilter() { return &mBorderFilter; }
    inline SwapFilter           *getSwapFilter() { return &mSwapFilter; }
    inline BackgroundFilter     *getBackgroundFilter() { return &mBackgroundFilter; }

    inline AnnotationGroupManager &getGroupManager() { return mGroupManager; }

    inline int getImageBorderSize()
    {
        if(getBorderFilter()->getEnabled())
        {
            return getBorderFilter()->getBorderSize().getValue();
        }
        else
        {
            return 0;
        }
    }
    inline void setImageBorderSize(int sz) { getBorderFilter()->getBorderSize().setValue(sz); }

    inline AutoCalib               *getAutoCalib() { return &mAutoCalib; }
    inline ExtrCalibration         *getExtrCalibration() { return &mExtrCalibration; }
    const WorldImageCorrespondence &getWorldImageCorrespondence();
    inline double                   getStatusFPS() const { return mShowFPS; }
    inline MoCapController         &getMoCapController() { return mMoCapController; }

    void updateWindowTitle();

    /**
     * @brief Sets the information about the compiled git commit hash, commit
     * date, and commit branch
     * @param gitCommitID commit hash of current version
     * @param gitCommitDate commit date of current version
     * @param gitCommitBranch commit branch of current version
     */
    void setGitInformation(
        const std::string &gitCommitID,
        const std::string &gitCommitDate,
        const std::string &gitCommitBranch);

    /**
     * @brief Sets the information about the used compiler and time stamp
     * @param compileTimeStamp time the program was compiled
     * @param compilerID name of the used compiler
     * @param compilerVersion version of the used compiler
     */
    void setCompileInformation(
        const std::string &compileOS,
        const std::string &compileTimeStamp,
        const std::string &compilerID,
        const std::string &compilerVersion);

    const QString &getLastTrackerExport() const;
    void           setLastTrackerExport(const QString &newLastTrackerExport);


    void getFilteredImage(
        bool imageChanged,
        bool brightContrastFilterChanged,
        bool swapFilterChanged,
        bool borderFilterChanged,
        bool calibFilterChanged);
    void resetExistingPoints();
    void performTracking();
    void performRecognition();

private slots:
    void openAutosaveSettings();

private:
    void createActions();
    void createMenus();
    void createStatusBar();
    void readSettings();
    void writeSettings();
    void resetUI();

    bool maybeSave();

    void keyPressEvent(QKeyEvent *event);
    void mousePressEvent(QMouseEvent *event);

    //------------------------------

    QHBoxLayout *mCentralLayout;

    QFrame                 *mCentralWidget;
    Control                *mControlWidget;
    QSplitter              *mSplitter;
    StereoWidget           *mStereoWidget;
    ColorRangeWidget       *mColorRangeWidget;
    ColorMarkerWidget      *mColorMarkerWidget;
    CodeMarkerWidget       *mCodeMarkerWidget;
    MultiColorMarkerWidget *mMultiColorMarkerWidget;
    YOLOMarkerWidget       *mYOLOMarkerWidget;
    LogWindow              *mLogWindow;
    AnnotationGroupWidget  *mGroupingWidget;

    QAction      *mOpenSeqAct;
    QAction      *mOpenCameraAct;
    QAction      *mOpenMoCapAct;
    QAction      *mEditMoCapAct;
    QAction      *mExportSeqVidAct;
    QAction      *mExportSeqVidViewAct;
    QAction      *mExportSeqImgAct;
    QAction      *mExportSeqViewAct;
    QAction      *mOpenPrAct;
    QAction      *mSaveAct;
    QAction      *mSavePrAct;
    QAction      *mExportImageAct;
    QAction      *mExportViewAct;
    QAction      *mPrintAct;
    QAction      *mResetSettingsAct;
    QAction      *mExitAct;
    QAction      *mSetSequenceFPSAct;
    QAction      *mFontAct;
    QAction      *mHideControlsAct;
    QAction      *mAntialiasAct;
    QAction      *mCropZoomViewAct;
    QAction      *mOpenGLAct;
    QAction      *mResetAct;
    QAction      *mFitViewAct;
    QAction      *mFitROIAct;
    QAction      *mCameraLeftViewAct;
    QAction      *mCameraRightViewAct;
    QAction      *mLimitPlaybackSpeed;
    QAction      *mFixPlaybackSpeed;
    QAction      *mSetToRealtime;
    QAction      *mSetTo0p75;
    QAction      *mSetTo0p50;
    QAction      *mSetTo0p25;
    QAction      *mSetTo1p25;
    QAction      *mSetTo1p50;
    QAction      *mSetTo1p75;
    QAction      *mSetTo2p00;
    QAction      *mPlayerLooping;
    QAction      *mUndoAct;
    QAction      *mRedoAct;
    QAction      *mDelPastAct;
    QAction      *mDelFutureAct;
    QAction      *mDelAllRoiAct;
    QAction      *mDelPartRoiAct;
    QAction      *mCommandAct;
    QAction      *mKeyAct;
    QAction      *mShowLogWindowAct;
    QAction      *mShowGroupAnnotationWindowAct;
    QAction      *mAboutAct;
    QAction      *mOnlineHelpAct;
    QAction      *mAutosaveSettings;
    QActionGroup *mCameraGroupView;
    QMenu        *mPlaybackSpeedMenu;

    QMenu *mFileMenu;
    QMenu *mViewMenu;
    QMenu *mEditMenu;
    QMenu *mCameraMenu;
    QMenu *mHelpMenu;

    QString mSeqFileName;
    QString mProFileName;
    QString mTrcFileName;
    QString mLastTrackerExport;
    QString mHeightFileName;
    QString mMarkerIDFileName;

    cv::Mat             mImg;
    cv::Mat             mImgFiltered;
    QImage             *mImage;
    Animation           mAnimation{this};
    pet::StereoContext *mStereoContext;
    Player             *mPlayerWidget;

    ViewWidget           *mViewWidget;
    GraphicsView         *mView;
    QGraphicsScene       *mScene;
    ImageItem            *mImageItem;
    LogoItem             *mLogoItem;
    CoordItem            *mCoordItem;
    GridItem             *mGridItem;
    RoiItem              *mRecognitionRoiItem;
    RoiItem              *mTrackingRoiItem;
    TrackerItem          *mTrackerItem;
    StereoItem           *mStereoItem;
    ColorMarkerItem      *mColorMarkerItem;
    CodeMarkerItem       *mCodeMarkerItem;
    MultiColorMarkerItem *mMultiColorMarkerItem;
    BackgroundItem       *mBackgroundItem;
    MoCapItem            *mMoCapItem;
    AnnotationGroupItem  *mActionGroupItem;

    QDoubleSpinBox *mStatusPosRealHeight;
    QLabel         *mStatusLabelStereo;
    QLabel         *mStatusLabelTime;
    QLabel         *mStatusLabelFPS;
    QLabel         *mStatusLabelPosReal;
    QLabel         *mStatusLabelPos;
    QLabel         *mStatusLabelColor;

    QPointF mMousePosOnImage;

    CalibFilter          mCalibFilter;
    BrightContrastFilter mBrightContrastFilter;
    BorderFilter         mBorderFilter;
    SwapFilter           mSwapFilter;
    BackgroundFilter     mBackgroundFilter;

    AutoCalib                       mAutoCalib;
    Autosave                        mAutosave{*this};
    PersonStorage                   mPersonStorage{*this, mAutosave};
    ExtrCalibration                 mExtrCalibration;
    const WorldImageCorrespondence *mWorldImageCorrespondence;

    bool mRecognitionChanged;
    bool mTrackChanged;

    reco::Recognizer mReco;

    QDomDocument mDefaultSettings;

    Tracker     *mTracker;
    TrackerReal *mTrackerReal;
    double       mHeadSize;
    double       mCmPerPixel;

    ManualTrackpointMover mManualTrackPointMover;

    double mShowFPS;

    bool mAutoBackTrack          = true;
    bool mAutoTrackOptimizeColor = false;
    bool mLoading;

    MoCapStorage    mMoCapStorage;
    MoCapController mMoCapController{mMoCapStorage, mExtrCalibration};

    AnnotationGroupManager mGroupManager{*this, mAnimation, mPersonStorage};

    QString mPetrackVersion{"Unknown"};  ///< Version of PeTrack used to compile
    QString mGitCommitID{"Unknown"};     ///< Commit hash used to compile
    QString mGitCommitDate{"Unknown"};   ///< Commit date used to compile
    QString mGitCommitBranch{"Unknown"}; ///< Branch used to compile

    QString mCompileOS{"Unknown"};       ///< Compile OS
    QString mCompileDate{"Unknown"};     ///< Compile date
    QString mCompilerID{"Unknown"};      ///< Used compiler
    QString mCompilerVersion{"Unknown"}; ///< Used compiler version

    std::vector<std::string> mAuthors;

    MissingFrames mMissingFrames{false, {}}; ///< Missing frame information
};

namespace util
{
std::optional<QSet<int>> splitStringToInt(const QString &input);
}

#endif
