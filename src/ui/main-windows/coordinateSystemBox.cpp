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

#include "coordinateSystemBox.h"

#include "coordItem.h"
#include "extrCalibration.h"
#include "extrinsicBox.h"
#include "imageItem.h"
#include "importHelper.h"
#include "intrinsicBox.h"
#include "pGroupBox.h"
#include "ui_coordinateSystemBox.h"

#include <opencv2/core/base.hpp>

CoordinateSystemBox::CoordinateSystemBox(
    QWidget               *parent,
    std::function<void()>  updateStatusPos,
    std::function<void()>  updateHeadSize,
    std::function<int()>   getBorderSizeCallback,
    const IntrinsicBox    &intrinsicBox,
    ExtrinsicBox          &extrinsicBox,
    const ImageItem       &imageItem,
    const ExtrCalibration &extrCalib) :
    CoordinateSystemBox(
        parent,
        new Ui::CoordinateSystemBox,
        updateStatusPos,
        updateHeadSize,
        getBorderSizeCallback,
        intrinsicBox,
        extrinsicBox,
        imageItem,
        extrCalib)
{
}

CoordinateSystemBox::CoordinateSystemBox(
    QWidget                 *parent,
    Ui::CoordinateSystemBox *ui,
    std::function<void()>    updateStatusPos,
    std::function<void()>    updateHeadSize,
    std::function<int()>     getBorderSizeCallback,
    const IntrinsicBox      &intrinsicBox,
    ExtrinsicBox            &extrinsicBox,
    const ImageItem         &imageItem,
    const ExtrCalibration   &extrCalib) :
    QWidget(parent),
    mUi(ui),
    mUpdateStatusPos(std::move(updateStatusPos)),
    mUpdateHeadSize(std::move(updateHeadSize)),
    mGetBorderSize(getBorderSizeCallback),
    mIntr(intrinsicBox),
    mExtrBox(extrinsicBox),
    mImageItem(imageItem),
    mExtrCalib(extrCalib)
{
    mUi->setupUi(this);
    setFocusProxy(mUi->coordShow);

    mPose3D.swap.x = mUi->coord3DSwapX->isChecked();
    mPose3D.swap.y = mUi->coord3DSwapY->isChecked();
    mPose3D.swap.z = mUi->coord3DSwapZ->isChecked();

    mPose3D.position[0] = mUi->coord3DTransX->value();
    mPose3D.position[1] = mUi->coord3DTransY->value();
    mPose3D.position[2] = mUi->coord3DTransZ->value();

    mPose2D.angle       = mUi->coordRotate->value();
    mPose2D.scale       = mUi->coordScale->value();
    mPose2D.unit        = mUi->coordUnit->value();
    mPose2D.position[0] = mUi->coordTransX->value();
    mPose2D.position[1] = mUi->coordTransY->value();

    connect(mUi->coordTab, &QTabWidget::currentChanged, this, &CoordinateSystemBox::onCoordTabCurrentChanged);
    connect(mUi->coordShow, &QCheckBox::checkStateChanged, this, &CoordinateSystemBox::onCoordShowStateChanged);
    connect(mUi->coordFix, &QCheckBox::checkStateChanged, this, &CoordinateSystemBox::onCoordFixStateChanged);

    // 2D coordinates
    connect(mUi->coordRotate, &PSlider::valueChanged, this, &CoordinateSystemBox::onCoordRotateValueChanged);
    connect(mUi->coordTransX, &PSlider::valueChanged, this, &CoordinateSystemBox::onCoordTransXValueChanged);
    connect(mUi->coordTransY, &PSlider::valueChanged, this, &CoordinateSystemBox::onCoordTransYValueChanged);
    connect(mUi->coordScale, &PSlider::valueChanged, this, &CoordinateSystemBox::onCoordScaleValueChanged);
    connect(
        mUi->coordAltitude,
        QOverload<double>::of(&PDoubleSpinBox::valueChanged),
        this,
        &CoordinateSystemBox::onCoordAltitudeValueChanged);
    connect(
        mUi->coordUnit,
        QOverload<double>::of(&PDoubleSpinBox::valueChanged),
        this,
        &CoordinateSystemBox::onCoordUnitValueChanged);
    connect(
        mUi->coordUseIntrinsic,
        &QCheckBox::checkStateChanged,
        this,
        &CoordinateSystemBox::onCoordUseIntrinsicStateChanged);

    // 3D coordinates
    connect(mUi->coord3DTransX, &PSlider::valueChanged, this, &CoordinateSystemBox::onCoord3DTransXValueChanged);
    connect(mUi->coord3DTransY, &PSlider::valueChanged, this, &CoordinateSystemBox::onCoord3DTransYValueChanged);
    connect(mUi->coord3DTransZ, &PSlider::valueChanged, this, &CoordinateSystemBox::onCoord3DTransZValueChanged);
    connect(mUi->coord3DAxeLen, &PSlider::valueChanged, this, &CoordinateSystemBox::onCoord3DAxeLenValueChanged);

    connect(mUi->coord3DSwapX, &QCheckBox::checkStateChanged, this, &CoordinateSystemBox::onCoord3DSwapXStateChanged);
    connect(mUi->coord3DSwapY, &QCheckBox::checkStateChanged, this, &CoordinateSystemBox::onCoord3DSwapYStateChanged);
    connect(mUi->coord3DSwapZ, &QCheckBox::checkStateChanged, this, &CoordinateSystemBox::onCoord3DSwapZStateChanged);

    // Extrinsic calibration display
    connect(
        mUi->extCalibPointsShow,
        &QCheckBox::checkStateChanged,
        this,
        &CoordinateSystemBox::onExtCalibPointsShowStateChanged);
    connect(
        mUi->extVanishPointsShow,
        &QCheckBox::checkStateChanged,
        this,
        &CoordinateSystemBox::onExtVanishPointsShowStateChanged);
}

