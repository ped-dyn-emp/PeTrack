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

#ifndef FILTER_H
#define FILTER_H

#include <opencv2/opencv.hpp>

class Filter;

/**
 * @brief Class for Filter-parameters
 *
 * This class is made to hold parameters for filters.
 * It has a minimum and maximum value and notifies the
 * associated filter of changes.
 *
 * @todo make a template out of it, so different types are supported
 */
class Parameter
{
private:
    double  mValue;
    double  mMinimum;
    double  mMaximum;
    bool    mChg;
    Filter *mFilter; // Filter where parameter is for

public:
    Parameter();

    void    setFilter(Filter *filter);
    Filter *getFilter();

    double value() const;
    double getValue() const;
    void   setValue(double d);

    double getMinimum() const;
    void   setMinimum(double d);
    double getMaximum() const;
    void   setMaximum(double d);

    bool changed() const;
    bool getChanged() const;
    void setChanged(bool b);
};

//------------------------------------------------------------

/**
 * @brief Base class for every image filter
 *
 * This class is the base class for every image filter,
 * like the CalibFilter or Background filter. The
 * interface this class defines includes a unified way
 * to apply the different filters, caching of the results,
 * activation/deactivation as well as automated detection
 * of changed parameters.
 */
class Filter
{
private:
    bool    mChg;    // if filter paramater were changed
    bool    mEnable; // if filter is actice
    bool    mOnCopy; // if filter works on a copy
    cv::Mat mRes;

    // pure virtual function, where to implement the filter conversion
    // returns the result over pointer res and as result
    virtual cv::Mat act(cv::Mat &img, cv::Mat &res) = 0;

public:
    Filter();
    virtual ~Filter(){};

    bool changed() const;
    bool getChanged();
    void setChanged(bool b);


    // apply on original Data
    cv::Mat apply(cv::Mat &img);

    cv::Mat getLastResult();
    void    freeLastResult();

    void enable();
    void disable();
    void setEnabled(bool b);
    bool getEnabled() const;

    void setOnCopy(bool b);
    bool getOnCopy() const;
};

#endif
