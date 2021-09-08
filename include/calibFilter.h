/*
 * PeTrack - Software for tracking pedestrians movement in videos
 * Copyright (C) 2010-2020 Forschungszentrum Jülich GmbH,
 * Maik Boltes, Juliane Adrian, Ricardo Martin Brualla, Arne Graf, Paul Häger, Daniel Hillebrand,
 * Deniz Kilic, Paul Lieberenz, Daniel Salden, Tobias Schrödter, Ann Katrin Seemann
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

#ifndef CALIBFILTER_H
#define CALIBFILTER_H

#include "filter.h"

/**
 * @brief Undistortion filter
 *
 * This class is a filter which undistorts the image using the camera matrix from intrinsic calibration.
 * It caches the mapping from distorted to undistorted image.
 */
class CalibFilter : public Filter
{
private:
    Parameter mFx;
    Parameter mFy;
    Parameter mCx;
    Parameter mCy;
    Parameter mR2; // mK1
    Parameter mR4; // mK2
    Parameter mTx;
    Parameter mTy;
    Parameter mR6; // mK3
    Parameter mK4;
    Parameter mK5;
    Parameter mK6;
    cv::Mat   map1;
    cv::Mat   map2;

public:
    CalibFilter();

    cv::Mat act(cv::Mat &img, cv::Mat &res);

    Parameter *getFx();
    Parameter *getFy();
    Parameter *getCx();
    Parameter *getCy();
    Parameter *getR2();
    Parameter *getR4();
    Parameter *getTx();
    Parameter *getTy();
    Parameter *getR6();
    Parameter *getK4();
    Parameter *getK5();
    Parameter *getK6();
};

#endif
