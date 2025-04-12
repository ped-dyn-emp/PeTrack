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

#ifndef EXTRINSICBOX_H
#define EXTRINSICBOX_H

#include "extrinsicParameters.h"

#include <QWidget>

namespace Ui
{
class extr;
}
class QDomElement;
class ExtrCalibration;


class ExtrinsicBox : public QWidget
{
    Q_OBJECT

public:
    explicit ExtrinsicBox(QWidget *parent, Ui::extr *ui, ExtrCalibration &extrCalib);
    explicit ExtrinsicBox(QWidget *parent, ExtrCalibration &extrCalib);
    ExtrinsicBox(const ExtrinsicBox &other)            = delete;
    ExtrinsicBox(ExtrinsicBox &&other)                 = delete;
    ExtrinsicBox &operator=(const ExtrinsicBox &other) = delete;
    ExtrinsicBox &operator=(ExtrinsicBox &&other)      = delete;
    ~ExtrinsicBox() override;

    void                       setEnabledExtrParams(bool enable);
    const ExtrinsicParameters &getExtrinsicParameters() const;
    void                       setExtrinsicParameters(const ExtrinsicParameters &params);

    bool getXml(QDomElement &subSubElem);
    void setXml(QDomElement &subElem) const;

    void loadCalibFile();

signals:
    void enabledChanged(bool enabled);
    void extrinsicChanged();

private slots:

    void onExtrCalibCalcClicked();
    void onExtrCalibFetchClicked();
    void onCoordLoad3DCalibPointsClicked();

    void onRot1ValueChanged(double newVal);
    void onRot2ValueChanged(double newVal);
    void onRot3ValueChanged(double newVal);
    void onTrans1ValueChanged(double newVal);
    void onTrans2ValueChanged(double newVal);
    void onTrans3ValueChanged(double newVal);

    void onExtrCalibSaveClicked();
    void onExtrCalibShowPointsClicked();
    void onExtrCalibShowErrorClicked();

private:
    Ui::extr           *mUi;
    ExtrCalibration    &mExtrCalibration;
    ExtrinsicParameters mParams;
};

#endif // EXTRINSICBOX_H
