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

#ifndef INTRINSICBOX_H
#define INTRINSICBOX_H


#include "intrinsicCameraParams.h"

#include <QWidget>
#include <opencv2/core.hpp>

namespace Ui
{
class IntrinsicBox;
}
class CalibFilter;
class AutoCalib;
class QDomElement;
class ExtrinsicBox;


struct CalibSettings
{
    bool quadAspectRatio;
    bool fixCenter;
    bool tangDist;
};

class IntrinsicBox : public QWidget
{
    Q_OBJECT

public:
    explicit IntrinsicBox(
        QWidget              *parent,
        AutoCalib            &autoCalib,
        CalibFilter          &calibFilter,
        ExtrinsicBox         &extrBox,
        std::function<void()> updateImageCallback);
    explicit IntrinsicBox(
        QWidget              *parent,
        Ui::IntrinsicBox     *ui,
        AutoCalib            &autoCalib,
        CalibFilter          &calibFilter,
        ExtrinsicBox         &extrBox,
        std::function<void()> updateImageCallback);
    IntrinsicBox(const IntrinsicBox &)            = delete;
    IntrinsicBox(IntrinsicBox &&)                 = delete;
    IntrinsicBox &operator=(const IntrinsicBox &) = delete;
    IntrinsicBox &operator=(IntrinsicBox &&)      = delete;
    ~IntrinsicBox() override;

    IntrinsicCameraParams     getIntrinsicCameraParams() const;
    IntrinsicModelsParameters getBothIntrinsicCameraParams() const;
    void                      setIntrinsicCameraParams(const IntrinsicModelsParameters params);

    void imageSizeChanged(int width, int height, int borderDiff);
    bool getXml(QDomElement &subSubElem);
    void setXml(QDomElement &subSubElem) const;
    void setCalibSettings();

private:
    void setExtendedParametersInOldModelToZero();
    void loadCalibFiles(QDomElement &subSubElem);

    IntrinsicCameraParams getXmlParams(const QDomElement &elem);
    void                  applyCurrentModelParamsToUi();
    void                  checkModelParams(const IntrinsicCameraParams &params);
    void                  setCurrentIntrinsicCameraParameters(IntrinsicCameraParams params);

signals:
    void paramsChanged(IntrinsicCameraParams newParams);

private slots:
    void showRecalibrationDialog();
    void on_extModelCheckBox_stateChanged(int);
    void on_extModelCheckBox_clicked(bool);
    void on_quadAspectRatio_stateChanged(int);
    void on_fixCenter_stateChanged(int);
    void on_tangDist_stateChanged(int);
    void on_fx_valueChanged(double d);
    void on_fy_valueChanged(double d);
    void on_cx_valueChanged(double d);
    void on_cy_valueChanged(double d);
    void on_r2_valueChanged(double d);
    void on_r4_valueChanged(double d);
    void on_r6_valueChanged(double d);
    void on_s1_valueChanged(double d);
    void on_s2_valueChanged(double d);
    void on_s3_valueChanged(double d);
    void on_s4_valueChanged(double d);
    void on_taux_valueChanged(double d);
    void on_tauy_valueChanged(double d);
    void on_tx_valueChanged(double d);
    void on_ty_valueChanged(double d);
    void on_k4_valueChanged(double d);
    void on_k5_valueChanged(double d);
    void on_k6_valueChanged(double d);
    void on_boardSizeX_valueChanged(int x);
    void on_boardSizeY_valueChanged(int y);
    void on_squareSize_valueChanged(double s);
    void on_calibFiles_clicked();
    void on_apply_stateChanged(int i);

public slots:
    void runAutoCalib();


private:
    Ui::IntrinsicBox         *mUi;
    CalibSettings             mCalibSettings;
    AutoCalib                &mAutoCalib;
    CalibFilter              &mCalibFilter;
    ExtrinsicBox             &mExtrBox;
    std::function<void()>     mUpdateImageCallback;
    IntrinsicModelsParameters mModelsParams;

    double mCxFixed = 0;
    double mCyFixed = 0;
};

#endif // INTRINSICBOX_H
