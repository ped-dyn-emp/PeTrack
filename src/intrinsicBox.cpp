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

#include "intrinsicBox.h"

#include "autoCalib.h"
#include "calibFilter.h"
#include "helper.h"
#include "pGroupBox.h"
#include "pMessageBox.h"
#include "ui_intrinsicBox.h"

#include <QDomElement>
#include <utility>

IntrinsicBox::IntrinsicBox(
    QWidget              *parent,
    AutoCalib            &autoCalib,
    CalibFilter          &calibFilter,
    std::function<void()> updateImageCallback) :
    IntrinsicBox(parent, new Ui::IntrinsicBox(), autoCalib, calibFilter, updateImageCallback)
{
}

IntrinsicBox::IntrinsicBox(
    QWidget              *parent,
    Ui::IntrinsicBox     *ui,
    AutoCalib            &autoCalib,
    CalibFilter          &calibFilter,
    std::function<void()> updateImageCallback) :
    QWidget(parent),
    mUi(ui),
    mAutoCalib(autoCalib),
    mCalibFilter(calibFilter),
    mUpdateImageCallback(std::move(updateImageCallback))
{
    mUi->setupUi(this);
    connect(mUi->extModelCheckBox, &QCheckBox::stateChanged, this, &IntrinsicBox::on_extModelCheckBox_stateChanged);
    connect(mUi->autoCalib, &QPushButton::clicked, this, &IntrinsicBox::runAutoCalib);
    // apply intrinsic
    mUi->apply->setCheckState(mCalibFilter.getEnabled() ? Qt::Checked : Qt::Unchecked);

    setIntrinsicCameraParams(mCalibFilter.getCamParams().getValue());

    // FocusPolicy: TabFocus and first ui-element as proxy are needed for tab order
    setFocusProxy(mUi->apply);
}

IntrinsicBox::~IntrinsicBox()
{
    delete mUi;
}

void IntrinsicBox::setIntrinsicCameraParams(const IntrinsicCameraParams &params)
{
    if(params == mParams)
    {
        // do not send signal if value hasn't changed
        return;
    }

    {
        // don't want to send signals for intermediate params
        const QSignalBlocker blocker(this);

        setValue(mUi->fx, params.getFx());
        setValue(mUi->fy, params.getFy());
        setValue(mUi->cx, params.getCx());
        setValue(mUi->cy, params.getCy());
        setValue(mUi->r2, static_cast<double>(params.distortionCoeffs.at<float>(0)));
        setValue(mUi->r4, static_cast<double>(params.distortionCoeffs.at<float>(1)));
        setValue(mUi->tx, static_cast<double>(params.distortionCoeffs.at<float>(2)));
        setValue(mUi->ty, static_cast<double>(params.distortionCoeffs.at<float>(3)));
        setValue(mUi->r6, static_cast<double>(params.distortionCoeffs.at<float>(4)));
        setValue(mUi->k4, static_cast<double>(params.distortionCoeffs.at<float>(5)));
        setValue(mUi->k5, static_cast<double>(params.distortionCoeffs.at<float>(6)));
        setValue(mUi->k6, static_cast<double>(params.distortionCoeffs.at<float>(7)));
        setValue(mUi->s1, static_cast<double>(params.distortionCoeffs.at<float>(8)));
        setValue(mUi->s2, static_cast<double>(params.distortionCoeffs.at<float>(9)));
        setValue(mUi->s3, static_cast<double>(params.distortionCoeffs.at<float>(10)));
        setValue(mUi->s4, static_cast<double>(params.distortionCoeffs.at<float>(11)));
        setValue(mUi->taux, static_cast<double>(params.distortionCoeffs.at<float>(12)));
        setValue(mUi->tauy, static_cast<double>(params.distortionCoeffs.at<float>(13)));

        if(params.reprojectionError == std::numeric_limits<float>::quiet_NaN())
        {
            mUi->intrError->setText(QString("invalid"));
        }
        else
        {
            mUi->intrError->setText(QString("%1").arg(params.reprojectionError));
        }

        // needed here because I surpress signals
        mParams = params;
    }
    emit paramsChanged(mParams);
}

void IntrinsicBox::imageSizeChanged(int width, int height, int borderDiff)
{
    const double cX = mParams.getCx();
    const double cY = mParams.getCy();

    mCxFixed = (width - 1) / 2.;
    mCyFixed = (height - 1) / 2.;

    if(mUi->fixCenter->isChecked())
    {
        mUi->cx->setValue(mCxFixed);
        mUi->cy->setValue(mCyFixed);
    }
    else
    {
        try
        {
            setValue(mUi->cx, cX + borderDiff);
            setValue(mUi->cy, cY + borderDiff);
        }
        catch(std::domain_error &)
        {
            PCritical(
                nullptr,
                "Image resize invalidated data",
                "The image has a different size for which the current values for cx and cy are not valid anymore.");
        }
    }
}

