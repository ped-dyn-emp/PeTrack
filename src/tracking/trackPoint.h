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

#ifndef TRACKPOINT_H
#define TRACKPOINT_H

#include "recognition.h"
#include "vector.h"

#include <QColor>
#include <QTextStream>
#include <cstddef>
#include <limits>
#include <optional>
#include <spdlog/fmt/bundled/format.h>
#include <spdlog/spdlog.h>
#include <unordered_map>

struct ParseResult;

struct MultiColorMarker
{
    Vec2F  mColorPoint;
    QColor mColor;
};

struct CasernMarker
{
    Vec2F  mColorPoint;
    QColor mColor;
};

struct CodeMarker
{
    int       mMarkerId;
    cv::Vec3d mOrientation = {-1., -1., -1.};
};

struct JapanMarker
{
    Vec2F                      mColorPoint;
    inline static const QColor COLOR = QColor(79, 45, 52);
};

struct HermesMarker
{
};

struct StereoMarker
{
    Vec3F mStereoPoint;
};


class TrackPoint
{
public:
    TrackPoint();
    explicit TrackPoint(const Vec2F &pixelPoint);
    TrackPoint(const Vec2F &pixelPoint, int qual);
    TrackPoint(const TrackPoint &other);
    TrackPoint(TrackPoint &&other) noexcept;

    ~TrackPoint() { clearMarkers(); }

    const Vec2F &pixelPoint() const { return mPixelPoint; }
    double       x() const { return mPixelPoint.x(); }
    double       y() const { return mPixelPoint.y(); }
    int          qual() const { return mQuality; }
    double distanceToPoint(const TrackPoint &other) const { return mPixelPoint.distanceToPoint(other.mPixelPoint); }
    double distanceToPoint(const Vec2F &other) const { return mPixelPoint.distanceToPoint(other); }

    void setQual(int qual) { mQuality = qual; }
    void setX(double x) { mPixelPoint.setX(x); }
    void setY(double y) { mPixelPoint.setY(y); }

    void clearMarkers() // NOLINT: modifies static maps
    {
        multiColorMap.erase(mId);
        casernMap.erase(mId);
        codeMap.erase(mId);
        japanMap.erase(mId);
        hermesMap.erase(mId);
        stereoMap.erase(mId);
    }

    void shift(const Vec2F &vec);


    // factory for special trackPoints
    static TrackPoint
    createMultiColorTrackPoint(const Vec2F &pixelPoint, int qual, const Vec2F &colorPoint, const QColor &color);
    static TrackPoint
    createCasernTrackPoint(const Vec2F &pixelPoint, int qual, const Vec2F &colorPoint, const QColor &color);
    static TrackPoint createJapanTrackPoint(const Vec2F &pixelPoint, int qual, const Vec2F &colorPoint);
    static TrackPoint createHermesTrackPoint(const Vec2F &pixelPoint, int qual);
    static TrackPoint createStereoTrackPoint(const Vec2F &pixelPoint, int qual, const Vec3F &stereoPoint);

    void setMultiColorMarker(const MultiColorMarker &marker);
    void setCasernMarker(const CasernMarker &marker);
    void setCodeMarker(const CodeMarker &marker);
    void setJapanMarker(const JapanMarker &marker);
    void setHermesMarker(const HermesMarker &marker);
    void setStereoMarker(const StereoMarker &marker);

    void deleteColorMarkers() // NOLINT: modifies static maps
    {
        multiColorMap.erase(mId);
        casernMap.erase(mId);
        japanMap.erase(mId);
    }

    // getters for similar marker properties
    std::optional<QColor> getColorForHeightMap() const;
    std::optional<Vec2F>  getColorPointForOrientation() const;

    // getters for specific attributes of the marker (throw if the trackpoint does not have that marker attached)
    Vec3F stereoGetStereoPoint() const;


    void copyAllMarkersFromTrackPoint(const TrackPoint &trackPoint);


    std::optional<MultiColorMarker> getMultiColorMarker() const { return getMapEntry(multiColorMap); }
    std::optional<CasernMarker>     getCasernMarker() const { return getMapEntry(casernMap); }
    std::optional<CodeMarker>       getCodeMarker() const { return getMapEntry(codeMap); }
    std::optional<JapanMarker>      getJapanMarker() const { return getMapEntry(japanMap); }
    std::optional<HermesMarker>     getHermesMarker() const { return getMapEntry(hermesMap); }
    std::optional<StereoMarker>     getStereoMarker() const { return getMapEntry(stereoMap); }


