/*
 * PeTrack - Software for tracking pedestrians movement in videos
 * Copyright (C) 2026 Forschungszentrum Jülich GmbH, IAS-7
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
#ifndef MOCAPPERSON_H
#define MOCAPPERSON_H

#include "moCapPersonMetadata.h"
#include "skeletonTree.h"

#include <vector>


class QDomElement;

/**
 * @brief The MoCapPerson class saves the MoCap recording of a whole person.
 *
 * This class saves the whole data of a persons motion capture output. This
 * is not done at the resolution of the system, but at the resolution of the
 * internal data structure, i.e. only points which are important for drawing
 * are saved.
 */
class MoCapPerson
{
public:
    double       getSampleIndex(double time) const;
    inline bool  hasSample(size_t index) const { return index < mSkeletons.size(); }
    SkeletonTree getSample(size_t sample) const;

    void                       setSamplerate(double samplerate);
    void                       setUserTimeOffset(double timeOffset);
    void                       setFileTimeOffset(double timeOffset);
    void                       setTranslation(const cv::Vec3f &trans);
    void                       setRotation(double angle);
    void                       setMetadata(const MoCapPersonMetadata &metadata);
    void                       addSkeleton(const SkeletonTree &skeleton);
    const SkeletonTree        &getSkeleton(size_t samples) const;
    const std::string         &getFilename() const;
    const MoCapPersonMetadata &getMetadata() const;
    bool                       isVisible() const;
    void                       setVisible(bool visible);


    void setXml(QDomElement &elem) const;

private:
    std::vector<SkeletonTree> mSkeletons;
    MoCapPersonMetadata       mMetadata;
};

class MoCapStorage
{
private:
    std::vector<MoCapPerson> mPersons;

public:
    void                            addPerson(const MoCapPerson &person);
    void                            addPerson(MoCapPerson &&person);
    std::vector<MoCapPerson>       &getPersons() { return mPersons; }
    const std::vector<MoCapPerson> &getPersons() const { return mPersons; }
};

#endif // MOCAPPERSON_H
