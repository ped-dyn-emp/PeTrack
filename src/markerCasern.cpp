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

//folgende zeile spaeter raus
#include <opencv2/highgui.hpp>

// fuer Aussentreppen, sehr gross, oben
//#define HEAD_SIZE_MIN 30
//#define HEAD_SIZE_MAX 45
//#define SPOT_SIZE_MIN 5
//#define SPOT_SIZE_MAX 12

// fuer Arena Kessel, Treppen:
#define HEAD_SIZE_MIN 13
#define HEAD_SIZE_MAX 37
#define SPOT_SIZE_MIN 1
#define SPOT_SIZE_MAX 9

#include "markerCasern.h"
#include "helper.h"
#include "tracker.h"

using namespace::cv;
using namespace std;

// bei marker koennnte sich fuer jeden spot eine liste gemerkt werden und spaeter ausgleich 
// regressionsgrade legen um optimale linie durch marker zu erhalten
MarkerCasern::MarkerCasern(MyEllipse head)
    : mHead(head),
      mHasHead(true),
      mHasQuadrangle(false),
      mCenterIndex(-1),
      mColorIndex(-1)
//       quadrangle[0](0,0),
//       quadrangle[1](0,0),
//       quadrangle[2](0,0),
//       quadrangle[3](0,0)
{
}
MarkerCasern::MarkerCasern()
    : mHasHead(false),
      mCenterIndex(-1),
      mColorIndex(-1)

{
}
// muesste alles automatisch durch den defaultdestructor gemacht werden!
MarkerCasern::~MarkerCasern()
{
    mSpots.clear();
    mSpotCount.clear();
}
  
// 3 inline functions to get access to class member varaibales only in .h file

bool MarkerCasern::isOverlappingHead(const MyEllipse& e) const
{
    if (hasHead())
        return mHead.isInside(e.center()) || e.isInside(mHead.center());
    else
        return false;
}
bool MarkerCasern::isInsideHead(const Vec2F& p) const
{
    if (hasHead())
        return mHead.isInside(p);
    else
        return false;
}

// returns spot number in spots list when inside, otherwise returns -1
int MarkerCasern::isOverlappingSpots(const MyEllipse& e) const
{
    for (int i = 0; i < mSpots.size(); ++i)
        if ((mSpots[i].isInside(e.center()) || e.isInside(mSpots[i].center())) && // Mittelpkte liegen ineinander
            ((e.center()-mSpots[i].center()).length() < 2)) // kein zu grosser Abstand
            return i;
    return -1;
}
int MarkerCasern::isInsideSpots(const Vec2F& p) const
{
    for (int i = 0; i < mSpots.size(); ++i)
        if (mSpots[i].isInside(p))
            return i;
    return -1;
}

void MarkerCasern::modifyHead(const MyEllipse &head)
{
    // man koennte auch spots und heads sammeln und am ende eine ueberpruefung vornehmen!!!!!! 
    // median , regressionsgrade
    // man koennte auch pruefen:
    // - ob ellips-mittelpkt besser auf linie von markern liegt
    // - ob die outline eine bestimmte groesse besitzt
    if (hasHead())
    {
        if (mHead.outline()>260 && mHead.outline()<340) // bevorzugte groesse - nur initial koennte andere erzeugt werden
            if (fabs(mHead.ratio()-1.5)>fabs(head.ratio()-1.5)) // because white plate is 14x21cm
                mHead = head;
    }
    else
    {
        mHasHead = true;
        mHead = head;
    }
}

void MarkerCasern::modifySpot(int i, const MyEllipse &spot)
{
    if (i < mSpots.size())
    {
        // man koennte auch pruefen:
        // - ob abstand zur mittellinie geringer
        // - ob form eher kreisfoermig
        if (spot.outline() > mSpots[i].outline())
            mSpots[i] = spot;
        ++mSpotCount[i];
    }
}
void MarkerCasern::addSpot(const MyEllipse &spot)
{
    mSpots.append(spot);
    mSpotCount.append(1);
}
void MarkerCasern::deleteSpot(int i)
{
    mSpots.removeAt(i);
    mSpotCount.removeAt(i);
}

