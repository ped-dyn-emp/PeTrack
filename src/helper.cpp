#include "helper.h"

#include <opencv.hpp>

using namespace::cv;

QString commandLineOptionsString = QObject::tr(
"<p><code>petrack [-help|-?] [[-project] project.pet] </code><br>"
"&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<code>[-sequence imageSequenceOrVideo]</code><br>"
"&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<code>[-autoSave|-autosave imgFldOrVideo|proj.pet|trackerFile]</code></code><br>"
"&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<code>[-autoTrack|-autotrack trackerFile]</code><br>"
"&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<code>[-autoPlay|-autoplay trackerFile]</code></p>"
"<dl><dt><kbd>-help|-?</kbd></dt><dd>shows help information for command line options</dd>"
"<dt><kbd>-project</kbd></dt><dd>optional option to set project file; otherwise the argument without option flag is used as project file</dd>"
"<dt><kbd>-sequence imageSequenceOrVideo</kbd></dt><dd>loads image sequence or video; option overwrites <kbd>SRC</kbd> attribute in project file</dd>"
"<dt><kbd>-autoSave|-autosave imgFldOrVideo|proj.pet|trackerFile</kbd></dt><dd>if the argument ends with <kbd>pet</kbd>, a project file will be written to <kbd>proj.pet</kbd> at the end; if the argument ends with <kbd>txt,dat </kbd>or<kbd> trav</kbd>, the trajectories will be written in a format according to the suffix of <kbd>trackerFile</kbd>; otherwise <kbd>imgFldOrVideo</kbd> is the folder to store the image sequence or a name of a video file for the direct export; in all cases <kbd>PeTrack</kbd> ends after finishing the work</dd>"
"<dt><kbd>-autoTrack|-autotrack trackerFile</kbd></dt><dd>calculates automatically the trajectories of marked pedestrians and stores the result to <kbd>trackerFile</kbd></dd>"
"<dt><kbd>-autoPlay|-autoplay trackerFile</kbd></dt><dd>plays the video or image sequence and stores the trajectories to <kbd>trackerFile</kbd></dd></dl>"
"<p>Example:<br>To generate trajectories from a single image sequence starting with <kbd>frame000.jpg</kbd>"
"with settings stored in the project file <kbd>project.pet</kbd>, export tracker file <kbd>trackerFile</kbd>"
"and exit with saving the project to <kbd>project.pet</kbd> again:</p>"
"<p><code>petrack.exe -project project.pet -sequence frame000.jpg</code><br>"
"&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<code>-autoTrack trackerFile -autoSave project.pet</code></p>");




