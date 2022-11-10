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

const std::string &MoCapPersonMetadata::getFilepath() const
{
    return mFilepath;
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
    return mUserTimeOffset + mFileTimeOffset;
}

double MoCapPersonMetadata::getUserTimeOffset() const
{
    return mUserTimeOffset;
}

bool operator==(const MoCapPersonMetadata &lhs, const MoCapPersonMetadata &rhs)
{
    return (
        lhs.getFilepath().compare(rhs.getFilepath()) == 0 && std::abs(lhs.getOffset() - rhs.getOffset()) < 1e-4 &&
        std::abs(lhs.getSamplerate() - rhs.getSamplerate()) < 1e-4 && lhs.getSystem() == rhs.getSystem());
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
