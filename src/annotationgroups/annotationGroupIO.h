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

#ifndef PETRACK_ANNOTATIONGROUPIO_H
#define PETRACK_ANNOTATIONGROUPIO_H

#include "annotationGroupManager.h"

#include <QJsonObject>


namespace annotationGroups
{

QString getFilename(const QString &lastDirectory = QString(), bool saving = true);


QJsonObject        configToJson(const GroupConfiguration &config);
GroupConfiguration configFromJson(const QJsonObject &json);

bool               writeConfigurationToFile(const GroupConfiguration &config, const QString &filename);
GroupConfiguration readConfigurationFromFile(const QString &filename);

} // namespace annotationGroups


#endif // PETRACK_ANNOTATIONGROUPIO_H
