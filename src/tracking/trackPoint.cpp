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

#include "trackPoint.h"

#include "petrack.h"

#include <utility>

TrackPoint::TrackPoint() : mId(nextId++), mQuality(0) {}
TrackPoint::TrackPoint(const Vec2F &pixelPoint) : mId(nextId++), mPixelPoint(pixelPoint), mQuality(0) {}
TrackPoint::TrackPoint(const Vec2F &pixelPoint, int qual) : mId(nextId++), mPixelPoint(pixelPoint), mQuality(qual) {}

TrackPoint::TrackPoint(const TrackPoint &other)
{
    *this = other;
}

TrackPoint::TrackPoint(TrackPoint &&other) noexcept
{
    *this = std::move(other);
}


/**
 * @brief Shifts the pixelPoint and colorPoints of the trackPoint with a given vector
 * @param vec vector to add for the shift
 */
void TrackPoint::shift(const Vec2F &vec)
{
    mPixelPoint += vec;
    if(auto marker = getMultiColorMarker())
    {
        marker->mColorPoint += vec;
        setMultiColorMarker(*marker);
    }
    if(auto marker = getCasernMarker())
    {
        marker->mColorPoint += vec;
        setCasernMarker(*marker);
    }
    if(auto marker = getJapanMarker())
    {
        marker->mColorPoint += vec;
        setJapanMarker(*marker);
    }
}


TrackPoint
TrackPoint::createMultiColorTrackPoint(const Vec2F &pixelPoint, int qual, const Vec2F &colorPoint, const QColor &color)
{
    TrackPoint       res{pixelPoint, qual};
    MultiColorMarker marker{colorPoint, color};
    res.setMultiColorMarker(marker);
    return res;
}

TrackPoint
TrackPoint::createCasernTrackPoint(const Vec2F &pixelPoint, int qual, const Vec2F &colorPoint, const QColor &color)
{
    TrackPoint   res{pixelPoint, qual};
    CasernMarker marker{colorPoint, color};
    res.setCasernMarker(marker);
    return res;
}
TrackPoint TrackPoint::createJapanTrackPoint(const Vec2F &pixelPoint, int qual, const Vec2F &colorPoint)
{
    TrackPoint  res{pixelPoint, qual};
    JapanMarker marker{colorPoint};
    res.setJapanMarker(marker);
    return res;
}
TrackPoint TrackPoint::createHermesTrackPoint(const Vec2F &pixelPoint, int qual)
{
    TrackPoint   res{pixelPoint, qual};
    HermesMarker marker{};
    res.setHermesMarker(marker);
    return res;
}
TrackPoint TrackPoint::createStereoTrackPoint(const Vec2F &pixelPoint, int qual, const Vec3F &stereoPoint)
{
    TrackPoint   res{pixelPoint, qual};
    StereoMarker marker{stereoPoint};
    res.setStereoMarker(marker);
    return res;
}


void TrackPoint::setMultiColorMarker(const MultiColorMarker &marker) // NOLINT: modifies static maps
{
    if(marker.mColor.isValid())
    {
        multiColorMap[mId] = marker;
    }
    else
    {
        multiColorMap.erase(mId);
    }
}

void TrackPoint::setCasernMarker(const CasernMarker &marker) // NOLINT: modifies static maps
{
    if(marker.mColor.isValid())
    {
        casernMap[mId] = marker;
    }
    else
    {
        casernMap.erase(mId);
    }
}

void TrackPoint::setCodeMarker(const CodeMarker &marker) // NOLINT: modifies static maps
{
    codeMap[mId] = marker;
}

void TrackPoint::setJapanMarker(const JapanMarker &marker) // NOLINT: modifies static maps
{
    japanMap[mId] = marker;
}
void TrackPoint::setHermesMarker(const HermesMarker &marker) // NOLINT: modifies static maps
{
    hermesMap[mId] = marker;
}
void TrackPoint::setStereoMarker(const StereoMarker &marker) // NOLINT: modifies static maps
{
    if(marker.mStereoPoint.z() >= 0)
    {
        stereoMap[mId] = marker;
    }
    else
    {
        stereoMap.erase(mId);
    }
}


std::optional<QColor> TrackPoint::getColorForHeightMap() const
{
    std::optional<QColor> result;
    int                   foundCount = 0;

    auto checkColor = [&](const auto &marker)
    {
        if(marker && ++foundCount > 1)
        {
            throw std::runtime_error("Multiple colors found for trackpoint!");
        }
        if(marker)
        {
            result = marker->mColor;
        }
    };

    if(auto marker = getMultiColorMarker())
    {
        checkColor(marker);
    }
    if(auto marker = getCasernMarker())
    {
        checkColor(marker);
    }

    return result;
}

std::optional<Vec2F> TrackPoint::getColorPointForOrientation() const
{
    std::optional<Vec2F> result;
    int                  foundCount = 0;

    auto checkColorPoint = [&](const auto &marker)
    {
        if(marker && ++foundCount > 1)
        {
            throw std::runtime_error("Multiple color points found for trackpoint!");
        }
        if(marker)
        {
            result = marker->mColorPoint;
        }
    };

    if(auto marker = getCasernMarker())
    {
        checkColorPoint(marker);
    }
    if(auto marker = getJapanMarker())
    {
        checkColorPoint(marker);
    }

    return result;
}


