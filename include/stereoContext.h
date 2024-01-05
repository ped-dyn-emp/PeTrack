/*
 * PeTrack - Software for tracking pedestrians movement in videos
 * Copyright (C) 2024 Forschungszentrum Jülich GmbH, IAS-7
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

#ifndef STEREOCONTEXT_H
#define STEREOCONTEXT_H

#include <QString>

#ifdef STEREO
#include <triclops.h>
#endif

#ifdef STEREO
#include "opencv2/calib3d.hpp"
#include "opencv2/calib3d/calib3d_c.h"
#include "pgrAviFile.h"
#else
#include "stereoAviFile.h"
#endif
#include "opencv2/calib3d.hpp"
#include "opencv2/calib3d/calib3d_c.h"

class Petrack;
class Animation;
class BackgroundFilter;

/* Namespace to stop multiple definition problem of Class StereoContext in Petrack and Triclops-Library */
namespace pet
{
enum stereoStatus
{
    clean        = 0,
    buildInput   = 1,
    preprocessed = 2,
    rectified    = 4,
    genDisparity = 8
};

class StereoContext
{
public:
    StereoContext(Petrack *main);

    ~StereoContext();

    // ---------------------------------------------------

    // to use before every new picture pair
    void init(cv::Mat &viewImg);

    void    preprocess();
    cv::Mat getRectified(enum Camera camera = cameraRight);
    cv::Mat getDisparity(bool *dispNew = nullptr);

    // von person.cpp benoetigt, um frame nummer zu erhalten
    inline Animation *getAnimation() { return mAnimation; }

    // ---------------------------------------------------


    double getCmPerPixel(float z);

    void calcMinMax();

    void indicateNewValues();

    bool getXYZ(int row, int col, float *x, float *y, float *z);

    bool getMedianXYZaround(int col, int row, float *x, float *y, float *z);

    float getZfromDisp(unsigned short int disp);

    bool dispValueValid(unsigned short int disp);

    // ---------------------------------------------------
#ifdef STEREO
    inline TriclopsContext getContext()
    {
        return mTriclopsContext;
    }
#endif
    inline unsigned char getSurfaceValue() const
    {
        return mSurfaceValue;
    }
    inline unsigned char getBackForthValue() const
    {
        return mBackForthValue;
    }

    inline unsigned short int getMin() const
    {
        return mMin;
    }
    inline unsigned short int getMax() const
    {
        return mMax;
    }

    inline void addStatus(enum stereoStatus s)
    {
        mStatus |= s;
    }
    inline void setStatus(enum stereoStatus s) // set status up to this level
    {
        mStatus = s * 2 - 1;
    }
    inline void resetStatus()
    {
        mStatus = clean;
    }

    cv::Mat getPointCloud();

    bool exportPointCloud(QString dest = "");

protected:
    Animation *mAnimation;
    Petrack   *mMain;
#ifdef STEREO
    TriclopsContext mTriclopsContext;
    TriclopsInput   mTriclopsInput;
    TriclopsImage   mTriRectLeft;
    TriclopsImage   mTriRectRight;
    TriclopsImage16 mTriDisparity;
#endif
    BackgroundFilter     *mBackgroundFilterLeft;
    BackgroundFilter     *mBackgroundFilterRight;
    cv::Mat               mRectLeft;
    cv::Mat               mRectRight;
    cv::Mat               mDisparity;
    cv::Ptr<cv::StereoBM> mBMState;


    cv::Ptr<cv::StereoSGBM> mSgbm;
    cv::Mat                 mBMdisparity16;
    cv::Mat                 mPointCloud;
    unsigned char           mSurfaceValue;
    unsigned char           mBackForthValue;
    unsigned short int      mMin;
    unsigned short int      mMax;
    short int               mStatus;
};


} // namespace pet

#endif // #ifndef STEREOCONTEXT
