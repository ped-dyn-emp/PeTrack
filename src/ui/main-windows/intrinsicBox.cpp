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

#include "intrinsicBox.h"

#include "autoCalib.h"
#include "calibFilter.h"
#include "extrinsicBox.h"
#include "helper.h"
#include "importHelper.h"
#include "pGroupBox.h"
#include "pMessageBox.h"
#include "ui_intrinsicBox.h"

#include <QDomElement>
#include <utility>

IntrinsicBox::IntrinsicBox(
    QWidget              *parent,
    AutoCalib            &autoCalib,
    CalibFilter          &calibFilter,
    ExtrinsicBox         &extrBox,
    std::function<void()> updateImageCallback) :
    IntrinsicBox(parent, new Ui::IntrinsicBox(), autoCalib, calibFilter, extrBox, updateImageCallback)
{
}

IntrinsicBox::IntrinsicBox(
    QWidget              *parent,
    Ui::IntrinsicBox     *ui,
    AutoCalib            &autoCalib,
    CalibFilter          &calibFilter,
    ExtrinsicBox         &extrBox,
    std::function<void()> updateImageCallback) :
    QWidget(parent),
    mUi(ui),
    mAutoCalib(autoCalib),
    mCalibFilter(calibFilter),
    mExtrBox(extrBox),
    mUpdateImageCallback(std::move(updateImageCallback))
{
    mUi->setupUi(this);
    connect(mUi->autoCalib, &QPushButton::clicked, this, &IntrinsicBox::runAutoCalib);
    connect(mUi->quadAspectRatio, &QCheckBox::clicked, this, &IntrinsicBox::showRecalibrationDialog);
    connect(mUi->fixCenter, &QCheckBox::clicked, this, &IntrinsicBox::showRecalibrationDialog);
    connect(mUi->tangDist, &QCheckBox::clicked, this, &IntrinsicBox::showRecalibrationDialog);
    connect(mUi->calibVideo, &QPushButton::clicked, this, &IntrinsicBox::onCalibVideoClicked);
    // apply intrinsic
    mUi->apply->setCheckState(mCalibFilter.getEnabled() ? Qt::Checked : Qt::Unchecked);

    mModelsParams.oldModelParams = IntrinsicCameraParams{};
    mModelsParams.extModelParams = IntrinsicCameraParams{};
    applyCurrentModelParamsToUi();
    setCalibSettings();
    // FocusPolicy: TabFocus and first ui-element as proxy are needed for tab order
    setFocusProxy(mUi->apply);
}

IntrinsicBox::~IntrinsicBox()
{
    delete mUi;
}

void IntrinsicBox::setIntrinsicCameraParams(const IntrinsicModelsParameters params)
{
    if(params == mModelsParams)
    {
        return;
    }
    mModelsParams = params;
    applyCurrentModelParamsToUi();
}

void IntrinsicBox::setCurrentIntrinsicCameraParameters(const IntrinsicCameraParams params)
{
    if(mUi->extModelCheckBox->isChecked())
    {
        mModelsParams.extModelParams = params;
    }
    else
    {
        mModelsParams.oldModelParams = params;
    }
    applyCurrentModelParamsToUi();
}

void IntrinsicBox::imageSizeChanged(int width, int height, int borderDiff)
{
    IntrinsicCameraParams params = getIntrinsicCameraParams();

    mCxFixed = (width - 1) / 2.;
    mCyFixed = (height - 1) / 2.;
    if(mUi->fixCenter->isChecked())
    {
        mModelsParams.oldModelParams.setCx(mCxFixed);
        mModelsParams.oldModelParams.setCy(mCyFixed);
        mModelsParams.extModelParams.setCx(mCxFixed);
        mModelsParams.extModelParams.setCy(mCyFixed);
        setValueBlocked(mUi->cx, mCxFixed);
        setValueBlocked(mUi->cy, mCyFixed);
    }
    else
    {
        try
        {
            mModelsParams.oldModelParams.setCx(mModelsParams.oldModelParams.getCx() + borderDiff);
            mModelsParams.oldModelParams.setCy(mModelsParams.oldModelParams.getCy() + borderDiff);
            mModelsParams.extModelParams.setCx(mModelsParams.extModelParams.getCx() + borderDiff);
            mModelsParams.extModelParams.setCy(mModelsParams.extModelParams.getCy() + borderDiff);
            setValueBlocked(mUi->cx, params.getCx() + borderDiff);
            setValueBlocked(mUi->cy, params.getCy() + borderDiff);
        }
        catch(std::domain_error &)
        {
            PCritical(
                nullptr,
                "Image resize invalidated data",
                "The image has a different size for which the current values for cx and cy are not valid anymore.");
        }
    }
    emit paramsChanged(getIntrinsicCameraParams());
}