bool IntrinsicBox::getXml(QDomElement &subSubElem)
{
    if(subSubElem.tagName() == "PATTERN")
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
        IntrinsicCameraParams params;
        if(subSubElem.hasAttribute("ENABLED"))
        {
            mUi->apply->setCheckState(subSubElem.attribute("ENABLED").toInt() ? Qt::Checked : Qt::Unchecked);
        }
        if(subSubElem.hasAttribute("FX"))
        {
            params.setFx(subSubElem.attribute("FX").toDouble());
        }
        if(subSubElem.hasAttribute("FY"))
        {
            params.setFy(subSubElem.attribute("FY").toDouble());
        }
        if(subSubElem.hasAttribute("CX"))
        {
            double cxVal = subSubElem.attribute("CX").toDouble();
            if(cxVal < mUi->cx->minimum())
            {
                mUi->cx->setMinimum(cxVal - 50);
            }
            if(cxVal > mUi->cx->maximum())
            {
                mUi->cx->setMaximum(cxVal + 50);
            }
            params.setCx(cxVal);
        }
        if(subSubElem.hasAttribute("CY"))
        {
            double cyVal = subSubElem.attribute("CY").toDouble();
            if(cyVal < mUi->cy->minimum())
            {
                mUi->cy->setMinimum(cyVal - 50);
            }
            if(cyVal > mUi->cy->maximum())
            {
                mUi->cy->setMaximum(cyVal + 50);
            }
            params.setCy(cyVal);
        }
        if(subSubElem.hasAttribute("R2"))
        {
            params.distortionCoeffs.at<float>(0) = subSubElem.attribute("R2").toDouble();
        }
        if(subSubElem.hasAttribute("R4"))
        {
            params.distortionCoeffs.at<float>(1) = subSubElem.attribute("R4").toDouble();
        }
        if(subSubElem.hasAttribute("R6"))
        {
            params.distortionCoeffs.at<float>(4) = subSubElem.attribute("R6").toDouble();
        }
        if(subSubElem.hasAttribute("TX"))
        {
            params.distortionCoeffs.at<float>(2) = subSubElem.attribute("TX").toDouble();
        }
        if(subSubElem.hasAttribute("TY"))
        {
            params.distortionCoeffs.at<float>(3) = subSubElem.attribute("TY").toDouble();
        }
        if(subSubElem.hasAttribute("K4"))
        {
            params.distortionCoeffs.at<float>(5) = subSubElem.attribute("K4").toDouble();
        }
        if(subSubElem.hasAttribute("K5"))
        {
            params.distortionCoeffs.at<float>(6) = subSubElem.attribute("K5").toDouble();
        }
        if(subSubElem.hasAttribute("K6"))
        {
            params.distortionCoeffs.at<float>(7) = subSubElem.attribute("K6").toDouble();
        }
        if(subSubElem.hasAttribute("S1"))
        {
            params.distortionCoeffs.at<float>(8) = subSubElem.attribute("S1").toDouble();
        }
        if(subSubElem.hasAttribute("S2"))
        {
            params.distortionCoeffs.at<float>(9) = subSubElem.attribute("S2").toDouble();
        }
        if(subSubElem.hasAttribute("S3"))
        {
            params.distortionCoeffs.at<float>(10) = subSubElem.attribute("S3").toDouble();
        }
        if(subSubElem.hasAttribute("S4"))
        {
            params.distortionCoeffs.at<float>(11) = subSubElem.attribute("S4").toDouble();
        }
        if(subSubElem.hasAttribute("TAUX"))
        {
            params.distortionCoeffs.at<float>(12) = subSubElem.attribute("TAUX").toDouble();
        }
        if(subSubElem.hasAttribute("TAUY"))
        {
            params.distortionCoeffs.at<float>(13) = subSubElem.attribute("TAUY").toDouble();
        }
        if(subSubElem.hasAttribute("ReprError"))
        {
            bool conversionSuccessful;
            params.reprojectionError = subSubElem.attribute("ReprError").toDouble(&conversionSuccessful);
            if(!conversionSuccessful)
            {
                // Qt sets to inf instead of nan
                params.reprojectionError = std::numeric_limits<double>::quiet_NaN();
            }
        }
        if(subSubElem.hasAttribute("QUAD_ASPECT_RATIO"))
        {
            mUi->quadAspectRatio->setCheckState(
                subSubElem.attribute("QUAD_ASPECT_RATIO").toInt() ? Qt::Checked : Qt::Unchecked);
        }
        if(subSubElem.hasAttribute("FIX_CENTER"))
        {
            mUi->fixCenter->setCheckState(subSubElem.attribute("FIX_CENTER").toInt() ? Qt::Checked : Qt::Unchecked);
        }
        if(subSubElem.hasAttribute("TANG_DIST"))
        {
            mUi->tangDist->setCheckState(subSubElem.attribute("TANG_DIST").toInt() ? Qt::Checked : Qt::Unchecked);
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

        const QDomElement root = subSubElem.ownerDocument().firstChildElement("PETRACK");

        if(newerThanVersion(QString("0.9.0"), root.attribute("VERSION")))
        {
            PWarning(
                this,
                tr("PeTrack"),
                tr("You are using a project version lower than 0.9: Therefore, the extended intrinsic calibration "
                   "model is disabled."));
            mUi->extModelCheckBox->setChecked(false);
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

        setIntrinsicCameraParams(params);

        if(subSubElem.hasAttribute("IMMUTABLE"))
        {
            if(this->parent())
            {
                auto *parent = dynamic_cast<PGroupBox *>(this->parent()->parent());
                if(parent)
                {
                    parent->setImmutable(subSubElem.attribute("IMMUTABLE").toInt());
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
    subSubElem.setAttribute("ReprError", mParams.reprojectionError);

    subSubElem.setAttribute("QUAD_ASPECT_RATIO", mUi->quadAspectRatio->isChecked());
    subSubElem.setAttribute("FIX_CENTER", mUi->fixCenter->isChecked());
    subSubElem.setAttribute("TANG_DIST", mUi->tangDist->isChecked());
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

void IntrinsicBox::on_fx_valueChanged(double d)
{
    mParams.setFx(d);
    if(mUi->quadAspectRatio->isChecked())
    {
        mUi->fy->setValue(d);
    }
    mUi->intrError->setText(QString("invalid"));
    mParams.reprojectionError = std::numeric_limits<double>::quiet_NaN();

    emit paramsChanged(mParams);
}

void IntrinsicBox::on_fy_valueChanged(double d)
{
    mParams.setFy(d);
    mUi->intrError->setText(QString("invalid"));
    mParams.reprojectionError = std::numeric_limits<double>::quiet_NaN();

    emit paramsChanged(mParams);
}

void IntrinsicBox::on_cx_valueChanged(double d)
{
    mParams.setCx(d);
    mUi->intrError->setText(QString("invalid"));
    mParams.reprojectionError = std::numeric_limits<double>::quiet_NaN();

    emit paramsChanged(mParams);
}

void IntrinsicBox::on_cy_valueChanged(double d)
{
    mParams.setCy(d);
    mUi->intrError->setText(QString("invalid"));
    mParams.reprojectionError = std::numeric_limits<double>::quiet_NaN();

    emit paramsChanged(mParams);
}

void IntrinsicBox::on_r2_valueChanged(double d)
{
    mParams.distortionCoeffs.at<float>(0) = static_cast<float>(d);
    mUi->intrError->setText(QString("invalid"));
    mParams.reprojectionError = std::numeric_limits<double>::quiet_NaN();

    emit paramsChanged(mParams);
}

void IntrinsicBox::on_r4_valueChanged(double d)
{
    mParams.distortionCoeffs.at<float>(1) = static_cast<float>(d);
    mUi->intrError->setText(QString("invalid"));
    mParams.reprojectionError = std::numeric_limits<double>::quiet_NaN();

    emit paramsChanged(mParams);
}

void IntrinsicBox::on_r6_valueChanged(double d)
{
    mParams.distortionCoeffs.at<float>(4) = static_cast<float>(d);
    mUi->intrError->setText(QString("invalid"));
    mParams.reprojectionError = std::numeric_limits<double>::quiet_NaN();

    emit paramsChanged(mParams);
}
void IntrinsicBox::on_s1_valueChanged(double d)
{
    mParams.distortionCoeffs.at<float>(8) = static_cast<float>(d);
    mUi->intrError->setText(QString("invalid"));
    mParams.reprojectionError = std::numeric_limits<double>::quiet_NaN();

    emit paramsChanged(mParams);
}
void IntrinsicBox::on_s2_valueChanged(double d)
{
    mParams.distortionCoeffs.at<float>(9) = static_cast<float>(d);
    mUi->intrError->setText(QString("invalid"));
    mParams.reprojectionError = std::numeric_limits<double>::quiet_NaN();

    emit paramsChanged(mParams);
}
void IntrinsicBox::on_s3_valueChanged(double d)
{
    mParams.distortionCoeffs.at<float>(10) = static_cast<float>(d);
    mUi->intrError->setText(QString("invalid"));
    mParams.reprojectionError = std::numeric_limits<double>::quiet_NaN();

    emit paramsChanged(mParams);
}
void IntrinsicBox::on_s4_valueChanged(double d)
{
    mParams.distortionCoeffs.at<float>(11) = static_cast<float>(d);
    mUi->intrError->setText(QString("invalid"));
    mParams.reprojectionError = std::numeric_limits<double>::quiet_NaN();

    emit paramsChanged(mParams);
}
void IntrinsicBox::on_taux_valueChanged(double d)
{
    mParams.distortionCoeffs.at<float>(12) = static_cast<float>(d);
    mUi->intrError->setText(QString("invalid"));
    mParams.reprojectionError = std::numeric_limits<double>::quiet_NaN();

    emit paramsChanged(mParams);
}
void IntrinsicBox::on_tauy_valueChanged(double d)
{
    mParams.distortionCoeffs.at<float>(13) = static_cast<float>(d);
    mUi->intrError->setText(QString("invalid"));
    mParams.reprojectionError = std::numeric_limits<double>::quiet_NaN();

    emit paramsChanged(mParams);
}


void IntrinsicBox::on_tx_valueChanged(double d)
{
    mParams.distortionCoeffs.at<float>(2) = static_cast<float>(d);
    mUi->intrError->setText(QString("invalid"));
    mParams.reprojectionError = std::numeric_limits<double>::quiet_NaN();

    emit paramsChanged(mParams);
}

void IntrinsicBox::on_ty_valueChanged(double d)
{
    mParams.distortionCoeffs.at<float>(3) = static_cast<float>(d);
    mUi->intrError->setText(QString("invalid"));
    mParams.reprojectionError = std::numeric_limits<double>::quiet_NaN();

    emit paramsChanged(mParams);
}
void IntrinsicBox::on_k4_valueChanged(double d)
{
    mParams.distortionCoeffs.at<float>(5) = static_cast<float>(d);
    mUi->intrError->setText(QString("invalid"));
    mParams.reprojectionError = std::numeric_limits<double>::quiet_NaN();

    emit paramsChanged(mParams);
}
void IntrinsicBox::on_k5_valueChanged(double d)
{
    mParams.distortionCoeffs.at<float>(6) = static_cast<float>(d);
    mUi->intrError->setText(QString("invalid"));
    mParams.reprojectionError = std::numeric_limits<double>::quiet_NaN();

    emit paramsChanged(mParams);
}
void IntrinsicBox::on_k6_valueChanged(double d)
{
    mParams.distortionCoeffs.at<float>(7) = static_cast<float>(d);
    mUi->intrError->setText(QString("invalid"));
    mParams.reprojectionError = std::numeric_limits<double>::quiet_NaN();

    emit paramsChanged(mParams);
}
void IntrinsicBox::on_quadAspectRatio_stateChanged(int i)
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
    mParams.reprojectionError = std::numeric_limits<double>::quiet_NaN();
}

void IntrinsicBox::on_fixCenter_stateChanged(int i)
{
    if(i == Qt::Checked)
    {
        mCxNormal = mUi->cx->value();
        mCyNormal = mUi->cy->value();

        mUi->cx->setValue(mCxFixed);
        mUi->cy->setValue(mCyFixed);

        mUi->cx->setDisabled(true);
        mUi->cy->setDisabled(true);
    }
    else if(i == Qt::Unchecked)
    {
        mUi->cx->setEnabled(true);
        mUi->cy->setEnabled(true);
        mUi->cx->setValue(mCxNormal);
        mUi->cy->setValue(mCyNormal);
    }
    mUi->intrError->setText(QString("invalid"));
    mParams.reprojectionError = std::numeric_limits<double>::quiet_NaN();
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

void IntrinsicBox::runAutoCalib()
{
    auto params = mAutoCalib.autoCalib(
        mUi->quadAspectRatio->isChecked(),
        mUi->fixCenter->isChecked(),
        mUi->tangDist->isChecked(),
        mUi->extModelCheckBox->isChecked());
    if(params)
    {
        setIntrinsicCameraParams(params.value());
    }
}
void IntrinsicBox::on_calibFiles_clicked()
{
    if(mAutoCalib.openCalibFiles())
    {
        mUi->autoCalib->setEnabled(true);
    }
}

void IntrinsicBox::on_tangDist_stateChanged(int i)
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
    mParams.reprojectionError = std::numeric_limits<double>::quiet_NaN();
}

void IntrinsicBox::on_extModelCheckBox_stateChanged(int i)
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
    mParams.reprojectionError = std::numeric_limits<double>::quiet_NaN();
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
