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

#include "moCapController.h"

#include "IO.h"
#include "helper.h"
#include "moCapPerson.h"

#include <QDomElement>
#include <QMessageBox>
#include <opencv2/opencv.hpp>


bool operator==(const SegmentRenderData &lhs, const SegmentRenderData &rhs)
{
    return lhs.mColor == rhs.mColor && lhs.mDirected == rhs.mDirected && lhs.mLine == rhs.mLine &&
           lhs.mThickness == rhs.mThickness;
}

/**
 * @brief Transforms the skeleton of a given Person into SegmentRenderData
 *
 * This function takes a person and a timepoint (given as frame and framerate)
 * as input, and transforms the skeleton of the person at the given timepoint
 * into a list of lines (and one arrow for the head direction) to be drawn.
 *
 * If there is no sample for the given timepoint, it is inerpolated. Never
 * extrapolated.
 *
 * @param [in]person Person whose skeleton to transform into SegmentRenderDAta
 * @param [in]framerate Framerate of the video (NOT the mocap-recording)
 * @param [in]currentFrame current frame of the video
 * @param [out]renderData array of SegmentRenderData, to which to append the result
 */
void MoCapController::transformPersonSkeleton(
    const MoCapPerson &             person,
    double                          framerate,
    int                             currentFrame,
    std::vector<SegmentRenderData> &renderData) const
{
    double currentTime = currentFrame / framerate;
    double sampleIndex = person.getSampleIndex(currentTime);
    bool   hasPre      = person.hasSample(std::floor(sampleIndex));
    bool   hasPost     = person.hasSample(std::ceil(sampleIndex));

    if(!hasPre && !hasPost)
    {
        return;
    }
    else if(!hasPre)
    {
        debout << "Start of XSens recording reached for file '" << person.getFilename() << "'" << std::endl;
        return;
    }
    else if(!hasPost)
    {
        debout << "End of XSens recording reached for file '" << person.getFilename() << "'" << std::endl;
        return;
    }

    const SkeletonTree &preSample  = person.getSample(std::floor(sampleIndex));
    const SkeletonTree &postSample = person.getSample(std::ceil(sampleIndex));
    double              intpart    = 0.0;
    double              weight     = modf(sampleIndex, &intpart);

    const auto prePairs  = preSample.getLines();
    const auto postPairs = postSample.getLines();

    const std::vector<SkeletonLine> interpolatedPairs = interpolate(prePairs, postPairs, weight);

    std::vector<std::pair<cv::Point2f, cv::Point2f>> projectedPairs;
    projectedPairs.reserve(prePairs.size());

    SkeletonLine neckToHead;
    Vec3F        neckToHead3D;

    for(const auto &pair : interpolatedPairs)
    {
        projectedPairs.emplace_back(mExtrCalib.getImagePoint(pair.start), mExtrCalib.getImagePoint(pair.end));
        if(pair.start_id == 1 && pair.end_id == 2)
        {
            neckToHead   = pair;
            neckToHead3D = Vec3F(pair.end - pair.start);
        }
    }

    for(const auto &pair : projectedPairs)
    {
        renderData.push_back(
            {/*.mLine =*/QLine(pair.first.x, pair.first.y, pair.second.x, pair.second.y),
             /*.mColor =*/mColor,
             /*.mThickness =*/mThickness,
             /*.mDirected =*/false});
    }

    // Head Direction Arrow
    Vec3F headDir_v = preSample.getHeadDir() * (1 - weight) + postSample.getHeadDir() * weight;
    headDir_v.normalize();
    cv::Point3f headDir = cv::Point3f(headDir_v.x(), headDir_v.y(), headDir_v.z());
    headDir *= neckToHead3D.length();

    // Start arrow at 75% the way from C7 to top of head
    auto        arrowBase   = neckToHead.start + (neckToHead.end - neckToHead.start) * 0.75;
    cv::Point2f arrowHead   = mExtrCalib.getImagePoint(arrowBase + headDir);
    auto        arrowBase2D = mExtrCalib.getImagePoint(arrowBase);

    renderData.push_back(
        {/*.mLine =*/QLine(arrowBase2D.x, arrowBase2D.y, arrowHead.x, arrowHead.y),
         /*.mColor =*/mColor,
         /*.mThickness =*/mThickness,
         /*.mDirected =*/true});
}