IntrinsicCameraParams IntrinsicBox::getXmlParams(const QDomElement &elem)
{
    IntrinsicCameraParams params;

    params.setFx(readDouble(elem, "FX", 881));
    params.setFy(readDouble(elem, "FY", 881));
    params.setCx(readDouble(elem, "CX", 551.5));
    params.setCy(readDouble(elem, "CY", 383.5));
    params.setR2(readDouble(elem, "R2", 0));
    params.setR4(readDouble(elem, "R4", 0));
    params.setR6(readDouble(elem, "R6", 0));
    params.setTx(readDouble(elem, "TX", 0));
    params.setTy(readDouble(elem, "TY", 0));
    params.setK4(readDouble(elem, "K4", 0));
    params.setK5(readDouble(elem, "K5", 0));
    params.setK6(readDouble(elem, "K6", 0));
    params.setS1(readDouble(elem, "S1", 0));
    params.setS2(readDouble(elem, "S2", 0));
    params.setS3(readDouble(elem, "S3", 0));
    params.setS4(readDouble(elem, "S4", 0));
    params.setTaux(readDouble(elem, "TAUX", 0));
    params.setTauy(readDouble(elem, "TAUY", 0));
    params.reprojectionError = readDouble(elem, "ReprError", std::numeric_limits<double>::quiet_NaN());

    return params;
}

void IntrinsicBox::loadCalibFiles(QDomElement &subSubElem)
{
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
                tmpStr = getExistingFile(fl[i]);
                if(tmpStr != "")
                {
                    fl[i] = tmpStr;
                }
            }
        }
        // auch setzen, wenn leer, vielleicht ist das ja gewuenscht
        mAutoCalib.setCalibFiles(fl);
        if(!fl.isEmpty())
        {
            mUi->autoCalib->setEnabled(true);
        }
    }
    mAutoCalib.setCalibVideo(getExistingFile(readQString(subSubElem, "CALIB_VIDEO", "")));
}

