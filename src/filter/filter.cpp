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

#include "filter.h"


Filter::Filter()
{
    mEnable = true;
    mOnCopy = true;
    mChg    = false;
}

/**
 * @brief Indicates if any of the parameters of the filter have changed
 *
 * was pure virtuell function must be implemented
 * and has to return, if any parameter is changed
 * so that the filter would return another result
 * now this function has also to be called
 * from a child function which looks for changing parameters
 *
 * @return true, if a parameter changed
 */
bool Filter::changed() const
{
    return mChg;
}
bool Filter::getChanged()
{
    return changed();
}
void Filter::setChanged(bool b)
{
    mChg = b;
}

/**
 * @brief Applies the filter to img and sets changed on false.
 *
 * The filter is applied, iff it is enabled. Else the image
 * is returned without any modifications. Depending on the
 * Filter::getOnCopy() result, the image is copied before the filter
 * is applied.
 *
 * The result is cached and can be accessed through Filter::getLastResult().
 * @param img image to be transformed
 * @return if enabled the transformed image else just img without changes (still gets cached)
 */
cv::Mat Filter::apply(cv::Mat &img)
{
    if(getEnabled())
    {
        if(getOnCopy())
        {
            cv::Mat res(cv::Size(img.cols, img.rows), CV_8UC(img.channels()));
            mRes = act(img, res);
            mChg = false;
            return mRes;
        }
        else
        {
            mRes = act(img, img);
            mChg = false;
            return mRes;
        }
    }
    else
    {
        mChg        = false;
        return mRes = img;
    }
}

cv::Mat Filter::getLastResult()
{
    return mRes;
}

void Filter::enable()
{
    mChg    = true;
    mEnable = true;
}
void Filter::disable()
{
    mChg    = true;
    mEnable = false;
}
void Filter::setEnabled(bool b)
{
    mChg    = true;
    mEnable = b;
}
bool Filter::getEnabled() const
{
    return mEnable;
}

void Filter::setOnCopy(bool b)
{
    mChg    = true;
    mOnCopy = b;
}
bool Filter::getOnCopy() const
{
    return mOnCopy;
}
