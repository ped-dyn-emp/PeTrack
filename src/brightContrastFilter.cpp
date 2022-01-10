/*
 * PeTrack - Software for tracking pedestrians movement in videos
 * Copyright (C) 2010-2022 Forschungszentrum Jülich GmbH,
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

#include "brightContrastFilter.h"


BrightContrastFilter::BrightContrastFilter() : Filter()
{
    mB.setMinimum(-100.);
    mB.setMaximum(100.);
    mB.setValue(0.);
    mB.setFilter(this);

    mC.setMinimum(-100.);
    mC.setMaximum(100.);
    mC.setValue(0.);
    mC.setFilter(this);
}

cv::Mat BrightContrastFilter::act(cv::Mat &img, cv::Mat &res)
{
    double delta, a, b;
    /*
     * The algorithm is by Werner D. Streidt
     * (http://visca.com/ffactory/archives/5-99/msg00021.html)
     */
    if(mC.getValue() > 0)
    {
        delta = 127. * mC.getValue() / 100.;
        a     = 255. / (255. - delta * 2.);
        b     = a * (mB.getValue() - delta);
    }
    else
    {
        delta = -128. * mC.getValue() / 100.;
        a     = (256. - delta * 2.) / 255.;
        b     = a * mB.getValue() + delta;
    }

    img.convertTo(res, -1, a, b);
    return res;
}

Parameter *BrightContrastFilter::getBrightness()
{
    return &mB;
}

Parameter *BrightContrastFilter::getContrast()
{
    return &mC;
}