CoordinateSystemBox::~CoordinateSystemBox()
{
    delete mUi;
}

void CoordinateSystemBox::setCoordTrans2DMinMax(const Vec2F &min, const Vec2F &max)
{
    mUi->coordTransX->setMinimum(static_cast<int>(min.x()));
    mUi->coordTransX_spin->setMinimum(static_cast<int>(min.x()));
    mUi->coordTransY->setMinimum(static_cast<int>(min.y()));
    mUi->coordTransY_spin->setMinimum(static_cast<int>(min.y()));

    mUi->coordTransX->setMaximum(static_cast<int>(max.x()));
    mUi->coordTransX_spin->setMaximum(static_cast<int>(max.x()));
    mUi->coordTransY->setMaximum(static_cast<int>(max.y()));
    mUi->coordTransY_spin->setMaximum(static_cast<int>(max.y()));
}

std::pair<Vec2F, Vec2F> CoordinateSystemBox::getCoordTrans2DMinMax() const
{
    Vec2F min(mUi->coordTransX->minimum(), mUi->coordTransY->minimum());
    Vec2F max(mUi->coordTransX->maximum(), mUi->coordTransY->maximum());
    return {min, max};
}

void CoordinateSystemBox::setCoordPose2D(const CoordPose2D &pose)
{
    mUi->coordTransX->setValue(pose.position.x());
    mUi->coordTransY->setValue(pose.position.y());
    mUi->coordScale->setValue(pose.scale);
    mUi->coordUnit->setValue(pose.unit);
    mUi->coordRotate->setValue(pose.angle);
}

double CoordinateSystemBox::getCameraAltitude() const
{
    return mUi->coordAltitude->value();
}

void CoordinateSystemBox::setCoordTrans2D(Vec2F trans)
{
    mUi->coordTransX->setValue(trans.x());
    mUi->coordTransY->setValue(trans.y());
}

bool CoordinateSystemBox::isCoordUseIntrinsicChecked() const
{
    return mUi->coordUseIntrinsic->isChecked();
}

void CoordinateSystemBox::setCoordTrans3D(Vec3F trans)
{
    mUi->coord3DTransX->setValue(trans.x());
    mUi->coord3DTransY->setValue(trans.y());
    mUi->coord3DTransZ->setValue(trans.z());
}

