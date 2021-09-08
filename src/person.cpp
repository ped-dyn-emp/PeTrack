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

#include "person.h"

#include "helper.h"

//#define SHOWELLIPSES // gibt die einzelnen schritte der personen detektion pyramide graphisch aus
//#define SAVEELLIPSES // ob alle ellips in datei geschrieben werden sollen
//#define PRINT_ERASE_REASON // zeigt an, ob ausgegeben werden soll, aus welchem Grund eine ellipse geloescht wurde
//#define SAVE_IMG_DETECTION_STEPS // setzen, wenn die gezeigten Bilder (SHOWELLIPSES) auch als Bildsequenz
// weggeschrieben werden soll #define SAVE_IMG_FILEBASE "D:/diss/personModel/ownPerson/petrack/density/d4.5_"

using namespace ::cv;

PersonList::PersonList()
{
    mSc = nullptr;
}

#ifndef STEREO_DISABLED
// searching for ellipses from isolines in a height field
void PersonList::searchEllipses(pet::StereoContext *sc, QRect &roi, BackgroundFilter *bgFilter)
{
    mSc = sc;

    Mat       pointCloud = cvarrToMat(mSc->getPointCloud()).clone();
    IplImage *disp       = mSc->getDisparity();
    CvSize    imgSize;
    imgSize.width  = disp->width;
    imgSize.height = disp->height;

#ifdef TIME_MEASUREMENT
    //        "==========: "
    debout << "go mkless : " << getElapsedTime() << endl;
#endif

    int    x, y;
    float *pcData, *yPcData;

    // nicht Fordergrund auf ungueltig -1 setzen
    bgFilter->maskBg(pointCloud, -1.);

    // Bestimmung von Minimum und Maximum von z in Meter
    float min = FLT_MAX;
    float max = FLT_MIN;
    pcData    = (float *) pointCloud.data;
    yPcData   = pcData;

    for(y = 0; y < pointCloud.rows; ++y)
    {
        for(x = 0; x < pointCloud.cols; ++x)
        {
            if(pcData[2] != -1)
            {
                if(pcData[2] > max)
                    max = pcData[2];
                else if(pcData[2] < min)
                    min = pcData[2];
            }
            pcData += 3;
        }
        pcData = (yPcData += pointCloud.cols / sizeof(float));
    }
    // pcData, min, max sind in Meter !!!!!!!!

    if(min >= max)
    {
        //        cvReleaseMat(&pointCloud);
        return;
    }

    // debout << "Minimal distance to camera in foreground: " << min*100 << "cm" <<endl;
    // debout << "Maximal distance to camera in foreground: " << max*100 << "cm" <<endl;
    if(max - min < 1)
        debout << "Warning: hight field difference is smaller 1m!" << endl;
    //    debout << sc->getCmPerPixel(min) <<endl;
    //    debout << sc->getCmPerPixel(max) <<endl;
    //    debout << sc->getZfromDisp(sc->getMin()) <<endl;
    //    debout << sc->getZfromDisp(sc->getMax()) <<endl;
    float l1 = 4.2 / mSc->getCmPerPixel(min);
    float l2 = 67. / mSc->getCmPerPixel(max);
    //    debout << l1*l1 <<endl;
    //    debout << l2*l2 <<endl;

    // interpolate z-values inbetween innerhalb zeile
    float *vorPtr;  // zeigt auf den letzten gueltigen Wert vor ungueltigem wert in einer Zeile
    float *nachPtr; // zeigt auf den ersten gueltigen Wert nach ungueltigen in einer Zeile
    float *fPtr;
    float  step;

    pcData  = (float *) pointCloud.data;
    yPcData = pcData;

    for(y = 0; y < pointCloud.rows; ++y)
    {
        vorPtr = nachPtr = NULL;
        for(x = 0; x < (pointCloud.cols) - 1; ++x)
        {
            if(vorPtr == NULL && pcData[2] != -1 && pcData[5] == -1) // eine unbestimmte gap beginnt
                vorPtr = pcData;
            else if(vorPtr != NULL && nachPtr == NULL && pcData[2] == -1 && pcData[5] != -1) // eine unbestimmte gap
                                                                                             // endet
                nachPtr = pcData + 3;
            if(vorPtr != NULL && nachPtr != NULL) // gap bestimmt, fuellung mit interpolerten werten
            {
                if((nachPtr - vorPtr) <
                   3 * DISP_GAP_SIZE_TO_FILL) // nur kleine luecken von weniger als 15 pixel fuellen
                {
                    step =
                        ((nachPtr[2] - vorPtr[2]) /
                         (nachPtr - vorPtr)); // lange zeit war falsch: /9.; // /3 ein /3 durch (fPtr-vorPtr) und nicht
                                              // (fPtr-vorPtr)/3 in schleife
                    for(fPtr = vorPtr + 3; fPtr < nachPtr; fPtr += 3)
                    {
                        // fPtr[0] = ; // x
                        // fPtr[1] = ; // y
                        fPtr[2] = vorPtr[2] + (fPtr - vorPtr) * step;
                    }
                }
                vorPtr = nachPtr = NULL;
            }
            pcData += 3;
        }
        pcData = (yPcData += pointCloud.cols / sizeof(float));
    }

    // grauwertbild erstellen zwischen min und max - ungueltige werte auf 255 setzen
    IplImage *     gray      = cvCreateImage(imgSize, IPL_DEPTH_8U, 1);
    unsigned char *grayData  = (unsigned char *) gray->imageData;
    unsigned char *yGrayData = grayData;

    pcData      = (float *) pointCloud.data;
    yPcData     = pcData;
    float scale = 254. / (max - min);

    for(y = 0; y < pointCloud.rows; ++y)
    {
        for(x = 0; x < pointCloud.cols; ++x)
        {
            if(pcData[2] == -1)
                *grayData = 255;
            else
                *grayData = scale * (pcData[2] - min);
            pcData += 3;
            ++grayData;
        }
        pcData   = (yPcData += pointCloud.cols / sizeof(float));
        grayData = (yGrayData += gray->width);
    }

    //    // umkopieren der pointcloud auf matrix mit nur z-werten
    //    IplImage *zPointCloud = cvCreateImage(cvGetSize(pointCloud),32,1);
    //    cvSplit(pointCloud, NULL, NULL, zPointCloud, NULL);
    //    // nun muesste noch -1 auf max gesetzt werden, damit isolinen richtig herum
    //    verlaufen!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    int i, count;
    // int threshold;
    float     threshold;
    double    angle;
    CvSeq *   contours, *firstContour;
    IplImage *binImg = cvCreateImage(imgSize, IPL_DEPTH_8U, 1);
    ;
    CvPoint *     PointArray;
    CvPoint2D32f *PointArray2D32f;
    // CvPoint center;
    // CvSize sizeTmp;
    // int expansion;
    double contourArea;
    // double contourLength;
    CvMemStorage *storage = cvCreateMemStorage(0);
    float         dist;
    bool          contourInside; // gigt an, ob die contour ueber den rand der bounding box roi hinweggeht

    int frameNum = mSc->getAnimation()->getCurrentFrameNum();
#ifdef SHOWELLIPSES
    IplImage *tmpAusgabe  = cvCreateImage(imgSize, IPL_DEPTH_8U, 3); // cvCloneImage(gray); // make a copy
    IplImage *tmpAusgabe2 = cvCreateImage(imgSize, IPL_DEPTH_8U, 3); // cvCloneImage(gray); // make a copy
    IplImage *tmpAusgabe3 = cvCreateImage(imgSize, IPL_DEPTH_8U, 3); // cvCloneImage(gray); // make a copy
    cvCvtColor(gray, tmpAusgabe, CV_GRAY2BGR);
    cvCvtColor(mSc->getRectified(cameraRight), tmpAusgabe2, CV_GRAY2BGR);
    cvCvtColor(gray, tmpAusgabe3, CV_GRAY2BGR);
#endif
#ifdef SAVEELLIPSES
    QFile ellipsFile("ellipses.txt");
    if(!ellipsFile.open(QIODevice::Append | QIODevice::Text))
        debout << "Error: Cannot open ellipses.txt: " << ellipsFile.errorString();
    QTextStream ellipsOut(&ellipsFile);
#endif

#ifdef TIME_MEASUREMENT
    //        "==========: "
    debout << "vor cont  : " << getElapsedTime() << endl;
#endif

    step = 2.55 / ((max - min) / STEP_SIZE); // STEP_SIZE cm Schritte, daher 2.55, da min und max in meter
    for(threshold = step; threshold < 255 - step; threshold += step) // von kopf zum fuss = von klein nach gross
    // for (threshold = 10; threshold < 250 ; threshold += 10) // von kopf zum fuss = von klein nach gross
    // for (threshold = min+step; threshold < max-step ; threshold += step) // von kopf zum fuss = von klein nach gross
    {
        dist = min * 100 + STEP_SIZE * threshold / step;
        QList<MyEllipse> el; // jeder level neu anlegen
        cvThreshold(gray, binImg, threshold, 255, CV_THRESH_BINARY);
        // cvThreshold(zPointCloud, binImg, threshold, 255, CV_THRESH_BINARY);

        // find contours and store them all as a list
        cvFindContours(
            binImg,
            storage,
            &firstContour,
            sizeof(CvContour),
            CV_RETR_LIST,
            CV_CHAIN_APPROX_SIMPLE); // binImg wird auch veraendert!!!
        contours = firstContour;

        // test each contour
        while(contours)
        {
            count = contours->total; // This is number point in contour

            if(count > 5) // fuer ellips fit mind 6 pkte benoetigt
            {
                // in rot contouren in eine drehrichtung, in gruen andere
#ifdef SHOWELLIPSES
                cvDrawContours(tmpAusgabe, contours, CV_RGB(255, 0, 0), CV_RGB(0, 255, 0), 0, 1, 8, cvPoint(0, 0));
#endif

                // IN OPENCV2.1 liefert cvContourArea KEIN VORZEICHEN ZUM ERKENNEN DER DREHRICHTUNG!!!! es ist ein
                // optionaler paramter hinzugefuegt worden!!!!
#if((CV_MAJOR_VERSION < 2) || ((CV_MAJOR_VERSION == 2) && (CV_MINOR_VERSION < 1)))
                contourArea = cvContourArea(contours, CV_WHOLE_SEQ);
#else
                contourArea = cvContourArea(contours, CV_WHOLE_SEQ, true);
#endif
                //                        if (contours->flags & CV_SEQ_FLAG_HOLE) == 0)
                //                            debout << "Negative contourArea! If not using the absolute size, the flag
                //                            shows already the direction!" << endl;

                // war: (contourArea>50 && contourArea<8000) // sollte abhaengig von untersuchten groessen gemacht
                // werden
                //      entspricht:
                //  < 4.2cm x 4.2cm=18cm^2  = 50 Pixel in maximaler personengroesse
                // in maximaler Entfernung zur Kamera
                //(in ebenen Versuchen entspricht dies 40cm über Bodenhöhe)
                // größer als 4500cm^2 = 8000 Pixel

                if(contourArea > l1 * l1 && contourArea < l2 * l2)
                {
                    // Alloc memory for contour point set.
                    PointArray      = (CvPoint *) malloc(count * sizeof(CvPoint));
                    PointArray2D32f = (CvPoint2D32f *) malloc(count * sizeof(CvPoint2D32f));

                    // cv::Mat PointMat = cv::Mat(1, count, CV_32SC2, contours);
                    // debout << cv::contourArea(PointMat) <<endl; // cvContourBoundingRect(...)
                    // debout << cv::isContourConvex(PointMat) <<endl;
                    // HoughCircles: Finds circles in a grayscale image using a Hough transform.

                    // Get contour point set.
                    cvCvtSeqToArray(contours, PointArray, CV_WHOLE_SEQ);

                    // Convert CvPoint set to CvBox2D32f set.
                    contourInside = true;
                    for(i = 0; i < count; i++)
                    {
                        x = PointArray[i].x;
                        y = PointArray[i].y;
                        //                        // delete point at blob border
                        //                        if (!((gray->imageData[x+y*gray->width-1]   == 255) ||
                        //                              (gray->imageData[x+y*gray->width+1]   == 255) ||
                        //                              (gray->imageData[x+(y-1)*gray->width] == 255) ||
                        //                              (gray->imageData[x+(y+1)*gray->width] == 255))) // nicht am rand
                        //                        {
                        PointArray2D32f[i].x = (float) x;
                        PointArray2D32f[i].y = (float) y;
                        //                        }
                        //                        else
                        //                        {
                        //                            --count;
                        //                            debout << "del contour point " << x << " " << y << endl;
                        //                        }
                        if(x < roi.left() || x > roi.right() || y < roi.top() || y > roi.bottom())
                            contourInside = false;
                    }
                    //                            contourLength = 0;
                    //                            for(i=1; i<=count; i++)
                    //                            {
                    //                                contourLength+=sqrt((PointArray[i%count].x-PointArray[i-1].x)*(PointArray[i%count].x-PointArray[i-1].x)
                    //                                +
                    //                                (PointArray[i%count].y-PointArray[i-1].y)*(PointArray[i%count].y-PointArray[i-1].y));
                    //                            }

                    if(count > 5 && contourInside)
                    {
#ifdef SHOWELLIPSES
                        cvDrawContours(
                            tmpAusgabe,
                            contours,
                            CV_RGB(255, 0, 255),
                            CV_RGB(0, 255, 255),
                            0,
                            1,
                            8,
                            cvPoint(
                                0,
                                0)); // um anzuzeigen welche wirklich genommen wurden
#endif

#if CV_MAJOR_VERSION == 2
                                     // Fits ellipse to current contour.
                                     // -------------------------------------------------------------------------------
                        CvBox2D32f box;
                        cvFitEllipse(PointArray2D32f, count, &box);
#elif CV_MAJOR_VERSION == 3
                        // neuer:
                        //// Fits ellipse to current contour.
                        //                        CvBox2D box = cvFitEllipse2(PointArray2D32f);
                        Mat mat(count, 2, CV_32F);
                        for(i = 0; i < count; i++)
                        {
                            mat.at<float>(i, 0) = (float) PointArray[i].x;
                            mat.at<float>(i, 1) = (float) PointArray[i].y;
                        }
                        RotatedRect box = fitEllipse(mat);
#endif
                        // Convert ellipse data from float to integer representation.
                        // center.x = myRound(box.center.x);
                        // center.y = myRound(box.center.y);
                        // box.angle = -box.angle;
                        // sizeTmp.width = myRound(box.size.width*0.5);
                        // sizeTmp.height = myRound(box.size.height*0.5);

                        angle = (box.angle) / 180. * PI;
                        if(box.size.width < box.size.height) // da bei meiner ellipse r1 immer der groesste radius
                            angle -= PI / 2;

                        MyEllipse elips(box.center.x, box.center.y, box.size.width * 0.5, box.size.height * 0.5, angle);
                        //                            debout << elips.area() << " " << contourArea << " " <<
                        //                            contourLength<< " " <<
                        //                            contourLength*contourLength/contourArea<<endl;
                        // if (elips.area()/contourArea < 1.2) // elips soll aehnliche flaechengroesse haben
                        // if (contourLength/contourArea < 0.2) // elips soll aehnliche flaechengroesse haben
                        // if (contourLength*contourLength/contourArea < 40.) // quadratisch, da in flaeche r
                        // quadratisch eingeht
                        //                        if (elips.r1() < 80) // wird nun beim einfuegen in person mit
                        //                        beruecksichtigt, damit an einer stelle ellipse ueberprueft wird!! - r1
                        //                        ist immer der groessere radius - um 2 Personen in einer ellipse zu
                        //                        vermeiden
                        //                        {
                        el.append(elips);
                        // cvEllipse(tmpAusgabe2, center, sizeTmp, box.angle, 0, 360, CV_RGB(255, 0, 0), 1, CV_AA, 0);
                        // cvEllipseBox(tmpAusgabe3, box, CV_RGB(255, 0, 0));

                        //                        }
                        //                        else
                        //                            debout << "del elips, because to big " << box.center.x <<" "<<
                        //                            box.center.y <<endl; if (contourLength*contourLength/contourArea
                        //                            >= 40.)
                        //                                debout << "l*l/a: " <<box.center.x <<" "<< box.center.y
                        //                                <<endl;
                        //                            if (contourLength/contourArea >= 0.2)
                        //                                debout << "l/a:   " <<box.center.x <<" "<< box.center.y
                        //                                <<endl;
                        //                            if (elips.area()/contourArea >= 1.2)
                        //                                debout << "ea/ca: " <<box.center.x <<" "<< box.center.y
                        //                                <<endl;
                    }
                    std::free(PointArray);
                    std::free(PointArray2D32f);
                }
            }
            // take the next contour
            contours = contours->h_next;
        }
#ifdef TIME_MEASUREMENT
        //        "==========: "
        debout << "vor insert: " << getElapsedTime() << endl;
#endif

        insertEllipses(el, dist);

#ifdef SHOWELLIPSES
        for(int i = 0; i < el.size(); ++i)
        {
            el.at(i).draw(tmpAusgabe3, 0, 255, 0);
        }
#endif
#ifdef SAVEELLIPSES
        // anhaengen der ellipsen in ausgabedatei zur spaeteren analyse
        // frame hoehe/z x(px) y(px) elips.winkel elips.r1 elips.r2
        for(int i = 0; i < el.size(); ++i)
        {
            ellipsOut << frameNum << " " << dist << " " << el.at(i).x() << " " << el.at(i).y() << " "
                      << el.at(i).angle() << " " << el.at(i).r1() << " " << el.at(i).r2() << " " << 0 << endl;
        }
#endif

        //  sprintf(outstr,"c:/%d.png",threshold);
        // cvSaveImage(outstr, tmpAusgabe);
        // cvWaitKey( 0 ); // zahl statt null, wenn nach bestimmter zeit weitergegangen werden soll

        // nicht noetig, aber so kann der neu erzeugte speicherplatz reduziert werden
        // freigabe des speicherplatz erst bei cvClearMemStorage(storage)
        if(firstContour)
            cvClearSeq(firstContour); // not free only available for next push
    }
#ifdef TIME_MEASUREMENT
    //        "==========: "
    debout << "nach cont : " << getElapsedTime() << endl;
#endif


    optimize();

#ifdef SHOWELLIPSES
    for(int i = 0; i < size(); ++i)
        for(int j = at(i).size() - 1; j >= 0;
            --j) // rueckwwaerts, damit hoeher liegende ellipsen oben aufgezeichnet werden
        {
            if(j == 0)
                at(i).at(j).draw(tmpAusgabe2, 0, 255, 0);
            else if(j == 1)
                at(i).at(j).draw(tmpAusgabe2, 255, 0, 0);
            else if(j == 2)
                at(i).at(j).draw(tmpAusgabe2, 0, 0, 255);
            else
                at(i).at(j).draw(tmpAusgabe2, 150, 150, j * 255 / at(i).size());
            // debout << i << " " << j << " " << pl.at(i).at(j).center().x() << " " << pl.at(i).at(j).center().y()<< " "
            // << pl.at(i).at(j).area() <<endl;
        }

    //    char outstr[120];
    //    //static int zaehl = 0;
    //    sprintf(outstr,"D:/data/ko/ko-240-240-240_cam2_isolines/ko-240-240_cam2_isolines_%04d.png",frameNum);
    //    cvSaveImage(outstr, tmpAusgabe);
    //    sprintf(outstr,"D:/data/ko/ko-240-240-240_cam2_ellipses/ko-240-240_cam2_ellipses_%04d.png",frameNum);
    //    cvSaveImage(outstr, tmpAusgabe2);
    //    sprintf(outstr,"D:/data/ko/ko-240-240-240_cam2_persons/ko-240-240_cam2_persons_%04d.png",frameNum);
    //    cvSaveImage(outstr, tmpAusgabe3);

    // cvNamedWindow("img", CV_WINDOW_AUTOSIZE ); // 0 wenn skalierbar sein soll
    // cvShowImage("img", tmpAusgabe);


    static QLabel imgLabel;
    showImg(&imgLabel, tmpAusgabe);
    static QLabel imgLabel2;
    showImg(&imgLabel2, tmpAusgabe2);
    static QLabel imgLabel3;
    showImg(&imgLabel3, tmpAusgabe3);

#ifdef SAVE_IMG_DETECTION_STEPS
    static int  frame = 0;
    static char outstr[1024];
    debout << "Images " << frame << " of detection steps are saved to basename " << SAVE_IMG_FILEBASE << " !" << endl;
    sprintf(outstr, "%scontour_%03d.png", SAVE_IMG_FILEBASE, frame);
    cvSaveImage(outstr, tmpAusgabe);
    sprintf(outstr, "%spyramide_%03d.png", SAVE_IMG_FILEBASE, frame);
    cvSaveImage(outstr, tmpAusgabe2);
    sprintf(outstr, "%sellipses_%03d.png", SAVE_IMG_FILEBASE, frame);
    cvSaveImage(outstr, tmpAusgabe3);
    frame++;
#endif

    cvReleaseImage(&tmpAusgabe);
    cvReleaseImage(&tmpAusgabe2);
    cvReleaseImage(&tmpAusgabe3);
#endif
#ifdef SAVEELLIPSES
    for(int i = 0; i < size(); ++i)
        for(int j = at(i).size() - 1; j >= 0;
            --j) // rueckwwaerts, damit hoeher liegende ellipsen oben aufgezeichnet werden
        {
            ellipsOut << frameNum << " " << at(i).getDistTopEllipse() + j * STEP_SIZE << " " << at(i).at(j).x() << " "
                      << at(i).at(j).y() << " " << at(i).at(j).angle() << " " << at(i).at(j).r1() << " "
                      << at(i).at(j).r2() << " " << i + 1
                      << endl; // i+1, damit 0 den rohen ellipsen vorbehaqlten bleibt
        }
    ellipsFile.close();
#endif

    //    cvReleaseMat(&pointCloud);
    // cvReleaseImage(&zPointCloud);
    cvReleaseImage(&gray);
    cvReleaseImage(&binImg);
    cvReleaseMemStorage(&storage);
#ifdef TIME_MEASUREMENT
    //        "==========: "
    debout << "ed search : " << getElapsedTime() << endl;
#endif
}

