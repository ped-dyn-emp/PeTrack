/*
 * PeTrack - Software for tracking pedestrians movement in videos
 * Copyright (C) 2023 Forschungszentrum Jülich GmbH, IAS-7
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
#include "moCapPersonMetadata.h"

#include <QFileInfo>

MoCapPersonMetadata::MoCapPersonMetadata(
    std::string filepath,
    MoCapSystem system,
    double      samplerate,
    double      userTimeOffset,
    double      fileTimeOffset)
{
    setMetadata(filepath, system, samplerate, userTimeOffset, fileTimeOffset);
}


void MoCapPersonMetadata::setMetadata(
    const std::string &filepath,
    MoCapSystem        system,
    double             samplerate,
    double             userTimeOffset,
    double             fileTimeOffset)
{
    setSamplerate(samplerate);
    setUserTimeOffset(userTimeOffset);
    setFileTimeOffset(fileTimeOffset);
    setFilepath(filepath, system);
}

/**
 *
 * @param filename: valid path with (e.g.'.c3d') extension @throws std::ios_base::failure
 */
void MoCapPersonMetadata::setFilepath(const std::string &filepath, MoCapSystem system)
{
    const std::string &extension = moCapFileExtensions.at(system);
    QFileInfo          info(QString::fromStdString(filepath));
    if(!info.exists())
    {
        std::stringstream msg;
        msg << "Invalid Filename '" << filepath << "': does not exist!\n";
        throw std::ios_base::failure(msg.str());
    }
    QString suffix = info.suffix();
    if(suffix.toStdString() != extension)
    {
        std::stringstream msg;
        msg << "Invalid Filename '" << filepath << "': It has no " << extension << " extension!\n";
        throw std::ios_base::failure(msg.str());
    }
    mFilepath = filepath;
    mSystem   = system;
}

/**
 * @param samplerate must be greater than zero. @throws std::invalid
 */
void MoCapPersonMetadata::setSamplerate(double samplerate)
{
    if(samplerate <= 0)
    {
        throw std::invalid_argument("Samplerate cannot be negative or zero!");
    }
    mSamplerate = samplerate;
}

void MoCapPersonMetadata::setUserTimeOffset(double offset)
{
    mUserTimeOffset = offset;
}

void MoCapPersonMetadata::setFileTimeOffset(double offset)
{
    mFileTimeOffset = offset;
}

void MoCapPersonMetadata::setAngle(double angle)
{
    // create rotation matrix rotating by angle in xy-plane
    // -angle because OpenCV is made for left-handed image coords
    auto rot2D = cv::getRotationMatrix2D({0, 0}, -angle, 1);
    rot2D.convertTo(rot2D, CV_32F);
    cv::Mat rot3D    = cv::Mat::eye({3, 3}, CV_32F);
    auto    rot_view = rot3D.rowRange(0, 2).colRange(0, 2);
    rot2D.rowRange(0, 2).colRange(0, 2).copyTo(rot_view);

    mRotation = cv::Affine3f(rot3D);
}

void MoCapPersonMetadata::setTranslation(const cv::Affine3f &trans)
{
    mTranslation = trans;
}


const std::string &MoCapPersonMetadata::getFilepath() const
{
    return mFilepath;
}

bool MoCapPersonMetadata::isVisible() const
{
    return mVisible;
}

void MoCapPersonMetadata::setVisible(bool newVisible)
{
    mVisible = newVisible;
}

const cv::Affine3f &MoCapPersonMetadata::getRotation() const
{
    return mRotation;
}

const cv::Affine3f &MoCapPersonMetadata::getTranslation() const
{
    return mTranslation;
}

/**
 * @brief Returns angle with which to rotate the person
 *
 * The person can be rotated around its rotation point. That is
 * usually the top of head. It is rotated in the xy-plane. This
 * is the angle the person is rotated by.
 *
 * @return returns angle in degrees
 */
double MoCapPersonMetadata::getAngle() const
{
    // assume perfect rotation matrix
    // [ cos a -sin a ]
    // [ sin a  cos a ]
    const auto &rot = mRotation.rotation();
    return atan2(rot(1, 0), rot(0, 0)) * (180 / CV_PI);
}

MoCapSystem MoCapPersonMetadata::getSystem() const
{
    return mSystem;
}

double MoCapPersonMetadata::getSamplerate() const
{
    return mSamplerate;
}

double MoCapPersonMetadata::getOffset() const
{
    return -mUserTimeOffset + mFileTimeOffset;
}

double MoCapPersonMetadata::getUserTimeOffset() const
{
    return mUserTimeOffset;
}

bool operator==(const MoCapPersonMetadata &lhs, const MoCapPersonMetadata &rhs)
{
    return (
        lhs.getFilepath().compare(rhs.getFilepath()) == 0 && std::abs(lhs.getOffset() - rhs.getOffset()) < 1e-4 &&
        std::abs(lhs.getSamplerate() - rhs.getSamplerate()) < 1e-4 && lhs.getSystem() == rhs.getSystem() &&
        lhs.isVisible() == rhs.isVisible());
}

bool operator!=(const MoCapPersonMetadata &lhs, const MoCapPersonMetadata &rhs)
{
    return !(lhs == rhs);
}

/**
 * @brief Indicates if both metadata result in the same Person after reading
 *
 * This function indicated whether the two given metadata would result in the
 * same person after calling IO::readMoCapC3D (or other) with them. That could be
 * the case, if everything is the same except the file offset, which is newly set
 * anyways.
 *
 * @param lhs first metadata
 * @param rhs second metadata
 * @return true if both result in same person after reading
 */
bool readsTheSame(const MoCapPersonMetadata &lhs, const MoCapPersonMetadata &rhs)
{
    return (
        lhs.getFilepath().compare(rhs.getFilepath()) == 0 &&
        std::abs(lhs.getUserTimeOffset() - rhs.getUserTimeOffset()) < 1e-4 &&
        std::abs(lhs.getSamplerate() - rhs.getSamplerate()) < 1e-4 && lhs.getSystem() == rhs.getSystem());
}
