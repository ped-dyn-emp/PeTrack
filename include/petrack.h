#ifndef PETRACK_H
#define PETRACK_H

#include <QMainWindow>
#include <QDomDocument>
#include <QKeyEvent>
#include <QMouseEvent>


#include <opencv.hpp>
//#include "opencv2/cxcore.h"
//#include "opencv2/cvaux.h"
//#include "opencv2/opencv.hpp"

#ifdef STEREO
#include "stereoContext.h"
#include "calibStereoFilter.h"
#endif
#include "helper.h"
#include "brightContrastFilter.h"
#include "calibFilter.h"
#include "borderFilter.h"
#include "swapFilter.h"
#include "backgroundFilter.h"
#include "autoCalib.h"
#include "coordItem.h"
#include "gridItem.h"
#include "recognitionRoiItem.h"
#include "trackingRoiItem.h"
#include "animation.h"
#include "extrCalibration.h"

#ifdef STEREO_DISABLED
enum Camera {cameraLeft, cameraRight, cameraUnset};
#endif

#define HEAD_SIZE 21. // durchschnittliche Kopflaenge in cm (Kopf 21x14)
#define AXIS_MARKERS_LENGTH 10

class QAction;
class QActionGroup;
class QLabel;
class QMenu;
class QHBoxLayout;
class QImage;
class QString;
class GraphicsView;
class QGraphicsScene;
class QDoubleSpinBox;
class QFrame;

class ImageItem;
class LogoItem;
class Control;
class StereoWidget;
class ColorRangeWidget;
class ColorMarkerWidget;
class CodeMarkerWidget;
class MultiColorMarkerWidget;
class ViewWidget;
class Player;
class TrackerItem;
class StereoItem;
class ColorMarkerItem;
class CodeMarkerItem;
class MultiColorMarkerItem;
class BackgroundItem;
class Tracker;
class TrackerReal;
//class ExtrCalibration;
//enum class Dimension;

class Petrack : public QMainWindow
{
    Q_OBJECT

public:
    Petrack();
    ~Petrack();
    static int trcVersion; // version numbr for writing TRC-Trajectorie files
//    enum class Dimension
//    {
//        dim2D=2,
//        dim3D=3
//    };

protected:
    void closeEvent(QCloseEvent *event);
    //void contextMenuEvent(QContextMenuEvent *event) Q_DECL_OVERRIDE;

private slots:
    void saveVideo();
    void saveVideoView();
    void saveImageSequence();
    void saveViewSequence();
    void print();
    void resetSettings();
    void setGlobalFont();
    void showHideControlWidget();
    void antialias();
    void opengl();
    void reset();
    void fitInView();
    void fitInROI();
    void commandLineOptions();
    void keyBindings();
    void about();
    void onlineHelp();
    void setCamera();

public slots:
    void openXml(QDomDocument &doc, bool openSequence = true);
    void openProject(QString fileName = "", bool openSequence = true);
    void saveXml(QDomDocument &doc);
    bool saveSameProject();
    bool saveProject(QString fileName = "");
    void writeXmlElement(QXmlStreamWriter& xmlStream, QDomElement element);
    void openSequence(QString fileName = "");
    void openCameraLiveStream(int camID = -1);
    void saveSequence(bool saveVideo, bool saveView = false, QString dest = "");
    void saveView(QString dest = "");
    void saveImage(QString dest = "");
    void setStatusPosReal();
    void addManualTrackPointOnlyVisible(QPointF pos);   
    void splitTrackPerson(QPointF pos);
    int  addManualTrackPoint(QPointF pos);
    void editTrackPersonComment(QPointF pos);
    void setTrackPersonHeight(QPointF pos);
    void resetTrackPersonHeight(QPointF pos);
    void deleteTrackPoint(QPointF pos, int direction);
    void deleteTrackPointAll(int direction);
    void deleteTrackPointROI();
    void deleteTrackPointInsideROI();
//    void showContextMenu(QPointF pos);
    void updateSourceInOutFrames();
    void skipToFrameWheel(int delta);

public:
    void updateControlWidget();
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);
    void updateSceneRect();
    double getStatusPosRealHeight();
    void setStatusStereo(float x, float y, float z);
    void setStatusTime();
    void setStatusFPS();
    void setShowFPS(double fps);
    void updateShowFPS();
    void setStatusPosReal(const QPointF &pos);
    void setStatusPos(const QPoint &pos);
    void setStatusColor(const QRgb &col);
    void setStatusColor();
    void setMousePosOnImage(QPointF pos);
    void updateControlImage(cv::Mat &img);
    int calculateRealTracker();
    void exportTracker(QString dest = "");
//    void load3DCalibPoints(QString src = "");
    void importTracker(QString dest = "");
    void testTracker();
    void trackAll();
    void playAll();
    int winSize(QPointF *pos=NULL, int pers=-1, int frame=-1, int level=-1);
    void updateImage(bool imageChanged = false);
    void updateImage(const cv::Mat &img);
//    void updateImage(IplImage *iplImg);
    void updateSequence();
