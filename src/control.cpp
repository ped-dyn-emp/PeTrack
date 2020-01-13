#include <QDomElement>
#include <iomanip>

#include "control.h"
#include "petrack.h"
#include "tracker.h"
#include "trackerItem.h"
#include "imageItem.h"
#include "backgroundItem.h"
#include "colorPlot.h"
#include "analysePlot.h"
#include "player.h"
#include "stereoWidget.h"
#include "colorRangeWidget.h"
#include "colorMarkerWidget.h"
#include "codeMarkerWidget.h"
#include "multiColorMarkerWidget.h"

#define DEFAULT_HEIGHT 180.0

Control::Control(QWidget *parent)
    : QWidget(parent)
{
    setAccessibleName("Control");
    mMainWindow = (class Petrack*) parent;
    mScene = NULL;
    mLoading = false;
    //beim erzeugen von new colorplot absturz!!!!
    setupUi(this);

    // Weitere Verzerrungsparameter werden vllt. spaeter mal gebraucht bisher von OpenCV nicht beruecksichtig. Muessen dann noch in die Oberflaeche eingebaut werden
    k4 = new QDoubleSpinBox();
    k5 = new QDoubleSpinBox();
    k6 = new QDoubleSpinBox();

    filterBrightContrast->setCheckState(mMainWindow->getBrightContrastFilter()->getEnabled() ? Qt::Checked : Qt::Unchecked);
//     filterContrast->setCheckState(mMainWindow->getContrastFilter()->getEnabled() ? Qt::Checked : Qt::Unchecked);
//     filterBright->setCheckState(mMainWindow->getBrightFilter()->getEnabled() ? Qt::Checked : Qt::Unchecked);
    filterBorder->setCheckState(mMainWindow->getBorderFilter()->getEnabled() ? Qt::Checked : Qt::Unchecked);
    filterBg->setCheckState(mMainWindow->getBackgroundFilter()->getEnabled() ? Qt::Checked : Qt::Unchecked);
    apply->setCheckState(mMainWindow->getCalibFilter()->getEnabled() ? Qt::Checked : Qt::Unchecked);

    filterSwap->setCheckState(mMainWindow->getSwapFilter()->getEnabled() ? Qt::Checked : Qt::Unchecked);
    filterSwapH->setCheckState((bool) mMainWindow->getSwapFilter()->getSwapHorizontally()->getValue() ? Qt::Checked : Qt::Unchecked);
    filterSwapV->setCheckState((bool) mMainWindow->getSwapFilter()->getSwapVertically()->getValue() ? Qt::Checked : Qt::Unchecked);

    setCalibFxMin(mMainWindow->getCalibFilter()->getFx()->getMinimum());
    setCalibFxMax(mMainWindow->getCalibFilter()->getFx()->getMaximum());
    setCalibFxValue(mMainWindow->getCalibFilter()->getFx()->getValue());

    setCalibFyMin(mMainWindow->getCalibFilter()->getFy()->getMinimum());
    setCalibFyMax(mMainWindow->getCalibFilter()->getFy()->getMaximum());
    setCalibFyValue(mMainWindow->getCalibFilter()->getFy()->getValue());

//    setCalibCxMin(mMainWindow->getCalibFilter()->getCx()->getMinimum());
//    setCalibCxMax(mMainWindow->getCalibFilter()->getCx()->getMaximum());
    setCalibCxValue(mMainWindow->getCalibFilter()->getCx()->getValue());

//    setCalibCyMin(mMainWindow->getCalibFilter()->getCy()->getMinimum());
//    setCalibCyMax(mMainWindow->getCalibFilter()->getCy()->getMaximum());
    setCalibCyValue(mMainWindow->getCalibFilter()->getCy()->getValue());

    setCalibR2Min(mMainWindow->getCalibFilter()->getR2()->getMinimum());
    setCalibR2Max(mMainWindow->getCalibFilter()->getR2()->getMaximum());
    setCalibR2Value(mMainWindow->getCalibFilter()->getR2()->getValue());

    setCalibR4Min(mMainWindow->getCalibFilter()->getR4()->getMinimum());
    setCalibR4Max(mMainWindow->getCalibFilter()->getR4()->getMaximum());
    setCalibR4Value(mMainWindow->getCalibFilter()->getR4()->getValue());

    setCalibR6Min(mMainWindow->getCalibFilter()->getR6()->getMinimum());
    setCalibR6Max(mMainWindow->getCalibFilter()->getR6()->getMaximum());
    setCalibR6Value(mMainWindow->getCalibFilter()->getR6()->getValue());

    setCalibTxMin(mMainWindow->getCalibFilter()->getTx()->getMinimum());
    setCalibTxMax(mMainWindow->getCalibFilter()->getTx()->getMaximum());
    setCalibTxValue(mMainWindow->getCalibFilter()->getTx()->getValue());

    setCalibTyMin(mMainWindow->getCalibFilter()->getTy()->getMinimum());
    setCalibTyMax(mMainWindow->getCalibFilter()->getTy()->getMaximum());
    setCalibTyValue(mMainWindow->getCalibFilter()->getTy()->getValue());

    setCalibK4Min(mMainWindow->getCalibFilter()->getK4()->getMinimum());
    setCalibK4Max(mMainWindow->getCalibFilter()->getK4()->getMaximum());
    setCalibK4Value(mMainWindow->getCalibFilter()->getK4()->getValue());

    setCalibK5Min(mMainWindow->getCalibFilter()->getK5()->getMinimum());
    setCalibK5Max(mMainWindow->getCalibFilter()->getK5()->getMaximum());
    setCalibK5Value(mMainWindow->getCalibFilter()->getK5()->getValue());

    setCalibK6Min(mMainWindow->getCalibFilter()->getK6()->getMinimum());
    setCalibK6Max(mMainWindow->getCalibFilter()->getK6()->getMaximum());
    setCalibK6Value(mMainWindow->getCalibFilter()->getK6()->getValue());
    //statt folgender Zeile kann zB on_cx_valueChanged einfach kodiert werden (su)
    //  connect(cx, SIGNAL(valueChanged(double cx)), this, SLOT(on_cx_valueChanged));

    //wird vom designer erledigt: colorPlot->setParent(colorBox); // da nur ueber frame in designer integriert

    colorPlot->setControlWidget(this);

    mIndexChanging = false;
    mColorChanging = true;
    recoColorModel->addItem("HSV");
    recoColorModel->addItem("RGB");

    recoColorX->addItem("H");
    recoColorX->addItem("S");
    recoColorX->addItem("V");

    recoColorY->addItem("H");
    recoColorY->addItem("S");
    recoColorY->addItem("V");
    recoColorY->setCurrentIndex(1); // default

    mColorChanging = false;

    on_recoColorModel_currentIndexChanged(0);

    // damit eine rectMap vorliegt, die angezeigt werden kann
    colorPlot->getMapItem()->addMap();

    analysePlot->setControlWidget(this);
    //analysePlot->setTrackerReal(mMainWindow->getTrackerReal());

    mIndexChanging = true;

    recoMethod->addItem("marker casern"); // 0
    recoMethod->addItem("marker hermes"); // 1
    recoMethod->addItem("without marker"); // 2
    recoMethod->addItem("color marker"); // 3
    recoMethod->addItem("marker Japan"); // 4
    recoMethod->addItem("multicolor marker"); // 5
    recoMethod->addItem("code marker"); // 6

    recoMethod->setCurrentIndex(5); // default multicolor marker (until 11/2016 hermes marker)
}

void Control::setScene(QGraphicsScene *sc)
{
    mScene = sc;
}

//---------------------------------------

bool Control::getTrackShow()
{
    return trackShow->isChecked();
}
void Control::setTrackShow(bool b)
{
    trackShow->setChecked(b);
}

bool Control::getTrackFix()
{
    return trackFix->isChecked();
}
void Control::setTrackFix(bool b)
{
    trackFix->setChecked(b);
}

QColor Control::getTrackPathColor()
{
    return trackPathColorButton->palette().color(QPalette::Button);
}
void Control::setTrackPathColor(QColor col)
{
    QPalette pal = trackPathColorButton->palette();
    pal.setColor(QPalette::Button, col);
    trackPathColorButton->setPalette(pal);
    //trackPathColorButton->setAutoFillBackground(true);
    //trackPathColorButton->update();
}
QColor Control::getTrackGroundPathColor()
{
    return trackGroundPathColorButton->palette().color(QPalette::Button);
}
void Control::setTrackGroundPathColor(QColor col)
{
    QPalette pal = trackGroundPathColorButton->palette();
    pal.setColor(QPalette::Button, col);
    trackGroundPathColorButton->setPalette(pal);
    //trackPathColorButton->setAutoFillBackground(true);
    //trackPathColorButton->update();
}

//---------------------------------------

bool Control::getRecoRoiShow()
{
    return roiShow->isChecked();
}
void Control::setRecoRoiShow(bool b)
{
    roiShow->setChecked(b);
}
bool Control::getRecoRoiFix()
{
    return roiFix->isChecked();
}
void Control::setRecoRoiFix(bool b)
{
    roiFix->setChecked(b);
}

bool Control::getTrackRoiShow()
{
    return trackRoiShow->isChecked();
}
void Control::setTrackRoiShow(bool b)
{
    trackRoiShow->setChecked(b);
}
bool Control::getTrackRoiFix()
{
    return trackRoiFix->isChecked();
}
void Control::setTrackRoiFix(bool b)
{
    trackRoiFix->setChecked(b);
}

//---------------------------------------

int Control::getFilterBorderSize()
{
    return filterBorderParamSize->value();
}

double Control::getCalibFxValue()
{
    return fx->value();
}
void Control::setCalibFxValue(double d)
{
    fx->setValue(d);
}
void Control::setCalibFxMin(double d)
{
    fx->setMinimum(d);
}
void Control::setCalibFxMax(double d)
{
    fx->setMaximum(d);
}

double Control::getCalibFyValue()
{
    return fy->value();
}
void Control::setCalibFyValue(double d)
{
    fy->setValue(d);
}
void Control::setCalibFyMin(double d)
{
    fy->setMinimum(d);
}
void Control::setCalibFyMax(double d)
{
    fy->setMaximum(d);
}

double Control::getCalibCxValue()
{
    return cx->value();
}
void Control::setCalibCxValue(double d)
{
//    if( d > cx->maximum() )
//        cx->setMaximum(d);
//    if( d < cx->minimum() )
//        cx->setMinimum(d);
    cx->setValue(d);
}
void Control::setCalibCxMin(double d)
{
    cx->setMinimum(d);
}
void Control::setCalibCxMax(double d)
{
    cx->setMaximum(d);
}

double Control::getCalibCyValue()
{
    return cy->value();
}
void Control::setCalibCyValue(double d)
{
//    if( d > cy->maximum() )
//        cy->setMaximum(d);
//    if( d < cy->minimum() )
//        cy->setMinimum(d);
    cy->setValue(d);
}
void Control::setCalibCyMin(double d)
{
    cy->setMinimum(d);
}
void Control::setCalibCyMax(double d)
{
    cy->setMaximum(d);
}

double Control::getCalibR2Value()
{
    return r2->value();
}
void Control::setCalibR2Value(double d)
{
    r2->setValue(d);
}
void Control::setCalibR2Min(double d)
{
    r2->setMinimum(d);
}
void Control::setCalibR2Max(double d)
{
    r2->setMaximum(d);
}

double Control::getCalibR4Value()
{
    return r4->value();
}
void Control::setCalibR4Value(double d)
{
    r4->setValue(d);
}
void Control::setCalibR4Min(double d)
{
    r4->setMinimum(d);
}
void Control::setCalibR4Max(double d)
{
    r4->setMaximum(d);
}

double Control::getCalibR6Value()
{
    return r6->value();
}
void Control::setCalibR6Value(double d)
{
    r6->setValue(d);
}
void Control::setCalibR6Min(double d)
{
    r6->setMinimum(d);
}
void Control::setCalibR6Max(double d)
{
    r6->setMaximum(d);
}

double Control::getCalibExtrRot1()
{
    return rot1->value();
}
void Control::setCalibExtrRot1(double d)
{
    rot1->setValue(d);
}
double Control::getCalibExtrRot2()
{
    return rot2->value();
}
void Control::setCalibExtrRot2(double d)
{
    rot2->setValue(d);
}
double Control::getCalibExtrRot3()
{
    return rot3->value();
}
void Control::setCalibExtrRot3(double d)
{
    rot3->setValue(d);
}
double Control::getCalibExtrTrans1()
{
    return trans1->value();
}
void Control::setCalibExtrTrans1(double d)
{
    trans1->setValue(d);
}
double Control::getCalibExtrTrans2()
{
    return trans2->value();
}
void Control::setCalibExtrTrans2(double d)
{
    trans2->setValue(d);
}
double Control::getCalibExtrTrans3()
{
    return trans3->value();
}
void Control::setCalibExtrTrans3(double d)
{
    trans3->setValue(d);
}

double Control::getCalibTxValue()
{
    return tx->value();
}
void Control::setCalibTxValue(double d)
{
    tx->setValue(d);
}
void Control::setCalibTxMin(double d)
{
    tx->setMinimum(d);
}
void Control::setCalibTxMax(double d)
{
    tx->setMaximum(d);
}

double Control::getCalibTyValue()
{
    return ty->value();
}
void Control::setCalibTyValue(double d)
{
    ty->setValue(d);
}
void Control::setCalibTyMin(double d)
{
    ty->setMinimum(d);
}
void Control::setCalibTyMax(double d)
{
    ty->setMaximum(d);
}

double Control::getCalibK4Value()
{
    return k4->value();
}
void Control::setCalibK4Value(double d)
{
    k4->setValue(d);
}
void Control::setCalibK4Min(double d)
{
    k4->setMinimum(d);
}
void Control::setCalibK4Max(double d)
{
    k4->setMaximum(d);
}

double Control::getCalibK5Value()
{
    return k5->value();
}
void Control::setCalibK5Value(double d)
{
    k5->setValue(d);
}
void Control::setCalibK5Min(double d)
{
    k5->setMinimum(d);
}
void Control::setCalibK5Max(double d)
{
    k5->setMaximum(d);
}

double Control::getCalibK6Value()
{
    return k6->value();
}
void Control::setCalibK6Value(double d)
{
    k6->setValue(d);
}
void Control::setCalibK6Min(double d)
{
    k6->setMinimum(d);
}
void Control::setCalibK6Max(double d)
{
    k6->setMaximum(d);
}

//---------------------------------------

int /*Petrack::Dimension*/ Control::getCalibGridDimension()
{
    return gridTab->currentIndex();
}
bool Control::getCalibGridShow()
{
    return gridShow->isChecked();
}
void Control::setCalibGridShow(bool b)
{
    gridShow->setChecked(b);
}

bool Control::getCalibGridFix()
{
    return gridFix->isChecked();
}
void Control::setCalibGridFix(bool b)
{
    gridFix->setChecked(b);
}

int Control::getCalibGridRotate()
{
    return gridRotate->value();
}
void Control::setCalibGridRotate(int i)
{
    gridRotate->setValue(i);
}

int Control::getCalibGridTransX()
{
    return gridTransX->value();
}
void Control::setCalibGridTransX(int i)
{
    gridTransX->setValue(i);
}

int Control::getCalibGridTransY()
{
    return gridTransY->value();
}
void Control::setCalibGridTransY(int i)
{
    gridTransY->setValue(i);
}

int Control::getCalibGridScale()
{
    return gridScale->value();
}
void Control::setCalibGridScale(int i)
{
    gridScale->setValue(i);
}

void Control::setEnabledExtrParams(bool enable){
    rot1->setEnabled(enable);
    rot2->setEnabled(enable);
    rot3->setEnabled(enable);
    trans1->setEnabled(enable);
    trans2->setEnabled(enable);
    trans3->setEnabled(enable);
}

void Control::setGridMinMaxTranslation(int minx, int maxx, int miny, int maxy)
{
    grid3DTransX->setMinimum(minx);
    grid3DTransX_spin->setMinimum(minx);
    grid3DTransX->setMaximum(maxx);
    grid3DTransX_spin->setMaximum(maxx);
    grid3DTransY->setMinimum(miny);
    grid3DTransY_spin->setMinimum(miny);
    grid3DTransY->setMaximum(maxy);
    grid3DTransY_spin->setMaximum(maxy);
    grid3DTransZ->setMinimum(-200);
    grid3DTransZ_spin->setMinimum(-200);
    grid3DTransZ->setMaximum( 500);
    grid3DTransZ_spin->setMaximum( 500);
}