void copyToQImage(QImage *qImg, cv::Mat &img) // war static functin in animatioln class
{
//    debout << "pointer " << iplImg << " to " << qImg << endl;
    // Check if the sizes are the same
//    debout << "iplImage: " << iplImg->width << "x" << iplImg->height << endl;
//    debout << "qImg    : " << qImg->width() << "x" << qImg->height() << endl;

//    Mat img = cvarrToMat(iplImg);

//    if (qImg->height() != img.rows || qImg->width() != img.cols)
//    {
//        if (!qImg->isNull())
//            delete qImg;
//        qImg = new QImage(QSize(img.cols,img.rows),QImage::Format_RGB32);
//    }

    if (qImg->height() != img.rows || qImg->width() != img.cols) //!qImg || ( or qImg has no object behind pointer
    {
        // If it exists, delete it and then create a new QImage with the size of the IplImage
        if (!qImg->isNull())
            delete qImg;
//        qImg = NULL;
        qImg = new QImage(QSize(img.cols,img.rows),QImage::Format_RGB888);
//        qImg = new QImage(QSize(img.cols,img.rows),QImage::Format_RGB32);
    }

//    Mat _tmp, image = cvarrToMat(iplImg);

//    debout << "image.type(): " << image.type() << endl;

//    switch (image.type()) {
//    case CV_8UC1:
//            cvtColor(image, image, CV_GRAY2RGB);
//            break;
//    case CV_8UC3:
////            image = image;
//            cvtColor(image, image, CV_BGR2RGB);
//            break;
//    case CV_8UC4:
//            debout << "unknown behavior: type=" << image.type() << endl;
//            break;
//    default:
//            debout << "unknown behavior: type=" << image.type() << endl;
//            break;
//    }

//    // QImage needs the data to be stored continuously in memory
//    assert(image.isContinuous());
//    // Assign OpenCV's image buffer to the QImage. Note that the bytesPerLine parameter
//    // (http://qt-project.org/doc/qt-4.8/qimage.html#QImage-6) is 3*width because each pixel
//    // has three bytes.
//    namedWindow("test");
//    imshow("test",image);
//    waitKey();

//    qImg = new QImage((uchar*) image.data, image.cols, image.rows, image.step/*cols*3*/, QImage::Format_RGB888);// 888 ???

////    QImage _qTmp =
////    qImg->bits() = _qTmp.bits();

//    return;

    int x,y;
    // Pointer to the data information in the IplImage
//    uchar *data = /*(char*)*/ img.data;//->imageData;
//    uchar *yData = data;
    int channels = img.channels();//->nChannels;
//    char *p;

    if (channels == 3)
    {
        // This loop is optimized so it has to calculate the least amount of indexes
        // Optimizing the access to the pointer data is useless (no difference in performance when tested)
        for (y = 0; y < img.rows; y++)
        {
            // Pointer to the data information in the QImage for just one column
            // set pointer to value before, because ++p is faster than p++
//            p = ((char*)qImg->scanLine(y))-1;
            for (x = 0; x < img.cols; x++)
            {
                Vec3b colour = img.at<Vec3b>(Point(x, y));

                QRgb value = qRgb(colour.val[2], colour.val[1], colour.val[0]);
                qImg->setPixel(x, y, value);
//                *(++p) = colour.val[0];//*(data);
//                *(++p) = colour.val[1];//*(data+1);
//                *(++p) = colour.val[2];//*(data+2);
//                *(++p) = 255;
                //printf("%02x%02x%02x ", (int)*(data), (int)*(data+1), (int)*(data+2));
//                data += 3; //channels;
            }
//            data = (yData += img.cols/*widthStep*/); // because sometimes widthStep != width
            //printf("\n");
        }
    }
    else if (channels == 1)
    {
        // This loop is optimized so it has to calculate the least amount of indexes
        // Optimizing the access to the pointer data is useless (no difference in performance when tested)
        for (y = 0; y < img.rows; y++)
        {
            // Pointer to the data information in the QImage for just one column
            // set pointer to value before, because ++p is faster than p++
//            p = ((char*)qImg->scanLine(y))-1;
            for (x = 0; x < img.cols; x++)
            {
                Scalar colour = img.at<uchar>(Point(x, y));
                qImg->setPixel(x,y,colour.val[0]);
//                *(++p) = colour.val[0];//*(data);
//                *(++p) = colour.val[0];//*(data);
//                *(++p) = colour.val[0];//*(data);
//                *(++p) = 255;
                //printf("%02x ", (int)*(data));
//                data++;// += channels;
            }
//            data = (yData += img.cols/*->widthStep*/); // because sometimes widthStep != width
            //printf("\n");
        }
    }
//    else if (channels == 4)
//    {
//        // This loop is optimized so it has to calculate the least amount of indexes
//        // Optimizing the access to the pointer data is useless (no difference in performance when tested)
//        for (y = 0; y < iplImg->height; y++)
//        {
//            // Pointer to the data information in the QImage for just one column
//            // set pointer to value before, because ++p is faster than p++
//            p = ((char*)qImg->scanLine(y))-1;
//            for (x = 0; x < iplImg->width; x++)
//            {
//                *(++p) = *(data);
//                *(++p) = *(data+1);
//                *(++p) = *(data+2);
//                *(++p) = *(data+3);
//                //*(++p) = 255;
//                //printf("%02x%02x%02x ", (int)*(data), (int)*(data+1), (int)*(data+2));
//                data += 4; //channels;
//            }
//            data = (yData += iplImg->widthStep); // because sometimes widthStep != width
//            //printf("\n");
//        }
//    }
    else
        cout << "Error: " << channels << " channels are not supported!" << endl;

//    debout << "copyImage ende" << endl;
}
#ifndef STEREO_DISABLED
void copyToQImage(QImage *qImg, IplImage *iplImg)
{
    Mat tempMat = cvarrToMat(iplImg);
    copyToQImage(qImg,tempMat);
}
#endif

#ifndef STEREO_DISABLED
void showImg(QLabel *l, IplImage *i)
{
    QImage qImg(i->width, i->height, QImage::Format_RGB32);
    copyToQImage(&qImg, i);
    l->setPixmap(QPixmap::fromImage(qImg));
    l->show();
}
#endif

