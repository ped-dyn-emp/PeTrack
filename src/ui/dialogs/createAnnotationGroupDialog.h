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

#ifndef PETRACK_CREATEANNOTATIONGROUPDIALOG_H
#define PETRACK_CREATEANNOTATIONGROUPDIALOG_H

#include "annotationGrouping.h"

#include <QDialog>
#include <QRadioButton>
#include <set>
#include <vector>

namespace Ui
{
class CreateAnnotationGroupDialog;
}

class CreateAnnotationGroupDialog : public QDialog
{
    Q_OBJECT


public:
    CreateAnnotationGroupDialog(
        const std::vector<annotationGroups::TopLevelGroup> &tlgs,
        const std::set<std::string>                        &types  = {"No Type"},
        QWidget                                            *parent = nullptr);
    ~CreateAnnotationGroupDialog() override;


    annotationGroups::Group         getGroup();
    std::string                     getName();
    std::string                     getType();
    QColor                          getColor();
    annotationGroups::TopLevelGroup getSelectedTopLevelGroup();

    bool isValid();

    /**
     * Provide a vector with predefined colors. One for each selectable top level group.
     * @param colors a list of QColors
     */
    void setPredefinedColors(const std::vector<QColor> &colors);


private:
    Ui::CreateAnnotationGroupDialog *mUi;

    std::vector<QRadioButton *>                  mRadioButtons;
    size_t                                       mSelectedButton = -1;
    std::vector<annotationGroups::TopLevelGroup> mTopLevelGroups;

    const QColor        mColorFallback = Qt::white;
    QColor              mSelectedColor;
    std::vector<QColor> mDefaultColors;

    void btnColorClicked();

    size_t getSelectedTopLevelGroupIndex();
    void   btnRadioClicked();
    void   selectionChanged();
    void   setSelectedColor(const QColor &color);
};


#endif // PETRACK_CREATEANNOTATIONGROUPDIALOG_H