void MarkerCasern::modifyQuadrangle(const Vec2F v[4])
{
    if (mHasQuadrangle)
    {
        // seitenverhaeltnis 2:3 koennte auch bedacht werden
        // angle on opposed edge is nearer to 90� 
        if ((fabs((v[3]-v[2]).angleBetweenVec(v[0]-v[3])-PI/2.)+
             fabs((v[1]-v[0]).angleBetweenVec(v[2]-v[1])-PI/2.)) <
            (fabs((mQuadrangle[3]-mQuadrangle[2]).angleBetweenVec(mQuadrangle[0]-mQuadrangle[3])-PI/2.)+
             fabs((mQuadrangle[1]-mQuadrangle[0]).angleBetweenVec(mQuadrangle[2]-mQuadrangle[1])-PI/2.)))
        {
            for (int i = 0; i < 4; ++i)
                mQuadrangle[i] = v[i];
        }
    }
    else
    {
        mHasQuadrangle = true;
        for (int i = 0; i < 4; ++i)
            mQuadrangle[i] = v[i];
    }
}


// nach organize gibt es entweder 0 oder 3 spots pro kopf
void MarkerCasern::organize(const Mat &img, bool autoWB)
{
    int i, j, k;

    // direkt herausspringen, da nichts zu tun ist
    if (mSpots.size() == 0)
        return;

//     for (i = 0; i < mSpots.size(); ++i)
//             mSpots[i].draw(img, 0, 255, 0);

    // durch mgl wachsen der spots muss ueberprueft werden, ob sich im nachhinnein Ueberlagerungen ergeben
    for (i = 0; i < mSpots.size(); ++i)
        for (j = i+1; j < mSpots.size(); ++j)
            if (mSpots[i].isInside(mSpots[j].center()) || mSpots[j].isInside(mSpots[i].center()))
            {
                modifySpot(i, mSpots[j]);
                deleteSpot(j);
                j=i; // es muss von vorne begonnen werden, da i wachsen koennte und dann vorgaenger von j nun hineinpassen koennten // --j;
            }
        
    // nach groesse sortieren und loeschen koennte kreus angreifen zugunsten von zahl

    // gucken, ob in ellipse wirklich zB bullet drin ist

    // spots loeschen, die selten erkannt wurden
    // mind 5 stehen lassen (marker und zahlen - auch von der seite zu sehen)
    int count = 100, anz = 0;
    while (count > 4)
    {
        ++anz;
        count = 0;
        for (i = 0; i < mSpotCount.size(); ++i)
            if (mSpotCount[i] > anz)
                count++;
    }
    for (i = 0; i < mSpotCount.size(); ++i)
        if (mSpotCount[i] < anz)
        {
            deleteSpot(i);
            --i;
        }


    // 4 spots heraussuchen, bei denen um center in quadrat. bereich in ellipse der dunkelste Pkt (kleiner value in hsv) gefunden wird
    // 4 uebrig lassen, weil entweder nummer an kopfseite sehr dunkel sein kann, wenn zu sehen oder in sehr hellen bereichen kreuz wegfallen wuerde
    // da aber immer die 3 marker von mittellinie drin bleiben kann bei suche solch einer linie nur diese entstehen,
    // da beide nummern nicht mehr drin sind
    QColor col;
    int cx, cy;
    if (mSpots.size() > 4)
    {
        int r;
        QList<int> minValList;
        for (i = 0; i < mSpots.size(); ++i)
        {
            cx = myRound(mSpots[i].x());
            cy = myRound(mSpots[i].y());
            r = myRound((mSpots[i].r1()+mSpots[i].r2())/2.); // mittlerer radius
// debout << cx+1 << " " << cy+1 << ": ";
            int minVal = 256; // groesser als komplett weiss (255)
            // bildrand muss nicht ueberprueft werden, da sie gar nicht erst hereingekommen waeren
            for (j = -r; j < r+1; ++j)
                for (k = -r; k < r+1; ++k)
                {
                    col.setRgb(getValue(img,cx+j,cy+k).rgb());//getR(img, cx+j, cy+k), getG(img, cx+j, cy+k), getB(img, cx+j, cy+k));
                    if (col.value() < minVal)
                        minVal = col.value(); // Helligkeit
                }
            minValList.append(minVal);
// cout << minVal << endl;
        }
        // loeschen aller zu hellen spots
        QList<int> minValListSort = minValList;
        std::sort(minValListSort.begin(), minValListSort.end()); // sortiert aufsteigend
        for (i = 0; i < mSpots.size(); ++i)
            if (minValList[i] > minValListSort[3])
            {
// debout << i << " groesser als " << minValListSort[3]<< endl;
                deleteSpot(i);
                minValList.removeAt(i);
                --i;
            }
    }

// for (i = 0; i < mSpots.size(); ++i)
// mSpots[i].draw(img, 255, 0, 0);

    // sort spot list: wird nicht gemacht!!!!!!!!
    // 1. bullet marker spot
    // 2. cross marker spot
    // 3. color marker spot
    // 4. number marker spot

    // die am nahesten auf Linie liegenden spots heraussuchen
    // cross marker spot index setzen
    // mCenterIndex = -1; wird im constructor gemacht
    int centerIndex = -1, lineIndex[3] = {-1, -1, -1};
    double difAbs = 1000, tmp2Abs = 0, tmpAbs = 0, minAbs = 1000; //tmpAbs = 0, tmp2Abs = 0 damit keine warnung
    double o1, o2, o3, minOutline = 12;
    bool found = false;
    for (i = 0; i < mSpots.size(); ++i)
        for (j = i+1; j < mSpots.size(); ++j)
            for (k = j+1; k < mSpots.size(); ++k)
            {
                o1 = mSpots[i].outline();
                o2 = mSpots[j].outline();
                o3 = mSpots[k].outline();
                if (((o1 > minOutline && o2 > minOutline) || (o1 > minOutline && o3 > minOutline) || (o2 > minOutline && o3 > minOutline)) && // mind 2 groesser
                    ((tmpAbs = mSpots[i].center().distanceToLine(mSpots[j].center(), mSpots[k].center())) < minAbs) &&
                    (tmpAbs < 2)) // abstand und damit fehler sollte nicht groesser als 2 sein, sonst maker loeschen
                {
                    // mittigen spot heraussuchen
                    if (((tmp2Abs=(0.5*(mSpots[i].center()+mSpots[j].center())-mSpots[k].center()).length()) < 2) &&
                        (tmp2Abs < difAbs) && 
                        ((mSpots[i].center()-mSpots[j].center()).length() > 13) && // damit nicht farbmarker alleine alle 3 marker
                        (o1 > minOutline && o2 > minOutline))
                    {
                        difAbs = tmp2Abs;
                        centerIndex = 2;
                        found = true;
                    }
                    if (((tmp2Abs=(0.5*(mSpots[j].center()+mSpots[k].center())-mSpots[i].center()).length()) < 2) &&
                        (tmp2Abs < difAbs) &&
                        ((mSpots[j].center()-mSpots[k].center()).length() > 13) && // damit nicht farbmarker alleine alle 3 marker
                        (o2 > minOutline && o3 > minOutline))
                    {
                        difAbs = tmp2Abs;
                        centerIndex = 0;
                        found = true;
                    }
                    if (((tmp2Abs=(0.5*(mSpots[k].center()+mSpots[i].center())-mSpots[j].center()).length()) < 2) &&
                        (tmp2Abs < difAbs) &&
                        ((mSpots[i].center()-mSpots[k].center()).length() > 13) && // damit nicht farbmarker alleine alle 3 marker
                        (o1 > minOutline && o3 > minOutline))
                    {
                        difAbs = tmp2Abs;
                        centerIndex = 1;
                        found = true;
                    }

                    // wenn mittiger spot nicht mittig, dann keine gueltige marke
                    if (found) // ueberhaupt eine Mitte gefunden
                    {
                        found = false;
                        lineIndex[0] = i;
                        lineIndex[1] = j;
                        lineIndex[2] = k;
                        minAbs = tmpAbs;
                    }
                }
            }
    // delete spots not laing on the line
    for (i = 0; i < mSpots.size(); ++i)
    {
        if (i != lineIndex[0] && i != lineIndex[1] && i != lineIndex[2])
        {
            deleteSpot(i);
            --i;
            --lineIndex[0]; --lineIndex[1]; --lineIndex[2];
        } // set index for cross marker
        else if ((centerIndex > -1) && (i == lineIndex[centerIndex]))
        {
            mCenterIndex = lineIndex[centerIndex];
        }
    }

    // ab hier nur noch 0 oder 3 spots!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // bei 0 muss keine farbe mehr bestimmt werden
    if (mSpots.size() == 0)
        return;

    // farbe bestimmen (richtiger marker koennte auch durch form oder dem abstand des hellsten pkt in groesserer umgebung vom mittelpkt bestimmt werden)
    bool colSet = false;
    QColor colOther; // farbe des anderen spots
    for (i = 0; i < mSpots.size(); ++i)
        if (i != mCenterIndex)
        {
            cx = myRound(mSpots[i].x());
            cy = myRound(mSpots[i].y());
            if (!colSet)
            {
                colSet = true;
                mCol.setRgb(getValue(img,cx,cy).rgb());//getR(img, cx, cy), getG(img, cx, cy), getB(img, cx, cy));
                mColorIndex = i;
            }
            else
            {
                colOther.setRgb(getValue(img,cx,cy).rgb());//getR(img, cx, cy), getG(img, cx, cy), getB(img, cx, cy));
                mOtherIndex = i;
            }
            // bildrand muss nicht ueberprueft werden, da sie gar nicht erst hereingekommen waeren
            for (j = -1; j < 2; ++j) // 3x3 feld wird ueberprueft
                for (k = -1; k < 2; ++k)
                {
//j=0, k=0;
                    col.setRgb(getValue(img,cx+j,cy+k).rgb());//getR(img, cx+j, cy+k), getG(img, cx+j, cy+k), getB(img, cx+j, cy+k));
//debout << mCol << ", " << col << ", " << colOther <<endl;
                    // faktor, um die mitte hoeher zu gewichten, da bei fd der schwarze rand mit dem weiss gemischt oft hell ist
                    if (col.value()/(1. + .5*(abs(j)+abs(k))) > mCol.value()) // wenn Helligkeit maximal
                    {
                        if (i != mColorIndex)
                        {
                            colOther = mCol;
                            mOtherIndex = mColorIndex;
                        }
                        mCol = col;
                        mColorIndex = i;
                    }
                }
        }

    if (autoWB)
    {
        // weissabgleich
        // 1. Dynamik der Helligkeit
        // 2. farbverschiebung entsprechend weisswertes
//         int ocx = myRound(mSpots[mOtherIndex].x());
//         int ocy = myRound(mSpots[mOtherIndex].y());
        // bildrand muss nicht ueberprueft werden, da sie gar nicht erst hereingekommen waeren
//         // schwarzwert bestimmen
//         QColor black(255,255,255); // dunkelster fleck
//         for (j = -1; j < 2; ++j) // 3x3 feld um ocx,ocy wird ueberprueft
//             for (k = -1; k < 2; ++k)
//             {
//                 col.setRgb(getR(img, ocx+j, ocy+k), getG(img, ocx+j, ocy+k), getB(img, ocx+j, ocy+k));
//                 if (col.value() < black.value()) // wenn Helligkeit geringer
//                     black = col;
//             }
        // weisswert und durchschnittlichen weisswert bestimmen
        // 2 bereiche neben farbmarker in weisser flaeche auf kopfpappe
        int wcx = myRound(mSpots[mColorIndex].x()+0.356*(mSpots[mColorIndex].y()-mSpots[mOtherIndex].y()));
        int wcy = myRound(mSpots[mColorIndex].y()+0.356*(mSpots[mColorIndex].x()-mSpots[mOtherIndex].x()));
        QColor white(0,0,0); // hellster fleck
        //QColor avgWhite; // durchschnittlichen weisswert
        int avgWhiteR=0, avgWhiteG=0, avgWhiteB=0; // durchschnittlichen weisswert
        for (j = -1; j < 2; ++j) // 3x3 feld um ocx,ocy wird ueberprueft
            for (k = -1; k < 2; ++k)
            {
                QColor color = getValue(img,wcx+j,wcy+k);
                avgWhiteR+=color.red();//getR(img, wcx+j, wcy+k);
                avgWhiteG+=color.green();//getG(img, wcx+j, wcy+k);
                avgWhiteB+=color.blue();//getB(img, wcx+j, wcy+k);
                col.setRgb(getValue(img,wcx+j,wcy+k).rgb());//getR(img, wcx+j, wcy+k), getG(img, wcx+j, wcy+k), getB(img, wcx+j, wcy+k));
                if (col.value() > white.value()) // wenn Helligkeit groesser
                    white = col;
            }
        wcx = myRound(mSpots[mColorIndex].x()-0.356*(mSpots[mColorIndex].y()-mSpots[mOtherIndex].y()));
        wcy = myRound(mSpots[mColorIndex].y()-0.356*(mSpots[mColorIndex].x()-mSpots[mOtherIndex].x()));
        for (j = -1; j < 2; ++j) // 3x3 feld um ocx,ocy wird ueberprueft
            for (k = -1; k < 2; ++k)
            {
                QColor color = getValue(img,wcx+j,wcy+k);
                avgWhiteR+=color.red();//getR(img, wcx+j, wcy+k);
                avgWhiteG+=color.green();//getG(img, wcx+j, wcy+k);
                avgWhiteB+=color.blue();//getB(img, wcx+j, wcy+k);
                col.setRgb(getValue(img,wcx+j,wcy+k).rgb());//getR(img, wcx+j, wcy+k), getG(img, wcx+j, wcy+k), getB(img, wcx+j, wcy+k));
                if (col.value() > white.value()) // wenn Helligkeit groesser
                    white = col;
            }
        //avgWhite.setRgb();
        avgWhiteR/=18; avgWhiteG/=18; avgWhiteB/=18;
        
        //     debout << black <<endl;    
        //     debout << white <<endl;    
        //     debout << avgWhiteR << " " << avgWhiteG << " " << avgWhiteB <<endl;    
        //     debout << mCol <<endl;    
        //debout << avgWhite <<endl;    
        
        // Anpassung
        // Value / Helligkeit anpassen
        // schwarzton wird nicht beruecksichtigt, weil dadurch alles dunkler wurde,
        // da verwischte aufnahmen den schwarzen pkt nicht sehr schwarz wiedergeben
        // DOCH, BEI RGB und HSV kam es zu ueberschreitungen, was mit warnungen einherging:
        //                  eine unwahrscheinliche uebersteuerung (zahlen > 255) wurde nicht abgefangen
        // mCol.setHsv(mCol.hue(), mCol.saturation(), (int)((mCol.value()-black.value())*(255./(white.value()-black.value()))));    //colOther.setHsv(colOther.hue(), colOther.saturation(), (int)((colOther.value()-black.value())*(255./(white.value()-black.value()))));
   
        //passiert nun im einen schritt      
//         // helligkeit hochsetzen, so dass weiss maximal hell
//         mCol.setHsv(mCol.hue(), mCol.saturation(), MIN(MAX((int)(mCol.value()*(255./white.value())), 0),255));
//         colOther.setHsv(colOther.hue(), colOther.saturation(), MIN(MAX((int)(colOther.value()*(255./white.value())), 0),255));
        
        //passiert nun im einen schritt und mit einem faktor statt einer summand
//         // Farbton anpassen
//         // uebersteuerung wird einfach gedeckelt und nicht angepasst!!!
//         mCol.setRgb(MIN(MAX(mCol.red()+(avgWhiteB-avgWhiteR), 0),255), MIN(MAX(mCol.green()+(avgWhiteB-avgWhiteG), 0),255), mCol.blue());
//         colOther.setRgb(MIN(MAX(colOther.red()+(avgWhiteB-avgWhiteR), 0),255), MIN(MAX(colOther.green()+(avgWhiteB-avgWhiteG), 0),255), colOther.blue());
        if ((avgWhiteR !=0) && (avgWhiteG !=0) && (avgWhiteB !=0))
        {
            mCol.setRgb((int) MIN(((255./avgWhiteR)*mCol.red()), 255.), (int) MIN(((255./avgWhiteG)*mCol.green()), 255.), (int) MIN(((255./avgWhiteB)*mCol.blue()), 255.));
            colOther.setRgb((int) MIN(((255./avgWhiteR)*colOther.red()), 255.), (int) MIN(((255./avgWhiteG)*colOther.green()), 255.), (int) MIN(((255./avgWhiteB)*colOther.blue()), 255.));
        }
        else
            debout << "weiss ist zu dunkel!!!!!!!!!!!!!!"<<endl;

        // eigentlich muesset saettigung ueber histogramm angepasst werden!!
    }
    
    // darauffolgende methode ist nach versuchen besser und sowieso schneller
    //     // wenn auf markern farbe nahezu identisch
    //     // weisst dies auf einen schwarzen marker hin,
    //     // dann wird der marker genommen, der die groesste Flaeche beansprucht
    //     if (abs(mCol.value()-colOther.value()) < 30) //&& abs(mCol.saturation()-colOther.saturation()) < 40
    //     {
    //         int count[3] = {0, 0, 0};
    //         int maxVal = (mCol.value()+colOther.value())/2 + 10;
    //         int maxCount = 0;
    //         int maxIdx = -1;
    //         for (i = 0; i < mSpots.size(); ++i)
    //             if (i != mCenterIndex)
    //             {
    //                 cx = myRound(mSpots[i].x());
    //                 cy = myRound(mSpots[i].y());
    //                 // bildrand muss nicht ueberprueft werden, da sie gar nicht erst hereingekommen waeren
    //                 for (j = -7; j < 8; ++j) // 15x15 feld wird ueberprueft
    //                     for (k = -7; k < 8; ++k)
    //                     {
    //                         col.setRgb(getR(img, cx+j, cy+k), getG(img, cx+j, cy+k), getB(img, cx+j, cy+k));
    //                         if (col.value() < maxVal)
    //                             count[i]++;
    //                     }
    //                 if (count[i] > maxCount)
    //                 {
    //                     maxCount = count[i];
    //                     maxIdx = i;
    //                 }
    //             }
    //         mColorIndex = maxIdx;
    //         if (maxIdx != -1)
    //             debout << "Warning: Blackpoint at " << mSpots[maxIdx].x() << ", " << mSpots[maxIdx].y() << "!" << endl;
    //     }
    
    // wenn auf markern farbe nahezu identisch
    // weisst dies auf einen schwarzen marker hin,
    // dann wird der marker genommen, der den groessten radius hat
    if (abs(mCol.value()-colOther.value()) < 30) //&& abs(mCol.saturation()-colOther.saturation()) < 40
    {
//     if (mCenterIndex != -1)
//         debout << myRound(mSpots[mCenterIndex].x())+1 << " " <<myRound(mSpots[mCenterIndex].y())+1<<endl;
// //     debout << mColorIndex <<" " << mCenterIndex << " " << mSpots.size()<<endl;
// //     debout << mCol.red() <<" "<< mCol.green() <<" "<< mCol.blue() <<endl;
// //     debout << colOther.red() <<" "<< colOther.green() <<" "<< colOther.blue() <<endl;
//     debout << mCol.hue() <<" "<< mCol.saturation() <<" "<< mCol.value() <<endl;
//     debout << colOther.hue() <<" "<< colOther.saturation() <<" "<< colOther.value() <<endl;
        double maxRadius = 0, rad;
        int maxIdx = -1;
        for (i = 0; i < mSpots.size(); ++i)
            if (i != mCenterIndex)
            {
                rad = (mSpots[i].r1()+mSpots[i].r2())/2.;
                if (rad > maxRadius)
                {
                    maxRadius = rad;
                    maxIdx = i;
                }
            }
        if (mColorIndex != maxIdx)
        {
            mOtherIndex = mColorIndex;
            mColorIndex = maxIdx;
            // wird im verlaufe nicht mehr benoetigt: colOther = mCol;
            mCol = colOther;
        }
//         if (mColorIndex != maxIdx)
//             debout << "Warning: Method radius has other result than method blackpoint at point " << mSpots[maxIdx].x() << ", " << mSpots[maxIdx].y() << " and " << mSpots[mColorIndex].x() << ", " << mSpots[mColorIndex].y() << "!" << endl;
    }

//     // koennten so bei schlechter beleuchtung graue bereiche auf muetze eigentliche marker ueberlagern?
//     // delete all without 4 with most counts and sort them
//     int max[4] = {-1, -1, -1, -1}; // max[0] is biggest
//     int maxIndex[4] = {-1, -1, -1, -1};
//     for (i = 0; i < mSpots.size(); ++i)
//     {
//         for (j = 0; j < 4; ++j)
//         {
//             if (mSpotCount[i] > max[j])
//             {
//                 for (k = j; k < 3; ++k)
//                 {
//                     max[k+1] = max[k];
//                     maxIndex[k+1] = maxIndex[k];
//                 }
//                 max[j] = mSpotCount[i];
//                 maxIndex[j] = i;
//                 break;
//             }
//         }
//         // ==
//         //             if (mSpotCount[i] > max[0])
//         //             {
//         //                 max[3] = max[2]; max[2] = max[1]; max[1] = max[0];
//         //                 maxIndex[3] = maxIndex[2]; maxIndex[2] = maxIndex[1]; maxIndex[1] = maxIndex[0];
//         //                 max[0] = mSpotCount[i];
//         //                 maxIndex[0] = i;
//         //             } else if (mSpotCount[i] > max[1])
//         //             {
//         //                 max[3] = max[2]; max[2] = max[1];
//         //                 maxIndex[3] = maxIndex[2]; maxIndex[2] = maxIndex[1];
//         //                 max[1] = mSpotCount[i];
//         //                 maxIndex[1] = i;
//         //             } else if (mSpotCount[i] > max[2])
//         //             {
//         //                 max[3] = max[2];
//         //                 maxIndex[3] = maxIndex[2];
//         //                 max[2] = mSpotCount[i];
//         //                 maxIndex[2] = i;
//         //             } else if (mSpotCount[i] > max[3])
//         //             {
//         //                 max[3] = mSpotCount[i];
//         //                 maxIndex[3] = i;
//         //             }
//     }
//     for (j = 0, i = 0; i < 4 && maxIndex[i] > -1; ++i)
//     {
//         mSpots.append(mSpots[maxIndex[i]]);
//         mSpotCount.append(max[i]);
//         j++; // number of max
//     }
//     k = mSpots.size();
//     for (i = 0; i < k-j; ++i)
//     {
//         mSpots.removeFirst();
//         mSpotCount.removeFirst();
//     }
//     //         debout << max[0] << " " << max[1] << " " << max[2] << " " << max[3] << " " <<endl;
}

