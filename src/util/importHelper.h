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

#ifndef IMPORTHELPER_H
#define IMPORTHELPER_H

#include <QDomElement>
#include <QFileInfo>
#include <QTextStream>
#include <optional>
#include <sstream>

class QCheckBox;
class QComboBox;
class QGroupBox;
class QTabWidget;
class QAction;


template <typename T>
void setValue(T widget, double value);
void setValue(QCheckBox *widget, bool value);

double  readDouble(const QDomElement &elem, const QString &name, const double &defaultVal);
double  readDouble(const QDomElement &elem, const QString &name);
int     readInt(const QDomElement &elem, const QString &name, const int defaultVal);
int     readInt(const QDomElement &elem, const QString &name);
QString readQString(const QDomElement &elem, const QString &name, const QString &defaultVal);
QString readQString(const QDomElement &elem, const QString &name);
bool    readBool(const QDomElement &elem, const QString &name, const bool &defaultVal);
bool    readBool(const QDomElement &elem, const QString &name);

template <typename T>
void loadIntValue(const QDomElement &elem, const QString &name, T widget, const int defaultVal);
template <typename T>
void loadIntValue(const QDomElement &elem, const QString &name, T widget);
template <typename T>
void loadDoubleValue(const QDomElement &elem, const QString &name, T widget, const double defaultVal);
template <typename T>
void loadDoubleValue(const QDomElement &elem, const QString &name, T widget);
template <typename T>
void loadBoolValue(const QDomElement &elem, const QString &name, T widget);
template <typename T>
void loadBoolValue(const QDomElement &elem, const QString &name, T widget);
void loadBoolValue(const QDomElement &elem, const QString &name, QGroupBox *widget, bool defaultVal);
void loadBoolValue(const QDomElement &elem, const QString &name, QGroupBox *widget);
void loadBoolValue(const QDomElement &elem, const QString &name, QAction *widget, bool defaultVal);
void loadBoolValue(const QDomElement &elem, const QString &name, QAction *widget);
void loadActiveIndex(const QDomElement &elem, const QString &name, QComboBox *widget, const int defaultVal);
void loadActiveIndex(const QDomElement &elem, const QString &name, QComboBox *widget);
void loadActiveIndex(const QDomElement &elem, const QString &name, QTabWidget *widget, const int defaultVal);
void loadActiveIndex(const QDomElement &elem, const QString &name, QTabWidget *widget);

template <typename T>
void setValueBlocked(T widget, double value);
void setValueBlocked(QCheckBox *widget, bool value);

template <typename T>
void loadDoubleValueBlocked(const QDomElement &elem, const QString &name, T widget);
void loadBoolValueBlocked(const QDomElement &elem, const QString &name, QCheckBox *widget);

template <typename T>
void checkValueValid(T widget, double value);


template <typename T>
inline void setValue(T widget, double value)
{
    if(widget->minimum() > value || widget->maximum() < value)
    {
        std::stringstream ss;
        ss << "Value " << value << " for " << widget->objectName().toStdString() << " is out of range from "
           << widget->minimum() << " to " << widget->maximum();
        throw std::domain_error(ss.str());
    }
    widget->setValue(value);
}

template <typename T>
void loadIntValue(const QDomElement &elem, const QString &name, T widget, const int defaultVal)
{
    setValue(widget, readInt(elem, name, defaultVal));
}

template <typename T>
void loadIntValue(const QDomElement &elem, const QString &name, T widget)
{
    setValue(widget, readInt(elem, name));
}

template <typename T>
void loadDoubleValue(const QDomElement &elem, const QString &name, T widget, const double defaultVal)
{
    setValue(widget, readDouble(elem, name, defaultVal));
}

template <typename T>
void loadDoubleValue(const QDomElement &elem, const QString &name, T widget)
{
    setValue(widget, readDouble(elem, name));
}

template <typename T>
void loadBoolValue(const QDomElement &elem, const QString &name, T widget, bool defaultVal)
{
    setValue(widget, readBool(elem, name, defaultVal));
}

template <typename T>
void loadBoolValue(const QDomElement &elem, const QString &name, T widget)
{
    setValue(widget, readBool(elem, name));
}

template <typename T>
void setValueBlocked(T widget, double value)
{
    if(widget->minimum() > value || widget->maximum() < value)
    {
        std::stringstream ss;
        ss << "Value " << value << " for " << widget->objectName().toStdString() << " is out of range from "
           << widget->minimum() << " to " << widget->maximum();
        throw std::domain_error(ss.str());
    }
    widget->blockSignals(true);
    widget->setValue(value);
    widget->blockSignals(false);
}

template <typename T>
void loadDoubleValueBlocked(const QDomElement &elem, const QString &name, T widget)
{
    QSignalBlocker blocker(widget);
    setValue(widget, readDouble(elem, name));
}

template <typename T>
void checkValueValid(T widget, double value)
{
    if(widget->minimum() > value || widget->maximum() < value)
    {
        std::stringstream ss;
        ss << "Value " << value << " for " << widget->objectName().toStdString() << " is out of range from "
           << widget->minimum() << " to " << widget->maximum();
        throw std::domain_error(ss.str());
    }
}

/**
 * @brief Reads the next stream value safely.
 * @param stream
 * @return stream value if it can be converted to T. Nullopt otherwise.
 */
template <typename T>
static std::optional<T> readStreamValue(QTextStream &stream)
{
    T value;
    stream >> value;
    if(stream.status() == QTextStream::Ok)
    {
        return value;
    }
    return std::nullopt;
}

#endif // IMPORTHELPER_H
