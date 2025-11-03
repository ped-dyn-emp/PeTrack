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

#ifndef CONTROL_H
#define CONTROL_H

#include "coordinateStructs.h"
#include "intrinsicCameraParams.h"
#include "recognition.h"

#include <Qt>
#include <QtWidgets>

class Petrack;
class Correction;
class QGraphicsScene;
class QDomElement;
class IntrinsicBox;
class ExtrinsicBox;
struct ExtrinsicParameters;
class FilterBeforeBox;
class Petrack;
class RoiItem;
class RectPlotItem;
class CoordinateSystemBox;
class ColorPlot;
class AnalysePlot;
class MissingFrames;
class WorldImageCorrespondence;
class AlignmentGridBox;

namespace Ui
{
class Control;
}

class Control : public QWidget
{
    Q_OBJECT

public:
    Control(
        QWidget             &parent,
        QGraphicsScene      &scene,
        reco::Recognizer    &recognizer,
        RoiItem             &trackRoiItem,
        RoiItem             &recoRoiItem,
        MissingFrames       &missingFrames,
        FilterBeforeBox     *filterBefore,
        IntrinsicBox        *intrinsicBox,
        ExtrinsicBox        *extrinsicBox,
        CoordinateSystemBox *coordSysBox,
        AlignmentGridBox    *gridBox);
    Control(
        QWidget             &parent,
        QGraphicsScene      &scene,
        reco::Recognizer    &recognizer,
        RoiItem             &trackRoiItem,
        RoiItem             &recoRoiItem,
        Ui::Control         *ui,
        MissingFrames       &missingFrames,
        FilterBeforeBox     *filterBefore,
        IntrinsicBox        *intrinsicBox,
        ExtrinsicBox        *extrinsicBox,
        CoordinateSystemBox *coordSysBox,
        AlignmentGridBox    *gridBox);

    void setScene(QGraphicsScene *sc);
    int  getCurrentTab() const;

    void toggleOnlineTracking();
    bool isOnlineTrackingChecked() const;
    void setOnlineTrackingChecked(bool checked);
    int  getTrackRegionLevels() const;
    bool isTrackRepeatChecked() const;
    int  getTrackRepeatQual() const;
    int  getTrackRegionScale() const;
    bool isTrackMergeChecked() const;
    bool isTrackExtrapolationChecked() const;

    void       toggleShowOnly();
    QLineEdit *trackShowOnlyNrList();
    QLineEdit *trackShowMarkerIDsNrList();
    void       setTrackNumberNow(const QString &val);
    void       setTrackShowOnlyNr(int val);
    int        getTrackShowOnlyNr() const;
    void       setTrackNumberAll(const QString &number);
    void       setTrackShowOnlyNrMaximum(int max);
    void       setTrackShowOnly(Qt::CheckState state);
    bool       isTrackShowOnlyChecked() const;
    bool       isTrackShowOnlyListChecked() const;
    void       setTrackShowOnlyListChecked(bool checked);
    bool       isTrackShowMarkerIDsListChecked() const;
    void       setTrackNumberVisible(const QString &value);
    bool       isTrackOnlySelectedChecked() const;
    int        getTrackShowBefore() const;
    int        getTrackShowAfter() const;
    bool       getTrackShow() const;
    void       setTrackShow(bool b);
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
    bool       isTrackShowComplPathChecked() const;

    QColor getMoCapColor() const;

    bool isPerformRecognitionChecked() const;
    void setPerformRecognitionChecked(bool checked);
    void toggleRecognition();
    void setRecoNumberNow(const QString &val);
    int  getRecoStep() const;
    bool getRecoRoiShow() const;
    void setRecoRoiShow(bool b);
    bool getRecoRoiFix() const;
    void setRecoRoiFix(bool b);


    reco::RecognitionMethod getRecoMethod() const;
    reco::MlMethod          getMlMethod() const;

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

