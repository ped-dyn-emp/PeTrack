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

#ifndef BORDERFILTER_H
#define BORDERFILTER_H

#include "filter.h"

class BorderFilter : public Filter
{
private:
    Parameter<int> mRed{this};
    Parameter<int> mGreen{this};
    Parameter<int> mBlue{this};
    Parameter<int> mSize{this};

public:
    BorderFilter();

    cv::Mat act(cv::Mat &img, cv::Mat &res);

    Parameter<int> &getBorderSize();
    Parameter<int> &getBorderColR();
    Parameter<int> &getBorderColG();
    Parameter<int> &getBorderColB();
};

#endif
