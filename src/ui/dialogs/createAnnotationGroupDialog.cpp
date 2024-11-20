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

#include "createAnnotationGroupDialog.h"

#include "logger.h"
#include "ui_createAnnotationGroupDialog.h"

#include <QColor>
#include <QColorDialog>
#include <QRadioButton>
#include <set>

using namespace annotationGroups;
using namespace std;

CreateAnnotationGroupDialog::CreateAnnotationGroupDialog(
    const std::vector<TopLevelGroup> &tlgs,
    const std::set<std::string>      &types,
    QWidget                          *parent) :
    QDialog(parent), mUi(new Ui::CreateAnnotationGroupDialog()), mTopLevelGroups(tlgs)
{
    if(tlgs.empty())
    {
        throw std::invalid_argument("Critical Error. No TopLevelGroups existing when creating 'Create Group' dialog!");
    }

    mUi->setupUi(this);

    this->setWindowTitle("Create new Group");

    auto *grp = mUi->groupBoxType;

    for(const auto &tlg : tlgs)
    {
        auto  btnName = tlg.name;
        auto *radio   = new QRadioButton(QString::fromStdString(btnName));
        grp->layout()->addWidget(radio);
        mRadioButtons.push_back(radio);

        connect(radio, &QRadioButton::clicked, this, &CreateAnnotationGroupDialog::btnRadioClicked);
    }
    mRadioButtons.front()->click();

    auto &typeBox = mUi->comboBox;
    for(const auto &type : types)
    {
        typeBox->addItem(QString::fromStdString(type));
    }
    typeBox->setEditable(true);

    connect(mUi->btnColor, &QPushButton::clicked, [&, this]() { this->btnColorClicked(); });
}

CreateAnnotationGroupDialog::~CreateAnnotationGroupDialog()
{
    for(auto *radio : mRadioButtons)
    {
        delete radio;
    }
}


annotationGroups::Group CreateAnnotationGroupDialog::getGroup()
{
    annotationGroups::Group group(0, getName(), getType());
    group.color = getColor();
    group.tlgId = getSelectedTopLevelGroup().id;
    return group;
}

annotationGroups::TopLevelGroup CreateAnnotationGroupDialog::getSelectedTopLevelGroup()
{
    size_t i = getSelectedTopLevelGroupIndex();

    return mTopLevelGroups.at(i);
}

size_t CreateAnnotationGroupDialog::getSelectedTopLevelGroupIndex()
{
    for(size_t i = 0; i < mRadioButtons.size(); ++i)
    {
        if(mRadioButtons.at(i)->isChecked())
        {
            return i;
        }
    }
    return -1;
}

void CreateAnnotationGroupDialog::btnColorClicked()
{
    QColor col = QColorDialog::getColor(QColor(0, 0, 0), this);
    if(col.isValid())
    {
        setSelectedColor(col);
    }
}

bool CreateAnnotationGroupDialog::isValid()
{
    return !getName().empty();
}

string CreateAnnotationGroupDialog::getName()
{
    return mUi->txtName->text().trimmed().toStdString();
}

string CreateAnnotationGroupDialog::getType()
{
    return mUi->comboBox->currentText().trimmed().toStdString();
}

QColor CreateAnnotationGroupDialog::getColor()
{
    return mSelectedColor;
}

void CreateAnnotationGroupDialog::setPredefinedColors(const vector<QColor> &colors)
{
    mDefaultColors = colors;
    selectionChanged();
}

void CreateAnnotationGroupDialog::btnRadioClicked()
{
    if(getSelectedTopLevelGroupIndex() != mSelectedButton)
    {
        selectionChanged();
    }
}

void CreateAnnotationGroupDialog::selectionChanged()
{
    mSelectedButton = getSelectedTopLevelGroupIndex();

    QColor color = (mSelectedButton < mDefaultColors.size()) ? mDefaultColors.at(mSelectedButton) : mColorFallback;

    setSelectedColor(color);
}

void CreateAnnotationGroupDialog::setSelectedColor(const QColor &color)
{
    QPalette pal = mUi->btnColor->palette();
    pal.setColor(QPalette::ColorRole::Button, color);
    mUi->btnColor->setPalette(pal);
    mSelectedColor = color;
}
