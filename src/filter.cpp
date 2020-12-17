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

#include "filter.h"
#include "helper.h"

using namespace::cv;

// // muesste fuer double und int existieren
Parameter::Parameter()
{
    mValue = mMinimum = mMaximum = 0;
    mChg = false;
    mFilter = NULL;
}

void Parameter::setFilter(Filter *filter)
{
    mFilter = filter;
}
Filter * Parameter::getFilter()
{
    return mFilter;
}

double Parameter::value()
{
    return mValue;
}
double Parameter::getValue()
{
    return mValue;
}
void Parameter::setValue(double d)
{
    if (d != mValue)
    {
        mValue = d;
        setChanged(true);
    }
}

double Parameter::getMinimum()
{
    return mMinimum;
}
void Parameter::setMinimum(double d)
{
    mMinimum = d;
}
double Parameter::getMaximum()
{
    return mMaximum;
}
void Parameter::setMaximum(double d)
{
    mMaximum = d;
}

bool Parameter::changed()
{
    return mChg;
}
bool Parameter::getChanged()
{
    return mChg;
}
void Parameter::setChanged(bool b)
{
    mChg = b;
    if (mFilter)
        mFilter->setChanged(true);
}

//------------------------------------------------------------

Filter::Filter()
{
    mEnable = true; //enable();
    mOnCopy = true; //setOnCopy(true);
//    mRes = NULL;
    mChg = false;
//    mResStored = false;
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
bool Filter::changed()
{
    return mChg;
}
bool Filter::getChanged()
{
    return changed();
}
void Filter::setChanged(bool b)
{
    mChg=b;
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
Mat Filter::apply(Mat &img)
{
    if (getEnabled())
    {
        if (getOnCopy())
        {
            Mat res(Size(img.cols,img.rows),CV_8UC(img.channels()));
            mRes = act(img,res);
            mChg = false;
            return mRes;
        }else
        {
            mRes = act(img,img);
            mChg = false;
            return mRes;
        }

    }else
    {
        mChg = false;
        return mRes = img;
    }
}

Mat Filter::getLastResult()
{
    return mRes;
}

void Filter::freeLastResult()
{
//    cvReleaseImage(&mRes);
} //free oder delete? - beides falsch

void Filter::enable()
{
    mChg = true;
    mEnable = true;
}
void Filter::disable()
{
    mChg = true;
    mEnable = false;
}
void Filter::setEnabled(bool b)
{
    mChg = true;
    mEnable = b;
}
bool Filter::getEnabled()
{
    return mEnable;
}

void Filter::setOnCopy(bool b)
{
    mChg = true;
    mOnCopy = b;
}
bool Filter::getOnCopy()
{
    return mOnCopy;
}

// void Filter::addParam(QString *name, min, max, default)
// {
// }

// flag zur Anzeige, ob auf Originaldaten Filter angewendet werden kann und somit setoncop(false) moeglich ist

// flag zur Anzeige, ob Filter Bildgroesse veraendert

// widget, mit dem parameter einstellbar sind
// grundlegender rahmen mit ok, cancel und preview wie in photoshop
// inneres kann von abgeleiteter klasse kommen

// einstellen welche parameter: anzahl, name, wertebereich, default
