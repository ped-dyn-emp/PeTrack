#ifndef CONTROL_H
#define CONTROL_H

#include <QtWidgets>

#include "ui_control.h"

class Petrack;
//class Image;
class QGraphicsScene;
class QDomElement;

class Control: public QWidget, public Ui::Control
{
    Q_OBJECT

public:
    Control(QWidget *parent = 0);

    void setScene(QGraphicsScene *sc);

    bool getTrackShow();
    void setTrackShow(bool b);
    bool getTrackFix();
    void setTrackFix(bool b);
    QColor getTrackPathColor();
    void setTrackPathColor(QColor col);
    QColor getTrackGroundPathColor();
    void setTrackGroundPathColor(QColor col);

    bool getRecoRoiShow();
    void setRecoRoiShow(bool b);
    bool getRecoRoiFix();
    void setRecoRoiFix(bool b);

    bool getTrackRoiShow();
    void setTrackRoiShow(bool b);
    bool getTrackRoiFix();
    void setTrackRoiFix(bool b);

    int getFilterBorderSize();

    double getCalibFxValue();
    void setCalibFxValue(double d);
    void setCalibFxMin(double d);
    void setCalibFxMax(double d);
    double getCalibFyValue();
    void setCalibFyValue(double d);
    void setCalibFyMin(double d);
    void setCalibFyMax(double d);
    double getCalibCxValue();
    void setCalibCxValue(double d);
    void setCalibCxMin(double d);
    void setCalibCxMax(double d);
    double getCalibCyValue();
    void setCalibCyValue(double d);
    void setCalibCyMin(double d);
    void setCalibCyMax(double d);
    double getCalibR2Value();
    void setCalibR2Value(double d);
    void setCalibR2Min(double d);
    void setCalibR2Max(double d);
    double getCalibR4Value();
    void setCalibR4Value(double d);
    void setCalibR4Min(double d);
    void setCalibR4Max(double d);
    double getCalibTxValue();
    void setCalibTxValue(double d);
    void setCalibTxMin(double d);
    void setCalibTxMax(double d);
    double getCalibTyValue();
    void setCalibTyValue(double d);
    void setCalibTyMin(double d);
    void setCalibTyMax(double d);
    double getCalibR6Value();
    void setCalibR6Value(double d);
    void setCalibR6Min(double d);
    void setCalibR6Max(double d);
    double getCalibK4Value();
    void setCalibK4Value(double d);
    void setCalibK4Min(double d);
    void setCalibK4Max(double d);
    double getCalibK5Value();
    void setCalibK5Value(double d);
    void setCalibK5Min(double d);
    void setCalibK5Max(double d);
    double getCalibK6Value();
    void setCalibK6Value(double d);
    void setCalibK6Min(double d);
    void setCalibK6Max(double d);

    double getCalibExtrRot1();
    void setCalibExtrRot1(double d);
    double getCalibExtrRot2();
    void setCalibExtrRot2(double d);
    double getCalibExtrRot3();
    void setCalibExtrRot3(double d);
    double getCalibExtrTrans1();
    void setCalibExtrTrans1(double d);
    double getCalibExtrTrans2();
    void setCalibExtrTrans2(double d);
    double getCalibExtrTrans3();
    void setCalibExtrTrans3(double d);
    void setEnabledExtrParams(bool enable);

    int /*Petrack::Dimension*/ getCalibCoordDimension();
    bool getCalibExtrCalibPointsShow();
    bool getCalibExtrVanishPointsShow();
    bool getCalibCoordShow();
    void setCalibCoordShow(bool b);
    bool getCalibCoordFix();
    void setCalibCoordFix(bool b);
    bool getIs3DView();
    void setIs3DView(bool b);
    int getCalibCoordRotate();
    void setCalibCoordRotate(int i);
    int getCalibCoordTransX();
    void setCalibCoordTransX(int i);
    int getCalibCoordTransXMax();
    void setCalibCoordTransXMax(int i);
    int getCalibCoordTransXMin();
    void setCalibCoordTransXMin(int i);
    int getCalibCoordTransY();
    void setCalibCoordTransY(int i);
    int getCalibCoordTransYMax();
    void setCalibCoordTransYMax(int i);
    int getCalibCoordTransYMin();
    void setCalibCoordTransYMin(int i);
    int getCalibCoordScale();
    void setCalibCoordScale(int i);
    double getCalibCoordUnit();
    void setCalibCoordUnit(double d);