// el liste aller ellipsen eines thresholds der hoehe
// der thrershold durchlaeuft hoehenfeld von kopf bis fuss
// distFromCam in cm
void PersonList::insertEllipses(
    const QList<MyEllipse> &el,
    float                   distFromCam) // nicht mehr: el wird angepasst, so dass nur noch eingefuegte ellipsen drin
{
    int    i, j;
    bool   found;
    double minDist, dist;
    int    minIdx;
    int    sub;
    Vec2F  center;

    float r1Max =
        60. / mSc->getCmPerPixel(
                  .01 * distFromCam); // war 80 px am boden ist 60cm // radius der ellips mit maxmalem radius 80 pixel
    float r1Min =
        10. /
        mSc->getCmPerPixel(
            .01 *
            distFromCam); // war 15 px in Schulterhohe sei 10cm // laenge der grossen Hauptachse in mittlerer Kopfhöhe

    for(i = 0; i < el.size(); ++i) // ueber uebergebene liste von ellipsen
    {
        // FOLGENDE ABFRAGE KANN DAZU FUEHREN, DASS IN EINER PYRAMIDE EIN LEVEL NICHT
        // VORKOMMT!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        if((el.at(i).ratio() < 4) ||
           (el.at(i).r1() >
            r1Max)) // war: > 80 // r1 ist immer der groessere radius - um 2 Personen in einer ellipse zu vermeiden
        {
            QList<int> considered;      // beruecksichtigte bereits existierende pyramiden fuer diese groessere ellipse
            for(j = 0; j < size(); ++j) // ueber existierende Personen, deren ellipsen kleiner sind
            {
                if(el.at(i).isInside(at(j).last().center()))
                {
                    if(at(j).getActLevel()) // mehrere neue grosse ellipsen beinhalten kleine exist ellipse
                    {
                        // debout << at(j).size() << endl;
                        //  die pyramide suchen, wo schon eingefuegt und testen welcher mittelpunkt naeher liegt
                        Vec2F centerLevelBefore = at(j).at(at(j).size() - 2).center();
                        if(at(j).last().center().distanceToPoint(
                               centerLevelBefore) > // abstand der letzten beiden pyramidenelememnte (2 muessen drin
                                                    // sein, weil sonst allConsidered es nicht beeinhalten wuerde)
                           el.at(i).center().distanceToPoint(centerLevelBefore))
                        { // genommen wird die, deren center naeher an existierender liegen
#ifdef PRINT_ERASE_REASON
                            debout << "Eliminate ellipse because of a second ellipse nearer pyramide: "
                                   << (*this)[j].last().center().x() << " " << (*this)[j].last().center().y() << endl;
#endif
                            (*this)[j].removeLast();
                            (*this)[j].append(el.at(i)); // auch moeglich, aber dann mgl gar nicht: considered += j; //
                        }
                        // debout << "Fall 4" <<endl;
                        // debout << (*this)[j].last().center().x() << " " << (*this)[j].last().center().y() << endl;
                        // QMessageBox Msgbox; //#include <QMessageBox> noetig
                        // Msgbox.setText("Warten");
                        // Msgbox.exec();
                        ////cvSaveImage("D:/contour.png", tmpAusgabe);
                        ////cvSaveImage("D:/pyramide.png", tmpAusgabe2);
                        ////cvSaveImage("D:/ellipses.png", tmpAusgabe3);

                        // debout << "Warning: multiple consideration for " << centerLevelBefore.x() << " " <<
                        // centerLevelBefore.y() <<endl;
                    }
                    else
                    {
                        considered += j;
                    }
                }
            }
            if(considered.size() == 0) // neue contour pyramide
            {
                // debout << "Fall 1" <<endl;
                append(Person(el.at(i), distFromCam));
            }
            else if(considered.size() == 1) // genau ein passende pyramide
            {
                // debout << "Fall 2" <<endl;
                (*this)[considered.at(0)].append(el.at(i));
                (*this)[considered.at(0)].setActLevel(true);
            }
            else // mehrere Anwaerter exist kleinerer ellipsen die in neuer grossen liegen
            {
                // debout << "Fall 3" <<endl;
                found = false;
                // suchen der groessten pyramide
                for(j = 0; j < considered.size(); ++j)
                {
                    // wenn mind. pyramide groesser 2 uebernehmen (2 personen laufen dicht nebeneinander) mit gewisser
                    // ausdehnung
                    //                    debout << 10./STEP_SIZE <<endl;
                    //                    debout << r1Min <<endl;
                    if((at(considered.at(j)).size() > 10. / STEP_SIZE) && // war 2
                       (at(considered.at(j)).last().r1() >
                        r1Min)) // war 15 // erhebung groesser 10 cm bei 5cm level-unterteilung
                    {
                        (*this)[considered.at(j)].append(el.at(i));
                        (*this)[considered.at(j)].setActLevel(true);
                        found = true;
                        // debout << "Fall a" <<endl;
                    }
                }
                if(!found) // wenn alle pyramiden kleiner, dann die mit dem naechsten center nehmen und restliche
                           // LOESCHEN!
                {
                    // debout << "Fall b" <<endl;
                    minDist = 1000000.;
                    minIdx  = -1;
                    for(j = 0; j < considered.size(); ++j)
                    {
                        center = at(considered.at(j)).last().center();
                        if(at(considered.at(j)).size() > 1) // da elips in pyramide systematisch wandern bei mehr als 1
                                                            // elips den offset auf letzten mittelpkt draufaddieren
                            center += center - at(considered.at(j)).at(at(considered.at(j)).size() - 2).center();
                        center = at(considered.at(j)).last().center();
                        if((dist = el.at(i).center().distanceToPoint(center)) < minDist)
                        {
                            minDist = dist;
                            minIdx  = j;
                        }
                    }
                    (*this)[considered.at(minIdx)].append(el.at(i));
                    (*this)[considered.at(minIdx)].setActLevel(true);
                    // alles pyramiden ausser mit minIdx loeschen (sehr unwahrscheinlich, aber es koennte eine andere
                    // neue grosse ellipse die nun entfernte beinhgalten - pech gehabt)
                    sub = 0; // was beim fortlaufenden loeschen abgezogen werden muss, da sich index verschiebt;
                             // considered ist immer aufsteigend!!!
                    for(j = 0; j < considered.size(); ++j)
                    {
                        if(j != minIdx)
                        {
#ifdef PRINT_ERASE_REASON
                            debout << "Eliminate pyramide because other pyramid nearer, with top ellipse: "
                                   << (*this)[considered.at(j) - sub].at(0).center().x() << " "
                                   << (*this)[considered.at(j) - sub].at(0).center().y() << endl;
#endif
                            removeAt(considered.at(j) - sub);
                            ++sub;
                        }
                    }
                }
            }
        }
        else
        {
#ifdef PRINT_ERASE_REASON
            debout << "Eliminate ellipse because of big aspect ratio and small size: " << el.at(i).center().x() << " "
                   << el.at(i).center().y() << endl;
#endif
            //                debout << i << " " << el.size()<<endl;
            //                el.removeAt(i);
            //                --i;
            //                debout << "ratio falsch "<<endl;
        }
    }
    for(j = 0; j < size(); ++j) // ueber existierende Personen, deren ellipsen kleiner sind
    {
        (*this)[j].setActLevel(false);
    }
}