std::vector<SegmentRenderData> MoCapController::getRenderData(int currentFrame, double framerate) const
{
    std::vector<SegmentRenderData> renderData;
    for(const auto &person : mStorage.getPersons())
    {
        transformPersonSkeleton(person, framerate, currentFrame, renderData);
    }
    return renderData;
}

std::vector<SkeletonLine> MoCapController::interpolate(
    const std::vector<SkeletonLine> &prePairs,
    const std::vector<SkeletonLine> &postPairs,
    double                           weight)
{
    // Needs: prePairs.size() == postPairs.size()
    // Needs: prePair and postPair be EXACTLY one sample away from each other
    std::vector<SkeletonLine> interpolatedPairs;
    interpolatedPairs.reserve(prePairs.size());

    for(size_t i = 0; i < prePairs.size(); ++i)
    {
        interpolatedPairs.push_back(
            {/*.start =*/prePairs[i].start * (1 - weight) + postPairs[i].start * weight,
             /*.start_id =*/prePairs[i].start_id,
             /*.end =*/prePairs[i].end * (1 - weight) + postPairs[i].end * weight,
             /*.end_id =*/prePairs[i].end_id});
    }
    return interpolatedPairs;
}

/**
 * @brief Sets visibility of moCap visualization
 *
 * @param visibility must be a bool
 * @emit showMoCapChanged event when visibility is different from mShowMoCap
 * */
void MoCapController::setShowMoCap(bool visibility)
{
    if(mShowMoCap != visibility)
    {
        mShowMoCap = visibility;
        emit showMoCapChanged(visibility);
    }
}

/**
 * @brief Sets color for moCap visualization
 *
 * @param color must be a valid QColor
 * @throw std::invalid_argument
 * @emit colorChanged event when color is different from mColor
 * */
void MoCapController::setColor(const QColor &color)
{
    if(!color.isValid())
    {
        throw std::invalid_argument("Selected color is invalid!");
    }
    if(mColor != color)
    {
        mColor = color;
        emit colorChanged(color);
    }
}

/**
 * @brief Sets thickness for moCap visualization
 *
 * @param thickness must greater than 0
 * @throw std::invalid_argument
 * @emit thicknessChanged event when thickness is different from mThickness
 * */
void MoCapController::setThickness(int thickness)
{
    if(thickness <= 0)
    {
        throw std::invalid_argument("Thickness must be greater than 0!");
    }
    if(mThickness != thickness)
    {
        mThickness = thickness;
        emit thicknessChanged(thickness);
    }
}

/**
 * @brief Emits update notification to all observers
 *
 * @emit showMoCapChanged, colorChanged, thicknessChanged
 * Notifies every observer manually to update the attributes
 * */
void MoCapController::notifyAllObserver()
{
    emit showMoCapChanged(mShowMoCap);
    emit colorChanged(mColor);
    emit thicknessChanged(mThickness);
}

/**
 * @return vector of MoCapPersonMetadata from the persons in mStorage
 */
std::vector<MoCapPersonMetadata> MoCapController::getAllMoCapPersonMetadata() const
{
    std::vector<MoCapPersonMetadata> metadata;
    for(const MoCapPerson &person : mStorage.getPersons())
    {
        metadata.push_back(person.getMetadata());
    }
    return metadata;
}


/**
 * @brief Reads those MoCapFiles whose new MoCapMetadata is different from the saved (and already read) Metadata.
 *
 * This method deletes every person from the storage whose metadata does not occur in the newMetadata and calls
 * the IO::readMoCapC3D-method for every new Metadata.
 */
void MoCapController::readMoCapFiles(const std::vector<MoCapPersonMetadata> &newMetadata)
{
    std::vector<MoCapPerson> &persons    = mStorage.getPersons();
    auto                      unselected = [&](MoCapPerson person)
    {
        return std::find_if(
                   newMetadata.cbegin(),
                   newMetadata.cend(),
                   [&person](const MoCapPersonMetadata &other)
                   { return readsTheSame(other, person.getMetadata()); }) == newMetadata.cend();
    };
    persons.erase(std::remove_if(persons.begin(), persons.end(), unselected), persons.end());
    std::vector<MoCapPersonMetadata> currentMetadata = getAllMoCapPersonMetadata();
    for(const MoCapPersonMetadata &md : newMetadata)
    {
        const auto isCurrentMD = [&md](const MoCapPersonMetadata &lhs) { return readsTheSame(lhs, md); };
        bool       isNewMd =
            std::find_if(currentMetadata.cbegin(), currentMetadata.cend(), isCurrentMD) == currentMetadata.cend();
        if(isNewMd)
        {
            IO::readMoCapC3D(mStorage, md);
        }
    }
}


