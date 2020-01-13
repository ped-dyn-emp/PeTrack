#ifndef CONTRASTFILTER_H
#define CONTRASTFILTER_H

#include "filter.h"
#include "helper.h"

class ContrastFilter : public Filter
{
private:
    Parameter mC;

public:
    ContrastFilter();
#ifndef STEREO_DISABLED
    IplImage* act(IplImage *img, IplImage *res);
#endif

//     bool changed();

    Parameter *getContrast();
};

#endif