int Control::getCalibGrid3DTransX()
{
    return grid3DTransX->value();
}
void Control::setCalibGrid3DTransX(int i)
{
    grid3DTransX->setValue(i);
}
int Control::getCalibGrid3DTransY()
{
    return grid3DTransY->value();
}
void Control::setCalibGrid3DTransY(int i)
{
    grid3DTransY->setValue(i);
}
int Control::getCalibGrid3DTransZ()
{
    return grid3DTransZ->value();
}
void Control::setCalibGrid3DTransZ(int i)
{
    grid3DTransZ->setValue(i);
}
int Control::getCalibGrid3DResolution()
{
    return grid3DResolution->value();
}
void Control::setCalibGrid3DResolution(int i)
{
    grid3DResolution->setValue(i);
}


//---------------------------------------

int/*Petrack::Dimension*/ Control::getCalibCoordDimension()
{
    return coordTab->currentIndex();
}
bool Control::getCalibExtrCalibPointsShow()
{
    return extCalibPointsShow->isChecked();
}
bool Control::getCalibExtrVanishPointsShow()
{
    return extVanishPointsShow->isChecked();
}

bool Control::getCalibCoordShow()
{
    return coordShow->isChecked();
}
void Control::setCalibCoordShow(bool b)
{
    coordShow->setChecked(b);
}

bool Control::getCalibCoordFix()
{
    return coordFix->isChecked();
}
void Control::setCalibCoordFix(bool b)
{
    coordFix->setChecked(b);
}
int Control::getCalibCoordRotate()
{
    return coordRotate->value();
}
void Control::setCalibCoordRotate(int i)
{
    coordRotate->setValue(i);
}
int Control::getCalibCoordTransX()
{
    return coordTransX->value();
}
void Control::setCalibCoordTransX(int i)
{
    coordTransX->setValue(i);
}

int Control::getCalibCoordTransXMax()
{
    return coordTransX->maximum();
}
void Control::setCalibCoordTransXMax(int i)
{
    coordTransX->setMaximum(i);
    coordTransX_spin->setMaximum(i);
}

int Control::getCalibCoordTransXMin()
{
    return coordTransX->minimum();
}
void Control::setCalibCoordTransXMin(int i)
{
    coordTransX->setMinimum(i);
    coordTransX_spin->setMinimum(i);
}

int Control::getCalibCoordTransY()
{
    return coordTransY->value();
}
void Control::setCalibCoordTransY(int i)
{
    coordTransY->setValue(i);
}

int Control::getCalibCoordTransYMax()
{
    return coordTransY->maximum();
}
void Control::setCalibCoordTransYMax(int i)
{
    coordTransY->setMaximum(i);
    coordTransY_spin->setMaximum(i);
}

int Control::getCalibCoordTransYMin()
{
    return coordTransY->minimum();
}
void Control::setCalibCoordTransYMin(int i)
{
    coordTransY->setMinimum(i);
    coordTransY_spin->setMinimum(i);
}

int Control::getCalibCoordScale()
{
    return coordScale->value();
}
void Control::setCalibCoordScale(int i)
{
    coordScale->setValue(i);
}

double Control::getCalibCoordUnit()
{
    return coordUnit->value();
}
void Control::setCalibCoordUnit(double d)
{
    coordUnit->setValue(d);
}

int Control::getCalibCoord3DTransX()
{
    return coord3DTransX->value();
}
void Control::setCalibCoord3DTransX(int i)
{
    coord3DTransX->setValue(i);
}
int Control::getCalibCoord3DTransY()
{
    return coord3DTransY->value();
}
void Control::setCalibCoord3DTransY(int i)
{
    coord3DTransY->setValue(i);
}
int Control::getCalibCoord3DTransZ()
{
    return coord3DTransZ->value();
}
void Control::setCalibCoord3DTransZ(int i)
{
    coord3DTransZ->setValue(i);
}
int Control::getCalibCoord3DAxeLen()
{
    return coord3DAxeLen->value();
}
void Control::setCalibCoord3DAxeLen(int i)
{
    coord3DAxeLen->setValue(i);
}
bool Control::getCalibCoord3DSwapX()
{
    return coord3DSwapX->isChecked();
}
void Control::setCalibCoord3DSwapX(bool b)
{
    coord3DSwapX->setChecked(b);
}
bool Control::getCalibCoord3DSwapY()
{
    return coord3DSwapY->isChecked();
}
void Control::setCalibCoord3DSwapY(bool b)
{
    coord3DSwapY->setChecked(b);
}
bool Control::getCalibCoord3DSwapZ()
{
    return coord3DSwapZ->isChecked();
}
void Control::setCalibCoord3DSwapZ(bool b)
{
    coord3DSwapZ->setChecked(b);
}

//--------------------------------------- automatic generated slots -----------------------

// void Control::on_temp1_valueChanged(int i)
// {
//     debout << i <<endl;
//     mMainWindow->getBorderFilter()->setChanged(true);// just to flag changes
//     mMainWindow->updateImage();
// }

// void Control::on_temp2_valueChanged(int i)
// {
//     debout << i <<endl;
//     mMainWindow->getBorderFilter()->setChanged(true);// just to flag changes
//     mMainWindow->updateImage();
// }

// void Control::on_temp3_valueChanged(int i)
// {
//     debout << i <<endl;
//     mMainWindow->getBorderFilter()->setChanged(true);// just to flag changes
//     mMainWindow->updateImage();
// }

// void Control::on_temp4_valueChanged(int i)
// {
//     debout << i <<endl;
//     mMainWindow->getBorderFilter()->setChanged(true);// just to flag changes
//     mMainWindow->updateImage();
// }

// void Control::on_temp5_valueChanged(int i)
// {
//     debout << i <<endl;
//     mMainWindow->getBorderFilter()->setChanged(true);// just to flag changes
//     mMainWindow->updateImage();
// }

//-------------------- analysation

void Control::on_anaCalculate_clicked()
{
    mMainWindow->calculateRealTracker();
    analysePlot->setScale();
    if (!isLoading())
        analysePlot->replot();
//     getTrackerItem()->show();
//     mapNr->setMaximum(mapNr->maximum()+1);
//     colorPlot->getMapItem()->addMap();
//     mapNr->setValue(mapNr->maximum());
}

void Control::on_anaStep_valueChanged(int i)
{
    if (!isLoading())
        analysePlot->replot();
}
void Control::on_anaMarkAct_stateChanged(int i)
{
    if (!isLoading())
        analysePlot->replot();
}
void Control::on_anaConsiderX_stateChanged(int i)
{
    if ((i == Qt::Checked) && (anaConsiderY->isChecked()))
    {
        anaConsiderAbs->setEnabled(false);
        anaConsiderRev->setEnabled(false);
    }
    else
    {
        anaConsiderAbs->setEnabled(true);
        anaConsiderRev->setEnabled(true);
    }
    if ((i == Qt::Unchecked) && (!anaConsiderY->isChecked()))
        anaConsiderX->setCheckState(Qt::Checked);
    else
    {
        if (!isLoading())
            analysePlot->replot();
    }
}
void Control::on_anaConsiderY_stateChanged(int i)
{
    if ((i == Qt::Checked) && (anaConsiderX->isChecked()))
    {
        anaConsiderAbs->setEnabled(false);
        anaConsiderRev->setEnabled(false);
    }
    else
    {
        anaConsiderAbs->setEnabled(true);
        anaConsiderRev->setEnabled(true);
    }
    if ((i == Qt::Unchecked) && (!anaConsiderX->isChecked()))
        anaConsiderY->setCheckState(Qt::Checked);
    else
    {
        if (!isLoading())
            analysePlot->replot();
    }
}
void Control::on_anaConsiderAbs_stateChanged(int i)
{
    if (!isLoading())
        analysePlot->replot();
}
void Control::on_anaConsiderRev_stateChanged(int i)
{
    if (!isLoading())
        analysePlot->replot();
}
void Control::on_showVoronoiCells_stateChanged(int arg1)
{
    if (!isLoading())
        mMainWindow->getScene()->update();
}


//------------------- tracking

void Control::on_trackShow_stateChanged(int i)
{
    if (i == Qt::Checked)
        mMainWindow->getTrackerItem()->show();
    else if (i == Qt::Unchecked)
        mMainWindow->getTrackerItem()->hide();
}
void Control::on_trackFix_stateChanged(int i)
{
    // erst sinnvoll, wenn einzelne Punkte verschiebbar sind
//     if (i == Qt::Checked)
//     {
//         //mMainWindow->getRecoRoiItem()->setAcceptsHoverEvents(false); wird in item gemacht
//         mMainWindow->getRecoRoiItem()->setFlags(!QGraphicsItem::ItemIsMovable);
//     }
//     else if (i == Qt::Unchecked)
//     {
//         mMainWindow->getRecoRoiItem()->setAcceptsHoverEvents(true);
//         mMainWindow->getRecoRoiItem()->setFlags(QGraphicsItem::ItemIsMovable);
//     }
}

void Control::on_trackOnlineCalc_stateChanged(int i)
{
    if (i == Qt::Checked)
    {
        mMainWindow->setTrackChanged(true);// flag changes of track parameters
        mMainWindow->getTracker()->reset();
        if( !mMainWindow->isLoading() )
            mMainWindow->updateImage();
    }
}
void Control::on_trackCalc_clicked()
{
    //    mMainWindow->getTracker()->init(); // wenn vorherige Daten weggenommen werden sollen
    mMainWindow->trackAll();
}
void Control::on_trackReset_clicked()
{
    if (mMainWindow->getImage())
    {
        cv::Size size;
        size.width = mMainWindow->getTrackRoiItem()->rect().width();
        size.height = mMainWindow->getTrackRoiItem()->rect().height();
        mMainWindow->setTrackChanged(true);// flag changes of track parameters
        mMainWindow->setRecognitionChanged(true);// flag changes of recognition parameters
        mMainWindow->getTracker()->init(size);
        if( !mMainWindow->isLoading() )
            mMainWindow->updateImage();
    }
}
void Control::on_trackExport_clicked()
{
    mMainWindow->exportTracker();
}
void Control::on_trackImport_clicked()
{
    mMainWindow->importTracker();
}
void Control::on_trackTest_clicked()
{
    mMainWindow->testTracker();
}
void Control::on_trackPathColorButton_clicked()
{
    QColor col = QColorDialog::getColor(getTrackPathColor(), this);
    if( col.isValid() )
    {
        setTrackPathColor(col);
    }
    if( !mMainWindow->isLoading() )
        mMainWindow->updateImage();
}
void Control::on_trackGroundPathColorButton_clicked()
{
    QColor col = QColorDialog::getColor(getTrackGroundPathColor(), this);
    if( col.isValid() )
    {
        setTrackGroundPathColor(col);
    }
    if( !mMainWindow->isLoading() )
        mMainWindow->updateImage();
}

void Control::on_trackRegionScale_valueChanged(int i) 
{ 
    mMainWindow->setTrackChanged(true); 
    mMainWindow->getTracker()->reset(); 
    if( !mMainWindow->isLoading() )
        mMainWindow->updateImage();
}
void Control::on_trackRegionLevels_valueChanged(int i) 
{ 
    mMainWindow->setTrackChanged(true); 
    mMainWindow->getTracker()->reset(); 
    if( !mMainWindow->isLoading() )
        mMainWindow->updateImage();
}
void Control::on_trackShowSearchSize_stateChanged(int i) 
{ 
    mScene->update(); 
}

void Control::on_trackShowOnly_stateChanged(int i)
{
    if( i > 0 && trackShowOnlyList->checkState() == Qt::Checked)
        trackShowOnlyList->setChecked(false);

    if (!isLoading()) mScene->update();
}
void Control::on_trackShowOnlyList_stateChanged(int i)
{
    if( i > 0 && trackShowOnly->checkState() == Qt::Checked)
        trackShowOnly->setChecked(false);

    trackShowOnlyListButton->setEnabled(i);
    trackShowOnlyNrList->setEnabled(i);

    if (!isLoading()) mScene->update();
}

void Control::on_trackShowOnlyNr_valueChanged(int i)
{
    if (!mMainWindow->isLoading())
        mScene->update();
//        mMainWindow->updateImage();
}

void Control::on_trackShowOnlyNrList_textChanged(const QString &arg1)
{
    if (!mMainWindow->isLoading())
        mScene->update();
//        mMainWindow->updateImage();
}
void Control::on_trackGotoNr_clicked()
{
    if (mMainWindow->getTracker()->size() >= trackShowOnlyNr->value())
    {
//        int firstMin = mMainWindow->getAnimation()->getMaxFrames(), lastMax = 0;
//        foreach(int onlyVisible, mMainWindow->getOnlyVisible() )
//        {
//            if(mMainWindow->getTracker()->at(onlyVisible).firstFrame() < firstMin)
//                firstMin = mMainWindow->getTracker()->at(onlyVisible).firstFrame();
//            if(mMainWindow->getTracker()->at(onlyVisible).lastFrame() > lastMax)
//                lastMax  = mMainWindow->getTracker()->at(onlyVisible).lastFrame();
//        }
        int idx = trackShowOnlyNr->value()-1;
        int firstFrame = mMainWindow->getTracker()->at(idx).firstFrame();
        int lastFrame = mMainWindow->getTracker()->at(idx).lastFrame();
        mMainWindow->getPlayer()->skipToFrame((lastFrame+firstFrame)/2);
    }
}
void Control::on_trackGotoStartNr_clicked()
{
    if (mMainWindow->getTracker()->size() >= trackShowOnlyNr->value())
    {
//        int start = mMainWindow->getAnimation()->getMaxFrames();
//        foreach(int onlyVisible, mMainWindow->getOnlyVisible() )
//        {
//            if(mMainWindow->getTracker()->at(onlyVisible).firstFrame() < start)
//                start = mMainWindow->getTracker()->at(onlyVisible).firstFrame();
//        }
        int idx = trackShowOnlyNr->value()-1;
        mMainWindow->getPlayer()->skipToFrame(mMainWindow->getTracker()->at(idx).firstFrame());
    }
}
void Control::on_trackGotoEndNr_clicked()
{
    if (mMainWindow->getTracker()->size() >= trackShowOnlyNr->value())
    {
//        int end = 0;
//        foreach(int onlyVisible, mMainWindow->getOnlyVisible() )
//        {
//            if(mMainWindow->getTracker()->at(onlyVisible).lastFrame() > end)
//                end = mMainWindow->getTracker()->at(onlyVisible).lastFrame();
//        }
        int idx = trackShowOnlyNr->value()-1;
        mMainWindow->getPlayer()->skipToFrame(mMainWindow->getTracker()->at(idx).lastFrame());
    }
}

void Control::on_trackShowOnlyListButton_clicked()
{

        QMessageBox nrListBox(mMainWindow);

        nrListBox.setWindowTitle(Petrack::tr("PeTrack"));
        nrListBox.setIcon(QMessageBox::NoIcon);
        nrListBox.setText(Petrack::tr("Select visible pedestrians:"));

        QGridLayout* layout = (QGridLayout*) nrListBox.layout();
//        QVBoxLayout* layout = (QVBoxLayout*) nrListBox.layout();
        QVector<QCheckBox*> checkBox;

        for(int i = 0; i < mMainWindow->getTracker()->size(); i++)
        {
            /// ToDo: parse from lineEdit
            checkBox.push_back(new QCheckBox(QString::number(i+1)));
            checkBox.at(i)->setChecked(mMainWindow->getOnlyVisible().contains(i));
//            layout->addWidget(checkBox.at(i));
            layout->addWidget(checkBox.at(i),3+i/5,i%5,1,1);
        }


        QPushButton *ok = new QPushButton(tr("Ok"));
        QPushButton *cancel = new QPushButton(tr("Cancel"));

//        layout->addWidget(ok,5,1,1,1);
//        layout->addWidget(cancel,5,2,1,1);

        nrListBox.addButton(cancel,QMessageBox::RejectRole);
        nrListBox.addButton(ok,QMessageBox::AcceptRole);
//        nrListBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
//        nrListBox.setDefaultButton(QMessageBox::Ok);

        int res = nrListBox.exec();

        if (res == QMessageBox::Accepted)
        {
//            debout << "ok" << endl;
            QStringList list;
            int first = -1, last = -1;
            for(int i = 0; i < mMainWindow->getTracker()->size(); i++)
            {
                if (checkBox.at(i)->isChecked()){
                    if (first == -1)
                        first = i+1;
//                    list.append(QString::number(i+1));
//                debout << (i+1) << ": " << checkBox.at(i)->isChecked() << endl;
                }else // not checked
                {
                    if(first != -1)
                    {
                        last = i;
                        if(first == last)
                            list.append(QString::number(first));
                        else
                            list.append(QString("%1-%2").arg(first).arg(last));
                        last = -1;
                        first = -1;
                    }
                }
                /// ToDo: lists
            }
            trackShowOnlyNrList->setText(list.join(","));

        }/*else if (res == QMessageBox::Rejected)
        {
            debout << "cancel" << endl;
        }*/

}
void Control::on_trackHeadSized_stateChanged(int i) 
{ 
    static int oldHeadSize = 60;
    if (i == Qt::Checked)
    {
        oldHeadSize = trackCurrentPointSize->value();
        trackCurrentPointSize->setValue((int) mMainWindow->getHeadSize());
        trackCurrentPointSize->setDisabled(true);
    }
    else
    {
        trackCurrentPointSize->setValue(oldHeadSize);
        trackCurrentPointSize->setEnabled(true);
    }
    mScene->update(); 
}