int CoordinateSystemBox::getCoord3DAxeLen() const
{
    return mUi->coord3DAxeLen->value();
}

void CoordinateSystemBox::setCoord3DAxeLen(int newLength)
{
    mUi->coord3DAxeLen->setValue(newLength);
}

int CoordinateSystemBox::getCalibCoordDimension() const
{
    return mUi->coordTab->currentIndex();
}

bool CoordinateSystemBox::getCalibExtrCalibPointsShow() const
{
    return mUi->extCalibPointsShow->isChecked();
}

bool CoordinateSystemBox::getCalibExtrVanishPointsShow() const
{
    return mUi->extVanishPointsShow->isChecked();
}

bool CoordinateSystemBox::getCalibCoordShow() const
{
    return mUi->coordShow->isChecked();
}

bool CoordinateSystemBox::getCalibCoordFix() const
{
    return mUi->coordFix->isChecked();
}

int CoordinateSystemBox::getCalibCoordRotate() const
{
    return mUi->coordRotate->value();
}

void CoordinateSystemBox::setCalibCoordRotate(int newVal)
{
    mUi->coordRotate->setValue(newVal);
}

// in x und y richtung identisch, da vorher intrinsische kamerakalibrierung geschehen ist
double CoordinateSystemBox::getCmPerPixel() const
{
    // das sollte nur einmal berechne werden, wenn einfliessende daten sich aendern
    auto       boderTransform = QTransform::fromTranslate(-mGetBorderSize(), -mGetBorderSize());
    auto       imgToWorld     = boderTransform * mCoordTransform.inverted();
    QPointF    p1             = imgToWorld.map(QPointF(0, 0));
    QPointF    p2             = imgToWorld.map(QPointF(1, 0));
    const auto coordUnit      = mUi->coordUnit->value();
    return coordUnit * sqrt(pow(p1.x() - p2.x(), 2) + pow(p1.y() - p2.y(), 2)) / 100.;
    // durch 100., da coordsys so gezeichnet, dass 1 bei 100 liegt
}

// Liefert zum Pixelpunkt (px,py) die Anzahl der Zentimeter in x- und y-Richtung
QPointF CoordinateSystemBox::getCmPerPixel(float px, float py, float h) const
{
    // ToDo:
    // 3D Punkte an (px-0.5, py) und (px+0.5, py) berechnen und Auswirkung in x-Richtung
    // und          (px, py-0.5) und (px, py+0.5) berechnen und Auswirkung in y-Richtung untersuchen
    //
    // Unterscheiden nach x- und y-Richtung?
    // Wie fliesst die Hoehe mit ein?

    cv::Point3f p3x1 = mExtrCalib.get3DPoint(cv::Point2f(px - 0.5, py), h);
    cv::Point3f p3x2 = mExtrCalib.get3DPoint(cv::Point2f(px + 0.5, py), h);

    cv::Point3f p3y1 = mExtrCalib.get3DPoint(cv::Point2f(px, py - 0.5), h);
    cv::Point3f p3y2 = mExtrCalib.get3DPoint(cv::Point2f(px, py + 0.5), h);

    double x_dir = norm(p3x1 - p3x2);
    double y_dir = norm(p3y1 - p3y2);

    QPointF res(x_dir, y_dir);

    return res;
}

///*
/// Liefert den Winkel zwischen der Geraden von der Kamera
/// zum uebergebenen Punkt mit der Hoehe height
/// zur Grundflaeche [0-90] 90 => senkrecht unter der Kamera
/// Punktkoordinaten beinhalten die Border
///*
double CoordinateSystemBox::getAngleToGround(float px, float py, float height) const
{
    const auto &extrParams = mExtrBox.getExtrinsicParameters();
    cv::Point3f cam(
        -mUi->coord3DTransX->value() - extrParams.trans1,
        -mUi->coord3DTransY->value() - extrParams.trans2,
        -mUi->coord3DTransZ->value() - extrParams.trans3);

    cv::Point3f posInImage = mExtrCalib.get3DPoint(cv::Point2f(px - mGetBorderSize(), py - mGetBorderSize()), height);

    cv::Point3f a(cam.x - posInImage.x, cam.y - posInImage.y, cam.z - posInImage.z), b(0, 0, 1);

    // NOTE: in C++20 can be replaced with numbers import
    const double pi = std::atan(1.0) * 4;
    return asin(
               (a.x * b.x + a.y * b.y + a.z * b.z) / (abs(sqrt(pow(a.x, 2) + pow(a.y, 2) + pow(a.z, 2))) *
                                                      abs(sqrt(pow(b.x, 2) + pow(b.y, 2) + pow(b.z, 2))))) *
           180 / pi;
}

