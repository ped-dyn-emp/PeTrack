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

#include "swapFilter.h"

#include "helper.h"

SwapFilter::SwapFilter() : Filter()
{
    mSwapVertically.setMinimum(0.);
    mSwapVertically.setMaximum(1.);
    mSwapVertically.setValue(0.);
    mSwapVertically.setFilter(this);

    mSwapHorizontally.setMinimum(0.);
    mSwapHorizontally.setMaximum(1.);
    mSwapHorizontally.setValue(0.);
    mSwapHorizontally.setFilter(this);

    setOnCopy(true);
}

cv::Mat SwapFilter::act(cv::Mat &img, cv::Mat &res)
{
    bool sV = (bool) mSwapVertically.getValue();
    bool sH = (bool) mSwapHorizontally.getValue();

    if(sV && sH)
    {
        cv::flip(img, res, -1); // both
    }
    else if(sV)
    {
        cv::flip(img, res, 0); // vertical
    }
    else if(sH)
    {
        cv::flip(img, res, 1); // horizontal
    }
    else
    {
        res = img; // nothing to do
    }


    return res;
}

Parameter *SwapFilter::getSwapHorizontally()
{
    return &mSwapHorizontally;
}
Parameter *SwapFilter::getSwapVertically()
{
    return &mSwapVertically;
}
