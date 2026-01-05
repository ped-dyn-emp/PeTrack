/*
 * PeTrack - Software for tracking pedestrians movement in videos
 * Copyright (C) 2026 Forschungszentrum Jülich GmbH, IAS-7
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
    void                  showRecalibrationDialog();

signals:
    void paramsChanged(IntrinsicCameraParams newParams);

private slots:
    void onExtModelCheckBoxStateChanged();
    void onExtModelCheckBoxClicked();
    void onQuadAspectRatioStateChanged();
    void onFixCenterStateChanged();
    void onTangDistStateChanged();
    void onFxValueChanged(double d);
    void onFyValueChanged(double d);
    void onCxValueChanged(double d);
    void onCyValueChanged(double d);
    void onR2ValueChanged(double d);
    void onR4ValueChanged(double d);
    void onR6ValueChanged(double d);
    void onS1ValueChanged(double d);
    void onS2ValueChanged(double d);
    void onS3ValueChanged(double d);
    void onS4ValueChanged(double d);
    void onTauxValueChanged(double d);
    void onTauyValueChanged(double d);
    void onTxValueChanged(double d);
    void onTyValueChanged(double d);
    void onK4ValueChanged(double d);
    void onK5ValueChanged(double d);
    void onK6ValueChanged(double d);
    void onBoardSizeXValueChanged(int x);
    void onBoardSizeYValueChanged(int y);
    void onSquareSizeValueChanged(double s);
    void onCalibFilesClicked();
    void onCalibVideoClicked();
    void onIntrActiveStateChanged(int i);

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
