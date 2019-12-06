#include "blurFilter.h"
#include "cv.h"
#include "opencv2/opencv.hpp"

using namespace::cv;

Mat BlurFilter::act(Mat &img, Mat &res){

    GaussianBlur(img,res,Size(0,0),p*10+1,p*10+1);

    return res;
}

void BlurFilter::setParam(double newp){
    p = newp;
}

