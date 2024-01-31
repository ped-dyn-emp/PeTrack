/*
 * PeTrack - Software for tracking pedestrians movement in videos
 * Copyright (C) 2024 Forschungszentrum Jülich GmbH, IAS-7
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

// an Treppen koennen grosse unterschiede da sein!!!!
#define HEAD_SIZE_MIN 7
#define HEAD_SIZE_MAX 30
#define SPOT_SIZE_MIN 1
#define SPOT_SIZE_MAX 9
#define MIN_CONTRAST  100 // war 150 bei zu geringem Kontrast wird spot geloescht

#include "helper.h"
#include "markerHermes.h"
#include "tracker.h"

// bei marker koennnte sich fuer jeden spot eine liste gemerkt werden und spaeter ausgleich
// regressionsgrade legen um optimale linie durch marker zu erhalten
MarkerHermes::MarkerHermes(MyEllipse head) : mHead(head), mHasHead(true), mCenterIndex(-1) {}
MarkerHermes::MarkerHermes() : mHasHead(false), mCenterIndex(-1) {}
// muesste alles automatisch durch den defaultdestructor gemacht werden!
MarkerHermes::~MarkerHermes()
{
    mSpots.clear();
    mSpotCount.clear();
}

bool MarkerHermes::isOverlappingHead(const MyEllipse &e) const
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
bool MarkerHermes::isInsideHead(const Vec2F &p) const
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
int MarkerHermes::isOverlappingSpots(const MyEllipse &e) const
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
int MarkerHermes::isInsideSpots(const Vec2F &p) const
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

void MarkerHermes::modifyHead(const MyEllipse &head)
{
    // man koennte auch spots und heads sammeln und am ende eine ueberpruefung vornehmen!!!!!!
    // median , regressionsgrade
    // man koennte auch pruefen:
    // - ob ellips-mittelpkt besser auf linie von markern liegt
    // - ob die outline eine bestimmte groesse besitzt
    if(hasHead())
    {
        if(mHead.outline() > 205 &&
           mHead.outline() < 268) // bevorzugte groesse - nur initial koennte andere erzeugt werden
        {
            if(fabs(mHead.ratio() - 1.5) > fabs(head.ratio() - 1.5)) // because white plate is 14x21cm like a real head
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

void MarkerHermes::modifySpot(int i, const MyEllipse &spot)
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
void MarkerHermes::addSpot(const MyEllipse &spot)
{
    mSpots.append(spot);
    mSpotCount.append(1);
}
void MarkerHermes::deleteSpot(int i)
{
    mSpots.removeAt(i);
    mSpotCount.removeAt(i);
}

// nach organize gibt es entweder 0 oder 1 spots pro kopf
void MarkerHermes::organize(const cv::Mat &img, bool /*autoWB*/)
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

    // nach groesse sortieren und loeschen koennte kreuz angreifen zugunsten von zahl

    // gucken, ob in ellipse wirklich zB bullet drin ist

    // spots loeschen, die selten erkannt wurden
    // mind 5 stehen lassen

    // nur die spots mit am meisten treffern stehen lassen
    int maxCount = 0;
    for(i = 0; i < mSpotCount.size(); ++i)
    {
        if(mSpotCount[i] > maxCount)
        {
            maxCount = mSpotCount[i];
        }
    }

    for(i = 0; i < mSpotCount.size(); ++i)
    {
        if(mSpotCount[i] < maxCount)
        {
            deleteSpot(i);
            --i;
        }
    }


    // 1 spots heraussuchen, bei denen um center in quadrat. bereich in ellipse der dunkelste Pkt (kleiner value in hsv)
    // gefunden wird 1 uebrig lassen, weil entweder nummer an kopfseite sehr dunkel sein kann, wenn zu sehen oder in
    // sehr hellen bereichen kreuz wegfallen wuerde
    int cx, cy, r;
    if(mSpots.size() > 1)
    {
        QList<int> minValList;
        for(i = 0; i < mSpots.size(); ++i)
        {
            cx         = myRound(mSpots[i].x());
            cy         = myRound(mSpots[i].y());
            r          = myRound((mSpots[i].r1() + mSpots[i].r2()) / 2.); // mittlerer radius
            int minVal = 256;                                             // groesser als komplett weiss (255)
            // bildrand muss nicht ueberprueft werden, da sie gar nicht erst hereingekommen waeren
            for(j = -r; j < r + 1; ++j)
            {
                for(k = -r; k < r + 1; ++k)
                {
                    if(getValue(img, cx + j, cy + k).value() < minVal)
                    {
                        minVal = getValue(img, cx + j, cy + k).value(); // Helligkeit
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
            if(minValList[i] > minValListSort[0]) // nur der dunkelste bleibt uebrig
            {
                deleteSpot(i);
                minValList.removeAt(i);
                --i;
            }
        }
    }

    // ab hier nur noch 0 oder 1 spots!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // bei 0 muss keine farbe mehr bestimmt werden
    if(mSpots.size() == 1)
    {
        cx         = myRound(mSpots[0].x());
        cy         = myRound(mSpots[0].y());
        r          = myRound((mSpots[0].r1() + mSpots[0].r2()) / 2.); // mittlerer radius
        int minVal = 256;                                             // groesser als komplett weiss (255)

        // bildrand muss nicht ueberprueft werden, da sie gar nicht erst hereingekommen waeren
        for(j = -r; j < r + 1; ++j)
        {
            for(k = -r; k < r + 1; ++k)
            {
                if(getValue(img, cx + j, cy + k).value() < minVal)
                {
                    minVal = getValue(img, cx + j, cy + k).value(); // Helligkeit
                }
            }
        }
        int maxVal = 0;
        for(j = -2 * r; j < 2 * r + 1; ++j)
        {
            for(k = -2 * r; k < 2 * r + 1; ++k)
            {
                if((cx + j < img.cols) && (cx + j > -1) && (cy + k < img.rows) &&
                   (cy + k > -1)) // bildrand muss ueberprueft werden
                {
                    if(getValue(img, cx + j, cy + k).value() > maxVal)
                    {
                        maxVal = getValue(img, cx + j, cy + k).value(); // Helligkeit
                    }
                }
            }
        }

        if(maxVal - minVal < MIN_CONTRAST) // war 150bei zu geringem Kontrast wird spot geloescht
        {
            deleteSpot(0);
            return;
        }

        mCenterIndex = 0;
    }
}

MyEllipse MarkerHermes::getCenterSpot() const
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

void MarkerHermes::draw(cv::Mat &img) const
{
    int i;

    // head
    if(hasHead())
    {
        head().draw(img, 50, 50, 255);
    }
    // marker
    for(i = 0; i < mSpots.size(); ++i)
    {
        if(i == mCenterIndex)
        {
            mSpots[i].draw(img, 150, 150, 0);
        }
        else
        {
            mSpots[i].draw(img, 0, 150, 150);
        }
    }
    if(hasHead() && mSpots.size() == 0)
    {
        MyEllipse e(head().center().x(), head().center().y(), 4, 4, 0);
        e.draw(img, 255, 0, 255);
    }
}

//------------------------------------------------------------------------------------------

// img is 1 channel black/white
// gibt zurueck, ob ellipse mgl als spot oder kopf eingefuegt wurde oder zur modifizierung mgl beigetragen hat
bool MarkerHermesList::mayAddEllipse(const cv::Mat &img, const MyEllipse &e, bool blackInside)
{
    int i;

    int cx = myRound(e.center().x());
    int cy = myRound(e.center().y());

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
            MarkerHermes m;
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
        for(i = 0; i < size() && !doesExist; ++i)
        {
            if(at(i).isOverlappingHead(e))
            {
                (*this)[i].modifyHead(e);
                doesExist = true;
            }
            else if(!at(i).hasHead())
            {
                for(j = 0; j < at(i).spots().size(); ++j)
                {
                    if(e.isInside(at(i).spots()[j].center()))
                    {
                        (*this)[i].modifyHead(e);
                        doesExist = true;
                        break;
                    }
                }
            }
        }
        if(!doesExist)
        {
            append(MarkerHermes(e));
        }
        return true;
    }
    return false;
}

// organize every marker and delete marker without head
void MarkerHermesList::organize(const cv::Mat &img, bool autoWB)
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
                    (*this)[i].modifyHead(at(j).head());
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
            (*this)[i++].organize(img, autoWB);
        }
        else
        {
            removeAt(i);
        }
    }
}

// draw ... Qt

void MarkerHermesList::draw(cv::Mat &img) const
{
    for(int i = 0; i < size(); ++i)
    {
        at(i).draw(img);
    }
}

void MarkerHermesList::toCrossList(QList<TrackPoint> *crossList, bool ignoreWithoutMarker) const
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
                    v1 = at(i).head().center();
                    crossList->append(TrackPoint(v1, 0)); // 0 schlechteste qualitaet
                }
            }
            else
            {
                v1 = at(i).getCenterSpot().center();
                crossList->append(TrackPoint(v1, 100)); // 100 beste qualitaet
            }
        }
    }
}
