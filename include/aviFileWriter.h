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

#undef UNICODE

#ifndef AVIFILEWRITER_H
#define AVIFILEWRITER_H

#include <opencv2/opencv.hpp>

// Byte Number (nicht alle Zahlen gehen?: 2*1280*960*32*20) enstehen avi mit einem bild
// unter 2GB fuer reader und unter 4 GB fuer writer bleiben!!!
// 2*1280*960*32*20 = 1,5 GB = 40 Sekunden = 640 Bilder
inline constexpr int AVI_FILE_SPLIT_SIZE = ((2 * 1280 * 960 * 32 * 20));

/**
 * A simple wrapper for an .AVI file .
 */
class AviFileWriter
{
public:
    /** Default constructor. */
    AviFileWriter();

    /** Default destructor. */
    virtual ~AviFileWriter();

    /**
     * Open an AVI for writing.
     *
     * @param   iCols       Width, in pixels, of each frame.
     * @param   iRows       Hight, in pixels, of each frame.
     * @param   ibpp	   Bits per pixel -- 24 (BGR), 32 (BGRU). or 8 bit greyscale.
     * @param   dFramerate  Framerate that the .avi will play back in.
     */
    bool open(const char *pszFilename, int iCols, int iRows, int ibpp, double dFramerate);

    /**
     * Open an AVI for writing.  Deprecated.
     *
     * @param   iCols       Width, in pixels, of each frame.
     * @param   iRows       Hight, in pixels, of each frame.
     * @param   ibpp	   Bits per pixel -- 24 (BGR), 32 (BGRU). or 8 bit greyscale.
     * @param   iFramerate  Framerate that the .avi will play back in.
     */
    bool open(const char *pszFilename, int iCols, int iRows, int ibpp, int iFramerate);

    /**
     * Open an .avi for writing. The size of the avi file will be limited to SPLIT_SIZE bytes.
     * The file is splited automatically.
     *
     * @param   iCols       Width, in pixels, of each frame.
     * @param   iRows       Hight, in pixels, of each frame.
     * @param   ibpp	       Bits per pixel -- 24 (BGR), 32 (BGRU). or 8 bit greyscale.
     * @param   iFramerate  Framerate that the .avi will play back in.
     */
    bool openSizeLimitedAVI(const char *pszFilename, int iCols, int iRows, int ibpp, double iFramerate);

    /** Get the the bytes written */
    long int bytesWritten() const;

    /**
     * Load a bitmap from a file and append it to the current open .avi.
     * Must be in the correct format.
     */
    bool appendBMP(const char *pszFilename);

    /**
     * Add a frame (in the specified format) to the open .avi
     */
    bool appendFrame(const unsigned char *pBuffer, bool bInvert = true);

    /** Close the .avi file.  This is also done by the destructor. */
    bool close();

protected:
    /** Height, in pixels, of each frame in the .avi. */
    int m_iRows;

    /** Width, in pixels, of each frame in the .avi. */
    int m_iCols;

    /** Bits per pixel of the .avi. */
    int m_iBPP;

    /** Row increment, in bytes. */
    int m_iRowInc;

    /** Image size in bytes. */
    int m_iSize;

    /** Frame rate */
    double m_frameRate;

    /** Time index for current frame. */
    int m_iTimeIndex;

    /** Temporary image buffer. */
    unsigned char *m_pTempBuffer;

    /** Temporary image buffer */
    cv::Mat m_frame;

    /** Temporary buffer for saving .bmps. */
    unsigned char *m_pTempBMPBuffer;

    /** Avi file counter. */
    int m_iSplitFile;

    /** Flag indicating if the size of the avi file is limited to AVI_FILE_SPLIT_SIZE bytes */
    bool m_bSizeLimited;

    /** Total bytes written. */
    long int m_liBytesWritten;

    /** avi file name */
    std::string m_szAVIDestFile;

    char *m_fourCC;

    /** Defines is color avi */
    bool m_isColor;

private:
    /** Read and verify the OpenCV version. */
    bool checkOpenCVVersion();

    /** Read the opened AVI-File */
    cv::VideoCapture m_vReader;

    /** Writes to the opened AVI-File */
    cv::VideoWriter m_vWriter;
};

#endif // #ifndef AVIFILEWRITER_H