bool IntrinsicBox::getXml(QDomElement &subSubElem)
{
    if(subSubElem.tagName() == "PATTERN")
    {
        loadIntValue(subSubElem, "BOARD_SIZE_X", mUi->boardSizeX, 6);
        loadIntValue(subSubElem, "BOARD_SIZE_Y", mUi->boardSizeY, 8);
        loadDoubleValue(subSubElem, "SQUARE_SIZE", mUi->squareSize, 4.6);
    }
    else if(subSubElem.tagName() == "INTRINSIC_PARAMETERS")
    {
        loadBoolValue(subSubElem, "ENABLED", mUi->apply, false);
        // signals are supposed to change model values; but since we read them in
        // from a valid file, no changes are neccessary -> can safely block signals
        loadBoolValue(subSubElem, "QUAD_ASPECT_RATIO", mUi->quadAspectRatio, false);
        loadBoolValue(subSubElem, "FIX_CENTER", mUi->fixCenter, false);
        loadBoolValue(subSubElem, "TANG_DIST", mUi->tangDist, false);

        if(subSubElem.elementsByTagName("OLD_MODEL").isEmpty())
        {
            bool useNewModel = true;
            auto params      = getXmlParams(subSubElem);

            if(subSubElem.hasAttribute("EXT_MODEL_ENABLED"))
            {
                useNewModel = readBool(subSubElem, "EXT_MODEL_ENABLED");
            }
            else
            {
                useNewModel = params.getS1() != 0.;
            }

            if(useNewModel)
            {
                mModelsParams.extModelParams = params;
                mModelsParams.oldModelParams = IntrinsicCameraParams{};
            }
            else
            {
                mModelsParams.oldModelParams = params;
                mModelsParams.extModelParams = IntrinsicCameraParams{};
            }
        }
        else
        {
            // new file with multiple models saved
            mModelsParams.extModelParams = getXmlParams(subSubElem.firstChildElement("EXT_MODEL"));
            mModelsParams.oldModelParams = getXmlParams(subSubElem.firstChildElement("OLD_MODEL"));
        }
        checkModelParams(mModelsParams.oldModelParams);
        checkModelParams(mModelsParams.extModelParams);

        loadBoolValue(subSubElem, "EXT_MODEL_ENABLED", mUi->extModelCheckBox, false);

        loadCalibFiles(subSubElem);

        const QDomElement root = subSubElem.ownerDocument().firstChildElement("PETRACK");

        if(newerThanVersion(QString("0.9.0"), root.attribute("VERSION")))
        {
            PWarning(
                this,
                tr("PeTrack"),
                tr("You are using a project version lower than 0.9: Therefore, the extended intrinsic calibration "
                   "model is disabled."));
            mUi->extModelCheckBox->setChecked(false);
            // when using the old model all extended model parameters should be set to zero
            setExtendedParametersInOldModelToZero();
        }
        else if(newerThanVersion(QString("0.9.2"), root.attribute("VERSION")))
        {
            // only checking one parameter because if the ext. model is used all parameters are not equal to zero
            if(mUi->s1->value() == 0.)
            {
                mUi->extModelCheckBox->setChecked(false);
            }
            else
            {
                mUi->extModelCheckBox->setChecked(true);
            }
        }
        if(newerThanVersion(QString("0.10.3"), root.attribute("VERSION")))
        {
            IntrinsicCameraParams standardParams{};
            if(mModelsParams.oldModelParams == standardParams || mModelsParams.extModelParams == standardParams)
            {
                PWarning(
                    this,
                    tr("PeTrack"),
                    tr("You are using a project version that only uses one calibration model. To use both, you need to "
                       "recalibrate"));
            }
        }
        applyCurrentModelParamsToUi();
        setCalibSettings();
        if(subSubElem.hasAttribute("IMMUTABLE"))
        {
            if(this->parent())
            {
                auto *parent = dynamic_cast<PGroupBox *>(this->parent()->parent());
                if(parent)
                {
                    parent->setImmutable(readBool(subSubElem, "IMMUTABLE", false));
                }
            }
        }
    }
    else
    {
        return false;
    }
    return true;
}