// get roi
// no copy of data, only new header which allows to access rect
// copys roi to rect by set values of roi to correct values inside rect
// rect wird veraendert, roi nicht
Mat getRoi(Mat &img, const QRect &roi, Rect &rect, bool evenPixelNumber)
{
//    Mat tImg;
//    IplImage *header;
//    CvSize size;
//    CvMat *p = NULL;

    rect.x = roi.x();
    rect.y = roi.y();
    rect.width = roi.width();
    rect.height = roi.height();
    if (evenPixelNumber)
    {
        // roi.width and roi.height must be even
        if (rect.width % 2 > 0)
            --rect.width;
        if (rect.height % 2 > 0)
            --rect.height;
    }
    // roi.width and roi.height must be >=0
    if (rect.x < 0)
    {
        rect.width += rect.x;
        rect.x = 0;
    }
    else if (rect.x > img.cols)
    {
        rect.width = 0;
        rect.x = img.cols;
    }
    if (rect.x+rect.width > img.cols)
    {
        rect.width -= (rect.x+rect.width-img.cols);
    }
    if (rect.y < 0)
    {
        rect.height += rect.y;
        rect.y = 0;
    }
    else if (rect.y > img.rows)
    {
        rect.height = 0;
        rect.y = img.rows;
    }
    if (rect.y+rect.height > img.rows)
    {
        rect.height -= (rect.y+rect.height-img.rows);
    }

//    PROCESS_MEMORY_COUNTERS pMC;
//    DWORD a;
//    GetProcessMemoryInfo(GetCurrentProcess(),&pMC, a);
//    debout << a << endl;
//    debout << pMC.cb << endl;
//    debout << pMC.PageFaultCount << endl;
//    debout << pMC.PeakWorkingSetSize << endl;
//    debout << pMC.WorkingSetSize << endl;
//    debout << pMC.QuotaPeakPagedPoolUsage << endl;
//    debout << pMC.QuotaPagedPoolUsage << endl;
//    debout << pMC.QuotaPeakNonPagedPoolUsage << endl;
//    debout << pMC.QuotaNonPagedPoolUsage << endl;
//    debout << pMC.PagefileUsage << endl;
//    debout << pMC.PeakPagefileUsage << endl;

// ----------------------------------------- folgende Zeilen -----------------------------------------------------------
//    //cvSetImageROI(img, rect); - absturz!
//    // no copy of data, only new header which allows to access rect
//    // must be done with mat, no image equivalents!
//    if (img->nChannels == 3)
//        p = cvCreateMatHeader(rect.height, rect.width, CV_32SC4); // CV_32SC4 mgl Problem bei graubildern
//    else if (img->nChannels == 1)
//        p = cvCreateMatHeader(rect.height, rect.width, CV_8SC1);
//    else
//    {
//        debout << "Error: wrong number of channels: " << img->nChannels << endl;
//        return NULL;
//    }
//    cvGetSubRect(img, p, rect); // rect ausschnitt von img wird in imageheader p gepackt (keine Datenkopie noetig)
//    size.width = rect.width;
//    size.height = rect.height;
//    header = cvCreateImageHeader(size, img->depth, img->nChannels);
//    tImg = cvGetImage(p, header);
//    cvFree(&p); // (void**) only header
// -------------------------- durch diese ersetzt, da es beim Verschieben des linken ROI Randes zum Absturz bei Zugriff auf tImg kam!!!!!!!!!!!
    /* copy ROI to subimg */
//    tImg = cvCreateImage(cvSize(rect.width,rect.height), img->depth, img->nChannels);
//    cvSetImageROI(img, rect);
//    cvCopy(img, tImg, NULL); // kopiert nur ROI
//    cvResetImageROI(img);
    // am schoensten waere natuerlich man wuerde roi setzen und die find-routinen wuerden nur diesen Bereich beachten
//-------------------------------------------------------------------------------------------------------

    return img(rect);

    //int x, y;
    //int channels = img->nChannels;
    //// Pointer to the data information in the IplImage
    //unsigned char *data  = (unsigned char *) img->imageData;
    //     // mark white regions green
    //     for (y = 0; y < img->height; ++y)
    //     {
    //         for (x = 0; x < img->width; ++x)
    //         {
    //             if (data[0] > 150 && data[1] > 150 && data[2] > 150)
    //                 // eher grautoene statt farben: && data[0]-data[1] < 5 && data[1]-data[2] < 5)
    //             {
    //                 data[0] = 0;
    //                 data[1] = 255;
    //                 data[2] = 0;
    //             }
    //             data += channels;
    //         }
    //     }
}
