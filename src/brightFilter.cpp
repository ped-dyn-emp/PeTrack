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

#include "brightFilter.h"

BrightFilter::BrightFilter()
    :Filter()
{
    mB.setMinimum(0.);
    mB.setMaximum(255.);
    mB.setValue(0.);
    mB.setFilter(this);
}

#ifndef STEREO_DISABLED
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
#endif
Parameter* BrightFilter::getBrightness()
{
    return &mB;
}
