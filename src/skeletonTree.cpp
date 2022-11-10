/*
 * PeTrack - Software for tracking pedestrians movement in videos
 * Copyright (C) 2022 Forschungszentrum Jülich GmbH, IAS-7
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
#include "skeletonTree.h"

#include <algorithm>

//--SkeletonNode


/**
 * @brief Constructor of the node.
 * @param[in] id  The identifier of this node.
 * @param[in] point  The position of this node.
 */
SkeletonNode::SkeletonNode(const uint8_t &id, cv::Point3f point) : mId(id), mPoint(std::move(point)) {}

/**
 * @brief Gets the Child with the specified id.
 *
 * This method returns a const reference to the first child found under this id.
 * A std::out_of_range error is throw if the id is not found.
 * @note The search is not recursive.
 * @param[in] id  The id of the child to be accessed.
 *
 * @return A const reference to the child with the specified id.
 * @throw std::out_of_range
 */
const SkeletonNode &SkeletonNode::getChildById(uint8_t id) const
{
    auto foundValue =
        std::find_if(mChildren.begin(), mChildren.end(), [id](const SkeletonNode &node) { return node.getId() == id; });
    if(foundValue == mChildren.end())
    {
        throw std::out_of_range("Id" + std::to_string(id) + " in children not found");
    }
    return *foundValue;
}

//--SkeletonTree

/**
 * @brief Recurses the skeleton and appends all connections to the vector.
 *
 * This method recurses the skeleton that is given in the argument.
 * It will follow all possible paths and return them in a prefix order.
 * @param[in] node  The node the recursion should start from.
 * @param[in, out] lines  The array to append the new lines.
 *
 */
void SkeletonTree::recurseSkeleton(const SkeletonNode &node, std::vector<SkeletonLine> &lines)
{
    for(const SkeletonNode &child : node.getChildren())
    {
        lines.push_back({/*Direct initialization of SkeletonLine struct*/
                         /*.start =*/node.getPos(),
                         /*.start_id =*/node.getId(),
                         /*.end =*/child.getPos(),
                         /*.end_id =*/child.getId()});
        recurseSkeleton(child, lines);
    }
}

/**
 * @brief Returns a vector that contains all connections.
 *
 * This method returns a vector with all connections. The data is contained in a @see SkeletonLine struct.
 * Each line can be intepreted as a bone. It is possible that the returned vector is empty.
 *
 * @return All valid connections. Might be empty.
 */
std::vector<SkeletonLine> SkeletonTree::getLines() const
{
    std::vector<SkeletonLine> lines;
    recurseSkeleton(mRoot, lines);
    return lines;
}
