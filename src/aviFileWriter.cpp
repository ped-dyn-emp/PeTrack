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

#include "aviFileWriter.h"

#include <cassert>
#include <cstdio>
#include <iostream>
#include <opencv2/videoio/videoio_c.h>

//
// Extra temp data bytes.
//
#define EXTRA_TEMP_SPACE 10000

AviFileWriter::AviFileWriter()
{
    m_pTempBMPBuffer = nullptr;
    m_pTempBuffer    = nullptr;

    m_iTimeIndex = 0;

    m_iCols     = 0;
    m_iRows     = 0;
    m_iBPP      = 0;
    m_iSize     = 0;
    m_iRowInc   = 0;
    m_frameRate = 0.0;

    m_bSizeLimited = false;
    m_iSplitFile   = 0;
    m_isColor      = true;

    m_liBytesWritten = 0;
}


AviFileWriter::~AviFileWriter()
{
    close();

    if(m_pTempBuffer != nullptr)
    {
        delete[] m_pTempBuffer;
        m_pTempBuffer = nullptr;
    }

    if(m_pTempBMPBuffer != nullptr)
    {
        delete[] m_pTempBMPBuffer;
        m_pTempBMPBuffer = nullptr;
    }
}


bool AviFileWriter::openSizeLimitedAVI(const char *pszFilename, int iCols, int iRows, int ibpp, double dFramerate)
{
    std::string szAVIFile;
    m_bSizeLimited = true;
    m_iSplitFile   = 0;

    std::stringstream stringStream;
    stringStream << pszFilename;
    szAVIFile = stringStream.str();

    // Remove any extensions.
    size_t lastindex = szAVIFile.find_first_of('.');
    szAVIFile        = szAVIFile.substr(0, lastindex);

    //
    // Keep the basic avi file name in m_szAVIDestFile.
    //
    stringStream.str("");
    stringStream << szAVIFile;
    m_szAVIDestFile = stringStream.str();

    //
    // The avi file names will be in the form  of ***_0000.avi, ***_0001.avi,
    // ***_0002.avi...
    // The size of each avi file is limited by AVI_FILE_SPLIT_SIZE bytes
    //
    stringStream.str("");
    stringStream << m_szAVIDestFile << "_" << std::setfill('0') << std::setw(4) << m_iSplitFile << ".avi";
    szAVIFile = stringStream.str();
    return open(szAVIFile.c_str(), iCols, iRows, ibpp, dFramerate);
}

bool AviFileWriter::open(const char *pszFilename, int iCols, int iRows, int ibpp, double dFramerate)
{
    //
    // Initialize the library.
    //
    //::AVIFileInit();

    if(!checkOpenCVVersion())
    {
        return false;
    }

    if(pszFilename == nullptr || iRows <= 0 || iCols <= 0 || dFramerate <= 0.0)
    {
        assert(false);
        return false;
    }

    m_frameRate      = dFramerate;
    m_iTimeIndex     = 0;
    m_liBytesWritten = 0;
    //
    // If these parameters change, set them and allocate the buffers
    //
    if(m_iCols != iCols || m_iRows != iRows || m_iBPP != ibpp)
    {
        //
        // Initialize members.
        //
        m_iCols   = iCols;
        m_iRows   = iRows;
        m_iBPP    = ibpp;
        m_iRowInc = m_iCols * (m_iBPP / 8);
        m_iSize   = m_iRows * m_iRowInc;
        //
        // Allocate temp image buffers.
        //
        if(m_pTempBMPBuffer != nullptr)
        {
            delete[] m_pTempBMPBuffer;
            m_pTempBMPBuffer = nullptr;
        }
        m_pTempBMPBuffer = new unsigned char[m_iSize + EXTRA_TEMP_SPACE];

        if(m_pTempBuffer != nullptr)
        {
            delete[] m_pTempBuffer;
            m_pTempBuffer = nullptr;
        }
        m_pTempBuffer = new unsigned char[m_iSize];
    }

    if(m_pTempBMPBuffer == nullptr || m_pTempBuffer == nullptr)
    {
        return false;
    }

    //
    // If this function is called from openSizeLimitedAVI(), pszFilename
    // is the file to open. m_szAVIDestFile should not be changed because
    // it is the base file name used in appendFrame() to generate the next
    // split file name.
    //
    // If this function is called directly (not by openSizeLimitedAVI()),
    // the AVI file can not be splited and m_szAVIDestFile is not used.
    // So we do not need to save the file name to m_szAVIDestFile.
    //
#if 0
   sprintf(m_szAVIDestFile, "%s", pszFilename );

   // Remove any extensions.
   char* pcExt = strstr( m_szAVIDestFile, "." );
   if( pcExt != NULL )
   {
      *( pcExt ) = 0x0;
   }

   // add an avi extension so that the file can be opened
   char szAVIFile[ _MAX_PATH ];
   sprintf(szAVIFile, "%s.avi", m_szAVIDestFile);
#endif

    return m_vWriter.open(pszFilename, CV_FOURCC_DEFAULT /*PROMPT*/, dFramerate, cv::Size(iCols, iRows), m_isColor);
}


