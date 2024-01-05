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
#ifndef MOCAPCONTROLLER_H
#define MOCAPCONTROLLER_H

#include "extrCalibration.h"
#include "moCapPersonMetadata.h"

#include <QColor>
#include <QLine>
#include <QObject>
#include <vector>

struct SkeletonLine;
class QDomElement;

struct SegmentRenderData
{
public:
    QLine  mLine;
    QColor mColor;
    int    mThickness;
    bool   mDirected;
};

bool operator==(const SegmentRenderData &lhs, const SegmentRenderData &rhs);

/**
 * @brief The MoCapController class controls the visualisation of MoCap data.
 *
 * This class coordinates the visualization of motion capturing data. It saves the
 * different parameters of the visualization and transforms the data in MoCapStorage
 * according to this parameters to generate the render data for MoCapItem.
 */
class MoCapController : public QObject
{
    Q_OBJECT
public:
    MoCapController(MoCapStorage &storage, ExtrCalibration &extrCalib) : mStorage(storage), mExtrCalib(extrCalib){};

    void transformPersonSkeleton(
        const MoCapPerson              &person,
        double                          framerate,
        int                             currentFrame,
        std::vector<SegmentRenderData> &renderData) const;
    std::vector<SegmentRenderData>   getRenderData(int currentFrame, double framerate) const;
    bool                             getShowMoCap() const { return mShowMoCap; };
    void                             setShowMoCap(bool visibility);
    void                             setColor(const QColor &color);
    void                             setThickness(int thickness);
    void                             notifyAllObserver();
    std::vector<MoCapPersonMetadata> getAllMoCapPersonMetadata() const;
    void                             readMoCapFiles(const std::vector<MoCapPersonMetadata> &newMetadata);

    void setXml(QDomElement &elem);
    void getXml(const QDomElement &elem);

signals:
    void showMoCapChanged(bool visibility);
    void colorChanged(const QColor &color);
    void thicknessChanged(int thickness);

private:
    static std::vector<SkeletonLine>
    interpolate(const std::vector<SkeletonLine> &prePairs, const std::vector<SkeletonLine> &postPairs, double weight);


    MoCapStorage    &mStorage;
    bool             mShowMoCap = false;
    QColor           mColor     = QColor(255, 255, 55);
    int              mThickness = 2;
    ExtrCalibration &mExtrCalib;
};

#endif // MOCAPCONTROLLER_H
