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

#ifndef WALKAREAWIDGET_H
#define WALKAREAWIDGET_H

#include "ui_walkArea.h"
#include "util/polygon.h"

#include <QWidget>
#include <map>

class QTimer;
class Petrack;
class WalkAreaManager;

class WalkAreaWidget : public QWidget
{
    Q_OBJECT

public:
    explicit WalkAreaWidget(QWidget *parent = nullptr);
    ~WalkAreaWidget();
    void setManager(WalkAreaManager *mgr);
    void setMainWindow(Petrack *mw) { mMainWindow = mw; }

private slots:
    void onVisibilityToggled(bool checked);
    void onWalkareaVisibilityToggled(bool checked);
    void onObstacleVisibilityToggled(bool checked);
    void onEditModeToggled(bool checked);
    void onDrawingTypeChanged(walkarea::PolygonType type);
    void onExportWKT();
    void onImportWKT();
    void onGeometryChanged();
    void onRowDeleteClicked();
    void onTogglePolygonEnabled(bool checked);
    void onHeightChanged(double value);
    void onValidationChanged();
    void onConfirmationTimeout();

private:
    void updatePolygonList();
    bool eventFilter(QObject *obj, QEvent *event) override;
    void resetDeleteConfirmation();
    void updateStatusLabel();
    void highlightInvalidRows();

    Ui::WalkAreaWidget *mUi = nullptr;

    Petrack         *mMainWindow = nullptr;
    WalkAreaManager *mManager    = nullptr;

    std::map<int, int> mPolygonIdToRow;               // Map polygon ID to table row
    bool               mDeleteConfirmMode  = false;   // Two-step delete confirmation
    QPushButton       *mConfirmationButton = nullptr; // Track which button is in confirm mode
    QTimer            *mConfirmationTimer  = nullptr; // Auto-reset after 3 seconds
};

#endif
