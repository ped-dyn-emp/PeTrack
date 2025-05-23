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
#ifndef LOGGER_H
#define LOGGER_H

#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_INFO

#include <QString>
#include <opencv2/core.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/core/operations.hpp>
#include <spdlog/fmt/bundled/format.h>
#include <spdlog/fmt/ostr.h>
#include <spdlog/sinks/ringbuffer_sink.h>
#include <spdlog/sinks/stdout_sinks.h>
#include <spdlog/spdlog.h>
#include <sstream>

namespace logger
{
// Note: when changing this, also adapt messageBoxLogFormat accordingly!
const std::string LOG_FORMAT              = "[%s:%#:%!][%l] %v";
const std::string MESSAGE_BOX_LOGGER_NAME = "pMessageBox";
const std::string MESSAGE_BOX_LOG_FORMAT  = "[{}:{}:{}][{}] {}";

inline void setupLogger()
{
    // add ringbuffer_sink to default logger to store messages for logwindow
    auto ringbufferSink = std::make_shared<spdlog::sinks::ringbuffer_sink_mt>(10);
    spdlog::default_logger()->sinks().push_back(ringbufferSink);

    // setup global logger, which should be used to display message on the command line only
    spdlog::set_level(spdlog::level::trace);

    spdlog::set_pattern(LOG_FORMAT);

    // setup logger, which is used when also a dialog is displayed to the user
    auto messageBoxLogger = spdlog::stdout_logger_mt("pMessageBox");
    messageBoxLogger->set_pattern("%v");
}
} // namespace logger

template <>
struct fmt::formatter<QString>
{
    constexpr auto parse(format_parse_context &ctx) -> decltype(ctx.begin()) { return ctx.end(); }

    template <typename FormatContext>
    auto format(const QString &input, FormatContext &ctx) const -> decltype(ctx.out())
    {
        return format_to(ctx.out(), "{}", input.toStdString());
    }
};

template <>
struct fmt::formatter<cv::Mat>
{
    constexpr auto parse(format_parse_context &ctx) -> decltype(ctx.begin()) { return ctx.end(); }

    template <typename FormatContext>
    auto format(const cv::Mat &mat, FormatContext &ctx) const -> decltype(ctx.out())
    {
        std::ostringstream s;
        s << cv::format(mat, cv::Formatter::FMT_NUMPY);
        return format_to(ctx.out(), "{}", s.str());
    }
};


#endif
