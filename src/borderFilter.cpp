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

#include "borderFilter.h"


BorderFilter::BorderFilter() : Filter()
{
    mS.setMinimum(0.);
    mS.setMaximum(300);
    mS.setValue(0.);
    mS.setFilter(this);

    mR.setMinimum(0.);
    mR.setMaximum(200.);
    mR.setValue(0.);
    mR.setFilter(this);

    mG.setMinimum(0.);
    mG.setMaximum(200.);
    mG.setValue(0.);
    mG.setFilter(this);

    mB.setMinimum(0.);
    mB.setMaximum(200.);
    mB.setValue(0.);
    mB.setFilter(this);

    setOnCopy(false);
}

cv::Mat BorderFilter::act(cv::Mat &img, cv::Mat &res)
{
    // border size
    int s = (int) mS.getValue(); // schon beim zuweisen auf param gemacht (2*, da undistord filter anscheinend sonst
                                 // probleme hat)
    int r = (int) mR.getValue();
    int g = (int) mG.getValue();
    int b = (int) mB.getValue();

    cv::copyMakeBorder(img, res, s, s, s, s, cv::BORDER_CONSTANT, cv::Scalar(b, g, r));

    return res;
}

Parameter *BorderFilter::getBorderSize()
{
    return &mS;
}
Parameter *BorderFilter::getBorderColR()
{
    return &mR;
}
Parameter *BorderFilter::getBorderColG()
{
    return &mG;
}
Parameter *BorderFilter::getBorderColB()
{
    return &mB;
}
