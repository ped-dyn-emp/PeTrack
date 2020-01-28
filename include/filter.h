#ifndef FILTER_H
#define FILTER_H


#include "opencv2/opencv.hpp"

class Filter;

class Parameter
{
private:
    double mValue;
    double mMinimum;
    double mMaximum;
    bool mChg;
    Filter *mFilter; // Filter where parameter is for

public:
    Parameter();

    void setFilter(Filter *filter);
    Filter * getFilter();

    double value();
    double getValue();
    void setValue(double d);

    double getMinimum();
    void setMinimum(double d);
    double getMaximum();
    void setMaximum(double d);

    bool changed();
    bool getChanged();
    void setChanged(bool b);
};

//------------------------------------------------------------

class Filter {
private:
    bool mChg;       // if filter paramater were changed
    bool mEnable;    // if filter is actice
    bool mOnCopy;    // if filter works on a copy 
//    bool mResStored; // if storage space was allocated
//    IplImage *mRes;  // result of filter
    cv::Mat mRes;
    
    // pure virtual function, where to implement the filter conversion
    // returns the result over pointer res and as result
//    virtual IplImage* act(IplImage *img, IplImage *res)=0;
    virtual cv::Mat act(cv::Mat &img, cv::Mat &res)=0;

public:
    Filter();
    virtual ~Filter() {};

    bool changed();
    bool getChanged();
    void setChanged(bool b);

//    void setResStored(bool b);

    // apply on original Data
//    IplImage* apply(IplImage *img);
    cv::Mat apply(cv::Mat &img);

    cv::Mat getLastResult();
//    IplImage* getLastResult();
    void freeLastResult();

    void enable();
    void disable();
    void setEnabled(bool b);
    bool getEnabled();

    void setOnCopy(bool b);
    bool getOnCopy();
};

#endif