QPointF CoordinateSystemBox::getPosImage(QPointF pos, float height) const
{
    auto        imageSize = mImageItem.boundingRect();
    cv::Point2f p2d;
    if(imageSize != QRectF{0, 0, 0, 0})
    {
        if(getCalibCoordDimension() == 0) // Tab coordinate system is on 3D
        {
            p2d = mExtrCalib.getImagePoint(cv::Point3f(pos.x(), pos.y(), height));
            pos.setX(p2d.x);
            pos.setY(p2d.y);
        }
        else
        {
            //////////////

            // Fehlerhaft funktioniert nicht wie gewollt

            //////////////
            // Old 2D mapping of Pixelpoints to RealPositions
            pos.setY(-pos.y());
            pos /= mUi->coordUnit->value() / 100.; // durch 100., da coordsys so gezeichnet, dass 1 bei 100 liegt

            auto borderTransforms = QTransform::fromTranslate(-mGetBorderSize(), -mGetBorderSize());
            auto imgToWorld       = borderTransforms * mCoordTransform.inverted();
            pos                   = imgToWorld.inverted().map(pos);
            // mImageItem.mapFromItem(&mCoordItem, pos); // Einheit anpassen...
            if(isCoordUseIntrinsicChecked())
            {
                const auto camMat = mIntr.getIntrinsicCameraParams();
                pos.rx() -= camMat.getCx();
                pos.ry() -= camMat.getCy();
            }
            else
            {
                pos.rx() -= imageSize.width() / 2. - .5;  // Bildmitte
                pos.ry() -= imageSize.height() / 2. - .5; // Bildmitte
            }
            const auto altitude = mUi->coordAltitude->value();
            pos                 = (altitude / (altitude - height)) * pos;


            if(isCoordUseIntrinsicChecked())
            {
                const auto camMat = mIntr.getIntrinsicCameraParams();
                pos.rx() += camMat.getCx();
                pos.ry() += camMat.getCy();
            }
            else
            {
                pos.rx() += imageSize.width() / 2. - .5;  // Bildmitte
                pos.ry() += imageSize.height() / 2. - .5; // Bildmitte
            }
        }
    }
    return pos;
}

