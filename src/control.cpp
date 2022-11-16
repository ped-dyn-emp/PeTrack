/*
 * PeTrack - Software for tracking pedestrians movement in videos
 * Copyright (C) 2022 Forschungszentrum Jülich GmbH, IAS-7
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

#include "control.h"

#include "IO.h"
#include "analysePlot.h"
#include "backgroundItem.h"
#include "calibFilter.h"
#include "codeMarkerWidget.h"
#include "colorMarkerWidget.h"
#include "colorPlot.h"
#include "colorRangeWidget.h"
#include "imageItem.h"
#include "moCapItem.h"
#include "multiColorMarkerWidget.h"
#include "pMessageBox.h"
#include "petrack.h"
#include "player.h"
#include "roiItem.h"
#include "stereoWidget.h"
#include "tracker.h"
#include "trackerItem.h"
#include "ui_control.h"
#include "view.h"

#include <QDomElement>
#include <iomanip>

#define DEFAULT_HEIGHT 180.0

Control::Control(
    QWidget          &parent,
    QGraphicsScene   &scene,
    reco::Recognizer &recognizer,
    RoiItem          &trackRoiItem,
    RoiItem          &recoRoiItem,
    MissingFrames    &missingFrames) :
    Control(parent, scene, recognizer, trackRoiItem, recoRoiItem, new Ui::Control(), missingFrames)
{
}

Control::Control(
    QWidget          &parent,
    QGraphicsScene   &scene,
    reco::Recognizer &recognizer,
    RoiItem          &trackRoiItem,
    RoiItem          &recoRoiItem,
    Ui::Control      *ui,
    MissingFrames    &missingFrames) :
    QWidget(&parent), mUi(ui)
{
    setAccessibleName("Control");
    mMainWindow = (class Petrack *) &parent;
    mScene      = &scene;
    mLoading    = false;
    // beim erzeugen von new colorplot absturz!!!!
    mUi->setupUi(this);

    // Observers for moCapShow, moCapSize and moCapColor in moCapController;
    // updates UI value when changed.
    QObject::connect(
        &mMainWindow->getMoCapController(), &MoCapController::showMoCapChanged, this, &Control::setMoCapShow);
    QObject::connect(
        &mMainWindow->getMoCapController(), &MoCapController::thicknessChanged, this, &Control::setMoCapSize);
    QObject::connect(&mMainWindow->getMoCapController(), &MoCapController::colorChanged, this, &Control::setMoCapColor);
    // Pulls all observable attributes
    mMainWindow->getMoCapController().notifyAllObserver();

    mUi->missingFramesCalculated->setAttribute(Qt::WA_TransparentForMouseEvents);
    mUi->missingFramesCalculated->setFocusPolicy(Qt::NoFocus);

    QObject::connect(mUi->missingFramesReset, &QPushButton::clicked, &missingFrames, &MissingFrames::reset);
    QObject::connect(
        &missingFrames, &MissingFrames::executeChanged, mUi->missingFramesCalculated, &QCheckBox::setChecked);

    // Weitere Verzerrungsparameter werden vllt. spaeter mal gebraucht bisher von OpenCV nicht beruecksichtig. Muessen
    // dann noch in die Oberflaeche eingebaut werden Deniz: OpenCV kann Sie mittlerweile benutzen, wenn man
    // calibrateCamera die Flag CALIB_RATIONAL_MODEL übergibt; Ergibt eine genauere Kalibration Auch rechenintensiver,
    // aber da wir das einmal machen und dann Speichern, ist das auch egal Diese Boxen müssten in die UI eingebaut und
    // der entsprechende Aufruf in AutoCalib müsste angepasst werden


    mUi->filterBrightContrast->setCheckState(
        mMainWindow->getBrightContrastFilter()->getEnabled() ? Qt::Checked : Qt::Unchecked);
    mUi->filterBorder->setCheckState(mMainWindow->getBorderFilter()->getEnabled() ? Qt::Checked : Qt::Unchecked);
    mUi->filterBg->setCheckState(mMainWindow->getBackgroundFilter()->getEnabled() ? Qt::Checked : Qt::Unchecked);
    mUi->apply->setCheckState(mMainWindow->getCalibFilter()->getEnabled() ? Qt::Checked : Qt::Unchecked);

    mUi->filterSwap->setCheckState(mMainWindow->getSwapFilter()->getEnabled() ? Qt::Checked : Qt::Unchecked);
    mUi->filterSwapH->setCheckState(
        (bool) mMainWindow->getSwapFilter()->getSwapHorizontally()->getValue() ? Qt::Checked : Qt::Unchecked);
    mUi->filterSwapV->setCheckState(
        (bool) mMainWindow->getSwapFilter()->getSwapVertically()->getValue() ? Qt::Checked : Qt::Unchecked);

    setFilterBorderSizeMin(mMainWindow->getBorderFilter()->getBorderSize()->getMinimum());
    setFilterBorderSizeMax(mMainWindow->getBorderFilter()->getBorderSize()->getMaximum());

    setCalibFxMin(mMainWindow->getCalibFilter()->getFx()->getMinimum());
    setCalibFxMax(mMainWindow->getCalibFilter()->getFx()->getMaximum());
    setCalibFx(mMainWindow->getCalibFilter()->getFx()->getValue());

    setCalibFyMin(mMainWindow->getCalibFilter()->getFy()->getMinimum());
    setCalibFyMax(mMainWindow->getCalibFilter()->getFy()->getMaximum());
    setCalibFy(mMainWindow->getCalibFilter()->getFy()->getValue());

    setCalibCx(mMainWindow->getCalibFilter()->getCx()->getValue());

    setCalibCy(mMainWindow->getCalibFilter()->getCy()->getValue());

    setCalibR2Min(mMainWindow->getCalibFilter()->getR2()->getMinimum());
    setCalibR2Max(mMainWindow->getCalibFilter()->getR2()->getMaximum());
    setCalibR2(mMainWindow->getCalibFilter()->getR2()->getValue());

    setCalibR4Min(mMainWindow->getCalibFilter()->getR4()->getMinimum());
    setCalibR4Max(mMainWindow->getCalibFilter()->getR4()->getMaximum());
    setCalibR4(mMainWindow->getCalibFilter()->getR4()->getValue());

    setCalibR6Min(mMainWindow->getCalibFilter()->getR6()->getMinimum());
    setCalibR6Max(mMainWindow->getCalibFilter()->getR6()->getMaximum());
    setCalibR6(mMainWindow->getCalibFilter()->getR6()->getValue());

    setCalibTxMin(mMainWindow->getCalibFilter()->getTx()->getMinimum());
    setCalibTxMax(mMainWindow->getCalibFilter()->getTx()->getMaximum());
    setCalibTx(mMainWindow->getCalibFilter()->getTx()->getValue());

    setCalibTyMin(mMainWindow->getCalibFilter()->getTy()->getMinimum());
    setCalibTyMax(mMainWindow->getCalibFilter()->getTy()->getMaximum());
    setCalibTy(mMainWindow->getCalibFilter()->getTy()->getValue());

    setCalibK4Min(mMainWindow->getCalibFilter()->getK4()->getMinimum());
    setCalibK4Max(mMainWindow->getCalibFilter()->getK4()->getMaximum());
    setCalibK4(mMainWindow->getCalibFilter()->getK4()->getValue());

    setCalibK5Min(mMainWindow->getCalibFilter()->getK5()->getMinimum());
    setCalibK5Max(mMainWindow->getCalibFilter()->getK5()->getMaximum());
    setCalibK5(mMainWindow->getCalibFilter()->getK5()->getValue());

    setCalibK6Min(mMainWindow->getCalibFilter()->getK6()->getMinimum());
    setCalibK6Max(mMainWindow->getCalibFilter()->getK6()->getMaximum());
    setCalibK6(mMainWindow->getCalibFilter()->getK6()->getValue());

    setCalibS1Min(mMainWindow->getCalibFilter()->getS1()->getMinimum());
    setCalibS1Max(mMainWindow->getCalibFilter()->getS1()->getMaximum());
    setCalibS1(mMainWindow->getCalibFilter()->getS1()->getValue());

    setCalibS2Min(mMainWindow->getCalibFilter()->getS2()->getMinimum());
    setCalibS2Max(mMainWindow->getCalibFilter()->getS2()->getMaximum());
    setCalibS2(mMainWindow->getCalibFilter()->getS2()->getValue());

    setCalibS3Min(mMainWindow->getCalibFilter()->getS3()->getMinimum());
    setCalibS3Max(mMainWindow->getCalibFilter()->getS3()->getMaximum());
    setCalibS3(mMainWindow->getCalibFilter()->getS3()->getValue());

    setCalibS4Min(mMainWindow->getCalibFilter()->getS4()->getMinimum());
    setCalibS4Max(mMainWindow->getCalibFilter()->getS4()->getMaximum());
    setCalibS4(mMainWindow->getCalibFilter()->getS4()->getValue());

    setCalibTAUXMin(mMainWindow->getCalibFilter()->getTAUX()->getMinimum());
    setCalibTAUXMax(mMainWindow->getCalibFilter()->getTAUX()->getMaximum());
    setCalibTAUX(mMainWindow->getCalibFilter()->getTAUX()->getValue());

    setCalibTAUYMin(mMainWindow->getCalibFilter()->getTAUY()->getMinimum());
    setCalibTAUYMax(mMainWindow->getCalibFilter()->getTAUY()->getMaximum());
    setCalibTAUY(mMainWindow->getCalibFilter()->getTAUY()->getValue());
    setCalibReprError(mMainWindow->getCalibFilter()->getReprojectionError());


    // statt folgender Zeile kann zB on_cx_valueChanged einfach kodiert werden (su)
    //  connect(mUi->cx, SIGNAL(valueChanged(double mUi->cx)), this, SLOT(on_cx_valueChanged));

    // will be done by designer: mUi->colorPlot->setParent(colorBox); //because it is just integrated via frame in
    // designer

    connect(mUi->extModelCheckBox, &QCheckBox::stateChanged, this, &Control::on_extModelCheckBox_stateChanged);
    mUi->colorPlot->setControlWidget(this);

    mIndexChanging = false;
    mColorChanging = true;
    mUi->recoColorModel->addItem("HSV");
    mUi->recoColorModel->addItem("RGB");

    mUi->recoColorX->addItem("H");
    mUi->recoColorX->addItem("S");
    mUi->recoColorX->addItem("V");

    mUi->recoColorY->addItem("H");
    mUi->recoColorY->addItem("S");
    mUi->recoColorY->addItem("V");
    mUi->recoColorY->setCurrentIndex(1); // default

    mColorChanging = false;

    on_recoColorModel_currentIndexChanged(0);

    // damit eine rectMap vorliegt, die angezeigt werden kann
    mUi->colorPlot->getMapItem()->addMap();

    mUi->analysePlot->setControlWidget(this);

    mIndexChanging = true;

    mUi->recoMethod->addItem("marker casern", QVariant::fromValue(reco::RecognitionMethod::Casern));
    mUi->recoMethod->addItem("marker hermes", QVariant::fromValue(reco::RecognitionMethod::Hermes));
    mUi->recoMethod->addItem("stereo", QVariant::fromValue(reco::RecognitionMethod::Stereo));
    mUi->recoMethod->addItem("color marker", QVariant::fromValue(reco::RecognitionMethod::Color));
    mUi->recoMethod->addItem("marker Japan", QVariant::fromValue(reco::RecognitionMethod::Japan));
    mUi->recoMethod->addItem("multicolor marker", QVariant::fromValue(reco::RecognitionMethod::MultiColor));
    mUi->recoMethod->addItem("code marker", QVariant::fromValue(reco::RecognitionMethod::Code));

    connect(&recognizer, &reco::Recognizer::recoMethodChanged, this, &Control::onRecoMethodChanged);
    connect(this, &Control::userChangedRecoMethod, &recognizer, &reco::Recognizer::userChangedRecoMethod);
    mUi->recoMethod->setCurrentIndex(mUi->recoMethod->findData(QVariant::fromValue(recognizer.getRecoMethod())));

    mUi->scrollArea->setMinimumWidth(
        mUi->scrollAreaWidgetContents->sizeHint().width() + 2 * mUi->scrollArea->frameWidth() +
        mUi->scrollArea->verticalScrollBar()->sizeHint().width() +
        mUi->scrollAreaWidgetContents->layout()->margin() * 2 + mUi->scrollAreaWidgetContents->layout()->spacing() * 2);
    mUi->scrollArea_2->setMinimumWidth(
        mUi->scrollAreaWidgetContents_2->sizeHint().width() + 2 * mUi->scrollArea_2->frameWidth() +
        mUi->scrollArea_2->verticalScrollBar()->sizeHint().width() +
        mUi->scrollAreaWidgetContents_2->layout()->margin() * 2 +
        mUi->scrollAreaWidgetContents_2->layout()->spacing() * 2);
    mUi->scrollArea_3->setMinimumWidth(
        mUi->scrollAreaWidgetContents_3->sizeHint().width() + 2 * mUi->scrollArea_3->frameWidth() +
        mUi->scrollArea_3->verticalScrollBar()->sizeHint().width() +
        mUi->scrollAreaWidgetContents_3->layout()->margin() * 2 +
        mUi->scrollAreaWidgetContents_3->layout()->spacing() * 2);
    mUi->scrollArea_4->setMinimumWidth(
        mUi->scrollAreaWidgetContents_4->sizeHint().width() + 2 * mUi->scrollArea_4->frameWidth() +
        mUi->scrollArea_4->verticalScrollBar()->sizeHint().width() +
        mUi->scrollAreaWidgetContents_4->layout()->margin() * 2 +
        mUi->scrollAreaWidgetContents_4->layout()->spacing() * 2);


    connect(mUi->trackRoiFix, &QCheckBox::stateChanged, &trackRoiItem, &RoiItem::setFixed);
    connect(mUi->recoRoiToFullImageSize, &QPushButton::clicked, &recoRoiItem, &RoiItem::setToFullImageSize);
    connect(
        mUi->recoRoiAdjustAutomatically,
        &QPushButton::clicked,
        this,
        [&recoRoiItem, &trackRoiItem]() { recoRoiItem.adjustToOtherROI(trackRoiItem, std::minus<>()); });

    connect(mUi->roiFix, &QCheckBox::stateChanged, &recoRoiItem, &RoiItem::setFixed);
    connect(mUi->trackRoiToFullImageSize, &QPushButton::clicked, &trackRoiItem, &RoiItem::setToFullImageSize);
    connect(
        mUi->trackRoiAdjustAutomatically,
        &QPushButton::clicked,
        this,
        [&recoRoiItem, &trackRoiItem]() { trackRoiItem.adjustToOtherROI(recoRoiItem, std::plus<>()); });

    connect(mUi->roiFix, &QCheckBox::stateChanged, this, &Control::toggleRecoROIButtons);
    connect(mUi->roiShow, &QCheckBox::stateChanged, this, &Control::toggleRecoROIButtons);

    connect(mUi->trackRoiFix, &QCheckBox::stateChanged, this, &Control::toggleTrackROIButtons);
    connect(mUi->trackRoiShow, &QCheckBox::stateChanged, this, &Control::toggleTrackROIButtons);

    // "Hide" analysis tab until it is fixed
    mUi->tabs->removeTab(3);
}

void Control::setScene(QGraphicsScene *sc)
{
    mScene = sc;
}

void Control::toggleOnlineTracking()
{
    mUi->trackOnlineCalc->toggle();
}

bool Control::isOnlineTrackingChecked() const
{
    return mUi->trackOnlineCalc->isChecked();
}

void Control::setOnlineTrackingChecked(bool checked)
{
    mUi->trackOnlineCalc->setChecked(checked);
}

int Control::getTrackRegionLevels() const
{
    return mUi->trackRegionLevels->value();
}

bool Control::isTrackRepeatChecked() const
{
    return mUi->trackRepeat->isChecked();
}

int Control::getTrackRepeatQual() const
{
    return mUi->trackRepeatQual->value();
}

int Control::getTrackRegionScale() const
{
    return mUi->trackRegionScale->value();
}

bool Control::isTrackMergeChecked() const
{
    return mUi->trackMerge->isChecked();
}

bool Control::isTrackExtrapolationChecked() const
{
    return mUi->trackExtrapolation->isChecked();
}

QLineEdit *Control::trackShowOnlyNrList()
{
    return mUi->trackShowOnlyNrList;
}

void Control::setTrackNumberNow(const QString &val)
{
    mUi->trackNumberNow->setText(val);
}

void Control::setTrackShowOnlyNr(int val)
{
    mUi->trackShowOnlyNr->setValue(val);
}

int Control::getTrackShowOnlyNr() const
{
    return mUi->trackShowOnlyNr->value();
}

void Control::setTrackNumberAll(const QString &number)
{
    mUi->trackNumberAll->setText(number);
}

void Control::setTrackShowOnlyNrMaximum(int max)
{
    mUi->trackShowOnlyNr->setMaximum(max);
}

void Control::setTrackShowOnly(Qt::CheckState state)
{
    mUi->trackShowOnly->setCheckState(state);
}

bool Control::isTrackShowOnlyChecked() const
{
    return mUi->trackShowOnly->isChecked();
}

bool Control::isTrackShowOnlyListChecked() const
{
    return mUi->trackShowOnlyList->isChecked();
}

void Control::setTrackShowOnlyListChecked(bool checked)
{
    mUi->trackShowOnlyList->setChecked(checked);
}

void Control::setTrackNumberVisible(const QString &value)
{
    mUi->trackNumberVisible->setText(value);
}

bool Control::isTrackOnlySelectedChecked() const
{
    return mUi->trackOnlySelected->isChecked();
}

int Control::getTrackShowBefore() const
{
    return mUi->trackShowBefore->value();
}

int Control::getTrackShowAfter() const
{
    return mUi->trackShowAfter->value();
}

bool Control::getTrackShow() const
{
    return mUi->trackShow->isChecked();
}
void Control::setTrackShow(bool b)
{
    mUi->trackShow->setChecked(b);
}

bool Control::getTrackFix() const
{
    return mUi->trackFix->isChecked();
}
void Control::setTrackFix(bool b)
{
    mUi->trackFix->setChecked(b);
}

QColor Control::getTrackPathColor() const
{
    return mUi->trackPathColorButton->palette().color(QPalette::Button);
}

void Control::setTrackPathColor(QColor col)
{
    QPalette pal = mUi->trackPathColorButton->palette();
    pal.setColor(QPalette::Button, col);
    mUi->trackPathColorButton->setPalette(pal);
}

QColor Control::getTrackGroundPathColor() const
{
    return mUi->trackGroundPathColorButton->palette().color(QPalette::Button);
}

void Control::setTrackGroundPathColor(QColor col)
{
    QPalette pal = mUi->trackGroundPathColorButton->palette();
    pal.setColor(QPalette::Button, col);
    mUi->trackGroundPathColorButton->setPalette(pal);
}

int Control::getTrackCurrentPointSize() const
{
    return mUi->trackCurrentPointSize->value();
}
int Control::getTrackCurrentPointLineWidth() const
{
    return mUi->trackCurrentPointLineWidth->value();
}
int Control::getTrackPointSize() const
{
    return mUi->trackPointSize->value();
}
int Control::getTrackShowPointsLineWidth() const
{
    return mUi->trackShowPointsLineWidth->value();
}
int Control::getTrackColColorSize() const
{
    return mUi->trackColColorSize->value();
}
int Control::getTrackColorMarkerSize() const
{
    return mUi->trackColorMarkerSize->value();
}
int Control::getTrackColorMarkerLineWidth() const
{
    return mUi->trackColorMarkerLineWidth->value();
}
int Control::getTrackNumberSize() const
{
    return mUi->trackNumberSize->value();
}

int Control::getTrackGroundPositionSize() const
{
    return mUi->trackGroundPositionSize->value();
}

int Control::getTrackGroundPathSize() const
{
    return mUi->trackGroundPathSize->value();
}

int Control::getTrackPathWidth() const
{
    return mUi->trackPathWidth->value();
}

bool Control::isTrackNumberBoldChecked() const
{
    return mUi->trackNumberBold->isChecked();
}

bool Control::isShowVoronoiCellsChecked() const
{
    return mUi->showVoronoiCells->isChecked();
}

bool Control::isTrackHeadSizedChecked() const
{
    return mUi->trackHeadSized->isChecked();
}

bool Control::isTrackShowCurrentPointChecked() const
{
    return mUi->trackShowCurrentPoint->isChecked();
}

bool Control::isTrackShowSearchSizeChecked() const
{
    return mUi->trackShowSearchSize->isChecked();
}

bool Control::isTrackShowColorMarkerChecked() const
{
    return mUi->trackShowColorMarker->isChecked();
}

bool Control::isTrackShowColColorChecked() const
{
    return mUi->trackShowColColor->isChecked();
}

bool Control::isTrackShowNumberChecked() const
{
    return mUi->trackShowNumber->isChecked();
}

bool Control::isTrackShowHeightIndividualChecked() const
{
    return mUi->trackShowHeightIndividual->isChecked();
}

bool Control::isTrackShowPointsChecked() const
{
    return mUi->trackShowPoints->isChecked();
}

bool Control::isTrackShowPathChecked() const
{
    return mUi->trackShowPath->isChecked();
}

bool Control::isTrackShowGroundPathChecked() const
{
    return mUi->trackShowGroundPath->isChecked();
}

bool Control::isTrackShowOnlyVisibleChecked() const
{
    return mUi->trackShowOnlyVisible->isChecked();
}

bool Control::isTrackShowPointsColoredChecked() const
{
    return mUi->trackShowPointsColored->isChecked();
}

bool Control::isTrackShowGroundPositionChecked() const
{
    return mUi->trackShowGroundPosition->isChecked();
}

/**
 * @brief Getter for MoCapColor selection
 * @return the current selected color
 * */
QColor Control::getMoCapColor() const
{
    return mUi->moCapColorButton->palette().color(QPalette::Button);
}

bool Control::isPerformRecognitionChecked() const
{
    return mUi->performRecognition->isChecked();
}

void Control::setPerformRecognitionChecked(bool checked)
{
    mUi->performRecognition->setChecked(checked);
}

void Control::setRecoNumberNow(const QString &val)
{
    mUi->recoNumberNow->setText(val);
}

int Control::getRecoStep() const
{
    return mUi->recoStep->value();
}

/**
 * @brief Setter for MoCapShow
 *
 * Sets the visibility of the moCap visualization
 * */
void Control::setMoCapShow(bool visibility)
{
    if(mUi->showMoCap->isChecked() != visibility)
    {
        mUi->showMoCap->toggle();
    }
}

/**
 * @brief Setter for MoCapColor palette and button
 *
 * Sets the button color and the palette selection
 * */