MyEllipse MarkerCasern::getCenterSpot() const
{
    if (mCenterIndex == -1)
        return mHead; // good fallback ?
    else
        return mSpots[mCenterIndex];
}

MyEllipse MarkerCasern::getColorSpot() const
{
    if (mColorIndex == -1)
        return mHead; // good fallback ?
    else
        return mSpots[mColorIndex];
}

void MarkerCasern::draw(cv::Mat &img) const
{
    int i;

    //head
    if (hasHead())
        head().draw(img, 255, 0, 255);
    //marker
    for (i = 0; i < mSpots.size(); ++i)
    {
        if (i == mCenterIndex)
            mSpots[i].draw(img, 255, 0, 0);
        else
            mSpots[i].draw(img, 0, 0, 255);
    }
    if (hasHead() && mSpots.size() == 0)
    {
        MyEllipse e(head().center().x(), head().center().y(), 4, 4, 0);
        e.draw(img, 255, 0, 0);
    }

    // quadrangle
    if (hasQuadrangle())
    {
//        CvPoint pt[4], *rect = pt;
        vector<Point> pt;
//        int count = 4;
        for (i = 0; i < 4; ++i)
            pt.push_back(Point(mQuadrangle[i].x(),mQuadrangle[i].y()));
//            pt[i] = mQuadrangle[i].toCvPoint();
        // draw the square as a closed polyline 
        cv::polylines(img,pt,true,CV_RGB(0,255,0),1,LINE_AA,0);
//        cvPolyLine(img, &rect, &count, 1, 1, CV_RGB(0,255,0), 1, CV_AA, 0); //3, CV_AA, 0
    }
}