// eingabe pos als pixelkoordinate des bildes
// result in cm (mit y-Achse nach oben gerichtet)
// default height = 0. in Headerdatei (in cm)
// wenn kein Bild vorliegt, wird eingabeposition durchgereicht - kommt nicht vor, da kein mouseevent
QPointF CoordinateSystemBox::getPosReal(QPointF pos, double height) const
{
    auto imageSize = mImageItem.boundingRect();

    if(imageSize != QRectF{0, 0, 0, 0})
    {
        int bS = mGetBorderSize();

        // Switch between 2D and 3D CameraCalibration/Position calculation
        if(getCalibCoordDimension() == 0)
        {
            // New 3D mapping of Pixelpoints to RealPositions
            cv::Point3f p3d = mExtrCalib.get3DPoint(cv::Point2f(pos.x() - bS, pos.y() - bS), height);

            // ToDo: Getting the floor point of the Person! (Only the x/y-coordinates?)
            pos = QPointF(p3d.x, p3d.y);
        }
        else
        {
            //  statt mControlWidget->getCalibFx() muesste spaeter wert stehen, der im verzerrten Bild fX=fY angibt
            //  a = mControlWidget->getCalibFx()*getMeterPerPixel();
            //  a = mControlWidget->getCameraAltitude();
            //  -.5 da pixel von 0..1023 (in skala bis 1024 anfaengt) laufen
            if(isCoordUseIntrinsicChecked())
            {
                const auto camMat = mIntr.getIntrinsicCameraParams();
                pos.rx() -= camMat.getCx();
                pos.ry() -= camMat.getCy();
            }
            else
            {
                pos.rx() -= imageSize.width() / 2. - .5;  // Bildmitte
                pos.ry() -= imageSize.height() / 2. - .5; // Bildmitte
            }

            const auto altitude = mUi->coordAltitude->value();
            pos                 = ((altitude - height) / altitude) * pos; //((a-height)/a)*pos;

            if(isCoordUseIntrinsicChecked())
            {
                const auto camMat = mIntr.getIntrinsicCameraParams();
                pos.rx() += camMat.getCx();
                pos.ry() += camMat.getCy();
            }
            else
            {
                pos.rx() += imageSize.width() / 2. - .5;  // Bildmitte
                pos.ry() += imageSize.height() / 2. - .5; // Bildmitte
            }

            // Old 2D mapping of Pixelpoints to RealPositions
            auto borderTransform = QTransform::fromTranslate(-mGetBorderSize(), -mGetBorderSize());
            auto imgToWorld      = borderTransform * mCoordTransform.inverted();
            pos                  = imgToWorld.map(pos);
            pos *= mUi->coordUnit->value() / 100.; // durch 100., da coordsys so gezeichnet, dass 1 bei 100 liegt
            pos.setY(-pos.y());
        }
    }
    return pos;
}

bool CoordinateSystemBox::getXml(const QDomElement &subSubElem)
{
    if(subSubElem.tagName() == "EXTRINSIC_PARAMETERS")
    {
        loadBoolValue(subSubElem, "SHOW_CALIB_POINTS", mUi->extCalibPointsShow, false);
        loadActiveIndex(subSubElem, "COORD_DIMENSION", mUi->coordTab, 0);
        loadBoolValue(subSubElem, "SHOW", mUi->coordShow, false);
        loadBoolValue(subSubElem, "FIX", mUi->coordFix, false);
        loadIntValue(subSubElem, "ROTATE", mUi->coordRotate, 0);


        int trans_x = readInt(subSubElem, "TRANS_X", 0);
        if(trans_x > mUi->coordTransX->maximum())
        {
            setCalibCoordTransXMax(trans_x);
        }
        mUi->coordTransX->setValue(trans_x);

        int trans_y = readInt(subSubElem, "TRANS_Y", 0);
        if(trans_y > mUi->coord3DTransY->maximum())
        {
            setCalibCoordTransYMax(trans_y);
        }
        loadIntValue(subSubElem, "TRANS_Y", mUi->coordTransY, 0);
        loadIntValue(subSubElem, "SCALE", mUi->coordScale, 100);
        loadDoubleValue(subSubElem, "ALTITUDE", mUi->coordAltitude, 535);
        loadDoubleValue(subSubElem, "UNIT", mUi->coordUnit, 100);
        loadBoolValue(subSubElem, "USE_INTRINSIC_CENTER", mUi->coordUseIntrinsic, false);
        loadIntValue(subSubElem, "COORD3D_TRANS_X", mUi->coord3DTransX, 0);
        loadIntValue(subSubElem, "COORD3D_TRANS_Y", mUi->coord3DTransY, 0);
        loadIntValue(subSubElem, "COORD3D_TRANS_Z", mUi->coord3DTransZ, 0);
        loadIntValue(subSubElem, "COORD3D_AXIS_LEN", mUi->coord3DAxeLen, 200);
        loadBoolValue(subSubElem, "COORD3D_SWAP_X", mUi->coord3DSwapX, false);
        loadBoolValue(subSubElem, "COORD3D_SWAP_Y", mUi->coord3DSwapY, false);
        loadBoolValue(subSubElem, "COORD3D_SWAP_Z", mUi->coord3DSwapZ, false);

        if(subSubElem.hasAttribute("IMMUTABLE_COORD_BOX"))
        {
            if(this->parent())
            {
                auto *parent = dynamic_cast<PGroupBox *>(this->parent()->parent());
                if(parent)
                {
                    parent->setImmutable(readBool(subSubElem, "IMMUTABLE_COORD_BOX", false));
                }
            }
        }


        return true;
    }
    return false;
}

