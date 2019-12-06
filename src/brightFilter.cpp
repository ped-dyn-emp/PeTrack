#include "brightFilter.h"

BrightFilter::BrightFilter()
    :Filter()
{
    mB.setMinimum(0.);
    mB.setMaximum(255.);
    mB.setValue(0.);
    mB.setFilter(this);
}

IplImage* BrightFilter::act(IplImage *img, IplImage *res)
{
    int x, y;
    // Pointer to the data information in the IplImage
    unsigned char *dataIn  = (unsigned char *) img->imageData; 
    unsigned char *dataOut = (unsigned char *) res->imageData; 
    // int channels = img->nChannels;
  
    // set poiner to value before array, because ++i is more effective than i++
    --dataIn; --dataOut; 

    // Contrast factor
    float b = mB.getValue();
    float fac = (255.-b)/255.;
    // cout << CHAR_MIN << " " << CHAR_MAX<<endl
    // leider oft von -128..127, so dass umwandlung in unsigned char bei berechung noetig

    // This loop is optimized so it has to calculate the least amount of indexes
    // Optimizing the access to the pointer data is useless (no difference in performance when tested)
    for (y = 0; y < img->height; ++y)
    {
        for (x = 0; x < img->width; ++x)
        {
            *(++dataOut) = (char) (*(++dataIn) * fac + b);
            *(++dataOut) = (char) (*(++dataIn) * fac + b);
            *(++dataOut) = (char) (*(++dataIn) * fac + b);
            // so wuerde man durchschleifen
//             *(++dataOut) = *(++dataIn);
//             *(++dataOut) = *(++dataIn);
//             *(++dataOut) = *(++dataIn);
            // dataIn+=channels; // es wird von 3 kanaelen ausgegenagen
        }
    }
//     // parameter was used, so it is now set to unchanged
//     // also enabling/disabling will be reset
//     mC.setChanged(false);
//     Filter::setUnchanged();

    return res;
}

// bool BrightFilter::changed()
// {
//     return Filter::changed() || mB.changed();   
// }

Parameter* BrightFilter::getBrightness()
{
    return &mB;
}