//------------------------------------------------------------------------------------------

// img is 1 channel black/white
// gibt zurueck, ob ellipse mgl als spot oder kopf eingefuegt wurde oder zur modifizierung mgl beigetragen hat
bool MarkerCasernList::mayAddEllipse(const Mat &img, const MyEllipse& e, bool blackInside)
{
    int i;

    int cx = myRound(e.center().x());
    int cy = myRound(e.center().y());
//     char *data = img->imageData; 
//     int iw = img->width;

    // maybe spot
    if (blackInside && // marker have to be black inside
        (e.r1()>SPOT_SIZE_MIN && e.r1()<SPOT_SIZE_MAX && e.r2()>SPOT_SIZE_MIN && e.r2()<SPOT_SIZE_MAX && e.ratio()<2.) && // ellipse size
        ((cx < img.cols-3) && (cx > 1) && (cy < img.rows-3) && (cy > 1))) // && // not near the image border
// folgende abfragen werden jetzt ueber blackInside abgefragt, was ueber die flaechenberechung erstellt wird 
//         ((*(data+cx+cy*iw) != -1) || // ellipse color must be somehow black inside (-1 == weiss, 0 == schwarz)
//          (*(data+cx+1+cy*iw) != -1) || (*(data+cx-1+cy*iw) != -1) ||
//          (*(data+cx+(cy+1)*iw) != -1) || (*(data+cx+(cy-1)*iw) != -1)))
//         // die suche nach schwarz fkt nicht immer: quadrat mit farbmarker koennte herausfallen
//         // oder geschwungene kontur wie bumerang koennte in der mitte der ellipse schwarz sein, obwohl bumerang weiss
    {
        int s = -1;
        for (i = 0; i < size(); ++i)
        {
            if ((s = at(i).isOverlappingSpots(e)) > -1)
            {
                // value(i) - returns T
                // [i] - returns T&
                // at(i) - returns const T&
                (*this)[i].modifySpot(s, e);
                break;
            } else if (at(i).isOverlappingHead(e))
            {
                (*this)[i].addSpot(e);
                break;
            }
        }
        if (s == -1)
        {
            MarkerCasern m;
            m.addSpot(e);
            append(m);
// if (cx > 141 && cx < 147 && cy > 185 && cy < 191)
// {
//     debout << "falsche ellipse!!"<<endl;
// cvNamedWindow("img", CV_WINDOW_AUTOSIZE ); // 0 wenn skalierbar sein soll
// cvShowImage("img", img);
// // // //cvWaitKey( 0 ); // zahl statt null, wenn nach bestimmter zeit weitergegangen werden soll
// }

        }
        return true;
    // maybe head
    } else if (!blackInside && // head has to be white inside
        e.r1()>HEAD_SIZE_MIN && e.r1()<HEAD_SIZE_MAX && e.r2()>HEAD_SIZE_MIN && e.r2()<HEAD_SIZE_MAX && e.ratio()<3.) // was 12..50
    {
        bool doesExist = false;
        int j;
        for (i = 0; i < size() && !doesExist; ++i)
        {
            if (at(i).isOverlappingHead(e))
            {
                (*this)[i].modifyHead(e);
                doesExist = true;
            }
            else if (!at(i).hasHead())
            {

                for (j = 0; j < at(i).spots().size(); ++j)
                    if (e.isInside(at(i).spots()[j].center()))
                    {
                        (*this)[i].modifyHead(e);
                        doesExist = true;
                        break;
                    }
            }
        }
        if (!doesExist)
            append(MarkerCasern(e));
        return true;
    }
    return false;
}

