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

#include "moCapPerson.h"

#include "helper.h"

#include <QDomElement>
#include <exception>

/**
 * @brief Gets index of sample at given time
 *
 * @param time timepoints in seconds
 * @return index of the sample for that timepoint (might be between two real samples)
 */
double MoCapPerson::getSampleIndex(double time) const
{
    return mMetadata.getSamplerate() * (time + mMetadata.getOffset());
}

void MoCapPerson::setSamplerate(double samplerate)
{
    mMetadata.setSamplerate(samplerate);
}

void MoCapPerson::setUserTimeOffset(double timeOffset)
{
    mMetadata.setUserTimeOffset(timeOffset);
}

void MoCapPerson::setFileTimeOffset(double timeOffset)
{
    mMetadata.setFileTimeOffset(timeOffset);
}

void MoCapPerson::addSkeleton(const SkeletonTree &skeleton)
{
    mSkeletons.push_back(skeleton);
}

const SkeletonTree &MoCapPerson::getSkeleton(size_t sample) const
{
    return mSkeletons.at(sample);
}

const std::string &MoCapPerson::getFilename() const
{
    return mMetadata.getFilepath();
}

const MoCapPersonMetadata &MoCapPerson::getMetadata() const
{
    return mMetadata;
}

bool MoCapPerson::isVisible() const
{
    return mMetadata.isVisible();
}

void MoCapPerson::setVisible(bool visible)
{
    mMetadata.setVisible(visible);
}

void MoCapPerson::setMetadata(const MoCapPersonMetadata &metadata)
{
    mMetadata = metadata;
}


void MoCapPerson::setXml(QDomElement &elem) const
{
    QDomElement subElem;
    subElem = elem.ownerDocument().createElement("PERSON");
    elem.appendChild(subElem);

    subElem.setAttribute("TIME_OFFSET", mMetadata.getUserTimeOffset());
    subElem.setAttribute("FILE", getFileList(QString::fromStdString(mMetadata.getFilepath())));
    subElem.setAttribute("SAMPLE_RATE", mMetadata.getSamplerate());
    subElem.setAttribute("SYSTEM", mMetadata.getSystem());
    subElem.setAttribute("VISIBLE", mMetadata.isVisible());
}

void MoCapStorage::addPerson(const MoCapPerson &person)
{
    mPersons.push_back(person);
}

void MoCapStorage::addPerson(MoCapPerson &&person)
{
    mPersons.push_back(person);
}