//------------------- recognition

void Control::on_recoShowColor_stateChanged(int i)
{
    colorPlot->getTrackerItem()->setVisible(i == Qt::Checked);
    if (!isLoading())
        colorPlot->replot();
}
void Control::on_recoOptimizeColor_clicked()
{
    mMainWindow->getTracker()->optimizeColor();
    colorPlot->replot();
    mScene->update(); // damit mgl angezeige farbpunkte geaendert/weggenommen werden
}
void Control::on_recoColorModel_currentIndexChanged(int index)
{
    static int xHsvIndex = 0; // H
    static int yHsvIndex = 1; // S
    static int xRgbIndex = 0; // R
    static int yRgbIndex = 1; // G
    //static bool firstTime = true;

    if (mColorChanging == false) // damit nicht vom constructor mehrmals bei additem durchlaufen wird
    {
        mColorChanging = true;
        if (index == 0) // HSV
        {
            recoColorX->setItemText(0, "H");
            recoColorX->setItemText(1, "S");
            recoColorX->setItemText(2, "V");
        
            recoColorY->setItemText(0, "H");
            recoColorY->setItemText(1, "S");
            recoColorY->setItemText(2, "V");

            if (mIndexChanging) // !firstTime
            {
                xRgbIndex = recoColorX->currentIndex();
                yRgbIndex = recoColorY->currentIndex();
                recoColorX->setCurrentIndex(xHsvIndex);
                recoColorY->setCurrentIndex(yHsvIndex);
            }
        }
        else //index == 1 == RGB
        {
            recoColorX->setItemText(0, "R");
            recoColorX->setItemText(1, "G");
            recoColorX->setItemText(2, "B");
        
            recoColorY->setItemText(0, "R");
            recoColorY->setItemText(1, "G");
            recoColorY->setItemText(2, "B");

            if (mIndexChanging) // !firstTime
            {
                xHsvIndex = recoColorX->currentIndex();
                yHsvIndex = recoColorY->currentIndex();
                recoColorX->setCurrentIndex(xRgbIndex);
                recoColorY->setCurrentIndex(yRgbIndex);
            }
        }
        mIndexChanging = true; // firstTime = false
        mColorChanging = false;

        //recoColorModel->currentIndex(), recoColorX->currentIndex(), recoColorY->currentIndex(), recoColorZ->value()
        colorPlot->setScale();
        colorPlot->generateImage();
//         colorPlot->updateTracker();
        if (!isLoading())
            colorPlot->replot();
    }
}

void Control::on_recoColorX_currentIndexChanged(int index)
{
    if (mColorChanging == false)
    {
        colorPlot->setScale();
        colorPlot->generateImage();
//         colorPlot->updateTracker();
        if (!isLoading())
            colorPlot->replot();
    }
    // schoen waere hier das gegenstueck bei y zu disablen, aber combobox bietet dies nicht auf einfachem weg
//         recoColorY->setItemText(0, "R");
//         recoColorY->setItemText(1, "G");
//         recoColorY->setItemText(2, "B");
}

void Control::on_recoColorY_currentIndexChanged(int index)
{
    if (mColorChanging == false)
    {
        colorPlot->setScale();
        colorPlot->generateImage();
//         colorPlot->updateTracker();
        if (!isLoading())
            colorPlot->replot();
    }
}

void Control::on_recoColorZ_valueChanged(int index)
{
    if (mColorChanging == false)
    {
        colorPlot->generateImage();
        if (!isLoading())
            colorPlot->replot();
    }
}

void Control::on_recoGreyLevel_valueChanged(int index)
{
    colorPlot->setGreyDiff(index); //getTrackerItem()->
    if (!isLoading())
        colorPlot->replot();
}

void Control::on_recoSymbolSize_valueChanged(int index)
{
    colorPlot->setSymbolSize(index); //getTrackerItem()->
    if (!isLoading())
        colorPlot->replot();
}
void Control::on_recoStereoShow_clicked()
{
    if (recoMethod->currentIndex() == 5)
        mMainWindow->getMultiColorMarkerWidget()->show();
    else if (recoMethod->currentIndex() == 3)
        mMainWindow->getColorMarkerWidget()->show();
    else if (recoMethod->currentIndex() == 6)
        mMainWindow->getCodeMarkerWidget()->show();
    else
        mMainWindow->getStereoWidget()->show();
}

void Control::on_mapColorRange_clicked()
{
    //if (recoMethod->currentIndex() == 3)
    //    mMainWindow->getColorMarkerWidget()->show();
    //else
    mMainWindow->getColorRangeWidget()->show();
}

void Control::on_mapNr_valueChanged(int i)
{
//    bool isloading = mMainWindow->isLoading();
//    mMainWindow->setLoading(true);
//    debout << "i=" << i << endl;
    RectMap map = colorPlot->getMapItem()->getMap(i);

    mapX->setValue(myRound(2.*map.x()));
    mapY->setValue(myRound(2.*map.y()));
    mapW->setValue(myRound(map.width()));
    mapH->setValue(myRound(map.height()));
    mapColor->setCheckState(map.colored() ? Qt::Checked : Qt::Unchecked);
    mapHeight->setValue(map.mapHeight());  
    colorPlot->getMapItem()->changeMap(mapNr->value(), mapX->value()/2., mapY->value()/2., mapW->value(), mapH->value(), 
                                       mapColor->isChecked(), mapHeight->value()); // nur, um aktivenindex anzugeben
    mMainWindow->getColorRangeWidget()->setToColor(map.toColor()); //on_toColor_clicked();on_toTriangle_colorChanged
    mMainWindow->getColorRangeWidget()->setFromColor(map.fromColor()); //on_fromColor_clicked();on_fromTriangle_colorChanged
    mMainWindow->getColorRangeWidget()->setInvHue(map.invHue()); //on_fromColor_clicked();on_fromTriangle_colorChanged
//    mMainWindow->setLoading(false);
    if (!isLoading())
    {
        colorPlot->replot(); // um aktiven gruen anzuzeigen
        mMainWindow->updateImage();
    }
//    mMainWindow->setLoading(isloading);
}
void Control::on_mapX_valueChanged(int i)
{
    colorPlot->getMapItem()->changeMap(mapNr->value(), mapX->value()/2., mapY->value()/2., mapW->value(), mapH->value(), 
                                       mapColor->isChecked(), mapHeight->value());
    if (!isLoading())
        colorPlot->replot();
}
void Control::on_mapY_valueChanged(int i)
{
    colorPlot->getMapItem()->changeMap(mapNr->value(), mapX->value()/2., mapY->value()/2., mapW->value(), mapH->value(), 
                                       mapColor->isChecked(), mapHeight->value());
    if (!isLoading())
        colorPlot->replot();
}
void Control::on_mapW_valueChanged(int i)
{
    colorPlot->getMapItem()->changeMap(mapNr->value(), mapX->value()/2., mapY->value()/2., mapW->value(), mapH->value(), 
                                       mapColor->isChecked(), mapHeight->value());
    if (!isLoading())
        colorPlot->replot();
}
void Control::on_mapH_valueChanged(int i)
{
    colorPlot->getMapItem()->changeMap(mapNr->value(), mapX->value()/2., mapY->value()/2., mapW->value(), mapH->value(), 
                                       mapColor->isChecked(), mapHeight->value());
    if (!isLoading())
        colorPlot->replot();
}
void Control::on_mapColor_stateChanged(int i)
{
    colorPlot->getMapItem()->changeMap(mapNr->value(), mapX->value()/2., mapY->value()/2., mapW->value(), mapH->value(), 
                                       mapColor->isChecked(), mapHeight->value());
    if (!isLoading())
        colorPlot->replot();
}
void Control::on_mapHeight_valueChanged(double d)
{
    colorPlot->getMapItem()->changeMap(mapNr->value(), mapX->value()/2., mapY->value()/2., mapW->value(), mapH->value(), 
                                       mapColor->isChecked(), mapHeight->value());
    //colorPlot->replot(); Anzeige aendert sich nichts
}
void Control::on_mapAdd_clicked()
{
    mapNr->setMaximum(mapNr->maximum()+1);
    colorPlot->getMapItem()->addMap();
    mapNr->setValue(mapNr->maximum());
}
void Control::on_mapDel_clicked()
{
    colorPlot->getMapItem()->delMap(mapNr->value());
    if (mapNr->value() == mapNr->maximum())
        mapNr->setValue(mapNr->value()>0 ? mapNr->maximum()-1 : 0);

    mapNr->setMaximum(mapNr->maximum()>0 ? mapNr->maximum()-1 : 0);

    if (!isLoading())
        colorPlot->replot();
}
void Control::on_mapDistribution_clicked()
{
    if (!colorPlot->printDistribution())
        mMainWindow->getTracker()->printHeightDistribution();
}
void Control::on_mapResetHeight_clicked()
{
    mMainWindow->getTracker()->resetHeight();
    mScene->update();
}
void Control::on_mapResetPos_clicked()
{
    mMainWindow->getTracker()->resetPos();
    mScene->update();
}
void Control::on_mapDefaultHeight_valueChanged(double d)
{
    mMainWindow->setHeadSize();
    mMainWindow->getBackgroundFilter()->setDefaultHeight(d);
}
void Control::on_performRecognition_stateChanged(int i)
{
    mMainWindow->setRecognitionChanged(true);// flag changes of recognition parameters
    if( !mMainWindow->isLoading() )
        mMainWindow->updateImage();
}

void Control::on_markerBrightness_valueChanged(int i)
{
    mMainWindow->setRecognitionChanged(true);// flag changes of recognition parameters
    if( !mMainWindow->isLoading() )
        mMainWindow->updateImage();
}
void Control::on_markerIgnoreWithout_stateChanged(int i)
{
    mMainWindow->setRecognitionChanged(true);// flag changes of recognition parameters
    if( !mMainWindow->isLoading() )
        mMainWindow->updateImage();
}

void Control::on_roiShow_stateChanged(int i)
{
    if (i == Qt::Checked)
        mMainWindow->getRecoRoiItem()->show(); // setVisible
    else if (i == Qt::Unchecked)
        mMainWindow->getRecoRoiItem()->hide(); // setVisible
    //mScene->update();
}
void Control::on_roiFix_stateChanged(int i)
{
    if (i == Qt::Checked)
    {
        //mMainWindow->getRecoRoiItem()->setAcceptsHoverEvents(false); wird in item gemacht
        //mMainWindow->getRecoRoiItem()->setFlags(!QGraphicsItem::ItemIsMovable);
        mMainWindow->getRecoRoiItem()->setFlag(QGraphicsItem::ItemIsMovable, false);
    }
    else if (i == Qt::Unchecked)
    {
        mMainWindow->getRecoRoiItem()->setAcceptHoverEvents(true);
        //mMainWindow->getRecoRoiItem()->setFlags(QGraphicsItem::ItemIsMovable);
        mMainWindow->getRecoRoiItem()->setFlag(QGraphicsItem::ItemIsMovable);
    }
    //mScene->update();
}

void Control::on_trackRoiShow_stateChanged(int i)
{
    if (i == Qt::Checked)
        mMainWindow->getTrackRoiItem()->show(); // setVisible
    else if (i == Qt::Unchecked)
        mMainWindow->getTrackRoiItem()->hide(); // setVisible
    //mScene->update();
}

void Control::on_trackRoiFix_stateChanged(int i)
{
    if (i == Qt::Checked)
    {
        //mMainWindow->getRecoRoiItem()->setAcceptsHoverEvents(false); wird in item gemacht
        //mMainWindow->getRecoRoiItem()->setFlags(!QGraphicsItem::ItemIsMovable);
        mMainWindow->getTrackRoiItem()->setFlag(QGraphicsItem::ItemIsMovable, false);
    }
    else if (i == Qt::Unchecked)
    {
        mMainWindow->getTrackRoiItem()->setAcceptHoverEvents(true);
        //mMainWindow->getRecoRoiItem()->setFlags(QGraphicsItem::ItemIsMovable);
        mMainWindow->getTrackRoiItem()->setFlag(QGraphicsItem::ItemIsMovable);
    }
    //mScene->update();
}

//---------------------- calibration

void Control::on_filterBrightContrast_stateChanged(int i)
{
    if (i == Qt::Checked)
        mMainWindow->getBrightContrastFilter()->enable();
    else if (i == Qt::Unchecked)
        mMainWindow->getBrightContrastFilter()->disable();
    if( !mMainWindow->isLoading() )
        mMainWindow->updateImage();
}

// void Control::on_filterContrast_stateChanged(int i)
// {
//     if (i == Qt::Checked)
//         mMainWindow->getContrastFilter()->enable();
//     else if (i == Qt::Unchecked)
//         mMainWindow->getContrastFilter()->disable();
//     mMainWindow->updateImage();
// }
void Control::on_filterContrastParam_valueChanged(int i)
{
    mMainWindow->getBrightContrastFilter()->getContrast()->setValue(i);
    if( !mMainWindow->isLoading() )
        mMainWindow->updateImage();
}
// void Control::on_filterBright_stateChanged(int i)
// {
//     if (i == Qt::Checked)
//         mMainWindow->getBrightFilter()->enable();
//     else if (i == Qt::Unchecked)
//         mMainWindow->getBrightFilter()->disable();
//     mMainWindow->updateImage();
// }
void Control::on_filterBrightParam_valueChanged(int i)
{
    //debout << "test i: "<< i << " isLoading: " << mMainWindow->isLoading() << endl;
    mMainWindow->getBrightContrastFilter()->getBrightness()->setValue(i);
    if( !mMainWindow->isLoading() )
        mMainWindow->updateImage();
}
void Control::on_filterBorder_stateChanged(int i)
{
    if (i == Qt::Checked)
        mMainWindow->getBorderFilter()->enable();
    else if (i == Qt::Unchecked)
        mMainWindow->getBorderFilter()->disable();
    if( !mMainWindow->isLoading() )
        mMainWindow->updateImage();
}
void Control::on_filterBorderParamSize_valueChanged(int i)
{
//    debout << "test: " << i << endl;
    mMainWindow->setImageBorderSize(2*i); // 2* because undistored has problem with sizes not dividable  of 4
    if( !mMainWindow->isLoading() )
        mMainWindow->updateImage();
//    debout << "test: out" << endl;
}
void Control::on_filterBorderParamCol_clicked()
{
    BorderFilter *bf = mMainWindow->getBorderFilter();
    QColor color = QColorDialog::getColor(QColor((int)bf->getBorderColR()->getValue(), (int)bf->getBorderColG()->getValue(), (int)bf->getBorderColB()->getValue()), this);
    bf->getBorderColR()->setValue(color.red());
    bf->getBorderColG()->setValue(color.green());
    bf->getBorderColB()->setValue(color.blue());
    if( !mMainWindow->isLoading() )
        mMainWindow->updateImage();
}
void Control::on_filterSwap_stateChanged(int i)
{
    if (i == Qt::Checked)
        mMainWindow->getSwapFilter()->enable();
    else if (i == Qt::Unchecked)
        mMainWindow->getSwapFilter()->disable();
    if( !mMainWindow->isLoading() )
        mMainWindow->updateImage();
}
void Control::on_filterSwapH_stateChanged(int i)
{
    if (i == Qt::Checked)
        mMainWindow->getSwapFilter()->getSwapHorizontally()->setValue(1);
    else if (i == Qt::Unchecked)
        mMainWindow->getSwapFilter()->getSwapHorizontally()->setValue(0);
    if( !mMainWindow->isLoading() )
        mMainWindow->updateImage();
}
void Control::on_filterSwapV_stateChanged(int i)
{
    if (i == Qt::Checked)
        mMainWindow->getSwapFilter()->getSwapVertically()->setValue(1);
    else if (i == Qt::Unchecked)
        mMainWindow->getSwapFilter()->getSwapVertically()->setValue(0);
    if( !mMainWindow->isLoading() )
        mMainWindow->updateImage();
}
//void Control::on_filterBgCalc_clicked()
//{
//    mMainWindow->calcBackground();
//    //BorderFilter *bf = mMainWindow->getBorderFilter();
//    //mMainWindow->updateImage();
//}
void Control::on_filterBg_stateChanged(int i)
{

    ////////
    // Bei der Umstellung auf die aktuellste OpenCV Version 3 haben sich die
    // Funktionsaufrufe/Interfaces stark geaendert. Da eine Anpassung viel Zeit kosten wuerde,
    // jedoch der Filter sehr selten genutzt wird, wird er bis auf weiteres nicht verfuegbar sein.
//    if (i == Qt::Checked)
//    {
//        debout << "The BackgroundFilter is temporary not available!" << endl;
//        QMessageBox::warning(mMainWindow, Petrack::tr("Petrack"), Petrack::tr("BackgroundFilter is temporary not available!"));
//        filterBg->setCheckState(Qt::Unchecked);
//    }
//    if(false){ /// temporary disabled backgroundfilter
    ////////
    static int showCheckState = filterBgShow->checkState();


    if (i == Qt::Checked)
    {
        filterBgShow->setEnabled(true);
        filterBgUpdate->setEnabled(true);
        filterBgReset->setEnabled(true);
        filterBgSave->setEnabled(true);
        filterBgLoad->setEnabled(true);
        if (showCheckState == Qt::Checked)
            filterBgShow->setCheckState(Qt::Checked);
        filterBgDeleteNumber->setEnabled(true);
        filterBgDeleteTrj->setEnabled(true);
        mMainWindow->getBackgroundFilter()->enable();
    }
    else if (i == Qt::Unchecked)
    {
        filterBgShow->setEnabled(false);
        filterBgUpdate->setEnabled(false);
        filterBgReset->setEnabled(false);
        filterBgSave->setEnabled(false);
        filterBgLoad->setEnabled(false);
        showCheckState = filterBgShow->checkState();
        filterBgShow->setCheckState(Qt::Unchecked);
        filterBgDeleteNumber->setEnabled(false);
        filterBgDeleteTrj->setEnabled(false);
        mMainWindow->getBackgroundFilter()->disable();
    }
    if( !mMainWindow->isLoading() )
        mMainWindow->updateImage(true); // true, da auch bei stehendem bild neue berechnungen durchgefuehrt werden sollen
//    }/// temporary disabled backgroundfilter
}
void Control::on_filterBgUpdate_stateChanged(int i)
{
    if (i == Qt::Checked)
        mMainWindow->getBackgroundFilter()->setUpdate(true);//  enable();
    else if (i == Qt::Unchecked)
        mMainWindow->getBackgroundFilter()->setUpdate(false);//->disable();
    //mMainWindow->updateImage();
}
void Control::on_filterBgReset_clicked()
{
    mMainWindow->getBackgroundFilter()->reset();
    if( !mMainWindow->isLoading() )
        mMainWindow->updateImage();
}
void Control::on_filterBgShow_stateChanged(int i)
{
    mMainWindow->getBackgroundItem()->setVisible(i);
    //mMainWindow->getScene()->update();
    mScene->update();
    //mMainWindow->updateImage();
}
void Control::on_filterBgSave_clicked()
{
    mMainWindow->getBackgroundFilter()->save();
}
void Control::on_filterBgLoad_clicked()
{
    mMainWindow->getBackgroundFilter()->load();
    if( !mMainWindow->isLoading() )
        mMainWindow->updateImage();
}