// gibt zurueck, ob Quadrangle mgl eingefuegt wurde oder zur modifizierung beigetragen hat
bool MarkerCasernList::mayAddQuadrangle(const Vec2F v[4]) //Vec2F p1, Vec2F p2, Vec2F p3, Vec2F p4
{
    Vec2F midPoint = (v[0]+v[1]+v[2]+v[3])*0.25;
    for (int i = 0; i < size(); ++i)
    {
        if (at(i).hasHead() && at(i).head().isInside(midPoint))
        {
            (*this)[i].modifyQuadrangle(v);
            return true; //break;
        }
    }
    return false;
}

// organize every marker and delete marker without head
void MarkerCasernList::organize(const cv::Mat &img, bool autoWB)
{
    int i, j, k, s;
//     for (i = 0; i < size(); ++i)

    // delete marker without head and organize every marker
    for (i = 0; i < size(); )
    {
        if (at(i).hasHead())
        {
            // durch mgl wachsen der head ellipsen muss ueberprueft werden, ob sich im nachhinnein Ueberlagerungen ergeben
            // passiert sehr selten!!
            for (j = i+1; j < size(); ++j)
            {
                if (at(i).isOverlappingHead(at(j).head()))
                {
                    (*this)[i].modifyHead(at(j).head());
                    for (k = 0; k < at(j).spots().size(); ++k)
                    {
                        if ((s = at(i).isOverlappingSpots(at(j).spots()[k])) > -1)
                        {
                            (*this)[i].modifySpot(s, at(j).spots()[k]);
                            break;
                        } else if (at(i).isOverlappingHead(at(j).spots()[k]))
                        {
                            (*this)[i].addSpot(at(j).spots()[k]);
                            break;
                        }
                    }
                    removeAt(j);
                    j=i; // muss von vorne begonnen werden, da head wachsen koennte und nun vorgaenger von j hineinpassen //--j;
                }
            }
            (*this)[i++].organize(img, autoWB);
        }
        else
            removeAt(i);
    }
}

//draw ... Qt

void MarkerCasernList::draw(Mat &img) const
{
    for (int i = 0; i < size(); ++i)
        at(i).draw(img);
}

void MarkerCasernList::toCrossList(QList<TrackPoint> *crossList, bool ignoreWithoutMarker) const
{
    Vec2F v1, v2;

    for (int i = 0; i < size(); ++i)
    {
        if (at(i).hasHead())
        {
            if (at(i).spots().size() == 0)
            {
                if (!ignoreWithoutMarker)
                {
                    v1 = at(i).head().center();
                    crossList->append(TrackPoint(v1, 0)); // 0 schlechteste qualitaet
                }
            }
            else
            {
                v1 = at(i).getCenterSpot().center();
                v2 = at(i).getColorSpot().center();
                crossList->append(TrackPoint(v1, 100, v2, at(i).color())); // 100 beste qualitaet
            }
        }
    }
}