//    void calcBackground();
    QSet<int> getOnlyVisible();
    double getCmPerPixel();
    void setHeadSize(double hS=-1);
    double getHeadSize(QPointF *pos=NULL, int pers=-1, int frame=-1);

    //------------------------------
    // inline function
    bool isLoading()
    {
        return mLoading;
    }
    void setLoading(bool b)
    {
        mLoading = b;
    }

    inline pet::StereoContext* getStereoContext()
    {
        return mStereoContext;
    }
    inline QString getProFileName()
    {
        return mProFileName;
    }
    inline QString getTrackFileName()
    {
        return mTrcFileName;
    }
    inline void setTrackFileName(const QString &fn)
    {
        mTrcFileName = fn;
    }
    inline Control* getControlWidget()
    {
        return mControlWidget;
    }
    inline StereoWidget* getStereoWidget()
    {
        return mStereoWidget;
    }
    inline ColorRangeWidget* getColorRangeWidget()
    {
        return mColorRangeWidget;
    }
    inline ColorMarkerWidget* getColorMarkerWidget()
    {
        return mColorMarkerWidget;
    }
    inline CodeMarkerWidget* getCodeMarkerWidget()
    {
        return mCodeMarkerWidget;
    }
    inline MultiColorMarkerWidget* getMultiColorMarkerWidget()
    {
        return mMultiColorMarkerWidget;
    }
    inline GraphicsView* getView()
    {
        return mView;
    }
    inline QGraphicsScene* getScene()
    {
        return mScene;
    }
    inline QImage* getImage()
    {
        return mImage;
    }
//    inline IplImage* getIplImage()
//    {
//        return mIplImg;
//    }
    inline cv::Mat getImg()
    {
        return mImg;// = cvarrToMat(mIplImg);
    }
//    inline IplImage* getIplImageFiltered()
//    {
//        return mIplImgFiltered;
//    }
    inline cv::Mat getImageFiltered()
    {
        return mImgFiltered;// = cvarrToMat(mIplImgFiltered);
    }
    inline Tracker* getTracker()
    {
        return mTracker;
    }
    inline TrackerReal* getTrackerReal()
    {
        return mTrackerReal;
    }
    inline ImageItem* getImageItem()
    {
        return mImageItem;
    }
    inline StereoItem* getStereoItem()
    {
        return mStereoItem;
    }
    inline ColorMarkerItem* getColorMarkerItem()
    {
        return mColorMarkerItem;
    }
    inline CodeMarkerItem* getCodeMarkerItem()
    {
        return mCodeMarkerItem;
    }
    inline MultiColorMarkerItem* getMultiColorMarkerItem()
    {
        return mMultiColorMarkerItem;
    }
    inline BackgroundItem* getBackgroundItem()
    {
        return mBackgroundItem;
    }
    inline RecognitionRoiItem* getRecoRoiItem()
    {
        return mRecognitionRoiItem;
    }
    inline TrackingRoiItem* getTrackRoiItem()
    {
        return mTrackingRoiItem;
    }

    inline TrackerItem* getTrackerItem()
    {
        return mTrackerItem;
    }
    inline Animation* getAnimation()
    {
        return mAnimation;
    }
    inline Player* getPlayer()
    {
        return mPlayerWidget;
    }

    inline void updateCoord()
    {
        if (mCoordItem)
            mCoordItem->updateData();
    }

    inline QPointF getMousePosOnImage()
    {
        return mMousePosOnImage;
    }

    inline void setRecognitionChanged(bool b)
    {
        mRecognitionChanged = b;
    }
    inline bool recognitionChanged()
    {
        return mRecognitionChanged;
    }

    inline void setTrackChanged(bool b)
    {
        mTrackChanged = b;
    }
    inline bool trackChanged()
    {
        return mTrackChanged;
    }
    inline QAction* getHideControlActor()
    {
        return mHideControlsAct;
    }

    // Attention: not type save, be care that animation is not stereo
    inline CalibFilter* getCalibFilter()
    {
        return (CalibFilter*) mCalibFilter;
    }
#ifdef STEREO
    // Attention: not type save, be care that animation is stereo
    inline CalibStereoFilter* getCalibStereoFilter()
    {
        return (CalibStereoFilter*) mCalibFilter;
    }
#endif
    inline BrightContrastFilter* getBrightContrastFilter()
    {
        return &mBrightContrastFilter;
    }
    // ContrastFilter *getContrastFilter()
    // {
    //     return &mContrastFilter;
    // }
    // BrightFilter *getBrightFilter()
    // {
    //     return &mBrightFilter;
    // }
    inline BorderFilter* getBorderFilter()
    {
        return &mBorderFilter;
    }
    inline SwapFilter* getSwapFilter()
    {
        return &mSwapFilter;
    }
    inline BackgroundFilter* getBackgroundFilter()
    {
        return &mBackgroundFilter;
    }

    inline int getImageBorderSize()
    {
        if (getBorderFilter()->getEnabled()) 
            return (int) getBorderFilter()->getBorderSize()->getValue();
        else 
            return 0;
        //mImageBorderSize;
    }
    inline void setImageBorderSize(int sz)
    {
        getBorderFilter()->getBorderSize()->setValue(sz);
        //mImageBorderSize = sz;
    }

    inline AutoCalib* getAutoCalib()
    {
        return &mAutoCalib;
    }
    inline ExtrCalibration* getExtrCalibration()
    {
        return &mExtrCalibration;
    }
    inline double getStatusFPS()
    {
        return mShowFPS;
    }


    void updateWindowTitle();

