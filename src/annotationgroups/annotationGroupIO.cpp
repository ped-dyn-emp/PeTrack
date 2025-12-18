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

#include "annotationGroupIO.h"

#include "logger.h"

#include <QFileDialog>
#include <QJsonArray>
#include <QJsonDocument>
#include <QString>

namespace
{

using namespace annotationGroups;

Group parseGroup(const QJsonObject &json)
{
    Group group;

    group.id    = json["id"].toInt();
    group.name  = json["name"].toString().toStdString();
    group.type  = json["type"].toString().toStdString();
    group.tlgId = json["tlgId"].toInt();
    group.color = QColor(json["color"].toString());

    return group;
}

TopLevelGroup parseTopLevelGroup(const QJsonObject &json)
{
    return {json["id"].toInt(), json["name"].toString().toStdString()};
}

} // namespace


QString annotationGroups::getFilename(const QString &lastDirectory, bool saving)
{
    QFileDialog fileDialog(
        nullptr,
        QString("Select file for %1 Grouping data").arg(saving ? "exporting" : "importing"),
        lastDirectory,
        QString(
            "Petrack groups (*.json);;Text (*.txt);;All supported types (*.txt *.json);;All files "
            "(*.*)"));
    fileDialog.setAcceptMode(saving ? QFileDialog::AcceptSave : QFileDialog::AcceptOpen);
    fileDialog.setFileMode(QFileDialog::AnyFile);
    fileDialog.setDefaultSuffix("json");

    QString dest = "";

    if(fileDialog.exec())
    {
        dest = fileDialog.selectedFiles().at(0);
    }

    return dest;
}

namespace annotationGroups
{

QJsonObject configToJson(const GroupConfiguration &config)
{
    QJsonObject json;
    json["version"] = QString::fromStdString(config.version);

    QJsonArray groups;
    for(const auto &entry : config.mGroups)
    {
        QJsonObject group;

        group["id"]    = entry.id;
        group["name"]  = QString::fromStdString(entry.name);
        group["type"]  = QString::fromStdString(entry.type);
        group["tlgId"] = entry.tlgId;
        group["color"] = entry.color.name();

        groups.push_back(group);
    }
    json["groups"] = groups;

    QJsonArray tlgs;
    for(const auto &entry : config.mTopLevelGroups)
    {
        QJsonObject tlg;
        tlg["id"]   = entry.id;
        tlg["name"] = QString::fromStdString(entry.name);

        tlgs.push_back(tlg);
    }
    json["topLevelGroups"] = tlgs;

    QJsonArray assignments;
    for(const auto &entry : config.mTrajectoryAssignments)
    {
        QJsonObject assignment;
        assignment["trackPersonId"] = entry.first;

        QJsonArray intervals;
        for(const auto &interv : entry.second)
        {
            QJsonObject interval;
            interval["start"]   = std::get<0>(interv);
            interval["groupId"] = std::get<1>(interv);

            intervals.push_back(interval);
        }
        assignment["intervals"] = intervals;
        assignments.push_back(assignment);
    }

    json["assignments"] = assignments;

    return json;
}

GroupConfiguration configFromJson(const QJsonObject &json)
{
    GroupConfiguration config;

    config.version = json["version"].toString().toStdString();

    if(json.contains("groups") && json["groups"].isArray())
    {
        QJsonArray const groupArray = json["groups"].toArray();
        for(const auto &entry : groupArray)
        {
            config.mGroups.push_back(::parseGroup(entry.toObject()));
        }
    }

    if(json.contains("topLevelGroups") && json["topLevelGroups"].isArray())
    {
        QJsonArray const tlgArray = json["topLevelGroups"].toArray();
        for(const auto &entry : tlgArray)
        {
            config.mTopLevelGroups.push_back(::parseTopLevelGroup(entry.toObject()));
        }
    }

    if(json.contains("assignments") && json["assignments"].isArray())
    {
        QJsonArray const assignArray = json["assignments"].toArray();
        for(const auto &assignment : assignArray)
        {
            std::vector<std::tuple<int, int>> assignments;

            const auto assignObj = assignment.toObject();
            for(const auto &intervalJson : assignObj["intervals"].toArray())
            {
                const auto object = intervalJson.toObject();
                assignments.push_back({object["start"].toInt(), object["groupId"].toInt()});
            }
            config.mTrajectoryAssignments[assignObj["trackPersonId"].toInt()] = assignments;
        }
    }

    return config;
}

bool writeConfigurationToFile(const GroupConfiguration &config, const QString &filename)
{
    QJsonObject json = configToJson(config);

    QFile file{filename};

    if(file.open(QIODevice::WriteOnly))
    {
        file.write(QJsonDocument(json).toJson());
        return true;
    }

    return false;
}

GroupConfiguration readConfigurationFromFile(const QString &filename)
{
    QFile              file{filename};
    GroupConfiguration config;

    if(file.open(QIODevice::ReadOnly))
    {
        QByteArray    data     = file.readAll();
        QJsonDocument document = QJsonDocument::fromJson(data);

        return configFromJson(document.object());
    }

    return config;
}

} // namespace annotationGroups