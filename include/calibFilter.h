/*
 * PeTrack - Software for tracking pedestrians movement in videos
 * Copyright (C) 2022 Forschungszentrum Jülich GmbH, IAS-7
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
    Parameter<double> mFx{this};
    Parameter<double> mFy{this};
    Parameter<double> mCx{this};
    Parameter<double> mCy{this};
    Parameter<double> mR2{this}; // mK1
    Parameter<double> mR4{this}; // mK2
    Parameter<double> mTx{this};
    Parameter<double> mTy{this};
    Parameter<double> mR6{this}; // mK3
    Parameter<double> mK4{this};
    Parameter<double> mK5{this};
    Parameter<double> mK6{this};
    Parameter<double> mS1{this};
    Parameter<double> mS2{this};
    Parameter<double> mS3{this};
    Parameter<double> mS4{this};
    Parameter<double> mTAUX{this};
    Parameter<double> mTAUY{this};
    double            mReprojectionError;
    cv::Mat           map1;
    cv::Mat           map2;

public:
    CalibFilter();

    cv::Mat act(cv::Mat &img, cv::Mat &res);

    Parameter<double> &getFx();
    Parameter<double> &getFy();
    Parameter<double> &getCx();
    Parameter<double> &getCy();
    Parameter<double> &getR2();
    Parameter<double> &getR4();
    Parameter<double> &getTx();
    Parameter<double> &getTy();
    Parameter<double> &getR6();
    Parameter<double> &getK4();
    Parameter<double> &getK5();
    Parameter<double> &getK6();
    Parameter<double> &getS1();
    Parameter<double> &getS2();
    Parameter<double> &getS3();
    Parameter<double> &getS4();
    Parameter<double> &getTAUX();
    Parameter<double> &getTAUY();
    double             getReprojectionError() const;
    void               setReprojectionError(double d);
};

#endif