    int getCalibCoord3DTransX();
    void setCalibCoord3DTransX(int i);
    int getCalibCoord3DTransY();
    void setCalibCoord3DTransY(int i);
    int getCalibCoord3DTransZ();
    void setCalibCoord3DTransZ(int i);
    int getCalibCoord3DAxeLen();
    void setCalibCoord3DAxeLen(int i);
    bool getCalibCoord3DSwapX();
    void setCalibCoord3DSwapX(bool b);
    bool getCalibCoord3DSwapY();
    void setCalibCoord3DSwapY(bool b);
    bool getCalibCoord3DSwapZ();
    void setCalibCoord3DSwapZ(bool b);

    int /*Petrack::Dimension*/ getCalibGridDimension();
    bool getCalibGridShow();
    void setCalibGridShow(bool b);
    bool getCalibGridFix();
    void setCalibGridFix(bool b);
    int getCalibGridRotate();
    void setCalibGridRotate(int i);
    int getCalibGridTransX();
    void setCalibGridTransX(int i);
    int getCalibGridTransY();
    void setCalibGridTransY(int i);
    int getCalibGridScale();
    void setCalibGridScale(int i);

    int getCalibGrid3DTransX();
    void setCalibGrid3DTransX(int i);
    int getCalibGrid3DTransY();
    void setCalibGrid3DTransY(int i);
    int getCalibGrid3DTransZ();
    void setCalibGrid3DTransZ(int i);
    void setGridMinMaxTranslation(int minx, int maxx, int miny, int maxy);
    int getCalibGrid3DResolution();
    void setCalibGrid3DResolution(int i);

    void expandRange(QColor& fromColor, QColor& toColor, const std::array<int, 3>& clickedColor);
    void saveChange(const QColor& fromColor, const QColor& toColor, RectPlotItem* map);
    bool getColors(QPoint& p, GraphicsView* graphicsView, std::array<int, 3>& clickedColor, QColor& toColor, QColor& fromColor, RectPlotItem*& map);

    void setXml(QDomElement &elem);
    void getXml(QDomElement &elem);
    bool isLoading()
    {
        return mMainWindow->isLoading();
    }
    inline ColorPlot *getColorPlot() const
    {
        return colorPlot;
    }

#ifdef QWT
    inline AnalysePlot *getAnalysePlot() const
    {
        return analysePlot;
    }
#endif

    inline Petrack *getMainWindow() const
    {
        return mMainWindow;
    }

    // 0 == Kaserne, 1 == Hermes, 2 == Ohne, 3 == Color, 4 == Japan
    inline int getRecoMethod()
    {
        return recoMethod->currentIndex();
    }

private slots:
    //temp:
//     void on_temp1_valueChanged(int i);
//     void on_temp2_valueChanged(int i);
//     void on_temp3_valueChanged(int i);
//     void on_temp4_valueChanged(int i);
//     void on_temp5_valueChanged(int i);

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

    void on_mapNr_valueChanged(int i);
    void on_mapX_valueChanged(int i);
    void on_mapY_valueChanged(int i);
    void on_mapW_valueChanged(int i);
    void on_mapH_valueChanged(int i);
    void on_mapColor_stateChanged(int i);
    void on_mapHeight_valueChanged(double d);
    void on_mapAdd_clicked();
    void on_mapDel_clicked();
    void on_mapDistribution_clicked();
    void on_mapResetHeight_clicked();
    void on_mapResetPos_clicked();
    void on_mapDefaultHeight_valueChanged(double d);
    void on_expandColor(QPoint p, GraphicsView* graphicsView);
    void on_setColor(QPoint, GraphicsView*);


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

    void on_trackShowOnlyVisible_stateChanged(int /*i*/) { if (!isLoading()) mScene->update(); }
//    void on_trackShowOnlyNr_valueChanged(int i) { if (!isLoading()) mScene->update(); }
    void on_trackShowCurrentPoint_stateChanged(int /*i*/) { if (!isLoading()) mScene->update(); }
    void on_trackGotoNr_clicked();
    void on_trackGotoStartNr_clicked();
    void on_trackGotoEndNr_clicked();

    void on_trackHeadSized_stateChanged(int i);

    void on_trackShowPoints_stateChanged(int /*i*/) { if (!isLoading()) mScene->update(); }
    void on_trackShowPointsColored_stateChanged(int /*i*/) { if (!isLoading()) mScene->update(); }
    void on_trackShowPath_stateChanged(int /*i*/) { if (!isLoading()) mScene->update(); }
    void on_trackShowColColor_stateChanged(int /*i*/) { if (!isLoading()) mScene->update(); }
    void on_trackShowColorMarker_stateChanged(int /*i*/) { if (!isLoading()) mScene->update(); }
    void on_trackShowNumber_stateChanged(int /*i*/) { if (!isLoading()) mScene->update(); }
    void on_trackShowGroundPosition_stateChanged(int /*i*/) { if (!isLoading()) mScene->update(); }
    void on_trackShowGroundPath_stateChanged(int /*i*/) { if (!isLoading()) mScene->update(); }
    void on_trackShowHeightIndividual_stateChanged(int /*i*/) { if (!isLoading()) mScene->update(); }
    void on_trackNumberBold_stateChanged(int /*i*/) { if (!isLoading()) mScene->update(); }

