/*
 * PeTrack - Software for tracking pedestrians movement in videos
 * Copyright (C) 2022 Forschungszentrum Jülich GmbH, IAS-7
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

#define CATCH_CONFIG_RUNNER
#include <QApplication>
#include <QtTest>
#include <catch2/catch.hpp>
#include <vector>

int main(int argc, char *argv[])
{
    // always start unit tests as offscreen, so PMessageBox-es only log
    bool platformAlreadyGiven = false;
    for(int i = 0; i < argc; ++i)
    {
        if(qstrcmp("-platform", argv[i]) == 0)
        {
            platformAlreadyGiven = true;
        }
    }
    int    argc2;
    char **argv2;
    auto   args = std::vector<const char *>(argv, argv + argc);
    if(!platformAlreadyGiven)
    {
        args.push_back("-platform");
        args.push_back("offscreen");
        argc2 = argc + 2;
        argv2 = const_cast<char **>(args.data());
    }
    else
    {
        argc2 = argc;
        argv2 = argv;
    }

    QApplication a(argc2, argv2);

    QTEST_SET_MAIN_SOURCE_PATH
    const int result = Catch::Session().run(argc2, argv2);

    return (result < 0xff ? result : 0xff);
}
