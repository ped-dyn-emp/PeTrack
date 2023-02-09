/*
 * PeTrack - Software for tracking pedestrians movement in videos
 * Copyright (C) 2023 Forschungszentrum Jülich GmbH, IAS-7
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

// folgende zeile spaeter raus
#include <opencv2/highgui.hpp>

#define SPOT_COLOR 0 // HUE==0, hier ROT // fuehrt zu folgendem range: 0-20 + 340-359

// Marker Size is a A4 copy of file (from takahiro from japan, nishinari group)

#include "helper.h"
#include "logger.h"
#include "markerJapan.h"
#include "tracker.h"

// bei marker koennnte sich fuer jeden spot eine liste gemerkt werden und spaeter ausgleich
// regressionsgrade legen um optimale linie durch marker zu erhalten
MarkerJapan::MarkerJapan(MyEllipse head) :
    mHead(head), mHasHead(true), mHasQuadrangle(false), mCenterIndex(-1), mColorIndex(-1)
{
}
MarkerJapan::MarkerJapan() : mHasHead(false), mCenterIndex(-1), mColorIndex(-1) {}
// muesste alles automatisch durch den defaultdestructor gemacht werden!
MarkerJapan::~MarkerJapan()
{
    mSpots.clear();
    mSpotCount.clear();
}

// 3 inline functions to get access to class member varaibales only in .h file

bool MarkerJapan::isOverlappingHead(const MyEllipse &e) const
{
    if(hasHead())
    {
        return mHead.isInside(e.center()) || e.isInside(mHead.center());
    }
    else
    {
        return false;
    }
}
bool MarkerJapan::isInsideHead(const Vec2F &p) const
{
    if(hasHead())
    {
        return mHead.isInside(p);
    }
    else
    {
        return false;
    }
}

// returns spot number in spots list when inside, otherwise returns -1
int MarkerJapan::isOverlappingSpots(const MyEllipse &e) const
{
    for(int i = 0; i < mSpots.size(); ++i)
    {
        if((mSpots[i].isInside(e.center()) || e.isInside(mSpots[i].center())) && // Mittelpkte liegen ineinander
           ((e.center() - mSpots[i].center()).length() < 2))                     // kein zu grosser Abstand
        {
            return i;
        }
    }
    return -1;
}
int MarkerJapan::isInsideSpots(const Vec2F &p) const
{
    for(int i = 0; i < mSpots.size(); ++i)
    {
        if(mSpots[i].isInside(p))
        {
            return i;
        }
    }
    return -1;
}

void MarkerJapan::modifyHead(const MyEllipse &head, float headSize)
{
    // man koennte auch spots und heads sammeln und am ende eine ueberpruefung vornehmen!!!!!!
    // median , regressionsgrade
    // man koennte auch pruefen:
    // - ob ellips-mittelpkt besser auf linie von markern liegt
    // - ob die outline eine bestimmte groesse besitzt
    float outlineMin = headSize * 3.5; // 3.92;
    float outlineMax = headSize * 4.5; // 4.967;

    if(hasHead())
    {
        if(mHead.outline() > outlineMin &&
           mHead.outline() < outlineMax) // bevorzugte groesse - nur initial koennte andere erzeugt werden
        {
            if(fabs(mHead.ratio() - 1.43) >
               fabs(head.ratio() - 1.43)) // because white plate marker casern is 14x21cm -> 1.5 // for A4 -> 1.4285
            {
                mHead = head;
            }
        }
    }
    else
    {
        mHasHead = true;
        mHead    = head;
    }
}

void MarkerJapan::modifySpot(int i, const MyEllipse &spot)
{
    if(i < mSpots.size())
    {
        // man koennte auch pruefen:
        // - ob abstand zur mittellinie geringer
        // - ob form eher kreisfoermig
        if(spot.outline() > mSpots[i].outline())
        {
            mSpots[i] = spot;
        }
        ++mSpotCount[i];
    }
}
void MarkerJapan::addSpot(const MyEllipse &spot)
{
    mSpots.append(spot);
    mSpotCount.append(1);
}
void MarkerJapan::deleteSpot(int i)
{
    mSpots.removeAt(i);
    mSpotCount.removeAt(i);
}

void MarkerJapan::modifyQuadrangle(const Vec2F v[4])
{
    if(mHasQuadrangle)
    {
        // seitenverhaeltnis 2:3 koennte auch bedacht werden
        // angle on opposed edge is nearer to 90
        if((fabs((v[3] - v[2]).angleBetweenVec(v[0] - v[3]) - PI / 2.) +
            fabs((v[1] - v[0]).angleBetweenVec(v[2] - v[1]) - PI / 2.)) <
           (fabs((mQuadrangle[3] - mQuadrangle[2]).angleBetweenVec(mQuadrangle[0] - mQuadrangle[3]) - PI / 2.) +
            fabs((mQuadrangle[1] - mQuadrangle[0]).angleBetweenVec(mQuadrangle[2] - mQuadrangle[1]) - PI / 2.)))
        {
            for(int i = 0; i < 4; ++i)
            {
                mQuadrangle[i] = v[i];
            }
        }
    }
    else
    {
        mHasQuadrangle = true;
        for(int i = 0; i < 4; ++i)
        {
            mQuadrangle[i] = v[i];
        }
    }
}


// nach organize gibt es entweder 0 oder 3 spots pro kopf
void MarkerJapan::organize(const cv::Mat &img, bool autoWB)
{
    int i, j, k;

    // direkt herausspringen, da nichts zu tun ist
    if(mSpots.size() == 0)
    {
        return;
    }

    // durch mgl wachsen der spots muss ueberprueft werden, ob sich im nachhinnein Ueberlagerungen ergeben
    for(i = 0; i < mSpots.size(); ++i)
    {
        for(j = i + 1; j < mSpots.size(); ++j)
        {
            if(mSpots[i].isInside(mSpots[j].center()) || mSpots[j].isInside(mSpots[i].center()))
            {
                modifySpot(i, mSpots[j]);
                deleteSpot(j);
                j = i; // es muss von vorne begonnen werden, da i wachsen koennte und dann vorgaenger von j nun
                       // hineinpassen koennten // --j;
            }
        }
    }

    // nach groesse sortieren und loeschen koennte kreus angreifen zugunsten von zahl

    // gucken, ob in ellipse wirklich zB bullet drin ist

    // spots loeschen, die selten erkannt wurden
    // mind 5 stehen lassen (marker und zahlen - auch von der seite zu sehen)
    int count = 100, anz = 0;
    while(count > 4)
    {
        ++anz;
        count = 0;
        for(i = 0; i < mSpotCount.size(); ++i)
        {
            if(mSpotCount[i] > anz)
            {
                count++;
            }
        }
    }
    for(i = 0; i < mSpotCount.size(); ++i)
    {
        if(mSpotCount[i] < anz)
        {
            deleteSpot(i);
            --i;
        }
    }

    // 2 spots heraussuchen, bei denen um center in quadrat. bereich in ellipse der dunkelste Pkt (kleiner value in hsv)
    // gefunden wird
    QColor col;
    int    cx, cy;
    int    numberRemaining = 2;
    if(mSpots.size() > numberRemaining)
    {
        int        r;
        QList<int> minValList;
        for(i = 0; i < mSpots.size(); ++i)
        {
            cx = myRound(mSpots[i].x());
            cy = myRound(mSpots[i].y());
            r  = 1;           // fuehrte dazu, das zuweit gesucht wird und inangrenzenden marker hineingesuct wurde:
                              // myRound((mSpots[i].r1()+mSpots[i].r2())/2.); // mittlerer radius
            int minVal = 256; // groesser als komplett weiss (255)
            // bildrand muss nicht ueberprueft werden, da sie gar nicht erst hereingekommen waeren
            for(j = -r; j < r + 1; ++j)
            {
                for(k = -r; k < r + 1; ++k)
                {
                    col.setRgb(getValue(img, cx + j, cy + k).rgb());
                    if(col.value() < minVal)
                    {
                        minVal = col.value(); // Helligkeit
                    }
                }
            }
            minValList.append(minVal);
        }
        // loeschen aller zu hellen spots
        QList<int> minValListSort = minValList;
        std::sort(minValListSort.begin(), minValListSort.end()); // sortiert aufsteigend
        for(i = 0; i < mSpots.size(); ++i)
        {
            if(minValList[i] > minValListSort[numberRemaining - 1])
            {
                deleteSpot(i);
                minValList.removeAt(i);
                --i;
            }
        }
    }

    // bei 0 muss keine farbe mehr bestimmt werden
    if(mSpots.size() < 2)
    {
        return;
    }

    // farbe bestimmen (richtiger marker koennte auch durch form oder dem abstand des hellsten pkt in groesserer
    // umgebung vom mittelpkt bestimmt werden)
    bool   colSet = false;
    QColor colOther; // farbe des anderen spots
    for(i = 0; i < mSpots.size(); ++i)
    {
        if(i != mCenterIndex)
        {
            cx = myRound(mSpots[i].x());
            cy = myRound(mSpots[i].y());
            if(!colSet)
            {
                colSet = true;
                mCol.setRgb(getValue(img, cx, cy).rgb());
                mColorIndex = i;
            }
            else
            {
                colOther.setRgb(getValue(img, cx, cy).rgb());
                mOtherIndex = i;
            }
            // bildrand muss nicht ueberprueft werden, da sie gar nicht erst hereingekommen waeren
            for(j = -1; j < 2; ++j) // 3x3 feld wird ueberprueft
            {
                for(k = -1; k < 2; ++k)
                {
                    col.setRgb(getValue(img, cx + j, cy + k).rgb());
                    // faktor, um die mitte hoeher zu gewichten, da bei fd der schwarze rand mit dem weiss gemischt oft
                    // hell ist
                    if(col.value() / (1. + .5 * (abs(j) + abs(k))) > mCol.value()) // wenn Helligkeit maximal
                    {
                        if(i != mColorIndex)
                        {
                            colOther    = mCol;
                            mOtherIndex = mColorIndex;
                        }
                        mCol        = col;
                        mColorIndex = i;
                    }
                }
            }
        }
    }

    int  fromColor   = (SPOT_COLOR + 340) % 360;
    int  toColor     = (SPOT_COLOR + 20) % 360;
    bool corectColor = false;
    int  hue         = mCol.hue();
    if(fromColor > toColor) // geht ueber farbrad 359 hinaus, alo 0..toColor + fromColor..359
    {
        if((hue <= toColor) || (hue >= fromColor))
        {
            corectColor = true;
        }
    }
    else
    {
        if((hue >= fromColor) && (hue <= toColor))
        {
            corectColor = true;
        }
    }
    if(!corectColor) // spots loeschen, wenn nicht die richtige farbe, dadurch wird marker spaeter aussortiert
    {
        deleteSpot(0);
        deleteSpot(1);
    }

    if(autoWB)
    {
        // weissabgleich
        // 1. Dynamik der Helligkeit
        // 2. farbverschiebung entsprechend weisswertes

        // weisswert und durchschnittlichen weisswert bestimmen
        // 2 bereiche neben farbmarker in weisser flaeche auf kopfpappe
        int    wcx = myRound(mSpots[mColorIndex].x() + 0.356 * (mSpots[mColorIndex].y() - mSpots[mOtherIndex].y()));
        int    wcy = myRound(mSpots[mColorIndex].y() + 0.356 * (mSpots[mColorIndex].x() - mSpots[mOtherIndex].x()));
        QColor white(0, 0, 0);                              // hellster fleck
        int    avgWhiteR = 0, avgWhiteG = 0, avgWhiteB = 0; // durchschnittlichen weisswert

        for(j = -1; j < 2; ++j) // 3x3 feld um ocx,ocy wird ueberprueft
        {
            for(k = -1; k < 2; ++k)
            {
                QColor color = getValue(img, wcx + j, wcy + k);
                avgWhiteR += color.red();
                avgWhiteG += color.green();
                avgWhiteB += color.blue();
                col.setRgb(getValue(img, wcx + j, wcy + k).rgb());
                if(col.value() > white.value()) // wenn Helligkeit groesser
                {
                    white = col;
                }
            }
        }
        wcx = myRound(mSpots[mColorIndex].x() - 0.356 * (mSpots[mColorIndex].y() - mSpots[mOtherIndex].y()));
        wcy = myRound(mSpots[mColorIndex].y() - 0.356 * (mSpots[mColorIndex].x() - mSpots[mOtherIndex].x()));
        for(j = -1; j < 2; ++j) // 3x3 feld um ocx,ocy wird ueberprueft
        {
            for(k = -1; k < 2; ++k)
            {
                QColor color = getValue(img, wcx + j, wcy + k);
                avgWhiteR += color.red();
                avgWhiteG += color.green();
                avgWhiteB += color.blue();
                col.setRgb(getValue(img, wcx + j, wcy + k).rgb());
                if(col.value() > white.value()) // wenn Helligkeit groesser
                {
                    white = col;
                }
            }
        }

        avgWhiteR /= 18;
        avgWhiteG /= 18;
        avgWhiteB /= 18;

        if((avgWhiteR != 0) && (avgWhiteG != 0) && (avgWhiteB != 0))
        {
            mCol.setRgb(
                (int) MIN(((255. / avgWhiteR) * mCol.red()), 255.),
                (int) MIN(((255. / avgWhiteG) * mCol.green()), 255.),
                (int) MIN(((255. / avgWhiteB) * mCol.blue()), 255.));
            colOther.setRgb(
                (int) MIN(((255. / avgWhiteR) * colOther.red()), 255.),
                (int) MIN(((255. / avgWhiteG) * colOther.green()), 255.),
                (int) MIN(((255. / avgWhiteB) * colOther.blue()), 255.));
        }
        else
        {
            SPDLOG_WARN("white is too dark!");
        }

        // eigentlich muesset saettigung ueber histogramm angepasst werden!!
    }


    // wenn auf markern farbe nahezu identisch
    // weisst dies auf einen schwarzen marker hin,
    // dann wird der marker genommen, der den groessten radius hat
    if(abs(mCol.value() - colOther.value()) < 10) //&& abs(mCol.saturation()-colOther.saturation()) < 40
    {
        SPDLOG_WARN(
            "both marker have nearly the same color at ({:.2f}, {:.2f}", mHead.center().x(), mHead.center().y());
    }
}

Vec2F MarkerJapan::getCenter() const
{
    if(mColorIndex == -1 || mOtherIndex == -1 || mSpots.size() < 2)
    {
        return mHead.center(); // good fallback ?
    }
    else
    {
        return (mSpots[0].center() + mSpots[1].center()) / 2.;
    }
}

MyEllipse MarkerJapan::getCenterSpot() const
{
    if(mCenterIndex == -1)
    {
        return mHead; // good fallback ?
    }
    else
    {
        return mSpots[mCenterIndex];
    }
}

MyEllipse MarkerJapan::getColorSpot() const
{
    if(mColorIndex == -1)
    {
        return mHead; // good fallback ?
    }
    else
    {
        return mSpots[mColorIndex];
    }
}

void MarkerJapan::draw(cv::Mat &img) const
{
    int i;

    // head
    if(hasHead())
    {
        head().draw(img, 255, 0, 255);
    }
    // marker
    for(i = 0; i < mSpots.size(); ++i)
    {
        if(i == mCenterIndex)
        {
            mSpots[i].draw(img, 255, 0, 0);
        }
        else
        {
            mSpots[i].draw(img, 0, 0, 255);
        }
    }
    if(hasHead() && mSpots.size() == 0)
    {
        MyEllipse e(head().center().x(), head().center().y(), 4, 4, 0);
        e.draw(img, 255, 0, 0);
    }

    // quadrangle
    if(hasQuadrangle())
    {
        std::vector<cv::Point> pt;
        for(i = 0; i < 4; ++i)
        {
            pt.push_back(cv::Point(mQuadrangle[i].x(), mQuadrangle[i].y()));
        }
        // draw the square as a closed polyline
        cv::polylines(img, pt, true, CV_RGB(0, 255, 0), 1, cv::LINE_AA, 0);
    }
}

//------------------------------------------------------------------------------------------

// img is 1 channel black/white
// gibt zurueck, ob ellipse mgl als spot oder kopf eingefuegt wurde oder zur modifizierung mgl beigetragen hat
bool MarkerJapanList::mayAddEllipse(const cv::Mat &img, const MyEllipse &e, bool blackInside)
{
    int i;

    int cx = myRound(e.center().x());
    int cy = myRound(e.center().y());

    // radius fuer japan ellipsen warvorher als Makroangegeben
    float HEAD_SIZE_MIN = mHeadSize / 2.;
    float HEAD_SIZE_MAX = mHeadSize; // mHeadSize ist Durchmesser eines kopfes in pixel
    float SPOT_SIZE_MIN = mHeadSize / 16.;
    float SPOT_SIZE_MAX = mHeadSize / 4.;
    int   iHead         = -1;

    // maybe spot
    if(blackInside && // marker have to be black inside
       (e.r1() > SPOT_SIZE_MIN && e.r1() < SPOT_SIZE_MAX && e.r2() > SPOT_SIZE_MIN && e.r2() < SPOT_SIZE_MAX &&
        e.ratio() < 2.) &&                                                   // ellipse size
       ((cx < img.cols - 3) && (cx > 1) && (cy < img.rows - 3) && (cy > 1))) // && // not near the image border
    {
        int s = -1;
        for(i = 0; i < size(); ++i)
        {
            if((s = at(i).isOverlappingSpots(e)) > -1)
            {
                // value(i) - returns T
                // [i] - returns T&
                // at(i) - returns const T&
                (*this)[i].modifySpot(s, e);
                break;
            }
            else if(at(i).isOverlappingHead(e))
            {
                (*this)[i].addSpot(e);
                break;
            }
        }
        if(s == -1)
        {
            MarkerJapan m;
            m.addSpot(e);
            append(m);
        }
        return true;
        // maybe head
    }
    else if(
        !blackInside && // head has to be white inside
        e.r1() > HEAD_SIZE_MIN && e.r1() < HEAD_SIZE_MAX && e.r2() > HEAD_SIZE_MIN && e.r2() < HEAD_SIZE_MAX &&
        e.ratio() < 3.) // was 12..50
    {
        bool doesExist = false;
        int  j;
        for(i = 0; i < size(); ++i) //  && !doesExist
        {
            if(!doesExist && at(i).isOverlappingHead(e)) // ueberlappen sich zwei koepfe
            {
                (*this)[i].modifyHead(e, mHeadSize);
                doesExist = true;
            }
            else if(!at(i).hasHead()) // marker hat noch keinen kopf
            {
                for(j = 0; j < at(i).spots().size(); ++j)
                {
                    if(e.isInside(at(i).spots()[j].center()))
                    {
                        if(!doesExist)
                        {
                            (*this)[i].modifyHead(e, mHeadSize);
                            iHead     = i;
                            doesExist = true;
                        }
                        else if(iHead != -1) // restlichen spots untersuchen, ob sie auch noch in kopf liegen
                        {
                            (*this)[iHead].addSpot(at(i).spots()[j]);
                            (*this)[i].deleteSpot(j);
                            --j;
                        }
                    }
                }
            }
        }

        if(!doesExist)
        {
            append(MarkerJapan(e));
        }
        return true;
    }
    return false;
}

// gibt zurueck, ob Quadrangle mgl eingefuegt wurde oder zur modifizierung beigetragen hat
bool MarkerJapanList::mayAddQuadrangle(const Vec2F v[4]) // Vec2F p1, Vec2F p2, Vec2F p3, Vec2F p4
{
    Vec2F midPoint = (v[0] + v[1] + v[2] + v[3]) * 0.25;
    for(int i = 0; i < size(); ++i)
    {
        if(at(i).hasHead() && at(i).head().isInside(midPoint))
        {
            (*this)[i].modifyQuadrangle(v);
            return true; // break;
        }
    }
    return false;
}

// organize every marker and delete marker without head
void MarkerJapanList::organize(const cv::Mat &img, bool autoWB)
{
    int i, j, k, s;

    // delete marker without head and organize every marker
    for(i = 0; i < size();)
    {
        if(at(i).hasHead())
        {
            // durch mgl wachsen der head ellipsen muss ueberprueft werden, ob sich im nachhinnein Ueberlagerungen
            // ergeben passiert sehr selten!!
            for(j = i + 1; j < size(); ++j)
            {
                if(at(i).isOverlappingHead(at(j).head()))
                {
                    (*this)[i].modifyHead(at(j).head(), mHeadSize);
                    for(k = 0; k < at(j).spots().size(); ++k)
                    {
                        if((s = at(i).isOverlappingSpots(at(j).spots()[k])) > -1)
                        {
                            (*this)[i].modifySpot(s, at(j).spots()[k]);
                            break;
                        }
                        else if(at(i).isOverlappingHead(at(j).spots()[k]))
                        {
                            (*this)[i].addSpot(at(j).spots()[k]);
                            break;
                        }
                    }
                    removeAt(j);
                    j = i; // muss von vorne begonnen werden, da head wachsen koennte und nun vorgaenger von j
                           // hineinpassen //--j;
                }
            }
            (*this)[i].organize(img, autoWB);
            if(!at(i).hasSpots()) // if after organization no spot is inside had
            {
                removeAt(i);
            }
            else // hat mind. 2 spots
            {
                // Abstand der Spots bestimmen (nur hier headsize bekannt)
                float spotDistance = at(i).spots().at(0).center().distanceToPoint(at(i).spots().at(1).center());
                if(spotDistance >
                   0.8 * mHeadSize) // ein Hoechstabstand soll eingehalten werden, in Japan waren es hier 12 // kein
                                    // Minimalabstand da bei Schraegsicht abstand kleiner werden kann
                {
                    removeAt(i);
                }
                else
                {
                    i++;
                }
            }
        }
        else
        {
            removeAt(i);
        }
    }
}

// draw ... Qt

void MarkerJapanList::draw(cv::Mat &img) const
{
    for(int i = 0; i < size(); ++i)
    {
        at(i).draw(img);
    }
}

void MarkerJapanList::toCrossList(QList<TrackPoint> *crossList, bool ignoreWithoutMarker) const
{
    Vec2F v1, v2;

    for(int i = 0; i < size(); ++i)
    {
        if(at(i).hasHead())
        {
            if(at(i).spots().size() == 0)
            {
                if(!ignoreWithoutMarker)
                {
                    v1 = at(i).getCenter();
                    crossList->append(TrackPoint(v1, 0)); // 0 schlechteste qualitaet
                }
            }
            else
            {
                v1 = at(i).getCenter();
                v2 = at(i).getColorSpot().center();
                crossList->append(TrackPoint(v1, 100, v2, at(i).color())); // 100 beste qualitaet
            }
        }
    }
}
