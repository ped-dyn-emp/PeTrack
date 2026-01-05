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

#include "importHelper.h"

#include <QAction>
#include <QCheckBox>
#include <QComboBox>
#include <QGroupBox>
#include <QTabWidget>

void loadBoolValueBlocked(const QDomElement &elem, const QString &name, QCheckBox *widget)
{
    QSignalBlocker blocker(widget);
    setValue(widget, readBool(elem, name));
}

void setValue(QCheckBox *widget, bool value)
{
    widget->setChecked(value);
}

double readDouble(const QDomElement &elem, const QString &name, const double &defaultVal)
{
    if(elem.hasAttribute(name))
    {
        bool   parsedSuccessfully = false;
        double value              = elem.attribute(name).toDouble(&parsedSuccessfully);
        if(!parsedSuccessfully)
        {
            throw std::domain_error(
                QString("Could not parse parameter %1 successfully. Value: %2, Node: %3. Ill-formed .pet-file.")
                    .arg(name)
                    .arg(value)
                    .arg(elem.tagName())
                    .toStdString());
        }
        return value;
    }
    return defaultVal;
}

double readDouble(const QDomElement &elem, const QString &name)
{
    if(elem.hasAttribute(name))
    {
        bool   parsedSuccessfully = false;
        double value              = elem.attribute(name).toDouble(&parsedSuccessfully);
        if(!parsedSuccessfully)
        {
            throw std::domain_error(
                QString("Could not parse parameter %1 successfully. Value: %2, Node: %3. Ill-formed .pet-file.")
                    .arg(name)
                    .arg(value)
                    .arg(elem.tagName())
                    .toStdString());
        }
        return value;
    }
    throw std::domain_error(QString("Required parameter %1 is missing in the .pet-file.").arg(name).toStdString());
}

int readInt(const QDomElement &elem, const QString &name, const int defaultVal)
{
    if(elem.hasAttribute(name))
    {
        bool parsedSuccessfully = false;
        int  value              = elem.attribute(name).toInt(&parsedSuccessfully);
        if(!parsedSuccessfully)
        {
            throw std::domain_error(
                QString("Could not parse parameter %1 successfully. Value: %2, Node: %3. Ill-formed .pet-file.")
                    .arg(name)
                    .arg(value)
                    .arg(elem.tagName())
                    .toStdString());
        }
        return value;
    }
    return defaultVal;
}

int readInt(const QDomElement &elem, const QString &name)
{
    if(elem.hasAttribute(name))
    {
        bool parsedSuccessfully = false;
        int  value              = elem.attribute(name).toInt(&parsedSuccessfully);
        if(!parsedSuccessfully)
        {
            throw std::domain_error(
                QString("Could not parse parameter %1 successfully. Value: %2, Node: %3. Ill-formed .pet-file.")
                    .arg(name)
                    .arg(value)
                    .arg(elem.tagName())
                    .toStdString());
        }
        return value;
    }
    throw std::domain_error(QString("Required parameter %1 is missing in the .pet-file.").arg(name).toStdString());
}

QString readQString(const QDomElement &elem, const QString &name, const QString &defaultVal)
{
    if(elem.hasAttribute(name))
    {
        return elem.attribute(name);
    }
    return defaultVal;
}

QString readQString(const QDomElement &elem, const QString &name)
{
    if(elem.hasAttribute(name))
    {
        return elem.attribute(name);
    }
    throw std::domain_error(QString("Required parameter %1 is missing in the .pet-file.").arg(name).toStdString());
}

bool readBool(const QDomElement &elem, const QString &name, const bool &defaultVal)
{
    if(elem.hasAttribute(name))
    {
        bool parsedSuccessfully = false;
        int  value              = elem.attribute(name).toInt(&parsedSuccessfully);
        if(!parsedSuccessfully)
        {
            throw std::domain_error(
                QString("Could not parse parameter %1 successfully. Value: %2, Node: %3. Ill-formed .pet-file.")
                    .arg(name)
                    .arg(value)
                    .arg(elem.tagName())
                    .toStdString());
        }
        // implicitely casting int to bool otherwise returns true for every number except 0
        if(value == 1 || value == Qt::Checked)
        {
            return true;
        }
        if(value == 0)
        {
            return false;
        }
        throw std::domain_error(
            QString("Could not parse parameter %1 successfully. Value: %2, Node: %3. Ill-formed .pet-file.")
                .arg(name)
                .arg(value)
                .arg(elem.tagName())
                .toStdString());
    }
    return defaultVal;
}

bool readBool(const QDomElement &elem, const QString &name)
{
    if(elem.hasAttribute(name))
    {
        bool parsedSuccessfully = false;
        int  value              = elem.attribute(name).toInt(&parsedSuccessfully);
        if(!parsedSuccessfully)
        {
            throw std::domain_error(
                QString("Could not parse parameter %1 successfully. Value: %2, Node: %3. Ill-formed .pet-file.")
                    .arg(name)
                    .arg(value)
                    .arg(elem.tagName())
                    .toStdString());
        }
        // implicitely casting int to bool otherwise returns true for every number except 0
        if(value == 1 || value == Qt::Checked)
        {
            return true;
        }
        else if(value == 0)
        {
            return false;
        }
        else
        {
            throw std::domain_error(
                QString("Could not parse parameter %1 successfully. Value: %2, Node: %3. Ill-formed .pet-file.")
                    .arg(name)
                    .arg(value)
                    .arg(elem.tagName())
                    .toStdString());
        }
    }
    throw std::domain_error(QString("Required parameter %1 is missing in the .pet-file.").arg(name).toStdString());
}

void setValueBlocked(QCheckBox *widget, bool value)
{
    QSignalBlocker blocker(widget);
    widget->setChecked(value);
}

void loadValue(const QDomElement &elem, const QString &name, QCheckBox *widget)
{
    setValue(widget, readBool(elem, name));
}

void loadActiveIndex(const QDomElement &elem, const QString &name, QComboBox *widget, const int defaultVal)
{
    widget->setCurrentIndex(readInt(elem, name, defaultVal));
}

void loadActiveIndex(const QDomElement &elem, const QString &name, QComboBox *widget)
{
    widget->setCurrentIndex(readInt(elem, name));
}

void loadActiveIndex(const QDomElement &elem, const QString &name, QTabWidget *widget, const int defaultVal)
{
    widget->setCurrentIndex(readInt(elem, name, defaultVal));
}

void loadActiveIndex(const QDomElement &elem, const QString &name, QTabWidget *widget)
{
    widget->setCurrentIndex(readInt(elem, name));
}

void loadBoolValue(const QDomElement &elem, const QString &name, QGroupBox *widget, const bool defaultVal)
{
    widget->setChecked(readBool(elem, name, defaultVal));
}

void loadBoolValue(const QDomElement &elem, const QString &name, QGroupBox *widget)
{
    widget->setChecked(readBool(elem, name));
}

void loadBoolValue(const QDomElement &elem, const QString &name, QAction *widget, const bool defaultVal)
{
    widget->setChecked(readBool(elem, name, defaultVal));
}
void loadBoolValue(const QDomElement &elem, const QString &name, QAction *widget)
{
    widget->setChecked(readBool(elem, name));
}