void IntrinsicBox::setXml(QDomElement &subElem) const
{
    auto subSubElem = (subElem.ownerDocument()).createElement("PATTERN");
    subSubElem.setAttribute("BOARD_SIZE_X", mAutoCalib.getBoardSizeX()); // 6
    subSubElem.setAttribute("BOARD_SIZE_Y", mAutoCalib.getBoardSizeY()); // 8 oder 9
    subSubElem.setAttribute("SQUARE_SIZE", mAutoCalib.getSquareSize());  // in cm
    subElem.appendChild(subSubElem);

    subSubElem = (subElem.ownerDocument()).createElement("INTRINSIC_PARAMETERS");
    subSubElem.setAttribute("ENABLED", mUi->apply->isChecked());

    auto subSubSubElem = (subSubElem.ownerDocument()).createElement("OLD_MODEL");
    subSubSubElem.setAttribute("FX", mModelsParams.oldModelParams.getFx());
    subSubSubElem.setAttribute("FY", mModelsParams.oldModelParams.getFy());
    subSubSubElem.setAttribute("CX", mModelsParams.oldModelParams.getCx());
    subSubSubElem.setAttribute("CY", mModelsParams.oldModelParams.getCy());
    subSubSubElem.setAttribute("R2", mModelsParams.oldModelParams.getR2());
    subSubSubElem.setAttribute("R4", mModelsParams.oldModelParams.getR4());
    subSubSubElem.setAttribute("R6", mModelsParams.oldModelParams.getR6());
    subSubSubElem.setAttribute("TX", mModelsParams.oldModelParams.getTx());
    subSubSubElem.setAttribute("TY", mModelsParams.oldModelParams.getTy());
    subSubSubElem.setAttribute("K4", mModelsParams.oldModelParams.getK4());
    subSubSubElem.setAttribute("K5", mModelsParams.oldModelParams.getK5());
    subSubSubElem.setAttribute("K6", mModelsParams.oldModelParams.getK6());
    subSubSubElem.setAttribute("S1", mModelsParams.oldModelParams.getS1());
    subSubSubElem.setAttribute("S2", mModelsParams.oldModelParams.getS2());
    subSubSubElem.setAttribute("S3", mModelsParams.oldModelParams.getS3());
    subSubSubElem.setAttribute("S4", mModelsParams.oldModelParams.getS4());
    subSubSubElem.setAttribute("TAUX", mModelsParams.oldModelParams.getTaux());
    subSubSubElem.setAttribute("TAUY", mModelsParams.oldModelParams.getTauy());
    subSubSubElem.setAttribute("ReprError", mModelsParams.oldModelParams.getReprojectionError());

    subSubElem.appendChild(subSubSubElem);

    subSubSubElem = (subSubElem.ownerDocument()).createElement("EXT_MODEL");
    subSubSubElem.setAttribute("FX", mModelsParams.extModelParams.getFx());
    subSubSubElem.setAttribute("FY", mModelsParams.extModelParams.getFy());
    subSubSubElem.setAttribute("CX", mModelsParams.extModelParams.getCx());
    subSubSubElem.setAttribute("CY", mModelsParams.extModelParams.getCy());
    subSubSubElem.setAttribute("R2", mModelsParams.extModelParams.getR2());
    subSubSubElem.setAttribute("R4", mModelsParams.extModelParams.getR4());
    subSubSubElem.setAttribute("R6", mModelsParams.extModelParams.getR6());
    subSubSubElem.setAttribute("TX", mModelsParams.extModelParams.getTx());
    subSubSubElem.setAttribute("TY", mModelsParams.extModelParams.getTy());
    subSubSubElem.setAttribute("K4", mModelsParams.extModelParams.getK4());
    subSubSubElem.setAttribute("K5", mModelsParams.extModelParams.getK5());
    subSubSubElem.setAttribute("K6", mModelsParams.extModelParams.getK6());
    subSubSubElem.setAttribute("S1", mModelsParams.extModelParams.getS1());
    subSubSubElem.setAttribute("S2", mModelsParams.extModelParams.getS2());
    subSubSubElem.setAttribute("S3", mModelsParams.extModelParams.getS3());
    subSubSubElem.setAttribute("S4", mModelsParams.extModelParams.getS4());
    subSubSubElem.setAttribute("TAUX", mModelsParams.extModelParams.getTaux());
    subSubSubElem.setAttribute("TAUY", mModelsParams.extModelParams.getTauy());
    subSubSubElem.setAttribute("ReprError", mModelsParams.extModelParams.getReprojectionError());

    subSubElem.appendChild(subSubSubElem);
    subSubElem.setAttribute("QUAD_ASPECT_RATIO", mCalibSettings.quadAspectRatio);
    subSubElem.setAttribute("FIX_CENTER", mCalibSettings.fixCenter);
    subSubElem.setAttribute("TANG_DIST", mCalibSettings.tangDist);
    subSubElem.setAttribute("EXT_MODEL_ENABLED", mUi->extModelCheckBox->isChecked());

    // in dateiname darf kein , vorkommen - das blank ", " zur uebersich - beim einlesen wird nur ","
    // genommen und blanks rundherum abgeschnitten, falls von hand editiert wurde
    QStringList fl = mAutoCalib.getCalibFiles();
    for(int i = 0; i < fl.size(); ++i)
    {
        if(QFileInfo(fl.at(i)).isRelative() && QFileInfo::exists(fl.at(i)))
        {
            fl.replace(i, fl.at(i) + ";" + QFileInfo(fl.at(i)).absoluteFilePath());
        }
    }
    subSubElem.setAttribute("CALIB_FILES", fl.join(", "));
    subSubElem.setAttribute("CALIB_VIDEO", mAutoCalib.getCalibVideo());

    if(this->parent())
    {
        auto *parent = dynamic_cast<PGroupBox *>(this->parent()->parent());
        if(parent)
        {
            subSubElem.setAttribute("IMMUTABLE", parent->isImmutable());
        }
    }
    subElem.appendChild(subSubElem);
}

