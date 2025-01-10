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

#ifndef FILTER_H
#define FILTER_H

#include <opencv2/core.hpp>


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
    virtual ~Filter() = default;

    bool changed() const;
    bool getChanged();
    void setChanged(bool b);


    // apply on original Data
    cv::Mat apply(cv::Mat &img);

    cv::Mat getLastResult();

    void enable();
    void disable();
    void setEnabled(bool b);
    bool getEnabled() const;

    void setOnCopy(bool b);
    bool getOnCopy() const;
};

//----------------------

/**
 * @brief Class for Filter-parameters
 *
 * This class is made to hold parameters for filters.
 * It has a minimum and maximum value and notifies the
 * associated filter of changes.
 *
 */
template <typename T>
class Parameter
{
private:
    T       mValue;
    T       mMinimum;
    T       mMaximum;
    bool    mChanged;
    Filter *mFilter; // Filter where parameter is for

public:
    explicit Parameter(Filter *filter)
    {
        if constexpr(std::is_arithmetic<T>::value)
        {
            mValue = mMinimum = mMaximum = 0;
        }
        mChanged = false;
        mFilter  = filter;
    }

    T    getValue() const { return mValue; }
    void setValue(T d)
    {
        if(d != mValue)
        {
            mValue = d;
            setChanged(true);
        }
    }

    T    getMinimum() const { return mMinimum; }
    void setMinimum(T d) { mMinimum = d; }
    T    getMaximum() const { return mMaximum; }
    void setMaximum(T d) { mMaximum = d; }

    bool getChanged() const { return mChanged; }
    void setChanged(bool b)
    {
        mChanged = b;
        if(mFilter != nullptr)
        {
            mFilter->setChanged(true);
        }
    }
};


#endif
