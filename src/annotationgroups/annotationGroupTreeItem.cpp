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

#include "annotationGroupTreeItem.h"

#include "logger.h"

#include <QString>
#include <QVariant>
#include <memory>
#include <utility>

AnnotationGroupTreeItem::AnnotationGroupTreeItem(AnnotationGroupTreeItem *parentItem) : mParent(parentItem) {}

AnnotationGroupTreeItem::~AnnotationGroupTreeItem()
{
    for(auto &child : mChildren)
    {
        child->setParent(nullptr);
        child.reset();
    }
}


void AnnotationGroupTreeItem::appendChild(std::unique_ptr<AnnotationGroupTreeItem> child)
{
    child->setParent(this);
    mChildren.push_back(std::move(child));
}

bool AnnotationGroupTreeItem::isChild(AnnotationGroupTreeItem *candidiate) const
{
    for(const auto &child : mChildren)
    {
        if(child.get() == candidiate)
        {
            return true;
        }
        if(child->isChild(candidiate))
        {
            return true;
        }
    }
    return false;
}

AnnotationGroupTreeItem *AnnotationGroupTreeItem::getChildAt(int row)
{
    if(row < 0 || row > childCount())
    {
        return nullptr;
    }
    return mChildren[row].get();
}

AnnotationGroupTreeItem *AnnotationGroupTreeItem::parentItem()
{
    return mParent;
}

int AnnotationGroupTreeItem::childCount() const
{
    return mChildren.size();
}

int AnnotationGroupTreeItem::columnCount() const
{
    return 2;
}
QString AnnotationGroupTreeItem::getData(int column) const
{
    switch(column)
    {
        case 0:
            return mKey;
        case 1:
            return mValue;
    }
    return "";
}

void AnnotationGroupTreeItem::setData(int column, const QString &data)
{
    switch(column)
    {
        case 0:
            mKey = data;
            break;
        case 1:
            mValue = data;
            break;
    }
}

int AnnotationGroupTreeItem::row() const
{
    if(mParent == nullptr)
    {
        return 0;
    }
    const auto it = std::find_if(
        mParent->mChildren.cbegin(),
        mParent->mChildren.cend(),
        [this](const std::unique_ptr<AnnotationGroupTreeItem> &item) { return item.get() == this; });
    if(it != mParent->mChildren.cend())
    {
        return std::distance(mParent->mChildren.cbegin(), it);
    }
    Q_ASSERT(false);
    SPDLOG_ERROR("child not in children list of parent");
    return -1;
}


void AnnotationGroupTreeItem::setParent(AnnotationGroupTreeItem *newParent)
{
    mParent = newParent;
}
void AnnotationGroupTreeItem::setColor(const QColor &color)
{
    mColor = color;
}

std::unique_ptr<AnnotationGroupTreeItem>
AnnotationGroupTreeItem::makeTopLevel(const std::string &name, AnnotationGroupTreeItem *parent)
{
    auto ptr = std::make_unique<AnnotationGroupTreeItem>(parent);
    ptr->setData(0, QString::fromStdString(name));
    return ptr;
}
std::unique_ptr<AnnotationGroupTreeItem>
AnnotationGroupTreeItem::makeGroup(const std::string &name, AnnotationGroupTreeItem *parent)
{
    return makeTopLevel(name, parent);
}
std::unique_ptr<AnnotationGroupTreeItem>
AnnotationGroupTreeItem::makeEntry(const std::string &key, const std::string &value, AnnotationGroupTreeItem *parent)
{
    auto ptr = makeGroup(key, parent);
    ptr->setData(1, QString::fromStdString(value));
    return ptr;
}
TopLevelGroupTreeItem::TopLevelGroupTreeItem(int id, const std::string &name, AnnotationGroupTreeItem *root) :
    AnnotationGroupTreeItem(root)
{
    AnnotationGroupTreeItem::setData(0, QString::fromStdString(name));
    this->mGroupId = id;
    this->mIsTLG   = true;
}
QString TopLevelGroupTreeItem::getData(int column) const
{
    return AnnotationGroupTreeItem::getData(column);
}


GroupTreeItem::GroupTreeItem(
    int                      id,
    const std::string       &name,
    const std::string       &type,
    AnnotationGroupTreeItem *topLevelGroup) :
    AnnotationGroupTreeItem(topLevelGroup)
{
    AnnotationGroupTreeItem::setData(0, QString::fromStdString(name));
    AnnotationGroupTreeItem::setData(1, QString::fromStdString(type));
    this->mGroupId = id;
    this->mIsTLG   = false;
}
QString GroupTreeItem::getData(int column) const
{
    if(column == 0)
    {
        return QString("%1 (%2)").arg(AnnotationGroupTreeItem::getData(0), AnnotationGroupTreeItem::getData(1));
    }
    return {};
}

GroupEntryTreeItem::GroupEntryTreeItem(
    const std::string       &key,
    const std::string       &value,
    AnnotationGroupTreeItem *group) :
    AnnotationGroupTreeItem(group)
{
    setData(0, QString::fromStdString(key));
    setData(1, QString::fromStdString(value));
}
