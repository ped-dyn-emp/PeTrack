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

#ifndef PETRACK_ANNOTATIONGROUPTREEMODEL_H
#define PETRACK_ANNOTATIONGROUPTREEMODEL_H

#include "annotationGroupTreeItem.h"

#include <QAbstractItemModel>
#include <memory>

/**
 * Custom TreeModel for the Tree view of the main grouping panel.
 * Uses GroupingItem instances as data model.
 */
class AnnotationGroupTreeModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit AnnotationGroupTreeModel(QObject *parent = nullptr);
    ~AnnotationGroupTreeModel() override                                  = default;
    AnnotationGroupTreeModel(const AnnotationGroupTreeModel &)            = delete;
    AnnotationGroupTreeModel(AnnotationGroupTreeModel &&)                 = delete;
    AnnotationGroupTreeModel &operator=(const AnnotationGroupTreeModel &) = delete;
    AnnotationGroupTreeModel &operator=(AnnotationGroupTreeModel &&)      = delete;

    QVariant      data(const QModelIndex &index, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant      headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    QModelIndex   index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex   parent(const QModelIndex &index) const override;
    int           rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int           columnCount(const QModelIndex &parent = QModelIndex()) const override;

    bool isIndexValid(const QModelIndex &idx) const;

    void                     setRootItem(std::unique_ptr<AnnotationGroupTreeItem> newRoot);
    AnnotationGroupTreeItem *getRoot() { return rootItem.get(); }

private:
    std::unique_ptr<AnnotationGroupTreeItem> rootItem;
};


#endif // PETRACK_ANNOTATIONGROUPTREEMODEL_H
