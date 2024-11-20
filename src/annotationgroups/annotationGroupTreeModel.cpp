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

#include "annotationGroupTreeModel.h"

#include "logger.h"

#include <QBrush>
#include <memory>
#include <utility>

AnnotationGroupTreeModel::AnnotationGroupTreeModel(QObject *parent) :
    QAbstractItemModel(parent), rootItem(std::make_unique<AnnotationGroupTreeItem>())
{
}


QModelIndex AnnotationGroupTreeModel::index(int row, int column, const QModelIndex &parent) const
{
    if(!hasIndex(row, column, parent))
    {
        return QModelIndex();
    }

    AnnotationGroupTreeItem *parentItem;

    if(!isIndexValid(parent))
    {
        parentItem = rootItem.get();
    }
    else
    {
        parentItem = static_cast<AnnotationGroupTreeItem *>(parent.internalPointer());
    }

    AnnotationGroupTreeItem *childItem = parentItem->getChildAt(row);
    if(childItem != nullptr)
    {
        return createIndex(row, column, childItem);
    }
    return QModelIndex();
}

QVariant AnnotationGroupTreeModel::data(const QModelIndex &index, int role) const
{
    if(!isIndexValid(index))
    {
        return {};
    }

    auto *item = static_cast<AnnotationGroupTreeItem *>(index.internalPointer());
    if(role == Qt::BackgroundRole && index.column() == 1)
    {
        return item->getColor();
    }
    if(role == Qt::DisplayRole)
    {
        return item->getData(index.column());
    }
    if(role == Qt::ForegroundRole)
    {
        QBrush brush;
        brush.setColor(item->getChildCount() > 0 ? QColor("black") : QColor("gray"));
        return brush;
    }
    return {};
}
Qt::ItemFlags AnnotationGroupTreeModel::flags(const QModelIndex &index) const
{
    if(!isIndexValid(index))
    {
        return Qt::NoItemFlags;
    }
    return QAbstractItemModel::flags(index);
}
QVariant AnnotationGroupTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(orientation == Qt::Horizontal && role == Qt::DisplayRole)
    {
        return rootItem->getData(section);
    }
    return QVariant();
}
QModelIndex AnnotationGroupTreeModel::parent(const QModelIndex &index) const
{
    if(!isIndexValid(index))
    {
        return QModelIndex();
    }

    auto                    *childItem  = static_cast<AnnotationGroupTreeItem *>(index.internalPointer());
    AnnotationGroupTreeItem *parentItem = childItem->parentItem();


    if(parentItem == nullptr || parentItem == rootItem.get())
    {
        return QModelIndex();
    }

    return createIndex(parentItem->row(), 0, parentItem);
}

int AnnotationGroupTreeModel::rowCount(const QModelIndex &parent) const
{
    AnnotationGroupTreeItem *parentItem;
    if(parent.column() > 0)
    {
        return 0;
    }

    if(!isIndexValid(parent))
    {
        parentItem = rootItem.get();
    }
    else
    {
        parentItem = static_cast<AnnotationGroupTreeItem *>(parent.internalPointer());
    }

    return parentItem->childCount();
}
int AnnotationGroupTreeModel::columnCount(const QModelIndex &parent) const
{
    if(isIndexValid(parent))
    {
        return static_cast<AnnotationGroupTreeItem *>(parent.internalPointer())->columnCount();
    }
    return rootItem->columnCount();
}

bool AnnotationGroupTreeModel::isIndexValid(const QModelIndex &idx) const
{
    if(!idx.isValid())
    {
        return false;
    }

    auto *item = static_cast<AnnotationGroupTreeItem *>(idx.internalPointer());
    return rootItem->isChild(item);
}

void AnnotationGroupTreeModel::setRootItem(std::unique_ptr<AnnotationGroupTreeItem> newRoot)
{
    rootItem = std::move(newRoot);
    emit layoutChanged();
}
