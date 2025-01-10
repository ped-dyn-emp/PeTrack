/*
 * PeTrack - Software for tracking pedestrians movement in videos
 * Copyright (C) 2025 Forschungszentrum Jülich GmbH, IAS-7
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
#ifndef PETRACK_ANNOTATIONGROUPTREEITEM_H
#define PETRACK_ANNOTATIONGROUPTREEITEM_H

#include <QColor>
#include <QString>
#include <QVector>
#include <memory>
#include <vector>

/**
 * Class, used for the custom TreeView in the main Group panel.
 * It is basically a key/value storage, to be very flexible for display.
 *
 * By inheritance, there are multiple sub-types for Toplevel groups or normal groups or additional group data.
 */
class AnnotationGroupTreeItem
{
public:
    explicit AnnotationGroupTreeItem(AnnotationGroupTreeItem *parentItem = nullptr);
    virtual ~AnnotationGroupTreeItem();
    AnnotationGroupTreeItem(const AnnotationGroupTreeItem &)            = delete;
    AnnotationGroupTreeItem(AnnotationGroupTreeItem &&)                 = delete;
    AnnotationGroupTreeItem &operator=(const AnnotationGroupTreeItem &) = delete;
    AnnotationGroupTreeItem &operator=(AnnotationGroupTreeItem &&)      = delete;

    void appendChild(std::unique_ptr<AnnotationGroupTreeItem> child);

    /**
     * Recursively check if the given pointer is a child of this object or
     * a child of any child.
     * @param candidate the pointer to check
     * @return true, if it is a child
     */
    bool isChild(AnnotationGroupTreeItem *candidate) const;

    AnnotationGroupTreeItem *getChildAt(int row);
    AnnotationGroupTreeItem *parentItem();
    int                      childCount() const;
    int                      columnCount() const;
    int                      row() const;

    virtual QString getData(int column) const;
    void            setData(int column, const QString &data);

    void   setParent(AnnotationGroupTreeItem *newParent);
    void   setColor(const QColor &color);
    QColor getColor() { return mColor; }

    static std::unique_ptr<AnnotationGroupTreeItem>
    makeTopLevel(const std::string &name, AnnotationGroupTreeItem *parent);
    static std::unique_ptr<AnnotationGroupTreeItem> makeGroup(const std::string &name, AnnotationGroupTreeItem *parent);
    static std::unique_ptr<AnnotationGroupTreeItem>
    makeEntry(const std::string &key, const std::string &value, AnnotationGroupTreeItem *parent);

    inline int  getId() { return mGroupId; }
    inline bool isTLG() { return mIsTLG; }
    inline int  getChildCount() { return mChildCount; }
    inline void setChildCount(int count) { mChildCount = count; }

private:
    std::vector<std::unique_ptr<AnnotationGroupTreeItem>> mChildren;
    AnnotationGroupTreeItem                              *mParent;


    QString mKey;
    QString mValue;
    QColor  mColor{Qt::white};


protected:
    int  mGroupId;
    bool mIsTLG;
    int  mChildCount;
};

class TopLevelGroupTreeItem : public AnnotationGroupTreeItem
{
public:
    TopLevelGroupTreeItem(int id, const std::string &name, AnnotationGroupTreeItem *root);

    QString getData(int column) const override;
};

class GroupTreeItem : public AnnotationGroupTreeItem
{
public:
    GroupTreeItem(int id, const std::string &name, const std::string &type, AnnotationGroupTreeItem *topLevelGroup);

    QString getData(int column) const override;
};

class GroupEntryTreeItem : public AnnotationGroupTreeItem
{
public:
    GroupEntryTreeItem(const std::string &key, const std::string &value, AnnotationGroupTreeItem *group);
};


#endif // PETRACK_ANNOTATIONGROUPTREEITEM_H
