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
