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
#ifndef MOCAPPERSONMETADATA_H
#define MOCAPPERSONMETADATA_H

#include "IO.h"

enum MoCapSystem{
    XSensC3D = 0,
    END ///< Used for checks before casting an int to MoCapSystem, must always stay at the end!
};
/** This map maps different MoCapSystems to an appropriate file extension. It is used for validating a given filepath.
     * in the OpenMoCapDialog and MoCapPersonMetadata and should be expanded with every new MoCapSystem.*/
const std::map<MoCapSystem, std::string> moCapFileExtensions = {{MoCapSystem::XSensC3D, "c3d"}};


/**
 * @brief Metadata of a MoCap-Recording
 *
 * The Metadata (Samplerate, time-offset, file/path, ...) of a given
 * MoCap-Recording. This class identifies a single recording and
 * includes all data needed by PeTrack to (re)load a MoCap-Recording.
 */
class MoCapPersonMetadata{
public:
    MoCapPersonMetadata() = default;
    MoCapPersonMetadata(const MoCapPersonMetadata&) = default;
    MoCapPersonMetadata(MoCapPersonMetadata&&) = default;
    MoCapPersonMetadata& operator=(const MoCapPersonMetadata&) = default;
    MoCapPersonMetadata& operator=(MoCapPersonMetadata&&) = default;
    ~MoCapPersonMetadata() = default;
    MoCapPersonMetadata(std::string filepath, MoCapSystem system, double samplerate, double offset);

    void setFilepath(const std::string &filepath, MoCapSystem system);
    void setSamplerate(double samplerate);
    void setOffset(double offset);
    void setMetadata(const std::string &filepath, MoCapSystem, double samplerate, double offset);
    MoCapSystem getSystem() const;
    double getSamplerate() const;
    double getOffset() const;
    const std::string &getFilepath() const;

private:
    std::string mFilepath = "";
    MoCapSystem mSystem = XSensC3D;
    double mSamplerate = 60;
    double mOffset = 0; ///< time offset from MoCap to video in seconds
};

bool operator==(const MoCapPersonMetadata &lhs, const MoCapPersonMetadata &rhs);
bool operator!=(const MoCapPersonMetadata &lhs, const MoCapPersonMetadata &rhs);
#endif // MOCAPPERSONMETADATA_H
