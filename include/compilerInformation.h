/*
 * PeTrack - Software for tracking pedestrians movement in videos
 * Copyright (C) 2010-2021 Forschungszentrum Jülich GmbH,
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

#ifndef COMPILERINFORMATION_H
#define COMPILERINFORMATION_H

// Taken from:
//  https://stackoverflow.com/questions/38530981/output-compiler-version-in-a-c-program#38531037
std::string versionString(int major, int minor, int patch)
{
    std::ostringstream ss;
    ss << major << '.' << minor << '.' << patch;
    return ss.str();
}

// Taken from:
// https://sourceforge.net/p/predef/wiki/Compilers/
constexpr const char *COMPILER_ID
{
#ifdef __clang__
    "clang++"
#elif defined(__GNUC__)
#if defined(__MINGW32__)
    "g++(MinGW)"
#else
    "g++"
#endif
#elif defined(_MSC_VER)
    "Visual Studio"
#else
    "Compiler not identified"
#endif
};

const std::string COMPILER_VERSION =
#ifdef __clang__
    versionString(__clang_major__, __clang_minor__, __clang_patchlevel__);
#elif defined(__GNUC__)
#if defined(__MINGW32__)
    versionString(__GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__) + "(" +
    versionString(__MINGW32__, __MINGW32_MAJOR_VERSION, __MINGW32_MINOR_VERSION) + ")";
#else
    versionString(__GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__);
#endif
#elif defined(_MSC_VER)
    versionString(_MSC_VER, _MSC_FULL_VER, _MSC_BUILD);
#else
    "";
#endif

const std::string COMPILE_TIMESTAMP = __TIMESTAMP__;

#endif // COMPILERINFORMATION_H
