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

#include "correction.h"
#include "petrack.h"

#include <QAbstractItemModelTester>
#include <catch2/catch_test_macros.hpp>

SCENARIO("Verify FailedChecksTableModel", "[correction]")
{
    Petrack pet{"Unknown"};

    FailedChecksTableModel *model = new FailedChecksTableModel(&pet);
    new QAbstractItemModelTester(model, QAbstractItemModelTester::FailureReportingMode::Fatal, &pet);
}
