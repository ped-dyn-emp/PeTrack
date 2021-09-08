/*
 * PeTrack - Software for tracking pedestrians movement in videos
 * Copyright (C) 2010-2020 Forschungszentrum Jülich GmbH,
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
#include "moCapPersonMetadata.h"

#include <QFileInfo>

MoCapPersonMetadata::MoCapPersonMetadata(std::string filepath, MoCapSystem system, double samplerate, double offset)
{
    setMetadata(filepath, system, samplerate, offset);
}


void MoCapPersonMetadata::setMetadata(const std::string &filepath, MoCapSystem system, double samplerate, double offset)
{
    setSamplerate(samplerate);
    setOffset(offset);
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

void MoCapPersonMetadata::setOffset(double offset)
{
    mOffset = offset;
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
    return mOffset;
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