void IntrinsicBox::showRecalibrationDialog()
{
    int ret = PCustom(
        this,
        tr("PeTrack"),
        tr("You have to calibrate again to bring your change into effect"),
        {"Recalibrate", "Continue with current calibration"},
        "Recalibrate");

    if(ret == 0)
    {
        runAutoCalib();
    }
}

void IntrinsicBox::on_fx_valueChanged(double d)
{
    IntrinsicCameraParams params = getIntrinsicCameraParams();
    params.setFx(d);
    params.reprojectionError = std::numeric_limits<double>::quiet_NaN();
    mUi->intrError->setText(QString("invalid"));

    setCurrentIntrinsicCameraParameters(params);
}

void IntrinsicBox::on_fy_valueChanged(double d)
{
    IntrinsicCameraParams params = getIntrinsicCameraParams();
    params.setFy(d);
    params.reprojectionError = std::numeric_limits<double>::quiet_NaN();
    mUi->intrError->setText(QString("invalid"));

    setCurrentIntrinsicCameraParameters(params);
}

void IntrinsicBox::on_cx_valueChanged(double d)
{
    IntrinsicCameraParams params = getIntrinsicCameraParams();
    params.setCx(d);
    params.reprojectionError = std::numeric_limits<double>::quiet_NaN();
    mUi->intrError->setText(QString("invalid"));

    setCurrentIntrinsicCameraParameters(params);
}

void IntrinsicBox::on_cy_valueChanged(double d)
{
    IntrinsicCameraParams params = getIntrinsicCameraParams();
    params.setCy(d);
    params.reprojectionError = std::numeric_limits<double>::quiet_NaN();
    mUi->intrError->setText(QString("invalid"));

    setCurrentIntrinsicCameraParameters(params);
}

void IntrinsicBox::on_r2_valueChanged(double d)
{
    IntrinsicCameraParams params = getIntrinsicCameraParams();
    params.setR2(d);
    params.reprojectionError = std::numeric_limits<double>::quiet_NaN();
    mUi->intrError->setText(QString("invalid"));

    setCurrentIntrinsicCameraParameters(params);
}

void IntrinsicBox::on_r4_valueChanged(double d)
{
    IntrinsicCameraParams params = getIntrinsicCameraParams();
    params.setR4(d);
    params.reprojectionError = std::numeric_limits<double>::quiet_NaN();
    mUi->intrError->setText(QString("invalid"));

    setCurrentIntrinsicCameraParameters(params);
}

