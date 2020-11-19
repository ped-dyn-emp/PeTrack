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

#ifndef PERSON_H
#define PERSON_H

#include <QList>
#include "ellipse.h"
#include "stereoContext.h"
#include "backgroundFilter.h"
#include "tracker.h"
//#include "opencv2/core/types_c.h"

#define STEP_SIZE 5 //10 // Hoehenschritte der Isolinien in cm
#define DISP_GAP_SIZE_TO_FILL 15 // 15 pixel innerhalb einer Zeile werden linear in disp interpoliert

class Person: public QList<MyEllipse> // List is approximated isoline
{
private:
    float mDistTopEllipse; // entfernung zur camera; Schrittweite ist STEP_SIZE zu entnehmen
    bool mActLevel; // zeigt an, ob fuer aktuellen Level eine Ellipse gefunden wurde
public:
    Person(const MyEllipse &e)
        : mDistTopEllipse(FLT_MAX), mActLevel(false)
    {
        append(e);
    }
    Person(const MyEllipse &e, float d)
        : mDistTopEllipse(d), mActLevel(false)
    {
//        if (d < mDistTopEllipse) // naeher dran an Kamera
//            mDistTopEllipse = d;
        append(e);
    }
    inline void setActLevel(bool b)
    {
        mActLevel = b;
    }
    inline bool getActLevel() const
    {
        return mActLevel;
    }
    inline void setDistTopEllipse(float h)
    {
        mDistTopEllipse = h;
    }
    inline float getDistTopEllipse() const
    {
        return mDistTopEllipse;
    }
    const MyEllipse *getHeadEllipse() const // muss pointer sein, da sonst NULL nicht moeglich
    {
        if (size() > (10/STEP_SIZE)) // war 1 //
            return &at(10/STEP_SIZE);
        return NULL;
    }
};

class PersonList: public QList<Person>
{
private:
    pet::StereoContext *mSc;
public:
    PersonList();
    //~PersonList();

    void searchEllipses(pet::StereoContext *sc, QRect &roi, BackgroundFilter* bgFilter);

    // el liste aller ellipsen eines thresholds der hoehe
    // der thrershold durchlaeuft hoehenfeld von kopf bis fuss
    void insertEllipses(const QList<MyEllipse> &el, float distFromCam); // nicht mehr: el wird angepasst, so dass nur noch eingefuegte ellipsen drin

    void optimize();

    // bestimmt kopfposition in markerlosen Ueberkopfaufnahmen aus Hoehenbild
    void calcPersonPos(const cv::Mat &img, QRect &roi, QList<TrackPoint> &persList, pet::StereoContext *sc, BackgroundFilter *bgFilter, bool markerLess=true);
};


#endif // PERSON_H