// eigentlich muessten alle werte in abhaengigkeit von cm-schritten bei isolinen und auf pixel/cm abgestimmt
// sein!!!!!!!!!!!!!!!!!!!
void PersonList::optimize()
{
    int minPyrLevel = myRound(20 / STEP_SIZE) + 1; // mindesthoehe von 20 cm
                                                   //    debout << minPyrLevel <<endl;
    for(int i = 0; i < size(); ++i)                // ueber uebergebene liste von ellipsen stacks
    {
        //        float r1Max = 34./mSc->getCmPerPixel(.01*(at(i).getDistTopEllipse()+10)); // war 37.cm // war 60 px //
        //        60px in kopfhoehe sind 37cm in kopfhoehe // radius der ellips mit maxmalem radius 80 pixel float
        //        areaMin = 22./mSc->getCmPerPixel(.01*(at(i).getDistTopEllipse()+100)); // 22. = 12.5*sqrt(Pi) wg area
        //        = r1*r2*Pi areaMin = areaMin*areaMin; if (at(i).size() == minPyrLevel-2 && at(i).last().area() >=
        //        areaMin && ((*(at(i).getHeadEllipse())).r1() <= r1Max))
        //            debout << "15 cm Hoehe fuegt hinzu bei "<< at(i).at(0).center().x() << " " <<
        //            at(i).at(0).center().y() << endl;
        //        if (at(i).size() == minPyrLevel && at(i).last().area() >= areaMin && ((*(at(i).getHeadEllipse())).r1()
        //        <= r1Max))
        //            debout << "25 cm Hoehe loescht bei "<< at(i).at(0).center().x() << " " << at(i).at(0).center().y()
        //            << endl;
        if(at(i).size() < minPyrLevel) // mindesthoehe von 20 cm // war 4 // pyramide eine mindesthoehe // myRound(25 cm
                                       // / STEP_SIZE) + 1
        {
#ifdef PRINT_ERASE_REASON
            debout << "Eliminate pyramide because of too small size (" << at(i).size()
                   << "), with top ellipse: " << at(i).at(0).center().x() << " " << at(i).at(0).center().y() << endl;
#endif
            removeAt(i);
            --i;
        }
    }

    // ab hier mindestens 4 ellipsen in pyramide (wenn 5 cm schritte, sonst auch weniger)
    for(int i = 0; i < size(); ++i) // ueber uebergebene liste von ellipsen stacks
    {                               // r1 ist immer die groessere ausdehnung
        // debout << at(i).at(0).center().x() << " " << at(i).at(0).center().y() << " " << at(i).last().area() << endl;
        //  at(i).getDistTopEllipse()+10) // in cm // 10 cm (eigentlich 10+step_size/2) unterhalb körpergröße
        float r1Max =
            34. /
            mSc->getCmPerPixel(
                .01 * (at(i).getDistTopEllipse() + 10)); // war 37.cm // war 60 px // 60px in kopfhoehe sind 37cm in
                                                         // kopfhoehe // radius der ellips mit maxmalem radius 80 pixel
        //        float r2Min = 12./mSc->getCmPerPixel(.01*(at(i).getDistTopEllipse()+20)); // war 60 px // 60px in
        //        kopfhoehe sind 37cm in kopfhoehe // radius der ellips mit maxmalem radius 80 pixel
        float areaMin =
            22. / mSc->getCmPerPixel(.01 * (at(i).getDistTopEllipse() + 100)); // 22. = 12.5*sqrt(Pi) wg area = r1*r2*Pi
        areaMin = areaMin * areaMin;
        //        float areaMin2 = 35.45/mSc->getCmPerPixel(.01*(at(i).getDistTopEllipse()+100)); // 22. = 20*sqrt(Pi)
        //        wg area = r1*r2*Pi areaMin2 = areaMin2*areaMin2;
        //        // 1000 px^2 entspricht r1=r2 = 18 px = 12,5cm radius in oberschenkelhöhe bzw 100cm unterhalb kopf
        //        if (at(i).last().area() < areaMin2 && at(i).last().area() >= areaMin)
        //            debout << "1250 cm2 loescht bei " << at(i).at(0).center().x() << " " << at(i).at(0).center().y()
        //            << endl;
        if(at(i).last().area() < areaMin) // war 1000 px // at(i).last().r1() < 15 nicht zu gross waehlen um auch nur
                                          // kopferkennung in dichten situationen zuzulassen // war: area() < 1000) ||
                                          // // groesste ellips in pyr muss mindestgroesse besitzen
        {
#ifdef PRINT_ERASE_REASON
            debout << "Eliminate pyramide because of too small (" << at(i).last().area()
                   << " pixel^2) last ellipse, with top ellipse: " << at(i).at(0).center().x() << " "
                   << at(i).at(0).center().y() << endl;
#endif
            removeAt(i);
            --i;
        }
        else if((*(at(i).getHeadEllipse())).r1() > r1Max) // war 60px // 60px in kopfhoehe sind 37cm in kopfhoehe //
                                                          // area() > 3000)) // im kopfbereich (ca. at(1)) sollte
                                                          // ellipse nicht so gross sein
        {
#ifdef PRINT_ERASE_REASON
            debout << "Eliminate pyramide because of too big head ellipse (norm. 2nd) (big radius: "
                   << (*(at(i).getHeadEllipse())).r1() << " pixel), with top ellipse: " << at(i).at(0).center().x()
                   << " " << at(i).at(0).center().y() << endl;
#endif
            removeAt(i);
            --i;
        }
        //        // fuehrte zu zu haeufigen Fehlrausschmissen:
        //         //minPyrLevel-2 ist die letzte auf jeden fall noch vorkommende ellipse
        //        // size()<minPyrLevel, damit minPyrLevel-2 letzter index
        //        else if (at(i).at(minPyrLevel-2).r2() < r2Min) //war at(i)at(2).r1() < 10 // war: 20, um schlanke
        //        gebilde wie gehobenen arm auszuschliessen
        //        {
        //                    //debout << minPyrLevel-1<<" " << r2Min<<endl;
        //#ifdef PRINT_ERASE_REASON
        //            debout << "Eliminate pyramide because of too small third ellipse, with top ellipse: " <<
        //            at(i).at(0).center().x() << " " << at(i).at(0).center().y() << endl;
        //#endif
        //            removeAt(i);
        //            --i;
        //        }
        // fuehrte zu fehlerhaften aussortieren bei virtueller scene onePerson wenn person mittig unter kamera war und
        // arme neben sich hatte
        //        else if (at(i).last().r1()-at(i).at((int) ((at(i).size()-1)*.5)).r1() < 0.01) // der radius der
        //        unterste und mittlere ellips muessen sich unterscheiden, so dass objekte die nur in der hoehe aber
        //        nicht weiter nach unten gehen zu eleminieren; gesehen bei fehldetektion des hintergrundes an waenden
        //        {
        //#ifdef PRINT_ERASE_REASON
        //            debout << "Eliminate pyramide because of equal big ellipses, with top ellipse: " <<
        //            at(i).at(0).center().x() << " " << at(i).at(0).center().y() << endl;
        //#endif
        //            removeAt(i);
        //            --i;
        //        }
    }
}

