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

    IplImage* act(IplImage *img, IplImage *res);

//     bool changed();

    Parameter *getContrast();
};

#endif