void CoordinateSystemBox::setXml(QDomElement &subSubElem) const
{
    subSubElem.setAttribute("SHOW_CALIB_POINTS", mUi->extCalibPointsShow->isChecked());

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

    if(this->parent())
    {
        auto *parent = dynamic_cast<PGroupBox *>(this->parent()->parent());
        if(parent)
        {
            subSubElem.setAttribute("IMMUTABLE_COORD_BOX", parent->isImmutable());
        }
    }
}

void CoordinateSystemBox::setCalibCoordTransXMax(int max)
{
    mUi->coordTransX->setMaximum(max);
    mUi->coordTransX_spin->setMaximum(max);
}

void CoordinateSystemBox::setCalibCoordTransYMax(int max)
{
    mUi->coordTransY->setMaximum(max);
    mUi->coordTransY_spin->setMaximum(max);
}

// may be called from other classes if sth. changed such that ExtrCalib has different results
void CoordinateSystemBox::updateCoordItem()
{
    emit coordDataChanged();
}


CoordItemState CoordinateSystemBox::getCoordItemState()
{
    CoordItemState state;
    state.isMovable = !mUi->coordFix->isChecked();


    state.coordDimension = getCalibCoordDimension();
    if(getCalibCoordDimension() == 1) // 2D
    {
        double sc = mPose2D.scale / 10.;
        double tX = mPose2D.position.x() / 10.;
        double tY = mPose2D.position.y() / 10.;
        double ro = mPose2D.angle / 10.;

        // aktualisierung der transformationsmatrix
        QTransform matrix;
        // matrix wird nur bei aenderungen neu bestimmt
        matrix.translate(tX, tY);
        matrix.rotate(ro);
        matrix.scale(sc / 100., sc / 100.);
        state.matrix = matrix;
    }
    else // 3D
    {
        ////////////////////////////////////////
        //     3D World-Coordinate-System     //
        ////////////////////////////////////////

        auto imageSize = mImageItem.boundingRect();
        if(imageSize != QRectF{0, 0, 0, 0})
        {
            // Reset Matrix - No Matrix Transformations for 3D Coordsystem
            // aktualisierung der transformationsmatrix
            QTransform matrix;
            // matrix wird nur bei aenderungen neu bestimmt
            matrix.translate(0, 0);
            matrix.rotate(0);
            matrix.scale(1, 1);
            state.matrix = matrix;

            const double axeLen = mUi->coord3DAxeLen->value();
            const int    bS     = mGetBorderSize();

            // Coordinate-system origin at (tX,tY,tZ)
            if(mExtrCalib.isSetExtrCalib())
            {
                auto ursprung = mExtrCalib.getImagePoint(cv::Point3f(0, 0, 0));

                auto x3D = cv::Point3f(axeLen, 0, 0);
                auto y3D = cv::Point3f(0, axeLen, 0);
                auto z3D = cv::Point3f(0, 0, axeLen);

                // Tests if the origin-point of the coordinate-system is outside the image
                if(mExtrCalib.isOutsideImage(ursprung))
                {
                    mCoordTransform = state.matrix;
                    return state;
                }
                x3D.x++;
                y3D.y++;
                z3D.z++;

                cv::Point2f x;
                cv::Point2f y;
                cv::Point2f z;
                auto        imgSize = mImageItem.boundingRect();
                // Kuerzt die Koordinaten-Achsen, falls sie aus dem angezeigten Bild raus laufen wuerden
                do
                {
                    x3D.x--;
                    x = mExtrCalib.getImagePoint(x3D);
                    // tests if the coord system axis are inside the view or outside, if outside short them till they
                    // are inside the image
                } while(x.x < -bS || x.x > imgSize.width() - bS || x.y < -bS || x.y > imgSize.height() - bS);
                state.x3D = x3D;
                do
                {
                    y3D.y--;
                    y = mExtrCalib.getImagePoint(y3D);
                } while(y.x < -bS || y.x > imgSize.width() - bS || y.y < -bS || y.y > imgSize.height() - bS);
                state.y3D = y3D;
                do
                {
                    z3D.z--;
                    z = mExtrCalib.getImagePoint(z3D);
                } while(z.x < -bS || z.x > imgSize.width() - bS || z.y < -bS || z.y > imgSize.height() - bS);
                state.z3D = z3D;
            }
        }
    }

    mCoordTransform = state.matrix;
    return state;
}