    const ExtrinsicParameters &getExtrinsicParameters() const;
    void                       loadExtrinsicCalibFile();

    const WorldImageCorrespondence &getWorldImageCorrespondence() const;
    void                            setCalibCoord2DTransMinMax(Vec2F min, Vec2F max);
    Vec2F                           getCalibCoord2DTrans();

    Vec3F    getCalibCoord3DTrans() const;
    SwapAxis getCalibCoord3DSwap() const;

    int  getCalibCoordDimension();
    bool getCalibCoordShow();
    int  getCalibCoordScale();

    void expandRange(QColor &fromColor, QColor &toColor, const QColor &clickedColor) const;
    void saveChange(const QColor &fromColor, const QColor &toColor, RectPlotItem *map);
    bool getColors(QColor &clickedColor, QColor &toColor, QColor &fromColor, RectPlotItem *&map);

    double getDefaultHeight() const;
    double getCameraAltitude() const;

    void       setXml(QDomElement &elem);
    void       getXml(const QDomElement &elem, const QString &version);
    bool       isLoading() const;
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

    void setScrollAreaMinimumWidth(QScrollArea *scrollArea, QWidget *scrollAreaWidgetContents);

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
    void           setExportUseMeter(bool b);

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

    void        resetCorrection();
    void        onTrackGotoStartNrClicked();
    inline void updateUi()
    {
        if(!isLoading())
        {
            mScene->update();
        }
    }

private:
    void onTrackShowOnlyVisibleStateChanged()
    {
        updateUi();
    }
private slots:
    void onAnaCalculateClicked();
    void onAnaStepValueChanged();
    void onAnaMarkActStateChanged();
    void onAnaConsiderXStateChanged(int i);
    void onAnaConsiderYStateChanged(int i);
    void onAnaConsiderAbsStateChanged();
    void onAnaConsiderRevStateChanged();

    void onRecoShowColorStateChanged(int i);
    void onRecoOptimizeColorClicked();
    void onRecoColorModelCurrentIndexChanged(int index);
    void onRecoColorXCurrentIndexChanged();
    void onRecoColorYCurrentIndexChanged();
    void onRecoColorZValueChanged();
    void onRecoGreyLevelValueChanged(int index);
    void onRecoSymbolSizeValueChanged(int index);
    void onRecoStereoShowClicked();
    void onMapColorRangeClicked();
    void onColorPickerButtonClicked(bool checked);
    void onMapReadHeightsClicked();
    void onMapReadMarkerIDClicked();

    void onMapNrValueChanged(int i);
    void onMapXValueChanged();
    void onMapYValueChanged();
    void onMapWValueChanged();
    void onMapHValueChanged();
    void onMapColorStateChanged();
    void onMapHeightValueChanged();
    void onMapHeightEditingFinished();
    void onMapAddClicked();
    void onMapDelClicked();
    void onMapDistributionClicked();
    void onMapResetHeightClicked();
    void onMapResetPosClicked();
    void onMapDefaultHeightValueChanged(double d);
    void onExpandColor();
    void onSetColor();


    void onTrackShowStateChanged(int i);
    void onTrackOnlineCalcStateChanged(int i);
    void onTrackCalcClicked();
    void onTrackResetClicked();
    void onTrackExportClicked();
    void onTrackImportClicked();

    void onTrackRegionScaleValueChanged();
    void onTrackRegionLevelsValueChanged();
    void onTrackShowSearchSizeStateChanged();


    void onTrackShowCurrentPointStateChanged()
    {
        updateUi();
    }
    void onTrackGotoNrClicked();
    void onTrackGotoEndNrClicked();

    void onTrackHeadSizedStateChanged(int i);


