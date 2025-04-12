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

#ifndef ALIGNMENTGRIDBOX_H
#define ALIGNMENTGRIDBOX_H

#include "vector.h"

#include <QWidget>
#include <variant>

namespace Ui
{
class AlignmentGridBox;
}
class QDomElement;

struct Grid2D
{
    Vec2F trans;
    int   angle;
    int   scale;
};

struct Grid3D
{
    Vec3F trans;
    int   resolution;
};

using Grid = std::variant<Grid2D, Grid3D>;

class AlignmentGridBox : public QWidget
{
    Q_OBJECT

public:
    explicit AlignmentGridBox(QWidget *parent = nullptr);
    ~AlignmentGridBox();

    Grid getGridParameters();
    bool isShow();
    bool isFix();

    void setTrans3DX(double x);
    void setTrans3DY(double y);
    void setResolution(double res);

    void setTrans2DX(double x);
    void setTrans2DY(double y);
    void setScale(double scale);
    void setRotation(double angle);


    void setXml(QDomElement &subSubElem);
    bool getXml(QDomElement &subSubElem);

signals:
    void gridChanged();

private slots:
    void onGridShowStateChanged();
    void onGridFixStateChanged();
    void onGridRotateValueChanged();
    void onGridTransXValueChanged();
    void onGridTransYValueChanged();
    void onGridScaleValueChanged();

    void onGridTabCurrentChanged();

    void onGrid3DTransXValueChanged();
    void onGrid3DTransYValueChanged();
    void onGrid3DTransZValueChanged();
    void onGrid3DResolutionValueChanged();

private:
    Grid2D assemble2DGrid();
    Grid3D assemble3DGrid();
    Grid   assembleGrid();

    Ui::AlignmentGridBox *mUi;
};

#endif // ALIGNMENTGRIDBOX_H