void IntrinsicBox::on_r6_valueChanged(double d)
{
    IntrinsicCameraParams params = getIntrinsicCameraParams();
    params.setR6(d);
    params.reprojectionError = std::numeric_limits<double>::quiet_NaN();
    mUi->intrError->setText(QString("invalid"));

    setCurrentIntrinsicCameraParameters(params);
}
void IntrinsicBox::on_s1_valueChanged(double d)
{
    IntrinsicCameraParams params = getIntrinsicCameraParams();
    params.setS1(d);
    params.reprojectionError = std::numeric_limits<double>::quiet_NaN();
    mUi->intrError->setText(QString("invalid"));

    setCurrentIntrinsicCameraParameters(params);
}
void IntrinsicBox::on_s2_valueChanged(double d)
{
    IntrinsicCameraParams params = getIntrinsicCameraParams();
    params.setS2(d);
    params.reprojectionError = std::numeric_limits<double>::quiet_NaN();
    mUi->intrError->setText(QString("invalid"));

    setCurrentIntrinsicCameraParameters(params);
}
void IntrinsicBox::on_s3_valueChanged(double d)
{
    IntrinsicCameraParams params = getIntrinsicCameraParams();
    params.setS3(d);
    params.reprojectionError = std::numeric_limits<double>::quiet_NaN();
    mUi->intrError->setText(QString("invalid"));

    setCurrentIntrinsicCameraParameters(params);
}
void IntrinsicBox::on_s4_valueChanged(double d)
{
    IntrinsicCameraParams params = getIntrinsicCameraParams();
    params.setS4(d);
    params.reprojectionError = std::numeric_limits<double>::quiet_NaN();
    mUi->intrError->setText(QString("invalid"));

    setCurrentIntrinsicCameraParameters(params);
}
void IntrinsicBox::on_taux_valueChanged(double d)
{
    IntrinsicCameraParams params = getIntrinsicCameraParams();
    params.setTaux(d);
    params.reprojectionError = std::numeric_limits<double>::quiet_NaN();
    mUi->intrError->setText(QString("invalid"));

    setCurrentIntrinsicCameraParameters(params);
}
void IntrinsicBox::on_tauy_valueChanged(double d)
{
    IntrinsicCameraParams params = getIntrinsicCameraParams();
    params.setTauy(d);
    params.reprojectionError = std::numeric_limits<double>::quiet_NaN();
    mUi->intrError->setText(QString("invalid"));

    setCurrentIntrinsicCameraParameters(params);
}
void IntrinsicBox::on_tx_valueChanged(double d)
{
    IntrinsicCameraParams params = getIntrinsicCameraParams();
    params.setTx(d);
    params.reprojectionError = std::numeric_limits<double>::quiet_NaN();
    mUi->intrError->setText(QString("invalid"));

    setCurrentIntrinsicCameraParameters(params);
}

void IntrinsicBox::on_ty_valueChanged(double d)
{
    IntrinsicCameraParams params = getIntrinsicCameraParams();
    params.setTy(d);
    params.reprojectionError = std::numeric_limits<double>::quiet_NaN();
    mUi->intrError->setText(QString("invalid"));

    setCurrentIntrinsicCameraParameters(params);
}
void IntrinsicBox::on_k4_valueChanged(double d)
{
    IntrinsicCameraParams params = getIntrinsicCameraParams();
    params.setK4(d);
    params.reprojectionError = std::numeric_limits<double>::quiet_NaN();
    mUi->intrError->setText(QString("invalid"));

    setCurrentIntrinsicCameraParameters(params);
}
void IntrinsicBox::on_k5_valueChanged(double d)
{
    IntrinsicCameraParams params = getIntrinsicCameraParams();
    params.setK5(d);
    params.reprojectionError = std::numeric_limits<double>::quiet_NaN();
    mUi->intrError->setText(QString("invalid"));

    setCurrentIntrinsicCameraParameters(params);
}
void IntrinsicBox::on_k6_valueChanged(double d)
{
    IntrinsicCameraParams params = getIntrinsicCameraParams();
    params.setK6(d);
    params.reprojectionError = std::numeric_limits<double>::quiet_NaN();
    mUi->intrError->setText(QString("invalid"));

    setCurrentIntrinsicCameraParameters(params);
}

void IntrinsicBox::on_boardSizeX_valueChanged(int x)
{
    mAutoCalib.setBoardSizeX(x);
}

void IntrinsicBox::on_boardSizeY_valueChanged(int y)
{
    mAutoCalib.setBoardSizeY(y);
}

void IntrinsicBox::on_squareSize_valueChanged(double s)
{
    mAutoCalib.setSquareSize(static_cast<float>(s));
}

void IntrinsicBox::setCalibSettings()
{
    mCalibSettings.quadAspectRatio = mUi->quadAspectRatio->isChecked();
    mCalibSettings.fixCenter       = mUi->fixCenter->isChecked();
    mCalibSettings.tangDist        = mUi->tangDist->isChecked();
}

