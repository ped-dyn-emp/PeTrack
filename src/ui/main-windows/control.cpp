/*
 * PeTrack - Software for tracking pedestrians movement in videos
 * Copyright (C) 2024 Forschungszentrum Jülich GmbH, IAS-7
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
#include "alignmentGridBox.h"
#include "analysePlot.h"
#include "calibFilter.h"
#include "codeMarkerWidget.h"
#include "colorMarkerWidget.h"
#include "colorPlot.h"
#include "colorRangeWidget.h"
#include "coordinateSystemBox.h"
#include "correction.h"
#include "extrinsicBox.h"
#include "extrinsicParameters.h"
#include "filterBeforeBox.h"
#include "importHelper.h"
#include "intrinsicBox.h"
#include "logger.h"
#include "multiColorMarkerWidget.h"
#include "pGroupBox.h"
#include "pMessageBox.h"
#include "petrack.h"
#include "player.h"
#include "roiItem.h"
#include "stereoWidget.h"
#include "tracker.h"
#include "trackerItem.h"
#include "ui_control.h"
#include "view.h"
#include "worldImageCorrespondence.h"

#include <QDomElement>
#include <iomanip>
#define DEFAULT_HEIGHT 180.0

Control::Control(
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
    AlignmentGridBox    *gridBox) :
    Control(
        parent,
        scene,
        recognizer,
        trackRoiItem,
        recoRoiItem,
        new Ui::Control(),
        missingFrames,
        filterBefore,
        intrinsicBox,
        extrinsicBox,
        coordSysBox,
        gridBox)
{
}

Control::Control(
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
    AlignmentGridBox    *gridBox) :
    QWidget(&parent), mUi(ui), mFilterBefore(filterBefore)
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

    // layout reparents widget
    auto *filterBeforePBox = new PGroupBox(this, "filter before", mFilterBefore);
    ui->verticalLayout_13->insertWidget(0, filterBeforePBox);
    mFilterBefore->setObjectName("filterBeforeBox");

    FilterSettings filterSettings;
    filterSettings.useBrightContrast = mMainWindow->getBrightContrastFilter()->getEnabled();
    filterSettings.useBorder         = mMainWindow->getBorderFilter()->getEnabled();
    filterSettings.useBackground     = mMainWindow->getBackgroundFilter()->getEnabled();

    filterSettings.useSwap  = mMainWindow->getSwapFilter()->getEnabled();
    filterSettings.useSwapH = mMainWindow->getSwapFilter()->getSwapHorizontally().getValue();
    filterSettings.useSwapV = mMainWindow->getSwapFilter()->getSwapVertically().getValue();
    mFilterBefore->setFilterSettings(filterSettings);

    mIntr = intrinsicBox;
    mIntr->setObjectName(QString::fromUtf8("intr"));
    auto *intrPBox = new PGroupBox(this, "intrinsic parameters", mIntr);
    ui->verticalLayout_13->insertWidget(1, intrPBox);

    mExtr = extrinsicBox;
    mExtr->setObjectName(QString::fromUtf8("extr"));
    auto *extrPBox = new PGroupBox(this, "extrinsic parameters", mExtr);
    ui->verticalLayout_13->insertWidget(2, extrPBox);

    mCoordSys = coordSysBox;
    coordSysBox->setObjectName(QString::fromUtf8("coord"));
    auto *coordSysPBox = new PGroupBox(this, "coordinate system", mCoordSys);
    ui->verticalLayout_13->insertWidget(3, coordSysPBox);

    mGrid          = gridBox;
    auto *gridPBox = new PGroupBox(this, "alignment grid", mGrid);
    ui->verticalLayout_13->insertWidget(4, gridPBox);

    // integrate new widgets in tabbing order
    QWidget::setTabOrder(mFilterBefore, mIntr);
    QWidget::setTabOrder(mIntr, mExtr);
    QWidget::setTabOrder(mExtr, mCoordSys);
    QWidget::setTabOrder(mCoordSys, mGrid);

    connect(mExtr, &ExtrinsicBox::extrinsicChanged, mCoordSys, &CoordinateSystemBox::updateCoordItem);
    connect(mIntr, &IntrinsicBox::paramsChanged, this, &Control::on_intrinsicParamsChanged);
    connect(
        mExtr,
        &ExtrinsicBox::enabledChanged,
        this,
        [this](bool enabled)
        {
            mUi->trackShowGroundPosition->setEnabled(enabled);
            mUi->trackShowGroundPath->setEnabled(enabled);
        });
    connect(
        mGrid,
        &AlignmentGridBox::gridChanged,
        this,
        [this]
        {
            if(!mMainWindow->isLoading())
            {
                mScene->update();
            }
        });

    setFilterBorderSizeMin(mMainWindow->getBorderFilter()->getBorderSize().getMinimum());
    setFilterBorderSizeMax(mMainWindow->getBorderFilter()->getBorderSize().getMaximum());

    // will be done by designer: mUi->colorPlot->setParent(colorBox); //because it is just integrated via frame in
    // designer

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

    mCorrectionWidget = new Correction(mMainWindow, mMainWindow->getPersonStorage(), this);
    mUi->tabs->insertTab(3, mCorrectionWidget, "correction");

    connect(
        mUi->trackShowComplPath,
        &QCheckBox::stateChanged,
        this,
        [this](int)
        {
            if(!isLoading())
            {
                mScene->update();
            }
        });
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

void Control::toggleShowOnly()
{
    static bool nrLastToggled = true;
    if(mUi->trackShowOnly->isChecked())
    {
        nrLastToggled = true;
        mUi->trackShowOnly->toggle();
        return;
    }
    if(mUi->trackShowOnlyList->isChecked())
    {
        nrLastToggled = false;
        mUi->trackShowOnlyList->toggle();
        return;
    }

    if(nrLastToggled)
    {
        mUi->trackShowOnly->toggle();
    }
    else
    {
        mUi->trackShowOnlyList->toggle();
    }
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

bool Control::isTrackShowComplPathChecked() const
{
    return mUi->trackShowComplPath->isChecked();
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

void Control::toggleRecognition()
{
    mUi->performRecognition->toggle();
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
    return mFilterBefore->getFilterBorderSize();
}
void Control::setFilterBorderSizeMin(int i)
{
    mFilterBefore->setFilterBorderSizeMin(i);
}
void Control::setFilterBorderSizeMax(int i)
{
    mFilterBefore->setFilterBorderSizeMax(i);
}

bool Control::isFilterBgChecked() const
{
    return mFilterBefore->isFilterBgChecked();
}

bool Control::isFilterBgDeleteTrjChecked() const
{
    return mFilterBefore->isFilterBgDeleteTrjChecked();
}

int Control::getFilterBgDeleteNumber() const
{
    return mFilterBefore->getFilterBgDeleteNumber();
}

void Control::imageSizeChanged(int width, int height, int borderDiff)
{
    mIntr->imageSizeChanged(width, height, borderDiff);
    // trans nicht setzen, da mgl mehrere videos mit gleicher scene und gleichem koord sinnvoll
    const Vec2F min(-10 * mMainWindow->getImageBorderSize(), -10 * mMainWindow->getImageBorderSize());
    const Vec2F max(
        10 * (width - mMainWindow->getImageBorderSize()), 10 * (height - mMainWindow->getImageBorderSize()));
    setCalibCoord2DTransMinMax(min, max);
    mMainWindow->updateSceneRect();
}

const ExtrinsicParameters &Control::getExtrinsicParameters() const
{
    return mExtr->getExtrinsicParameters();
}

void Control::loadExtrinsicCalibFile()
{
    mExtr->loadCalibFile();
}

const WorldImageCorrespondence &Control::getWorldImageCorrespondence() const
{
    return *mCoordSys;
}

void Control::setCalibCoord2DTransMinMax(Vec2F min, Vec2F max)
{
    mCoordSys->setCoordTrans2DMinMax(min, max);
}

Vec2F Control::getCalibCoord2DTrans()
{
    return mCoordSys->getCoordTrans2D();
}

Vec3F Control::getCalibCoord3DTrans() const
{
    return mCoordSys->getCoordTrans3D();
}

SwapAxis Control::getCalibCoord3DSwap() const
{
    return mCoordSys->getSwap3D();
}


int Control::getCalibCoordDimension()
{
    return mCoordSys->getCalibCoordDimension();
}


bool Control::getCalibCoordShow()
{
    return mCoordSys->getCalibCoordShow();
}

int Control::getCalibCoordScale()
{
    return mCoordSys->getCoordPose2D().scale;
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
        // Interval detection requires an unchecked checkbox following up the checked ones.
        // If the checkbox for the last ID is checked, no unchecked checkbox follows.
        // This edgecase is identified by 'first' not being reset to -1
        if(first != -1)
        {
            last = static_cast<int>(mMainWindow->getPersonStorage().nbPersons());
            if(first == last)
            {
                list.append(QString::number(first));
            }
            else
            {
                list.append(QString("%1-%2").arg(first).arg(last));
            }
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


void Control::on_intrinsicParamsChanged(IntrinsicCameraParams params)
{
    mMainWindow->getCalibFilter()->getCamParams().setValue(params);
    mMainWindow->setStatusPosReal();
    if(!mMainWindow->isLoading())
    {
        mMainWindow->updateImage();
        mCoordSys->updateCoordItem();
    }
    mCoordSys->setMeasuredAltitude();
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

    BorderFilter *bf = mMainWindow->getBorderFilter();
    QColor        col(bf->getBorderColR().getValue(), bf->getBorderColG().getValue(), bf->getBorderColB().getValue());
    fn = mMainWindow->getBackgroundFilter()->getFilename();
    if(fn != "")
    {
        fn = getFileList(fn, mMainWindow->getProFileName());
    }
    mFilterBefore->setXml(subElem, col, fn);

    // PATTERN and INTRINSIC_PARAMETERS elements
    mIntr->setXml(subElem);

    subSubElem = (subElem.ownerDocument()).createElement("EXTRINSIC_PARAMETERS");
    mExtr->setXml(subSubElem);


    QString ef = mMainWindow->getExtrCalibration()->getExtrCalibFile();
    if(ef != "")
    {
        ef = getFileList(ef, mMainWindow->getProFileName());
    }
    subSubElem.setAttribute("EXTERNAL_CALIB_FILE", ef);

    mCoordSys->setXml(subSubElem);

    subElem.appendChild(subSubElem);

    subSubElem = (elem.ownerDocument()).createElement("ALIGNMENT_GRID");
    mGrid->setXml(subSubElem);
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
    subSubElem.setAttribute("SHOW_COMPLETE", mUi->trackShowComplPath->isChecked());

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

    auto correctionElem = (elem.ownerDocument()).createElement("CORRECTION");
    mCorrectionWidget->setXml(correctionElem);
    elem.appendChild(correctionElem);
}

// read data from xml node
void Control::getXml(const QDomElement &elem, const QString &version)
{
    QDomElement subElem, subSubElem, subSubSubElem;

    loadActiveIndex(elem, "TAB", mUi->tabs, 0);

    for(subElem = elem.firstChildElement(); !subElem.isNull(); subElem = subElem.nextSiblingElement())
    {
        if(subElem.tagName() == "CALIBRATION")
        {
            for(subSubElem = subElem.firstChildElement(); !subSubElem.isNull();
                subSubElem = subSubElem.nextSiblingElement())
            {
                if(mFilterBefore->getXmlSub(subSubElem))
                {
                    // intentionally left blank
                }
                else if(mIntr->getXml(subSubElem))
                {
                    // intentionally left blank
                }
                else if(mExtr->getXml(subSubElem))
                {
                    // intentionally left blank
                }
                else if(mCoordSys->getXml(subSubElem))
                {
                    // intentionally left blank
                }
                else if(mGrid->getXml(subSubElem))
                {
                    // intentionally left blank
                }
                else if(subSubElem.tagName() == "BORDER")
                {
                    QColor color(readQString(subSubElem, "COLOR"));
                    mMainWindow->getBorderFilter()->getBorderColR().setValue(color.red());
                    mMainWindow->getBorderFilter()->getBorderColG().setValue(color.green());
                    mMainWindow->getBorderFilter()->getBorderColB().setValue(color.blue());
                }
                else if(subSubElem.tagName() == "BG_SUB")
                {
                    QString f = readQString(subSubElem, "FILE", "");
                    if(f != "")
                    {
                        if(getExistingFile(f, mMainWindow->getProFileName()) != "")
                        {
                            mMainWindow->getBackgroundFilter()->setFilename(
                                getExistingFile(f, mMainWindow->getProFileName()));
                        }
                        else
                        {
                            SPDLOG_WARN("Background subtracting file not readable!");
                        }
                    }
                }
                else
                {
                    SPDLOG_WARN("Unknown CALIBRATION tag: {}", subSubElem.tagName());
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
                    loadBoolValue(subSubElem, "ENABLED", mUi->performRecognition);

                    auto recognitionMethod = static_cast<reco::RecognitionMethod>(
                        readInt(subSubElem, "METHOD", static_cast<int>(reco::RecognitionMethod::MultiColor)));
                    auto foundIndex = mUi->recoMethod->findData(QVariant::fromValue(recognitionMethod));
                    if(foundIndex == -1)
                    {
                        throw std::invalid_argument("Recognition Method could not be found, please check your input");
                    }
                    mUi->recoMethod->setCurrentIndex(foundIndex);
                    if(newerThanVersion("0.8.6", version))
                    {
                        // old default was Hermes-Marker
                        auto foundIndex =
                            mUi->recoMethod->findData(QVariant::fromValue(reco::RecognitionMethod::Hermes));
                        if(foundIndex == -1)
                        {
                            throw std::invalid_argument(
                                "Recognition Method could not be found, please check your input");
                        }
                        mUi->recoMethod->setCurrentIndex(foundIndex);
                    }
                    loadIntValue(subSubElem, "STEP", mUi->recoStep);
                }
                else if(subSubElem.tagName() == "REGION_OF_INTEREST")
                {
                    double x = 0, y = 0, w = 0, h = 0;
                    loadBoolValue(subSubElem, "SHOW", mUi->roiShow, false);
                    loadBoolValue(subSubElem, "FIX", mUi->roiFix, false);

                    x = readDouble(subSubElem, "X");
                    y = readDouble(subSubElem, "Y");
                    w = readDouble(subSubElem, "WIDTH");
                    h = readDouble(subSubElem, "HEIGHT");

                    mMainWindow->getRecoRoiItem()->setRect(x, y, w, h);
                }
                else if(subSubElem.tagName() == "MARKER")
                {
                    loadIntValue(subSubElem, "BRIGHTNESS", mUi->markerBrightness);
                    loadBoolValue(subSubElem, "IGNORE_WITHOUT", mUi->markerIgnoreWithout);
                }
                else if(subSubElem.tagName() == "SIZE_COLOR")
                {
                    mColorChanging = true; // damit bei Anpassungen Farbbild nicht immer wieder neu bestimmt wird
                    loadBoolValue(subSubElem, "SHOW", mUi->recoShowColor);
                    loadBoolValue(subSubElem, "AUTO_WB", mUi->recoAutoWB);
                    loadActiveIndex(subSubElem, "X", mUi->recoColorX);
                    loadActiveIndex(subSubElem, "Y", mUi->recoColorY);
                    loadIntValue(subSubElem, "Z", mUi->recoColorZ);
                    loadIntValue(subSubElem, "GREY_LEVEL", mUi->recoGreyLevel);
                    loadIntValue(subSubElem, "SYMBOL_SIZE", mUi->recoSymbolSize);

                    mColorChanging = false;
                    // MODEL setzen erzeugt Bild neu
                    // damit auch bild neu erzeugt wird, wenn sich index nicht aendert:
                    if(mUi->recoColorModel->currentIndex() == readInt(subSubElem, "MODEL"))
                    {
                        mIndexChanging = false;
                        on_recoColorModel_currentIndexChanged(mUi->recoColorModel->currentIndex());
                    }
                    else
                    {
                        mUi->recoColorModel->setCurrentIndex(readInt(subSubElem, "MODEL"));
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
                            x              = readDouble(subSubSubElem, "X");
                            y              = readDouble(subSubSubElem, "Y");
                            width          = readDouble(subSubSubElem, "WIDTH");
                            height         = readDouble(subSubSubElem, "HEIGHT");
                            colored        = readBool(subSubSubElem, "COLORED");
                            mapHeightValue = readDouble(subSubSubElem, "MAP_HEIGHT");
                            h              = readInt(subSubSubElem, "FROM_HUE", 0);
                            s              = readInt(subSubSubElem, "FROM_SAT", 0);
                            v              = readInt(subSubSubElem, "FROM_VAL", 128);

                            if(h >= 0)
                            {
                                fromCol.setHsv(h, s, v);
                            }

                            h = readInt(subSubSubElem, "TO_HUE", 359);
                            s = readInt(subSubSubElem, "TO_SAT", 255);
                            v = readInt(subSubSubElem, "TO_VAL", 255);

                            if(h >= 0)
                            {
                                toCol.setHsv(h, s, v);
                            }
                            invHue = readBool(subSubSubElem, "INV_HUE", false);

                            mUi->colorPlot->getMapItem()->addMap(
                                x, y, width, height, colored, mapHeightValue, fromCol, toCol, invHue);
                        }

                        else
                        {
                            SPDLOG_WARN("Unknown RECOGNITION tag: {}", subSubElem.tagName());
                        }
                    }

                    mUi->mapNr->setMaximum(mUi->colorPlot->getMapItem()->mapNum() - 1);
                    loadIntValue(
                        subSubElem,
                        "MAP_NUMBER",
                        mUi->mapNr); // here conversion from map data structure to real map, therefore at the end
                    on_mapNr_valueChanged(readInt(
                        subSubElem,
                        "MAP_NUMBER")); // explicitely called if 0, otherwise the signal wouldn't fire
                    loadDoubleValue(subSubElem, "DEFAULT_HEIGHT", mUi->mapDefaultHeight);
                }
                else if(subSubElem.tagName() == "READ_HEIGHTS")
                {
                    QString heightFileName = readQString(subSubElem, "HEIGHT_FILE");
                    if(!getExistingFile(heightFileName, mMainWindow->getProFileName()).isEmpty())
                    {
                        mMainWindow->setHeightFileName(getExistingFile(heightFileName, mMainWindow->getProFileName()));
                    }
                    else
                    {
                        mMainWindow->setHeightFileName(heightFileName);
                    }
                }

                else if(subSubElem.tagName() == "READ_MARKER_IDS")
                {
                    QString fm = readQString(subSubElem, "MARKER_FILE");
                    if(getExistingFile(fm, mMainWindow->getProFileName()) != "")
                    {
                        mMainWindow->setMarkerIDFileName(getExistingFile(fm, mMainWindow->getProFileName()));
                    }
                    else
                    {
                        mMainWindow->setMarkerIDFileName(fm);
                    }
                }

                else
                {
                    SPDLOG_WARN("Unknown RECOGNITION tag: {}", subSubElem.tagName());
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
                    loadBoolValue(subSubElem, "ENABLED", mUi->trackOnlineCalc);
                }
                else if(subSubElem.tagName() == "REPEAT_BELOW")
                {
                    loadBoolValue(subSubElem, "ENABLED", mUi->trackRepeat);
                    loadIntValue(subSubElem, "QUALITY", mUi->trackRepeatQual);
                }
                else if(subSubElem.tagName() == "EXTRAPOLATION")
                {
                    loadBoolValue(subSubElem, "ENABLED", mUi->trackExtrapolation);
                }
                else if(subSubElem.tagName() == "MERGE")
                {
                    loadBoolValue(subSubElem, "ENABLED", mUi->trackMerge);
                }
                else if(subSubElem.tagName() == "ONLY_VISIBLE")
                {
                    loadBoolValue(subSubElem, "ENABLED", mUi->trackOnlySelected);
                }
                else if(subSubElem.tagName() == "SEARCH_MISSING_FRAMES")
                {
                    loadBoolValue(subSubElem, "ENABLED", mUi->trackMissingFrames);
                }
                else if(subSubElem.tagName() == "RECALCULATE_MEDIAN_HEIGHT")
                {
                    loadBoolValue(subSubElem, "ENABLED", mUi->trackRecalcHeight);
                }
                else if(subSubElem.tagName() == "REGION_OF_INTEREST")
                {
                    double x = 0, y = 0, w = 0, h = 0;

                    loadBoolValue(subSubElem, "SHOW", mUi->trackRoiShow, false);
                    loadBoolValue(subSubElem, "FIX", mUi->trackRoiFix, false);

                    x = readDouble(subSubElem, "X");
                    y = readDouble(subSubElem, "Y");
                    w = readDouble(subSubElem, "WIDTH");
                    h = readDouble(subSubElem, "HEIGHT");

                    mMainWindow->getTrackRoiItem()->setRect(x, y, w, h);
                }
                else if(subSubElem.tagName() == "ALLOW_ALTERNATE_HEIGHT")
                {
                    loadBoolValue(subSubElem, "ENABLED", mUi->trackAlternateHeight);
                }
                else if(subSubElem.tagName() == "EXPORT_ELIMINATE_TRACKPOINT_WITHOUT_HEIGHT")
                {
                    loadBoolValue(subSubElem, "ENABLED", mUi->exportElimTp);
                }
                else if(subSubElem.tagName() == "EXPORT_ELIMINATE_TRAJECTORY_WITHOUT_HEIGHT")
                {
                    loadBoolValue(subSubElem, "ENABLED", mUi->exportElimTrj);
                }
                else if(subSubElem.tagName() == "EXPORT_SMOOTH")
                {
                    loadBoolValue(subSubElem, "ENABLED", mUi->exportSmooth);
                }
                else if(subSubElem.tagName() == "EXPORT_VIEWING_DIRECTION")
                {
                    loadBoolValue(subSubElem, "ENABLED", mUi->exportViewDir);
                }
                else if(subSubElem.tagName() == "EXPORT_ANGLE_OF_VIEW")
                {
                    loadBoolValue(subSubElem, "ENABLED", mUi->exportAngleOfView);
                }
                else if(subSubElem.tagName() == "EXPORT_USE_METER")
                {
                    loadBoolValue(subSubElem, "ENABLED", mUi->exportUseM);
                }
                else if(subSubElem.tagName() == "EXPORT_COMMENT")
                {
                    loadBoolValue(subSubElem, "ENABLED", mUi->exportComment);
                }
                else if(subSubElem.tagName() == "EXPORT_MARKERID")
                {
                    loadBoolValue(subSubElem, "ENABLED", mUi->exportMarkerID);
                }
                else if((subSubElem.tagName() == "TEST_EQUAL") && (!newerThanVersion(version, QString("0.10.0"))))
                {
                    mCorrectionWidget->setTestEqualChecked(readBool(subSubElem, "ENABLED"));
                }
                else if((subSubElem.tagName() == "TEST_VELOCITY") && (!newerThanVersion(version, QString("0.10.0"))))
                {
                    mCorrectionWidget->setTestVelocityChecked(readBool(subSubElem, "ENABLED"));
                }
                else if((subSubElem.tagName() == "TEST_INSIDE") && (!newerThanVersion(version, QString("0.10.0"))))
                {
                    mCorrectionWidget->setTestInsideChecked(readBool(subSubElem, "ENABLED"));
                }
                else if((subSubElem.tagName() == "TEST_LENGTH") && (!newerThanVersion(version, QString("0.10.0"))))
                {
                    mCorrectionWidget->setTestLengthChecked(readBool(subSubElem, "ENABLED"));
                }
                else if(subSubElem.tagName() == "TRACK_FILE")
                {
                    if(getExistingFile(readQString(subSubElem, "FILENAME"), mMainWindow->getProFileName()) != "")
                    {
                        mMainWindow->setTrackFileName(
                            getExistingFile(readQString(subSubElem, "FILENAME"), mMainWindow->getProFileName()));
                    }
                    else // eigentlich nicht lesbar, aber so wird wenigstens beim projekt speichern wieder mit
                         // weggeschrieben
                    {
                        mMainWindow->setTrackFileName(readQString(subSubElem, "FILENAME"));
                    }
                }
                else if(subSubElem.tagName() == "SEARCH_REGION")
                {
                    loadIntValue(subSubElem, "SCALE", mUi->trackRegionScale);
                    loadIntValue(subSubElem, "LEVELS", mUi->trackRegionLevels);
                    loadIntValue(subSubElem, "MAX_ERROR", mUi->trackErrorExponent, 0);
                    loadBoolValue(subSubElem, "SHOW", mUi->trackShowSearchSize, false);
                    loadBoolValue(subSubElem, "ADAPTIVE", mUi->adaptiveLevel, false);
                }
                else if(subSubElem.tagName() == "PATH")
                {
                    loadBoolValue(subSubElem, "SHOW_COMPLETE", mUi->trackShowComplPath, false);
                    loadBoolValue(subSubElem, "SHOW", mUi->trackShow, true);
                    loadBoolValue(subSubElem, "FIX", mUi->trackFix, false);
                    loadBoolValue(subSubElem, "ONLY_VISIBLE", mUi->trackShowOnlyVisible, false);
                    loadBoolValue(subSubElem, "ONLY_PEOPLE", mUi->trackShowOnly, false);
                    loadBoolValue(subSubElem, "ONLY_PEOPLE_LIST", mUi->trackShowOnlyList, false);
                    // IMPORTANT: reading ONLY_PEOPLE_NR is done in petrack.cpp, as the trajectories need to be
                    // loaded before!
                    loadBoolValue(subSubElem, "SHOW_CURRENT_POINT", mUi->trackShowCurrentPoint, true);
                    loadBoolValue(subSubElem, "SHOW_POINTS", mUi->trackShowPoints, false);
                    loadBoolValue(subSubElem, "SHOW_PATH", mUi->trackShowPath, true);
                    loadBoolValue(subSubElem, "SHOW_COLLECTIVE_COLOR", mUi->trackShowColColor, true);
                    loadBoolValue(subSubElem, "SHOW_COLOR_MARKER", mUi->trackShowColorMarker, true);
                    loadBoolValue(subSubElem, "SHOW_NUMBER", mUi->trackShowNumber, true);
                    loadBoolValue(subSubElem, "SHOW_GROUND_POSITION", mUi->trackShowGroundPosition, false);
                    loadBoolValue(subSubElem, "SHOW_GROUND_PATH", mUi->trackShowGroundPath, false);

                    setTrackPathColor(readQString(subSubElem, "TRACK_PATH_COLOR", "#ff0000"));
                    setTrackGroundPathColor(readQString(subSubElem, "TRACK_GROUND_PATH_COLOR", "#00ff00"));

                    loadBoolValue(subSubElem, "HEAD_SIZE", mUi->trackHeadSized, true);
                    loadIntValue(subSubElem, "CURRENT_POINT_SIZE", mUi->trackCurrentPointSize, 60);
                    loadIntValue(subSubElem, "CURRENT_POINT_LINE_WIDTH", mUi->trackCurrentPointLineWidth, 1);
                    loadIntValue(subSubElem, "POINTS_SIZE", mUi->trackPointSize, 7);
                    loadIntValue(subSubElem, "SHOW_POINTS_LINE_WIDTH", mUi->trackShowPointsLineWidth, 1);
                    loadIntValue(subSubElem, "PATH_SIZE", mUi->trackPathWidth, 2);
                    loadIntValue(subSubElem, "COLLECTIVE_COLOR_SIZE", mUi->trackColColorSize, 11);
                    loadIntValue(subSubElem, "COLOR_MARKER_SIZE", mUi->trackColorMarkerSize, 14);
                    loadIntValue(subSubElem, "COLOR_MARKER_LINE_WIDTH", mUi->trackColorMarkerLineWidth, 1);
                    loadIntValue(subSubElem, "NUMBER_SIZE", mUi->trackNumberSize, 14);
                    loadIntValue(subSubElem, "GROUND_POSITION_SIZE", mUi->trackGroundPositionSize, 1);
                    loadIntValue(subSubElem, "GROUND_PATH_SIZE", mUi->trackGroundPathSize, 1);
                    loadBoolValue(subSubElem, "POINTS_COLORED", mUi->trackShowPointsColored, true);
                    loadBoolValue(subSubElem, "NUMBER_BOLD", mUi->trackNumberBold, true);
                    loadIntValue(subSubElem, "BEFORE", mUi->trackShowBefore, 15);
                    loadIntValue(subSubElem, "AFTER", mUi->trackShowAfter, 15);
                }
                else
                {
                    SPDLOG_WARN("Unknown TRACKING tag: {}", subSubElem.tagName());
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
                    loadBoolValue(subSubElem, "ENABLED", mUi->anaMissingFrames);
                }
                else if(subSubElem.tagName() == "MARK_ACTUAL")
                {
                    loadBoolValue(subSubElem, "ENABLED", mUi->anaMarkAct);
                }
                else if(subSubElem.tagName() == "CALCULATION")
                {
                    loadIntValue(subSubElem, "STEP_SIZE", mUi->anaStep);
                    loadBoolValue(subSubElem, "CONSIDER_X", mUi->anaConsiderX);
                    loadBoolValue(subSubElem, "CONSIDER_Y", mUi->anaConsiderY);
                    loadBoolValue(subSubElem, "ABSOLUTE", mUi->anaConsiderAbs);
                    loadBoolValue(subSubElem, "REVERSE", mUi->anaConsiderRev);
                    loadBoolValue(subSubElem, "SHOW_VORONOI", mUi->showVoronoiCells, false);
                }
                else
                {
                    SPDLOG_WARN("Unknown ANALYSIS tag: {}", subSubElem.tagName());
                }
            }
        }
        else if(subElem.tagName() == "CORRECTION")
        {
            mCorrectionWidget->getXml(subElem);
        }
        else
        {
            SPDLOG_WARN("Unknown CONTROL tag: {}", subSubElem.tagName());
        }
    }
    mCoordSys->updateCoordItem();
}

bool Control::isLoading() const
{
    return mMainWindow->isLoading();
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
        SPDLOG_INFO("Clicked outside the image with color picker.");

        return false;
    }

    clickedColor = QColor{hsvImg->pixel(imgPoint.toPoint())};

    map       = this->getColorPlot()->getMapItem();
    toColor   = map->getActMapToColor();
    fromColor = map->getActMapFromColor();
    if(!toColor.isValid() || !fromColor.isValid())
    {
        SPDLOG_WARN("Map is corrupted");
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
    return mCorrectionWidget->getTestEqualChecked();
}

bool Control::isTestVelocityChecked() const
{
    return mCorrectionWidget->getTestVelocityChecked();
}

bool Control::isTestInsideChecked() const
{
    return mCorrectionWidget->getTestInsideChecked();
}

bool Control::isTestLengthChecked() const
{
    return mCorrectionWidget->getTestLengthChecked();
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

IntrinsicCameraParams Control::getIntrinsicCameraParams() const
{
    return mIntr->getIntrinsicCameraParams();
}

void Control::runAutoCalib()
{
    mIntr->runAutoCalib();
}

double Control::getDefaultHeight() const
{
    return mUi->mapDefaultHeight->value();
}

double Control::getCameraAltitude() const
{
    return mCoordSys->getCameraAltitude();
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

void Control::resetCorrection()
{
    mCorrectionWidget->clear();
}

#include "moc_control.cpp"
