#ifndef BRIGHTFILTER_H
#define BRIGHTFILTER_H

#include "filter.h"

class BrightFilter : public Filter
{
private:
    Parameter mB;

public:
    BrightFilter();

    IplImage* act(IplImage *img, IplImage *res);

//     bool changed();

    Parameter *getBrightness();
};

#endif
