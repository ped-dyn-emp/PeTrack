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

#include "borderFilter.h"


BorderFilter::BorderFilter() : Filter()
{
    mSize.setMinimum(0.);
    mSize.setMaximum(300);
    mSize.setValue(0.);

    mRed.setMinimum(0.);
    mRed.setMaximum(200.);
    mRed.setValue(0.);

    mGreen.setMinimum(0.);
    mGreen.setMaximum(200.);
    mGreen.setValue(0.);

    mBlue.setMinimum(0.);
    mBlue.setMaximum(200.);
    mBlue.setValue(0.);

    setOnCopy(false);
}

cv::Mat BorderFilter::act(cv::Mat &img, cv::Mat &res)
{
    // border size
    int s =
        mSize.getValue(); // param is 2x user-defined border-size, because border needs to be even (for undistortion)
    int r = mRed.getValue();
    int g = mGreen.getValue();
    int b = mBlue.getValue();

    cv::copyMakeBorder(img, res, s, s, s, s, cv::BORDER_CONSTANT, cv::Scalar(b, g, r));

    return res;
}

Parameter<int> &BorderFilter::getBorderSize()
{
    return mSize;
}
Parameter<int> &BorderFilter::getBorderColR()
{
    return mRed;
}
Parameter<int> &BorderFilter::getBorderColG()
{
    return mGreen;
}
Parameter<int> &BorderFilter::getBorderColB()
{
    return mBlue;
}
