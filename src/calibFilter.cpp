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

#include "calibFilter.h"
#include "helper.h"

using namespace::cv;

CalibFilter::CalibFilter()
    :Filter()
{
    getFx()->setMinimum(500.);
    getFx()->setMaximum(5000.);
    getFx()->setValue(881.);
    getFx()->setFilter(this);

    getFy()->setMinimum(500.);
    getFy()->setMaximum(5000.);
    getFy()->setValue(881.);
    getFy()->setFilter(this);

    getCx()->setMinimum(0.);
    getCx()->setMaximum(4096 /*1023.*/);
    getCx()->setValue(511.5);
    getCx()->setFilter(this);

    getCy()->setMinimum(0.);
    getCy()->setMaximum(2160 /*767.*/);
    getCy()->setValue(383.5);
    getCy()->setFilter(this);

    getR2()->setMinimum(-5.);
    getR2()->setMaximum(5.);
    getR2()->setValue(0.);
    getR2()->setFilter(this);

    getR4()->setMinimum(-5.);
    getR4()->setMaximum(5.);
    getR4()->setValue(0.);
    getR4()->setFilter(this);

    getTx()->setMinimum(-5.);
    getTx()->setMaximum(5.);
    getTx()->setValue(0.);
    getTx()->setFilter(this);

    getTy()->setMinimum(-5.);
    getTy()->setMaximum(5.);
    getTy()->setValue(0.);
    getTy()->setFilter(this);

    getR6()->setMinimum(-5.);
    getR6()->setMaximum(5.);
    getR6()->setValue(0.);
    getR6()->setFilter(this);

    getK4()->setMinimum(-5.);
    getK4()->setMaximum(5.);
    getK4()->setValue(0.);
    getK4()->setFilter(this);

    getK5()->setMinimum(-5.);
    getK5()->setMaximum(5.);
    getK5()->setValue(0.);
    getK5()->setFilter(this);

    getK6()->setMinimum(-5.);
    getK6()->setMaximum(5.);
    getK6()->setValue(0.);
    getK6()->setFilter(this);
}

Mat CalibFilter::act(Mat &img, Mat &res)
{
    if(this->changed() || map1.empty() || map2.empty())
    {
    Mat camera = (Mat_<float>(3,3) << getFx()->getValue(), 0,                   getCx()->getValue(),
                                          0,                   getFy()->getValue(), getCy()->getValue(),
                                          0,                   0,                   1                   );
    Mat dist = (Mat_<float>(1,8) << getR2()->getValue(), getR4()->getValue(),
                                        getTx()->getValue(), getTy()->getValue(),
                                        getR6()->getValue(),
                                        getK4()->getValue(), getK5()->getValue(), getK6()->getValue());


        initUndistortRectifyMap(camera, dist, Mat_<double>::eye(3,3),
                                camera,
                                img.size(), CV_16SC2, map1, map2);
    }

    remap(img, res, map1, map2, INTER_LINEAR, BORDER_CONSTANT);
    return res;
}

Parameter* CalibFilter::getFx()
{
    return &mFx;
}
Parameter* CalibFilter::getFy()
{
    return &mFy;
}
Parameter* CalibFilter::getCx()
{
    return &mCx;
}
Parameter* CalibFilter::getCy()
{
    return &mCy;
}
Parameter* CalibFilter::getR2()
{
    return &mR2;
}
Parameter* CalibFilter::getR4()
{
    return &mR4;
}
Parameter* CalibFilter::getTx()
{
    return &mTx;
}
Parameter* CalibFilter::getTy()
{
    return &mTy;
}
Parameter* CalibFilter::getR6()
{
    return &mR6;
}
Parameter* CalibFilter::getK4()
{
    return &mK4;
}
Parameter* CalibFilter::getK5()
{
    return &mK5;
}
Parameter* CalibFilter::getK6()
{
    return &mK6;
}

