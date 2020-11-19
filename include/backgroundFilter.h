/*
 * PeTrack - Software for tracking pedestrians movement in videos
 * Copyright (C) 2010-2020 Forschungszentrum Jülich GmbH,
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

#ifndef BACKGROUNDFILTER_H
#define BACKGROUNDFILTER_H

#include "filter.h"


#include "opencv2/opencv.hpp"
#include "opencv2/video/background_segm.hpp"


#include <QString>

#include "stereoContext.h"
//#include "opencv2/video/background_segm.hpp"

//class pet::StereoContext;

class BackgroundFilter : public Filter
{
private:

    cv::Ptr<cv::BackgroundSubtractorMOG2> mBgModel;
    //Ptr<BackgroundSubtractor> mBgModel;

    bool mUpdate; // if 0, kein update des models, sonst schon
    pet::StereoContext** mStereoContext; // zeiger auf den zeiger in petrack mit stereocontext
    cv::Mat mBgPointCloud;
    cv::Mat mForeground;
    QString mLastFile;
    double mDefaultHeight;

public:
    BackgroundFilter();
    ~BackgroundFilter();

    void setDefaultHeight(double h);

    void setUpdate(bool b);
    bool update();

    QString getFilename();
    void setFilename(const QString &fn);

    bool load(QString dest = "");
    bool save(QString dest = "");

    void setStereoContext(pet::StereoContext **sc);
    pet::StereoContext** stereoContext();

    cv::Mat getForeground(); // nutzen, wenn ueber ganzes bild foreground benutzt wird; NULL, wenn keine background subtraction aktiviert
    bool isForeground(int i, int j); // nutzen, wenn einzelne pixel abgefraget werden

    void reset();

    cv::Mat act(cv::Mat &img, cv::Mat &res);
//    IplImage* act(IplImage *img, IplImage *res);

    void maskBg(cv::Mat &mat, float val);
};

#endif
