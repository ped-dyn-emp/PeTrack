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

#ifndef STEREOAVIFILE_H
#define STEREOAVIFILE_H

#include "logger.h"

#include <opencv2/core/mat.hpp>
#include <stdexcept>

enum class Camera
{
    cameraLeft,
    cameraRight,
    cameraUnset
};

/// Stub for Point Grey avi file in case PeTrack is compiled without triclops support
class StereoAviFile
{
public:
    int  m_iRows;
    int  m_iCols;
    bool open(const char *, cv::Mat &, cv::Mat &)
    {
        SPDLOG_ERROR("Version was not compiled with triclops support!");
        return false;
    }

    cv::Mat getFrame(Camera) { throw std::runtime_error("Version was not compiled with triclops support!"); }

    cv::Mat readFrame(int) { throw std::runtime_error("Version was not compiled with triclops support!"); }


    bool close() { throw std::runtime_error("Version was not compiled with triclops support!"); }


    Camera getCamera() { throw std::runtime_error("Version was not compiled with triclops support!"); }


    void setCamera(Camera) { throw std::runtime_error("Version was not compiled with triclops support!"); }
};

#endif // STEREOAVIFILE_H
