/*
 * PeTrack - Software for tracking pedestrians movement in videos
 * Copyright (C) 2010-2021 Forschungszentrum Jülich GmbH,
 * Maik Boltes, Juliane Adrian, Ricardo Martin Brualla, Arne Graf, Paul Häger, Daniel Hillebrand,
 * Deniz Kilic, Paul Lieberenz, Daniel Salden, Tobias Schrödter, Ann Katrin Seemann
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
#ifndef SKELETONTREE_H
#define SKELETONTREE_H
#include "vector.h"

#include <algorithm>
#include <cstdint>
#include <memory>
#include <opencv2/core/matx.hpp>
#include <opencv2/core/types.hpp>
#include <stdexcept>
#include <string>
#include <vector>

/**
 * @brief A node of the SkeletonTree class.
 *
 * This node can have 0 or more children.
 * The children are stored as a vector, so they are ordered sequentially in memory.
 * Every bone originating from this point is represented using the children. To get the line for a
 * bone the origin of this node and the origin of a child has to be taken.
 * This class is used by @see SkeletonTree
 *
 **/
class SkeletonNode
{
public:
    SkeletonNode(const uint8_t &id, cv::Point3f point);

    SkeletonNode(const SkeletonNode &rhs) = default;
    SkeletonNode(SkeletonNode &&rhs)      = default;

    ~SkeletonNode() = default;

    SkeletonNode &operator=(const SkeletonNode &rhs) = default;
    SkeletonNode &operator=(SkeletonNode &&rhs) = default;

    /**
     * @brief Gets the children as a vector.
     * Returns the underlying vector with children as a reference.
     * @note Changing values in this array will change them for the called object too.
     *
     * @return A reference to a vector containing the children.
     */
    inline std::vector<SkeletonNode> &getChildren() { return mChildren; }

    /**
     * @brief Gets the children as a vector.
     * Returns the underlying vector with children as a const reference.
     *
     * @return A const reference to a vector containing the children.
     */
    inline const std::vector<SkeletonNode> &getChildren() const { return mChildren; }

    /**
     * @brief Gets a const reference to the child with the specified memory index. This method is bounds checked.
     * This method gets the child by the specified memory index. Children are stored sequentially in memory.
     * If the index is out of range a std::out_of_range is thrown.
     * @param[in] index  The index of the child to be retrieved.
     *
     * @return A const reference to the child at the memory location.
     * @throw std::out_of_range
     */
    const SkeletonNode &getChild(size_t index) const { return mChildren.at(index); }

    const SkeletonNode &getChildById(uint8_t id) const;


    /**
     * @brief Gets the id of the node.
     * @return The id of the node.
     */
    inline uint8_t getId() const { return mId; }

    /**
     * @brief Gets the number of children.
     * @return The number of children.
     */
    inline size_t getChildrenCount() const { return mChildren.size(); }

    /**
     * @brief Makes a node a child.
     * Appends the node to the list of children. There are no checks for correctness or uniqueness.
     * @param[in] node  The node to be parented.
     *
     * @return The added node.
     *
     * @throw std::bad_alloc
     */
    inline SkeletonNode &addChild(const SkeletonNode &node)
    {
        mChildren.push_back(node);
        return mChildren.back();
    }

    /**
     * @brief Makes a node the child using move semantics.
     * Appends the node to the list of children. There are no checks for correctness or uniqueness.
     * This method is there to allow for move optimizations.
     * @param[in] node The node to be parented.
     *
     * @return The added node.
     *
     * @throw std::bad_alloc
     */
    inline SkeletonNode &addChild(SkeletonNode &&node)
    {
        mChildren.push_back(std::move(node));
        return mChildren.back();
    }

    /**
     * @brief Gets the position of the node.
     *
     * @return The position of the node.
     */
    inline cv::Point3f getPos() const { return mPoint; }

private:
    uint8_t                   mId;       /**< An identifier for the node. */
    cv::Point3f               mPoint;    /**< Position of the node. */
    std::vector<SkeletonNode> mChildren; /**< A vector containing the child nodes of this node.*/
};

struct SkeletonLine
{
    cv::Point3f start;
    uint8_t     start_id;
    cv::Point3f end;
    uint8_t     end_id;
};

/**
 * @brief The SkeletonTree class saves a Motion Capture Skeleton.
 *
 * This class saves the position of marker points and the direction of the head as a vector pointing into
 * the appropiate direction of a person equipped with a motion capturing
 * system.
 * Internally the Nodes are saved as a tree with a variable number of children.
 * \see SkeletonNode
 */

class SkeletonTree
{
    static void recurseSkeleton(const SkeletonNode &node, std::vector<SkeletonLine> &lines);

public:
    /**
     * @brief Constructor for the Skeleton.
     * @param[in] root  The root node for the Skeleton.
     * @param[in] dir  The direction the head is facing. The vector will be normalized.
     */
    SkeletonTree(SkeletonNode root, const Vec3F &dir) : mRoot(std::move(root)), mHeadDir(dir) { mHeadDir.normalize(); }

    /**
     * @brief Moving constructor for the Skeleton.
     *
     * Similar to the other constructor, but this one takes r-values as arguments.
     *
     * @param[in] root  The root node as a r-value.
     * @param[in] dir  The direction the head is facing in as a r-value
     */
    SkeletonTree(SkeletonNode &&root, Vec3F &&dir) : mRoot(root), mHeadDir(dir) { mHeadDir.normalize(); }

    std::vector<SkeletonLine> getLines() const;

    /**
     * @brief Gets a reference to the root node of the skeleton.
     *
     * @return The root node of the skeleton as a reference.
     */
    inline SkeletonNode &getRoot() { return mRoot; }

    /**
     * @brief Gets the root node of the skeleton as a const reference.
     *
     * @return The root node of the skeleton as a const reference.
     */
    inline const SkeletonNode &getRoot() const { return mRoot; }

    /**
     * @brief Gets the direction of the head.
     * Gets the direction of the head. The returned value is normalized.
     *
     * @return The normalized direction of the head.
     */
    inline const Vec3F &getHeadDir() const { return mHeadDir; }

private:
    SkeletonNode mRoot;    /**< Root node of the skeleton. */
    Vec3F        mHeadDir; /**< Direction the head is facing to. This value is normalized. */
};


#endif // SKELETONTREE_H
