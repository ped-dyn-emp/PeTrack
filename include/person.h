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
