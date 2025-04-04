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

#ifndef BRIGHTCONTRASTFILTER_H
#define BRIGHTCONTRASTFILTER_H

#include "filter.h"
#include "helper.h"


class BrightContrastFilter : public Filter
{
private:
    Parameter<double> mBrightness{this};
    Parameter<double> mContrast{this};

public:
    BrightContrastFilter();

    cv::Mat act(cv::Mat &img, cv::Mat &res);

    Parameter<double> &getBrightness();
    Parameter<double> &getContrast();
};

#endif