/**
 * @brief Saves the setting and loaded MoCap-Data in Xml/Project-File
 *
 * Saves its own attributes and lets loaded Persons save themselves as
 * child-nodes.
 *
 * Results in the following Structure:
 *     <MOCAP SHOW="1" SIZE="2" COLOR="#ff0000" >
 *         <PERSON FILE="/example.c3d" SAMPLE_RATE="60" TIME_OFFSET="0.5" SYSTEM="0" />
 *         ...
 *     </MOCAP>
 *
 * @param elem MOCAP-Node
 */
void MoCapController::setXml(QDomElement &elem)
{
    elem.setAttribute("SHOW", mShowMoCap);
    elem.setAttribute("SIZE", mThickness);
    elem.setAttribute("COLOR", mColor.name());

    const auto &persons = mStorage.getPersons();
    for(const auto &person : persons)
    {
        person.setXml(elem);
    }
}

/**
 * @brief Reads in a previously saved MOCAP Xml-Node
 *
 * Sets own members according to saved values and reads/constucts
 * MoCapPersons according to the saved values. If the data for the
 * MoCapPersons is invalid, a MessageBox with the exception-msg is
 * shown to the user and the reading of MoCap-files is aborted.
 *
 * @param elem MOCAP-Node
 */
void MoCapController::getXml(const QDomElement &elem)
{
    // NOTE: Maybe only load the c3d-files once show is activated
    if(elem.hasAttribute("SHOW"))
    {
        bool isVisible = elem.attribute("SHOW").toInt();
        setShowMoCap(isVisible);
    }
    if(elem.hasAttribute("COLOR"))
    {
        QColor color(elem.attribute("COLOR"));
        setColor(color);
    }
    if(elem.hasAttribute("SIZE"))
    {
        setThickness(elem.attribute("SIZE").toInt());
    }

    std::vector<MoCapPersonMetadata> savedMetadata;
    try
    {
        for(QDomElement subElem = elem.firstChildElement(); !subElem.isNull(); subElem = subElem.nextSiblingElement())
        {
            if(subElem.tagName() == "PERSON")
            {
                MoCapPersonMetadata metadata;
                bool                ok;
                QString             path;
                if(subElem.hasAttribute("FILE") && subElem.hasAttribute("SYSTEM"))
                {
                    path               = subElem.attribute("FILE");
                    const QString file = getExistingFile(path);
                    path               = path.split(";").size() == 2 ? path.split(";").at(1) :
                                                                       "Saved path is invalid"
                                                         "";
                    int system         = subElem.attribute("SYSTEM").toInt(&ok);

                    if(!ok)
                    {
                        std::stringstream ss;
                        ss << "Element SYSTEM of " << path << " does not contain a valid integer!";
                        throw std::invalid_argument(ss.str());
                    }
                    if(system < 0 || system >= MoCapSystem::END)
                    {
                        std::stringstream ss;
                        ss << "System index " << system << " of file " << path
                           << " is not associated with an implemented MoCap system.";
                        throw std::invalid_argument(ss.str());
                    }

                    metadata.setFilepath(file.toStdString(), static_cast<MoCapSystem>(system));
                }
                if(subElem.hasAttribute("TIME_OFFSET"))
                {
                    metadata.setUserTimeOffset(subElem.attribute("TIME_OFFSET").toDouble(&ok));
                    std::stringstream ss;
                    ss << "Element TIME_OFFSET of file " << path << " does not contain a valid floating-point number!";
                    if(!ok)
                        throw std::invalid_argument(ss.str());
                }
                if(subElem.hasAttribute("SAMPLE_RATE"))
                {
                    metadata.setSamplerate(subElem.attribute("SAMPLE_RATE").toInt(&ok));
                    std::stringstream ss;
                    ss << "Element SAMPLE_RATE of file " << path << " does not contain a valid integer!";
                    if(!ok)
                        throw std::invalid_argument(ss.str());
                }
                savedMetadata.push_back(metadata);
            }
        }
        readMoCapFiles(savedMetadata);
    }
    catch(const std::exception &e)
    {
        std::stringstream ss;
        ss << "Problem reading the C3D-file(s): " << e.what();
        debout << ss.str() << std::endl;
        QMessageBox::critical(nullptr, tr("Error"), QString::fromStdString(ss.str()));
    }
}