    TrackPoint &operator=(const TrackPoint &other);
    TrackPoint &operator=(TrackPoint &&other) noexcept;
    TrackPoint &operator=(const Vec2F &vec);
    TrackPoint &operator+=(const Vec2F &vec);
    TrackPoint &operator-=(const Vec2F &vec);
    TrackPoint  operator+(const Vec2F &vec) const;
    TrackPoint  operator+(const TrackPoint &other) const;
    TrackPoint  operator-(const Vec2F &vec) const;
    TrackPoint  operator-(const TrackPoint &other) const;

    static constexpr int         MIN_DETECTION_QUAL  = 80;
    static constexpr int         BEST_DETECTION_QUAL = 100;
    static constexpr std::size_t SENTINEL            = std::numeric_limits<std::size_t>::max();
    [[nodiscard]] bool           isDetection() const;


private:
    inline static std::unordered_map<std::size_t, MultiColorMarker> multiColorMap;
    inline static std::unordered_map<std::size_t, CasernMarker>     casernMap;
    inline static std::unordered_map<std::size_t, CodeMarker>       codeMap;
    inline static std::unordered_map<std::size_t, JapanMarker>      japanMap;
    inline static std::unordered_map<std::size_t, HermesMarker>     hermesMap;
    inline static std::unordered_map<std::size_t, StereoMarker>     stereoMap;
    inline static std::size_t                                       nextId = 0;

    std::size_t mId = SENTINEL; // make sure id is always initialized! ub otherwise
    Vec2F       mPixelPoint;
    int         mQuality;


    template <typename T>
    std::optional<T> getMapEntry(const std::unordered_map<std::size_t, T> &map) const
    {
        if(auto iter = map.find(mId); iter != map.end())
        {
            return iter->second;
        }
        return std::nullopt;
    }

public:
    friend QTextStream &operator>>(QTextStream &stream, TrackPoint &trackPoint);
    template <typename Stream>
    static Stream &streamTrackPoint(Stream &stream, const TrackPoint &trackPoint, int trcVersion)
    {
        Vec3F stereoPoint = {-1., -1., -1.};
        if(auto stereoMarker = trackPoint.getStereoMarker())
        {
            stereoPoint = stereoMarker->mStereoPoint;
        }

        Vec2F  colorPoint;
        QColor color;
        if(auto casernMarker = trackPoint.getCasernMarker())
        {
            colorPoint = casernMarker->mColorPoint;
            color      = casernMarker->mColor;
        }
        else if(auto multiColorMarker = trackPoint.getMultiColorMarker())
        {
            colorPoint = multiColorMarker->mColorPoint;
            color      = multiColorMarker->mColor;
        }
        else if(auto japanMarker = trackPoint.getJapanMarker())
        {
            colorPoint = japanMarker->mColorPoint;
        }

        int markerId = -1;
        if(auto codeMarker = trackPoint.getCodeMarker())
        {
            markerId = codeMarker->mMarkerId;
        }


        if(trcVersion > 2)
        {
            stream << trackPoint.x() << " " << trackPoint.y() << " " << stereoPoint.x() << " " << stereoPoint.y() << " "
                   << stereoPoint.z() << " " << trackPoint.qual() << " " << colorPoint.x() << " " << colorPoint.y()
                   << " " << color << " " << markerId;
        }
        else if(trcVersion == 2)
        {
            stream << trackPoint.x() << " " << trackPoint.y() << " " << stereoPoint.x() << " " << stereoPoint.y() << " "
                   << stereoPoint.z() << " " << trackPoint.qual() << " " << colorPoint.x() << " " << colorPoint.y()
                   << " " << color;
        }
        else
        {
            stream << trackPoint.x() << " " << trackPoint.y() << " " << trackPoint.qual() << " " << colorPoint.x()
                   << " " << colorPoint.y() << " " << color;
        }
        return stream;
    }
    friend QTextStream  &operator<<(QTextStream &stream, const TrackPoint &trackPoint);
    friend std::ostream &operator<<(std::ostream &stream, const TrackPoint &trackPoint);
};


template <>
struct fmt::formatter<TrackPoint>
{
    constexpr auto parse(format_parse_context &ctx) -> decltype(ctx.begin()) { return ctx.end(); }

    template <typename FormatContext>
    auto format(const TrackPoint &input, FormatContext &ctx) const -> decltype(ctx.out())
    {
        std::stringstream stream;
        stream << input;
        return format_to(ctx.out(), "{}", stream.str());
    }
};

ParseResult
parseTrackPoint(QStringView line, int lineNumber, TrackPoint &trackPoint, reco::RecognitionMethod recoMethod);

#endif
