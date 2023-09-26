/*
 * PeTrack - Software for tracking pedestrians movement in videos
 * Copyright (C) 2023 Forschungszentrum Jülich GmbH, IAS-7
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

#ifndef COORDINATESYSTEMBOX_H
#define COORDINATESYSTEMBOX_H

#include "coordinateStructs.h"
#include "vector.h"
#include "worldImageCorrespondence.h"

#include <QRectF>
#include <QTransform>
#include <QWidget>
#include <opencv2/core/base.hpp>

namespace Ui
{
class CoordinateSystemBox;
}
class IntrinsicBox;
class ExtrinsicBox;
class ImageItem;
class CoordItem;
class ExtrCalibration;
class QDomElement;

struct CoordItemState
{
    bool        isMovable;
    QTransform  matrix;
    int         coordDimension;
    cv::Point3f x3D;
    cv::Point3f y3D;
    cv::Point3f z3D;
};

class CoordinateSystemBox : public QWidget, public WorldImageCorrespondence
{
    Q_OBJECT

public:
    explicit CoordinateSystemBox(
        QWidget               *parent,
        std::function<void()>  updateStatusPos,
        std::function<void()>  updateHeadSize,
        std::function<int()>   getBorderSizeCallback,
        const IntrinsicBox    &intrinsicBox,
        ExtrinsicBox          &extrinsicBox,
        const ImageItem       &imageItem,
        const ExtrCalibration &extrCalib);
    explicit CoordinateSystemBox(
        QWidget                 *parent,
        Ui::CoordinateSystemBox *ui,
        std::function<void()>    updateStatusPos,
        std::function<void()>    updateHeadSize,
        std::function<int()>     getBorderSizeCallback,
        const IntrinsicBox      &intrinsicBox,
        ExtrinsicBox            &extrinsicBox,
        const ImageItem         &imageItem,
        const ExtrCalibration   &extrCalib);
    CoordinateSystemBox(const CoordinateSystemBox &)            = delete;
    CoordinateSystemBox(CoordinateSystemBox &&)                 = delete;
    CoordinateSystemBox &operator=(const CoordinateSystemBox &) = delete;
    CoordinateSystemBox &operator=(CoordinateSystemBox &&)      = delete;
    ~CoordinateSystemBox() override;

    const CoordPose2D      &getCoordPose2D() const { return mPose2D; }
    Vec2F                   getCoordTrans2D() const { return mPose2D.position; }
    void                    setCoordTrans2D(Vec2F trans);
    std::pair<Vec2F, Vec2F> getCoordTrans2DMinMax() const;
    void                    setCoordTrans2DMinMax(const Vec2F &min, const Vec2F &max);
    void                    setCoordPose2D(const CoordPose2D &pose);
    double                  getCameraAltitude() const;
    bool                    isCoordUseIntrinsicChecked() const;

    Vec3F    getCoordTrans3D() const { return mPose3D.position; }
    void     setCoordTrans3D(Vec3F trans);
    SwapAxis getSwap3D() const { return mPose3D.swap; }
    int      getCoord3DAxeLen() const;
    void     setCoord3DAxeLen(int newLength);

    int  getCalibCoordDimension() const;
    bool getCalibExtrCalibPointsShow() const;
    bool getCalibExtrVanishPointsShow() const;
    bool getCalibCoordShow() const;
    bool getCalibCoordFix() const;
    int  getCalibCoordRotate() const;
    void setCalibCoordRotate(int newVal);
    void setMeasuredAltitude();

    // WorldImageCorrespondence Interface
    double  getCmPerPixel() const override;
    QPointF getCmPerPixel(float px, float py, float h = 0.) const override;
    double  getAngleToGround(float px, float py, float h = 0) const override;
    QPointF getPosImage(QPointF pos, float height = 0.) const override;
    QPointF getPosReal(QPointF pos, double height = 0.) const override;

    bool getXml(const QDomElement &subSubElem);
    void setXml(QDomElement &subSubElem) const;

    CoordItemState getCoordItemState();

private:
    void setCalibCoordTransXMax(int max);
    void setCalibCoordTransYMax(int max);

public slots:
    void updateCoordItem();

private slots:
    // selection
    void on_coordTab_currentChanged(int index);
    void on_coordShow_stateChanged(int i);
    void on_coordFix_stateChanged(int i);

    // 2D
    void on_coordRotate_valueChanged(int newAngle);
    void on_coordTransX_valueChanged(int newX);
    void on_coordTransY_valueChanged(int newY);
    void on_coordScale_valueChanged(int newScale);
    void on_coordAltitude_valueChanged(double d);
    void on_coordUnit_valueChanged(double newUnit);
    void on_coordUseIntrinsic_stateChanged(int i);


    // 3D
    void on_coord3DTransX_valueChanged(int newX);
    void on_coord3DTransY_valueChanged(int newY);
    void on_coord3DTransZ_valueChanged(int newZ);
    void on_coord3DAxeLen_valueChanged(int value);

    void on_coord3DSwapX_stateChanged(int newSwap);
    void on_coord3DSwapY_stateChanged(int newSwap);
    void on_coord3DSwapZ_stateChanged(int newSwap);

    // display of extrinsic calibration
    void on_extCalibPointsShow_stateChanged(int arg1);
    void on_extVanishPointsShow_stateChanged(int arg1);

signals:
    void coordDataChanged();

private:
    Ui::CoordinateSystemBox *mUi;
    std::function<void()>    mUpdateStatusPos;
    std::function<void()>    mUpdateHeadSize;
    std::function<int()>     mGetBorderSize;
    const IntrinsicBox      &mIntr;
    ExtrinsicBox            &mExtrBox;
    const ImageItem         &mImageItem;
    const ExtrCalibration   &mExtrCalib;
    QTransform               mCoordTransform;
    CoordPose2D              mPose2D;
    CoordPose3D              mPose3D;
};

#endif // COORDINATESYSTEMBOX_H
