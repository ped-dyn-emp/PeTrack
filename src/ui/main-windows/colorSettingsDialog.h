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

#ifndef COLORSETTINGSDIALOG_H
#define COLORSETTINGSDIALOG_H

#include <QColor>
#include <QDialog>

class QPushButton;

namespace Ui
{
class ColorSettingsDialog;
}

/**
 * @brief Dialog bundling all tracking visualization colors in one place.
 */
class ColorSettingsDialog : public QDialog
{
    Q_OBJECT

public:
    static constexpr QColor DEFAULT_TRACK_PATH_COLOR{255, 0, 0};    // red
    static constexpr QColor DEFAULT_GROUND_PATH_COLOR{0, 255, 0};   // green
    static constexpr QColor DEFAULT_RECOGNIZED_COLOR{0, 255, 0};    // green
    static constexpr QColor DEFAULT_TRACKED_COLOR{0, 0, 255};       // blue
    static constexpr QColor DEFAULT_PERSON_NUMBER_COLOR{255, 0, 0}; // red

    explicit ColorSettingsDialog(QWidget *parent = nullptr);
    ~ColorSettingsDialog() override;
    ColorSettingsDialog(const ColorSettingsDialog &)            = delete;
    ColorSettingsDialog &operator=(const ColorSettingsDialog &) = delete;
    ColorSettingsDialog(ColorSettingsDialog &&)                 = delete;
    ColorSettingsDialog &operator=(ColorSettingsDialog &&)      = delete;

    QColor getTrackPathColor() const;
    void   setTrackPathColor(const QColor &col);
    QColor getTrackGroundPathColor() const;
    void   setTrackGroundPathColor(const QColor &col);
    QColor getTrackPersonNumberColor() const;
    void   setTrackPersonNumberColor(const QColor &col);
    QColor getTrackRecognizedColor() const;
    void   setTrackRecognizedColor(const QColor &col);
    QColor getTrackTrackedColor() const;
    void   setTrackTrackedColor(const QColor &col);
    QColor getMoCapColor() const;
    void   setMoCapColor(const QColor &col);

signals:
    /// emitted when the user picked a new color for one of the track colors
    void colorChanged();
    /// emitted when the user picked a new MoCap color (the MoCap color is owned by the MoCapController)
    void moCapColorSelected(const QColor &col);

private:
    QColor pickColor(const QPushButton *button);

    Ui::ColorSettingsDialog *mUi;
};

#endif // COLORSETTINGSDIALOG_H
