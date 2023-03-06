/*
 * PeTrack - Software for tracking pedestrians movement in videos
 * Copyright (C) 2023 Forschungszentrum Jülich GmbH, IAS-7
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

#ifndef CONTROL_H
#define CONTROL_H

#include "analysePlot.h"
#include "colorPlot.h"
#include "intrinsicCameraParams.h"
#include "petrack.h"
#include "recognition.h"

#include <Qt>
#include <QtWidgets>

class QGraphicsScene;
class QDomElement;
class IntrinsicBox;
class FilterBeforeBox;
namespace Ui
{
class Control;
}

class Control : public QWidget
{
    Q_OBJECT

public:
    Control(
        QWidget          &parent,
        QGraphicsScene   &scene,
        reco::Recognizer &recognizer,
        RoiItem          &trackRoiItem,
        RoiItem          &recoRoiItem,
        MissingFrames    &missingFrames,
        FilterBeforeBox  *filterBefore);
    Control(
        QWidget          &parent,
        QGraphicsScene   &scene,
        reco::Recognizer &recognizer,
        RoiItem          &trackRoiItem,
        RoiItem          &recoRoiItem,
        Ui::Control      *ui,
        MissingFrames    &missingFrames,
        FilterBeforeBox  *filterBefore);

    void setScene(QGraphicsScene *sc);

    void toggleOnlineTracking();
    bool isOnlineTrackingChecked() const;
    void setOnlineTrackingChecked(bool checked);
    int  getTrackRegionLevels() const;
    bool isTrackRepeatChecked() const;
    int  getTrackRepeatQual() const;
    int  getTrackRegionScale() const;
    bool isTrackMergeChecked() const;
    bool isTrackExtrapolationChecked() const;

    QLineEdit *trackShowOnlyNrList();
    void       setTrackNumberNow(const QString &val);
    void       setTrackShowOnlyNr(int val);
    int        getTrackShowOnlyNr() const;
    void       setTrackNumberAll(const QString &number);
    void       setTrackShowOnlyNrMaximum(int max);
    void       setTrackShowOnly(Qt::CheckState state);
    bool       isTrackShowOnlyChecked() const;
    bool       isTrackShowOnlyListChecked() const;
    void       setTrackShowOnlyListChecked(bool checked);
    void       setTrackNumberVisible(const QString &value);
    bool       isTrackOnlySelectedChecked() const;
    int        getTrackShowBefore() const;
    int        getTrackShowAfter() const;
    bool       getTrackShow() const;
    void       setTrackShow(bool b);
    bool       getTrackFix() const;
    void       setTrackFix(bool b);
    QColor     getTrackPathColor() const;
    void       setTrackPathColor(QColor col);
    QColor     getTrackGroundPathColor() const;
    void       setTrackGroundPathColor(QColor col);
    int        getTrackCurrentPointSize() const;
    int        getTrackCurrentPointLineWidth() const;
    int        getTrackPointSize() const;
    int        getTrackShowPointsLineWidth() const;
    int        getTrackColColorSize() const;
    int        getTrackColorMarkerSize() const;
    int        getTrackColorMarkerLineWidth() const;
    int        getTrackNumberSize() const;
    int        getTrackGroundPositionSize() const;
    int        getTrackGroundPathSize() const;
    int        getTrackPathWidth() const;
    bool       isTrackNumberBoldChecked() const;
    bool       isShowVoronoiCellsChecked() const;
    bool       isTrackHeadSizedChecked() const;
    bool       isTrackShowCurrentPointChecked() const;
    bool       isTrackShowSearchSizeChecked() const;
    bool       isTrackShowColorMarkerChecked() const;
    bool       isTrackShowColColorChecked() const;
    bool       isTrackShowNumberChecked() const;
    bool       isTrackShowHeightIndividualChecked() const;
    bool       isTrackShowPointsChecked() const;
    bool       isTrackShowPathChecked() const;
    bool       isTrackShowGroundPathChecked() const;
    bool       isTrackShowOnlyVisibleChecked() const;
    bool       isTrackShowPointsColoredChecked() const;
    bool       isTrackShowGroundPositionChecked() const;

    QColor getMoCapColor() const;

    bool isPerformRecognitionChecked() const;
    void setPerformRecognitionChecked(bool checked);
    void setRecoNumberNow(const QString &val);
    int  getRecoStep() const;
    bool getRecoRoiShow() const;
    void setRecoRoiShow(bool b);
    bool getRecoRoiFix() const;
    void setRecoRoiFix(bool b);

    reco::RecognitionMethod getRecoMethod() const;

    bool getTrackRoiShow() const;
    void setTrackRoiShow(bool b);
    bool getTrackRoiFix() const;
    void setTrackRoiFix(bool b);

    bool getAdaptiveLevel() const;

    int  getFilterBorderSize() const;
    void setFilterBorderSizeMin(int i);
    void setFilterBorderSizeMax(int i);
    bool isFilterBgChecked() const;
    bool isFilterBgDeleteTrjChecked() const;
    int  getFilterBgDeleteNumber() const;

    double getCalibReprError() const;
    void   setCalibReprError(double d);
    void   imageSizeChanged(int width, int height, int borderDiff);

    double getCalibExtrRot1();
    void   setCalibExtrRot1(double d);
    double getCalibExtrRot2();
    void   setCalibExtrRot2(double d);
    double getCalibExtrRot3();
    void   setCalibExtrRot3(double d);
    double getCalibExtrTrans1();
    void   setCalibExtrTrans1(double d);
    double getCalibExtrTrans2();
    void   setCalibExtrTrans2(double d);
    double getCalibExtrTrans3();
    void   setCalibExtrTrans3(double d);
    void   setEnabledExtrParams(bool enable);

    int    getCalibCoordDimension();
    bool   getCalibExtrCalibPointsShow();
    bool   getCalibExtrVanishPointsShow();
    bool   getCalibCoordShow();
    void   setCalibCoordShow(bool b);
    bool   getCalibCoordFix();
    void   setCalibCoordFix(bool b);
    bool   getIs3DView();
    void   setIs3DView(bool b);
    int    getCalibCoordRotate();
    void   setCalibCoordRotate(int i);
    int    getCalibCoordTransX();
    void   setCalibCoordTransX(int i);
    int    getCalibCoordTransXMax();
    void   setCalibCoordTransXMax(int i);
    int    getCalibCoordTransXMin();
    void   setCalibCoordTransXMin(int i);
    int    getCalibCoordTransY();
    void   setCalibCoordTransY(int i);
    int    getCalibCoordTransYMax();
    void   setCalibCoordTransYMax(int i);
    int    getCalibCoordTransYMin();
    void   setCalibCoordTransYMin(int i);
    int    getCalibCoordScale();
    void   setCalibCoordScale(int i);
    double getCalibCoordUnit();
    void   setCalibCoordUnit(double d);
    bool   isCoordUseIntrinsicChecked() const;

    int  getCalibCoord3DTransX();
    void setCalibCoord3DTransX(int i);
    int  getCalibCoord3DTransY();
    void setCalibCoord3DTransY(int i);
    int  getCalibCoord3DTransZ();
    void setCalibCoord3DTransZ(int i);
    int  getCalibCoord3DAxeLen();
    void setCalibCoord3DAxeLen(int i);
    bool getCalibCoord3DSwapX();
    void setCalibCoord3DSwapX(bool b);
    bool getCalibCoord3DSwapY();
    void setCalibCoord3DSwapY(bool b);
    bool getCalibCoord3DSwapZ();
    void setCalibCoord3DSwapZ(bool b);

    int  getCalibGridDimension();
    bool getCalibGridShow();
    void setCalibGridShow(bool b);
    bool getCalibGridFix();
    void setCalibGridFix(bool b);
    int  getCalibGridRotate();
    void setCalibGridRotate(int i);
    int  getCalibGridTransX();
    void setCalibGridTransX(int i);
    int  getCalibGridTransY();
    void setCalibGridTransY(int i);
    int  getCalibGridScale();
    void setCalibGridScale(int i);

    int  getCalibGrid3DTransX();
    void setCalibGrid3DTransX(int i);
    int  getCalibGrid3DTransY();
    void setCalibGrid3DTransY(int i);
    int  getCalibGrid3DTransZ();
    void setCalibGrid3DTransZ(int i);
    void setGridMinMaxTranslation(int minx, int maxx, int miny, int maxy);
    int  getCalibGrid3DResolution();
    void setCalibGrid3DResolution(int i);

    void expandRange(QColor &fromColor, QColor &toColor, const QColor &clickedColor) const;
    void saveChange(const QColor &fromColor, const QColor &toColor, RectPlotItem *map);
    bool getColors(QColor &clickedColor, QColor &toColor, QColor &fromColor, RectPlotItem *&map);

    double getDefaultHeight() const;
    double getCameraAltitude() const;

    void       setXml(QDomElement &elem);
    void       getXml(QDomElement &elem);
    bool       isLoading() const { return mMainWindow->isLoading(); }
    ColorPlot *getColorPlot() const;
    void       replotColorplot();

    bool isTestEqualChecked() const;
    bool isTestVelocityChecked() const;
    bool isTestInsideChecked() const;
    bool isTestLengthChecked() const;

    void setMapX(int val);
    int  getMapX() const;
    void setMapY(int val);
    int  getMapY() const;
    void setMapW(int val);
    int  getMapW() const;
    void setMapH(int val);
    int  getMapH() const;
    int  getRecoColorX() const;
    int  getRecoColorY() const;
    int  getRecoColorZ() const;
    int  getRecoColorModel() const;
    int  getMapNr() const;

    int  getMarkerBrightness() const;
    bool isMarkerIgnoreWithoutChecked() const;
    bool isRecoAutoWBChecked() const;

    Qt::CheckState getAnaMissingFrames() const;
    Qt::CheckState getTrackAlternateHeight() const;
    bool           isExportElimTpChecked() const;
    bool           isExportElimTrjChecked() const;
    bool           isExportSmoothChecked() const;
    bool           isExportViewDirChecked() const;
    bool           isExportAngleOfViewChecked() const;
    bool           isExportMarkerIDChecked() const;
    bool           isTrackRecalcHeightChecked() const;
    bool           isTrackMissingFramesChecked() const;
    bool           isExportUseMeterChecked() const;
    bool           isExportCommentChecked() const;

    IntrinsicCameraParams getIntrinsicCameraParams() const;
    void                  runAutoCalib();


#ifdef QWT
    AnalysePlot *getAnalysePlot() const;
#endif
    bool isAnaConsiderXChecked() const;
    bool isAnaConsiderYChecked() const;
    bool isAnaConsiderAbsChecked() const;
    bool isAnaConsiderRevChecked() const;
    bool isAnaMarkActChecked() const;
    int  getAnaStep() const;

    inline Petrack *getMainWindow() const
    {
        return mMainWindow;
    }


private slots:
    void on_anaCalculate_clicked();
    void on_anaStep_valueChanged(int i);
    void on_anaMarkAct_stateChanged(int i);
    void on_anaConsiderX_stateChanged(int i);
    void on_anaConsiderY_stateChanged(int i);
    void on_anaConsiderAbs_stateChanged(int i);
    void on_anaConsiderRev_stateChanged(int i);

    void on_recoShowColor_stateChanged(int i);
    void on_recoOptimizeColor_clicked();
    void on_recoColorModel_currentIndexChanged(int index);
    void on_recoColorX_currentIndexChanged(int index);
    void on_recoColorY_currentIndexChanged(int index);
    void on_recoColorZ_valueChanged(int index);
    void on_recoGreyLevel_valueChanged(int index);
    void on_recoSymbolSize_valueChanged(int index);
    void on_recoStereoShow_clicked();
    void on_mapColorRange_clicked();
    void on_colorPickerButton_clicked(bool checked);
    void on_mapReadHeights_clicked();
    void on_mapReadMarkerID_clicked();

    void on_mapNr_valueChanged(int i);
    void on_mapX_valueChanged(int i);
    void on_mapY_valueChanged(int i);
    void on_mapW_valueChanged(int i);
    void on_mapH_valueChanged(int i);
    void on_mapColor_stateChanged(int i);
    void on_mapHeight_valueChanged(double d);
    void on_mapHeight_editingFinished();
    void on_mapAdd_clicked();
    void on_mapDel_clicked();
    void on_mapDistribution_clicked();
    void on_mapResetHeight_clicked();
    void on_mapResetPos_clicked();
    void on_mapDefaultHeight_valueChanged(double d);
    void on_expandColor();
    void on_setColor();


    void on_trackShow_stateChanged(int i);
    void on_trackFix_stateChanged(int i);
    void on_trackOnlineCalc_stateChanged(int i);
    void on_trackCalc_clicked();
    void on_trackReset_clicked();
    void on_trackExport_clicked();
    void on_trackImport_clicked();
    void on_trackTest_clicked();

    void on_trackRegionScale_valueChanged(int i);
    void on_trackRegionLevels_valueChanged(int i);
    void on_trackShowSearchSize_stateChanged(int i);

    void on_trackShowOnlyVisible_stateChanged(int /*i*/)
    {
        if(!isLoading())
        {
            mScene->update();
        }
    }
    void on_trackShowCurrentPoint_stateChanged(int /*i*/)
    {
        if(!isLoading())
        {
            mScene->update();
        }
    }
    void on_trackGotoNr_clicked();
    void on_trackGotoStartNr_clicked();
    void on_trackGotoEndNr_clicked();

    void on_trackHeadSized_stateChanged(int i);


    void on_trackShowPoints_stateChanged(int /*i*/)
    {
        if(!isLoading())
        {
            mScene->update();
        }
    }
    void on_trackShowPointsColored_stateChanged(int /*i*/)
    {
        if(!isLoading())
        {
            mScene->update();
        }
    }
    void on_trackShowPath_stateChanged(int /*i*/)
    {
        if(!isLoading())
        {
            mScene->update();
        }
    }
    void on_trackShowColColor_stateChanged(int /*i*/)
    {
        if(!isLoading())
        {
            mScene->update();
        }
    }
    void on_trackShowColorMarker_stateChanged(int /*i*/)
    {
        if(!isLoading())
        {
            mScene->update();
        }
    }
    void on_trackShowNumber_stateChanged(int /*i*/)
    {
        if(!isLoading())
        {
            mScene->update();
        }
    }
    void on_trackShowGroundPosition_stateChanged(int /*i*/)
    {
        if(!isLoading())
        {
            mScene->update();
        }
    }
    void on_trackShowGroundPath_stateChanged(int /*i*/)
    {
        if(!isLoading())
        {
            mScene->update();
        }
    }
    void on_trackShowHeightIndividual_stateChanged(int /*i*/)
    {
        if(!isLoading())
        {
            mScene->update();
        }
    }
    void on_trackNumberBold_stateChanged(int /*i*/)
    {
        if(!isLoading())
        {
            mScene->update();
        }
    }
    void on_showMoCap_stateChanged(int i);

    void on_trackCurrentPointSize_valueChanged(int /*i*/)
    {
        if(!isLoading())
        {
            mScene->update();
        }
    }
    void on_trackCurrentPointLineWidth_valueChanged(int /*i*/)
    {
        if(!isLoading())
        {
            mScene->update();
        }
    }

    void on_trackPointSize_valueChanged(int /*i*/)
    {
        if(!isLoading())
        {
            mScene->update();
        }
    }
    void on_trackShowPointsLineWidth_valueChanged(int /*i*/)
    {
        if(!isLoading())
        {
            mScene->update();
        }
    }

    void on_trackPathWidth_valueChanged(int /*i*/)
    {
        if(!isLoading())
        {
            mScene->update();
        }
    }
    void on_trackColColorSize_valueChanged(int /*i*/)
    {
        if(!isLoading())
        {
            mScene->update();
        }
    }
    void on_trackColorMarkerSize_valueChanged(int /*i*/)
    {
        mScene->update();
    }
    void on_trackColorMarkerLineWidth_valueChanged(int /*i*/)
    {
        if(!isLoading())
        {
            mScene->update();
        }
    }
    void on_trackNumberSize_valueChanged(int /*i*/)
    {
        if(!isLoading())
        {
            mScene->update();
        }
    }
    void on_trackGroundPositionSize_valueChanged(int /*i*/)
    {
        if(!isLoading())
        {
            mScene->update();
        }
    }
    void on_trackGroundPathSize_valueChanged(int /*i*/)
    {
        if(!isLoading())
        {
            mScene->update();
        }
    }
    void on_trackShowBefore_valueChanged(int /*i*/)
    {
        if(!isLoading())
        {
            mScene->update();
        }
    }
    void on_trackShowAfter_valueChanged(int /*i*/)
    {
        if(!isLoading())
        {
            mScene->update();
        }
    }
    void on_moCapSize_valueChanged(int i);

    void on_recoMethod_currentIndexChanged(int index);
    void onRecoMethodChanged(reco::RecognitionMethod method);
    void on_performRecognition_stateChanged(int i);
    void on_markerBrightness_valueChanged(int i);
    void on_markerIgnoreWithout_stateChanged(int i);
    void on_roiShow_stateChanged(int i);
    void on_roiFix_stateChanged(int i);

    void on_intrinsicParamsChanged(IntrinsicCameraParams params);

    void on_coordShow_stateChanged(int i);
    void on_coordFix_stateChanged(int i);
    void on_coordRotate_valueChanged(int i);
    void on_coordTransX_valueChanged(int i);
    void on_coordTransY_valueChanged(int i);
    void on_coordScale_valueChanged(int i);
    void on_coordAltitude_valueChanged(double d);
    void on_coordUnit_valueChanged(double d);
    void on_coordUseIntrinsic_stateChanged(int i);
    void on_coordLoad3DCalibPoints_clicked();

    void setMeasuredAltitude();

    void on_gridShow_stateChanged(int i);
    void on_gridFix_stateChanged(int i);
    void on_gridRotate_valueChanged(int i);
    void on_gridTransX_valueChanged(int i);
    void on_gridTransY_valueChanged(int i);
    void on_gridScale_valueChanged(int i);

    void on_gridTab_currentChanged(int index);
    void on_coordTab_currentChanged(int index);

    void on_extCalibPointsShow_stateChanged(int arg1);
    void on_extVanishPointsShow_stateChanged(int arg1);

    void on_grid3DTransX_valueChanged(int value);
    void on_grid3DTransY_valueChanged(int value);
    void on_grid3DTransZ_valueChanged(int value);
    void on_grid3DResolution_valueChanged(int value);
    void on_coord3DTransX_valueChanged(int value);
    void on_coord3DTransY_valueChanged(int value);
    void on_coord3DTransZ_valueChanged(int value);
    void on_coord3DAxeLen_valueChanged(int value);

    void on_rot1_valueChanged(double arg1);
    void on_rot2_valueChanged(double arg1);
    void on_rot3_valueChanged(double arg1);
    void on_trans1_valueChanged(double arg1);
    void on_trans2_valueChanged(double arg1);
    void on_trans3_valueChanged(double arg1);

    void on_coord3DSwapX_stateChanged(int arg1);
    void on_coord3DSwapY_stateChanged(int arg1);
    void on_coord3DSwapZ_stateChanged(int arg1);

    void on_extrCalibSave_clicked();
    void on_extrCalibFetch_clicked();
    void on_extrCalibShowPoints_clicked();
    void on_extrCalibShowError_clicked();

    void on_trackPathColorButton_clicked();
    void on_trackGroundPathColorButton_clicked();
    void on_moCapColorButton_clicked();

    void on_showVoronoiCells_stateChanged(int arg1);

    void on_trackShowOnlyNr_valueChanged(int i);
    void on_trackShowOnlyNrList_textChanged(const QString &arg1);

    void on_trackShowOnlyListButton_clicked();

    void on_trackShowOnly_stateChanged(int arg1);
    void on_trackShowOnlyList_stateChanged(int arg1);

    void on_trackRoiShow_stateChanged(int arg1);

    void on_trackRoiFix_stateChanged(int arg1);

    void setMoCapShow(bool visibility);
    void setMoCapColor(QColor col);
    void setMoCapSize(int size);
    void toggleRecoROIButtons();
    void toggleTrackROIButtons();

signals:
    void userChangedRecoMethod(reco::RecognitionMethod method);

private:
    Petrack         *mMainWindow;
    Ui::Control     *mUi;
    IntrinsicBox    *mIntr;
    FilterBeforeBox *mFilterBefore;
    QGraphicsScene  *mScene;
    bool             mColorChanging;
    bool mIndexChanging; // shows, if the index of the color model is really changing; nor while constructor (initialer
                         // durchlauf) and may be while loading xml file
    bool mLoading;       // shows, if new project is just loading
};

#endif
