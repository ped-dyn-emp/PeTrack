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

#ifndef AUTOCALIB_H
#define AUTOCALIB_H

#include<QString>
#include<QStringList>

class Petrack;
class Control;

class AutoCalib
{
public:
    AutoCalib();
    ~AutoCalib();

    void setMainWindow(Petrack *mw);
    bool isEmptyCalibFiles();
    void addCalibFile(const QString &f);
    QString getCalibFile(int i);
    QStringList getCalibFiles();
    void setCalibFiles(const QStringList &fl);
    bool openCalibFiles(); // return true if at least one file is selected
    inline void setBoardSizeX(int i) // 6
    {
        mBoardSizeX = i;
    }
    inline int getBoardSizeX() const // 6
    {
        return mBoardSizeX;
    }
    inline void setBoardSizeY(int i) // 8 oder 9
    {
        mBoardSizeY = i;
    }
    inline int getBoardSizeY() const // 8 oder 9
    {
        return mBoardSizeY;
    }
    inline void setSquareSize(float d) // in cm
    {
        mSquareSize = d;
    }
    inline float getSquareSize() const // in cm
    {
        return mSquareSize;
    }

    void autoCalib();

private:
    int runCalibration(std::vector<std::vector<cv::Point2f> > corners, cv::Size img_size, cv::Size board_size,
        float square_size, float aspect_ratio, int flags,
        cv::Mat &camera_matrix, cv::Mat &dist_coeffs, double *reproj_errs);//, Mat *extr_params);//,
       // double &reproj_errs);//, double* avg_reproj_err);

    Petrack *mMainWindow;
    Control *mControlWidget;
    QStringList mCalibFiles;
    int mBoardSizeX, mBoardSizeY;
    float mSquareSize;
};

#endif