void Control::on_apply_stateChanged(int i)
{
    if (i == Qt::Checked)
        mMainWindow->getCalibFilter()->enable();
    else if (i == Qt::Unchecked)
        mMainWindow->getCalibFilter()->disable();
    if( !mMainWindow->isLoading() )
        mMainWindow->updateImage();
}
void Control::on_fx_valueChanged(double d)
{
    //cout << (mMainWindow->accessibleName()).toStdString() <<endl;
    //cout << accessibleName().toStdString() <<endl;
    //mMainWindow->statusBar()->showMessage((new QString)->setNum(d)); //
    mMainWindow->getCalibFilter()->getFx()->setValue(d);
    if (quadAspectRatio->isChecked())
        fy->setValue(d);
    if( !mMainWindow->isLoading() )
        mMainWindow->updateImage();
    setMeasuredAltitude();
}
void Control::on_fy_valueChanged(double d)
{
    mMainWindow->getCalibFilter()->getFy()->setValue(d);
    if( !mMainWindow->isLoading() )
        mMainWindow->updateImage();
    setMeasuredAltitude();
}
void Control::on_cx_valueChanged(double d)
{
    mMainWindow->setStatusPosReal();
    mMainWindow->getCalibFilter()->getCx()->setValue(d);
    if( !mMainWindow->isLoading() )
    {
        mMainWindow->updateCoord();
        //mMainWindow->updateImage();
    }
}
void Control::on_cy_valueChanged(double d)
{
    mMainWindow->setStatusPosReal();
    mMainWindow->getCalibFilter()->getCy()->setValue(d);
    if( !mMainWindow->isLoading() )
    {
        mMainWindow->updateCoord();
        //mMainWindow->updateImage();
    }
}
void Control::on_r2_valueChanged(double d)
{
    mMainWindow->getCalibFilter()->getR2()->setValue(d);
    if( !mMainWindow->isLoading() )
        mMainWindow->updateImage();
}
void Control::on_r4_valueChanged(double d)
{
    mMainWindow->getCalibFilter()->getR4()->setValue(d);
    if( !mMainWindow->isLoading() )
        mMainWindow->updateImage();
}
void Control::on_r6_valueChanged(double d)
{
    mMainWindow->getCalibFilter()->getR6()->setValue(d);
    if( !mMainWindow->isLoading() )
        mMainWindow->updateImage();
}
void Control::on_tx_valueChanged(double d)
{
    mMainWindow->getCalibFilter()->getTx()->setValue(d);
    if( !mMainWindow->isLoading() )
        mMainWindow->updateImage();
}
void Control::on_ty_valueChanged(double d)
{
    mMainWindow->getCalibFilter()->getTy()->setValue(d);
    if( !mMainWindow->isLoading() )
        mMainWindow->updateImage();
}

void Control::on_quadAspectRatio_stateChanged(int i)
{
    static double oldFyValue = 0;

    if (i == Qt::Checked)
    {
        oldFyValue = fy->value();
        fy->setValue(fx->value());
        fy->setDisabled(true);
    }
    else if (i == Qt::Unchecked)
    {
        fy->setEnabled(true);
        fy->setValue(oldFyValue);
    }
}
void Control::on_fixCenter_stateChanged(int i)
{
    static double oldCxValue = 0;
    static double oldCyValue = 0;

    if (i == Qt::Checked)
    {
        oldCxValue = cx->value();
        oldCyValue = cy->value();
        if (mMainWindow->getImage())
        {
            cx->setValue((mMainWindow->getImage()->width()-1)/2.); // mgl auch iplimg,wenn bild vergroessert wird
            cy->setValue((mMainWindow->getImage()->height()-1)/2.);
        }
        cx->setDisabled(true);
        cy->setDisabled(true);
    }
    else if (i == Qt::Unchecked)
    {
        cx->setEnabled(true);
        cy->setEnabled(true);
        cx->setValue(oldCxValue);
        cy->setValue(oldCyValue);
    }
}
void Control::on_tangDist_stateChanged(int i)
{
    static double oldTxValue = 0;
    static double oldTyValue = 0;

    if (i == Qt::Checked)
    {
        tx->setEnabled(true);
        ty->setEnabled(true);
        tx->setValue(oldTxValue);
        ty->setValue(oldTyValue);
    }
    else if (i == Qt::Unchecked)
    {
        oldTxValue = tx->value();
        oldTyValue = ty->value();
        tx->setValue(0.);
        ty->setValue(0.);
        tx->setDisabled(true);
        ty->setDisabled(true);
    }
}

void Control::on_autoCalib_clicked()
{
    mMainWindow->getAutoCalib()->autoCalib();
}
void Control::on_calibFiles_clicked()
{
    if (mMainWindow->getAutoCalib()->openCalibFiles())
        autoCalib->setEnabled(true);
}
//---------------------------------------