bool AviFileWriter::open(const char *pszFilename, int iCols, int iRows, int /*ibpp*/, int iFramerate)
{
    return m_vWriter.open(pszFilename, CV_FOURCC_PROMPT, (double) iFramerate, cv::Size(iCols, iRows), m_isColor);
}

long int AviFileWriter::bytesWritten() const
{
    return m_liBytesWritten;
}

bool AviFileWriter::appendFrame(const unsigned char *pBuffer, bool /*bInvert*/)
{
    if(!m_vWriter.isOpened())
    {
        assert(false);
        return false;
    }


    unsigned char *pWriteBuffer = (unsigned char *) pBuffer;
    cv::Mat        frame;

    if((m_iRowInc / m_iCols) == 1)
    {
        frame = cv::Mat(m_iRows, m_iCols, CV_8UC1, pWriteBuffer, m_iRowInc); // = imread(pszFilename);
    }
    else if((m_iRowInc / m_iCols) == 3)
    {
        frame = cv::Mat(m_iRows, m_iCols, CV_8UC3, pWriteBuffer, m_iRowInc); // = imread(pszFilename);
    }
    else if((m_iRowInc / m_iCols) == 4)
    {
        frame = cv::Mat(m_iRows, m_iCols, CV_8UC4, pWriteBuffer, m_iRowInc); // = imread(pszFilename);
        cv::cvtColor(frame, frame, cv::COLOR_RGBA2RGB);                      // need for right image interpretation
    }
    else
    {
        std::cout << "error: unkown video format." << std::endl;
        return false;
    }
    if(!frame.data)
    {
        std::cout << "error: while saving video data." << std::endl;
        return false;
    }

    m_vWriter.write(frame);

    /**
     * ToDo: invertieren
     *
     */

    long lBytesWritten = 0;

    //
    // If the AVI file is opened with openSizeLimitedAVI(), split it if necessory.
    //
    if(m_bSizeLimited)
    {
        //
        // Split the file if it is more than 'AVI_FILE_SPLIT_SIZE' bytes
        //
        // if(bytesWritten() >= (__int64)(AVI_FILE_SPLIT_SIZE))
        if(bytesWritten() >= (long int) (AVI_FILE_SPLIT_SIZE))
        {
            std::string       szAVIFile;
            std::stringstream stringStream;
            close();
            m_iSplitFile++;
            stringStream << m_szAVIDestFile << "_" << std::setfill('0') << std::setw(4) << m_iSplitFile << ".avi";
            szAVIFile = stringStream.str();

            if(!open(szAVIFile.c_str(), m_iCols, m_iRows, m_iBPP, m_frameRate))
            {
                return false;
            }
        }
    }

    m_liBytesWritten += lBytesWritten;

    m_iTimeIndex++;

    return true;
}

bool AviFileWriter::close()
{
    m_vWriter.release();
    return true;
}


bool AviFileWriter::appendBMP(const char *pszFilename)
{
    cv::Mat bmpFrame = cv::imread(pszFilename);

    m_vWriter.write(bmpFrame);

    m_iTimeIndex++;

    return true;
}


bool AviFileWriter::checkOpenCVVersion()
{
    if constexpr(CV_MAJOR_VERSION < 3)
    {
        assert(false); // opencv version too old.
        return false;
    }
    return true;
}
