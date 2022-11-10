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

#ifndef STEREO_DISABLED
#include <stdio.h>

//=============================================================================
// System Includes
//=============================================================================
#include <cassert>
#include <cstdio>

//=============================================================================
// Project Includes
//=============================================================================
#include "helper.h"
#include "stereoAviFile.h"

//
// Extra temp data bytes.
//
#define EXTRA_TEMP_SPACE 10000

StereoAviFile::StereoAviFile()
{
    m_pTempBuffer = NULL;

    m_iTimeIndex = -1;

    m_iCols   = 0;
    m_iRows   = 0;
    m_iBPP    = 0;
    m_iSize   = 0;
    m_iRowInc = 0;

    mCamera =
        cameraRight; // wie in petrack.cpp right als default genommen da reference image in triclops auch right ist

    mImageLeft  = NULL;
    mImageRight = NULL;
}


StereoAviFile::~StereoAviFile()
{
    close();

    if(m_pTempBuffer != NULL)
    {
        delete[] m_pTempBuffer;
        m_pTempBuffer = NULL;
    }
}

// if return false there may be some allocated memory left!!!
// stereoImg is used to use the same img for a bunch of files
bool StereoAviFile::open(const char *pszFilename, IplImage *stereoImgLeft, IplImage *stereoImgRight)
{
    if(pszFilename == NULL)
        return false;

    m_vcReader = cvCreateFileCapture(pszFilename);

    m_iTimeIndex = -1; // damit das erste readframe wirklich einen frame auch bei index=0 liesst

    m_iCols       = cvGetCaptureProperty(m_vcReader, CV_CAP_PROP_FRAME_WIDTH);  // bi.biWidth;
    m_iRows       = cvGetCaptureProperty(m_vcReader, CV_CAP_PROP_FRAME_HEIGHT); // bi.biHeight;
    m_iBPP        = 8;                                                          // bi.biBitCount;
    m_iRowInc     = m_iCols * (m_iBPP / 8);
    m_iSize       = m_iRows * m_iRowInc;
    m_pTempBuffer = new unsigned char[m_iSize];

    // if (stereoImg != NULL)
    mImageLeft  = stereoImgLeft; // Attention: nur 1280x960 image wird angelegt fuer bumblebee!!!
    mImageRight = stereoImgRight;
    //   CvSize size;
    //   size.width = m_iCols;
    //   size.height = m_iRows;
    // cvCreateImage(size, 8, 1);

    return true;
}

IplImage *StereoAviFile::getFrame(enum Camera camera)
{
    if(camera == cameraLeft)
        return mImageLeft;
    else if(camera == cameraRight)
        return mImageRight;
    else
        return NULL;
}

IplImage *StereoAviFile::readFrame(int index)
{
    if(index != m_iTimeIndex)
    {
        int x, y;
        // Pointer to the data information in the IplImage
        // char *data = mImage->imageData;
        char *dataLeft   = mImageLeft->imageData;
        char *dataRight  = mImageRight->imageData;
        char *yDataLeft  = dataLeft;
        char *yDataRight = dataRight;
        char *p          = (char *) m_pTempBuffer;
        // if (mCamera == cameraLeft)
        //     ++p; //p+=2*mImage->height*mImage->height;
        //  This loop is optimized so it has to calculate the least amount of indexes
        //  Optimizing the access to the pointer data is useless (no difference in performance when tested)
        for(y = 0; y < mImageLeft->height; ++y)
        {
            for(x = 0; x < mImageLeft->width; ++x)
            {
                *dataRight = *p;
                ++dataRight;
                ++p;
                *dataLeft = *p;
                ++dataLeft;
                ++p; // p+=2;
            }
            // p += mImage->width; // die linke oder rechte kamera ueberspringen
            dataLeft  = (yDataLeft += mImageLeft->widthStep); // because sometimes widthStep != width
            dataRight = (yDataRight += mImageRight->widthStep);
        }

        m_iTimeIndex = index;
    }

    if(mCamera == cameraLeft)
        return mImageLeft;
    else
        return mImageRight;
    // return mImage;
}

bool StereoAviFile::close()
{
    if(m_pTempBuffer != NULL)
    {
        delete[] m_pTempBuffer;
        m_pTempBuffer = NULL;
    }

    // cvReleaseImage(&mImage); wird nun in animation.cpp angelegt

    //
    // Release the library.
    //

    return true;
}

enum Camera StereoAviFile::getCamera()
{
    return mCamera;
}
void StereoAviFile::setCamera(enum Camera c)
{
    mCamera = c;
    //    if (mCamera != c)
    //    {
    //        mCamera = c;
    //        readFrame(m_iTimeIndex); // direktes neulesen des streames
    //    }
}

#endif