void IntrinsicBox::runAutoCalib()
{
    auto params = mAutoCalib.autoCalib(
        mUi->quadAspectRatio->isChecked(), mUi->fixCenter->isChecked(), mUi->tangDist->isChecked());
    if(params)
    {
        setCalibSettings();
        IntrinsicModelsParameters validParams = std::move(*params);
        mModelsParams.oldModelParams          = validParams.oldModelParams;
        mModelsParams.extModelParams          = validParams.extModelParams;
        applyCurrentModelParamsToUi();
    }
}
void IntrinsicBox::on_calibFiles_clicked()
{
    if(mAutoCalib.openCalibFiles())
    {
        mUi->autoCalib->setEnabled(true);
    }
}

void IntrinsicBox::onCalibVideoClicked()
{
    if(mAutoCalib.openCalibVideo())
    {
        mUi->autoCalib->setEnabled(true);
    }
}

void IntrinsicBox::on_extModelCheckBox_stateChanged(int)
{
    bool checked = mUi->extModelCheckBox->isChecked();

    mUi->k4->setEnabled(checked);
    mUi->k5->setEnabled(checked);
    mUi->k6->setEnabled(checked);
    mUi->s1->setEnabled(checked);
    mUi->s2->setEnabled(checked);
    mUi->s3->setEnabled(checked);
    mUi->s4->setEnabled(checked);
    mUi->taux->setEnabled(checked);
    mUi->tauy->setEnabled(checked);

    applyCurrentModelParamsToUi();
}

void IntrinsicBox::on_extModelCheckBox_clicked(bool checked)
{
    // clicked only called in user action
    const ExtrinsicParameters standardExtParams{};
    if(standardExtParams != mExtrBox.getExtrinsicParameters())
    {
        int ret = PCustom(
            this,
            tr("PeTrack"),
            tr("Using different intrinsic calibration would result in a different extrinsic calibration"),
            {"Delete extrinsic calibration", "Keep intrinsic calibration", "Keep invalid extrinsic calibration"},
            "Delete extrinsic calibration");
        if(ret == 0)
        {
            mExtrBox.setExtrinsicParameters(standardExtParams);
        }
        else if(ret == 1)
        {
            mUi->extModelCheckBox->setCheckState(checked ? Qt::Unchecked : Qt::Checked);
        }
    }
}

IntrinsicCameraParams IntrinsicBox::getIntrinsicCameraParams() const
{
    if(mUi->extModelCheckBox->isChecked())
    {
        return mModelsParams.extModelParams;
    }
    else
    {
        return mModelsParams.oldModelParams;
    }
}
IntrinsicModelsParameters IntrinsicBox::getBothIntrinsicCameraParams() const
{
    return mModelsParams;
}
void IntrinsicBox::applyCurrentModelParamsToUi()
{
    IntrinsicCameraParams modelParams = getIntrinsicCameraParams();

    setValueBlocked(mUi->fx, modelParams.getFx());
    setValueBlocked(mUi->fy, modelParams.getFy());
    setValueBlocked(mUi->cx, modelParams.getCx());
    setValueBlocked(mUi->cy, modelParams.getCy());
    setValueBlocked(mUi->r2, static_cast<double>(modelParams.getR2()));
    setValueBlocked(mUi->r4, static_cast<double>(modelParams.getR4()));
    setValueBlocked(mUi->tx, static_cast<double>(modelParams.getTx()));
    setValueBlocked(mUi->ty, static_cast<double>(modelParams.getTy()));
    setValueBlocked(mUi->r6, static_cast<double>(modelParams.getR6()));
    setValueBlocked(mUi->k4, static_cast<double>(modelParams.getK4()));
    setValueBlocked(mUi->k5, static_cast<double>(modelParams.getK5()));
    setValueBlocked(mUi->k6, static_cast<double>(modelParams.getK6()));
    setValueBlocked(mUi->s1, static_cast<double>(modelParams.getS1()));
    setValueBlocked(mUi->s2, static_cast<double>(modelParams.getS2()));
    setValueBlocked(mUi->s3, static_cast<double>(modelParams.getS3()));
    setValueBlocked(mUi->s4, static_cast<double>(modelParams.getS4()));
    setValueBlocked(mUi->taux, static_cast<double>(modelParams.getTaux()));
    setValueBlocked(mUi->tauy, static_cast<double>(modelParams.getTauy()));

    if(qIsNaN(modelParams.reprojectionError))
    {
        mUi->intrError->setText(QString("invalid"));
    }
    else
    {
        mUi->intrError->setText(QString("%1").arg(modelParams.reprojectionError));
    }

    emit paramsChanged(modelParams);
}