void Control::setMoCapColor(QColor col)
{
    if(getMoCapColor() != col)
    {
        QPalette pal = mUi->moCapColorButton->palette();
        pal.setColor(QPalette::Button, col);
        mUi->moCapColorButton->setPalette(pal);
    }
}

/**
 * @brief Setter for MoCapSize
 *
 * Sets the moCap line thickness
 * */
void Control::setMoCapSize(int size)
{
    if(mUi->moCapSize->value() != size)
    {
        mUi->moCapSize->setValue(size);
    }
}

bool Control::getRecoRoiShow() const
{
    return mUi->roiShow->isChecked();
}

void Control::setRecoRoiShow(bool b)
{
    mUi->roiShow->setChecked(b);
}

bool Control::getRecoRoiFix() const
{
    return mUi->roiFix->isChecked();
}

void Control::setRecoRoiFix(bool b)
{
    mUi->roiFix->setChecked(b);
}

bool Control::getTrackRoiShow() const
{
    return mUi->trackRoiShow->isChecked();
}

void Control::setTrackRoiShow(bool b)
{
    mUi->trackRoiShow->setChecked(b);
}

bool Control::getTrackRoiFix() const
{
    return mUi->trackRoiFix->isChecked();
}

void Control::setTrackRoiFix(bool b)
{
    mUi->trackRoiFix->setChecked(b);
}

bool Control::getAdaptiveLevel() const
{
    return mUi->adaptiveLevel->isChecked();
}

int Control::getFilterBorderSize() const
{
    return mUi->filterBorderParamSize->value();
}
void Control::setFilterBorderSizeMin(int i)
{
    mUi->filterBorderParamSize->setMinimum(i);
    mUi->filterBorderParamSize_spin->setMinimum(i);
}
void Control::setFilterBorderSizeMax(int i)
{
    mUi->filterBorderParamSize->setMaximum(i);
    mUi->filterBorderParamSize_spin->setMaximum(i);
}

bool Control::isFilterBgChecked() const
{
    return mUi->filterBg->isChecked();
}

bool Control::isFilterBgDeleteTrjChecked() const
{
    return mUi->filterBgDeleteTrj->isChecked();
}

int Control::getFilterBgDeleteNumber() const
{
    return mUi->filterBgDeleteNumber->value();
}

double Control::getCalibFx() const
{
    return mUi->fx->value();
}

void Control::setCalibFx(double d)
{
    mUi->fx->setValue(d);
}

void Control::setCalibFxMin(double d)
{
    mUi->fx->setMinimum(d);
}

void Control::setCalibFxMax(double d)
{
    mUi->fx->setMaximum(d);
}

double Control::getCalibFy() const
{
    return mUi->fy->value();
}

void Control::setCalibFy(double d)
{
    mUi->fy->setValue(d);
}

void Control::setCalibFyMin(double d)
{
    mUi->fy->setMinimum(d);
}

void Control::setCalibFyMax(double d)
{
    mUi->fy->setMaximum(d);
}

double Control::getCalibCx() const
{
    return mUi->cx->value();
}

void Control::setCalibCx(double d)
{
    mUi->cx->setValue(d);
}

void Control::setCalibCxMin(double d)
{
    mUi->cx->setMinimum(d);
}

void Control::setCalibCxMax(double d)
{
    mUi->cx->setMaximum(d);
}

double Control::getCalibCy() const
{
    return mUi->cy->value();
}

void Control::setCalibCy(double d)
{
    mUi->cy->setValue(d);
}

void Control::setCalibCyMin(double d)
{
    mUi->cy->setMinimum(d);
}

void Control::setCalibCyMax(double d)
{
    mUi->cy->setMaximum(d);
}

double Control::getCalibR2() const
{
    return mUi->r2->value();
}

void Control::setCalibR2(double d)
{
    mUi->r2->setValue(d);
}

void Control::setCalibR2Min(double d)
{
    mUi->r2->setMinimum(d);
}

void Control::setCalibR2Max(double d)
{
    mUi->r2->setMaximum(d);
}

double Control::getCalibR4() const
{
    return mUi->r4->value();
}

void Control::setCalibR4(double d)
{
    mUi->r4->setValue(d);
}

void Control::setCalibR4Min(double d)
{
    mUi->r4->setMinimum(d);
}

void Control::setCalibR4Max(double d)
{
    mUi->r4->setMaximum(d);
}

double Control::getCalibR6() const
{
    return mUi->r6->value();
}

void Control::setCalibR6(double d)
{
    mUi->r6->setValue(d);
}

void Control::setCalibR6Min(double d)
{
    mUi->r6->setMinimum(d);
}
void Control::setCalibR6Max(double d)
{
    mUi->r6->setMaximum(d);
}

double Control::getCalibS1() const
{
    return mUi->s1->value();
}
void Control::setCalibS1(double d)
{
    mUi->s1->setValue(d);
}
void Control::setCalibS1Min(double d)
{
    mUi->s1->setMinimum(d);
}
void Control::setCalibS1Max(double d)
{
    mUi->s1->setMaximum(d);
}

double Control::getCalibS2() const
{
    return mUi->s2->value();
}
void Control::setCalibS2(double d)
{
    mUi->s2->setValue(d);
}
void Control::setCalibS2Min(double d)
{
    mUi->s2->setMinimum(d);
}
void Control::setCalibS2Max(double d)
{
    mUi->s2->setMaximum(d);
}

double Control::getCalibS3() const
{
    return mUi->s3->value();
}
void Control::setCalibS3(double d)
{
    mUi->s3->setValue(d);
}
void Control::setCalibS3Min(double d)
{
    mUi->s3->setMinimum(d);
}
void Control::setCalibS3Max(double d)
{
    mUi->s3->setMaximum(d);
}

double Control::getCalibS4() const
{
    return mUi->s4->value();
}
void Control::setCalibS4(double d)
{
    mUi->s4->setValue(d);
}
void Control::setCalibS4Min(double d)
{
    mUi->s4->setMinimum(d);
}
void Control::setCalibS4Max(double d)
{
    mUi->s4->setMaximum(d);
}

double Control::getCalibTAUX() const
{
    return mUi->taux->value();
}
void Control::setCalibTAUX(double d)
{
    mUi->taux->setValue(d);
}
void Control::setCalibTAUXMin(double d)
{
    mUi->taux->setMinimum(d);
}
void Control::setCalibTAUXMax(double d)
{
    mUi->taux->setMaximum(d);
}

double Control::getCalibTAUY() const
{
    return mUi->tauy->value();
}
void Control::setCalibTAUY(double d)
{
    mUi->tauy->setValue(d);
}
void Control::setCalibTAUYMin(double d)
{
    mUi->tauy->setMinimum(d);
}
void Control::setCalibTAUYMax(double d)
{
    mUi->tauy->setMaximum(d);
}

void Control::setCalibReprError(double d)
{
    mUi->intrError->setText(QString("%1").arg(d));
}
void Control::setExtModelChecked(bool b)
{
    if(b)
    {
        mUi->extModelCheckBox->setChecked(true);
    }
    else
    {
        mUi->extModelCheckBox->setChecked(false);
    }
}

bool Control::isFixCenterChecked() const
{
    return mUi->fixCenter->isChecked();
}

bool Control::isQuadAspectRatioChecked() const
{
    return mUi->quadAspectRatio->isChecked();
}

bool Control::isTangDistChecked() const
{
    return mUi->tangDist->isChecked();
}

bool Control::isExtModelChecked() const
{
    return mUi->extModelCheckBox->isChecked();
}


double Control::getCalibExtrRot1()
{
    return mUi->rot1->value();
}

void Control::setCalibExtrRot1(double d)
{
    mUi->rot1->setValue(d);
}

double Control::getCalibExtrRot2()
{
    return mUi->rot2->value();
}

void Control::setCalibExtrRot2(double d)
{
    mUi->rot2->setValue(d);
}

double Control::getCalibExtrRot3()
{
    return mUi->rot3->value();
}

void Control::setCalibExtrRot3(double d)
{
    mUi->rot3->setValue(d);
}

double Control::getCalibExtrTrans1()
{
    return mUi->trans1->value();
}

void Control::setCalibExtrTrans1(double d)
{
    mUi->trans1->setValue(d);
}

double Control::getCalibExtrTrans2()
{
    return mUi->trans2->value();
}

void Control::setCalibExtrTrans2(double d)
{
    mUi->trans2->setValue(d);
}

double Control::getCalibExtrTrans3()
{
    return mUi->trans3->value();
}

void Control::setCalibExtrTrans3(double d)
{
    mUi->trans3->setValue(d);
}

double Control::getCalibTx() const
{
    return mUi->tx->value();
}

void Control::setCalibTx(double d)
{
    mUi->tx->setValue(d);
}

void Control::setCalibTxMin(double d)
{
    mUi->tx->setMinimum(d);
}

void Control::setCalibTxMax(double d)
{
    mUi->tx->setMaximum(d);
}

double Control::getCalibTy() const
{
    return mUi->ty->value();
}

void Control::setCalibTy(double d)
{
    mUi->ty->setValue(d);
}

void Control::setCalibTyMin(double d)
{
    mUi->ty->setMinimum(d);
}

void Control::setCalibTyMax(double d)
{
    mUi->ty->setMaximum(d);
}

double Control::getCalibK4() const
{
    return mUi->k4->value();
}

void Control::setCalibK4(double d)
{
    mUi->k4->setValue(d);
}

void Control::setCalibK4Min(double d)
{
    mUi->k4->setMinimum(d);
}

void Control::setCalibK4Max(double d)
{
    mUi->k4->setMaximum(d);
}

double Control::getCalibK5() const
{
    return mUi->k5->value();
}

void Control::setCalibK5(double d)
{
    mUi->k5->setValue(d);
}

void Control::setCalibK5Min(double d)
{
    mUi->k5->setMinimum(d);
}

void Control::setCalibK5Max(double d)
{
    mUi->k5->setMaximum(d);
}

double Control::getCalibK6() const
{
    return mUi->k6->value();
}

void Control::setCalibK6(double d)
{
    mUi->k6->setValue(d);
}

void Control::setCalibK6Min(double d)
{
    mUi->k6->setMinimum(d);
}

void Control::setCalibK6Max(double d)
{
    mUi->k6->setMaximum(d);
}

int Control::getCalibGridDimension()
{
    return mUi->gridTab->currentIndex();
}

bool Control::getCalibGridShow()
{
    return mUi->gridShow->isChecked();
}

void Control::setCalibGridShow(bool b)
{
    mUi->gridShow->setChecked(b);
}

bool Control::getCalibGridFix()
{
    return mUi->gridFix->isChecked();
}

void Control::setCalibGridFix(bool b)
{
    mUi->gridFix->setChecked(b);
}

int Control::getCalibGridRotate()
{
    return mUi->gridRotate->value();
}

void Control::setCalibGridRotate(int i)
{
    mUi->gridRotate->setValue(i);
}

int Control::getCalibGridTransX()
{
    return mUi->gridTransX->value();
}

void Control::setCalibGridTransX(int i)
{
    mUi->gridTransX->setValue(i);
}

int Control::getCalibGridTransY()
{
    return mUi->gridTransY->value();
}

void Control::setCalibGridTransY(int i)
{
    mUi->gridTransY->setValue(i);
}

int Control::getCalibGridScale()
{
    return mUi->gridScale->value();
}

void Control::setCalibGridScale(int i)
{
    mUi->gridScale->setValue(i);
}

void Control::setEnabledExtrParams(bool enable)
{
    mUi->rot1->setEnabled(enable);
    mUi->rot2->setEnabled(enable);
    mUi->rot3->setEnabled(enable);
    mUi->trans1->setEnabled(enable);
    mUi->trans2->setEnabled(enable);
    mUi->trans3->setEnabled(enable);
}

void Control::setGridMinMaxTranslation(int minx, int maxx, int miny, int maxy)
{
    mUi->grid3DTransX->setMinimum(minx);
    mUi->grid3DTransX_spin->setMinimum(minx);
    mUi->grid3DTransX->setMaximum(maxx);
    mUi->grid3DTransX_spin->setMaximum(maxx);
    mUi->grid3DTransY->setMinimum(miny);
    mUi->grid3DTransY_spin->setMinimum(miny);
    mUi->grid3DTransY->setMaximum(maxy);
    mUi->grid3DTransY_spin->setMaximum(maxy);
    mUi->grid3DTransZ->setMinimum(-200);
    mUi->grid3DTransZ_spin->setMinimum(-200);
    mUi->grid3DTransZ->setMaximum(500);
    mUi->grid3DTransZ_spin->setMaximum(500);
}

int Control::getCalibGrid3DTransX()
{
    return mUi->grid3DTransX->value();
}

void Control::setCalibGrid3DTransX(int i)
{
    mUi->grid3DTransX->setValue(i);
}

int Control::getCalibGrid3DTransY()
{
    return mUi->grid3DTransY->value();
}

void Control::setCalibGrid3DTransY(int i)
{
    mUi->grid3DTransY->setValue(i);
}

int Control::getCalibGrid3DTransZ()
{
    return mUi->grid3DTransZ->value();
}

void Control::setCalibGrid3DTransZ(int i)
{
    mUi->grid3DTransZ->setValue(i);
}

int Control::getCalibGrid3DResolution()
{
    return mUi->grid3DResolution->value();
}

void Control::setCalibGrid3DResolution(int i)
{
    mUi->grid3DResolution->setValue(i);
}

int Control::getCalibCoordDimension()
{
    return mUi->coordTab->currentIndex();
}

bool Control::getCalibExtrCalibPointsShow()
{
    return mUi->extCalibPointsShow->isChecked();
}

bool Control::getCalibExtrVanishPointsShow()
{
    return mUi->extVanishPointsShow->isChecked();
}

bool Control::getCalibCoordShow()
{
    return mUi->coordShow->isChecked();
}

void Control::setCalibCoordShow(bool b)
{
    mUi->coordShow->setChecked(b);
}

bool Control::getCalibCoordFix()
{
    return mUi->coordFix->isChecked();
}

void Control::setCalibCoordFix(bool b)
{
    mUi->coordFix->setChecked(b);
}

int Control::getCalibCoordRotate()
{
    return mUi->coordRotate->value();
}

void Control::setCalibCoordRotate(int i)
{
    mUi->coordRotate->setValue(i);
}

int Control::getCalibCoordTransX()
{
    return mUi->coordTransX->value();
}

void Control::setCalibCoordTransX(int i)
{
    mUi->coordTransX->setValue(i);
}

int Control::getCalibCoordTransXMax()
{
    return mUi->coordTransX->maximum();
}

void Control::setCalibCoordTransXMax(int i)
{
    mUi->coordTransX->setMaximum(i);
    mUi->coordTransX_spin->setMaximum(i);
}

int Control::getCalibCoordTransXMin()
{
    return mUi->coordTransX->minimum();
}

void Control::setCalibCoordTransXMin(int i)
{
    mUi->coordTransX->setMinimum(i);
    mUi->coordTransX_spin->setMinimum(i);
}

int Control::getCalibCoordTransY()
{
    return mUi->coordTransY->value();
}

void Control::setCalibCoordTransY(int i)
{
    mUi->coordTransY->setValue(i);
}

int Control::getCalibCoordTransYMax()
{
    return mUi->coordTransY->maximum();
}

void Control::setCalibCoordTransYMax(int i)
{
    mUi->coordTransY->setMaximum(i);
    mUi->coordTransY_spin->setMaximum(i);
}

int Control::getCalibCoordTransYMin()
{
    return mUi->coordTransY->minimum();
}

void Control::setCalibCoordTransYMin(int i)
{
    mUi->coordTransY->setMinimum(i);
    mUi->coordTransY_spin->setMinimum(i);
}

int Control::getCalibCoordScale()
{
    return mUi->coordScale->value();
}

void Control::setCalibCoordScale(int i)
{
    mUi->coordScale->setValue(i);
}

double Control::getCalibCoordUnit()
{
    return mUi->coordUnit->value();
}

void Control::setCalibCoordUnit(double d)
{
    mUi->coordUnit->setValue(d);
}

bool Control::isCoordUseIntrinsicChecked() const
{
    return mUi->coordUseIntrinsic->isChecked();
}

int Control::getCalibCoord3DTransX()
{
    return mUi->coord3DTransX->value();
}

void Control::setCalibCoord3DTransX(int i)
{
    mUi->coord3DTransX->setValue(i);
}

int Control::getCalibCoord3DTransY()
{
    return mUi->coord3DTransY->value();
}

void Control::setCalibCoord3DTransY(int i)
{
    mUi->coord3DTransY->setValue(i);
}

int Control::getCalibCoord3DTransZ()
{
    return mUi->coord3DTransZ->value();
}

void Control::setCalibCoord3DTransZ(int i)
{
    mUi->coord3DTransZ->setValue(i);
}

int Control::getCalibCoord3DAxeLen()
{
    return mUi->coord3DAxeLen->value();
}

void Control::setCalibCoord3DAxeLen(int i)
{
    mUi->coord3DAxeLen->setValue(i);
}

bool Control::getCalibCoord3DSwapX()
{
    return mUi->coord3DSwapX->isChecked();
}

void Control::setCalibCoord3DSwapX(bool b)
{
    mUi->coord3DSwapX->setChecked(b);
}

bool Control::getCalibCoord3DSwapY()
{
    return mUi->coord3DSwapY->isChecked();
}

void Control::setCalibCoord3DSwapY(bool b)
{
    mUi->coord3DSwapY->setChecked(b);
}

bool Control::getCalibCoord3DSwapZ()
{
    return mUi->coord3DSwapZ->isChecked();
}

void Control::setCalibCoord3DSwapZ(bool b)
{
    mUi->coord3DSwapZ->setChecked(b);
}

//-------------------- analysis
void Control::on_anaCalculate_clicked()
{
    mMainWindow->calculateRealTracker();
    mUi->analysePlot->setScale();
    if(!isLoading())
    {
        mUi->analysePlot->replot();
    }
}

void Control::on_anaStep_valueChanged(int /*i*/)
{
    if(!isLoading())
    {
        mUi->analysePlot->replot();
    }
}

void Control::on_anaMarkAct_stateChanged(int /*i*/)
{
    if(!isLoading())
    {
        mUi->analysePlot->replot();
    }
}

void Control::on_anaConsiderX_stateChanged(int i)
{
    if((i == Qt::Checked) && (mUi->anaConsiderY->isChecked()))
    {
        mUi->anaConsiderAbs->setEnabled(false);
        mUi->anaConsiderRev->setEnabled(false);
    }
    else
    {
        mUi->anaConsiderAbs->setEnabled(true);
        mUi->anaConsiderRev->setEnabled(true);
    }
    if((i == Qt::Unchecked) && (!mUi->anaConsiderY->isChecked()))
    {
        mUi->anaConsiderX->setCheckState(Qt::Checked);
    }
    else
    {
        if(!isLoading())
        {
            mUi->analysePlot->replot();
        }
    }
}

void Control::on_anaConsiderY_stateChanged(int i)
{
    if((i == Qt::Checked) && (mUi->anaConsiderX->isChecked()))
    {
        mUi->anaConsiderAbs->setEnabled(false);
        mUi->anaConsiderRev->setEnabled(false);
    }
    else
    {
        mUi->anaConsiderAbs->setEnabled(true);
        mUi->anaConsiderRev->setEnabled(true);
    }
    if((i == Qt::Unchecked) && (!mUi->anaConsiderX->isChecked()))
    {
        mUi->anaConsiderY->setCheckState(Qt::Checked);
    }
    else
    {
        if(!isLoading())
        {
            mUi->analysePlot->replot();
        }
    }
}

void Control::on_anaConsiderAbs_stateChanged(int /*i*/)
{
    if(!isLoading())
    {
        mUi->analysePlot->replot();
    }
}

void Control::on_anaConsiderRev_stateChanged(int /*i*/)
{
    if(!isLoading())
    {
        mUi->analysePlot->replot();
    }
}

void Control::on_showVoronoiCells_stateChanged(int /*arg1*/)
{
    if(!isLoading())
    {
        mMainWindow->getScene()->update();
    }
}

//------------------- tracking
void Control::on_trackShow_stateChanged(int i)
{
    if(i == Qt::Checked)
    {
        mMainWindow->getTrackerItem()->show();
    }
    else if(i == Qt::Unchecked)
    {
        mMainWindow->getTrackerItem()->hide();
    }
}

void Control::on_trackFix_stateChanged(int /*i*/)
{
    // TODO only make sense if single points are draggable
}

void Control::on_trackOnlineCalc_stateChanged(int i)
{
    if(i == Qt::Checked)
    {
        mMainWindow->setTrackChanged(true); // flag changes of track parameters
        mMainWindow->getTracker()->reset();
        if(!mMainWindow->isLoading())
        {
            mMainWindow->updateImage();
        }
    }
}

void Control::on_trackCalc_clicked()
{
    mMainWindow->trackAll();
}