    void onTrackShowPointsStateChanged()
    {
        updateUi();
    }
    void onTrackShowPointsColoredStateChanged()
    {
        updateUi();
    }
    void onTrackShowPathStateChanged()
    {
        updateUi();
    }
    void onTrackShowColColorStateChanged()
    {
        updateUi();
    }
    void onTrackShowColorMarkerStateChanged()
    {
        updateUi();
    }
    void onTrackShowNumberStateChanged()
    {
        updateUi();
    }
    void onTrackShowGroundPositionStateChanged()
    {
        updateUi();
    }
    void onTrackShowGroundPathStateChanged()
    {
        updateUi();
    }
    void onTrackShowHeightIndividualStateChanged()
    {
        updateUi();
    }
    void onTrackNumberBoldStateChanged()
    {
        updateUi();
    }
    void onShowMoCapStateChanged(int i);

    void onTrackCurrentPointSizeValueChanged()
    {
        updateUi();
    }
    void onTrackCurrentPointLineWidthValueChanged()
    {
        updateUi();
    }

    void onTrackPointSizeValueChanged()
    {
        updateUi();
    }
    void onTrackShowPointsLineWidthValueChanged()
    {
        updateUi();
    }

    void onTrackPathWidthValueChanged()
    {
        updateUi();
    }
    void onTrackColColorSizeValueChanged()
    {
        updateUi();
    }
    void onTrackColorMarkerSizeValueChanged()
    {
        updateUi();
    }
    void onTrackColorMarkerLineWidthValueChanged()
    {
        updateUi();
    }
    void onTrackNumberSizeValueChanged()
    {
        updateUi();
    }
    void onTrackGroundPositionSizeValueChanged()
    {
        updateUi();
    }
    void onTrackGroundPathSizeValueChanged()
    {
        updateUi();
    }
    void onTrackShowBeforeValueChanged()
    {
        updateUi();
    }
    void onTrackShowAfterValueChanged()
    {
        updateUi();
    }
    void onMoCapSizeValueChanged(int i);

    void onRecoMethodCurrentIndexChanged();
    void onRecoMethodChanged(reco::RecognitionMethod method);
    void onMlMethodCurrentIndexChanged();
    void onMlMethodChanged(reco::MlMethod method);
    void onPerformRecognitionStateChanged(int i);
    void onMarkerBrightnessValueChanged();
    void onMarkerIgnoreWithoutStateChanged();
    void onRoiShowStateChanged(int i);

    void onIntrinsicParamsChanged(IntrinsicCameraParams params);

    void onTrackPathColorButtonClicked();
    void onTrackGroundPathColorButtonClicked();
    void onMoCapColorButtonClicked();

    void onShowVoronoiCellsStateChanged();

    void onTrackShowOnlyNrValueChanged();
    void onTrackShowOnlyNrListTextChanged();

    void onTrackShowOnlyListButtonClicked();

    void onTrackShowOnlyStateChanged(int arg1);
    void onTrackShowOnlyListStateChanged(int arg1);

    void onTrackShowMarkerIDsStateChanged(int arg1);

    void onTrackRoiShowStateChanged(int arg1);

    void setMoCapShow(bool visibility);
    void setMoCapColor(QColor col);
    void setMoCapSize(int size);
    void toggleRecoROIButtons();
    void toggleTrackROIButtons();

public slots:
    void imageSizeChanged(int width, int height, int borderDiff);


signals:
    void userChangedRecoMethod(reco::RecognitionMethod method);
    void userChangedMlMethod(reco::MlMethod method);


private:
    Petrack             *mMainWindow;
    Ui::Control         *mUi;
    IntrinsicBox        *mIntr;
    FilterBeforeBox     *mFilterBefore;
    ExtrinsicBox        *mExtr;
    CoordinateSystemBox *mCoordSys;
    AlignmentGridBox    *mGrid;
    QGraphicsScene      *mScene;
    Correction          *mCorrectionWidget;

    bool mColorChanging;
    bool mIndexChanging; // shows, if the index of the color model is really changing; nor while constructor (initialer
    // durchlauf) and may be while loading xml file
    bool mLoading; // shows, if new project is just loading
};

#endif