private:
    void createActions();
    void createMenus();
    //void createToolBars();
    void createStatusBar();
    void readSettings();
    void writeSettings();
    void resetUI();

    bool maybeSave();


    void keyPressEvent(QKeyEvent * event);
    void mousePressEvent(QMouseEvent *event);

    //------------------------------

    QHBoxLayout *mCentralLayout;

    QFrame *mCentralWidget;
    Control *mControlWidget;
    StereoWidget *mStereoWidget;
    ColorRangeWidget *mColorRangeWidget;
    ColorMarkerWidget *mColorMarkerWidget;
    CodeMarkerWidget *mCodeMarkerWidget;
    MultiColorMarkerWidget *mMultiColorMarkerWidget;

    QAction *mOpenSeqAct;
    QAction *mOpenCameraAct;
    QAction *mSaveSeqVidAct;
    QAction *mSaveSeqVidViewAct;
    QAction *mSaveSeqImgAct;
    QAction *mSaveSeqViewAct;
    QAction *mOpenPrAct;
    QAction *mSaveAct;
    QAction *mSavePrAct;
    QAction *mSaveImageAct;
    QAction *mSaveViewAct;
    QAction *mPrintAct;
    QAction *mResetSettingsAct;
    QAction *mExitAct;
    QAction *mFontAct;
    QAction *mHideControlsAct;
    QAction *mAntialiasAct;
    QAction *mCropZoomViewAct;
    QAction *mOpenGLAct;
    QAction *mResetAct;
    QAction *mFitViewAct;
    QAction *mFitROIAct;
    QAction *mCameraLeftViewAct;
    QAction *mCameraRightViewAct;
    QAction *mDelPastAct;
    QAction *mDelFutureAct;
    QAction *mDelAllRoiAct;
    QAction *mDelPartRoiAct;
    QAction *mCommandAct;
    QAction *mKeyAct;
    QAction *mAboutAct;
    QAction *mOnlineHelpAct;
    QActionGroup *mCameraGroupView;

    QMenu *mFileMenu;
    QMenu *mViewMenu;
    QMenu *mDeleteMenu;
    QMenu *mCameraMenu;
    QMenu *mHelpMenu;

    QString mSeqFileName;
    QString mProFileName;
    QString mTrcFileName;

//    IplImage *mIplImg;
//     IplImage *mPrevIplImgFiltered;
//    IplImage *mIplImgFiltered;
    cv::Mat mImg;
    cv::Mat mImgFiltered;
    QImage *mImage;
    Animation *mAnimation;
    pet::StereoContext *mStereoContext;
    Player *mPlayerWidget;

    ViewWidget *mViewWidget;
    GraphicsView *mView;
    QGraphicsScene *mScene;
    ImageItem *mImageItem;
    LogoItem *mLogoItem;
    CoordItem *mCoordItem;
    GridItem *mGridItem;
    RecognitionRoiItem *mRecognitionRoiItem;
    TrackingRoiItem *mTrackingRoiItem;
    TrackerItem *mTrackerItem;
    StereoItem *mStereoItem;
    ColorMarkerItem *mColorMarkerItem;
    CodeMarkerItem *mCodeMarkerItem;
    MultiColorMarkerItem *mMultiColorMarkerItem;
    BackgroundItem *mBackgroundItem;

    QDoubleSpinBox *mStatusPosRealHeight;
    QLabel *mStatusLabelStereo;
    QLabel *mStatusLabelTime;
    QLabel *mStatusLabelFPS;
    QLabel *mStatusLabelPosReal;
    QLabel *mStatusLabelPos;
    QLabel *mStatusLabelColor;

    QPointF mMousePosOnImage;

    //CalibFilter mCalibFilter;
    Filter *mCalibFilter;
    BrightContrastFilter mBrightContrastFilter;
//     BrightFilter mBrightFilter;
//     ContrastFilter mContrastFilter;
    BorderFilter mBorderFilter;
    SwapFilter mSwapFilter;
    BackgroundFilter mBackgroundFilter;

    AutoCalib mAutoCalib;
    ExtrCalibration mExtrCalibration;

    bool mRecognitionChanged;
    bool mTrackChanged;

    Tracker *mTracker;
    TrackerReal *mTrackerReal;
    double mHeadSize;
    double mCmPerPixel;

//    QString mDefaultSettings;
    QDomDocument mDefaultSettings;

    double mShowFPS;
//    clock_t mLastTime;

    bool mAutoBackTrack;
    bool mAutoTrackOptimizeColor;
    bool mLoading;
};


#endif
