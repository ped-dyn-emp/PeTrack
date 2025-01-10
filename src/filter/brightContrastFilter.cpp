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

#include "brightContrastFilter.h"


BrightContrastFilter::BrightContrastFilter() : Filter()
{
    mBrightness.setMinimum(-100.);
    mBrightness.setMaximum(100.);
    mBrightness.setValue(0.);

    mContrast.setMinimum(-100.);
    mContrast.setMaximum(100.);
    mContrast.setValue(0.);
}

cv::Mat BrightContrastFilter::act(cv::Mat &img, cv::Mat &res)
{
    double delta, a, b;
    /*
     * The algorithm is by Werner D. Streidt
     * (http://visca.com/ffactory/archives/5-99/msg00021.html)
     */
    if(mContrast.getValue() > 0)
    {
        delta = 127. * mContrast.getValue() / 100.;
        a     = 255. / (255. - delta * 2.);
        b     = a * (mBrightness.getValue() - delta);
    }
    else
    {
        delta = -128. * mContrast.getValue() / 100.;
        a     = (256. - delta * 2.) / 255.;
        b     = a * mBrightness.getValue() + delta;
    }

    img.convertTo(res, -1, a, b);
    return res;
}

Parameter<double> &BrightContrastFilter::getBrightness()
{
    return mBrightness;
}

Parameter<double> &BrightContrastFilter::getContrast()
{
    return mContrast;
}
