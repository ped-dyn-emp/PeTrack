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

#ifndef PETRACK_INTERVALLIST_H
#define PETRACK_INTERVALLIST_H

#include "util/logger.h"

#include <cstddef>
#include <list>
#include <string>
#include <vector>

/**
 * The IntervalList is a data-structure to organize data that only changes at specific points of interest and stays the
 * same between these points (intervals).
 * For example on a timeline, where data may change at specific frames, but stays the same in between.
 * The Data is stored in an efficient way, by only saving these key points.
 *
 * The key values are Integers.
 *
 * A value for an empty entry is needed for easy deletion and emptiness-checking of the structure.
 * In case of emptiness, the underlying container contains a single "empty" entry.
 *
 * @tparam T the data type that shall be stored into key frames.
 */
template <typename T>
class IntervalList
{
public:
    struct Entry
    {
        int start;
        T   data;
    };

private:
    std::vector<Entry> mEntries;

    T mUndefinedValue;

public:
    /**
     * Create a new IntervalList.
     * @param undefinedValue the value to fill gaps in the list.
     */
    IntervalList(T undefinedValue) : mUndefinedValue(undefinedValue) { clear(); };

    /**
     * Clears all stored entries and sets it like freshly created.
     */
    void clear()
    {
        mEntries.clear();
        // "null item" needed for some checks
        mEntries.push_back({0, mUndefinedValue});
    }

    /**
     * Inserts a value at a specific position.
     * If a value for the same position is already present, it will be replaced.
     *
     * After insertion the list will compact i.e. consecutive intervals with the same value will collapse into one.
     * This might make the list smaller w.r.t. the `size()`-function.
     *
     * @param pos value to insert at
     * @param value data value
     *
     */
    void insert(int pos, const T &value)
    {
        if(this->empty())
        {
            // when empty, there is a dummy entry in the vector
            mEntries.clear();
            mEntries.push_back({pos, value});
            return;
        }
        for(auto it = mEntries.begin(); it != mEntries.end(); it++)
        {
            if(pos == it->start)
            {
                it->data = value;
                break;
            }

            if(pos < it->start)
            {
                mEntries.insert(it, {pos, value});
                break;
            }
            // append if position is behind start of last entry
            if(it == mEntries.end() - 1)
            {
                mEntries.push_back({pos, value});
                break;
            }
        }
        compact();
    }

    /**
     * Remove an interval (i.e. assign the undefinedValue for this part).
     * Works similar insert with the undefinedValue.
     *
     * If the position is the start of the first interval, the size will decrease.
     * If the position is behind the start of the last interval, the size might increase.
     *
     * This might introduce gaps into the data structure
     *
     * @param position the position index
     *
     * @see insert
     */
    void remove(int position)
    {
        auto size  = this->size();
        auto index = this->indexOf(position);

        if(size == 0 || index == -1)
        {
            return;
        }
        if(index == 0 && position == getEntry(position).start)
        {
            mEntries.erase(mEntries.begin());
            if(mEntries.size() == 0)
            {
                mEntries.push_back({0, mUndefinedValue});
            }
            return;
        }
        insert(position, mUndefinedValue);
    }

    /**
     * Go through the entries and return the index that corresponds to the given position.
     * @param position the position to find
     * @return the index of the entry. -1 if it is before the first entry
     */
    int indexOf(int position) const
    {
        if(position < mEntries.front().start)
        {
            return -1;
        }
        for(int i = 1; i < (int) mEntries.size(); ++i)
        {
            if(position < mEntries.at(i).start)
            {
                return i - 1;
            }
        }
        return mEntries.size() - 1;
    }

    /**
     * Compact the data-structure i.e. consecutive intervals with the same value shall collapse into a single interval.
     *
     * Example:
     * Let the list be (0, A) - (5, A) - (10, B) - (15, C) - (20, C)
     * will collapse into
     * (0, A) - (10, B) - (20, C)
     */
    void compact()
    {
        if(mEntries.size() < 2)
        {
            return;
        }
        T lastVal = mEntries.back().data;
        for(size_t i = mEntries.size() - 1; i > 0; i--)
        {
            auto prev = mEntries.at(i - 1);
            if(prev.data == lastVal)
            {
                mEntries.erase(mEntries.begin() + i);
            }
            lastVal = prev.data;
        }
    }

    /**
     * Get the Data at a specific position.
     *
     * If the list is empty or some parts are removed, undefinedValue will be returned.
     */
    T getValue(int position) const
    {
        auto index = indexOf(position);
        if(index == -1)
        {
            return mUndefinedValue;
        }
        return mEntries.at(index).data;
    }

    /**
     * Get the whole entry at a specific position.
     *
     * If some intervals were removed or the list is empty, an entry of the undefinedValue will be returned.
     */
    Entry &getEntry(int position)
    {
        auto index = this->indexOf(position);
        if(index == -1)
        {
            throw std::invalid_argument("Index out of bounds");
        }
        return mEntries.at(index);
    }

    /**
     * Get the minimum index occurring in a non-empty interval.
     */
    int getMinimum() const
    {
        if(mEntries.empty())
        {
            throw std::invalid_argument("Invalid State: empty list. This should be impossible.");
        }
        return mEntries.front().start;
    }

    const std::vector<Entry> &getEntries() const { return mEntries; }

    /**
     * Get the size of the data-structure.
     * The size of the IntervalList is considered the amount of different intervals.
     * Intervals without values (having the undefinedValue) will counted as well.
     * Since a list cannot start with such an interval, it has a size of zero in that case.
     *
     * @return the size
     */
    size_t size() const
    {
        if(mEntries.front().data == mUndefinedValue)
        {
            return 0;
        }
        return mEntries.size();
    }

    bool contains(int key) const { return indexOf(key) != -1; }

    /**
     * @return if the IntervalList is empty i.e. the size is zero
     */
    bool empty() const { return this->size() == 0; }

    /**
     *
     * @param value_tostring optional, custom to_string function to cast the value template parameter to a string.
     *
     * @return a string for console debugging purpose
     */
    std::string
    toString(std::function<std::string(T)> valueTostring = [](const T &val) { return std::to_string(val); }) const
    {
        if(mEntries.empty())
        {
            return "";
        }
        std::string s = "";
        for(auto i = 0; i < mEntries.size() - 1; ++i)
        {
            auto entry = mEntries.at(i);
            s.append("(" + std::to_string(entry.start) + ", " + valueTostring(entry.data) + ") - ");
        }
        auto entry = mEntries.back();
        s.append("(" + std::to_string(entry.start) + ", " + valueTostring(entry.data) + ")");
        return s;
    }
};
#endif // PETRACK_INTERVALLIST_H