void Control::on_rot1_valueChanged(double arg1)
{
    if (!isLoading())
        mMainWindow->updateCoord();
}
void Control::on_rot2_valueChanged(double arg1)
{
    if (!isLoading())
         mMainWindow->updateCoord();
}
void Control::on_rot3_valueChanged(double arg1)
{
    if (!isLoading())
        mMainWindow->updateCoord();
}
void Control::on_trans1_valueChanged(double arg1)
{
    if (!isLoading())
         mMainWindow->updateCoord();
}
void Control::on_trans2_valueChanged(double arg1)
{
    if (!isLoading())
        mMainWindow->updateCoord();
}
void Control::on_trans3_valueChanged(double arg1)
{
    if (!isLoading())
        mMainWindow->updateCoord();
}
void Control::on_extCalibPointsShow_stateChanged(int arg1)
{
    if (!isLoading())
         mScene->update();
}
void Control::on_extrCalibShowError_clicked()
{

    QString out;
    QMessageBox msgBox;

    if( mMainWindow->getExtrCalibration()->getReprojectionError().isEmpty() )
    {
        out = QString("No File for extrinisc calibration found!");
        msgBox.setInformativeText( out );

    }
    else
    {

        out = QString("<table>"
                      "<tr><th></th>"
                      "<th>average   &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</th>"
                      "<th>std. deviation                          &nbsp;</th>"
                      "<th>variance  &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</th>"
                      "<th>max       &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</th></tr>"
                "<tr><td>Point   height: &nbsp;&nbsp;            </td><td> %0 cm</td><td> %1 cm</td><td> %2 cm</td><td> %3 cm</td></tr>"
                "<tr><td>Default height: <small>[%12 cm]</small> </td><td> %4 cm</td><td> %5 cm</td><td> %6 cm</td><td> %7 cm</td></tr>"
                "<tr><td>Pixel    error: &nbsp;&nbsp;            </td><td> %8 px</td><td> %9 px</td><td> %10 px</td><td> %11 px</td></tr>"
                "</table>");
        QVector<double> reproError = mMainWindow->getExtrCalibration()->getReprojectionError();
        msgBox.setInformativeText( out.arg(reproError.at(0)).arg(reproError.at(1)).arg(reproError.at(2)).arg(reproError.at(3))
                                   .arg(reproError.at(4)).arg(reproError.at(5)).arg(reproError.at(6)).arg(reproError.at(7))
                                   .arg(reproError.at(8)).arg(reproError.at(9)).arg(reproError.at(10)).arg(reproError.at(11))
                                   .arg(reproError.at(12)));

    }
    QSpacerItem* horizontalSpacer = new QSpacerItem(500, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
    QGridLayout* layout = (QGridLayout*)msgBox.layout();
    msgBox.setWindowTitle("PeTrack");
    msgBox.setIcon(QMessageBox::Information);
    msgBox.setText("Reprojection error for extrinsic calibration:");
    layout->addItem(horizontalSpacer, layout->rowCount(), 0, 1, layout->columnCount());
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.setDefaultButton(QMessageBox::Ok);
    msgBox.exec();

}
void Control::on_extVanishPointsShow_stateChanged(int arg1)
{
    if (!isLoading())
        mScene->update();
}
void Control::on_coordLoad3DCalibPoints_clicked()
{
    //debout << "On_load3DCalibPoints clicked!" << endl;
    mMainWindow->getExtrCalibration()->openExtrCalibFile();

    mMainWindow->updateCoord();
    mScene->update();
}
void Control::on_extrCalibSave_clicked()
{
    mMainWindow->getExtrCalibration()->saveExtrCalibPoints();
}

void Control::on_extrCalibFetch_clicked()
{
    mMainWindow->getExtrCalibration()->fetch2DPoints();
}

void Control::on_extrCalibShowPoints_clicked()
{
    QString out_str;
    QTextStream out(&out_str);

    unsigned int i;

    out << "<table><tr><th>Nr.</th><th>3D.x</th><th>3D.y</th><th>3D.z</th><th>2D.x</th><th>2D.y</th></tr>" << endl;


    for (i = 0; i < max(mMainWindow->getExtrCalibration()->get3DList().size(),mMainWindow->getExtrCalibration()->get2DList().size()); ++i)
    {
        out << "<tr>";
        if( i < mMainWindow->getExtrCalibration()->get3DList().size() )
        {
            out << "<td>[" << QString::number(i+1,'i',0) << "]: </td><td>"
                << QString::number(mMainWindow->getExtrCalibration()->get3DList().at(i).x,'f',1) << "</td><td>"
                << QString::number(mMainWindow->getExtrCalibration()->get3DList().at(i).y,'f',1) << "</td><td>"
                << QString::number(mMainWindow->getExtrCalibration()->get3DList().at(i).z,'f',1) << "</td><td>";
        }else
        {
            out << "<td>-</td><td>-</td><td>-</td>";
        }
        if( i < mMainWindow->getExtrCalibration()->get2DList().size() )
        {
            out << QString::number(mMainWindow->getExtrCalibration()->get2DList().at(i).x,'f',3) << "</td><td>"
                << QString::number(mMainWindow->getExtrCalibration()->get2DList().at(i).y,'f',3) << "</td>";
        }else
        {
            out << "<td>-</td><td>-</td>";
        }
        out << "</tr>" <<  endl;
    }
    out << "</table>" << endl;

    QMessageBox msgBox;
    msgBox.setWindowTitle("PeTrack");
    msgBox.setIcon(QMessageBox::Information);
    msgBox.setText("Currently loaded point correspondences<br />for extrinsic calibration:");
    msgBox.setInformativeText( out_str );
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.setDefaultButton(QMessageBox::Ok);
    msgBox.exec();

}
//---------------------------------------
void Control::on_gridTab_currentChanged(int index)
{
    //cout << "on_gridTabView_currentchanged: index=" << index << endl;
    mScene->update();
}
void Control::on_gridShow_stateChanged(int i)
{
    mScene->update();
//     mMainWindow->getImageWidget()->update(); // repaint() zeichnet sofort - schneller aber mgl flicker
}
void Control::on_gridFix_stateChanged(int i)
{
    mScene->update();
}
void Control::on_gridRotate_valueChanged(int i)
{
    mScene->update();
}
void Control::on_gridTransX_valueChanged(int i)
{
    mScene->update();
}
void Control::on_gridTransY_valueChanged(int i)
{
    mScene->update();
}
void Control::on_gridScale_valueChanged(int i)
{
    mScene->update();
}
void Control::on_grid3DTransX_valueChanged(int value)
{
    if (!isLoading())
         mScene->update();
}
void Control::on_grid3DTransY_valueChanged(int value)
{
    if (!isLoading())
         mScene->update();
}

void Control::on_grid3DTransZ_valueChanged(int value)
{
    if (!isLoading())
         mScene->update();
}

void Control::on_grid3DResolution_valueChanged(int value)
{
    if (!isLoading())
         mScene->update();
}
//---------------------------------------
void Control::on_coordTab_currentChanged(int index)
{
    //debout << "on_coordTabView_currentchanged: index=" << index << endl;
    if( index == 1 ){
        setEnabledExtrParams(false);
        trackShowGroundPosition->setEnabled(false);
        trackShowGroundPath->setEnabled(false);
    }else{
        setEnabledExtrParams(true);
        trackShowGroundPosition->setEnabled(true);
        trackShowGroundPath->setEnabled(true);
    }
     if (!isLoading()){
        mMainWindow->updateCoord();
        mScene->update();
     }
}
void Control::on_coordShow_stateChanged(int i)
{
    if (!isLoading()){
        mMainWindow->updateCoord();
        mScene->update();
        setMeasuredAltitude(); // da measured nicht aktualisiert wird, waehrend scale verschoben und show deaktiviert und beim aktivieren sonst ein falscher wert zum angezeigten koord waere
    }
//     mScene->update(); //mScene->sceneRect() // ging auch, aber dann wurde zu oft matrix berechnet etc
//     mMainWindow->getImageWidget()->update(); // repaint() zeichnet sofort - schneller aber mgl flicker
}
void Control::on_coordFix_stateChanged(int i)
{
    if (!isLoading()){
        mMainWindow->updateCoord();
    }
}
void Control::on_coordRotate_valueChanged(int i)
{
    if (!isLoading()){
        mMainWindow->updateCoord();
    }
}
void Control::on_coordTransX_valueChanged(int i)
{
    if (!isLoading()){
        mMainWindow->updateCoord();
    }
}
void Control::on_coordTransY_valueChanged(int i)
{
    if (!isLoading()){
        mMainWindow->updateCoord();
    }
}
void Control::on_coordScale_valueChanged(int i)
{
    mMainWindow->updateCoord();
    setMeasuredAltitude();
    mMainWindow->setHeadSize();
}

void Control::on_coordAltitude_valueChanged(double d)
{
    mMainWindow->setHeadSize();
    mScene->update(); // fuer kreis um kopf, der mgl der realen kopfgroesse angepasst wird
}
void Control::on_coordUnit_valueChanged(double d)
{
    setMeasuredAltitude();
    mMainWindow->setHeadSize();
    mScene->update(); // fuer kreis um kopf, der mgl der realen kopfgroesse angepasst wird
}
void Control::on_coordUseIntrinsic_stateChanged(int i)
{
    mMainWindow->setStatusPosReal();
}

void Control::on_coord3DTransX_valueChanged(int value)
{
    if (!isLoading()){
        mMainWindow->updateCoord();
    }
}

void Control::on_coord3DTransY_valueChanged(int value)
{
    if (!isLoading()){
        mMainWindow->updateCoord();
    }
}

void Control::on_coord3DTransZ_valueChanged(int value)
{
    if (!isLoading()){
        mMainWindow->updateCoord();
    }
}

void Control::on_coord3DAxeLen_valueChanged(int value)
{
    if (!isLoading()){
        mMainWindow->updateCoord();
    }
}

void Control::on_coord3DSwapX_stateChanged(int arg1)
{
    if (!isLoading()){
        mMainWindow->updateCoord();
    }
}

void Control::on_coord3DSwapY_stateChanged(int arg1)
{
    if (!isLoading()){
        bool hans = isLoading();
        mMainWindow->updateCoord();
    }
}

void Control::on_coord3DSwapZ_stateChanged(int arg1)
{
    if (!isLoading()){
        mMainWindow->updateCoord();
    }
}
void Control::setMeasuredAltitude()
{
    if (mMainWindow->getImageItem())
        coordAltitudeMeasured->setText(QString("(measured: %1)").arg((getCalibFxValue()+getCalibFyValue())/2.*mMainWindow->getImageItem()->getCmPerPixel(), 6, 'f', 1));

}


//---------------------------------------

// store data in xml node
void Control::setXml(QDomElement &elem)
{
        QDomElement subElem;
        QDomElement subSubElem;
        QDomElement subSubSubElem;
        QString fn;

        elem.setAttribute("TAB", tabs->currentIndex());

        // - - - - - - - - - - - - - - - - - - - 
        subElem = (elem.ownerDocument()).createElement("CALIBRATION");
        elem.appendChild(subElem);

        subSubElem = (elem.ownerDocument()).createElement("BRIGHTNESS");
        subSubElem.setAttribute("ENABLED", filterBrightContrast->isChecked());
        subSubElem.setAttribute("VALUE", filterBrightParam->value());
        subElem.appendChild(subSubElem);

        subSubElem = (elem.ownerDocument()).createElement("CONTRAST");
        subSubElem.setAttribute("ENABLED", filterBrightContrast->isChecked());
        subSubElem.setAttribute("VALUE", filterContrastParam->value());
        subElem.appendChild(subSubElem);

        BorderFilter *bf = mMainWindow->getBorderFilter();
        QColor col((int)bf->getBorderColR()->getValue(), (int)bf->getBorderColG()->getValue(), (int)bf->getBorderColB()->getValue());
        subSubElem = (elem.ownerDocument()).createElement("BORDER");
        subSubElem.setAttribute("ENABLED", filterBorder->isChecked());
        subSubElem.setAttribute("VALUE", filterBorderParamSize->value());
        subSubElem.setAttribute("COLOR", col.name());
        subElem.appendChild(subSubElem);

        subSubElem = (elem.ownerDocument()).createElement("SWAP");
        subSubElem.setAttribute("ENABLED", filterSwap->isChecked());
        subSubElem.setAttribute("HORIZONTALLY", filterSwapH->isChecked());
        subSubElem.setAttribute("VERTICALLY", filterSwapV->isChecked());
        subElem.appendChild(subSubElem);

        subSubElem = (elem.ownerDocument()).createElement("BG_SUB");
        subSubElem.setAttribute("ENABLED", filterBg->isChecked());
        subSubElem.setAttribute("UPDATE", filterBgUpdate->isChecked());
        subSubElem.setAttribute("SHOW", filterBgShow->isChecked());
        fn = mMainWindow->getBackgroundFilter()->getFilename();
        if (fn != "")
            fn = getFileList(fn, mMainWindow->getProFileName());
        subSubElem.setAttribute("FILE", fn);
        subSubElem.setAttribute("DELETE", filterBgDeleteTrj->isChecked());
        subSubElem.setAttribute("DELETE_NUMBER", filterBgDeleteNumber->value());
        subElem.appendChild(subSubElem);

        subSubElem = (elem.ownerDocument()).createElement("PATTERN");
        subSubElem.setAttribute("BOARD_SIZE_X", mMainWindow->getAutoCalib()->getBoardSizeX()); // 6
        subSubElem.setAttribute("BOARD_SIZE_Y", mMainWindow->getAutoCalib()->getBoardSizeY()); // 8 oder 9
        subSubElem.setAttribute("SQUARE_SIZE", mMainWindow->getAutoCalib()->getSquareSize()); // in cm
        subElem.appendChild(subSubElem);

        subSubElem = (elem.ownerDocument()).createElement("INTRINSIC_PARAMETERS");
        subSubElem.setAttribute("ENABLED", apply->isChecked());
        subSubElem.setAttribute("FX", fx->value());
        subSubElem.setAttribute("FY", fy->value());
        subSubElem.setAttribute("CX", cx->value());
        subSubElem.setAttribute("CY", cy->value());
        subSubElem.setAttribute("R2", r2->value());
        subSubElem.setAttribute("R4", r4->value());
        subSubElem.setAttribute("R6", r6->value());
        subSubElem.setAttribute("TX", tx->value());
        subSubElem.setAttribute("TY", ty->value());
        subSubElem.setAttribute("K4", k4->value());
        subSubElem.setAttribute("K5", k5->value());
        subSubElem.setAttribute("K6", k6->value());
        subSubElem.setAttribute("QUAD_ASPECT_RATIO", quadAspectRatio->isChecked());
        subSubElem.setAttribute("FIX_CENTER", fixCenter->isChecked());
        subSubElem.setAttribute("TANG_DIST", tangDist->isChecked());
        // in dateiname darf kein , vorkommen - das blank ", " zur uebersich - beim einlesen wird nur ","
        // genommen und blanks rundherum abgeschnitten, falls von hand editiert wurde
        QStringList fl = mMainWindow->getAutoCalib()->getCalibFiles();
        for (int i = 0; i < fl.size(); ++i)
            if (QFileInfo(fl.at(i)).isRelative())
                fl.replace(i, fl.at(i)+";"+QFileInfo(fl.at(i)).absoluteFilePath());
        subSubElem.setAttribute("CALIB_FILES", fl.join(", "));
        subElem.appendChild(subSubElem);

        subSubElem = (elem.ownerDocument()).createElement("EXTRINSIC_PARAMETERS");

        subSubElem.setAttribute("EXTR_ROT_1", rot1->value());
        subSubElem.setAttribute("EXTR_ROT_2", rot2->value());
        subSubElem.setAttribute("EXTR_ROT_3", rot3->value());
        subSubElem.setAttribute("EXTR_TRANS_1", trans1->value());
        subSubElem.setAttribute("EXTR_TRANS_2", trans2->value());
        subSubElem.setAttribute("EXTR_TRANS_3", trans3->value());

        subSubElem.setAttribute("SHOW_CALIB_POINTS", extCalibPointsShow->isChecked());

        QString ef = mMainWindow->getExtrCalibration()->getExtrCalibFile();
        if (ef != "")
            ef = getFileList(ef, mMainWindow->getProFileName());
        subSubElem.setAttribute("EXTERNAL_CALIB_FILE",ef);

        subSubElem.setAttribute("COORD_DIMENSION", coordTab->currentIndex());

        subSubElem.setAttribute("SHOW", coordShow->isChecked());
        subSubElem.setAttribute("FIX", coordFix->isChecked());
        subSubElem.setAttribute("ROTATE", coordRotate->value());
        subSubElem.setAttribute("TRANS_X", coordTransX->value());
        subSubElem.setAttribute("TRANS_Y", coordTransY->value());
        subSubElem.setAttribute("SCALE", coordScale->value());
        subSubElem.setAttribute("ALTITUDE", coordAltitude->value());
        //subSubElem.setAttribute("ALTITUDE_MEASURED", coordAltitudeMeasured->text());
        subSubElem.setAttribute("UNIT", coordUnit->value());
        subSubElem.setAttribute("USE_INTRINSIC_CENTER", coordUseIntrinsic->isChecked());
        subSubElem.setAttribute("COORD3D_TRANS_X", coord3DTransX->value());
        subSubElem.setAttribute("COORD3D_TRANS_Y", coord3DTransY->value());
        subSubElem.setAttribute("COORD3D_TRANS_Z", coord3DTransZ->value());
        //cout << "x: " << coord3DTransX->value() << " y: " << coord3DTransY->value() << " z: " << coord3DTransZ->value() << endl;
        subSubElem.setAttribute("COORD3D_AXIS_LEN", coord3DAxeLen->value());
        subSubElem.setAttribute("COORD3D_SWAP_X", coord3DSwapX->isChecked());
        subSubElem.setAttribute("COORD3D_SWAP_Y", coord3DSwapY->isChecked());
        subSubElem.setAttribute("COORD3D_SWAP_Z", coord3DSwapZ->isChecked());
        subElem.appendChild(subSubElem);

        subSubElem = (elem.ownerDocument()).createElement("ALIGNMENT_GRID");
        subSubElem.setAttribute("GRID_DIMENSION", gridTab->currentIndex());
        subSubElem.setAttribute("SHOW", gridShow->isChecked());
        subSubElem.setAttribute("FIX", gridFix->isChecked());
        subSubElem.setAttribute("ROTATE", gridRotate->value());
        subSubElem.setAttribute("TRANS_X", gridTransX->value());
        subSubElem.setAttribute("TRANS_Y", gridTransY->value());
        subSubElem.setAttribute("SCALE", gridScale->value());
        subSubElem.setAttribute("GRID3D_TRANS_X", grid3DTransX->value());
        subSubElem.setAttribute("GRID3D_TRANS_Y", grid3DTransY->value());
        subSubElem.setAttribute("GRID3D_TRANS_Z", grid3DTransZ->value());
        subSubElem.setAttribute("GRID3D_RESOLUTION", grid3DResolution->value());
        subElem.appendChild(subSubElem);

        // - - - - - - - - - - - - - - - - - - - 
        subElem = (elem.ownerDocument()).createElement("RECOGNITION");
        elem.appendChild(subElem);

        subSubElem = (elem.ownerDocument()).createElement("PERFORM");
        subSubElem.setAttribute("ENABLED", performRecognition->isChecked());
        subSubElem.setAttribute("METHOD", recoMethod->currentIndex());
        subSubElem.setAttribute("STEP", recoStep->value());
        subElem.appendChild(subSubElem);

        subSubElem = (elem.ownerDocument()).createElement("REGION_OF_INTEREST");
        subSubElem.setAttribute("SHOW", roiShow->isChecked());
        subSubElem.setAttribute("FIX", roiFix->isChecked());
        subSubElem.setAttribute("X", mMainWindow->getRecoRoiItem()->rect().x());
        subSubElem.setAttribute("Y", mMainWindow->getRecoRoiItem()->rect().y());
        subSubElem.setAttribute("WIDTH", mMainWindow->getRecoRoiItem()->rect().width());
        subSubElem.setAttribute("HEIGHT", mMainWindow->getRecoRoiItem()->rect().height());
        subElem.appendChild(subSubElem);

        subSubElem = (elem.ownerDocument()).createElement("MARKER");
        subSubElem.setAttribute("BRIGHTNESS", markerBrightness->value());
        subSubElem.setAttribute("IGNORE_WITHOUT", markerIgnoreWithout->isChecked());
        subElem.appendChild(subSubElem);

        subSubElem = (elem.ownerDocument()).createElement("SIZE_COLOR");
        subSubElem.setAttribute("SHOW", recoShowColor->isChecked());
        subSubElem.setAttribute("MODEL", recoColorModel->currentIndex());
        subSubElem.setAttribute("AUTO_WB", recoAutoWB->isChecked());
        subSubElem.setAttribute("X", recoColorX->currentIndex());
        subSubElem.setAttribute("Y", recoColorY->currentIndex());
        subSubElem.setAttribute("Z", recoColorZ->value());
        subSubElem.setAttribute("GREY_LEVEL", recoGreyLevel->value());
        subSubElem.setAttribute("SYMBOL_SIZE", recoSymbolSize->value());

        subSubElem.setAttribute("MAP_NUMBER", mapNr->value()); // MAP_MAX noetig ?: mapNr->maximum()+1
        for (int i = 0; i <= mapNr->maximum(); ++i)
        {
            subSubSubElem = (elem.ownerDocument()).createElement("MAP"); //QString("MAP %1").arg(i)
            RectMap map = colorPlot->getMapItem()->getMap(i);
            subSubSubElem.setAttribute("X", map.x());
            subSubSubElem.setAttribute("Y", map.y());
            subSubSubElem.setAttribute("WIDTH", map.width());
            subSubSubElem.setAttribute("HEIGHT", map.height());
            subSubSubElem.setAttribute("COLORED", map.colored());
            subSubSubElem.setAttribute("MAP_HEIGHT", map.mapHeight());
            subSubSubElem.setAttribute("FROM_HUE", map.fromColor().hue());
            subSubSubElem.setAttribute("FROM_SAT", map.fromColor().saturation());
            subSubSubElem.setAttribute("FROM_VAL", map.fromColor().value());
            subSubSubElem.setAttribute("TO_HUE", map.toColor().hue());
            subSubSubElem.setAttribute("TO_SAT", map.toColor().saturation());
            subSubSubElem.setAttribute("TO_VAL", map.toColor().value());
            subSubSubElem.setAttribute("INV_HUE", map.invHue());
            subSubElem.appendChild(subSubSubElem);
        }
        subSubElem.setAttribute("DEFAULT_HEIGHT", mapDefaultHeight->value());
        subElem.appendChild(subSubElem);

        // - - - - - - - - - - - - - - - - - - - 
        subElem = (elem.ownerDocument()).createElement("TRACKING");
        elem.appendChild(subElem);

        subSubElem = (elem.ownerDocument()).createElement("ONLINE_CALCULATION");
        subSubElem.setAttribute("ENABLED", trackOnlineCalc->isChecked());
        subElem.appendChild(subSubElem);

        subSubElem = (elem.ownerDocument()).createElement("REPEAT_BELOW");
        subSubElem.setAttribute("ENABLED", trackRepeat->isChecked());
        subSubElem.setAttribute("QUALITY", trackRepeatQual->value());
        subElem.appendChild(subSubElem);

        subSubElem = (elem.ownerDocument()).createElement("EXTRAPOLATION");
        subSubElem.setAttribute("ENABLED", trackExtrapolation->isChecked());
        subElem.appendChild(subSubElem);

        subSubElem = (elem.ownerDocument()).createElement("MERGE");
        subSubElem.setAttribute("ENABLED", trackMerge->isChecked());
        subElem.appendChild(subSubElem);

        subSubElem = (elem.ownerDocument()).createElement("ONLY_VISIBLE");
        subSubElem.setAttribute("ENABLED", trackOnlyVisible->isChecked());
        subElem.appendChild(subSubElem);

        subSubElem = (elem.ownerDocument()).createElement("REGION_OF_INTEREST");
        subSubElem.setAttribute("SHOW", trackRoiShow->isChecked());
        subSubElem.setAttribute("FIX", trackRoiFix->isChecked());
        subSubElem.setAttribute("X", mMainWindow->getTrackRoiItem()->rect().x());
        subSubElem.setAttribute("Y", mMainWindow->getTrackRoiItem()->rect().y());
        subSubElem.setAttribute("WIDTH", mMainWindow->getTrackRoiItem()->rect().width());
        subSubElem.setAttribute("HEIGHT", mMainWindow->getTrackRoiItem()->rect().height());
        subElem.appendChild(subSubElem);

        // export options
        subSubElem = (elem.ownerDocument()).createElement("SEARCH_MISSING_FRAMES");
        subSubElem.setAttribute("ENABLED", trackMissingFrames->isChecked());
        subElem.appendChild(subSubElem);

        subSubElem = (elem.ownerDocument()).createElement("RECALCULATE_MEDIAN_HEIGHT");
        subSubElem.setAttribute("ENABLED", trackRecalcHeight->isChecked());
        subElem.appendChild(subSubElem);

        subSubElem = (elem.ownerDocument()).createElement("ALLOW_ALTERNATE_HEIGHT");
        subSubElem.setAttribute("ENABLED", trackAlternateHeight->isChecked());
        subElem.appendChild(subSubElem);

        subSubElem = (elem.ownerDocument()).createElement("EXPORT_ELIMINATE_TRACKPOINT_WITHOUT_HEIGHT");
        subSubElem.setAttribute("ENABLED", exportElimTp->isChecked());
        subElem.appendChild(subSubElem);

        subSubElem = (elem.ownerDocument()).createElement("EXPORT_ELIMINATE_TRAJECTORY_WITHOUT_HEIGHT");
        subSubElem.setAttribute("ENABLED", exportElimTrj->isChecked());
        subElem.appendChild(subSubElem);

        subSubElem = (elem.ownerDocument()).createElement("EXPORT_SMOOTH");
        subSubElem.setAttribute("ENABLED", exportSmooth->isChecked());
        subElem.appendChild(subSubElem);

        subSubElem = (elem.ownerDocument()).createElement("EXPORT_VIEWING_DIRECTION");
        subSubElem.setAttribute("ENABLED", exportViewDir->isChecked());
        subElem.appendChild(subSubElem);

        subSubElem = (elem.ownerDocument()).createElement("EXPORT_ANGLE_OF_VIEW");
        subSubElem.setAttribute("ENABLED", exportAngleOfView->isChecked());
        subElem.appendChild(subSubElem);

        subSubElem = (elem.ownerDocument()).createElement("EXPORT_USE_METER");
        subSubElem.setAttribute("ENABLED", exportUseM->isChecked());
        subElem.appendChild(subSubElem);

        subSubElem = (elem.ownerDocument()).createElement("EXPORT_COMMENT");
        subSubElem.setAttribute("ENABLED", exportComment->isChecked());
        subElem.appendChild(subSubElem);

        subSubElem = (elem.ownerDocument()).createElement("EXPORT_MARKERID");
        subSubElem.setAttribute("ENABLED", exportMarkerID->isChecked());
        subElem.appendChild(subSubElem);


        subSubElem = (elem.ownerDocument()).createElement("TEST_EQUAL");
        subSubElem.setAttribute("ENABLED", testEqual->isChecked());
        subElem.appendChild(subSubElem);

        subSubElem = (elem.ownerDocument()).createElement("TEST_VELOCITY");
        subSubElem.setAttribute("ENABLED", testVelocity->isChecked());
        subElem.appendChild(subSubElem);

        subSubElem = (elem.ownerDocument()).createElement("TEST_INSIDE");
        subSubElem.setAttribute("ENABLED", testInside->isChecked());
        subElem.appendChild(subSubElem);

        subSubElem = (elem.ownerDocument()).createElement("TEST_LENGTH");
        subSubElem.setAttribute("ENABLED", testLength->isChecked());
        subElem.appendChild(subSubElem);


        subSubElem = (elem.ownerDocument()).createElement("TRACK_FILE");
        fn = mMainWindow->getTrackFileName();
        if (fn != "")
            fn = getFileList(fn, mMainWindow->getProFileName());
//        if ((fn != "") && (QFileInfo(fn).isRelative()))
//            fn = fn+";"+QFileInfo(fn).absoluteFilePath();
        subSubElem.setAttribute("FILENAME", fn);
        subElem.appendChild(subSubElem);

        subSubElem = (elem.ownerDocument()).createElement("SEARCH_REGION");
        subSubElem.setAttribute("SCALE", trackRegionScale->value());
        subSubElem.setAttribute("LEVELS", trackRegionLevels->value());
        subSubElem.setAttribute("MAX_ERROR", trackErrorExponent->value());
        subSubElem.setAttribute("SHOW", trackShowSearchSize->isChecked());
        subElem.appendChild(subSubElem);

        subSubElem = (elem.ownerDocument()).createElement("PATH");
        subSubElem.setAttribute("SHOW", trackShow->isChecked());
        subSubElem.setAttribute("FIX", trackFix->isChecked());

        subSubElem.setAttribute("ONLY_VISIBLE", trackShowOnlyVisible->isChecked());
        subSubElem.setAttribute("ONLY_PEOPLE", trackShowOnly->isChecked());
        subSubElem.setAttribute("ONLY_PEOPLE_LIST", trackShowOnlyList->isChecked());
        subSubElem.setAttribute("ONLY_PEOPLE_NR", trackShowOnlyNr->value());
        subSubElem.setAttribute("ONLY_PEOPLE_NR_LIST", trackShowOnlyNr->text());

        subSubElem.setAttribute("SHOW_CURRENT_POINT", trackShowCurrentPoint->isChecked());
        subSubElem.setAttribute("SHOW_POINTS", trackShowPoints->isChecked());
        subSubElem.setAttribute("SHOW_PATH", trackShowPath->isChecked());
        subSubElem.setAttribute("SHOW_COLLECTIVE_COLOR", trackShowColColor->isChecked());
        subSubElem.setAttribute("SHOW_COLOR_MARKER", trackShowColorMarker->isChecked());
        subSubElem.setAttribute("SHOW_NUMBER", trackShowNumber->isChecked());
        subSubElem.setAttribute("SHOW_GROUND_POSITION", trackShowGroundPosition->isChecked());
        subSubElem.setAttribute("SHOW_GROUND_PATH", trackShowGroundPath->isChecked());

        subSubElem.setAttribute("TRACK_GROUND_PATH_COLOR", getTrackGroundPathColor().name());
        subSubElem.setAttribute("TRACK_PATH_COLOR", getTrackPathColor().name());
        subSubElem.setAttribute("CURRENT_POINT_SIZE", trackCurrentPointSize->value());
        subSubElem.setAttribute("POINTS_SIZE", trackPointSize->value());
        subSubElem.setAttribute("PATH_SIZE", trackPathWidth->value());
        subSubElem.setAttribute("COLLECTIVE_COLOR_SIZE", trackColColorSize->value());
        subSubElem.setAttribute("COLOR_MARKER_SIZE", trackColorMarkerSize->value());
        subSubElem.setAttribute("NUMBER_SIZE", trackNumberSize->value());
        subSubElem.setAttribute("GROUND_POSITION_SIZE", trackGroundPositionSize->value());
        subSubElem.setAttribute("GROUND_PATH_SIZE", trackGroundPathSize->value());

        subSubElem.setAttribute("HEAD_SIZE", trackHeadSized->isChecked());
        subSubElem.setAttribute("POINTS_COLORED", trackShowPointsColored->isChecked());
        subSubElem.setAttribute("NUMBER_BOLD", trackNumberBold->isChecked());

        subSubElem.setAttribute("BEFORE", trackShowBefore->value());
        subSubElem.setAttribute("AFTER", trackShowAfter->value());

        subElem.appendChild(subSubElem);

        // - - - - - - - - - - - - - - - - - - - 
        subElem = (elem.ownerDocument()).createElement("ANALYSIS");
        elem.appendChild(subElem);

        subSubElem = (elem.ownerDocument()).createElement("SEARCH_MISSING_FRAMES");
        subSubElem.setAttribute("ENABLED", anaMissingFrames->isChecked());
        subElem.appendChild(subSubElem);

        subSubElem = (elem.ownerDocument()).createElement("MARK_ACTUAL");
        subSubElem.setAttribute("ENABLED", anaMarkAct->isChecked());
        subElem.appendChild(subSubElem);

        subSubElem = (elem.ownerDocument()).createElement("CALCULATION");
        subSubElem.setAttribute("STEP_SIZE", anaStep->value());
        subSubElem.setAttribute("CONSIDER_X", anaConsiderX->isChecked());
        subSubElem.setAttribute("CONSIDER_Y", anaConsiderY->isChecked());
        subSubElem.setAttribute("ABSOLUTE", anaConsiderAbs->isChecked());
        subSubElem.setAttribute("REVERSE", anaConsiderRev->isChecked());
        subSubElem.setAttribute("SHOW_VORONOI", showVoronoiCells->isChecked());
        subElem.appendChild(subSubElem);
}

// read data from xml node
void Control::getXml(QDomElement &elem)
{
    QDomElement subElem, subSubElem, subSubSubElem;

    if (elem.hasAttribute("TAB"))
        tabs->setCurrentIndex(elem.attribute("TAB").toInt());
    for(subElem = elem.firstChildElement(); !subElem.isNull(); subElem = subElem.nextSiblingElement())
        if (subElem.tagName() == "CALIBRATION")
        {
            for(subSubElem = subElem.firstChildElement(); !subSubElem.isNull(); subSubElem = subSubElem.nextSiblingElement())
                if (subSubElem.tagName() == "BRIGHTNESS")
                {
                    if (subSubElem.hasAttribute("ENABLED"))
                        filterBrightContrast->setCheckState(subSubElem.attribute("ENABLED").toInt() ? Qt::Checked : Qt::Unchecked);
                    if (subSubElem.hasAttribute("VALUE"))
                        filterBrightParam->setValue(subSubElem.attribute("VALUE").toInt());
                }
                else if (subSubElem.tagName() == "CONTRAST")
                {
                    if (subSubElem.hasAttribute("ENABLED"))
                        filterBrightContrast->setCheckState(subSubElem.attribute("ENABLED").toInt() ? Qt::Checked : Qt::Unchecked);
                    //,mMainWindow->getContrastFilter()->getEnabled() ? "1" : "0" //Qt::Unchecked
                    if (subSubElem.hasAttribute("VALUE"))
                        filterContrastParam->setValue(subSubElem.attribute("VALUE").toInt());
                }
                else if (subSubElem.tagName() == "BORDER")
                {
                    if (subSubElem.hasAttribute("ENABLED"))
                        filterBorder->setCheckState(subSubElem.attribute("ENABLED").toInt() ? Qt::Checked : Qt::Unchecked);
                    if (subSubElem.hasAttribute("VALUE"))
                        filterBorderParamSize->setValue(subSubElem.attribute("VALUE").toInt());
                    if (subSubElem.hasAttribute("COLOR"))
                    {
                        QColor color(subSubElem.attribute("COLOR"));
                        mMainWindow->getBorderFilter()->getBorderColR()->setValue(color.red());
                        mMainWindow->getBorderFilter()->getBorderColG()->setValue(color.green());
                        mMainWindow->getBorderFilter()->getBorderColB()->setValue(color.blue());
                        // mMainWindow->updateImage(); auskommentiert, da noch andere werte sehr wahrscheinlich mitgeladen werden
                    }
                }
                else if (subSubElem.tagName() == "SWAP")
                {
                    if (subSubElem.hasAttribute("ENABLED"))
                        filterSwap->setCheckState(subSubElem.attribute("ENABLED").toInt() ? Qt::Checked : Qt::Unchecked);
                    if (subSubElem.hasAttribute("HORIZONTALLY"))
                        filterSwapH->setCheckState(subSubElem.attribute("HORIZONTALLY").toInt() ? Qt::Checked : Qt::Unchecked);
                    if (subSubElem.hasAttribute("VERTICALLY"))
                        filterSwapV->setCheckState(subSubElem.attribute("VERTICALLY").toInt() ? Qt::Checked : Qt::Unchecked);
                }
                else if (subSubElem.tagName() == "BG_SUB")
                {
                    if (subSubElem.hasAttribute("ENABLED"))
                        filterBg->setCheckState(subSubElem.attribute("ENABLED").toInt() ? Qt::Checked : Qt::Unchecked);
                    if (subSubElem.hasAttribute("UPDATE"))
                        filterBgUpdate->setCheckState(subSubElem.attribute("UPDATE").toInt() ? Qt::Checked : Qt::Unchecked);
                    if (subSubElem.hasAttribute("SHOW"))
                        filterBgShow->setCheckState(subSubElem.attribute("SHOW").toInt() ? Qt::Checked : Qt::Unchecked);
                    if (subSubElem.hasAttribute("FILE"))
                    {
                        QString f = subSubElem.attribute("FILE");
                        if (f != "")
                        {
                            if (getExistingFile(f, mMainWindow->getProFileName()) != "")
                            {
//                            if (!(mMainWindow->getBackgroundFilter()->load(getExistingFile(subSubElem.attribute("FILE")))));
//                                debout << "Warning: Background subtracting file import error!" << endl;
                                mMainWindow->getBackgroundFilter()->setFilename(getExistingFile(subSubElem.attribute("FILE"), mMainWindow->getProFileName()));
                            }
                            else
                                debout << "Warning: Background subtracting file not readable!" << endl;
                        }
                    }
                    if (subSubElem.hasAttribute("DELETE"))
                        filterBgDeleteTrj->setCheckState(subSubElem.attribute("DELETE").toInt() ? Qt::Checked : Qt::Unchecked);
                    if (subSubElem.hasAttribute("DELETE_NUMBER"))
                        filterBgDeleteNumber->setValue(subSubElem.attribute("DELETE_NUMBER").toInt());
                }
                else if (subSubElem.tagName() == "PATTERN")
                {
                    if (subSubElem.hasAttribute("BOARD_SIZE_X"))
                        mMainWindow->getAutoCalib()->setBoardSizeX(subSubElem.attribute("BOARD_SIZE_X").toInt());// 6
                    if (subSubElem.hasAttribute("BOARD_SIZE_Y"))
                        mMainWindow->getAutoCalib()->setBoardSizeY(subSubElem.attribute("BOARD_SIZE_Y").toInt()); // 8 oder 9
                    if (subSubElem.hasAttribute("SQUARE_SIZE"))
                        mMainWindow->getAutoCalib()->setSquareSize(subSubElem.attribute("SQUARE_SIZE").toDouble()); // in cm
                }
                else if (subSubElem.tagName() == "INTRINSIC_PARAMETERS")
                {
                    if (subSubElem.hasAttribute("ENABLED"))
                        apply->setCheckState(subSubElem.attribute("ENABLED").toInt() ? Qt::Checked : Qt::Unchecked);
                    if (subSubElem.hasAttribute("FX"))
                        fx->setValue(subSubElem.attribute("FX").toDouble());
                    if (subSubElem.hasAttribute("FY"))
                        fy->setValue(subSubElem.attribute("FY").toDouble());
                    if (subSubElem.hasAttribute("CX"))
                    {
                        double cx_val = subSubElem.attribute("CX").toDouble();
                        if( cx_val < cx->minimum() )
                            cx->setMinimum(cx_val-50);
                        if( cx_val > cx->maximum() )
                            cx->setMaximum(cx_val+50);
                        cx->setValue(cx_val);
                    }
                    if (subSubElem.hasAttribute("CY"))
                    {
                        double cy_val = subSubElem.attribute("CY").toDouble();
                        if( cy_val < cy->minimum() )
                            cy->setMinimum(cy_val-50);
                        if( cy_val > cy->maximum() )
                            cy->setMaximum(cy_val+50);
                        cy->setValue(cy_val);
                    }
                    if (subSubElem.hasAttribute("R2"))
                        r2->setValue(subSubElem.attribute("R2").toDouble());
                    if (subSubElem.hasAttribute("R4"))
                        r4->setValue(subSubElem.attribute("R4").toDouble());
                    if (subSubElem.hasAttribute("R6"))
                        r6->setValue(subSubElem.attribute("R6").toDouble());
                    if (subSubElem.hasAttribute("TX"))
                        tx->setValue(subSubElem.attribute("TX").toDouble());
                    if (subSubElem.hasAttribute("TY"))
                        ty->setValue(subSubElem.attribute("TY").toDouble());
                    if (subSubElem.hasAttribute("K4"))
                        k4->setValue(subSubElem.attribute("K4").toDouble());
                    if (subSubElem.hasAttribute("K5"))
                        k5->setValue(subSubElem.attribute("K5").toDouble());
                    if (subSubElem.hasAttribute("K6"))
                        k6->setValue(subSubElem.attribute("K6").toDouble());
                    if (subSubElem.hasAttribute("QUAD_ASPECT_RATIO"))
                        quadAspectRatio->setCheckState(subSubElem.attribute("QUAD_ASPECT_RATIO").toInt() ? Qt::Checked : Qt::Unchecked);
                    if (subSubElem.hasAttribute("FIX_CENTER"))
                        fixCenter->setCheckState(subSubElem.attribute("FIX_CENTER").toInt() ? Qt::Checked : Qt::Unchecked);
                    if (subSubElem.hasAttribute("TANG_DIST"))
                        tangDist->setCheckState(subSubElem.attribute("TANG_DIST").toInt() ? Qt::Checked : Qt::Unchecked);
                    if (subSubElem.hasAttribute("CALIB_FILES"))
                    {
                        QStringList fl = (subSubElem.attribute("CALIB_FILES")).split(",");
                        QString tmpStr;
                        for (int i = 0; i < fl.size(); ++i)
                            if ((fl[i] = fl[i].trimmed()) == "")
                                fl.removeAt(i);
                            else
                            {
                                tmpStr = getExistingFile(fl[i], mMainWindow->getProFileName());
                                if (tmpStr != "")
                                    fl[i] = tmpStr;
                            }
//                         foreach (QString f, fl) { //new keyword from QT, but must be slower!!!
//                             if (str.contains("Bill"))
//                                 result += str;
//                         }
                        // auch setzen, wenn leer, vielleicht ist das ja gewuenscht
                        mMainWindow->getAutoCalib()->setCalibFiles(fl);
                        if (!fl.isEmpty())
                            autoCalib->setEnabled(true);
                    }
                }
                else if (subSubElem.tagName() == "EXTRINSIC_PARAMETERS")
                {
                    if (subSubElem.hasAttribute("EXTR_ROT_1"))
                        rot1->setValue(subSubElem.attribute("EXTR_ROT_1").toDouble());
                    if (subSubElem.hasAttribute("EXTR_ROT_2"))
                        rot2->setValue(subSubElem.attribute("EXTR_ROT_2").toDouble());
                    if (subSubElem.hasAttribute("EXTR_ROT_3"))
                        rot3->setValue(subSubElem.attribute("EXTR_ROT_3").toDouble());
                    if (subSubElem.hasAttribute("EXTR_TRANS_1"))
                        trans1->setValue(subSubElem.attribute("EXTR_TRANS_1").toDouble());
                    if (subSubElem.hasAttribute("EXTR_TRANS_2"))
                        trans2->setValue(subSubElem.attribute("EXTR_TRANS_2").toDouble());
                    if (subSubElem.hasAttribute("EXTR_TRANS_3"))
                        trans3->setValue(subSubElem.attribute("EXTR_TRANS_3").toDouble());
                    if (subSubElem.hasAttribute("SHOW_CALIB_POINTS"))
                        extCalibPointsShow->setCheckState(subSubElem.attribute("SHOW_CALIB_POINTS").toInt() ? Qt::Checked : Qt::Unchecked);

                    if (subSubElem.hasAttribute("COORD_DIMENSION"))
                    {
                        coordTab->setCurrentIndex(subSubElem.attribute("COORD_DIMENSION").toInt());
                    }else
                    {
                        coordTab->setCurrentIndex(1);//  = 2D
                        setEnabledExtrParams(false);
                    }
                    if (subSubElem.hasAttribute("EXTERNAL_CALIB_FILE"))
                    {
                        if( getExistingFile(QString::fromStdString(subSubElem.attribute("EXTERNAL_CALIB_FILE").toStdString()), mMainWindow->getProFileName()) != "" )
                        {
                            mMainWindow->getExtrCalibration()->setExtrCalibFile(getExistingFile(QString::fromStdString(subSubElem.attribute("EXTERNAL_CALIB_FILE").toStdString()), mMainWindow->getProFileName()));
                            mMainWindow->getExtrCalibration()->loadExtrCalibFile();
                        }
                    }

                    if (subSubElem.hasAttribute("SHOW"))
                        coordShow->setCheckState(subSubElem.attribute("SHOW").toInt() ? Qt::Checked : Qt::Unchecked);
                    if (subSubElem.hasAttribute("FIX"))
                        coordFix->setCheckState(subSubElem.attribute("FIX").toInt() ? Qt::Checked : Qt::Unchecked);
                    if (subSubElem.hasAttribute("ROTATE"))
                        coordRotate->setValue(subSubElem.attribute("ROTATE").toInt());
                    //debout << "coordTransX_max: " << coordTransX->maximum() << " coordTransX: " << subSubElem.attribute("TRANS_X").toInt() << endl;
                    if (subSubElem.hasAttribute("TRANS_X")){
//                        coordTransX->setValue(subSubElem.attribute("TRANS_X").toInt());
                        int trans_x = subSubElem.attribute("TRANS_X").toInt();
                        if( trans_x > coordTransX->maximum() )
                            setCalibCoordTransXMax(trans_x);
                        //debout << "trans_x aus pet-datei: " << subSubElem.attribute("TRANS_X").toInt() << endl;
                        coordTransX->setValue(trans_x);
                        //debout << "trans_x in slider: " << coordTransX->value() << endl;
                    }
                    if (subSubElem.hasAttribute("TRANS_Y")){
//                        coordTransX->setValue(subSubElem.attribute("TRANS_Y").toInt());
                        int trans_y = subSubElem.attribute("TRANS_Y").toInt();
                        if( trans_y > coord3DTransY->maximum() )
                            setCalibCoordTransYMax(trans_y);
                        //debout << "trans_y aus pet-datei: " << subSubElem.attribute("TRANS_Y").toInt() << endl;
                        coordTransY->setValue(trans_y);
                        //debout << "trans_y in slider: " << coordTransY->value() << endl;
                    }
                        coordTransY->setValue(subSubElem.attribute("TRANS_Y").toInt());
                    if (subSubElem.hasAttribute("SCALE"))
                        coordScale->setValue(subSubElem.attribute("SCALE").toInt());
                    if (subSubElem.hasAttribute("ALTITUDE"))
                        coordAltitude->setValue(subSubElem.attribute("ALTITUDE").toDouble());
                    if (subSubElem.hasAttribute("UNIT"))
                        coordUnit->setValue(subSubElem.attribute("UNIT").toDouble());
                    if (subSubElem.hasAttribute("USE_INTRINSIC_CENTER"))
                        coordUseIntrinsic->setCheckState(subSubElem.attribute("USE_INTRINSIC_CENTER").toInt() ? Qt::Checked : Qt::Unchecked);
                    if (subSubElem.hasAttribute("COORD3D_TRANS_X"))
                        coord3DTransX->setValue(subSubElem.attribute("COORD3D_TRANS_X").toInt());
                    if (subSubElem.hasAttribute("COORD3D_TRANS_Y"))
                        coord3DTransY->setValue(subSubElem.attribute("COORD3D_TRANS_Y").toInt());
                    if (subSubElem.hasAttribute("COORD3D_TRANS_Z"))
                        coord3DTransZ->setValue(subSubElem.attribute("COORD3D_TRANS_Z").toInt());
                    if (subSubElem.hasAttribute("COORD3D_AXIS_LEN"))
                        coord3DAxeLen->setValue(subSubElem.attribute("COORD3D_AXIS_LEN").toInt());
                    if (subSubElem.hasAttribute("COORD3D_SWAP_X"))
                        coord3DSwapX->setCheckState(subSubElem.attribute("COORD3D_SWAP_X").toInt() ? Qt::Checked : Qt::Unchecked);
                    if (subSubElem.hasAttribute("COORD3D_SWAP_Y"))
                        coord3DSwapY->setCheckState(subSubElem.attribute("COORD3D_SWAP_Y").toInt() ? Qt::Checked : Qt::Unchecked);
                    if (subSubElem.hasAttribute("COORD3D_SWAP_Z"))
                        coord3DSwapZ->setCheckState(subSubElem.attribute("COORD3D_SWAP_Z").toInt() ? Qt::Checked : Qt::Unchecked);
                }
                else if (subSubElem.tagName() == "ALIGNMENT_GRID")
                {
                    if (subSubElem.hasAttribute("GRID_DIMENSION"))
                    {
                        gridTab->setCurrentIndex(subSubElem.attribute("GRID_DIMENSION").toInt());
                    }else
                    {
                        gridTab->setCurrentIndex(1);//  = 2D
                    }
                    if (subSubElem.hasAttribute("SHOW"))
                        gridShow->setCheckState(subSubElem.attribute("SHOW").toInt() ? Qt::Checked : Qt::Unchecked);
                    if (subSubElem.hasAttribute("FIX"))
                        gridFix->setCheckState(subSubElem.attribute("FIX").toInt() ? Qt::Checked : Qt::Unchecked);
                    if (subSubElem.hasAttribute("ROTATE"))
                        gridRotate->setValue(subSubElem.attribute("ROTATE").toInt());
                    if (subSubElem.hasAttribute("TRANS_X"))
                        gridTransX->setValue(subSubElem.attribute("TRANS_X").toInt());
                    if (subSubElem.hasAttribute("TRANS_Y"))
                        gridTransY->setValue(subSubElem.attribute("TRANS_Y").toInt());
                    if (subSubElem.hasAttribute("SCALE"))
                        gridScale->setValue(subSubElem.attribute("SCALE").toInt());
                    if (subSubElem.hasAttribute("GRID3D_TRANS_X"))
                        grid3DTransX->setValue(subSubElem.attribute("GRID3D_TRANS_X").toInt());
                    if (subSubElem.hasAttribute("GRID3D_TRANS_Y"))
                        grid3DTransY->setValue(subSubElem.attribute("GRID3D_TRANS_Y").toInt());
                    if (subSubElem.hasAttribute("GRID3D_TRANS_Z"))
                        grid3DTransZ->setValue(subSubElem.attribute("GRID3D_TRANS_Z").toInt());
                    if (subSubElem.hasAttribute("GRID3D_RESOLUTION"))
                        grid3DResolution->setValue(subSubElem.attribute("GRID3D_RESOLUTION").toInt());
                }
                else
                    debout << "Unknown CALIBRATION tag " << subSubElem.tagName() << endl;

        }
        else if (subElem.tagName() == "RECOGNITION")
        {
           for(subSubElem = subElem.firstChildElement(); !subSubElem.isNull(); subSubElem = subSubElem.nextSiblingElement())
                if (subSubElem.tagName() == "PERFORM")
                {
                    if (subSubElem.hasAttribute("ENABLED"))
                        performRecognition->setCheckState(subSubElem.attribute("ENABLED").toInt() ? Qt::Checked : Qt::Unchecked);
                    if (subSubElem.hasAttribute("METHOD"))
                        recoMethod->setCurrentIndex(subSubElem.attribute("METHOD").toInt());
                    if (subSubElem.hasAttribute("STEP"))
                        recoStep->setValue(subSubElem.attribute("STEP").toInt());
                }
                else if (subSubElem.tagName() == "REGION_OF_INTEREST")
                {
                    double x=0, y=0, w=0, h=0;
                    if (subSubElem.hasAttribute("SHOW"))
                        roiShow->setCheckState(subSubElem.attribute("SHOW").toInt() ? Qt::Checked : Qt::Unchecked);
                    if (subSubElem.hasAttribute("FIX"))
                        roiFix->setCheckState(subSubElem.attribute("FIX").toInt() ? Qt::Checked : Qt::Unchecked);
                    if (subSubElem.hasAttribute("X"))
                        x = subSubElem.attribute("X").toDouble();
                    if (subSubElem.hasAttribute("Y"))
                        y = subSubElem.attribute("Y").toDouble();
                    if (subSubElem.hasAttribute("WIDTH"))
                        w = subSubElem.attribute("WIDTH").toDouble();
                    if (subSubElem.hasAttribute("HEIGHT"))
                        h = subSubElem.attribute("HEIGHT").toDouble();
                    mMainWindow->getRecoRoiItem()->setRect(x, y, w, h);
                }
                else if (subSubElem.tagName() == "MARKER")
                {
                    if (subSubElem.hasAttribute("BRIGHTNESS"))
                        markerBrightness->setValue(subSubElem.attribute("BRIGHTNESS").toInt());
                    if (subSubElem.hasAttribute("IGNORE_WITHOUT"))
                        markerIgnoreWithout->setCheckState(subSubElem.attribute("IGNORE_WITHOUT").toInt() ? Qt::Checked : Qt::Unchecked);
                }
                else if (subSubElem.tagName() == "SIZE_COLOR")
                {
                    mColorChanging = true; // damit bei Anpassungen Farbbild nicht immer wieder neu bestimmt wird
                    if (subSubElem.hasAttribute("SHOW"))
                        recoShowColor->setCheckState(subSubElem.attribute("SHOW").toInt() ? Qt::Checked : Qt::Unchecked);
                    if (subSubElem.hasAttribute("AUTO_WB"))
                        recoAutoWB->setCheckState(subSubElem.attribute("AUTO_WB").toInt() ? Qt::Checked : Qt::Unchecked);
                    if (subSubElem.hasAttribute("X"))
                        recoColorX->setCurrentIndex(subSubElem.attribute("X").toInt());
                    if (subSubElem.hasAttribute("Y"))
                        recoColorY->setCurrentIndex(subSubElem.attribute("Y").toInt());
                    if (subSubElem.hasAttribute("Z"))
                        recoColorZ->setValue(subSubElem.attribute("Z").toInt());
                    if (subSubElem.hasAttribute("GREY_LEVEL"))
                        recoGreyLevel->setValue(subSubElem.attribute("GREY_LEVEL").toInt());
                    if (subSubElem.hasAttribute("SYMBOL_SIZE"))
                        recoSymbolSize->setValue(subSubElem.attribute("SYMBOL_SIZE").toInt());
                    mColorChanging = false; // MODEL setzen erzeugt Bild neu
                    if (subSubElem.hasAttribute("MODEL"))
                    {
                        // damit auch bild neu erzeugt wird, wenn sich index nicht aendert:
                        if (recoColorModel->currentIndex() == subSubElem.attribute("MODEL").toInt())
                        {
                            mIndexChanging = false;
                            on_recoColorModel_currentIndexChanged(recoColorModel->currentIndex());
                        }
                        else
                            recoColorModel->setCurrentIndex(subSubElem.attribute("MODEL").toInt());
                    }

                    double x=0., y=0., width=0., height=0., mapHeight=DEFAULT_HEIGHT;
                    bool colored=true, invHue=false;
                    QColor fromCol, toCol;
                    int h=-1, s=-1, v=-1;

                    fromCol.convertTo(QColor::Hsv);
                    toCol.convertTo(QColor::Hsv);
                    colorPlot->getMapItem()->delMaps();
                    for(subSubSubElem = subSubElem.firstChildElement(); !subSubSubElem.isNull(); subSubSubElem = subSubSubElem.nextSiblingElement())
                        if (subSubSubElem.tagName() == "MAP")
                        {
                            if (subSubSubElem.hasAttribute("X"))
                                x = subSubSubElem.attribute("X").toDouble();
                            if (subSubSubElem.hasAttribute("Y"))
                                y = subSubSubElem.attribute("Y").toDouble();
                            if (subSubSubElem.hasAttribute("WIDTH"))
                                width = subSubSubElem.attribute("WIDTH").toDouble();
                            if (subSubSubElem.hasAttribute("HEIGHT"))
                                height = subSubSubElem.attribute("HEIGHT").toDouble();
                            if (subSubSubElem.hasAttribute("COLORED"))
                                colored = subSubSubElem.attribute("COLORED").toInt();
                            if (subSubSubElem.hasAttribute("MAP_HEIGHT"))
                                mapHeight = subSubSubElem.attribute("MAP_HEIGHT").toDouble();

                            if (subSubSubElem.hasAttribute("FROM_HUE"))
                                h = subSubSubElem.attribute("FROM_HUE").toInt();
                            if (subSubSubElem.hasAttribute("FROM_SAT"))
                                s = subSubSubElem.attribute("FROM_SAT").toInt();
                            if (subSubSubElem.hasAttribute("FROM_VAL"))
                                v = subSubSubElem.attribute("FROM_VAL").toInt();
                            if (h >= 0)
                                fromCol.setHsv(h, s, v);
                            if (subSubSubElem.hasAttribute("TO_HUE"))
                                h = subSubSubElem.attribute("TO_HUE").toInt();
                            if (subSubSubElem.hasAttribute("TO_SAT"))
                                s = subSubSubElem.attribute("TO_SAT").toInt();
                            if (subSubSubElem.hasAttribute("TO_VAL"))
                                v = subSubSubElem.attribute("TO_VAL").toInt();
                            if (h >= 0)
                                toCol.setHsv(h, s, v);
                            if (subSubSubElem.hasAttribute("INV_HUE"))
                                invHue = subSubSubElem.attribute("INV_HUE").toInt();

                            colorPlot->getMapItem()->addMap(x, y, width, height, colored, mapHeight, fromCol, toCol, invHue);

                        }
                        else
                            debout << "Unknown RECOGNITION MAP tag " << subSubElem.tagName() << endl;

                    mapNr->setMaximum(colorPlot->getMapItem()->mapNum()-1);
                    if (subSubElem.hasAttribute("MAP_NUMBER")) // hiermit werden aus map-datenstruktur richtige map angezeigt, daher am ende
                    {
                        mapNr->setValue(subSubElem.attribute("MAP_NUMBER").toInt());
                        on_mapNr_valueChanged(subSubElem.attribute("MAP_NUMBER").toInt()); // nochmal explizit aufrufen, falls 0, dann wuerde valueChanged nicht on_... durchlaufen
                    }
                    if (subSubElem.hasAttribute("DEFAULT_HEIGHT"))
                        mapDefaultHeight->setValue(subSubElem.attribute("DEFAULT_HEIGHT").toDouble());
                }
                else
                    debout << "Unknown RECOGNITION tag " << subSubElem.tagName() << endl;
        }
        else if (subElem.tagName() == "TRACKING")
        {
            for(subSubElem = subElem.firstChildElement(); !subSubElem.isNull(); subSubElem = subSubElem.nextSiblingElement())
                if (subSubElem.tagName() == "ONLINE_CALCULATION")
                {
                    if (subSubElem.hasAttribute("ENABLED"))
                        trackOnlineCalc->setCheckState(subSubElem.attribute("ENABLED").toInt() ? Qt::Checked : Qt::Unchecked);
                }
                else if (subSubElem.tagName() == "REPEAT_BELOW")
                {
                    if (subSubElem.hasAttribute("ENABLED"))
                        trackRepeat->setCheckState(subSubElem.attribute("ENABLED").toInt() ? Qt::Checked : Qt::Unchecked);
                    if (subSubElem.hasAttribute("QUALITY"))
                        trackRepeatQual->setValue(subSubElem.attribute("QUALITY").toInt());
                }
                else if (subSubElem.tagName() == "EXTRAPOLATION")
                {
                    if (subSubElem.hasAttribute("ENABLED"))
                        trackExtrapolation->setCheckState(subSubElem.attribute("ENABLED").toInt() ? Qt::Checked : Qt::Unchecked);
                }
                else if (subSubElem.tagName() == "MERGE")
                {
                    if (subSubElem.hasAttribute("ENABLED"))
                        trackMerge->setCheckState(subSubElem.attribute("ENABLED").toInt() ? Qt::Checked : Qt::Unchecked);
                }
                else if (subSubElem.tagName() == "ONLY_VISIBLE")
                {
                    if (subSubElem.hasAttribute("ENABLED"))
                        trackOnlyVisible->setCheckState(subSubElem.attribute("ENABLED").toInt() ? Qt::Checked : Qt::Unchecked);
                }
                else if (subSubElem.tagName() == "SEARCH_MISSING_FRAMES")
                {
                    if (subSubElem.hasAttribute("ENABLED"))
                        trackMissingFrames->setCheckState(subSubElem.attribute("ENABLED").toInt() ? Qt::Checked : Qt::Unchecked);
                }
                else if (subSubElem.tagName() == "RECALCULATE_MEDIAN_HEIGHT")
                {
                    if (subSubElem.hasAttribute("ENABLED"))
                        trackRecalcHeight->setCheckState(subSubElem.attribute("ENABLED").toInt() ? Qt::Checked : Qt::Unchecked);
                }
                else if (subSubElem.tagName() == "REGION_OF_INTEREST")
                {
                    double x=0, y=0, w=0, h=0;
                    if (subSubElem.hasAttribute("SHOW"))
                        trackRoiShow->setCheckState(subSubElem.attribute("SHOW").toInt() ? Qt::Checked : Qt::Unchecked);
                    if (subSubElem.hasAttribute("FIX"))
                        trackRoiFix->setCheckState(subSubElem.attribute("FIX").toInt() ? Qt::Checked : Qt::Unchecked);
                    if (subSubElem.hasAttribute("X"))
                        x = subSubElem.attribute("X").toDouble();
                    if (subSubElem.hasAttribute("Y"))
                        y = subSubElem.attribute("Y").toDouble();
                    if (subSubElem.hasAttribute("WIDTH"))
                        w = subSubElem.attribute("WIDTH").toDouble();
                    if (subSubElem.hasAttribute("HEIGHT"))
                        h = subSubElem.attribute("HEIGHT").toDouble();
                    mMainWindow->getTrackRoiItem()->setRect(x, y, w, h);
                }
                else if (subSubElem.tagName() == "ALLOW_ALTERNATE_HEIGHT")
                {
                    if (subSubElem.hasAttribute("ENABLED"))
                        trackAlternateHeight->setCheckState(subSubElem.attribute("ENABLED").toInt() ? Qt::Checked : Qt::Unchecked);
                }
                else if (subSubElem.tagName() == "EXPORT_ELIMINATE_TRACKPOINT_WITHOUT_HEIGHT")
                {
                    if (subSubElem.hasAttribute("ENABLED"))
                        exportElimTp->setCheckState(subSubElem.attribute("ENABLED").toInt() ? Qt::Checked : Qt::Unchecked);
                }
                else if (subSubElem.tagName() == "EXPORT_ELIMINATE_TRAJECTORY_WITHOUT_HEIGHT")
                {
                    if (subSubElem.hasAttribute("ENABLED"))
                        exportElimTrj->setCheckState(subSubElem.attribute("ENABLED").toInt() ? Qt::Checked : Qt::Unchecked);
                }
                else if (subSubElem.tagName() == "EXPORT_SMOOTH")
                {
                    if (subSubElem.hasAttribute("ENABLED"))
                        exportSmooth->setCheckState(subSubElem.attribute("ENABLED").toInt() ? Qt::Checked : Qt::Unchecked);
                }
                else if (subSubElem.tagName() == "EXPORT_VIEWING_DIRECTION")
                {
                    if (subSubElem.hasAttribute("ENABLED"))
                        exportViewDir->setCheckState(subSubElem.attribute("ENABLED").toInt() ? Qt::Checked : Qt::Unchecked);
                }
                else if (subSubElem.tagName() == "EXPORT_ANGLE_OF_VIEW")
                {
                    if (subSubElem.hasAttribute("ENABLED"))
                        exportAngleOfView->setCheckState(subSubElem.attribute("ENABLED").toInt() ? Qt::Checked : Qt::Unchecked);
                }
                else if (subSubElem.tagName() == "EXPORT_USE_METER")
                {
                    if (subSubElem.hasAttribute("ENABLED"))
                        exportUseM->setCheckState(subSubElem.attribute("ENABLED").toInt() ? Qt::Checked : Qt::Unchecked);
                }
                else if (subSubElem.tagName() == "EXPORT_COMMENT")
                {
                    if (subSubElem.hasAttribute("ENABLED"))
                        exportComment->setCheckState(subSubElem.attribute("ENABLED").toInt() ? Qt::Checked : Qt::Unchecked);
                }
                else if (subSubElem.tagName() == "EXPORT_MARKERID")
                {
                    if (subSubElem.hasAttribute("ENABLED"))
                        exportMarkerID->setCheckState(subSubElem.attribute("ENABLED").toInt() ? Qt::Checked : Qt::Unchecked);
                }
                else if (subSubElem.tagName() == "TEST_EQUAL")
                {
                    if (subSubElem.hasAttribute("ENABLED"))
                        testEqual->setCheckState(subSubElem.attribute("ENABLED").toInt() ? Qt::Checked : Qt::Unchecked);
                }
                else if (subSubElem.tagName() == "TEST_VELOCITY")
                {
                    if (subSubElem.hasAttribute("ENABLED"))
                        testVelocity->setCheckState(subSubElem.attribute("ENABLED").toInt() ? Qt::Checked : Qt::Unchecked);
                }
                else if (subSubElem.tagName() == "TEST_INSIDE")
                {
                    if (subSubElem.hasAttribute("ENABLED"))
                        testInside->setCheckState(subSubElem.attribute("ENABLED").toInt() ? Qt::Checked : Qt::Unchecked);
                }
                else if (subSubElem.tagName() == "TEST_LENGTH")
                {
                    if (subSubElem.hasAttribute("ENABLED"))
                        testLength->setCheckState(subSubElem.attribute("ENABLED").toInt() ? Qt::Checked : Qt::Unchecked);
                }
                else if (subSubElem.tagName() == "TRACK_FILE")
                {
                    if (subSubElem.hasAttribute("FILENAME"))
                    {
                        if (getExistingFile(subSubElem.attribute("FILENAME"), mMainWindow->getProFileName()) != "")
                            mMainWindow->setTrackFileName(getExistingFile(subSubElem.attribute("FILENAME"), mMainWindow->getProFileName()));
                        else // eigentlich nicht lesbar, aber so wird wenigstens beim projekt speichern wieder mit weggeschrieben
                            mMainWindow->setTrackFileName(subSubElem.attribute("FILENAME"));
                    }
                }
                else if (subSubElem.tagName() == "SEARCH_REGION")
                {
                    if (subSubElem.hasAttribute("SCALE"))
                        trackRegionScale->setValue(subSubElem.attribute("SCALE").toInt());
                    if (subSubElem.hasAttribute("LEVELS"))
                        trackRegionLevels->setValue(subSubElem.attribute("LEVELS").toInt());
                    if (subSubElem.hasAttribute("MAX_ERROR"))
                        trackErrorExponent->setValue(subSubElem.attribute("MAX_ERROR").toInt());
                    if (subSubElem.hasAttribute("SHOW"))
                        trackShowSearchSize->setCheckState(subSubElem.attribute("SHOW").toInt() ? Qt::Checked : Qt::Unchecked);
                }
                else if (subSubElem.tagName() == "PATH")
                {
                    if (subSubElem.hasAttribute("SHOW"))
                        trackShow->setCheckState(subSubElem.attribute("SHOW").toInt() ? Qt::Checked : Qt::Unchecked);
                    if (subSubElem.hasAttribute("FIX"))
                        trackFix->setCheckState(subSubElem.attribute("FIX").toInt() ? Qt::Checked : Qt::Unchecked);

                    if (subSubElem.hasAttribute("ONLY_VISIBLE"))
                        trackShowOnlyVisible->setCheckState(subSubElem.attribute("ONLY_VISIBLE").toInt() ? Qt::Checked : Qt::Unchecked);
                    if (subSubElem.hasAttribute("ONLY_PEOPLE"))
                        trackShowOnly->setCheckState(subSubElem.attribute("ONLY_PEOPLE").toInt() ? Qt::Checked : Qt::Unchecked);
                    if (subSubElem.hasAttribute("ONLY_PEOPLE_LIST"))
                        trackShowOnly->setCheckState(subSubElem.attribute("ONLY_PEOPLE_LIST").toInt() ? Qt::Checked : Qt::Unchecked);

                    // wird in petrack.cpp abgewickelt, da es nach dem laden der trajektorien gemacht werden muss
//                     if (subSubElem.hasAttribute("ONLY_PEOPLE_NR"))
//                     {
//                         trackShowOnlyNr->setValue(subSubElem.attribute("ONLY_PEOPLE_NR").toInt());
//                     }
//                     if (subSubElem.hasAttribute("ONLY_PEOPLE_NR"))
//                     {
//                         int peopleNr = subSubElem.attribute("ONLY_PEOPLE_NR").toInt();
//                         // nur so kann nr vor laden von trajectorien richtig gesetzt werden
//                         // nicht ganz sauber
//                         trackShowOnlyNr->setMaximum(peopleNr);
//                         trackShowOnlyNr->setValue(peopleNr);
//                     }
                    if (subSubElem.hasAttribute("SHOW_CURRENT_POINT"))
                        trackShowCurrentPoint->setCheckState(subSubElem.attribute("SHOW_CURRENT_POINT").toInt() ? Qt::Checked : Qt::Unchecked);
                    if (subSubElem.hasAttribute("SHOW_POINTS"))
                        trackShowPoints->setCheckState(subSubElem.attribute("SHOW_POINTS").toInt() ? Qt::Checked : Qt::Unchecked);
                    if (subSubElem.hasAttribute("SHOW_PATH"))
                        trackShowPath->setCheckState(subSubElem.attribute("SHOW_PATH").toInt() ? Qt::Checked : Qt::Unchecked);
                    if (subSubElem.hasAttribute("SHOW_COLLECTIVE_COLOR"))
                        trackShowColColor->setCheckState(subSubElem.attribute("SHOW_COLLECTIVE_COLOR").toInt() ? Qt::Checked : Qt::Unchecked);
                    if (subSubElem.hasAttribute("SHOW_COLOR_MARKER"))
                        trackShowColorMarker->setCheckState(subSubElem.attribute("SHOW_COLOR_MARKER").toInt() ? Qt::Checked : Qt::Unchecked);
                    if (subSubElem.hasAttribute("SHOW_NUMBER"))
                        trackShowNumber->setCheckState(subSubElem.attribute("SHOW_NUMBER").toInt() ? Qt::Checked : Qt::Unchecked);
                    if (subSubElem.hasAttribute("SHOW_GROUND_POSITION"))
                        trackShowGroundPosition->setCheckState(subSubElem.attribute("SHOW_GROUND_POSITION").toInt() ? Qt::Checked : Qt::Unchecked);
                    if (subSubElem.hasAttribute("SHOW_GROUND_PATH"))
                        trackShowGroundPath->setCheckState(subSubElem.attribute("SHOW_GROUND_PATH").toInt() ? Qt::Checked : Qt::Unchecked);

                    if (subSubElem.hasAttribute("TRACK_PATH_COLOR"))
                    {
                        QColor color(subSubElem.attribute("TRACK_PATH_COLOR"));
                        setTrackPathColor(color);
                    }
                    if (subSubElem.hasAttribute("TRACK_GROUND_PATH_COLOR"))
                    {
                        QColor color(subSubElem.attribute("TRACK_GROUND_PATH_COLOR"));
                        setTrackGroundPathColor(color);
                    }
                    if (subSubElem.hasAttribute("HEAD_SIZE"))
                        trackHeadSized->setCheckState(subSubElem.attribute("HEAD_SIZE").toInt() ? Qt::Checked : Qt::Unchecked);
                    if (subSubElem.hasAttribute("CURRENT_POINT_SIZE"))
                        trackCurrentPointSize->setValue(subSubElem.attribute("CURRENT_POINT_SIZE").toInt());
                    if (subSubElem.hasAttribute("POINTS_SIZE"))
                        trackPointSize->setValue(subSubElem.attribute("POINTS_SIZE").toInt());
                    if (subSubElem.hasAttribute("PATH_SIZE"))
                        trackPathWidth->setValue(subSubElem.attribute("PATH_SIZE").toInt());
                    if (subSubElem.hasAttribute("COLLECTIVE_COLOR_SIZE"))
                        trackColColorSize->setValue(subSubElem.attribute("COLLECTIVE_COLOR_SIZE").toInt());
                    if (subSubElem.hasAttribute("COLOR_MARKER_SIZE"))
                        trackColorMarkerSize->setValue(subSubElem.attribute("COLOR_MARKER_SIZE").toInt());
                    if (subSubElem.hasAttribute("NUMBER_SIZE"))
                        trackNumberSize->setValue(subSubElem.attribute("NUMBER_SIZE").toInt());
                    if (subSubElem.hasAttribute("GROUND_POSITION_SIZE"))
                        trackGroundPositionSize->setValue(subSubElem.attribute("GROUND_POSITION_SIZE").toInt());
                    if (subSubElem.hasAttribute("GROUND_PATH_SIZE"))
                        trackGroundPathSize->setValue(subSubElem.attribute("GROUND_PATH_SIZE").toInt());

                    if (subSubElem.hasAttribute("POINTS_COLORED"))
                        trackShowPointsColored->setCheckState(subSubElem.attribute("POINTS_COLORED").toInt() ? Qt::Checked : Qt::Unchecked);
                    if (subSubElem.hasAttribute("NUMBER_BOLD"))
                        trackNumberBold->setCheckState(subSubElem.attribute("NUMBER_BOLD").toInt() ? Qt::Checked : Qt::Unchecked);
                    if (subSubElem.hasAttribute("BEFORE"))
                        trackShowBefore->setValue(subSubElem.attribute("BEFORE").toInt());
                    if (subSubElem.hasAttribute("AFTER"))
                        trackShowAfter->setValue(subSubElem.attribute("AFTER").toInt());
                }
                else
                    debout << "Unknown TRACKING tag " << subSubElem.tagName() << endl;
        }
        else if (subElem.tagName() == "ANALYSIS")
        {
            for(subSubElem = subElem.firstChildElement(); !subSubElem.isNull(); subSubElem = subSubElem.nextSiblingElement())
                if (subSubElem.tagName() == "SEARCH_MISSING_FRAMES")
                {
                    if (subSubElem.hasAttribute("ENABLED"))
                        anaMissingFrames->setCheckState(subSubElem.attribute("ENABLED").toInt() ? Qt::Checked : Qt::Unchecked);
                }
                else if (subSubElem.tagName() == "MARK_ACTUAL")
                {
                    if (subSubElem.hasAttribute("ENABLED"))
                        anaMarkAct->setCheckState(subSubElem.attribute("ENABLED").toInt() ? Qt::Checked : Qt::Unchecked);
                }
                else if (subSubElem.tagName() == "CALCULATION")
                {
                    if (subSubElem.hasAttribute("STEP_SIZE"))
                        anaStep->setValue(subSubElem.attribute("STEP_SIZE").toInt());
                    if (subSubElem.hasAttribute("CONSIDER_X"))
                        anaConsiderX->setCheckState(subSubElem.attribute("CONSIDER_X").toInt() ? Qt::Checked : Qt::Unchecked);
                    if (subSubElem.hasAttribute("CONSIDER_Y"))
                        anaConsiderY->setCheckState(subSubElem.attribute("CONSIDER_Y").toInt() ? Qt::Checked : Qt::Unchecked);
                    if (subSubElem.hasAttribute("ABSOLUTE"))
                        anaConsiderAbs->setCheckState(subSubElem.attribute("ABSOLUTE").toInt() ? Qt::Checked : Qt::Unchecked);
                    if (subSubElem.hasAttribute("REVERSE"))
                        anaConsiderRev->setCheckState(subSubElem.attribute("REVERSE").toInt() ? Qt::Checked : Qt::Unchecked);
                    if (subSubElem.hasAttribute("SHOW_VORONOI"))
                        showVoronoiCells->setCheckState(subSubElem.attribute("SHOW_VORONOI").toInt() ? Qt::Checked : Qt::Unchecked);
                }
                else
                    debout << "Unknown ANAYSIS tag " << subSubElem.tagName() << endl;
        }
        else
            debout << "Unknown CONTROL tag " << subElem.tagName() << endl;

    mMainWindow->updateCoord();
}


