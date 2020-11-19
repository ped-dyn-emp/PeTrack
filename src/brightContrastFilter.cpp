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

#include "brightContrastFilter.h"

using namespace::cv;

BrightContrastFilter::BrightContrastFilter()
    :Filter()
{
    mB.setMinimum(-100.);
    mB.setMaximum(100.);
    mB.setValue(0.);
    mB.setFilter(this);

    mC.setMinimum(-100.);
    mC.setMaximum(100.);
    mC.setValue(0.);
    mC.setFilter(this);

//    mLutMat = cvCreateMatHeader(1, 256, CV_8UC1);
//    cvSetData(mLutMat, mLut, 0);
//    setOnCopy(true);
}

Mat BrightContrastFilter::act(Mat &img, Mat &res)
{

    double delta, a, b;
    /*
     * The algorithm is by Werner D. Streidt
     * (http://visca.com/ffactory/archives/5-99/msg00021.html)
     */
    if (mC.getValue() > 0)
    {
        delta = 127.*mC.getValue()/100.;
        a = 255./(255. - delta*2.);
        b = a*(mB.getValue() - delta);
//        for (i = 0; i < 256; i++)
//        {
//            v = cvRound(a*i + b);
//            if (v < 0)
//                v = 0;
//            else if (v > 255)
//                v = 255;
//            mLut[i] = (uchar) v;
//        }
    }
    else
    {
        delta = -128.*mC.getValue()/100.;
        a = (256.-delta*2.)/255.;
        b = a*mB.getValue() + delta;
//        for (i = 0; i < 256; i++)
//        {
//            v = cvRound(a*i + b);
//            if (v < 0)
//                v = 0;
//            else if (v > 255)
//                v = 255;
//            mLut[i] = (uchar) v;
//        }
    }

//    cvLUT(imgIpl, resIpl, mLutMat);

    img.convertTo(res,-1,a,b);
//    LUT(img,mLutMat,res);
//    cvReleaseImage(&resIpl);
//    resIpl = cvCreateImage(cvSize(res.cols,res.rows),8,3);
//    IplImage tmpIpl = res;
//    cvCopy(&tmpIpl,resIpl);


    return res;
}

Parameter* BrightContrastFilter::getBrightness()
{
    return &mB;
}

Parameter* BrightContrastFilter::getContrast()
{
    return &mC;
}
