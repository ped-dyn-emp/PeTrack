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
#include "plausibility.h"

#include "personStorage.h"
#include "petrack.h"

#include <QApplication> // for qApp
#include <QProgressDialog>

namespace plausibility
{

/**
 * Checks if the length for any trajectory is less than minLength frames.
 *
 * @param personStorage data container for trajectories
 * @param progressDialog dialog for showing progress of all checks
 * @param minLength minimum length each trajectory needs to have to not be reported as faulty
 *
 * @return vector of all trajectories which consists of less than minLength frames
 */
std::vector<FailedCheck> checkLength(const PersonStorage &personStorage, QProgressDialog *progressDialog, int minLength)
{
    std::vector<FailedCheck> failedChecks;
    progressDialog->setValue(0);
    progressDialog->setLabelText("Check trajectories lengths...");
    qApp->processEvents();

    for(size_t i = 0; i < personStorage.nbPersons(); ++i)
    {
        if(personStorage.at(i).size() < minLength)
        {
            failedChecks.push_back(
                {i + 1,
                 personStorage.at(i).firstFrame(),
                 fmt::format("Has less than {} trackpoints!", minLength),
                 plausibility::CheckType::Length});
        }
    }
    return failedChecks;
}

/**
 * Checks if the first or last frame of trajectory are inside rect and the picture.
 *
 * @param personStorage data container for trajectories
 * @param progressDialog dialog for showing progress of all checks
 * @param sequenceSize size of the sequence
 * @param imageBorderSize border size
 * @param rect rectangle used for checking, currently reco ROI
 * @param lastFrame last frame of sequence
 * @param margin distance to image border, in which the trajectory may get lost
 *
 * @return vector of all trajectories where the first or last frame are inside rect and the picture
 */
std::vector<FailedCheck> checkInside(
    const PersonStorage &personStorage,
    QProgressDialog     *progressDialog,
    const cv::Size      &sequenceSize,
    int                  imageBorderSize,
    QRectF               rect,
    int                  firstFrame,
    int                  lastFrame,
    int                  margin)
{
    std::vector<FailedCheck> failedChecks;

    progressDialog->setValue(100);
    progressDialog->setLabelText("Check if trajectories are inside image...");
    qApp->processEvents();

    const int    imgWidth  = sequenceSize.width - 1 - 2 * imageBorderSize - margin;
    const int    imgHeight = sequenceSize.height - 1 - 2 * imageBorderSize - margin;
    const QRectF imgRect(margin, margin, imgWidth, imgHeight);
    const QRectF checkRect = imgRect.intersected(rect);

    for(size_t i = 0; i < personStorage.nbPersons(); ++i)
    {
        qApp->processEvents();
        progressDialog->setValue(100 + i * 100. / personStorage.nbPersons());
        double x = personStorage.at(i).first().x();
        double y = personStorage.at(i).first().y();

        if(personStorage.at(i).firstFrame() != firstFrame && checkRect.contains(x, y))
        {
            failedChecks.push_back(
                {i + 1,
                 personStorage.at(i).firstFrame(),
                 "Start of trajectory is inside picture and reco ROI!",
                 plausibility::CheckType::Inside});
        }

        x = personStorage.at(i).last().x();
        y = personStorage.at(i).last().y();

        if(personStorage.at(i).lastFrame() != lastFrame && checkRect.contains(x, y))
        {
            failedChecks.push_back(
                {i + 1,
                 personStorage.at(i).lastFrame(),
                 "End of trajectory is inside picture and reco ROI!",
                 plausibility::CheckType::Inside});
        }
    }

    return failedChecks;
}

/**
 * Check for large velocity variations in all trajectories
 *
 * A faulty velocity variations means:
 * - speed changed with a factor > 1.8
 * AND
 * - more than 6 px movement OR average in 2 frames > 3 px
 *
 * For computation, following points are taken into account:
 *
 * frame-1      frame       frame+1         frame+2
 *    o-----------o------------o---------------o
 *    |           |            |
 *    |... d01 ...|.... d12 ...|..... d23 .....|
 *
 * Note: Instead of the distance, comparing vectors would be accurate.
 *
 * @param personStorage data container for trajectories
 * @param progressDialog dialog for showing progress of all checks
 *
 * @return vector of all trajectories with large velocity variations
 */
std::vector<FailedCheck> checkVelocityVariation(const PersonStorage &personStorage, QProgressDialog *progressDialog)
{
    std::vector<FailedCheck> failedChecks;

    qApp->processEvents();
    progressDialog->setValue(200);
    progressDialog->setLabelText("Check velocity...");

    for(size_t i = 0; i < personStorage.nbPersons(); ++i) // ueber TrackPerson
    {
        qApp->processEvents();
        progressDialog->setValue(200 + i * 100. / personStorage.nbPersons());

        // ignore first and two last TrackPoints, as these points are needed as buffer
        for(int j = 1; j < personStorage.at(i).size() - 2; ++j)
        {
            double d01 = personStorage.at(i).at(j).distanceToPoint(personStorage.at(i).at(j - 1));
            double d12 = personStorage.at(i).at(j + 1).distanceToPoint(personStorage.at(i).at(j));
            double d23 = personStorage.at(i).at(j + 2).distanceToPoint(personStorage.at(i).at(j + 1));

            const bool largeVariation = (1.8 * (d01 + d23) / 2.) < d12;
            const bool moving         = (d12 > 6.) || ((d01 + d23) / 2. > 3.);

            if(largeVariation && moving)
            {
                failedChecks.push_back(
                    {i + 1,
                     j + personStorage.at(i).firstFrame(),
                     "Fast variation of velocity to following frame!",
                     plausibility::CheckType::Velocity});
            }
        }
    }

    return failedChecks;
}

/**
 * Checks if two trajectories are close to each other in a specific frame.
 *
 * @param personStorage data container for trajectories
 * @param progressDialog dialog for showing progress of all checks
 * @param petrack main window
 * @param headSizeFactor factor used to determine the distance at which two traj are considered equal
 *
 * @return vector of all trajectories which were too close to an other trajectory in one frame
 */
std::vector<FailedCheck> checkEquality(
    const PersonStorage &personStorage,
    QProgressDialog     *progressDialog,
    Petrack             &petrack,
    double               headSizeFactor)
{
    std::vector<FailedCheck> failedChecks;
    progressDialog->setValue(300);
    progressDialog->setLabelText("Check if trajectories are equal...");
    qApp->processEvents();

    int largestLastFrame = personStorage.largestLastFrame();
    for(int frame = personStorage.smallestFirstFrame(); frame <= largestLastFrame; ++frame)
    {
        progressDialog->setValue(300 + frame * 100. / largestLastFrame);
        qApp->processEvents();

        for(size_t i = 0; i < personStorage.nbPersons(); ++i)
        {
            for(size_t j = i + 1; j < personStorage.nbPersons(); ++j)
            {
                if(personStorage.at(i).trackPointExist(frame) && personStorage.at(j).trackPointExist(frame))
                {
                    if(personStorage.at(i).trackPointAt(frame).distanceToPoint(personStorage.at(j).trackPointAt(
                           frame)) < headSizeFactor * petrack.getHeadSize(nullptr, static_cast<int>(i), frame))
                    {
                        failedChecks.push_back(
                            {i + 1,
                             frame,
                             fmt::format("Trajectory is very close to Person {}!", j + 1),
                             plausibility::CheckType::Equality});
                    }
                }
            }
        }
    }

    return failedChecks;
}
} // namespace plausibility
