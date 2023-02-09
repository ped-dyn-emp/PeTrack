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
#ifndef LOGGER_H
#define LOGGER_H

#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_INFO

#include <QString>
#include <spdlog/fmt/bundled/format.h>
#include <spdlog/fmt/ostr.h>
#include <spdlog/sinks/stdout_sinks.h>
#include <spdlog/spdlog.h>

namespace logger
{
const std::string messageBoxLoggerName = "pMessageBox";
const std::string messageBoxLogFormat  = "[{}:{}:{}][{}] {}";

inline void setupLogger()
{
    // setup global logger, which should be used to display message on the command line only
    spdlog::set_level(spdlog::level::trace);

    // Note: when changing this, also adapt messageBoxLogFormat accordingly!
    spdlog::set_pattern("[%s:%#:%!][%l] %v");

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
    auto format(const QString &input, FormatContext &ctx) -> decltype(ctx.out())
    {
        return format_to(ctx.out(), "{}", input.toStdString());
    }
};

#endif