void Control::on_trackReset_clicked()
{
    if(mMainWindow->getImage())
    {
        cv::Size size;
        size.width  = mMainWindow->getTrackRoiItem()->rect().width();
        size.height = mMainWindow->getTrackRoiItem()->rect().height();
        mMainWindow->setTrackChanged(true);       // flag changes of track parameters
        mMainWindow->setRecognitionChanged(true); // flag changes of recognition parameters
        mMainWindow->getTracker()->init(size);
        if(!mMainWindow->isLoading())
        {
            mMainWindow->updateImage();
        }
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
    if(col.isValid())
    {
        setTrackPathColor(col);
    }
    if(!mMainWindow->isLoading())
    {
        mMainWindow->updateImage();
    }
}

void Control::on_trackGroundPathColorButton_clicked()
{
    QColor col = QColorDialog::getColor(getTrackGroundPathColor(), this);
    if(col.isValid())
    {
        setTrackGroundPathColor(col);
    }
    if(!mMainWindow->isLoading())
    {
        mMainWindow->updateImage();
    }
}

void Control::on_moCapColorButton_clicked()
{
    QColor col = QColorDialog::getColor(getMoCapColor(), this);
    if(col.isValid())
    {
        mMainWindow->getMoCapController().setColor(col);
    }
    mScene->update();
    if(!mMainWindow->isLoading())
    {
        mMainWindow->updateImage();
    }
}

void Control::on_trackRegionScale_valueChanged(int /*i*/)
{
    mMainWindow->setTrackChanged(true);
    mMainWindow->getTracker()->reset();
    if(!mMainWindow->isLoading())
    {
        mMainWindow->updateImage();
    }
}

void Control::on_trackRegionLevels_valueChanged(int /*i*/)
{
    mMainWindow->setTrackChanged(true);
    mMainWindow->getTracker()->reset();
    if(!mMainWindow->isLoading())
    {
        mMainWindow->updateImage();
    }
}

void Control::on_trackShowSearchSize_stateChanged(int /*i*/)
{
    mScene->update();
}

void Control::on_trackShowOnly_stateChanged(int i)
{
    if(i > 0 && mUi->trackShowOnlyList->checkState() == Qt::Checked)
    {
        mUi->trackShowOnlyList->setChecked(false);
    }

    if(!isLoading())
    {
        mScene->update();
    }
}

void Control::on_trackShowOnlyList_stateChanged(int i)
{
    if(i > 0 && mUi->trackShowOnly->checkState() == Qt::Checked)
    {
        mUi->trackShowOnly->setChecked(false);
    }

    mUi->trackShowOnlyListButton->setEnabled(i);
    mUi->trackShowOnlyNrList->setEnabled(i);

    if(!isLoading())
    {
        mScene->update();
    }
}

void Control::on_trackShowOnlyNr_valueChanged(int /*i*/)
{
    if(!mMainWindow->isLoading())
    {
        mScene->update();
    }
}

void Control::on_trackShowOnlyNrList_textChanged(const QString & /*arg1*/)
{
    if(!mMainWindow->isLoading())
    {
        mScene->update();
    }
}

void Control::on_trackGotoNr_clicked()
{
    if(static_cast<int>(mMainWindow->getPersonStorage().nbPersons()) >= mUi->trackShowOnlyNr->value())
    {
        int idx        = mUi->trackShowOnlyNr->value() - 1;
        int firstFrame = mMainWindow->getPersonStorage().at(idx).firstFrame();
        int lastFrame  = mMainWindow->getPersonStorage().at(idx).lastFrame();
        mMainWindow->getPlayer()->skipToFrame((lastFrame + firstFrame) / 2);
    }
}

void Control::on_trackGotoStartNr_clicked()
{
    if(static_cast<int>(mMainWindow->getPersonStorage().nbPersons()) >= mUi->trackShowOnlyNr->value())
    {
        int idx = mUi->trackShowOnlyNr->value() - 1;
        mMainWindow->getPlayer()->skipToFrame(mMainWindow->getPersonStorage().at(idx).firstFrame());
    }
}

void Control::on_trackGotoEndNr_clicked()
{
    if(static_cast<int>(mMainWindow->getPersonStorage().nbPersons()) >= mUi->trackShowOnlyNr->value())
    {
        int idx = mUi->trackShowOnlyNr->value() - 1;
        mMainWindow->getPlayer()->skipToFrame(mMainWindow->getPersonStorage().at(idx).lastFrame());
    }
}

void Control::on_trackShowOnlyListButton_clicked()
{
    QMessageBox nrListBox(mMainWindow);

    nrListBox.setWindowTitle(Petrack::tr("PeTrack"));
    nrListBox.setIcon(QMessageBox::NoIcon);
    nrListBox.setText(Petrack::tr("Select visible pedestrians:"));

    QGridLayout         *layout = (QGridLayout *) nrListBox.layout();
    QVector<QCheckBox *> checkBox;

    for(int i = 0; i < static_cast<int>(mMainWindow->getPersonStorage().nbPersons()); i++)
    {
        /// ToDo: parse from lineEdit
        checkBox.push_back(new QCheckBox(QString::number(i + 1)));
        checkBox.at(i)->setChecked(mMainWindow->getPedestriansToTrack().contains(i));
        layout->addWidget(checkBox.at(i), 3 + i / 5, i % 5, 1, 1);
    }

    QPushButton *ok     = new QPushButton(tr("Ok"));
    QPushButton *cancel = new QPushButton(tr("Cancel"));

    nrListBox.addButton(cancel, QMessageBox::RejectRole);
    nrListBox.addButton(ok, QMessageBox::AcceptRole);

    int res = nrListBox.exec();

    if(res == QMessageBox::Accepted)
    {
        QStringList list;
        int         first = -1, last = -1;
        for(int i = 0; i < static_cast<int>(mMainWindow->getPersonStorage().nbPersons()); i++)
        {
            if(checkBox.at(i)->isChecked())
            {
                if(first == -1)
                {
                    first = i + 1;
                }
            }
            else // not checked
            {
                if(first != -1)
                {
                    last = i;
                    if(first == last)
                    {
                        list.append(QString::number(first));
                    }
                    else
                    {
                        list.append(QString("%1-%2").arg(first).arg(last));
                    }
                    last  = -1;
                    first = -1;
                }
            }
            /// ToDo: lists
        }
        mUi->trackShowOnlyNrList->setText(list.join(","));
    }
}

void Control::on_trackHeadSized_stateChanged(int i)
{
    static int oldHeadSize = 60;
    if(i == Qt::Checked)
    {
        oldHeadSize = mUi->trackCurrentPointSize->value();
        mUi->trackCurrentPointSize->setValue((int) mMainWindow->getHeadSize());
        mUi->trackCurrentPointSize->setDisabled(true);
    }
    else
    {
        mUi->trackCurrentPointSize->setValue(oldHeadSize);
        mUi->trackCurrentPointSize->setEnabled(true);
    }
    mScene->update();
}

void Control::on_showMoCap_stateChanged(int i)
{
    mMainWindow->getMoCapController().setShowMoCap(i == Qt::Checked);
    mScene->update();
}

void Control::on_moCapSize_valueChanged(int i)
{
    mMainWindow->getMoCapController().setThickness(i);
    mScene->update();
}

//------------------- recognition
void Control::on_recoShowColor_stateChanged(int i)
{
    mUi->colorPlot->getTrackerItem()->setVisible(i == Qt::Checked);
    if(!isLoading())
    {
        replotColorplot();
    }
}

void Control::on_recoOptimizeColor_clicked()
{
    mMainWindow->getPersonStorage().optimizeColor();
    replotColorplot();
    mScene->update(); // damit mgl angezeige farbpunkte geaendert/weggenommen werden
}

void Control::on_recoColorModel_currentIndexChanged(int index)
{
    static int xHsvIndex = 0; // H
    static int yHsvIndex = 1; // S
    static int xRgbIndex = 0; // R
    static int yRgbIndex = 1; // G

    if(mColorChanging == false) // damit nicht vom constructor mehrmals bei additem durchlaufen wird
    {
        mColorChanging = true;
        if(index == 0) // HSV
        {
            mUi->recoColorX->setItemText(0, "H");
            mUi->recoColorX->setItemText(1, "S");
            mUi->recoColorX->setItemText(2, "V");

            mUi->recoColorY->setItemText(0, "H");
            mUi->recoColorY->setItemText(1, "S");
            mUi->recoColorY->setItemText(2, "V");

            if(mIndexChanging)
            {
                xRgbIndex = mUi->recoColorX->currentIndex();
                yRgbIndex = mUi->recoColorY->currentIndex();
                mUi->recoColorX->setCurrentIndex(xHsvIndex);
                mUi->recoColorY->setCurrentIndex(yHsvIndex);
            }
        }
        else // index == 1 == RGB
        {
            mUi->recoColorX->setItemText(0, "R");
            mUi->recoColorX->setItemText(1, "G");
            mUi->recoColorX->setItemText(2, "B");

            mUi->recoColorY->setItemText(0, "R");
            mUi->recoColorY->setItemText(1, "G");
            mUi->recoColorY->setItemText(2, "B");

            if(mIndexChanging)
            {
                xHsvIndex = mUi->recoColorX->currentIndex();
                yHsvIndex = mUi->recoColorY->currentIndex();
                mUi->recoColorX->setCurrentIndex(xRgbIndex);
                mUi->recoColorY->setCurrentIndex(yRgbIndex);
            }
        }
        mIndexChanging = true; // firstTime = false
        mColorChanging = false;

        mUi->colorPlot->setScale();
        mUi->colorPlot->generateImage();
        if(!isLoading())
        {
            replotColorplot();
        }
    }
}

void Control::on_recoColorX_currentIndexChanged(int /*index*/)
{
    if(mColorChanging == false)
    {
        mUi->colorPlot->setScale();
        mUi->colorPlot->generateImage();
        if(!isLoading())
        {
            replotColorplot();
        }
    }
    // TODO schoen waere hier das gegenstueck bei y zu disablen, aber combobox bietet dies nicht auf einfachem weg
}

void Control::on_recoColorY_currentIndexChanged(int /*index*/)
{
    if(mColorChanging == false)
    {
        mUi->colorPlot->setScale();
        mUi->colorPlot->generateImage();
        if(!isLoading())
        {
            replotColorplot();
        }
    }
}

void Control::on_recoColorZ_valueChanged(int /*index*/)
{
    if(mColorChanging == false)
    {
        mUi->colorPlot->generateImage();
        if(!isLoading())
        {
            replotColorplot();
        }
    }
}

void Control::on_recoGreyLevel_valueChanged(int index)
{
    mUi->colorPlot->setGreyDiff(index);
    if(!isLoading())
    {
        replotColorplot();
    }
}

void Control::on_recoSymbolSize_valueChanged(int index)
{
    mUi->colorPlot->setSymbolSize(index);
    if(!isLoading())
    {
        replotColorplot();
    }
}
void Control::on_recoStereoShow_clicked()
{
    auto selectedRecognitionMethod = getRecoMethod();
    if(selectedRecognitionMethod == reco::RecognitionMethod::MultiColor)
    {
        mMainWindow->getMultiColorMarkerWidget()->show();
    }
    else if(selectedRecognitionMethod == reco::RecognitionMethod::Color)
    {
        mMainWindow->getColorMarkerWidget()->show();
    }
    else if(selectedRecognitionMethod == reco::RecognitionMethod::Code)
    {
        mMainWindow->getCodeMarkerWidget()->show();
    }
    else
    {
        mMainWindow->getStereoWidget()->show();
    }
}

void Control::on_mapColorRange_clicked()
{
    mMainWindow->getColorRangeWidget()->show();
}

void Control::on_mapNr_valueChanged(int i)
{
    RectMap map = mUi->colorPlot->getMapItem()->getMap(i);

    mUi->mapX->setValue(myRound(2. * map.x()));
    mUi->mapY->setValue(myRound(2. * map.y()));
    mUi->mapW->setValue(myRound(map.width()));
    mUi->mapH->setValue(myRound(map.height()));
    mUi->mapColor->setCheckState(map.colored() ? Qt::Checked : Qt::Unchecked);
    mUi->mapHeight->setValue(map.mapHeight());
    mUi->colorPlot->getMapItem()->changeMap(
        mUi->mapNr->value(),
        mUi->mapX->value() / 2.,
        mUi->mapY->value() / 2.,
        mUi->mapW->value(),
        mUi->mapH->value(),
        mUi->mapColor->isChecked(),
        mUi->mapHeight->value()); // nur, um aktivenindex anzugeben
    mMainWindow->getColorRangeWidget()->setToColor(map.toColor());
    mMainWindow->getColorRangeWidget()->setFromColor(map.fromColor());
    mMainWindow->getColorRangeWidget()->setInvHue(map.invHue());

    if(!isLoading())
    {
        replotColorplot(); // um aktiven gruen anzuzeigen
        mMainWindow->updateImage();
    }
}

void Control::on_mapX_valueChanged(int /*i*/)
{
    mUi->colorPlot->getMapItem()->changeMap(
        mUi->mapNr->value(),
        mUi->mapX->value() / 2.,
        mUi->mapY->value() / 2.,
        mUi->mapW->value(),
        mUi->mapH->value(),
        mUi->mapColor->isChecked(),
        mUi->mapHeight->value());
    if(!isLoading())
    {
        replotColorplot();
    }
}
void Control::on_mapY_valueChanged(int /*i*/)
{
    mUi->colorPlot->getMapItem()->changeMap(
        mUi->mapNr->value(),
        mUi->mapX->value() / 2.,
        mUi->mapY->value() / 2.,
        mUi->mapW->value(),
        mUi->mapH->value(),
        mUi->mapColor->isChecked(),
        mUi->mapHeight->value());
    if(!isLoading())
    {
        replotColorplot();
    }
}
void Control::on_mapW_valueChanged(int /*i*/)
{
    mUi->colorPlot->getMapItem()->changeMap(
        mUi->mapNr->value(),
        mUi->mapX->value() / 2.,
        mUi->mapY->value() / 2.,
        mUi->mapW->value(),
        mUi->mapH->value(),
        mUi->mapColor->isChecked(),
        mUi->mapHeight->value());
    if(!isLoading())
    {
        replotColorplot();
    }
}
void Control::on_mapH_valueChanged(int /*i*/)
{
    mUi->colorPlot->getMapItem()->changeMap(
        mUi->mapNr->value(),
        mUi->mapX->value() / 2.,
        mUi->mapY->value() / 2.,
        mUi->mapW->value(),
        mUi->mapH->value(),
        mUi->mapColor->isChecked(),
        mUi->mapHeight->value());
    if(!isLoading())
    {
        replotColorplot();
    }
}
void Control::on_mapColor_stateChanged(int /*i*/)
{
    mUi->colorPlot->getMapItem()->changeMap(
        mUi->mapNr->value(),
        mUi->mapX->value() / 2.,
        mUi->mapY->value() / 2.,
        mUi->mapW->value(),
        mUi->mapH->value(),
        mUi->mapColor->isChecked(),
        mUi->mapHeight->value());
    if(!isLoading())
    {
        replotColorplot();
    }
}
void Control::on_mapHeight_valueChanged(double /*d*/)
{
    mUi->colorPlot->getMapItem()->changeMap(
        mUi->mapNr->value(),
        mUi->mapX->value() / 2.,
        mUi->mapY->value() / 2.,
        mUi->mapW->value(),
        mUi->mapH->value(),
        mUi->mapColor->isChecked(),
        mUi->mapHeight->value());
}

void Control::on_mapHeight_editingFinished()
{
    if(mUi->mapHeight->value() < 100)
    {
        PInformation(
            this,
            "Height is in cm, not m!",
            "You put in a low value for height.\n Gentle reminder that height is in cm, not m.");
    }
}

void Control::on_mapAdd_clicked()
{
    mUi->mapNr->setMaximum(mUi->mapNr->maximum() + 1);
    mUi->colorPlot->getMapItem()->addMap();
    mUi->mapNr->setValue(mUi->mapNr->maximum());
}
void Control::on_mapDel_clicked()
{
    mUi->colorPlot->getMapItem()->delMap(mUi->mapNr->value());
    if(mUi->mapNr->value() == mUi->mapNr->maximum())
    {
        mUi->mapNr->setValue(mUi->mapNr->value() > 0 ? mUi->mapNr->maximum() - 1 : 0);
    }

    mUi->mapNr->setMaximum(mUi->mapNr->maximum() > 0 ? mUi->mapNr->maximum() - 1 : 0);

    if(!isLoading())
    {
        replotColorplot();
    }
}
void Control::on_mapDistribution_clicked()
{
    if(!mUi->colorPlot->printDistribution())
    {
        mMainWindow->getPersonStorage().printHeightDistribution();
    }
}
void Control::on_mapResetHeight_clicked()
{
    mMainWindow->getPersonStorage().resetHeight();
    mScene->update();
}
void Control::on_mapResetPos_clicked()
{
    mMainWindow->getPersonStorage().resetPos();
    mScene->update();
}
void Control::on_mapDefaultHeight_valueChanged(double d)
{
    mMainWindow->setHeadSize();
    mMainWindow->getBackgroundFilter()->setDefaultHeight(d);
}

void Control::on_mapReadHeights_clicked()
{
    QString heightFile = QFileDialog::getOpenFileName(
        mMainWindow,
        Petrack::tr("Select text file with height information"),
        mMainWindow->getHeightFileName(),
        Petrack::tr("Height File (*.txt);;All files (*.*)"));

    auto heights = IO::readHeightFile(heightFile);

    if(std::holds_alternative<std::unordered_map<int, float>>(heights)) // heights contains the height map
    {
        mMainWindow->getPersonStorage().resetHeight();
        mMainWindow->getPersonStorage().setMarkerHeights(std::get<std::unordered_map<int, float>>(heights));
        mMainWindow->setHeightFileName(heightFile);
    }
    else // heights contains an error string
    {
        PCritical(mMainWindow, Petrack::tr("PeTrack"), Petrack::tr(std::get<std::string>(heights).c_str()));
    }

    mMainWindow->getPersonStorage().printHeightDistribution();
    mScene->update();
}

void Control::on_mapReadMarkerID_clicked()
{
    QString markerFile = QFileDialog::getOpenFileName(
        mMainWindow,
        Petrack::tr("Select text file with marker information"),
        mMainWindow->getHeightFileName(),
        Petrack::tr("Marker File (*.txt);;All files (*.*)"));

    auto markerIDs = IO::readMarkerIDFile(markerFile);

    if(std::holds_alternative<std::unordered_map<int, int>>(markerIDs)) // markerIDs contains the marker information
    {
        mMainWindow->getPersonStorage().setMarkerIDs(std::get<std::unordered_map<int, int>>(markerIDs));
        mMainWindow->setMarkerIDFileName(markerFile);
    }
    else // heights contains an error string
    {
        QMessageBox::critical(
            mMainWindow, Petrack::tr("PeTrack"), Petrack::tr(std::get<std::string>(markerIDs).c_str()));
    }

    mMainWindow->getPersonStorage().printHeightDistribution();
    mScene->update();
}

void Control::on_performRecognition_stateChanged(int /*i*/)
{
    mMainWindow->setRecognitionChanged(true); // flag changes of recognition parameters
    if(!mMainWindow->isLoading())
    {
        mMainWindow->updateImage();
    }
}

void Control::on_recoMethod_currentIndexChanged(int /*index*/)
{
    auto method = mUi->recoMethod->itemData(mUi->recoMethod->currentIndex());
    emit userChangedRecoMethod(method.value<reco::RecognitionMethod>());
}

void Control::onRecoMethodChanged(reco::RecognitionMethod method)
{
    mUi->recoMethod->setCurrentIndex(mUi->recoMethod->findData(QVariant::fromValue(method)));
}

void Control::on_markerBrightness_valueChanged(int /*i*/)
{
    mMainWindow->setRecognitionChanged(true); // flag changes of recognition parameters
    if(!mMainWindow->isLoading())
    {
        mMainWindow->updateImage();
    }
}
void Control::on_markerIgnoreWithout_stateChanged(int /*i*/)
{
    mMainWindow->setRecognitionChanged(true); // flag changes of recognition parameters
    if(!mMainWindow->isLoading())
    {
        mMainWindow->updateImage();
    }
}

void Control::on_roiShow_stateChanged(int i)
{
    if(i == Qt::Checked)
    {
        mMainWindow->getRecoRoiItem()->show(); // setVisible
    }
    else if(i == Qt::Unchecked)
    {
        mMainWindow->getRecoRoiItem()->hide(); // setVisible
    }
}
void Control::on_roiFix_stateChanged(int i)
{
    if(i == Qt::Checked)
    {
        mMainWindow->getRecoRoiItem()->setFlag(QGraphicsItem::ItemIsMovable, false);
    }
    else if(i == Qt::Unchecked)
    {
        mMainWindow->getRecoRoiItem()->setAcceptHoverEvents(true);
        mMainWindow->getRecoRoiItem()->setFlag(QGraphicsItem::ItemIsMovable);
    }
}

void Control::on_trackRoiShow_stateChanged(int i)
{
    if(i == Qt::Checked)
    {
        mMainWindow->getTrackRoiItem()->show(); // setVisible
    }
    else if(i == Qt::Unchecked)
    {
        mMainWindow->getTrackRoiItem()->hide(); // setVisible
    }
}

void Control::on_trackRoiFix_stateChanged(int i)
{
    if(i == Qt::Checked)
    {
        mMainWindow->getTrackRoiItem()->setFlag(QGraphicsItem::ItemIsMovable, false);
    }
    else if(i == Qt::Unchecked)
    {
        mMainWindow->getTrackRoiItem()->setAcceptHoverEvents(true);
        mMainWindow->getTrackRoiItem()->setFlag(QGraphicsItem::ItemIsMovable);
    }
}

//---------------------- calibration
void Control::on_filterBrightContrast_stateChanged(int i)
{
    if(i == Qt::Checked)
    {
        mMainWindow->getBrightContrastFilter()->enable();
    }
    else if(i == Qt::Unchecked)
    {
        mMainWindow->getBrightContrastFilter()->disable();
    }
    if(!mMainWindow->isLoading())
    {
        mMainWindow->updateImage();
    }
}

void Control::on_filterContrastParam_valueChanged(int i)
{
    mMainWindow->getBrightContrastFilter()->getContrast()->setValue(i);
    if(!mMainWindow->isLoading())
    {
        mMainWindow->updateImage();
    }
}

void Control::on_filterBrightParam_valueChanged(int i)
{
    mMainWindow->getBrightContrastFilter()->getBrightness()->setValue(i);
    if(!mMainWindow->isLoading())
    {
        mMainWindow->updateImage();
    }
}

void Control::on_filterBorder_stateChanged(int i)
{
    if(i == Qt::Checked)
    {
        mMainWindow->getBorderFilter()->enable();
    }
    else if(i == Qt::Unchecked)
    {
        mMainWindow->getBorderFilter()->disable();
    }
    if(!mMainWindow->isLoading())
    {
        mMainWindow->updateImage();
    }
}

void Control::on_filterBorderParamSize_valueChanged(int i)
{
    mMainWindow->setImageBorderSize(2 * i); // 2* because undistored has problem with sizes not dividable  of 4
    if(!mMainWindow->isLoading())
    {
        mMainWindow->updateImage();
    }
}

void Control::on_filterBorderParamCol_clicked()
{
    BorderFilter *bf    = mMainWindow->getBorderFilter();
    QColor        color = QColorDialog::getColor(
        QColor(
            (int) bf->getBorderColR()->getValue(),
            (int) bf->getBorderColG()->getValue(),
            (int) bf->getBorderColB()->getValue()),
        this);
    bf->getBorderColR()->setValue(color.red());
    bf->getBorderColG()->setValue(color.green());
    bf->getBorderColB()->setValue(color.blue());
    if(!mMainWindow->isLoading())
    {
        mMainWindow->updateImage();
    }
}

void Control::on_filterSwap_stateChanged(int i)
{
    if(i == Qt::Checked)
    {
        mMainWindow->getSwapFilter()->enable();
    }
    else if(i == Qt::Unchecked)
    {
        mMainWindow->getSwapFilter()->disable();
    }
    if(!mMainWindow->isLoading())
    {
        mMainWindow->updateImage();
    }
}

void Control::on_filterSwapH_stateChanged(int i)
{
    if(i == Qt::Checked)
    {
        mMainWindow->getSwapFilter()->getSwapHorizontally()->setValue(1);
    }
    else if(i == Qt::Unchecked)
    {
        mMainWindow->getSwapFilter()->getSwapHorizontally()->setValue(0);
    }
    if(!mMainWindow->isLoading())
    {
        mMainWindow->updateImage();
    }
}

void Control::on_filterSwapV_stateChanged(int i)
{
    if(i == Qt::Checked)
    {
        mMainWindow->getSwapFilter()->getSwapVertically()->setValue(1);
    }
    else if(i == Qt::Unchecked)
    {
        mMainWindow->getSwapFilter()->getSwapVertically()->setValue(0);
    }
    if(!mMainWindow->isLoading())
    {
        mMainWindow->updateImage();
    }
}

void Control::on_filterBg_stateChanged(int i)
{
    static int showCheckState = mUi->filterBgShow->checkState();

    if(i == Qt::Checked)
    {
        mUi->filterBgShow->setEnabled(true);
        mUi->filterBgUpdate->setEnabled(true);
        mUi->filterBgReset->setEnabled(true);
        mUi->filterBgSave->setEnabled(true);
        mUi->filterBgLoad->setEnabled(true);
        if(showCheckState == Qt::Checked)
        {
            mUi->filterBgShow->setCheckState(Qt::Checked);
        }
        mUi->filterBgDeleteNumber->setEnabled(true);
        mUi->filterBgDeleteTrj->setEnabled(true);
        mMainWindow->getBackgroundFilter()->enable();
    }
    else if(i == Qt::Unchecked)
    {
        mUi->filterBgShow->setEnabled(false);
        mUi->filterBgUpdate->setEnabled(false);
        mUi->filterBgReset->setEnabled(false);
        mUi->filterBgSave->setEnabled(false);
        mUi->filterBgLoad->setEnabled(false);
        showCheckState = mUi->filterBgShow->checkState();
        mUi->filterBgShow->setCheckState(Qt::Unchecked);
        mUi->filterBgDeleteNumber->setEnabled(false);
        mUi->filterBgDeleteTrj->setEnabled(false);
        mMainWindow->getBackgroundFilter()->disable();
    }
    if(!mMainWindow->isLoading())
    {
        mMainWindow->updateImage(
            true); // true, da auch bei stehendem bild neue berechnungen durchgefuehrt werden sollen
    }
}

void Control::on_filterBgUpdate_stateChanged(int i)
{
    if(i == Qt::Checked)
    {
        mMainWindow->getBackgroundFilter()->setUpdate(true); //  enable();
    }
    else if(i == Qt::Unchecked)
    {
        mMainWindow->getBackgroundFilter()->setUpdate(false); //->disable();
    }
}

void Control::on_filterBgReset_clicked()
{
    mMainWindow->getBackgroundFilter()->reset();
    if(!mMainWindow->isLoading())
    {
        mMainWindow->updateImage();
    }
}

void Control::on_filterBgShow_stateChanged(int i)
{
    mMainWindow->getBackgroundItem()->setVisible(i);
    mScene->update();
}

void Control::on_filterBgSave_clicked()
{
    mMainWindow->getBackgroundFilter()->save();
}

void Control::on_filterBgLoad_clicked()
{
    mMainWindow->getBackgroundFilter()->load();
    if(!mMainWindow->isLoading())
    {
        mMainWindow->updateImage();
    }
}

void Control::on_apply_stateChanged(int i)
{
    if(i == Qt::Checked)
    {
        mMainWindow->getCalibFilter()->enable();
    }
    else if(i == Qt::Unchecked)
    {
        mMainWindow->getCalibFilter()->disable();
    }
    if(!mMainWindow->isLoading())
    {
        mMainWindow->updateImage();
    }
}

void Control::on_fx_valueChanged(double d)
{
    mIntrinsicCameraParams.cameraMatrix.at<double>(0, 0) = d;
    mMainWindow->getCalibFilter()->getFx()->setValue(d);
    if(mUi->quadAspectRatio->isChecked())
    {
        mUi->fy->setValue(d);
    }
    if(!mMainWindow->isLoading())
    {
        mMainWindow->updateImage();
    }
    setMeasuredAltitude();
    mUi->intrError->setText(QString("invalid"));
}

void Control::on_fy_valueChanged(double d)
{
    mIntrinsicCameraParams.cameraMatrix.at<double>(1, 1) = d;
    mMainWindow->getCalibFilter()->getFy()->setValue(d);
    if(!mMainWindow->isLoading())
    {
        mMainWindow->updateImage();
    }
    setMeasuredAltitude();
    mUi->intrError->setText(QString("invalid"));
}

void Control::on_cx_valueChanged(double d)
{
    mIntrinsicCameraParams.cameraMatrix.at<double>(0, 2) = d;
    mMainWindow->setStatusPosReal();
    mMainWindow->getCalibFilter()->getCx()->setValue(d);
    if(!mMainWindow->isLoading())
    {
        mMainWindow->updateCoord();
    }
    mUi->intrError->setText(QString("invalid"));
}

void Control::on_cy_valueChanged(double d)
{
    mIntrinsicCameraParams.cameraMatrix.at<double>(1, 2) = d;
    mMainWindow->setStatusPosReal();
    mMainWindow->getCalibFilter()->getCy()->setValue(d);
    if(!mMainWindow->isLoading())
    {
        mMainWindow->updateCoord();
    }
    mUi->intrError->setText(QString("invalid"));
}

void Control::on_r2_valueChanged(double d)
{
    mMainWindow->getCalibFilter()->getR2()->setValue(d);
    if(!mMainWindow->isLoading())
    {
        mMainWindow->updateImage();
    }
    mUi->intrError->setText(QString("invalid"));
}

void Control::on_r4_valueChanged(double d)
{
    mMainWindow->getCalibFilter()->getR4()->setValue(d);
    if(!mMainWindow->isLoading())
    {
        mMainWindow->updateImage();
    }
    mUi->intrError->setText(QString("invalid"));
}

void Control::on_r6_valueChanged(double d)
{
    mMainWindow->getCalibFilter()->getR6()->setValue(d);
    if(!mMainWindow->isLoading())
    {
        mMainWindow->updateImage();
    }
    mUi->intrError->setText(QString("invalid"));
}
void Control::on_s1_valueChanged(double d)
{
    mMainWindow->getCalibFilter()->getS1()->setValue(d);
    if(!mMainWindow->isLoading())
    {
        mMainWindow->updateImage();
    }
    mUi->intrError->setText(QString("invalid"));
}
void Control::on_s2_valueChanged(double d)
{
    mMainWindow->getCalibFilter()->getS2()->setValue(d);
    if(!mMainWindow->isLoading())
    {
        mMainWindow->updateImage();
    }
    mUi->intrError->setText(QString("invalid"));
}
void Control::on_s3_valueChanged(double d)
{
    mMainWindow->getCalibFilter()->getS3()->setValue(d);
    if(!mMainWindow->isLoading())
    {
        mMainWindow->updateImage();
    }
    mUi->intrError->setText(QString("invalid"));
}
void Control::on_s4_valueChanged(double d)
{
    mMainWindow->getCalibFilter()->getS4()->setValue(d);
    if(!mMainWindow->isLoading())
    {
        mMainWindow->updateImage();
    }
    mUi->intrError->setText(QString("invalid"));
}
void Control::on_taux_valueChanged(double d)
{
    mMainWindow->getCalibFilter()->getTAUX()->setValue(d);
    if(!mMainWindow->isLoading())
    {
        mMainWindow->updateImage();
    }
    mUi->intrError->setText(QString("invalid"));
}
void Control::on_tauy_valueChanged(double d)
{
    mMainWindow->getCalibFilter()->getTAUY()->setValue(d);
    if(!mMainWindow->isLoading())
    {
        mMainWindow->updateImage();
    }
    mUi->intrError->setText(QString("invalid"));
}


void Control::on_tx_valueChanged(double d)
{
    mMainWindow->getCalibFilter()->getTx()->setValue(d);
    if(!mMainWindow->isLoading())
    {
        mMainWindow->updateImage();
    }
    mUi->intrError->setText(QString("invalid"));
}

void Control::on_ty_valueChanged(double d)
{
    mMainWindow->getCalibFilter()->getTy()->setValue(d);
    if(!mMainWindow->isLoading())
    {
        mMainWindow->updateImage();
    }
    mUi->intrError->setText(QString("invalid"));
}
void Control::on_k4_valueChanged(double d)
{
    mMainWindow->getCalibFilter()->getK4()->setValue(d);
    if(!mMainWindow->isLoading())
    {
        mMainWindow->updateImage();
    }
    mUi->intrError->setText(QString("invalid"));
}
void Control::on_k5_valueChanged(double d)
{
    mMainWindow->getCalibFilter()->getK5()->setValue(d);
    if(!mMainWindow->isLoading())
    {
        mMainWindow->updateImage();
    }
    mUi->intrError->setText(QString("invalid"));
}
void Control::on_k6_valueChanged(double d)
{
    mMainWindow->getCalibFilter()->getK6()->setValue(d);
    if(!mMainWindow->isLoading())
    {
        mMainWindow->updateImage();
    }
    mUi->intrError->setText(QString("invalid"));
}
void Control::on_quadAspectRatio_stateChanged(int i)
{
    static double oldFyValue = 0;

    if(i == Qt::Checked)
    {
        oldFyValue = mUi->fy->value();
        mUi->fy->setValue(mUi->fx->value());
        mUi->fy->setDisabled(true);
    }
    else if(i == Qt::Unchecked)
    {
        mUi->fy->setEnabled(true);
        mUi->fy->setValue(oldFyValue);
    }
    mUi->intrError->setText(QString("invalid"));
}

void Control::on_fixCenter_stateChanged(int i)
{
    static double oldCxValue = 0;
    static double oldCyValue = 0;

    if(i == Qt::Checked)
    {
        oldCxValue = mUi->cx->value();
        oldCyValue = mUi->cy->value();
        if(mMainWindow->getImage())
        {
            mUi->cx->setValue(
                (mMainWindow->getImage()->width() - 1) / 2.); // mgl auch iplimg,wenn bild vergroessert wird
            mUi->cy->setValue((mMainWindow->getImage()->height() - 1) / 2.);
        }
        mUi->cx->setDisabled(true);
        mUi->cy->setDisabled(true);
    }
    else if(i == Qt::Unchecked)
    {
        mUi->cx->setEnabled(true);
        mUi->cy->setEnabled(true);
        mUi->cx->setValue(oldCxValue);
        mUi->cy->setValue(oldCyValue);
    }
    mUi->intrError->setText(QString("invalid"));
}

void Control::on_tangDist_stateChanged(int i)
{
    static double oldTxValue = 0;
    static double oldTyValue = 0;

    if(i == Qt::Checked)
    {
        mUi->tx->setEnabled(true);
        mUi->ty->setEnabled(true);
        mUi->tx->setValue(oldTxValue);
        mUi->ty->setValue(oldTyValue);
    }
    else if(i == Qt::Unchecked)
    {
        oldTxValue = mUi->tx->value();
        oldTyValue = mUi->ty->value();
        mUi->tx->setValue(0.);
        mUi->ty->setValue(0.);
        mUi->tx->setDisabled(true);
        mUi->ty->setDisabled(true);
    }
    mUi->intrError->setText(QString("invalid"));
}

void Control::on_extModelCheckBox_stateChanged(int i)
{
    if(i == Qt::Checked)
    {
        mUi->k4->setEnabled(true);
        mUi->k5->setEnabled(true);
        mUi->k6->setEnabled(true);
        mUi->s1->setEnabled(true);
        mUi->s2->setEnabled(true);
        mUi->s3->setEnabled(true);
        mUi->s4->setEnabled(true);
        mUi->taux->setEnabled(true);
        mUi->tauy->setEnabled(true);
    }
    else if(i == Qt::Unchecked)
    {
        mUi->k4->setDisabled(true);
        mUi->k5->setDisabled(true);
        mUi->k6->setDisabled(true);
        mUi->s1->setDisabled(true);
        mUi->s2->setDisabled(true);
        mUi->s3->setDisabled(true);
        mUi->s4->setDisabled(true);
        mUi->taux->setDisabled(true);
        mUi->tauy->setDisabled(true);
    }
    mUi->intrError->setText(QString("invalid"));
}

void Control::on_boardSizeX_valueChanged(int x)
{
    mMainWindow->getAutoCalib()->setBoardSizeX(x);
}

void Control::on_boardSizeY_valueChanged(int y)
{
    mMainWindow->getAutoCalib()->setBoardSizeY(y);
}

void Control::on_squareSize_valueChanged(double s)
{
    mMainWindow->getAutoCalib()->setSquareSize(s);
}

void Control::on_autoCalib_clicked()
{
    mMainWindow->getAutoCalib()->autoCalib();
}
void Control::on_calibFiles_clicked()
{
    if(mMainWindow->getAutoCalib()->openCalibFiles())
    {
        mUi->autoCalib->setEnabled(true);
    }
}

//---------------------------------------
void Control::on_rot1_valueChanged(double /*arg1*/)
{
    if(!isLoading())
    {
        mMainWindow->updateCoord();
    }
}

void Control::on_rot2_valueChanged(double /*arg1*/)
{
    if(!isLoading())
    {
        mMainWindow->updateCoord();
    }
}

void Control::on_rot3_valueChanged(double /*arg1*/)
{
    if(!isLoading())
    {
        mMainWindow->updateCoord();
    }
}

void Control::on_trans1_valueChanged(double /*arg1*/)
{
    if(!isLoading())
    {
        mMainWindow->updateCoord();
    }
}

void Control::on_trans2_valueChanged(double /*arg1*/)
{
    if(!isLoading())
    {
        mMainWindow->updateCoord();
    }
}

void Control::on_trans3_valueChanged(double /*arg1*/)
{
    if(!isLoading())
    {
        mMainWindow->updateCoord();
    }
}

void Control::on_extCalibPointsShow_stateChanged(int /*arg1*/)
{
    if(!isLoading())
    {
        mScene->update();
    }
}

void Control::on_extrCalibShowError_clicked()
{
    QString      out;
    QDialog      msgBox;
    QGridLayout *layout = new QGridLayout();
    msgBox.setLayout(layout);
    QLabel *tableView = new QLabel(&msgBox);
    layout->addWidget(tableView, 1, 1);
    QLabel *titel = new QLabel(&msgBox);
    titel->setText("<b>Reprojection error for extrinsic calibration:</b>");
    layout->addWidget(titel, 0, 1);

    if(!mMainWindow->getExtrCalibration()->getReprojectionError().isValid())
    {
        out = QString("No File for extrinsic calibration found!");
        tableView->setText(out);
    }
    else
    {
        out                    = QString("<table>"
                                         "<tr><th></th>"
                                         "<th>average   &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</th>"
                                         "<th>std. deviation                          &nbsp;</th>"
                                         "<th>variance  &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</th>"
                                         "<th>max       &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</th></tr>"
                                         "<tr><td>Point   height: &nbsp;&nbsp;            </td><td> %0 cm</td><td> %1 cm</td><td> %2 "
                                         "cm</td><td> %3 cm</td></tr>"
                                         "<tr><td>Default height: <small>[%12 cm]</small> </td><td> %4 cm</td><td> %5 cm</td><td> %6 "
                                         "cm</td><td> %7 cm</td></tr>"
                                         "<tr><td>Pixel    error: &nbsp;&nbsp;            </td><td> %8 px</td><td> %9 px</td><td> %10 "
                                         "px</td><td> %11 px</td></tr>"
                                         "</table>");
        const auto &reproError = mMainWindow->getExtrCalibration()->getReprojectionError().getData();
        for(double value : reproError)
        {
            if(value < 0)
            {
                out = out.arg("-");
            }
            else
            {
                out = out.arg(value);
            }
        }
        tableView->setText(out);
    }

    msgBox.setWindowTitle("PeTrack");
    QIcon   icon     = QApplication::style()->standardIcon(QStyle::SP_MessageBoxInformation);
    QLabel *infoIcon = new QLabel(&msgBox);
    int     iconSize = msgBox.style()->pixelMetric(QStyle::PM_MessageBoxIconSize, nullptr, &msgBox);
    infoIcon->setPixmap(icon.pixmap(iconSize, iconSize));
    layout->addWidget(infoIcon, 0, 0);
    QDialogButtonBox *ok = new QDialogButtonBox(QDialogButtonBox::Ok);
    layout->addWidget(ok, 2, 1);
    connect(ok, &QDialogButtonBox::clicked, &msgBox, &QDialog::close);
    msgBox.setFixedSize(msgBox.sizeHint());
    msgBox.exec();
}

void Control::on_extVanishPointsShow_stateChanged(int /*arg1*/)
{
    if(!isLoading())
    {
        mScene->update();
    }
}
void Control::on_coordLoad3DCalibPoints_clicked()
{
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
    QString     out_str;
    QTextStream out(&out_str);

    unsigned int i;

    out << "<table><tr><th>Nr.</th><th>3D.x</th><th>3D.y</th><th>3D.z</th><th>2D.x</th><th>2D.y</th></tr>" << Qt::endl;


    for(i = 0; i < std::max(
                       mMainWindow->getExtrCalibration()->get3DList().size(),
                       mMainWindow->getExtrCalibration()->get2DList().size());
        ++i)
    {
        out << "<tr>";
        if(i < mMainWindow->getExtrCalibration()->get3DList().size())
        {
            out << "<td>[" << QString::number(i + 1, 'i', 0) << "]: </td><td>"
                << QString::number(mMainWindow->getExtrCalibration()->get3DList().at(i).x, 'f', 1) << "</td><td>"
                << QString::number(mMainWindow->getExtrCalibration()->get3DList().at(i).y, 'f', 1) << "</td><td>"
                << QString::number(mMainWindow->getExtrCalibration()->get3DList().at(i).z, 'f', 1) << "</td><td>";
        }
        else
        {
            out << "<td>-</td><td>-</td><td>-</td>";
        }
        if(i < mMainWindow->getExtrCalibration()->get2DList().size())
        {
            out << QString::number(mMainWindow->getExtrCalibration()->get2DList().at(i).x, 'f', 3) << "</td><td>"
                << QString::number(mMainWindow->getExtrCalibration()->get2DList().at(i).y, 'f', 3) << "</td>";
        }
        else
        {
            out << "<td>-</td><td>-</td>";
        }
        out << "</tr>" << Qt::endl;
    }
    out << "</table>" << Qt::endl;

    QMessageBox msgBox;
    msgBox.setWindowTitle("PeTrack");
    msgBox.setIcon(QMessageBox::Information);
    msgBox.setText("Currently loaded point correspondences<br />for extrinsic calibration:");
    msgBox.setInformativeText(out_str);
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.setDefaultButton(QMessageBox::Ok);
    msgBox.exec();
}

//---------------------------------------
void Control::on_gridTab_currentChanged(int /*index*/)
{
    mScene->update();
}

void Control::on_gridShow_stateChanged(int /*i*/)
{
    mScene->update();
}

void Control::on_gridFix_stateChanged(int /*i*/)
{
    mScene->update();
}

void Control::on_gridRotate_valueChanged(int /*i*/)
{
    mScene->update();
}

void Control::on_gridTransX_valueChanged(int /*i*/)
{
    mScene->update();
}

void Control::on_gridTransY_valueChanged(int /*i*/)
{
    mScene->update();
}

void Control::on_gridScale_valueChanged(int /*i*/)
{
    mScene->update();
}

void Control::on_grid3DTransX_valueChanged(int /*value*/)
{
    if(!isLoading())
    {
        mScene->update();
    }
}

void Control::on_grid3DTransY_valueChanged(int /*value*/)
{
    if(!isLoading())
    {
        mScene->update();
    }
}

void Control::on_grid3DTransZ_valueChanged(int /*value*/)
{
    if(!isLoading())
    {
        mScene->update();
    }
}

void Control::on_grid3DResolution_valueChanged(int /*value*/)
{
    if(!isLoading())
    {
        mScene->update();
    }
}

//---------------------------------------
void Control::on_coordTab_currentChanged(int index)
{
    if(index == 1)
    {
        setEnabledExtrParams(false);
        mUi->trackShowGroundPosition->setEnabled(false);
        mUi->trackShowGroundPath->setEnabled(false);
    }
    else
    {
        setEnabledExtrParams(true);
        mUi->trackShowGroundPosition->setEnabled(true);
        mUi->trackShowGroundPath->setEnabled(true);
    }
    if(!isLoading())
    {
        mMainWindow->updateCoord();
        mScene->update();
    }
}

void Control::on_coordShow_stateChanged(int /*i*/)
{
    if(!isLoading())
    {
        mMainWindow->updateCoord();
        mScene->update();
        setMeasuredAltitude(); // da measured nicht aktualisiert wird, waehrend scale verschoben und show
                               // deaktiviert und beim aktivieren sonst ein falscher wert zum angezeigten koord
                               // waere
    }
    // mScene->update(); //mScene->sceneRect() // ging auch, aber dann wurde zu oft matrix berechnet etc
    // mMainWindow->getImageWidget()->update(); // repaint() zeichnet sofort - schneller aber mgl flicker
}

void Control::on_coordFix_stateChanged(int /*i*/)
{
    if(!isLoading())
    {
        mMainWindow->updateCoord();
    }
}

void Control::on_coordRotate_valueChanged(int /*i*/)
{
    if(!isLoading())
    {
        mMainWindow->updateCoord();
    }
}

void Control::on_coordTransX_valueChanged(int /*i*/)
{
    if(!isLoading())
    {
        mMainWindow->updateCoord();
    }
}

void Control::on_coordTransY_valueChanged(int /*i*/)
{
    if(!isLoading())
    {
        mMainWindow->updateCoord();
    }
}

void Control::on_coordScale_valueChanged(int /*i*/)
{
    mMainWindow->updateCoord();
    setMeasuredAltitude();
    mMainWindow->setHeadSize();
}

void Control::on_coordAltitude_valueChanged(double /*d*/)
{
    mMainWindow->setHeadSize();
    mScene->update(); // fuer kreis um kopf, der mgl der realen kopfgroesse angepasst wird
}

void Control::on_coordUnit_valueChanged(double /*d*/)
{
    setMeasuredAltitude();
    mMainWindow->setHeadSize();
    mScene->update(); // fuer kreis um kopf, der mgl der realen kopfgroesse angepasst wird
}

void Control::on_coordUseIntrinsic_stateChanged(int /*i*/)
{
    mMainWindow->setStatusPosReal();
}

void Control::on_coord3DTransX_valueChanged(int /*value*/)
{
    if(!isLoading())
    {
        mMainWindow->updateCoord();
    }
}

void Control::on_coord3DTransY_valueChanged(int /*value*/)
{
    if(!isLoading())
    {
        mMainWindow->updateCoord();
    }
}

void Control::on_coord3DTransZ_valueChanged(int /*value*/)
{
    if(!isLoading())
    {
        mMainWindow->updateCoord();
    }
}

void Control::on_coord3DAxeLen_valueChanged(int /*value*/)
{
    if(!isLoading())
    {
        mMainWindow->updateCoord();
    }
}

void Control::on_coord3DSwapX_stateChanged(int /*arg1*/)
{
    if(!isLoading())
    {
        mMainWindow->updateCoord();
    }
}

void Control::on_coord3DSwapY_stateChanged(int /*arg1*/)
{
    if(!isLoading())
    {
        mMainWindow->updateCoord();
    }
}

void Control::on_coord3DSwapZ_stateChanged(int /*arg1*/)
{
    if(!isLoading())
    {
        mMainWindow->updateCoord();
    }
}

void Control::setMeasuredAltitude()
{
    if(mMainWindow->getImageItem())
    {
        mUi->coordAltitudeMeasured->setText(
            QString("(measured: %1)")
                .arg((getCalibFx() + getCalibFy()) / 2. * mMainWindow->getImageItem()->getCmPerPixel(), 6, 'f', 1));
    }
}

//---------------------------------------
// store data in xml node
void Control::setXml(QDomElement &elem)
{
    QDomElement subElem;
    QDomElement subSubElem;
    QDomElement subSubSubElem;
    QString     fn;
    QString     heightFile;
    QString     markerFile;

    elem.setAttribute("TAB", mUi->tabs->currentIndex());

    // - - - - - - - - - - - - - - - - - - -
    subElem = (elem.ownerDocument()).createElement("CALIBRATION");
    elem.appendChild(subElem);

    subSubElem = (elem.ownerDocument()).createElement("BRIGHTNESS");
    subSubElem.setAttribute("ENABLED", mUi->filterBrightContrast->isChecked());
    subSubElem.setAttribute("VALUE", mUi->filterBrightParam->value());
    subElem.appendChild(subSubElem);

    subSubElem = (elem.ownerDocument()).createElement("CONTRAST");
    subSubElem.setAttribute("ENABLED", mUi->filterBrightContrast->isChecked());
    subSubElem.setAttribute("VALUE", mUi->filterContrastParam->value());
    subElem.appendChild(subSubElem);

    BorderFilter *bf = mMainWindow->getBorderFilter();
    QColor        col(
        (int) bf->getBorderColR()->getValue(),
        (int) bf->getBorderColG()->getValue(),
        (int) bf->getBorderColB()->getValue());
    subSubElem = (elem.ownerDocument()).createElement("BORDER");
    subSubElem.setAttribute("ENABLED", mUi->filterBorder->isChecked());
    subSubElem.setAttribute("VALUE", mUi->filterBorderParamSize->value());
    subSubElem.setAttribute("COLOR", col.name());
    subElem.appendChild(subSubElem);

    subSubElem = (elem.ownerDocument()).createElement("SWAP");
    subSubElem.setAttribute("ENABLED", mUi->filterSwap->isChecked());
    subSubElem.setAttribute("HORIZONTALLY", mUi->filterSwapH->isChecked());
    subSubElem.setAttribute("VERTICALLY", mUi->filterSwapV->isChecked());
    subElem.appendChild(subSubElem);

    subSubElem = (elem.ownerDocument()).createElement("BG_SUB");
    subSubElem.setAttribute("ENABLED", isFilterBgChecked());
    subSubElem.setAttribute("UPDATE", mUi->filterBgUpdate->isChecked());
    subSubElem.setAttribute("SHOW", mUi->filterBgShow->isChecked());
    fn = mMainWindow->getBackgroundFilter()->getFilename();
    if(fn != "")
    {
        fn = getFileList(fn, mMainWindow->getProFileName());
    }
    subSubElem.setAttribute("FILE", fn);
    subSubElem.setAttribute("DELETE", mUi->filterBgDeleteTrj->isChecked());
    subSubElem.setAttribute("DELETE_NUMBER", mUi->filterBgDeleteNumber->value());
    subElem.appendChild(subSubElem);

    subSubElem = (elem.ownerDocument()).createElement("PATTERN");
    subSubElem.setAttribute("BOARD_SIZE_X", mMainWindow->getAutoCalib()->getBoardSizeX()); // 6
    subSubElem.setAttribute("BOARD_SIZE_Y", mMainWindow->getAutoCalib()->getBoardSizeY()); // 8 oder 9
    subSubElem.setAttribute("SQUARE_SIZE", mMainWindow->getAutoCalib()->getSquareSize());  // in cm
    subElem.appendChild(subSubElem);

    subSubElem = (elem.ownerDocument()).createElement("INTRINSIC_PARAMETERS");
    subSubElem.setAttribute("ENABLED", mUi->apply->isChecked());
    subSubElem.setAttribute("FX", mUi->fx->value());
    subSubElem.setAttribute("FY", mUi->fy->value());
    subSubElem.setAttribute("CX", mUi->cx->value());
    subSubElem.setAttribute("CY", mUi->cy->value());
    subSubElem.setAttribute("R2", mUi->r2->value());
    subSubElem.setAttribute("R4", mUi->r4->value());
    subSubElem.setAttribute("R6", mUi->r6->value());
    subSubElem.setAttribute("TX", mUi->tx->value());
    subSubElem.setAttribute("TY", mUi->ty->value());
    subSubElem.setAttribute("K4", mUi->k4->value());
    subSubElem.setAttribute("K5", mUi->k5->value());
    subSubElem.setAttribute("K6", mUi->k6->value());
    subSubElem.setAttribute("S1", mUi->s1->value());
    subSubElem.setAttribute("S2", mUi->s2->value());
    subSubElem.setAttribute("S3", mUi->s3->value());
    subSubElem.setAttribute("S4", mUi->s4->value());
    subSubElem.setAttribute("TAUX", mUi->taux->value());
    subSubElem.setAttribute("TAUY", mUi->tauy->value());
    subSubElem.setAttribute("ReprError", mMainWindow->getCalibFilter()->getReprojectionError());

    subSubElem.setAttribute("QUAD_ASPECT_RATIO", mUi->quadAspectRatio->isChecked());
    subSubElem.setAttribute("FIX_CENTER", mUi->fixCenter->isChecked());
    subSubElem.setAttribute("TANG_DIST", mUi->tangDist->isChecked());
    subSubElem.setAttribute("EXT_MODEL_ENABLED", mUi->extModelCheckBox->isChecked());
    // in dateiname darf kein , vorkommen - das blank ", " zur uebersich - beim einlesen wird nur ","
    // genommen und blanks rundherum abgeschnitten, falls von hand editiert wurde
    QStringList fl = mMainWindow->getAutoCalib()->getCalibFiles();
    for(int i = 0; i < fl.size(); ++i)
    {
        if(QFileInfo(fl.at(i)).isRelative() && QFileInfo(fl.at(i)).exists())
        {
            fl.replace(i, fl.at(i) + ";" + QFileInfo(fl.at(i)).absoluteFilePath());
        }
    }
    subSubElem.setAttribute("CALIB_FILES", fl.join(", "));
    subElem.appendChild(subSubElem);


    subSubElem = (elem.ownerDocument()).createElement("EXTRINSIC_PARAMETERS");

    subSubElem.setAttribute("EXTR_ROT_1", mUi->rot1->value());
    subSubElem.setAttribute("EXTR_ROT_2", mUi->rot2->value());
    subSubElem.setAttribute("EXTR_ROT_3", mUi->rot3->value());
    subSubElem.setAttribute("EXTR_TRANS_1", mUi->trans1->value());
    subSubElem.setAttribute("EXTR_TRANS_2", mUi->trans2->value());
    subSubElem.setAttribute("EXTR_TRANS_3", mUi->trans3->value());

    subSubElem.setAttribute("SHOW_CALIB_POINTS", mUi->extCalibPointsShow->isChecked());

    QString ef = mMainWindow->getExtrCalibration()->getExtrCalibFile();
    if(ef != "")
    {
        ef = getFileList(ef, mMainWindow->getProFileName());
    }
    subSubElem.setAttribute("EXTERNAL_CALIB_FILE", ef);

    subSubElem.setAttribute("COORD_DIMENSION", mUi->coordTab->currentIndex());

    subSubElem.setAttribute("SHOW", mUi->coordShow->isChecked());
    subSubElem.setAttribute("FIX", mUi->coordFix->isChecked());
    subSubElem.setAttribute("ROTATE", mUi->coordRotate->value());
    subSubElem.setAttribute("TRANS_X", mUi->coordTransX->value());
    subSubElem.setAttribute("TRANS_Y", mUi->coordTransY->value());
    subSubElem.setAttribute("SCALE", mUi->coordScale->value());
    subSubElem.setAttribute("ALTITUDE", mUi->coordAltitude->value());
    subSubElem.setAttribute("UNIT", mUi->coordUnit->value());
    subSubElem.setAttribute("USE_INTRINSIC_CENTER", mUi->coordUseIntrinsic->isChecked());
    subSubElem.setAttribute("COORD3D_TRANS_X", mUi->coord3DTransX->value());
    subSubElem.setAttribute("COORD3D_TRANS_Y", mUi->coord3DTransY->value());
    subSubElem.setAttribute("COORD3D_TRANS_Z", mUi->coord3DTransZ->value());
    subSubElem.setAttribute("COORD3D_AXIS_LEN", mUi->coord3DAxeLen->value());
    subSubElem.setAttribute("COORD3D_SWAP_X", mUi->coord3DSwapX->isChecked());
    subSubElem.setAttribute("COORD3D_SWAP_Y", mUi->coord3DSwapY->isChecked());
    subSubElem.setAttribute("COORD3D_SWAP_Z", mUi->coord3DSwapZ->isChecked());
    subElem.appendChild(subSubElem);

    subSubElem = (elem.ownerDocument()).createElement("ALIGNMENT_GRID");
    subSubElem.setAttribute("GRID_DIMENSION", mUi->gridTab->currentIndex());
    subSubElem.setAttribute("SHOW", mUi->gridShow->isChecked());
    subSubElem.setAttribute("FIX", mUi->gridFix->isChecked());
    subSubElem.setAttribute("ROTATE", mUi->gridRotate->value());
    subSubElem.setAttribute("TRANS_X", mUi->gridTransX->value());
    subSubElem.setAttribute("TRANS_Y", mUi->gridTransY->value());
    subSubElem.setAttribute("SCALE", mUi->gridScale->value());
    subSubElem.setAttribute("GRID3D_TRANS_X", mUi->grid3DTransX->value());
    subSubElem.setAttribute("GRID3D_TRANS_Y", mUi->grid3DTransY->value());
    subSubElem.setAttribute("GRID3D_TRANS_Z", mUi->grid3DTransZ->value());
    subSubElem.setAttribute("GRID3D_RESOLUTION", mUi->grid3DResolution->value());
    subElem.appendChild(subSubElem);

    // - - - - - - - - - - - - - - - - - - -
    subElem = (elem.ownerDocument()).createElement("RECOGNITION");
    elem.appendChild(subElem);

    subSubElem = (elem.ownerDocument()).createElement("PERFORM");
    subSubElem.setAttribute("ENABLED", mUi->performRecognition->isChecked());
    subSubElem.setAttribute(
        "METHOD",
        static_cast<int>(mUi->recoMethod->itemData(mUi->recoMethod->currentIndex()).value<reco::RecognitionMethod>()));
    subSubElem.setAttribute("STEP", getRecoStep());
    subElem.appendChild(subSubElem);

    subSubElem = (elem.ownerDocument()).createElement("REGION_OF_INTEREST");
    subSubElem.setAttribute("SHOW", mUi->roiShow->isChecked());
    subSubElem.setAttribute("FIX", mUi->roiFix->isChecked());
    subSubElem.setAttribute("X", mMainWindow->getRecoRoiItem()->rect().x());
    subSubElem.setAttribute("Y", mMainWindow->getRecoRoiItem()->rect().y());
    subSubElem.setAttribute("WIDTH", mMainWindow->getRecoRoiItem()->rect().width());
    subSubElem.setAttribute("HEIGHT", mMainWindow->getRecoRoiItem()->rect().height());
    subElem.appendChild(subSubElem);

    subSubElem = (elem.ownerDocument()).createElement("MARKER");
    subSubElem.setAttribute("BRIGHTNESS", mUi->markerBrightness->value());
    subSubElem.setAttribute("IGNORE_WITHOUT", mUi->markerIgnoreWithout->isChecked());
    subElem.appendChild(subSubElem);

    subSubElem = (elem.ownerDocument()).createElement("SIZE_COLOR");
    subSubElem.setAttribute("SHOW", mUi->recoShowColor->isChecked());
    subSubElem.setAttribute("MODEL", mUi->recoColorModel->currentIndex());
    subSubElem.setAttribute("AUTO_WB", mUi->recoAutoWB->isChecked());
    subSubElem.setAttribute("X", mUi->recoColorX->currentIndex());
    subSubElem.setAttribute("Y", mUi->recoColorY->currentIndex());
    subSubElem.setAttribute("Z", mUi->recoColorZ->value());
    subSubElem.setAttribute("GREY_LEVEL", mUi->recoGreyLevel->value());
    subSubElem.setAttribute("SYMBOL_SIZE", mUi->recoSymbolSize->value());

    subSubElem.setAttribute("MAP_NUMBER", mUi->mapNr->value()); // MAP_MAX noetig ?: mapNr->maximum()+1
    for(int i = 0; i <= mUi->mapNr->maximum(); ++i)
    {
        subSubSubElem = (elem.ownerDocument()).createElement("MAP"); // QString("MAP %1").arg(i)
        RectMap map   = mUi->colorPlot->getMapItem()->getMap(i);
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
    subSubElem.setAttribute("DEFAULT_HEIGHT", mUi->mapDefaultHeight->value());

    subElem.appendChild(subSubElem);

    subSubElem = (elem.ownerDocument()).createElement("READ_HEIGHTS");

    heightFile = mMainWindow->getHeightFileName();
    if(!heightFile.isEmpty())
    {
        heightFile = getFileList(heightFile, mMainWindow->getProFileName());
    }
    subSubElem.setAttribute("HEIGHT_FILE", heightFile);
    subElem.appendChild(subSubElem);

    subSubElem = (elem.ownerDocument()).createElement("READ_MARKER_IDS");
    markerFile = mMainWindow->getMarkerIDFileName();
    if(!markerFile.isEmpty())
    {
        markerFile = getFileList(markerFile, mMainWindow->getProFileName());
    }
    subSubElem.setAttribute("MARKER_FILE", markerFile);
    subElem.appendChild(subSubElem);

    // - - - - - - - - - - - - - - - - - - -
    subElem = (elem.ownerDocument()).createElement("TRACKING");
    elem.appendChild(subElem);

    subSubElem = (elem.ownerDocument()).createElement("ONLINE_CALCULATION");
    subSubElem.setAttribute("ENABLED", mUi->trackOnlineCalc->isChecked());
    subElem.appendChild(subSubElem);

    subSubElem = (elem.ownerDocument()).createElement("REPEAT_BELOW");
    subSubElem.setAttribute("ENABLED", isTrackRepeatChecked());
    subSubElem.setAttribute("QUALITY", getTrackRepeatQual());
    subElem.appendChild(subSubElem);

    subSubElem = (elem.ownerDocument()).createElement("EXTRAPOLATION");
    subSubElem.setAttribute("ENABLED", mUi->trackExtrapolation->isChecked());
    subElem.appendChild(subSubElem);

    subSubElem = (elem.ownerDocument()).createElement("MERGE");
    subSubElem.setAttribute("ENABLED", mUi->trackMerge->isChecked());
    subElem.appendChild(subSubElem);

    subSubElem = (elem.ownerDocument()).createElement("ONLY_VISIBLE");
    subSubElem.setAttribute("ENABLED", mUi->trackOnlySelected->isChecked());
    subElem.appendChild(subSubElem);

    subSubElem = (elem.ownerDocument()).createElement("REGION_OF_INTEREST");
    subSubElem.setAttribute("SHOW", mUi->trackRoiShow->isChecked());
    subSubElem.setAttribute("FIX", mUi->trackRoiFix->isChecked());
    subSubElem.setAttribute("X", mMainWindow->getTrackRoiItem()->rect().x());
    subSubElem.setAttribute("Y", mMainWindow->getTrackRoiItem()->rect().y());
    subSubElem.setAttribute("WIDTH", mMainWindow->getTrackRoiItem()->rect().width());
    subSubElem.setAttribute("HEIGHT", mMainWindow->getTrackRoiItem()->rect().height());
    subElem.appendChild(subSubElem);

    // export options
    subSubElem = (elem.ownerDocument()).createElement("SEARCH_MISSING_FRAMES");
    subSubElem.setAttribute("ENABLED", mUi->trackMissingFrames->isChecked());
    subElem.appendChild(subSubElem);

    subSubElem = (elem.ownerDocument()).createElement("RECALCULATE_MEDIAN_HEIGHT");
    subSubElem.setAttribute("ENABLED", mUi->trackRecalcHeight->isChecked());
    subElem.appendChild(subSubElem);

    subSubElem = (elem.ownerDocument()).createElement("ALLOW_ALTERNATE_HEIGHT");
    subSubElem.setAttribute("ENABLED", mUi->trackAlternateHeight->isChecked());
    subElem.appendChild(subSubElem);

    subSubElem = (elem.ownerDocument()).createElement("EXPORT_ELIMINATE_TRACKPOINT_WITHOUT_HEIGHT");
    subSubElem.setAttribute("ENABLED", mUi->exportElimTp->isChecked());
    subElem.appendChild(subSubElem);

    subSubElem = (elem.ownerDocument()).createElement("EXPORT_ELIMINATE_TRAJECTORY_WITHOUT_HEIGHT");
    subSubElem.setAttribute("ENABLED", mUi->exportElimTrj->isChecked());
    subElem.appendChild(subSubElem);

    subSubElem = (elem.ownerDocument()).createElement("EXPORT_SMOOTH");
    subSubElem.setAttribute("ENABLED", mUi->exportSmooth->isChecked());
    subElem.appendChild(subSubElem);

    subSubElem = (elem.ownerDocument()).createElement("EXPORT_VIEWING_DIRECTION");
    subSubElem.setAttribute("ENABLED", isExportViewDirChecked());
    subElem.appendChild(subSubElem);

    subSubElem = (elem.ownerDocument()).createElement("EXPORT_ANGLE_OF_VIEW");
    subSubElem.setAttribute("ENABLED", mUi->exportAngleOfView->isChecked());
    subElem.appendChild(subSubElem);

    subSubElem = (elem.ownerDocument()).createElement("EXPORT_USE_METER");
    subSubElem.setAttribute("ENABLED", mUi->exportUseM->isChecked());
    subElem.appendChild(subSubElem);

    subSubElem = (elem.ownerDocument()).createElement("EXPORT_COMMENT");
    subSubElem.setAttribute("ENABLED", mUi->exportComment->isChecked());
    subElem.appendChild(subSubElem);

    subSubElem = (elem.ownerDocument()).createElement("EXPORT_MARKERID");
    subSubElem.setAttribute("ENABLED", mUi->exportMarkerID->isChecked());
    subElem.appendChild(subSubElem);


    subSubElem = (elem.ownerDocument()).createElement("TEST_EQUAL");
    subSubElem.setAttribute("ENABLED", mUi->testEqual->isChecked());
    subElem.appendChild(subSubElem);

    subSubElem = (elem.ownerDocument()).createElement("TEST_VELOCITY");
    subSubElem.setAttribute("ENABLED", mUi->testVelocity->isChecked());
    subElem.appendChild(subSubElem);

    subSubElem = (elem.ownerDocument()).createElement("TEST_INSIDE");
    subSubElem.setAttribute("ENABLED", mUi->testInside->isChecked());
    subElem.appendChild(subSubElem);

    subSubElem = (elem.ownerDocument()).createElement("TEST_LENGTH");
    subSubElem.setAttribute("ENABLED", mUi->testLength->isChecked());
    subElem.appendChild(subSubElem);


    subSubElem = (elem.ownerDocument()).createElement("TRACK_FILE");
    fn         = mMainWindow->getTrackFileName();
    if(fn != "")
    {
        fn = getFileList(fn, mMainWindow->getProFileName());
    }
    subSubElem.setAttribute("FILENAME", fn);
    subElem.appendChild(subSubElem);

    subSubElem = (elem.ownerDocument()).createElement("SEARCH_REGION");
    subSubElem.setAttribute("SCALE", mUi->trackRegionScale->value());
    subSubElem.setAttribute("LEVELS", getTrackRegionLevels());
    subSubElem.setAttribute("MAX_ERROR", mUi->trackErrorExponent->value());
    subSubElem.setAttribute("SHOW", mUi->trackShowSearchSize->isChecked());
    subSubElem.setAttribute("ADAPTIVE", mUi->adaptiveLevel->isChecked());
    subElem.appendChild(subSubElem);

    subSubElem = (elem.ownerDocument()).createElement("PATH");
    subSubElem.setAttribute("SHOW", mUi->trackShow->isChecked());
    subSubElem.setAttribute("FIX", mUi->trackFix->isChecked());

    subSubElem.setAttribute("ONLY_VISIBLE", mUi->trackShowOnlyVisible->isChecked());
    subSubElem.setAttribute("ONLY_PEOPLE", mUi->trackShowOnly->isChecked());
    subSubElem.setAttribute("ONLY_PEOPLE_LIST", mUi->trackShowOnlyList->isChecked());
    subSubElem.setAttribute("ONLY_PEOPLE_NR", mUi->trackShowOnlyNr->value());
    subSubElem.setAttribute("ONLY_PEOPLE_NR_LIST", mUi->trackShowOnlyNrList->text());

    subSubElem.setAttribute("SHOW_CURRENT_POINT", mUi->trackShowCurrentPoint->isChecked());
    subSubElem.setAttribute("SHOW_POINTS", mUi->trackShowPoints->isChecked());
    subSubElem.setAttribute("SHOW_PATH", mUi->trackShowPath->isChecked());
    subSubElem.setAttribute("SHOW_COLLECTIVE_COLOR", mUi->trackShowColColor->isChecked());
    subSubElem.setAttribute("SHOW_COLOR_MARKER", mUi->trackShowColorMarker->isChecked());
    subSubElem.setAttribute("SHOW_NUMBER", mUi->trackShowNumber->isChecked());
    subSubElem.setAttribute("SHOW_GROUND_POSITION", mUi->trackShowGroundPosition->isChecked());
    subSubElem.setAttribute("SHOW_GROUND_PATH", mUi->trackShowGroundPath->isChecked());

    subSubElem.setAttribute("TRACK_GROUND_PATH_COLOR", getTrackGroundPathColor().name());
    subSubElem.setAttribute("TRACK_PATH_COLOR", getTrackPathColor().name());
    subSubElem.setAttribute("CURRENT_POINT_SIZE", mUi->trackCurrentPointSize->value());
    subSubElem.setAttribute("CURRENT_POINT_LINE_WIDTH", mUi->trackCurrentPointLineWidth->value());
    subSubElem.setAttribute("POINTS_SIZE", mUi->trackPointSize->value());
    subSubElem.setAttribute("SHOW_POINTS_LINE_WIDTH", mUi->trackShowPointsLineWidth->value());
    subSubElem.setAttribute("PATH_SIZE", mUi->trackPathWidth->value());
    subSubElem.setAttribute("COLLECTIVE_COLOR_SIZE", mUi->trackColColorSize->value());
    subSubElem.setAttribute("COLOR_MARKER_SIZE", mUi->trackColorMarkerSize->value());
    subSubElem.setAttribute("COLOR_MARKER_LINE_WIDTH", mUi->trackColorMarkerLineWidth->value());
    subSubElem.setAttribute("NUMBER_SIZE", mUi->trackNumberSize->value());
    subSubElem.setAttribute("GROUND_POSITION_SIZE", mUi->trackGroundPositionSize->value());
    subSubElem.setAttribute("GROUND_PATH_SIZE", mUi->trackGroundPathSize->value());

    subSubElem.setAttribute("HEAD_SIZE", mUi->trackHeadSized->isChecked());
    subSubElem.setAttribute("POINTS_COLORED", mUi->trackShowPointsColored->isChecked());
    subSubElem.setAttribute("NUMBER_BOLD", mUi->trackNumberBold->isChecked());

    subSubElem.setAttribute("BEFORE", mUi->trackShowBefore->value());
    subSubElem.setAttribute("AFTER", mUi->trackShowAfter->value());

    subElem.appendChild(subSubElem);

    // - - - - - - - - - - - - - - - - - - -
    subElem = (elem.ownerDocument()).createElement("ANALYSIS");
    elem.appendChild(subElem);

    subSubElem = (elem.ownerDocument()).createElement("SEARCH_MISSING_FRAMES");
    subSubElem.setAttribute("ENABLED", mUi->anaMissingFrames->isChecked());
    subElem.appendChild(subSubElem);

    subSubElem = (elem.ownerDocument()).createElement("MARK_ACTUAL");
    subSubElem.setAttribute("ENABLED", mUi->anaMarkAct->isChecked());
    subElem.appendChild(subSubElem);

    subSubElem = (elem.ownerDocument()).createElement("CALCULATION");
    subSubElem.setAttribute("STEP_SIZE", mUi->anaStep->value());
    subSubElem.setAttribute("CONSIDER_X", mUi->anaConsiderX->isChecked());
    subSubElem.setAttribute("CONSIDER_Y", mUi->anaConsiderY->isChecked());
    subSubElem.setAttribute("ABSOLUTE", mUi->anaConsiderAbs->isChecked());
    subSubElem.setAttribute("REVERSE", mUi->anaConsiderRev->isChecked());
    subSubElem.setAttribute("SHOW_VORONOI", mUi->showVoronoiCells->isChecked());
    subElem.appendChild(subSubElem);
}

// read data from xml node
void Control::getXml(QDomElement &elem)
{
    QDomElement subElem, subSubElem, subSubSubElem;

    if(elem.hasAttribute("TAB"))
    {
        mUi->tabs->setCurrentIndex(elem.attribute("TAB").toInt());
    }
    for(subElem = elem.firstChildElement(); !subElem.isNull(); subElem = subElem.nextSiblingElement())
    {
        if(subElem.tagName() == "CALIBRATION")
        {
            for(subSubElem = subElem.firstChildElement(); !subSubElem.isNull();
                subSubElem = subSubElem.nextSiblingElement())
            {
                if(subSubElem.tagName() == "BRIGHTNESS")
                {
                    if(subSubElem.hasAttribute("ENABLED"))
                    {
                        mUi->filterBrightContrast->setCheckState(
                            subSubElem.attribute("ENABLED").toInt() ? Qt::Checked : Qt::Unchecked);
                    }
                    if(subSubElem.hasAttribute("VALUE"))
                    {
                        mUi->filterBrightParam->setValue(subSubElem.attribute("VALUE").toInt());
                    }
                }
                else if(subSubElem.tagName() == "CONTRAST")
                {
                    if(subSubElem.hasAttribute("ENABLED"))
                    {
                        mUi->filterBrightContrast->setCheckState(
                            subSubElem.attribute("ENABLED").toInt() ? Qt::Checked : Qt::Unchecked);
                    }
                    //,mMainWindow->getContrastFilter()->getEnabled() ? "1" : "0" //Qt::Unchecked
                    if(subSubElem.hasAttribute("VALUE"))
                    {
                        mUi->filterContrastParam->setValue(subSubElem.attribute("VALUE").toInt());
                    }
                }
                else if(subSubElem.tagName() == "BORDER")
                {
                    if(subSubElem.hasAttribute("ENABLED"))
                    {
                        mUi->filterBorder->setCheckState(
                            subSubElem.attribute("ENABLED").toInt() ? Qt::Checked : Qt::Unchecked);
                    }
                    if(subSubElem.hasAttribute("VALUE"))
                    {
                        mUi->filterBorderParamSize->setValue(subSubElem.attribute("VALUE").toInt());
                    }
                    if(subSubElem.hasAttribute("COLOR"))
                    {
                        QColor color(subSubElem.attribute("COLOR"));
                        mMainWindow->getBorderFilter()->getBorderColR()->setValue(color.red());
                        mMainWindow->getBorderFilter()->getBorderColG()->setValue(color.green());
                        mMainWindow->getBorderFilter()->getBorderColB()->setValue(color.blue());
                    }
                }
                else if(subSubElem.tagName() == "SWAP")
                {
                    if(subSubElem.hasAttribute("ENABLED"))
                    {
                        mUi->filterSwap->setCheckState(
                            subSubElem.attribute("ENABLED").toInt() ? Qt::Checked : Qt::Unchecked);
                    }
                    if(subSubElem.hasAttribute("HORIZONTALLY"))
                    {
                        mUi->filterSwapH->setCheckState(
                            subSubElem.attribute("HORIZONTALLY").toInt() ? Qt::Checked : Qt::Unchecked);
                    }
                    if(subSubElem.hasAttribute("VERTICALLY"))
                    {
                        mUi->filterSwapV->setCheckState(
                            subSubElem.attribute("VERTICALLY").toInt() ? Qt::Checked : Qt::Unchecked);
                    }
                }
                else if(subSubElem.tagName() == "BG_SUB")
                {
                    if(subSubElem.hasAttribute("ENABLED"))
                    {
                        mUi->filterBg->setCheckState(
                            subSubElem.attribute("ENABLED").toInt() ? Qt::Checked : Qt::Unchecked);
                    }
                    if(subSubElem.hasAttribute("UPDATE"))
                    {
                        mUi->filterBgUpdate->setCheckState(
                            subSubElem.attribute("UPDATE").toInt() ? Qt::Checked : Qt::Unchecked);
                    }
                    if(subSubElem.hasAttribute("SHOW"))
                    {
                        mUi->filterBgShow->setCheckState(
                            subSubElem.attribute("SHOW").toInt() ? Qt::Checked : Qt::Unchecked);
                    }
                    if(subSubElem.hasAttribute("FILE"))
                    {
                        QString f = subSubElem.attribute("FILE");
                        if(f != "")
                        {
                            if(getExistingFile(f, mMainWindow->getProFileName()) != "")
                            {
                                mMainWindow->getBackgroundFilter()->setFilename(
                                    getExistingFile(subSubElem.attribute("FILE"), mMainWindow->getProFileName()));
                            }
                            else
                            {
                                debout << "Warning: Background subtracting file not readable!" << std::endl;
                            }
                        }
                    }
                    if(subSubElem.hasAttribute("DELETE"))
                    {
                        mUi->filterBgDeleteTrj->setCheckState(
                            subSubElem.attribute("DELETE").toInt() ? Qt::Checked : Qt::Unchecked);
                    }
                    if(subSubElem.hasAttribute("DELETE_NUMBER"))
                    {
                        mUi->filterBgDeleteNumber->setValue(subSubElem.attribute("DELETE_NUMBER").toInt());
                    }
                }
                else if(subSubElem.tagName() == "PATTERN")
                {
                    if(subSubElem.hasAttribute("BOARD_SIZE_X"))
                    {
                        mUi->boardSizeX->setValue(subSubElem.attribute("BOARD_SIZE_X").toInt());
                    }
                    if(subSubElem.hasAttribute("BOARD_SIZE_Y"))
                    {
                        mUi->boardSizeY->setValue(subSubElem.attribute("BOARD_SIZE_Y").toInt());
                    }
                    if(subSubElem.hasAttribute("SQUARE_SIZE"))
                    {
                        mUi->squareSize->setValue(subSubElem.attribute("SQUARE_SIZE").toDouble());
                    }
                }
                else if(subSubElem.tagName() == "INTRINSIC_PARAMETERS")
                {
                    if(subSubElem.hasAttribute("ENABLED"))
                    {
                        mUi->apply->setCheckState(
                            subSubElem.attribute("ENABLED").toInt() ? Qt::Checked : Qt::Unchecked);
                    }
                    if(subSubElem.hasAttribute("FX"))
                    {
                        mUi->fx->setValue(subSubElem.attribute("FX").toDouble());
                    }
                    if(subSubElem.hasAttribute("FY"))
                    {
                        mUi->fy->setValue(subSubElem.attribute("FY").toDouble());
                    }
                    if(subSubElem.hasAttribute("CX"))
                    {
                        double cx_val = subSubElem.attribute("CX").toDouble();
                        if(cx_val < mUi->cx->minimum())
                        {
                            mUi->cx->setMinimum(cx_val - 50);
                        }
                        if(cx_val > mUi->cx->maximum())
                        {
                            mUi->cx->setMaximum(cx_val + 50);
                        }
                        mUi->cx->setValue(cx_val);
                    }
                    if(subSubElem.hasAttribute("CY"))
                    {
                        double cy_val = subSubElem.attribute("CY").toDouble();
                        if(cy_val < mUi->cy->minimum())
                        {
                            mUi->cy->setMinimum(cy_val - 50);
                        }
                        if(cy_val > mUi->cy->maximum())
                        {
                            mUi->cy->setMaximum(cy_val + 50);
                        }
                        mUi->cy->setValue(cy_val);
                    }
                    if(subSubElem.hasAttribute("R2"))
                    {
                        mUi->r2->setValue(subSubElem.attribute("R2").toDouble());
                    }
                    if(subSubElem.hasAttribute("R4"))
                    {
                        mUi->r4->setValue(subSubElem.attribute("R4").toDouble());
                    }
                    if(subSubElem.hasAttribute("R6"))
                    {
                        mUi->r6->setValue(subSubElem.attribute("R6").toDouble());
                    }
                    if(subSubElem.hasAttribute("TX"))
                    {
                        mUi->tx->setValue(subSubElem.attribute("TX").toDouble());
                    }
                    if(subSubElem.hasAttribute("TY"))
                    {
                        mUi->ty->setValue(subSubElem.attribute("TY").toDouble());
                    }
                    if(subSubElem.hasAttribute("K4"))
                    {
                        mUi->k4->setValue(subSubElem.attribute("K4").toDouble());
                    }
                    if(subSubElem.hasAttribute("K5"))
                    {
                        mUi->k5->setValue(subSubElem.attribute("K5").toDouble());
                    }
                    if(subSubElem.hasAttribute("K6"))
                    {
                        mUi->k6->setValue(subSubElem.attribute("K6").toDouble());
                    }
                    if(subSubElem.hasAttribute("S1"))
                    {
                        mUi->s1->setValue(subSubElem.attribute("S1").toDouble());
                    }
                    if(subSubElem.hasAttribute("S2"))
                    {
                        mUi->s2->setValue(subSubElem.attribute("S2").toDouble());
                    }
                    if(subSubElem.hasAttribute("S3"))
                    {
                        mUi->s3->setValue(subSubElem.attribute("S3").toDouble());
                    }
                    if(subSubElem.hasAttribute("S4"))
                    {
                        mUi->s4->setValue(subSubElem.attribute("S4").toDouble());
                    }
                    if(subSubElem.hasAttribute("TAUX"))
                    {
                        mUi->taux->setValue(subSubElem.attribute("TAUX").toDouble());
                    }
                    if(subSubElem.hasAttribute("TAUY"))
                    {
                        mUi->tauy->setValue(subSubElem.attribute("TAUY").toDouble());
                    }
                    if(subSubElem.hasAttribute("ReprError"))
                    {
                        mUi->intrError->setText(QString("%1").arg(subSubElem.attribute("ReprError").toDouble()));
                    }
                    if(subSubElem.hasAttribute("QUAD_ASPECT_RATIO"))
                    {
                        mUi->quadAspectRatio->setCheckState(
                            subSubElem.attribute("QUAD_ASPECT_RATIO").toInt() ? Qt::Checked : Qt::Unchecked);
                    }
                    if(subSubElem.hasAttribute("FIX_CENTER"))
                    {
                        mUi->fixCenter->setCheckState(
                            subSubElem.attribute("FIX_CENTER").toInt() ? Qt::Checked : Qt::Unchecked);
                    }
                    if(subSubElem.hasAttribute("TANG_DIST"))
                    {
                        mUi->tangDist->setCheckState(
                            subSubElem.attribute("TANG_DIST").toInt() ? Qt::Checked : Qt::Unchecked);
                    }
                    if(subSubElem.hasAttribute("EXT_MODEL_ENABLED"))
                    {
                        mUi->extModelCheckBox->setCheckState(
                            subSubElem.attribute("EXT_MODEL_ENABLED").toInt() ? Qt::Checked : Qt::Unchecked);
                    }
                    if(subSubElem.hasAttribute("CALIB_FILES"))
                    {
                        QStringList fl = (subSubElem.attribute("CALIB_FILES")).split(",");
                        QString     tmpStr;
                        for(int i = 0; i < fl.size(); ++i)
                        {
                            if((fl[i] = fl[i].trimmed()) == "")
                            {
                                fl.removeAt(i);
                            }
                            else
                            {
                                tmpStr = getExistingFile(fl[i], mMainWindow->getProFileName());
                                if(tmpStr != "")
                                {
                                    fl[i] = tmpStr;
                                }
                            }
                        }
                        // auch setzen, wenn leer, vielleicht ist das ja gewuenscht
                        mMainWindow->getAutoCalib()->setCalibFiles(fl);
                        if(!fl.isEmpty())
                        {
                            mUi->autoCalib->setEnabled(true);
                        }
                    }
                }
                else if(subSubElem.tagName() == "EXTRINSIC_PARAMETERS")
                {
                    if(subSubElem.hasAttribute("EXTR_ROT_1"))
                    {
                        mUi->rot1->setValue(subSubElem.attribute("EXTR_ROT_1").toDouble());
                    }
                    if(subSubElem.hasAttribute("EXTR_ROT_2"))
                    {
                        mUi->rot2->setValue(subSubElem.attribute("EXTR_ROT_2").toDouble());
                    }
                    if(subSubElem.hasAttribute("EXTR_ROT_3"))
                    {
                        mUi->rot3->setValue(subSubElem.attribute("EXTR_ROT_3").toDouble());
                    }
                    if(subSubElem.hasAttribute("EXTR_TRANS_1"))
                    {
                        mUi->trans1->setValue(subSubElem.attribute("EXTR_TRANS_1").toDouble());
                    }
                    if(subSubElem.hasAttribute("EXTR_TRANS_2"))
                    {
                        mUi->trans2->setValue(subSubElem.attribute("EXTR_TRANS_2").toDouble());
                    }
                    if(subSubElem.hasAttribute("EXTR_TRANS_3"))
                    {
                        mUi->trans3->setValue(subSubElem.attribute("EXTR_TRANS_3").toDouble());
                    }
                    if(subSubElem.hasAttribute("SHOW_CALIB_POINTS"))
                    {
                        mUi->extCalibPointsShow->setCheckState(
                            subSubElem.attribute("SHOW_CALIB_POINTS").toInt() ? Qt::Checked : Qt::Unchecked);
                    }

                    if(subSubElem.hasAttribute("COORD_DIMENSION"))
                    {
                        mUi->coordTab->setCurrentIndex(subSubElem.attribute("COORD_DIMENSION").toInt());
                    }
                    else
                    {
                        mUi->coordTab->setCurrentIndex(1); //  = 2D
                        setEnabledExtrParams(false);
                    }
                    if(subSubElem.hasAttribute("EXTERNAL_CALIB_FILE"))
                    {
                        if(getExistingFile(
                               QString::fromStdString(subSubElem.attribute("EXTERNAL_CALIB_FILE").toStdString()),
                               mMainWindow->getProFileName()) != "")
                        {
                            mMainWindow->getExtrCalibration()->setExtrCalibFile(getExistingFile(
                                QString::fromStdString(subSubElem.attribute("EXTERNAL_CALIB_FILE").toStdString()),
                                mMainWindow->getProFileName()));
                            mMainWindow->getExtrCalibration()->loadExtrCalibFile();
                        }
                    }

                    if(subSubElem.hasAttribute("SHOW"))
                    {
                        mUi->coordShow->setCheckState(
                            subSubElem.attribute("SHOW").toInt() ? Qt::Checked : Qt::Unchecked);
                    }
                    if(subSubElem.hasAttribute("FIX"))
                    {
                        mUi->coordFix->setCheckState(subSubElem.attribute("FIX").toInt() ? Qt::Checked : Qt::Unchecked);
                    }
                    if(subSubElem.hasAttribute("ROTATE"))
                    {
                        mUi->coordRotate->setValue(subSubElem.attribute("ROTATE").toInt());
                    }
                    if(subSubElem.hasAttribute("TRANS_X"))
                    {
                        int trans_x = subSubElem.attribute("TRANS_X").toInt();
                        if(trans_x > mUi->coordTransX->maximum())
                        {
                            setCalibCoordTransXMax(trans_x);
                        }
                        mUi->coordTransX->setValue(trans_x);
                    }
                    if(subSubElem.hasAttribute("TRANS_Y"))
                    {
                        int trans_y = subSubElem.attribute("TRANS_Y").toInt();
                        if(trans_y > mUi->coord3DTransY->maximum())
                        {
                            setCalibCoordTransYMax(trans_y);
                        }
                        mUi->coordTransY->setValue(trans_y);
                    }
                    mUi->coordTransY->setValue(subSubElem.attribute("TRANS_Y").toInt());
                    if(subSubElem.hasAttribute("SCALE"))
                    {
                        mUi->coordScale->setValue(subSubElem.attribute("SCALE").toInt());
                    }
                    if(subSubElem.hasAttribute("ALTITUDE"))
                    {
                        mUi->coordAltitude->setValue(subSubElem.attribute("ALTITUDE").toDouble());
                    }
                    if(subSubElem.hasAttribute("UNIT"))
                    {
                        mUi->coordUnit->setValue(subSubElem.attribute("UNIT").toDouble());
                    }
                    if(subSubElem.hasAttribute("USE_INTRINSIC_CENTER"))
                    {
                        mUi->coordUseIntrinsic->setCheckState(
                            subSubElem.attribute("USE_INTRINSIC_CENTER").toInt() ? Qt::Checked : Qt::Unchecked);
                    }
                    if(subSubElem.hasAttribute("COORD3D_TRANS_X"))
                    {
                        mUi->coord3DTransX->setValue(subSubElem.attribute("COORD3D_TRANS_X").toInt());
                    }
                    if(subSubElem.hasAttribute("COORD3D_TRANS_Y"))
                    {
                        mUi->coord3DTransY->setValue(subSubElem.attribute("COORD3D_TRANS_Y").toInt());
                    }
                    if(subSubElem.hasAttribute("COORD3D_TRANS_Z"))
                    {
                        mUi->coord3DTransZ->setValue(subSubElem.attribute("COORD3D_TRANS_Z").toInt());
                    }
                    if(subSubElem.hasAttribute("COORD3D_AXIS_LEN"))
                    {
                        mUi->coord3DAxeLen->setValue(subSubElem.attribute("COORD3D_AXIS_LEN").toInt());
                    }
                    if(subSubElem.hasAttribute("COORD3D_SWAP_X"))
                    {
                        mUi->coord3DSwapX->setCheckState(
                            subSubElem.attribute("COORD3D_SWAP_X").toInt() ? Qt::Checked : Qt::Unchecked);
                    }
                    if(subSubElem.hasAttribute("COORD3D_SWAP_Y"))
                    {
                        mUi->coord3DSwapY->setCheckState(
                            subSubElem.attribute("COORD3D_SWAP_Y").toInt() ? Qt::Checked : Qt::Unchecked);
                    }
                    if(subSubElem.hasAttribute("COORD3D_SWAP_Z"))
                    {
                        mUi->coord3DSwapZ->setCheckState(
                            subSubElem.attribute("COORD3D_SWAP_Z").toInt() ? Qt::Checked : Qt::Unchecked);
                    }
                }
                else if(subSubElem.tagName() == "ALIGNMENT_GRID")
                {
                    if(subSubElem.hasAttribute("GRID_DIMENSION"))
                    {
                        mUi->gridTab->setCurrentIndex(subSubElem.attribute("GRID_DIMENSION").toInt());
                    }
                    else
                    {
                        mUi->gridTab->setCurrentIndex(1); //  = 2D
                    }
                    if(subSubElem.hasAttribute("SHOW"))
                    {
                        mUi->gridShow->setCheckState(
                            subSubElem.attribute("SHOW").toInt() ? Qt::Checked : Qt::Unchecked);
                    }
                    if(subSubElem.hasAttribute("FIX"))
                    {
                        mUi->gridFix->setCheckState(subSubElem.attribute("FIX").toInt() ? Qt::Checked : Qt::Unchecked);
                    }
                    if(subSubElem.hasAttribute("ROTATE"))
                    {
                        mUi->gridRotate->setValue(subSubElem.attribute("ROTATE").toInt());
                    }
                    if(subSubElem.hasAttribute("TRANS_X"))
                    {
                        mUi->gridTransX->setValue(subSubElem.attribute("TRANS_X").toInt());
                    }
                    if(subSubElem.hasAttribute("TRANS_Y"))
                    {
                        mUi->gridTransY->setValue(subSubElem.attribute("TRANS_Y").toInt());
                    }
                    if(subSubElem.hasAttribute("SCALE"))
                    {
                        mUi->gridScale->setValue(subSubElem.attribute("SCALE").toInt());
                    }
                    if(subSubElem.hasAttribute("GRID3D_TRANS_X"))
                    {
                        mUi->grid3DTransX->setValue(subSubElem.attribute("GRID3D_TRANS_X").toInt());
                    }
                    if(subSubElem.hasAttribute("GRID3D_TRANS_Y"))
                    {
                        mUi->grid3DTransY->setValue(subSubElem.attribute("GRID3D_TRANS_Y").toInt());
                    }
                    if(subSubElem.hasAttribute("GRID3D_TRANS_Z"))
                    {
                        mUi->grid3DTransZ->setValue(subSubElem.attribute("GRID3D_TRANS_Z").toInt());
                    }
                    if(subSubElem.hasAttribute("GRID3D_RESOLUTION"))
                    {
                        mUi->grid3DResolution->setValue(subSubElem.attribute("GRID3D_RESOLUTION").toInt());
                    }
                }
                else
                {
                    debout << "Unknown CALIBRATION tag " << subSubElem.tagName() << std::endl;
                }
            }
        }
        else if(subElem.tagName() == "RECOGNITION")
        {
            for(subSubElem = subElem.firstChildElement(); !subSubElem.isNull();
                subSubElem = subSubElem.nextSiblingElement())
            {
                if(subSubElem.tagName() == "PERFORM")
                {
                    if(subSubElem.hasAttribute("ENABLED"))
                    {
                        mUi->performRecognition->setCheckState(
                            subSubElem.attribute("ENABLED").toInt() ? Qt::Checked : Qt::Unchecked);
                    }
                    if(subSubElem.hasAttribute("METHOD"))
                    {
                        auto recognitionMethod =
                            static_cast<reco::RecognitionMethod>(subSubElem.attribute("METHOD").toInt());
                        auto foundIndex = mUi->recoMethod->findData(QVariant::fromValue(recognitionMethod));
                        if(foundIndex == -1)
                        {
                            throw std::invalid_argument(
                                "Recognition Method could not be found, please check your input");
                        }
                        mUi->recoMethod->setCurrentIndex(foundIndex);
                    }
                    if(subSubElem.hasAttribute("STEP"))
                    {
                        mUi->recoStep->setValue(subSubElem.attribute("STEP").toInt());
                    }
                }
                else if(subSubElem.tagName() == "REGION_OF_INTEREST")
                {
                    double x = 0, y = 0, w = 0, h = 0;
                    if(subSubElem.hasAttribute("SHOW"))
                    {
                        mUi->roiShow->setCheckState(subSubElem.attribute("SHOW").toInt() ? Qt::Checked : Qt::Unchecked);
                    }
                    if(subSubElem.hasAttribute("FIX"))
                    {
                        mUi->roiFix->setCheckState(subSubElem.attribute("FIX").toInt() ? Qt::Checked : Qt::Unchecked);
                    }
                    if(subSubElem.hasAttribute("X"))
                    {
                        x = subSubElem.attribute("X").toDouble();
                    }
                    if(subSubElem.hasAttribute("Y"))
                    {
                        y = subSubElem.attribute("Y").toDouble();
                    }
                    if(subSubElem.hasAttribute("WIDTH"))
                    {
                        w = subSubElem.attribute("WIDTH").toDouble();
                    }
                    if(subSubElem.hasAttribute("HEIGHT"))
                    {
                        h = subSubElem.attribute("HEIGHT").toDouble();
                    }
                    mMainWindow->getRecoRoiItem()->setRect(x, y, w, h);
                }
                else if(subSubElem.tagName() == "MARKER")
                {
                    if(subSubElem.hasAttribute("BRIGHTNESS"))
                    {
                        mUi->markerBrightness->setValue(subSubElem.attribute("BRIGHTNESS").toInt());
                    }
                    if(subSubElem.hasAttribute("IGNORE_WITHOUT"))
                    {
                        mUi->markerIgnoreWithout->setCheckState(
                            subSubElem.attribute("IGNORE_WITHOUT").toInt() ? Qt::Checked : Qt::Unchecked);
                    }
                }
                else if(subSubElem.tagName() == "SIZE_COLOR")
                {
                    mColorChanging = true; // damit bei Anpassungen Farbbild nicht immer wieder neu bestimmt wird
                    if(subSubElem.hasAttribute("SHOW"))
                    {
                        mUi->recoShowColor->setCheckState(
                            subSubElem.attribute("SHOW").toInt() ? Qt::Checked : Qt::Unchecked);
                    }
                    if(subSubElem.hasAttribute("AUTO_WB"))
                    {
                        mUi->recoAutoWB->setCheckState(
                            subSubElem.attribute("AUTO_WB").toInt() ? Qt::Checked : Qt::Unchecked);
                    }
                    if(subSubElem.hasAttribute("X"))
                    {
                        mUi->recoColorX->setCurrentIndex(subSubElem.attribute("X").toInt());
                    }
                    if(subSubElem.hasAttribute("Y"))
                    {
                        mUi->recoColorY->setCurrentIndex(subSubElem.attribute("Y").toInt());
                    }
                    if(subSubElem.hasAttribute("Z"))
                    {
                        mUi->recoColorZ->setValue(subSubElem.attribute("Z").toInt());
                    }
                    if(subSubElem.hasAttribute("GREY_LEVEL"))
                    {
                        mUi->recoGreyLevel->setValue(subSubElem.attribute("GREY_LEVEL").toInt());
                    }
                    if(subSubElem.hasAttribute("SYMBOL_SIZE"))
                    {
                        mUi->recoSymbolSize->setValue(subSubElem.attribute("SYMBOL_SIZE").toInt());
                    }
                    mColorChanging = false; // MODEL setzen erzeugt Bild neu
                    if(subSubElem.hasAttribute("MODEL"))
                    {
                        // damit auch bild neu erzeugt wird, wenn sich index nicht aendert:
                        if(mUi->recoColorModel->currentIndex() == subSubElem.attribute("MODEL").toInt())
                        {
                            mIndexChanging = false;
                            on_recoColorModel_currentIndexChanged(mUi->recoColorModel->currentIndex());
                        }
                        else
                        {
                            mUi->recoColorModel->setCurrentIndex(subSubElem.attribute("MODEL").toInt());
                        }
                    }

                    double x = 0., y = 0., width = 0., height = 0., mapHeightValue = DEFAULT_HEIGHT;
                    bool   colored = true, invHue = false;
                    QColor fromCol, toCol;
                    int    h = -1, s = -1, v = -1;

                    fromCol = fromCol.toHsv();
                    toCol   = toCol.toHsv();
                    mUi->colorPlot->getMapItem()->delMaps();
                    for(subSubSubElem = subSubElem.firstChildElement(); !subSubSubElem.isNull();
                        subSubSubElem = subSubSubElem.nextSiblingElement())
                    {
                        if(subSubSubElem.tagName() == "MAP")
                        {
                            if(subSubSubElem.hasAttribute("X"))
                            {
                                x = subSubSubElem.attribute("X").toDouble();
                            }
                            if(subSubSubElem.hasAttribute("Y"))
                            {
                                y = subSubSubElem.attribute("Y").toDouble();
                            }
                            if(subSubSubElem.hasAttribute("WIDTH"))
                            {
                                width = subSubSubElem.attribute("WIDTH").toDouble();
                            }
                            if(subSubSubElem.hasAttribute("HEIGHT"))
                            {
                                height = subSubSubElem.attribute("HEIGHT").toDouble();
                            }
                            if(subSubSubElem.hasAttribute("COLORED"))
                            {
                                colored = subSubSubElem.attribute("COLORED").toInt();
                            }
                            if(subSubSubElem.hasAttribute("MAP_HEIGHT"))
                            {
                                mapHeightValue = subSubSubElem.attribute("MAP_HEIGHT").toDouble();
                            }

                            if(subSubSubElem.hasAttribute("FROM_HUE"))
                            {
                                h = subSubSubElem.attribute("FROM_HUE").toInt();
                            }
                            if(subSubSubElem.hasAttribute("FROM_SAT"))
                            {
                                s = subSubSubElem.attribute("FROM_SAT").toInt();
                            }
                            if(subSubSubElem.hasAttribute("FROM_VAL"))
                            {
                                v = subSubSubElem.attribute("FROM_VAL").toInt();
                            }
                            if(h >= 0)
                            {
                                fromCol.setHsv(h, s, v);
                            }
                            if(subSubSubElem.hasAttribute("TO_HUE"))
                            {
                                h = subSubSubElem.attribute("TO_HUE").toInt();
                            }
                            if(subSubSubElem.hasAttribute("TO_SAT"))
                            {
                                s = subSubSubElem.attribute("TO_SAT").toInt();
                            }
                            if(subSubSubElem.hasAttribute("TO_VAL"))
                            {
                                v = subSubSubElem.attribute("TO_VAL").toInt();
                            }
                            if(h >= 0)
                            {
                                toCol.setHsv(h, s, v);
                            }
                            if(subSubSubElem.hasAttribute("INV_HUE"))
                            {
                                invHue = subSubSubElem.attribute("INV_HUE").toInt();
                            }

                            mUi->colorPlot->getMapItem()->addMap(
                                x, y, width, height, colored, mapHeightValue, fromCol, toCol, invHue);
                        }

                        else
                        {
                            debout << "Unknown RECOGNITION MAP tag " << subSubElem.tagName() << std::endl;
                        }
                    }

                    mUi->mapNr->setMaximum(mUi->colorPlot->getMapItem()->mapNum() - 1);
                    if(subSubElem.hasAttribute("MAP_NUMBER")) // hiermit werden aus map-datenstruktur richtige map
                                                              // angezeigt, daher am ende
                    {
                        mUi->mapNr->setValue(subSubElem.attribute("MAP_NUMBER").toInt());
                        on_mapNr_valueChanged(
                            subSubElem.attribute("MAP_NUMBER").toInt()); // nochmal explizit aufrufen, falls 0, dann
                                                                         // wuerde valueChanged nicht on_... durchlaufen
                    }
                    if(subSubElem.hasAttribute("DEFAULT_HEIGHT"))
                    {
                        mUi->mapDefaultHeight->setValue(subSubElem.attribute("DEFAULT_HEIGHT").toDouble());
                    }
                }

                else if(subSubElem.tagName() == "READ_HEIGHTS")
                {
                    if(subSubElem.hasAttribute("HEIGHT_FILE"))
                    {
                        QString heightFileName = (subSubElem.attribute("HEIGHT_FILE"));
                        if(!getExistingFile(heightFileName, mMainWindow->getProFileName()).isEmpty())
                        {
                            mMainWindow->setHeightFileName(
                                getExistingFile(heightFileName, mMainWindow->getProFileName()));
                        }
                        else
                        {
                            mMainWindow->setHeightFileName(heightFileName);
                        }
                    }
                }

                else if(subSubElem.tagName() == "READ_MARKER_IDS")
                {
                    if(subSubElem.hasAttribute("MARKER_FILE"))
                    {
                        QString fm = subSubElem.attribute("MARKER_FILE");
                        if(getExistingFile(fm, mMainWindow->getProFileName()) != "")
                        {
                            mMainWindow->setMarkerIDFileName(getExistingFile(fm, mMainWindow->getProFileName()));
                        }
                        else
                        {
                            mMainWindow->setMarkerIDFileName(fm);
                        }
                    }
                }

                else
                {
                    debout << "Unknown RECOGNITION tag " << subSubElem.tagName() << std::endl;
                }
            }
        }
        else if(subElem.tagName() == "TRACKING")
        {
            for(subSubElem = subElem.firstChildElement(); !subSubElem.isNull();
                subSubElem = subSubElem.nextSiblingElement())
            {
                if(subSubElem.tagName() == "ONLINE_CALCULATION")
                {
                    if(subSubElem.hasAttribute("ENABLED"))
                    {
                        mUi->trackOnlineCalc->setCheckState(
                            subSubElem.attribute("ENABLED").toInt() ? Qt::Checked : Qt::Unchecked);
                    }
                }
                else if(subSubElem.tagName() == "REPEAT_BELOW")
                {
                    if(subSubElem.hasAttribute("ENABLED"))
                    {
                        mUi->trackRepeat->setCheckState(
                            subSubElem.attribute("ENABLED").toInt() ? Qt::Checked : Qt::Unchecked);
                    }
                    if(subSubElem.hasAttribute("QUALITY"))
                    {
                        mUi->trackRepeatQual->setValue(subSubElem.attribute("QUALITY").toInt());
                    }
                }
                else if(subSubElem.tagName() == "EXTRAPOLATION")
                {
                    if(subSubElem.hasAttribute("ENABLED"))
                    {
                        mUi->trackExtrapolation->setCheckState(
                            subSubElem.attribute("ENABLED").toInt() ? Qt::Checked : Qt::Unchecked);
                    }
                }
                else if(subSubElem.tagName() == "MERGE")
                {
                    if(subSubElem.hasAttribute("ENABLED"))
                    {
                        mUi->trackMerge->setCheckState(
                            subSubElem.attribute("ENABLED").toInt() ? Qt::Checked : Qt::Unchecked);
                    }
                }
                else if(subSubElem.tagName() == "ONLY_VISIBLE")
                {
                    if(subSubElem.hasAttribute("ENABLED"))
                    {
                        mUi->trackOnlySelected->setCheckState(
                            subSubElem.attribute("ENABLED").toInt() ? Qt::Checked : Qt::Unchecked);
                    }
                }
                else if(subSubElem.tagName() == "SEARCH_MISSING_FRAMES")
                {
                    if(subSubElem.hasAttribute("ENABLED"))
                    {
                        mUi->trackMissingFrames->setCheckState(
                            subSubElem.attribute("ENABLED").toInt() ? Qt::Checked : Qt::Unchecked);
                    }
                }
                else if(subSubElem.tagName() == "RECALCULATE_MEDIAN_HEIGHT")
                {
                    if(subSubElem.hasAttribute("ENABLED"))
                    {
                        mUi->trackRecalcHeight->setCheckState(
                            subSubElem.attribute("ENABLED").toInt() ? Qt::Checked : Qt::Unchecked);
                    }
                }
                else if(subSubElem.tagName() == "REGION_OF_INTEREST")
                {
                    double x = 0, y = 0, w = 0, h = 0;
                    if(subSubElem.hasAttribute("SHOW"))
                    {
                        mUi->trackRoiShow->setCheckState(
                            subSubElem.attribute("SHOW").toInt() ? Qt::Checked : Qt::Unchecked);
                    }
                    if(subSubElem.hasAttribute("FIX"))
                    {
                        mUi->trackRoiFix->setCheckState(
                            subSubElem.attribute("FIX").toInt() ? Qt::Checked : Qt::Unchecked);
                    }
                    if(subSubElem.hasAttribute("X"))
                    {
                        x = subSubElem.attribute("X").toDouble();
                    }
                    if(subSubElem.hasAttribute("Y"))
                    {
                        y = subSubElem.attribute("Y").toDouble();
                    }
                    if(subSubElem.hasAttribute("WIDTH"))
                    {
                        w = subSubElem.attribute("WIDTH").toDouble();
                    }
                    if(subSubElem.hasAttribute("HEIGHT"))
                    {
                        h = subSubElem.attribute("HEIGHT").toDouble();
                    }
                    mMainWindow->getTrackRoiItem()->setRect(x, y, w, h);
                }
                else if(subSubElem.tagName() == "ALLOW_ALTERNATE_HEIGHT")
                {
                    if(subSubElem.hasAttribute("ENABLED"))
                    {
                        mUi->trackAlternateHeight->setCheckState(
                            subSubElem.attribute("ENABLED").toInt() ? Qt::Checked : Qt::Unchecked);
                    }
                }
                else if(subSubElem.tagName() == "EXPORT_ELIMINATE_TRACKPOINT_WITHOUT_HEIGHT")
                {
                    if(subSubElem.hasAttribute("ENABLED"))
                    {
                        mUi->exportElimTp->setCheckState(
                            subSubElem.attribute("ENABLED").toInt() ? Qt::Checked : Qt::Unchecked);
                    }
                }
                else if(subSubElem.tagName() == "EXPORT_ELIMINATE_TRAJECTORY_WITHOUT_HEIGHT")
                {
                    if(subSubElem.hasAttribute("ENABLED"))
                    {
                        mUi->exportElimTrj->setCheckState(
                            subSubElem.attribute("ENABLED").toInt() ? Qt::Checked : Qt::Unchecked);
                    }
                }
                else if(subSubElem.tagName() == "EXPORT_SMOOTH")
                {
                    if(subSubElem.hasAttribute("ENABLED"))
                    {
                        mUi->exportSmooth->setCheckState(
                            subSubElem.attribute("ENABLED").toInt() ? Qt::Checked : Qt::Unchecked);
                    }
                }
                else if(subSubElem.tagName() == "EXPORT_VIEWING_DIRECTION")
                {
                    if(subSubElem.hasAttribute("ENABLED"))
                    {
                        mUi->exportViewDir->setCheckState(
                            subSubElem.attribute("ENABLED").toInt() ? Qt::Checked : Qt::Unchecked);
                    }
                }
                else if(subSubElem.tagName() == "EXPORT_ANGLE_OF_VIEW")
                {
                    if(subSubElem.hasAttribute("ENABLED"))
                    {
                        mUi->exportAngleOfView->setCheckState(
                            subSubElem.attribute("ENABLED").toInt() ? Qt::Checked : Qt::Unchecked);
                    }
                }
                else if(subSubElem.tagName() == "EXPORT_USE_METER")
                {
                    if(subSubElem.hasAttribute("ENABLED"))
                    {
                        mUi->exportUseM->setCheckState(
                            subSubElem.attribute("ENABLED").toInt() ? Qt::Checked : Qt::Unchecked);
                    }
                }
                else if(subSubElem.tagName() == "EXPORT_COMMENT")
                {
                    if(subSubElem.hasAttribute("ENABLED"))
                    {
                        mUi->exportComment->setCheckState(
                            subSubElem.attribute("ENABLED").toInt() ? Qt::Checked : Qt::Unchecked);
                    }
                }
                else if(subSubElem.tagName() == "EXPORT_MARKERID")
                {
                    if(subSubElem.hasAttribute("ENABLED"))
                    {
                        mUi->exportMarkerID->setCheckState(
                            subSubElem.attribute("ENABLED").toInt() ? Qt::Checked : Qt::Unchecked);
                    }
                }
                else if(subSubElem.tagName() == "TEST_EQUAL")
                {
                    if(subSubElem.hasAttribute("ENABLED"))
                    {
                        mUi->testEqual->setCheckState(
                            subSubElem.attribute("ENABLED").toInt() ? Qt::Checked : Qt::Unchecked);
                    }
                }
                else if(subSubElem.tagName() == "TEST_VELOCITY")
                {
                    if(subSubElem.hasAttribute("ENABLED"))
                    {
                        mUi->testVelocity->setCheckState(
                            subSubElem.attribute("ENABLED").toInt() ? Qt::Checked : Qt::Unchecked);
                    }
                }
                else if(subSubElem.tagName() == "TEST_INSIDE")
                {
                    if(subSubElem.hasAttribute("ENABLED"))
                    {
                        mUi->testInside->setCheckState(
                            subSubElem.attribute("ENABLED").toInt() ? Qt::Checked : Qt::Unchecked);
                    }
                }
                else if(subSubElem.tagName() == "TEST_LENGTH")
                {
                    if(subSubElem.hasAttribute("ENABLED"))
                    {
                        mUi->testLength->setCheckState(
                            subSubElem.attribute("ENABLED").toInt() ? Qt::Checked : Qt::Unchecked);
                    }
                }
                else if(subSubElem.tagName() == "TRACK_FILE")
                {
                    if(subSubElem.hasAttribute("FILENAME"))
                    {
                        if(getExistingFile(subSubElem.attribute("FILENAME"), mMainWindow->getProFileName()) != "")
                        {
                            mMainWindow->setTrackFileName(
                                getExistingFile(subSubElem.attribute("FILENAME"), mMainWindow->getProFileName()));
                        }
                        else // eigentlich nicht lesbar, aber so wird wenigstens beim projekt speichern wieder mit
                             // weggeschrieben
                        {
                            mMainWindow->setTrackFileName(subSubElem.attribute("FILENAME"));
                        }
                    }
                }
                else if(subSubElem.tagName() == "SEARCH_REGION")
                {
                    if(subSubElem.hasAttribute("SCALE"))
                    {
                        mUi->trackRegionScale->setValue(subSubElem.attribute("SCALE").toInt());
                    }
                    if(subSubElem.hasAttribute("LEVELS"))
                    {
                        mUi->trackRegionLevels->setValue(subSubElem.attribute("LEVELS").toInt());
                    }
                    if(subSubElem.hasAttribute("MAX_ERROR"))
                    {
                        mUi->trackErrorExponent->setValue(subSubElem.attribute("MAX_ERROR").toInt());
                    }
                    if(subSubElem.hasAttribute("SHOW"))
                    {
                        mUi->trackShowSearchSize->setCheckState(
                            subSubElem.attribute("SHOW").toInt() ? Qt::Checked : Qt::Unchecked);
                    }
                    if(subSubElem.hasAttribute("ADAPTIVE"))
                    {
                        mUi->adaptiveLevel->setCheckState(
                            subSubElem.attribute("ADAPTIVE").toInt() ? Qt::Checked : Qt::Unchecked);
                    }
                }
                else if(subSubElem.tagName() == "PATH")
                {
                    if(subSubElem.hasAttribute("SHOW"))
                    {
                        mUi->trackShow->setCheckState(
                            subSubElem.attribute("SHOW").toInt() ? Qt::Checked : Qt::Unchecked);
                    }
                    if(subSubElem.hasAttribute("FIX"))
                    {
                        mUi->trackFix->setCheckState(subSubElem.attribute("FIX").toInt() ? Qt::Checked : Qt::Unchecked);
                    }

                    if(subSubElem.hasAttribute("ONLY_VISIBLE"))
                    {
                        mUi->trackShowOnlyVisible->setCheckState(
                            subSubElem.attribute("ONLY_VISIBLE").toInt() ? Qt::Checked : Qt::Unchecked);
                    }
                    if(subSubElem.hasAttribute("ONLY_PEOPLE"))
                    {
                        mUi->trackShowOnly->setCheckState(
                            subSubElem.attribute("ONLY_PEOPLE").toInt() ? Qt::Checked : Qt::Unchecked);
                    }

                    if(subSubElem.hasAttribute("ONLY_PEOPLE_LIST"))
                    {
                        mUi->trackShowOnlyList->setCheckState(
                            subSubElem.attribute("ONLY_PEOPLE_LIST").toInt() ? Qt::Checked : Qt::Unchecked);
                    }

                    // IMPORTANT: reading ONLY_PEOPLE_NR is done in petrack.cpp, as the trajectories need to be
                    // loaded before!
                    if(subSubElem.hasAttribute("SHOW_CURRENT_POINT"))
                    {
                        mUi->trackShowCurrentPoint->setCheckState(
                            subSubElem.attribute("SHOW_CURRENT_POINT").toInt() ? Qt::Checked : Qt::Unchecked);
                    }
                    if(subSubElem.hasAttribute("SHOW_POINTS"))
                    {
                        mUi->trackShowPoints->setCheckState(
                            subSubElem.attribute("SHOW_POINTS").toInt() ? Qt::Checked : Qt::Unchecked);
                    }
                    if(subSubElem.hasAttribute("SHOW_PATH"))
                    {
                        mUi->trackShowPath->setCheckState(
                            subSubElem.attribute("SHOW_PATH").toInt() ? Qt::Checked : Qt::Unchecked);
                    }
                    if(subSubElem.hasAttribute("SHOW_COLLECTIVE_COLOR"))
                    {
                        mUi->trackShowColColor->setCheckState(
                            subSubElem.attribute("SHOW_COLLECTIVE_COLOR").toInt() ? Qt::Checked : Qt::Unchecked);
                    }
                    if(subSubElem.hasAttribute("SHOW_COLOR_MARKER"))
                    {
                        mUi->trackShowColorMarker->setCheckState(
                            subSubElem.attribute("SHOW_COLOR_MARKER").toInt() ? Qt::Checked : Qt::Unchecked);
                    }
                    if(subSubElem.hasAttribute("SHOW_NUMBER"))
                    {
                        mUi->trackShowNumber->setCheckState(
                            subSubElem.attribute("SHOW_NUMBER").toInt() ? Qt::Checked : Qt::Unchecked);
                    }
                    if(subSubElem.hasAttribute("SHOW_GROUND_POSITION"))
                    {
                        mUi->trackShowGroundPosition->setCheckState(
                            subSubElem.attribute("SHOW_GROUND_POSITION").toInt() ? Qt::Checked : Qt::Unchecked);
                    }
                    if(subSubElem.hasAttribute("SHOW_GROUND_PATH"))
                    {
                        mUi->trackShowGroundPath->setCheckState(
                            subSubElem.attribute("SHOW_GROUND_PATH").toInt() ? Qt::Checked : Qt::Unchecked);
                    }
                    if(subSubElem.hasAttribute("TRACK_PATH_COLOR"))
                    {
                        QColor color(subSubElem.attribute("TRACK_PATH_COLOR"));
                        setTrackPathColor(color);
                    }
                    if(subSubElem.hasAttribute("TRACK_GROUND_PATH_COLOR"))
                    {
                        QColor color(subSubElem.attribute("TRACK_GROUND_PATH_COLOR"));
                        setTrackGroundPathColor(color);
                    }
                    if(subSubElem.hasAttribute("HEAD_SIZE"))
                    {
                        mUi->trackHeadSized->setCheckState(
                            subSubElem.attribute("HEAD_SIZE").toInt() ? Qt::Checked : Qt::Unchecked);
                    }
                    if(subSubElem.hasAttribute("CURRENT_POINT_SIZE"))
                    {
                        mUi->trackCurrentPointSize->setValue(subSubElem.attribute("CURRENT_POINT_SIZE").toInt());
                    }
                    if(subSubElem.hasAttribute("CURRENT_POINT_LINE_WIDTH"))
                    {
                        mUi->trackCurrentPointLineWidth->setValue(
                            subSubElem.attribute("CURRENT_POINT_LINE_WIDTH").toInt());
                    }
                    if(subSubElem.hasAttribute("POINTS_SIZE"))
                    {
                        mUi->trackPointSize->setValue(subSubElem.attribute("POINTS_SIZE").toInt());
                    }
                    if(subSubElem.hasAttribute("SHOW_POINTS_LINE_WIDTH"))
                    {
                        mUi->trackShowPointsLineWidth->setValue(subSubElem.attribute("SHOW_POINTS_LINE_WIDTH").toInt());
                    }
                    if(subSubElem.hasAttribute("PATH_SIZE"))
                    {
                        mUi->trackPathWidth->setValue(subSubElem.attribute("PATH_SIZE").toInt());
                    }
                    if(subSubElem.hasAttribute("COLLECTIVE_COLOR_SIZE"))
                    {
                        mUi->trackColColorSize->setValue(subSubElem.attribute("COLLECTIVE_COLOR_SIZE").toInt());
                    }
                    if(subSubElem.hasAttribute("COLOR_MARKER_SIZE"))
                    {
                        mUi->trackColorMarkerSize->setValue(subSubElem.attribute("COLOR_MARKER_SIZE").toInt());
                    }
                    if(subSubElem.hasAttribute("COLOR_MARKER_LINE_WIDTH"))
                    {
                        mUi->trackColorMarkerLineWidth->setValue(
                            subSubElem.attribute("COLOR_MARKER_LINE_WIDTH").toInt());
                    }
                    if(subSubElem.hasAttribute("NUMBER_SIZE"))
                    {
                        mUi->trackNumberSize->setValue(subSubElem.attribute("NUMBER_SIZE").toInt());
                    }
                    if(subSubElem.hasAttribute("GROUND_POSITION_SIZE"))
                    {
                        mUi->trackGroundPositionSize->setValue(subSubElem.attribute("GROUND_POSITION_SIZE").toInt());
                    }
                    if(subSubElem.hasAttribute("GROUND_PATH_SIZE"))
                    {
                        mUi->trackGroundPathSize->setValue(subSubElem.attribute("GROUND_PATH_SIZE").toInt());
                    }
                    if(subSubElem.hasAttribute("POINTS_COLORED"))
                    {
                        mUi->trackShowPointsColored->setCheckState(
                            subSubElem.attribute("POINTS_COLORED").toInt() ? Qt::Checked : Qt::Unchecked);
                    }
                    if(subSubElem.hasAttribute("NUMBER_BOLD"))
                    {
                        mUi->trackNumberBold->setCheckState(
                            subSubElem.attribute("NUMBER_BOLD").toInt() ? Qt::Checked : Qt::Unchecked);
                    }
                    if(subSubElem.hasAttribute("BEFORE"))
                    {
                        mUi->trackShowBefore->setValue(subSubElem.attribute("BEFORE").toInt());
                    }
                    if(subSubElem.hasAttribute("AFTER"))
                    {
                        mUi->trackShowAfter->setValue(subSubElem.attribute("AFTER").toInt());
                    }
                }
                else
                {
                    debout << "Unknown TRACKING tag " << subSubElem.tagName() << std::endl;
                }
            }
        }
        else if(subElem.tagName() == "ANALYSIS")
        {
            for(subSubElem = subElem.firstChildElement(); !subSubElem.isNull();
                subSubElem = subSubElem.nextSiblingElement())
            {
                if(subSubElem.tagName() == "SEARCH_MISSING_FRAMES")
                {
                    if(subSubElem.hasAttribute("ENABLED"))
                    {
                        mUi->anaMissingFrames->setCheckState(
                            subSubElem.attribute("ENABLED").toInt() ? Qt::Checked : Qt::Unchecked);
                    }
                }
                else if(subSubElem.tagName() == "MARK_ACTUAL")
                {
                    if(subSubElem.hasAttribute("ENABLED"))
                    {
                        mUi->anaMarkAct->setCheckState(
                            subSubElem.attribute("ENABLED").toInt() ? Qt::Checked : Qt::Unchecked);
                    }
                }
                else if(subSubElem.tagName() == "CALCULATION")
                {
                    if(subSubElem.hasAttribute("STEP_SIZE"))
                    {
                        mUi->anaStep->setValue(subSubElem.attribute("STEP_SIZE").toInt());
                    }
                    if(subSubElem.hasAttribute("CONSIDER_X"))
                    {
                        mUi->anaConsiderX->setCheckState(
                            subSubElem.attribute("CONSIDER_X").toInt() ? Qt::Checked : Qt::Unchecked);
                    }
                    if(subSubElem.hasAttribute("CONSIDER_Y"))
                    {
                        mUi->anaConsiderY->setCheckState(
                            subSubElem.attribute("CONSIDER_Y").toInt() ? Qt::Checked : Qt::Unchecked);
                    }
                    if(subSubElem.hasAttribute("ABSOLUTE"))
                    {
                        mUi->anaConsiderAbs->setCheckState(
                            subSubElem.attribute("ABSOLUTE").toInt() ? Qt::Checked : Qt::Unchecked);
                    }
                    if(subSubElem.hasAttribute("REVERSE"))
                    {
                        mUi->anaConsiderRev->setCheckState(
                            subSubElem.attribute("REVERSE").toInt() ? Qt::Checked : Qt::Unchecked);
                    }
                    if(subSubElem.hasAttribute("SHOW_VORONOI"))
                    {
                        mUi->showVoronoiCells->setCheckState(
                            subSubElem.attribute("SHOW_VORONOI").toInt() ? Qt::Checked : Qt::Unchecked);
                    }
                }
                else
                {
                    debout << "Unknown ANAYSIS tag " << subSubElem.tagName() << std::endl;
                }
            }
        }
        else
        {
            debout << "Unknown CONTROL tag " << subElem.tagName() << std::endl;
        }
    }

    mMainWindow->updateCoord();
}

ColorPlot *Control::getColorPlot() const
{
    return mUi->colorPlot;
}

#ifdef QWT
AnalysePlot *Control::getAnalysePlot() const
{
    return mUi->analysePlot;
}
#endif

bool Control::isAnaConsiderXChecked() const
{
    return mUi->anaConsiderX->isChecked();
}

bool Control::isAnaConsiderYChecked() const
{
    return mUi->anaConsiderY->isChecked();
}

bool Control::isAnaConsiderAbsChecked() const
{
    return mUi->anaConsiderAbs->isChecked();
}

bool Control::isAnaConsiderRevChecked() const
{
    return mUi->anaConsiderRev->isChecked();
}

bool Control::isAnaMarkActChecked() const
{
    return mUi->anaMarkAct->isChecked();
}

int Control::getAnaStep() const
{
    return mUi->anaStep->value();
}

reco::RecognitionMethod Control::getRecoMethod() const
{
    auto method = mUi->recoMethod->itemData(mUi->recoMethod->currentIndex());
    return method.value<reco::RecognitionMethod>();
}

void Control::on_colorPickerButton_clicked(bool checked)
{
    // true wenn neu gechecked, false wenn wieder abgewählt
    GraphicsView *view = mMainWindow->getView();
    if(checked)
    {
        connect(view, &GraphicsView::colorSelected, this, &Control::on_expandColor);
        connect(view, &GraphicsView::setColorEvent, this, &Control::on_setColor);
    }
    else
    {
        disconnect(view, &GraphicsView::colorSelected, this, &Control::on_expandColor);
        disconnect(view, &GraphicsView::setColorEvent, this, &Control::on_setColor);
    }
}

/**
 * @brief Gets necessary colors and the RectPlotItem
 *
 * @param[out] clickedColor color which was clicked on, with Hue from 0-360 instead of OpenCVs 0-180
 * @param[out] toColor toColor as in model with triangle color picker
 * @param[out] fromColor fromColor as in model with triangle picker
 * @param[out] map
 *
 * @return Boolean describing, if a color was retrieved
 */
bool Control::getColors(QColor &clickedColor, QColor &toColor, QColor &fromColor, RectPlotItem *&map)
{
    if(mMainWindow->getImg().empty())
    {
        return false;
    }

    QImage *hsvImg   = mMainWindow->getImage();
    QPointF imgPoint = mMainWindow->getMousePosOnImage();

    if(imgPoint.x() < 0 || imgPoint.x() > hsvImg->width() || imgPoint.y() < 0 || imgPoint.y() > hsvImg->height())
    {
        debout << "Clicked outside the image with color picker." << std::endl;
        return false;
    }

    clickedColor = QColor{hsvImg->pixel(imgPoint.toPoint())};

    map       = this->getColorPlot()->getMapItem();
    toColor   = map->getActMapToColor();
    fromColor = map->getActMapFromColor();
    if(!toColor.isValid() || !fromColor.isValid())
    {
        debout << "Map is corrupted" << std::endl;
        return false;
    }
    return true;
}

void Control::replotColorplot()
{
    mUi->colorPlot->replot();
}

bool Control::isTestEqualChecked() const
{
    return mUi->testEqual->isChecked();
}

bool Control::isTestVelocityChecked() const
{
    return mUi->testVelocity->isChecked();
}

bool Control::isTestInsideChecked() const
{
    return mUi->testInside->isChecked();
}

bool Control::isTestLengthChecked() const
{
    return mUi->testLength->isChecked();
}

void Control::setMapX(int val)
{
    mUi->mapX->setValue(val);
}

int Control::getMapX() const
{
    return mUi->mapX->value();
}

void Control::setMapY(int val)
{
    mUi->mapY->setValue(val);
}

int Control::getMapY() const
{
    return mUi->mapY->value();
}

void Control::setMapW(int val)
{
    mUi->mapW->setValue(val);
}

int Control::getMapW() const
{
    return mUi->mapW->value();
}

void Control::setMapH(int val)
{
    mUi->mapH->setValue(val);
}

int Control::getMapH() const
{
    return mUi->mapH->value();
}

int Control::getRecoColorX() const
{
    return mUi->recoColorX->currentIndex();
}

int Control::getRecoColorY() const
{
    return mUi->recoColorY->currentIndex();
}

int Control::getRecoColorZ() const
{
    return mUi->recoColorZ->value();
}

int Control::getRecoColorModel() const
{
    return mUi->recoColorModel->currentIndex();
}

int Control::getMapNr() const
{
    return mUi->mapNr->value();
}

int Control::getMarkerBrightness() const
{
    return mUi->markerBrightness->value();
}

bool Control::isMarkerIgnoreWithoutChecked() const
{
    return mUi->markerIgnoreWithout->isChecked();
}

bool Control::isRecoAutoWBChecked() const
{
    return mUi->recoAutoWB->isChecked();
}

Qt::CheckState Control::getAnaMissingFrames() const
{
    return mUi->anaMissingFrames->checkState();
}

Qt::CheckState Control::getTrackAlternateHeight() const
{
    return mUi->trackAlternateHeight->checkState();
}

bool Control::isExportElimTpChecked() const
{
    return mUi->exportElimTp->isChecked();
}

bool Control::isExportElimTrjChecked() const
{
    return mUi->exportElimTrj->isChecked();
}

bool Control::isExportSmoothChecked() const
{
    return mUi->exportSmooth->isChecked();
}

bool Control::isExportViewDirChecked() const
{
    return mUi->exportViewDir->isChecked();
}

bool Control::isExportAngleOfViewChecked() const
{
    return mUi->exportAngleOfView->isChecked();
}

bool Control::isExportMarkerIDChecked() const
{
    return mUi->exportMarkerID->isChecked();
}

bool Control::isTrackRecalcHeightChecked() const
{
    return mUi->trackRecalcHeight->isChecked();
}

bool Control::isTrackMissingFramesChecked() const
{
    return mUi->trackMissingFrames->checkState();
}

bool Control::isExportUseMeterChecked() const
{
    return mUi->exportUseM->isChecked();
}

bool Control::isExportCommentChecked() const
{
    return mUi->exportComment->isChecked();
}

double Control::getDefaultHeight() const
{
    return mUi->mapDefaultHeight->value();
}

double Control::getCameraAltitude() const
{
    return mUi->coordAltitude->value();
}

/**
 * @brief Expands the range of detcted colors to include the color clicked on
 *
 * Expands the range of detected colors, so that hue, saturation and value of the
 * color clicked on are within the range. A Buffer is used as well (more is added to the
 * range than strictly necessary).  It is possible that the inverse hue flag is set.
 *
 * @param p
 * @param graphicsView
 */
void Control::on_expandColor()
{
    QColor        fromColor, toColor;
    RectPlotItem *map;
    QColor        clickedColor;
    if(!getColors(clickedColor, toColor, fromColor, map))
    {
        return;
    }

    expandRange(fromColor, toColor, clickedColor);

    saveChange(fromColor, toColor, map);
}

/**
 * @brief Selects one color as starting point for furher additions
 *
 * @param p
 * @param graphicsView
 */
void Control::on_setColor()
{
    constexpr int BUFFER = 5;

    QColor        fromColor, toColor;
    RectPlotItem *map;
    QColor        clickedColor;
    if(!getColors(clickedColor, toColor, fromColor, map))
    {
        return;
    }

    int minHue, maxHue;
    if((clickedColor.hue() + BUFFER) % 359 < clickedColor.hue() + BUFFER)
    {
        maxHue = (clickedColor.hue() + BUFFER) % BUFFER;
        map->changeActMapInvHue(true);
    }
    else
    {
        maxHue = clickedColor.hue() + BUFFER;
        map->changeActMapInvHue(false);
    }
    if((clickedColor.hue() - BUFFER) < 0)
    {
        minHue = 360 + (clickedColor.hue() - BUFFER);
        map->changeActMapInvHue(true);
    }
    else
    {
        minHue = clickedColor.hue() - BUFFER;
    }
    toColor.setHsv(
        maxHue, std::min((clickedColor.saturation() + BUFFER), 255), std::min(clickedColor.value() + BUFFER, 255));
    fromColor.setHsv(
        minHue, std::max(clickedColor.saturation() - BUFFER, 0), std::max(clickedColor.value() - BUFFER, 0));

    saveChange(fromColor, toColor, map);
}

// NOTE Use duplicate observed data on color Data
//      Meaning: Own class for the Data and every View works with his class!!!
// NOTE that color slides are not actually working properly on their own; use this or colorRangeWidget!
void Control::saveChange(const QColor &fromColor, const QColor &toColor, RectPlotItem *map)
{
    map->changeActMapToColor(toColor);
    map->changeActMapFromColor(fromColor);

    mMainWindow->getColorRangeWidget()->setControlWidget(
        toColor.hue(), fromColor.hue(), toColor.saturation(), fromColor.saturation());
    mMainWindow->setRecognitionChanged(true); // flag indicates that changes of recognition parameters happens
    if(!mMainWindow->isLoading())
    {
        mMainWindow->updateImage();
    }
}

/**
 * @brief Expands the color range of the map to include clickedColor
 *
 * @test Change invHue via Extension
 * @test Hue same, value diff
 * @test value same, hue diff etc...
 * @test neues Ding gerade so noch drinnen (Auch von thresholding seite messen)
 *
 * @param fromColor[in,out] fromColor of the map, gets changed!
 * @param toColor[in,out] toColor of the map, gets changed!
 * @param clickedColor[in]
 */
void Control::expandRange(QColor &fromColor, QColor &toColor, const QColor &clickedColor) const
{
    // NOTE BUFFER in Klassenebene verschieben und bei setColor auch nutzen? (verschiedene Größe vllt. gewünscht?)
    constexpr int BUFFER = 10;

    std::array<int, 3> clickedColorArr;
    clickedColor.getHsv(&clickedColorArr[0], &clickedColorArr[1], &clickedColorArr[2]);
    std::array<int, 3> toColorArr;
    toColor.getHsv(&toColorArr[0], &toColorArr[1], &toColorArr[2]);
    std::array<int, 3> fromColorArr;
    fromColor.getHsv(&fromColorArr[0], &fromColorArr[1], &fromColorArr[2]);


    std::array<bool, 3> isInRange{true, true, true};
    bool                invHue = getColorPlot()->getMapItem()->getActMapInvHue();

    // What values do need to be altered?
    if(invHue)
    {
        if(toColor.hue() > fromColor.hue())
        {
            if(!(clickedColor.hue() >= toColor.hue() || clickedColor.hue() <= fromColor.hue()))
            {
                isInRange[0] = false;
            }
        }
        else
        {
            if(!(clickedColor.hue() <= toColor.hue() || clickedColor.hue() >= fromColor.hue()))
            {
                isInRange[0] = false;
            }
        }
    }
    else
    {
        if(toColor.hue() > fromColor.hue())
        {
            if(!(clickedColor.hue() <= toColor.hue() && clickedColor.hue() >= fromColor.hue()))
            {
                isInRange[0] = false;
            }
        }
        else
        {
            if(!(clickedColor.hue() >= toColor.hue() && clickedColor.hue() <= fromColor.hue()))
            {
                isInRange[0] = false;
            }
        }
    }

    for(int i = 1; i < 3; ++i)
    {
        if(toColorArr[i] > fromColorArr[i])
        {
            if(!(clickedColorArr[i] <= toColorArr[i] && clickedColorArr[i] >= fromColorArr[i]))
            {
                isInRange[i] = false;
            }
        }
        else
        {
            if(!(clickedColorArr[i] >= toColorArr[i] && clickedColorArr[i] <= fromColorArr[i]))
            {
                isInRange[i] = false;
            }
        }
    }

    // if all in range, no expanding required
    if(std::count(isInRange.cbegin(), isInRange.cend(), true) == 3)
    {
        return;
    }

    int distToColor   = 0;
    int distFromColor = 0;

    for(int i = 0; i < 3; ++i)
    {
        if(isInRange[i])
        {
            continue;
        }

        distToColor   = abs(toColorArr[i] - clickedColorArr[i]);
        distFromColor = abs(fromColorArr[i] - clickedColorArr[i]);
        if(distFromColor < distToColor)
        {
            int buffer = fromColorArr[i] - clickedColorArr[i] < 0 ? BUFFER : -BUFFER;
            if(i == 0) // Hue
            {
                fromColorArr[i] = std::min(359, std::max(0, clickedColorArr[i] + buffer));
            }
            else
            {
                fromColorArr[i] = std::min(255, std::max(0, clickedColorArr[i] + buffer));
            }
        }
        else
        {
            int buffer = toColorArr[i] - clickedColorArr[i] < 0 ? BUFFER : -BUFFER;
            if(i == 0) // Hue
            {
                toColorArr[i] = std::min(359, std::max(0, clickedColorArr[i] + buffer));
            }
            else
            {
                toColorArr[i] = std::min(255, std::max(0, clickedColorArr[i] + buffer));
            }
        }
    }

    int toHue   = toColorArr[0];
    int fromHue = fromColorArr[0];
    if(invHue)
    {
        if(toHue + 360 - fromHue > 180)
        {
            getColorPlot()->getMapItem()->changeActMapInvHue(false);
        }
    }
    else
    {
        if(toHue - fromHue > 180)
        {
            getColorPlot()->getMapItem()->changeActMapInvHue(true);
        }
    }

    toColor.setHsv(toColorArr[0], toColorArr[1], toColorArr[2]);
    fromColor.setHsv(fromColorArr[0], fromColorArr[1], fromColorArr[2]);
}

void Control::toggleRecoROIButtons()
{
    bool enabled = (!mUi->roiFix->isChecked()) && mUi->roiShow->isChecked();
    mUi->recoRoiAdjustAutomatically->setEnabled(enabled);
    mUi->recoRoiToFullImageSize->setEnabled(enabled);
}

void Control::toggleTrackROIButtons()
{
    bool enabled = (!mUi->trackRoiFix->isChecked()) && mUi->trackRoiShow->isChecked();
    mUi->trackRoiAdjustAutomatically->setEnabled(enabled);
    mUi->trackRoiToFullImageSize->setEnabled(enabled);
}

#include "moc_control.cpp"