void CoordinateSystemBox::onCoordTabCurrentChanged(int index)
{
    if(index == 1)
    {
        mExtrBox.setEnabledExtrParams(false);
    }
    else
    {
        mExtrBox.setEnabledExtrParams(true);
    }
    updateCoordItem();
}

void CoordinateSystemBox::onCoordShowStateChanged()
{
    if(!mUi->coordShow->isChecked())
    {
        mUi->coordFix->setChecked(true);
    }
    updateCoordItem();
    setMeasuredAltitude(); // measured isn't updated, when scale is moved and show deactivated
                           // and would have lead to a false value in the coord system when activating
}

void CoordinateSystemBox::onCoordFixStateChanged()
{
    updateCoordItem();
}

void CoordinateSystemBox::onCoordRotateValueChanged(int newAngle)
{
    mPose2D.angle = newAngle;
    updateCoordItem();
}

void CoordinateSystemBox::onCoordTransXValueChanged(int newX)
{
    mPose2D.position[0] = newX;
    updateCoordItem();
}

void CoordinateSystemBox::onCoordTransYValueChanged(int newY)
{
    mPose2D.position[1] = newY;
    updateCoordItem();
}

void CoordinateSystemBox::onCoordScaleValueChanged(int newScale)
{
    mPose2D.scale = newScale;
    setMeasuredAltitude();
    mUpdateHeadSize();
    updateCoordItem();
}

void CoordinateSystemBox::onCoordAltitudeValueChanged()
{
    mUpdateHeadSize();
    updateCoordItem();
}

void CoordinateSystemBox::onCoordUnitValueChanged(double newUnit)
{
    mPose2D.unit = newUnit;
    setMeasuredAltitude();
    mUpdateHeadSize();
    updateCoordItem();
}

void CoordinateSystemBox::onCoordUseIntrinsicStateChanged()
{
    mUpdateStatusPos();
}

void CoordinateSystemBox::onCoord3DTransXValueChanged(int newX)
{
    mPose3D.position[0] = newX;
    updateCoordItem();
}

void CoordinateSystemBox::onCoord3DTransYValueChanged(int newY)
{
    mPose3D.position[1] = newY;
    updateCoordItem();
}

void CoordinateSystemBox::onCoord3DTransZValueChanged(int newZ)
{
    mPose3D.position[2] = newZ;
    updateCoordItem();
}

void CoordinateSystemBox::onCoord3DAxeLenValueChanged()
{
    updateCoordItem();
}

void CoordinateSystemBox::onCoord3DSwapXStateChanged(int newSwap)
{
    mPose3D.swap.x = newSwap == Qt::Checked;
    updateCoordItem();
}

void CoordinateSystemBox::onCoord3DSwapYStateChanged(int newSwap)
{
    mPose3D.swap.y = newSwap == Qt::Checked;
    updateCoordItem();
}

void CoordinateSystemBox::onCoord3DSwapZStateChanged(int newSwap)
{
    mPose3D.swap.z = newSwap == Qt::Checked;
    updateCoordItem();
}

void CoordinateSystemBox::setMeasuredAltitude()
{
    auto       camMat = mIntr.getIntrinsicCameraParams();
    const auto fx     = camMat.getFx();
    const auto fy     = camMat.getFy();
    mUi->coordAltitudeMeasured->setText(QString("(measured: %1)").arg((fx + fy) / 2. * getCmPerPixel(), 6, 'f', 1));
}

void CoordinateSystemBox::onExtCalibPointsShowStateChanged()
{
    updateCoordItem();
}


void CoordinateSystemBox::onExtVanishPointsShowStateChanged()
{
    updateCoordItem();
}
