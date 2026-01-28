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

#ifndef TRACKER_H
#define TRACKER_H

#include "recognition.h"
#include "trackPerson.h"
#include "trackPoint.h"

#include <QColor>
#include <QList>
#include <QRegularExpression>
#include <QSet>
#include <QTextStream>
#include <spdlog/fmt/bundled/format.h>

class PersonStorage;
class Petrack;


//----------------------------------------------------------------------------

/**
 * @brief Class orchestrating tracking and related stuff
 *
 * using tracker:
 * 1. initial recognition
 * 2. next frame track existing track points
 * 3. new recognition and check if exist otherwise include new
 * (4. delete inner tracking point, which does not recognized over a longer time)
 * 5. backward tracking from firstFrame() on
 * 6. calculate color over tracking (accumulated over tracking while procedure above) path and set height
 * 7. recalc coord with real coord with known height
 */
class Tracker
{
private:
    enum class TrackStatus
    {
        Tracked,
        NotTracked,
        Merged
    };

    Petrack                 *mMainWindow;
    cv::Mat                  mGrey, mPrevGrey;
    std::vector<cv::Mat>     mPrevPyr, mCurrentPyr;
    std::vector<cv::Point2f> mPrevFeaturePoints, mFeaturePoints;
    std::vector<TrackStatus> mStatus;
    int                      mPrevFrame;
    std::vector<int>         mPrevFeaturePointsIdx;
    std::vector<float>       mTrackError;
    cv::TermCriteria         mTermCriteria;
    PersonStorage           &mPersonStorage;

public:
    Tracker(QWidget *wParent, PersonStorage &storage);

    // neben loeschen der liste muessen auch ...
    void init(cv::Size size);

    void reset();

    void resize(cv::Size size);

    size_t calcPrevFeaturePoints(
        int          prevFrame,
        cv::Rect    &rect,
        int          frame,
        bool         reTrack,
        int          reQual,
        int          borderSize,
        QSet<size_t> onlyVisible);

    int insertFeaturePoints(int frame, size_t count, cv::Mat &img, int borderSize, cv::Mat map1, float errorScale);

    // frame ist frame fuer naechsten prev frame
    int track(
        cv::Mat                &img,
        cv::Rect               &rect,
        cv::Mat                 map1,
        int                     frame,
        bool                    reTrack,
        int                     reQual,
        int                     borderSize,
        reco::RecognitionMethod recoMethod,
        int                     level              = 3,
        QSet<size_t>            onlyVisible        = QSet<size_t>(),
        int                     errorScaleExponent = 0);

    void checkPlausibility(
        QList<int> &pers,
        QList<int> &frame,
        bool        testEqual    = true,
        bool        testVelocity = true,
        bool        testInside   = true,
        bool        testLength   = true);

private:
    bool tryMergeTrajectories(const TrackPoint &v, size_t i, int frame);

    void trackFeaturePointsLK(int level);
    void trackFeaturePointsLK(int level, bool adaptive);
    void refineViaColorPointLK(int level, float errorScale);
    void useBackgroundFilter(QList<int> &trjToDel, BackgroundFilter *bgFilter);
    void refineViaNearDarkPoint();
    void preCalculateImagePyramids(int level);
};

#endif
