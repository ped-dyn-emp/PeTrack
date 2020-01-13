#ifndef BRIGHTFILTER_H
#define BRIGHTFILTER_H

#include "filter.h"

class BrightFilter : public Filter
{
private:
    Parameter mB;

public:
    BrightFilter();
#ifndef STEREO_DISABLED
    IplImage* act(IplImage *img, IplImage *res);
#endif

//     bool changed();

    Parameter *getBrightness();
};

#endif
