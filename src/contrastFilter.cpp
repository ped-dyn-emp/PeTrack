#include "contrastFilter.h"

ContrastFilter::ContrastFilter()
    :Filter()
{
    mC.setMinimum(0.);
    mC.setMaximum(255.);
    mC.setValue(0.);
    mC.setFilter(this);
}

IplImage* ContrastFilter::act(IplImage *img, IplImage *res)
{
    int x, y;
    float val;
    // Pointer to the data information in the IplImage
    // using (unsigned char *) garantees values 0..255 (mingw under windows results -128..127 for char)
    unsigned char *dataIn  = (unsigned char *) img->imageData; 
    unsigned char *dataOut = (unsigned char *) res->imageData; 
    // int channels = img->nChannels;
  
    // set poiner to value before array, because ++i is more effective than i++
    --dataIn; --dataOut; 

    // Contrast factor
    float fac = 256./(256.-mC.getValue());

    // This loop is optimized so it has to calculate the least amount of indexes
    // Optimizing the access to the pointer data is useless (no difference in performance when tested)

//     int maxval = -1000;
//     int minval = 1000;
//     for (y = 0; y < img->height; ++y)
//     {
//         for (x = 0; x < img->width; ++x)
//         {
//             if (*(dataIn+x+y*img->width) > maxval)
//                 maxval = *(dataIn+x+y*img->width);
//             if (*(dataIn+x+y*img->width) < minval)
//                 minval = *(dataIn+x+y*img->width);
//         }
//     }
//     cout << minval << " " <<maxval <<endl;

    for (y = 0; y < img->height; ++y)
    {
        for (x = 0; x < img->width; ++x)
        {
            val = (*(++dataIn)-128) * fac; // (signed char) to move to -128..127
            *(++dataOut) = (unsigned char) MAX(MIN(val+128, 255), 0);
            val = (*(++dataIn)-128) * fac;
            *(++dataOut) = (unsigned char) MAX(MIN(val+128, 255), 0);
            val = (*(++dataIn)-128) * fac;
            *(++dataOut) = (unsigned char) MAX(MIN(val+128, 255), 0);
            // so wuerde man durchschleifen
//             *(++dataOut) = *(++dataIn);
//             *(++dataOut) = *(++dataIn);
//             *(++dataOut) = *(++dataIn);
            // dataIn+=channels; // es wird von 3 kanaelen ausgegenagen
        }
    }

    return res;
}

Parameter* ContrastFilter::getContrast()
{
    return &mC;
}