void IntrinsicBox::checkModelParams(const IntrinsicCameraParams &modelParams)
{
    checkValueValid(mUi->fx, modelParams.getFx());
    checkValueValid(mUi->fy, modelParams.getFy());
    checkValueValid(mUi->cx, modelParams.getCx());
    checkValueValid(mUi->cy, modelParams.getCy());
    checkValueValid(mUi->r2, static_cast<double>(modelParams.getR2()));
    checkValueValid(mUi->r4, static_cast<double>(modelParams.getR4()));
    checkValueValid(mUi->tx, static_cast<double>(modelParams.getTx()));
    checkValueValid(mUi->ty, static_cast<double>(modelParams.getTy()));
    checkValueValid(mUi->r6, static_cast<double>(modelParams.getR6()));
    checkValueValid(mUi->k4, static_cast<double>(modelParams.getK4()));
    checkValueValid(mUi->k5, static_cast<double>(modelParams.getK5()));
    checkValueValid(mUi->k6, static_cast<double>(modelParams.getK6()));
    checkValueValid(mUi->s1, static_cast<double>(modelParams.getS1()));
    checkValueValid(mUi->s2, static_cast<double>(modelParams.getS2()));
    checkValueValid(mUi->s3, static_cast<double>(modelParams.getS3()));
    checkValueValid(mUi->s4, static_cast<double>(modelParams.getS4()));
    checkValueValid(mUi->taux, static_cast<double>(modelParams.getTaux()));
    checkValueValid(mUi->tauy, static_cast<double>(modelParams.getTauy()));
}

void IntrinsicBox::on_quadAspectRatio_stateChanged(int)
{
    bool checked = mUi->quadAspectRatio->isChecked();
    mUi->fy->setEnabled(!checked);
}

void IntrinsicBox::on_fixCenter_stateChanged(int)
{
    bool checked = mUi->fixCenter->isChecked();
    mUi->cx->setEnabled(!checked);
    mUi->cy->setEnabled(!checked);
}

void IntrinsicBox::on_tangDist_stateChanged(int)
{
    bool checked = mUi->tangDist->isChecked();
    mUi->tx->setEnabled(checked);
    mUi->ty->setEnabled(checked);
}

void IntrinsicBox::on_apply_stateChanged(int i)
{
    if(i == Qt::Checked)
    {
        mCalibFilter.enable();
    }
    else if(i == Qt::Unchecked)
    {
        mCalibFilter.disable();
    }
    mUpdateImageCallback();
}
/**
 * @brief Set all extended parameters in the old model to zero
 *
 * In older versions the parameters k4, k5 and k6 were part of the ui but didn't have an actual influence on the
 * calibration. By implementing the extended model these parameters now have an influence and thus old project files
 * with non-zero values for these parameters lead to incorrect calibrations in the newer versions. Setting all extended
 * parameters to zero solves that issue.
 */
void IntrinsicBox::setExtendedParametersInOldModelToZero()
{
    mModelsParams.oldModelParams.setK4(0);
    mModelsParams.oldModelParams.setK5(0);
    mModelsParams.oldModelParams.setK6(0);
    mModelsParams.oldModelParams.setS1(0);
    mModelsParams.oldModelParams.setS2(0);
    mModelsParams.oldModelParams.setS3(0);
    mModelsParams.oldModelParams.setS4(0);
    mModelsParams.oldModelParams.setTaux(0);
    mModelsParams.oldModelParams.setTauy(0);
}