    void on_trackCurrentPointSize_valueChanged(int /*i*/) { if (!isLoading()) mScene->update(); }
    void on_trackPointSize_valueChanged(int /*i*/) { if (!isLoading()) mScene->update(); }
    void on_trackPathWidth_valueChanged(int /*i*/) { if (!isLoading()) mScene->update(); }
    void on_trackColColorSize_valueChanged(int /*i*/) { if (!isLoading()) mScene->update(); }
    void on_trackColorMarkerSize_valueChanged(int /*i*/) { mScene->update(); }
    void on_trackNumberSize_valueChanged(int /*i*/) { if (!isLoading()) mScene->update(); }
    void on_trackGroundPositionSize_valueChanged(int /*i*/) { if (!isLoading()) mScene->update(); }
    void on_trackGroundPathSize_valueChanged(int /*i*/) { if (!isLoading()) mScene->update(); }
    void on_trackShowBefore_valueChanged(int /*i*/) { if (!isLoading()) mScene->update(); }
    void on_trackShowAfter_valueChanged(int /*i*/) { if (!isLoading()) mScene->update(); }

    void on_performRecognition_stateChanged(int i);
    void on_markerBrightness_valueChanged(int i);
    void on_markerIgnoreWithout_stateChanged(int i);
    void on_roiShow_stateChanged(int i);
    void on_roiFix_stateChanged(int i);

    void on_filterBrightContrast_stateChanged(int i);
//     void on_filterContrast_stateChanged(int i);
    void on_filterContrastParam_valueChanged(int i);
//     void on_filterBright_stateChanged(int i);
    void on_filterBrightParam_valueChanged(int i);
    void on_filterBorder_stateChanged(int i);
    void on_filterBorderParamSize_valueChanged(int i);
    void on_filterBorderParamCol_clicked();
//    void on_filterBgCalc_clicked();
    void on_filterBg_stateChanged(int i);
    void on_filterBgShow_stateChanged(int i);
    void on_filterBgUpdate_stateChanged(int i);
    void on_filterBgReset_clicked();
    void on_filterBgSave_clicked();
    void on_filterBgLoad_clicked();
    void on_filterSwap_stateChanged(int i);
    void on_filterSwapH_stateChanged(int i);
    void on_filterSwapV_stateChanged(int i);

    void on_apply_stateChanged(int i);
    void on_fx_valueChanged(double d);
    void on_fy_valueChanged(double d);
    void on_cx_valueChanged(double d);
    void on_cy_valueChanged(double d);
    void on_r2_valueChanged(double d);
    void on_r4_valueChanged(double d);
    void on_r6_valueChanged(double d);
    void on_tx_valueChanged(double d);
    void on_ty_valueChanged(double d);
    /// Weitere Entzerrungsparameter fuer spaeter
    //void on_k4_valueChanged(double d);
    //void on_k5_valueChanged(double d);
    //void on_k6_valueChanged(double d);
    void on_quadAspectRatio_stateChanged(int i);
    void on_fixCenter_stateChanged(int i);
    void on_tangDist_stateChanged(int i);
    void on_autoCalib_clicked();
    void on_calibFiles_clicked();

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

    void on_showVoronoiCells_stateChanged(int arg1);

    void on_trackShowOnlyNr_valueChanged(int i);
    void on_trackShowOnlyNrList_textChanged(const QString &arg1);

    void on_trackShowOnlyListButton_clicked();

    void on_trackShowOnly_stateChanged(int arg1);
    void on_trackShowOnlyList_stateChanged(int arg1);

    void on_trackRoiShow_stateChanged(int arg1);

    void on_trackRoiFix_stateChanged(int arg1);

    //void on_trackExport_released();

    void on_colorPickerButton_clicked(bool checked);

private:
    Petrack *mMainWindow;
    QGraphicsScene *mScene;
    bool mColorChanging;
    bool mIndexChanging; // shows, if the index of the color model is really changing; nor while constructor (initialer durchlauf) and may be while loading xml file
    bool mLoading; // shows, if new project is just loading
    QDoubleSpinBox *k4,*k5,*k6; // Muss noch in die Oberflaeche eingebaut werden
    };

#endif