// bestimmt kopfposition in markerlosen Ueberkopfaufnahmen aus Hoehenbild
void PersonList::calcPersonPos(
    const Mat &         img,
    QRect &             roi,
    QList<TrackPoint> & persList,
    pet::StereoContext *sc,
    BackgroundFilter *  bgFilter,
    bool                markerLess)
{
    int i, j;

    if(!bgFilter->getEnabled()) // nur fuer den fall von bgSubtraction durchfuehren
    {
        debout << "Warning: Person Detection without Background subtraction may cause a lot false detectiones!" << endl;
    }

    searchEllipses(sc, roi, bgFilter);

    if(!markerLess) // schon zuvor mit markern personen detektiert
    {
        bool found;
        for(j = 0; j < persList.size(); ++j)
        {
            found = false;
            for(i = 0; (!found) && (i < size()); ++i)
            {
                if(at(i).getHeadEllipse()->isInside(persList[j].x(), persList[j].y()))
                {
                    found = true;
                }
            }
            if(!found) // wenn zuvor durch marker gefundene Person nicht in kopfellipse, dann loeschen
            {          // hier koennte auch nur die Qualitaet heruntergesetzt werden
                persList.removeAt(j);
                --j;
            }
        }
    }
    else
    {
        for(i = 0; i < size(); ++i)
        {
            Vec2F c(at(i).getHeadEllipse()->center()); // war at(i).at(1)...
            if(!(c.x() < roi.left() || c.x() > roi.right() || c.y() < roi.top() || c.y() > roi.bottom()))
            {
                TrackPoint tp(c, 100); // 100 beste qualitaet
                // hier koennte auch sofort x/y/z gesetzt werden
                persList.append(tp);
            }
        }
    }

    /*
    // kopf/koerpermodell, wobei kpfmittelpunkt in 0 liegt
    // in cm
    float model(float x, float y)
    {
        return 0.001*(x*x*x*x + y*y*y*y);
    }

    //float errorModel(float x, float y)
    //{
    //
(rx-(medianX+j))*(rx-(medianX+j))+(ry-(medianY+i))*(ry-(medianY+i))+(rz-(medianZ+model(j,i)))*(rz-(medianZ+model(j,i)));
    //}

    if (false)
    {
    int x, y, i, j, *sRow, sCol;
    int frameSize=mMain->getControlWidget()->trackNumberSize->value(); // war 24
    unsigned short* data = (unsigned short*) mDisparity.imageData+frameSize/2; //war 500; // char*
    unsigned short* yData = data+(mDisparity.width*frameSize/2); //war 300);
    unsigned short* sData;
    float val, rx, ry, rz, row, col;
    float medianX, medianY, medianZ, diff, diffMin=100000., diffMax=0.;
    int diffAnz, diffAnz2=0;
    double diffMed=0;

    sRow = (int *) malloc((frameSize+1)*sizeof(int));
    CvSize sz = cvSize(mRectRight.width, mRectRight.height);
    IplImage *tmpImg = cvCreateImage(sz, 8, 3);
    CvMat *tmpMat = cvCreateMat(mRectRight.height, mRectRight.width, CV_32FC1);
    //IplImage *tmpImg =  cvCloneImage(&mRectRight); // make a copy
    //debout << tmpImg->width << " " << tmpImg->height <<endl;
    //debout << mDisparity.width << " " << mDisparity.widthStep <<endl;

    for (y = frameSize/2; y < mDisparity.height-frameSize/2; ++y)
    {
        for (x = frameSize/2; x < mDisparity.width-frameSize/2; ++x)
        {
//        for (y = 300; y < 850; ++y)
//        {
//            for (x = 500; x < 900; ++x)
//            {


            // wenn hintergrund subtracxtion an, dann wird nur der vordergrund untersucht
            if (( mMain->getBackgroundFilter()->getEnabled() && mMain->getBackgroundFilter()->isForeground(x, y) &&
dispValueValid(*data)) ||
                (!mMain->getBackgroundFilter()->getEnabled() && dispValueValid(*data))) //(*data != mSurfaceValue+65280)
&& (*data != mBackForthValue+65280)) // 65280 == 0xff00
            {
                diff=0.;
                diffAnz=0;
//                        val = (*data-mMin)/(double)(mMax-mMin); // disp zwischen 0..1
//                        if (val>0.5)
//                        {
//                            (tmpImg->imageData)[3*x+y*tmpImg->widthStep] = 255;
//                            (tmpImg->imageData)[3*x+y*tmpImg->widthStep+1] = 255;
//                            (tmpImg->imageData)[3*x+y*tmpImg->widthStep+2] = 255;
//                        }
                getMedianXYZaround(x, y, &medianX, &medianY, &medianZ); // median in 5x5 umfeld um aktuellen pkt
                // ohne beruecksichtigung der perspektive
//                    for (i = -frameSize/2; i <= frameSize/2; ++i) // y
//                        for (j = -frameSize/2; j <= frameSize/2; ++j) // x
//                        {
//                            sData = data+j+i*mDisparity.width;
//                            if (dispValueValid(*sData)) // Kopf soll -0.01*(x*x+y*y) annehmen // so gewaehlt, dass
pixel ca cm
//                            {
//                                triclopsRCD16ToXYZ(mTriclopsContext, y+i, x+j, *sData, &rx, &ry, &rz);
//                                rx *= 100.; ry *= 100.; rz *= 100.; // in cm
//
diff+=(rx-(medianX+j))*(rx-(medianX+j))+(ry-(medianY+i))*(ry-(medianY+i))+(rz-(medianZ+0.001*(j*j*j*j+i*i*i*i)))*(rz-(medianZ+0.001*(j*j*j*j+i*i*i*i)));
//                                ++diffAnz;
//                            }
//                        }
                // pixel, an dem modell liegt wird bestimmt und dort die differenz zur disparity aufsummiert
                // ACHTUNG wichtig, sonst fehelr: der nicht sichtbare kopfbereich wuerde auch auf pixel gemappt an dem
per disp die andere ansicht gegen nicht sichtbaren kopfbeeich verglichen wird - falsch!


                //i, j in cm
                for (i=0;i<=frameSize; ++i)
                    sRow[i] = -1;
                for (i = -frameSize/2; i <= frameSize/2; ++i) // y
                {
                    sCol=-1;
                    for (j = -frameSize/2; j <= frameSize/2; ++j) // x
                    { // ich interpretier row und col genau anders herum: col = x row = y
                        triclopsXYZToRCD(mTriclopsContext, medianX+j, medianY+i, medianZ+model(j,i), &row, &col, &val);
// auf welchem pixel liegt modellform
                        // wenn resultierende col nicht mehr steigt, dann for-schleifen-abbruch
                        // erst beginnen mit dem zaehlen, wenn col mehr wird, auch wenn dann erstenes datum verworfen
wird if ((sRow[j+frameSize/2] < 0) || (myRound(row)<=sRow[j+frameSize/2]) || (sCol < 0) || (myRound(col)<=sCol))
                        {
                            sRow[j+frameSize/2] = myRound(row);
                            sCol = myRound(col);
                            continue;
                        }

                        sCol = myRound(col);
                        sRow[j+frameSize/2] = myRound(row);
                        if (0<=sCol && sCol<mDisparity.width && 0<=sRow[j+frameSize/2] &&
sRow[j+frameSize/2]<mDisparity.height)
                        {
                        sData = ((unsigned short*) mDisparity.imageData) + sCol + sRow[j+frameSize/2]*mDisparity.width;
                        if (dispValueValid(*sData)) // hier kam es zum Absturz bevor vorvorheriges if eingefuegt wurde
                        {
                            triclopsRCD16ToXYZ(mTriclopsContext, sRow[j+frameSize/2], sCol, *sData, &rx, &ry, &rz);
                            rx *= 100.; ry *= 100.; rz *= 100.; // in cm
                            if
(i*i+j*j<mMain->getControlWidget()->trackColorMarkerSize->value()*mMain->getControlWidget()->trackColorMarkerSize->value())
// war 120 kopf
                            {
                                diff +=
(rx-(medianX+j))*(rx-(medianX+j))+(ry-(medianY+i))*(ry-(medianY+i))+(rz-(medianZ+model(j,i)))*(rz-(medianZ+model(j,i)));
// pow(a,2)
                                ++diffAnz;
                            }
                            else //fehler um kopfumfang 21cm alles was uber medianZ-30cm liegt als fehler gewichtet
(hoeher als Schulter)
                            {
                                val = rz-(medianZ+30.); // Abstand zur Schulter
                                val = (val>0?0:val*val); // oberhalb Schulter wird quadratisch negativ bewertet
                                if ((i<6 && y>=mDisparity.height/2) ||
                                    (i>6 && y<mDisparity.height/2) ||
                                    (j<6 && x>=mDisparity.width/2) ||
                                    (j>6 && x<mDisparity.width/2))
                                {
                                    diff += val;
                                    ++diffAnz;
                                }
                            }
                        }
                        }
                        //if ((x==193 && y==237) || (x==1045 && y==264))
                        //    debout << (int)(i*i+j*j<120) << " " << x << " " << y << " " << j << " " << i << " " <<
sCol << " " << sRow[j+frameSize/2] << " " << diff << " " << endl;
                        // wenn resultierende row nicht mehr steigt, dann for-schleifen-abbruch
                    }
                }


                if (diffAnz < frameSize*frameSize/16)
                    diff = -2.;
                else
                {
                    diff/=diffAnz;
                    ++diffAnz2;
                    diffMed+=diff;
                    if (diff<diffMin)
                        diffMin=diff;
                    if (diff>diffMax)
                        diffMax=diff;
                }
            }
            else
                diff = -1.;

            *(((float *)(tmpMat->data.ptr+y*tmpMat->step))+x) = diff;// step zaehlt in char!!
            ++data;
        }
        data = (yData += mDisparity.width); // falsch, da nicht mehr char sondern short: (yData +=
mDisparity.widthStep); // because sometimes widthStep != width
    }
    diffMed=diffMed/diffAnz2;
    debout << diffMin<< " " << diffMax<<" " << diffMed << endl;
    free(sRow);

    float zoom = 1000.*(diffMax-diffMed)/(diffMax-diffMin);
    for (y = frameSize/2; y < mDisparity.height-frameSize/2; ++y)
    {
        for (x = frameSize/2; x < mDisparity.width-frameSize/2; ++x)
        {
            val = *(((float *)(tmpMat->data.ptr+y*tmpMat->step))+x);
            if (val<0)
            {
                (tmpImg->imageData)[3*x+y*tmpImg->widthStep] = 255; // blau
                (tmpImg->imageData)[3*x+y*tmpImg->widthStep+1] = 0;
                (tmpImg->imageData)[3*x+y*tmpImg->widthStep+2] = 0;
            }
            else
            {
                val = (val-diffMin)/(diffMax-diffMin);
                //val = sqrt(val);
                val = val>(1./zoom)?1:val*zoom;
                val = 255-255*val;
                (tmpImg->imageData)[3*x+y*tmpImg->widthStep] = val;
                (tmpImg->imageData)[3*x+y*tmpImg->widthStep+1] = val;
                (tmpImg->imageData)[3*x+y*tmpImg->widthStep+2] = val;
            }
        }
    }

//            // weichzeichnen
//            IplImage *blurImg =  cvCloneImage(&mDisparity); // make a copy
//            //cvSmooth(blurImg, blurImg, CV_BLUR, 20);
//            // selber blur
//            data = (unsigned short*) mDisparity.imageData; // char*
//            yData = data;
//            for (y = 0; y < mDisparity.height; ++y)
//            {
//                for (x = 0; x < mDisparity.width; ++x)
//                {
//                    if (dispValueValid(*data))
//                    {
//                        if (x>j && x<mDisparity.width-10 && y>i && y<mDisparity.height-10)
//                        {
//                            val = 0;
//                            diffAnz = 0;
//                            for (i = -10; i <= 10; ++i) // y
//                                for (j = -10; j <= 10; ++j) // x
//                                {
//                                    if (dispValueValid(*(data+j+i*mDisparity.width)))
//                                    {
//                                        ++diffAnz;
//                                        val+=*(data+j+i*mDisparity.width);
//                                    }
//                                }
//                            val/=diffAnz; // miondestens 1 muss gefunden worden sein
//                            *(((unsigned short*)blurImg->imageData)+y*blurImg->width+x) = val;
//                        }
//                    }
//                    ++data;
//                }
//                data = (yData += mDisparity.width); // falsch, da nicht mehr char sondern short: (yData +=
mDisparity.widthStep); // because sometimes widthStep != width
//            }

//            // lokale maxima rot markieren
//            data = (unsigned short*) blurImg->imageData+frameSize/2; // char*
//            yData = data+(blurImg->width*frameSize/2);
//            for (y = frameSize/2; y < blurImg->height-frameSize/2; ++y)
//            {
//                for (x = frameSize/2; x < blurImg->width-frameSize/2; ++x)
//                {
//                    if (dispValueValid(*data)) //(*data != mSurfaceValue+65280) && (*data != mBackForthValue+65280))
// 65280 == 0xff00
//                    {
//                        triclopsRCD16ToXYZ(mTriclopsContext, y, x, *data, &rx, &ry, &rz);
//                        diff=1.;
//                        diffAnz=0;
//                        for (i = -2; i <= 2; ++i) // y
//                        {
//                            for (j = -2; j <= 2; ++j) // x
//                            {
//                                sData = data+j+i*blurImg->width;
//                                if (dispValueValid(*sData))
//                                {
//                                    ++diffAnz;
//                                    triclopsRCD16ToXYZ(mTriclopsContext, y+i, x+j, *sData, &rx, &ry, &val);
//                                    if (val<rz)
//                                        diff=-1.;
//                                }
//                            }
//                        }
//                        if (diff>0 && diffAnz>15)
//                        {
//                            (tmpImg->imageData)[3*x+y*tmpImg->widthStep] = 0; // rot
//                            (tmpImg->imageData)[3*x+y*tmpImg->widthStep+1] = 0;
//                            (tmpImg->imageData)[3*x+y*tmpImg->widthStep+2] = 255;
//                        }
//                    }
//                    ++data;
//                }
//                data = (yData += blurImg->width); // falsch, da nicht mehr char sondern short: (yData +=
mDisparity.widthStep); // because sometimes widthStep != width
//            }

    //cvNamedWindow("mDisparity", CV_WINDOW_AUTOSIZE ); // 0 wenn skalierbar sein soll
    //cvShowImage("mDisparity", &mDisparity);
    //cvNamedWindow("blurImg", CV_WINDOW_AUTOSIZE ); // 0 wenn skalierbar sein soll
    //cvShowImage("blurImg", blurImg);
    cvNamedWindow("tmpImg", CV_WINDOW_AUTOSIZE ); // 0 wenn skalierbar sein soll
    cvShowImage("tmpImg", tmpImg);
    //cvWaitKey( 0 ); // zahl statt null, wenn nach bestimmter zeit weitergegangen werden soll

    }
*/
}
#endif // STEREO_DISABLED
