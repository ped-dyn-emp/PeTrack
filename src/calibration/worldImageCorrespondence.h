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

#ifndef WORLDIMAGECORRESPONDENCE_H

#include <QPoint>

class WorldImageCorrespondence
{
public:
    /**
     * @brief Returns the length of a pixel in cm
     *
     * This method assumes quadratic pixels. This is valid, since we
     * did an intrinsic calibration beforehand. Further, it assumes that we
     * are working with our "2D calibration".
     *
     * @return length of pixel in cm
     */
    virtual double getCmPerPixel() const = 0;

    /**
     * @brief Returns the side length of a pixel in cm
     * @param px x-coordinate of the pixel
     * @param py y-coordinate of the pixel
     * @param h world-coordinate height at which the length of the pixel is probed
     * @return lengths of the sides of the pixel
     */
    virtual QPointF getCmPerPixel(float px, float py, float h = 0.) const = 0;

    /**
     * @brief Returns the angle between the line from camera to point an from camera othogonal to ground
     *
     * For clarification, see Abb. 3.31 (a) or Abb. 3.75 in Maik's thesis
     *
     * @param px x-coordinate of probed pixel
     * @param py y-coordinate of probed pixel
     * @param h world-coordinate height at which the angle is probed
     * @return angle between line from cam to point and from cam to ground
     */
    virtual double getAngleToGround(float px, float py, float h = 0) const = 0;

    /**
     * @brief Calcultaes the position of the point in image/pixel coordinates
     * @param pos x and y-coord of  position in world coordinates in cm
     * @param height z-coord of position in world coordinates in cm
     * @return image coordinate in pixels
     */
    virtual QPointF getPosImage(QPointF pos, float height = 0.) const = 0;

    /**
     * @brief Calculates the position of the image point in world coordinates
     * @param pos pixel in image coordinates
     * @param height z-coordinate in world coordinate system
     * @return point in world coordinate system
     */
    virtual QPointF getPosReal(QPointF pos, double height = 0.) const = 0;

    virtual ~WorldImageCorrespondence() = default;
};


#define WORLDIMAGECORRESPONDENCE_H

#endif // WORLDIMAGECORRESPONDENCE_H
