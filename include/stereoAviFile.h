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

#ifndef STEREO_DISABLED
#ifndef STEREOAVIFILE_H
#define STEREOAVIFILE_H
#undef UNICODE

//=============================================================================
// System Includes
//=============================================================================


//=============================================================================
// Project Includes
//=============================================================================

//#include <cv.h>
//#include <highgui.h>
#include "opencv2/opencv.hpp"

enum Camera
{
    cameraLeft,
    cameraRight,
    cameraUnset
};

/**
 * A simple wrapper for the .AVI file interface.
 *
 */

class StereoAviFile
{
public:
    /** Default constructor. */
    StereoAviFile();

    /** Default destructor. */
    virtual ~StereoAviFile();

    /** Open an .avi file for reading. */
    bool open(const char *pszFilename, cv::Mat &stereoImgLeft, cv::Mat &stereoImgRight);

    //   /**
    //    * Read the next frame from the avi stream.  File must have been opened for
    //    * reading.
    //    *
    //    * @return false if read error, or last frame, true on success.
    //    * @bug Need a better return value.
    //    */
    //   bool readNextFrame();

    cv::Mat getFrame(enum Camera camera);

    // return iplImage for using in openCV
    cv::Mat readFrame(int index);

    /** Close the .avi file.  This is also done by the destructor. */
    bool close();

    //   /*
    //    * Converts the error to a string a posts a message.
    //    *
    //    * @param   hrErr      The resulting error.
    //    */
    //   void errorToString( HRESULT hrErr );

    enum Camera getCamera();

    void setCamera(enum Camera);

    // from here up the var were protected before

    /** Height, in pixels, of each frame in the .avi. */
    int m_iRows;

    /** Width, in pixels, of each frame in the .avi. */
    int m_iCols;

    /** Bits per pixel of the .avi. */
    int m_iBPP;

protected:
    enum Camera mCamera;

    /** Row increment, in bytes. */
    int m_iRowInc;

    /** Image size in bytes. */
    int m_iSize;

    /** Time index for current frame. */
    int m_iTimeIndex;

    /** Temporary image buffer. */
    unsigned char *m_pTempBuffer;

    // IplImage* mImage;
    cv::Mat mImageLeft;
    cv::Mat mImageRight;
    //   /** Temporary buffer for saving .bmps. */
    //
    //   /** Our bitmapinfo structure */
    //   BITMAPINFO* m_pBitmapInfo;

    //   /** avi file name */

private:
    /** Read the opened AVI-File */
    // VideoCapture m_vcReader;
    cv::VideoCapture m_vcReader;

    /** Writes to the opened AVI-File */
    // VideoWriter m_vcWriter;
};

#endif // STEREOAVIFILE_H
#endif // STEREO_DISABLED
