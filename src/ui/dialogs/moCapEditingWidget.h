/*
 * PeTrack - Software for tracking pedestrians movement in videos
 * Copyright (C) 2024 Forschungszentrum Jülich GmbH, IAS-7
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

#ifndef MOCAPEDITINGWIDGET_H
#define MOCAPEDITINGWIDGET_H

#include <QFrame>

class MoCapPerson;

namespace Ui
{
class MoCapEditingWidget;
}

class MoCapEditingWidget : public QFrame
{
    Q_OBJECT

public:
    explicit MoCapEditingWidget(QWidget *parent, MoCapPerson &moCapPerson, std::function<void()> updateOverlay);
    MoCapEditingWidget(const MoCapEditingWidget &)             = delete;
    MoCapEditingWidget(MoCapEditingWidget &&)                  = delete;
    MoCapEditingWidget &operator==(const MoCapEditingWidget &) = delete;
    MoCapEditingWidget &operator==(MoCapEditingWidget &&)      = delete;
    ~MoCapEditingWidget() override;

    void select();
    void deselect();

private slots:
    void onTimeOffsetChanged(double newOffset);
    void onVisibleChanged(int newState);
    void onTranslationChanged(double);
    void onRotationChanged(double newAngle);

private:
    Ui::MoCapEditingWidget *mUi;
    MoCapPerson            &mPerson;
    std::function<void()>   mUpdateOverlay;
};

#endif // MOCAPEDITINGWIDGET_H