void TrackPoint::copyAllMarkersFromTrackPoint(const TrackPoint &trackPoint)
{
    if(const auto multiColorMarker = trackPoint.getMultiColorMarker())
    {
        setMultiColorMarker(MultiColorMarker(*multiColorMarker));
    }
    if(const auto casernMarker = trackPoint.getCasernMarker())
    {
        setCasernMarker(CasernMarker(*casernMarker));
    }
    if(const auto codeMarker = trackPoint.getCodeMarker())
    {
        setCodeMarker(CodeMarker(*codeMarker));
    }
    if(const auto japanMarker = trackPoint.getJapanMarker())
    {
        setJapanMarker(JapanMarker(*japanMarker));
    }
    if(const auto hermesMarker = trackPoint.getHermesMarker())
    {
        setHermesMarker(HermesMarker(*hermesMarker));
    }
    if(const auto stereoMarker = trackPoint.getStereoMarker())
    {
        setStereoMarker(StereoMarker(*stereoMarker));
    }
}


TrackPoint &TrackPoint::operator=(const Vec2F &vec)
{
    mPixelPoint = vec;
    return *this;
}

TrackPoint &TrackPoint::operator=(const TrackPoint &other)
{
    if(this == &other)
    {
        return *this;
    }
    mId         = nextId++;
    mPixelPoint = other.mPixelPoint;
    mQuality    = other.mQuality;
    clearMarkers(); // clear all old markers (we only want the new ones)
    copyAllMarkersFromTrackPoint(other);
    return *this;
}


TrackPoint &TrackPoint::operator=(TrackPoint &&other) noexcept
{
    if(this == &other)
    {
        return *this;
    }
    mPixelPoint = other.mPixelPoint;
    mQuality    = other.mQuality;
    clearMarkers();
    mId = std::exchange(other.mId, TrackPoint::SENTINEL);

    return *this;
}

TrackPoint &TrackPoint::operator+=(const Vec2F &vec)
{
    mPixelPoint += vec;
    return *this;
}

TrackPoint &TrackPoint::operator-=(const Vec2F &vec)
{
    mPixelPoint -= vec;
    return *this;
}

TrackPoint TrackPoint::operator+(const Vec2F &vec) const
{
    return TrackPoint(*this) += vec;
}

TrackPoint TrackPoint::operator+(const TrackPoint &other) const
{
    return TrackPoint(*this) += other.pixelPoint();
}

TrackPoint TrackPoint::operator-(const Vec2F &vec) const
{
    return TrackPoint(*this) -= vec;
}

TrackPoint TrackPoint::operator-(const TrackPoint &other) const
{
    return TrackPoint(*this) -= other.pixelPoint();
}

bool TrackPoint::isDetection() const
{
    return mQuality > MIN_DETECTION_QUAL;
}

ParseResult
parseTrackPoint(QStringView line, int lineNumber, TrackPoint &trackPoint, reco::RecognitionMethod recoMethod)
{
    TrcLineParser parser(line, lineNumber);
    int           expectedTokens = 8; // base: x, y, color(r,g,b), qual, colPointX, colPointY
    if(Petrack::trcVersion > 1)
    {
        expectedTokens += 3;
    }; // stereo point x, y, z
    if(Petrack::trcVersion > 2)
    {
        expectedTokens += 1;
    }; // markerID
    auto result = parser.validateTokenCount(expectedTokens);
    if(!result.success)
    {
        return result;
    }

    // parse coordinates
    double x, y;
    result = parser.parseDouble(x);
    if(!result.success)
    {
        return result;
    }
    trackPoint.setX(x);

    result = parser.parseDouble(y);
    if(!result.success)
    {
        return result;
    }
    trackPoint.setY(y);

    // parse stereoPoint
    if(Petrack::trcVersion > 1)
    {
        Vec3F  sp;
        double spX, spY, spZ;
        result = parser.parseDouble(spX);
        if(!result.success)
        {
            return result;
        }
        sp.setX(spX);

        result = parser.parseDouble(spY);
        if(!result.success)
        {
            return result;
        }
        sp.setY(spY);

        result = parser.parseDouble(spZ);
        if(!result.success)
        {
            return result;
        }
        sp.setZ(spZ);
        trackPoint.setStereoMarker({sp});
    }
    // parse quality
    int qual;
    result = parser.parseInt(qual);
    if(!result.success)
    {
        return result;
    }
    trackPoint.setQual(qual);

    // parse colorPoint
    Vec2F  colorPoint;
    double colX, colY;
    result = parser.parseDouble(colX);
    if(!result.success)
    {
        return result;
    }
    colorPoint.setX(colX);

    result = parser.parseDouble(colY);
    if(!result.success)
    {
        return result;
    }
    colorPoint.setY(colY);

    // parse color
    QColor col;
    result = parser.parseColor(col);
    if(!result.success)
    {
        return result;
    }

    if(recoMethod == reco::RecognitionMethod::Casern)
    {
        trackPoint.setCasernMarker({colorPoint, col});
    }
    else if(recoMethod == reco::RecognitionMethod::Japan)
    {
        trackPoint.setJapanMarker({colorPoint});
    }
    else // should be multicolor
    {
        trackPoint.setMultiColorMarker({colorPoint, col});
    }

    // parse markerID
    if(Petrack::trcVersion > 2)
    {
        CodeMarker codeMarker;
        result = parser.parseInt(codeMarker.mMarkerId);
        if(!result.success)
        {
            return result;
        }
        trackPoint.setCodeMarker(codeMarker);
    }

    // everything successfull
    return {};
}


QTextStream &operator<<(QTextStream &stream, const TrackPoint &trackPoint)
{
    return TrackPoint::streamTrackPoint(stream, trackPoint, Petrack::trcVersion);
}

std::ostream &operator<<(std::ostream &stream, const TrackPoint &trackPoint)
{
    return TrackPoint::streamTrackPoint(stream, trackPoint, Petrack::trcVersion);
}
