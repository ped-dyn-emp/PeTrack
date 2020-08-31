#include "opencv2/opencv.hpp"

#include <time.h>
#include <iomanip>

#include "helper.h"
#include "tracker.h"
#include "petrack.h"
#include "animation.h"
#include "control.h"
#include "stereoWidget.h"
#include "multiColorMarkerWidget.h"

using namespace::cv;
using namespace std;

// Measure Time spending in functions
//#define TIME_MEASUREMENT


#define MIN_WIN_SIZE 3.


inline float errorToQual(float error)
{
    return 80.F - error/20.F;
}

TrackPoint::TrackPoint()
    : mMarkerID(-1),
      mQual(0),
      mSp(-1., -1., -1.)
{
}
TrackPoint::TrackPoint(const Vec2F &p)
    : Vec2F(p),
      mMarkerID(-1),
      mQual(0),
      mSp(-1., -1., -1.)
{
}
TrackPoint::TrackPoint(const Vec2F &p, int qual)
    : Vec2F(p),
      mMarkerID(-1),
      mQual(qual),
      mSp(-1., -1., -1.)
{
}
TrackPoint::TrackPoint(const Vec2F &p, int qual, int markerID)
    : Vec2F(p),
      mMarkerID(markerID),
      mQual(qual),
      mSp(-1.,-1.,-1.)
{
}
TrackPoint::TrackPoint(const Vec2F &p, int qual, const QColor &col)
    : Vec2F(p),
      mCol(col),
      mMarkerID(-1),
      mQual(qual),
      mSp(-1., -1., -1.)
{
}

TrackPoint::TrackPoint(const Vec2F &p, int qual, const Vec2F &colPoint, const QColor &col)
    : Vec2F(p),
      mColPoint(colPoint),
      mCol(col),
      mMarkerID(-1),
      mQual(qual),
      mSp(-1., -1., -1.)
{
}

// const TrackPoint& TrackPoint::operator=(const TrackPoint& tp)
// {
//     Vec2F::operator=(tp);
//     mQual = tp.qual();
//     mColPoint = tp.colPoint();
//     mCol = tp.color();
//     return *this;
// }

const TrackPoint& TrackPoint::operator=(const Vec2F& v)
{
    Vec2F::operator=(v);
    return *this;
}

const TrackPoint& TrackPoint::operator+=(const Vec2F& v)
{
    Vec2F::operator+=(v);
    return *this;
}
const TrackPoint TrackPoint::operator+(const Vec2F& v) const
{
    return TrackPoint(*this) += v; //Vec2F(mX + v.mX, mY + v.mY);
}

//--------------------------------------------------------------------------

// the list index is the frame number plus mFirstFrame 0..mLastFrame-mFirstFrame
// no frame is left blank
TrackPerson::TrackPerson()
    : mNr(0),
      mHeight(MIN_HEIGHT),
      mHeightCount(0),
      mFirstFrame(-1),
      mLastFrame(0),
      mNewReco(false),
      mComment(),
      mColCount(0),
      mNrInBg(0)
{
}
// TrackPerson::TrackPerson(int nr, int frame, const Vec2F &p)
//     : mNr(0),
//       mHeight(MIN_HEIGHT),
//       mFirstFrame(frame),
//       mLastFrame(frame),
//       mNewReco(true)
// {
//     //TrackPoint t = p;
//     //for (int i = 0; i <= frame; ++i)
//     append(p);
// }
TrackPerson::TrackPerson(int nr, int frame, const TrackPoint &p)
    : mNr(0),
      mHeight(MIN_HEIGHT),
      mHeightCount(0),
      mFirstFrame(frame),
      mLastFrame(frame),
      mNewReco(true),
      mCol(p.color()),
      mComment(),
      mColCount(1)
{
    append(p);
}

// mittelt alle erkannten farben der trackpoints zu einer farbe der trackperson
void TrackPerson::addColor(const QColor &col)
{
    mCol.setRed(  (mColCount * mCol.red()   + col.red())   / (mColCount+1));
    mCol.setBlue( (mColCount * mCol.blue()  + col.blue())  / (mColCount+1));
    mCol.setGreen((mColCount * mCol.green() + col.green()) / (mColCount+1));

    ++mColCount;
}

// die gemittelte farb wird hier verbessert
void TrackPerson::optimizeColor()
{
    // ausreisser herausnehmen ueber die koordinate der farbe
    int i, j;
    Vec2F v, vBefore;
    int anz1 = 0, anz2 = 0;
    bool swap = false;

    // den ersten farbpunkt suchen und vBefore initial setzen
    for (i = 0; i < size(); ++i)
    {
        if (at(i).color().isValid())
        {
            vBefore = at(i).colPoint() - at(i);
            break;
        }
    }
    if (at(i).color().isValid())
        ++anz1;
    // testen, auf welcher seit der farbmarker haeufiger gesehen wird
    for (j = i+1; j < size(); ++j)
    {
        if (at(j).color().isValid())
        {
            v = at(j).colPoint() - at(j);
            if ((v * vBefore) < 0)
                swap = !swap;

            if (swap)
                ++anz2;
            else
                ++anz1;
            vBefore = v;
        }
    }
    //debout << anz1 << "  " << anz2 <<endl;
    swap = false;
    if (at(i).color().isValid())
        vBefore = at(i).colPoint() - at(i);
    // farben mit geringerer anzahl loeschen
    QColor colInvalid;
    if (anz2 > anz1)
        (*this)[i].setColor(colInvalid);
    for (j = i+1; j < size(); ++j)
    {
        if (at(j).color().isValid())
        {
            v = at(j).colPoint() - at(j);
            if ((v * vBefore) < 0)
                swap = !swap;

            if (swap)
            {
                if (anz1 > anz2)
                    (*this)[j].setColor(colInvalid);
            }
            else
            {
                if (anz2 > anz1)
                    (*this)[j].setColor(colInvalid);
            }
            vBefore = v;
        }
    }

    // median statt mittelwert nehmen
    QList<int> r;
    QList<int> g;
    QList<int> b;
    for (i = 0; i < size(); ++i)
    {
        if (at(i).color().isValid())
        {
            r.append(at(i).color().red());
            g.append(at(i).color().green());
            b.append(at(i).color().blue());
        }
    }
    std::sort(r.begin(), r.end());
    std::sort(g.begin(), g.end());
    std::sort(b.begin(), b.end());
    if (r.size()>0 && g.size()>0 && b.size()>0) // kann eigentlich nicht vorkommen
        setColor(QColor(r[(r.size()-1)/2],g[(g.size()-1)/2],b[(b.size()-1)/2]));
}

void TrackPerson::recalcHeight(float altitude)
{
    double z = 0;
    //double h = 0;
    // median statt mittelwert nehmen (bei gerader anzahl an werten den kleiner als mitte)
    QList<double> zList;

    resetHeight(); // mHeight = MIN_HEIGHT, mHeightCount = 0

    for (int i = 0; i < size(); ++i)
    {
        z = at(i).sp().z();
        if (z >= 0)
        {
            ++mHeightCount;
            //h += z;
            zList.append(z);
        }
    }
    if (mHeightCount > 0)
    {
        std::sort(zList.begin(), zList.end());
        //mHeight = h / mHeightCount;
        mHeight = zList[mHeightCount/2];
        mHeight = altitude - mHeight;
    }
}

// gibt den ersten z-wert um index i heraus der ungleich -1 ist
// zudem interpolation zwischen Werten!
double TrackPerson::getNearestZ(int i, int *extrapolated)
{
    *extrapolated = 0;
    if ((i < 0) || (i >= size())) // indexueberpruefung
        return -1.;
    if (at(i).sp().z() >= 0)
        return at(i).sp().z();
    else // -1 an aktueller hoehe
    {
        int nrFor = 1;
        int nrRew = 1;
        while ((i+nrFor < size()) && (at(i+nrFor).sp().z() < 0)) // nach && wird nur ausgefuehrt, wenn erstes true == size() also nicht
            nrFor++;
        while ((i-nrRew >= 0) && (at(i-nrRew).sp().z() < 0)) // nach && wird nur ausgefuehrt, wenn erstes true == size() also nicht
            nrRew++;
        if ((i+nrFor == size()) && (i-nrRew < 0)) // gar keine Hoeheninfo in trj gefunden
            return -1.;
        else if (i+nrFor == size()) // nur in Vergangenheit hoeheninfo gefunden
        {
            *extrapolated = 2;
            return at(i-nrRew).sp().z();
        }
        else if (i-nrRew < 0) // nur in der zukunft hoeheninfo gefunden
        {
            *extrapolated = 1;
            return at(i+nrFor).sp().z();
        }
        else // in beiden richtungen hoeheninfo gefunden - INTERPOLATION, NICHT EXTRAPOLATION
            return at(i-nrRew).sp().z()+nrRew*(at(i+nrFor).sp().z()-at(i-nrRew).sp().z())/(nrFor+nrRew); // lineare interpolation
    }
}

// rueckgabe zeigt an, ob neuer p eingefuegt wurde oder nicht, falls qualitaet schlechter
// persNr ist index in uebergeordneter liste zur sinnvollen warnungs-ausgabe
bool TrackPerson::insertAtFrame(int frame, const TrackPoint &p, int persNr, bool extrapolate)
{
//         if (frame == mLastFrame+1) //included in following lines
//             append(p);
    int i;
    Vec2F tmp; // ua. zur linearen Interpolation
    TrackPoint tp; // default: 0 = ist schlechteste qualitaet
    double distance;

//    debout << "frame: " << frame << " first: " << mFirstFrame << " last: " << mLastFrame << endl;

    if (frame > mLastFrame)
    {
        // lineare interpolation, wenn frames uebersprungen wurden
        if (frame-mLastFrame-1 > 0)
        {
            tmp.setX((p.x()-last().x())/(frame-mLastFrame));
            tmp.setY((p.y()-last().y())/(frame-mLastFrame));
            tp = last();
            tp.setQual(0);
            //tp.setZdistanceToCam(p.zDistanceToCam()); keine Interpolation der Groesse
            for (i = 0; i < frame-mLastFrame-1; ++i)
            {
                tp += tmp;
                append(tp);
            }
            append(p);
        }
        else if (extrapolate && ((mLastFrame-mFirstFrame) > 0)) // mind. 2 trackpoints sind in liste!
        {
            tmp = last()-at(size()-2); // vektor zw letztem und vorletztem pkt
            // der Abstand zum extrapoliertem Ziel darf nicht groesser als 2x so gross sein, wie die entfernung vorheriger trackpoints
            // eine mindestbewegung wird vorausgesetzt, da sonst bewegungen aus dem "stillstand" immer als fehler angesehen werden
            //   am besten waere eine fehlerbetrachtung in abhaengigkeit von der geschwindigkeit - nicht gemacht!
            if (((distance = ((last()+tmp).distanceToPoint(p))) > EXTRAPOLATE_FACTOR*tmp.length()) && (distance > 3))
            {
                if (!((last().qual() == 0) && (at(size()-2).qual() == 0))) // das vorherige einfuegen ist 2x nicht auch schon schlecht gewesen
                {
                    tp = p;
//                     if (tp.qual() == 100) // reco oder haendisch - KOMMT NIE VOR, DA VOR RECO AUCH IMMER GETRACKT WIRD !!!!!!!
//                         debout << "Warning: Big difference from tracked point in speed and direction between frame " << mLastFrame << " and " << mLastFrame+1 << "!" << endl;
//                     else
//                     {
                    debout << "Warning: Extrapolation instaed of tracking because of big difference from tracked point in speed and direction of person " << persNr+1 << " between frame " << mLastFrame << " and " << mLastFrame+1 << "!" << endl;
                        tp = last()+tmp; // nur vektor wird hier durch + geaendert
                        tp.setQual(0);
                        // im anschluss koennte noch dunkelster pkt in umgebung gesucht werden!!!
//                     }
                        // keine Extrapolation der Groesse
                    append(tp);
                }
                else
                {
                    debout << "Warning: Because of three big differences from tracked point in speed and direction between last three frames the track point of person " << persNr+1 << " at " << mLastFrame+1 << " was NOT inserted!" <<endl;
                    return false;
                }
            }
            else
                append(p);
        }
        else
            append(p);
        mLastFrame = frame;
    }
    else if (frame < mFirstFrame)
    {
        if (mFirstFrame-frame-1 > 0)
        {
            tmp.setX((p.x()-first().x())/(mFirstFrame-frame));
            tmp.setY((p.y()-first().y())/(mFirstFrame-frame));
            tp = first();
            tp.setQual(0);
            for (i = 0; i < mFirstFrame-frame-1; ++i)
            {
                tp += tmp;
                prepend(tp);
            }
            prepend(p);
        }
        else if (extrapolate && ((mLastFrame-mFirstFrame) > 0)) // mind. 2 trackpoints sind in liste!
        {
            tmp = at(0)-at(1); // vektor zw letztem und vorletztem pkt
            // der Abstand zum extrapoliertem Ziel darf nicht groesser als 2x so gross sein, wie die entfernung vorheriger trackpoints
            // eine mindestbewegung wird vorausgesetzt, da sonst bewegungen aus dem "stillstand" immer als fehler angesehen werden
            //   am besten waere eine fehlerbetrachtung in abhaengigkeit von der geschwindigkeit - nicht gemacht!
            if (((distance = (at(0)+tmp).distanceToPoint(p)) > EXTRAPOLATE_FACTOR*tmp.length()) && (distance > 3))
            {
                if (!((at(0).qual() == 0) && (at(1).qual() == 0))) // das vorherige einfuegen ist 2x nicht auch schon schlecht gewesen
                {
                    tp = p;
                    debout << "Warning: Extrapolation instaed of tracking because of big difference from tracked point in speed and direction of person " << persNr+1 << " between frame " << mFirstFrame << " and " << mFirstFrame-1 << "!" << endl;
                    tp = at(0)+tmp; // nur vektor wird hier durch + geaendert
                    tp.setQual(0);
                    prepend(tp);
                }
                else
                {
                    debout << "Warning: Because of three big differences from tracked point in speed and direction between last three frames the track point of person " << persNr+1 << " at " << mLastFrame-1 << " was NOT inserted!" <<endl;
                    return false;
                }
            }
            else
                prepend(p);
        }
        else
            prepend(p);
        mFirstFrame = frame;
    }
    else
    {
        // dieser Zweig wird insbesondere von reco durchlaufen, da vorher immer auch noch getrackt wird und reco draufgesetzt wird!!!

        tp = p;
//        debout << "qual: " << p.qual() << endl;
        if (p.qual()<100 && p.qual()>80) // erkannte Person aber ohne strukturmarker
        {
            // wenn in angrenzenden Frames qual groesse 90 (100 oder durch vorheriges verschieben entstanden), dann verschieben
            if (trackPointExist(frame-1) && trackPointAt(frame-1).qual()>90)
            {
                tp.setQual(95);
                tmp = p+(trackPointAt(frame-1)-trackPointAt(frame-1).colPoint());
                tp.set(tmp.x(),tmp.y());
                debout << "Warning: move trackpoint according to last distance of structur marker and color marker of person "<< persNr+1 << ":"<<endl;
                debout << "         "<< p <<" -> "<< tp <<endl;
                //debout <<trackPointAt(frame-1)<<endl;
                //debout <<trackPointAt(frame)<<endl;

                //debout << trackPointAt(frame).colPoint()<<endl;
                //at(frame-mFirstFrame)
            }
            else if (trackPointExist(frame+1) && trackPointAt(frame+1).qual()>90)
            {
                tp.setQual(95);
                tmp = p+(trackPointAt(frame+1)-trackPointAt(frame+1).colPoint());
                tp.set(tmp.x(),tmp.y());
                debout << "Warning: move trackpoint according to last distance of structur marker and color marker of person "<< persNr+1 << ":"<<endl;
                debout << "         "<< p <<" -> "<< tp <<endl;
            }
        }
//        debout << tp.qual()<<endl;
//        debout << at(frame-mFirstFrame).qual()<<endl;

        // ueberprueft, welcher punkt besser
        if (tp.qual() > at(frame-mFirstFrame).qual()) // at(frame-mFirstFrame) == trackPointAt(frame)
        {
            // warnung ausgeben, wenn replacement (fuer gewoehnlich von reco) den pfadverlauf abrupt aendert
            if (trackPointExist(frame-1))
                tmp = trackPointAt(frame) - trackPointAt(frame-1);
            else if (trackPointExist(frame+1))
                tmp = trackPointAt(frame+1) - trackPointAt(frame);
            if ((trackPointExist(frame-1) || trackPointExist(frame+1)) && ((distance = (trackPointAt(frame).distanceToPoint(tp))) > 1.5*tmp.length()) && (distance > 3))
            {
                int anz;
                debout << "Warning: Big difference in location between existing and replacing track point of person " << persNr+1 << " in frame " << frame << "!" << endl;
                // qualitaet anpassen, da der weg zum pkt nicht der richtige gewesen sein kann
                // zurueck
                for (anz=1; trackPointExist(frame-anz) && (trackPointAt(frame-anz).qual() < 100); ++anz)
                    ;
                for (i=1; i<(anz-1); ++i) // anz ist einer zu viel; zudem nur boie anz-1 , da sonst eh nur mit 1 multipliziert wuerde
                    (*this)[frame-mFirstFrame-i].setQual((i*trackPointAt(frame-i).qual())/anz);
                // vor
                for (anz=1; trackPointExist(frame+anz) && (trackPointAt(frame+anz).qual() < 100); ++anz)
                    ;
                for (i=1; i<(anz-1); ++i) // anz ist einer zu viel; zudem nur boie anz-1 , da sonst eh nur mit 1 multipliziert wuerde
                    (*this)[frame-mFirstFrame+i].setQual((i*trackPointAt(frame+i).qual())/anz);
            }

            replace(frame-mFirstFrame, tp);
            if (tp.qual() > 100) // manual add // after inserting, because p ist const
                (*this)[frame-mFirstFrame].setQual(100); // so moving of a point is possible
            //debout << "Warning: frame exists already in trajectory!" << endl;
        }
        else
            return false;
    }
    return true;
}

bool TrackPerson::trackPointExist(int frame) const
{
    if (frame >= mFirstFrame && frame <= mLastFrame)
        return true;
    else
        return false;
}
const TrackPoint& TrackPerson::trackPointAt(int frame) const // & macht bei else probleme, sonst mit [] zugreifbar
{
//     if (frame >= mFirstFrame && frame <= mLastFrame) ////!!!! muss vorher ueberprueft werden, ob es existiert!!!
        return at(frame-mFirstFrame);
//     else
//         return TrackPoint();
}

//     ~TrackerPerson();

//     inline const MyEllipse& head() const
//     {
//         return mHead;
//     }
    //void draw(IplImage *img) const;

// gibt -1 zurueck, wenn frame oder naechster frame nicht existiert
// entfernung ist absolut
double TrackPerson::distanceToNextFrame(int frame) const
{
    if (frame >= mFirstFrame && frame+1 <= mLastFrame)
        return at(frame-mFirstFrame).distanceToPoint(at(frame-mFirstFrame+1));
    else
        return -1;
}

//----------------------------------------------------------------------------

// using tracker:
// 1. initial recognition
// 2. next frame track existing track points
// 3. new recognition and check if exist otherwise include new
// (4. delete inner tracking point, which does not recognized over a longer time)
// 5. backward tracking from firstFrame() on
// 6. calculate color over tracking (accumulated over tracking while procedure above) path and set height
// 7. recalc coord with real coord with known height

Tracker::Tracker(QWidget *wParent)
//         : QList::QList()
{
    mMainWindow = (class Petrack*) wParent;

//    mGrey = NULL;
//    mPrevGrey = NULL;
//    mPyramid = NULL;
//    mPrevPyramid = NULL;
//     mPrevImg = NULL;

//    mPrevFeaturePoints.resize(MAX_COUNT);// = vector<Point2f>(MAX_COUNT);
//    mPrevFeaturePointsIdx.resize(MAX_COUNT);// = vector<int>(MAX_COUNT);
//    mFeaturePoints.resize(MAX_COUNT);// = vector<Point2f>(MAX_COUNT);
//    mStatus.resize(MAX_COUNT);// = vector<uchar>(MAX_COUNT);
//    mTrackError.resize(MAX_COUNT);// = vector<float>(MAX_COUNT);

//    mPrevFeaturePoints = (CvPoint2D32f*) cvAlloc(MAX_COUNT*sizeof(CvPoint2D32f)); //points[0]
////    mPrevColorFeaturePoints = (CvPoint2D32f*) cvAlloc(MAX_COUNT*sizeof(CvPoint2D32f)); //points[0]
//    mFeaturePoints = (CvPoint2D32f*) cvAlloc(MAX_COUNT*sizeof(CvPoint2D32f)); //points[1]
////    mColorFeaturePoints = (CvPoint2D32f*) cvAlloc(MAX_COUNT*sizeof(CvPoint2D32f)); //points[1]
//    mStatus = (char*) cvAlloc(MAX_COUNT);
////    mColorStatus = (char*) cvAlloc(MAX_COUNT);
//    mPrevFeaturePointsIdx = (int*) cvAlloc(MAX_COUNT*sizeof(int));
//    mTrackError = (float*) cvAlloc(MAX_COUNT*sizeof(float));
////    mColorTrackError = (float*) cvAlloc(MAX_COUNT*sizeof(float));
    mTermCriteria = TermCriteria(TermCriteria::COUNT|TermCriteria::EPS, 20, 0.03); ///< maxIter=20 and epsilon=0.03

    reset();
}


// neben loeschen der liste muessen auch ...
void Tracker::init(Size size)
{
    clear(); // loescht liste aller getrackten personen
    // nicht mehr noetig, da nicht mehr in track selber // jetzt start, war prevImg == NULL && prevFrame == -1 zeigt an, dass ein neuer Trackingprozess beginnt / neue Bildfolge
//    if (mGrey)
//        cvReleaseImage(&mGrey);
//    if (mPrevGrey)
//        cvReleaseImage(&mPrevGrey);
//    if (mPyramid)
//        cvReleaseImage(&mPyramid);
//    if (mPrevPyramid)
//        cvReleaseImage(&mPrevPyramid);
//debout << "test" << endl;
    mGrey.create(size,CV_8UC1);// = cvCreateImage(size, 8, 1);
    mPrevGrey.create(size,CV_8UC1);// = cvCreateImage(size, 8, 1);
//    debout << "test" << endl;
//    mPyramid.create(size,CV_8UC1);// = cvCreateImage(size, 8, 1);
//    mPrevPyramid.create(size,CV_8UC1);// = cvCreateImage(size, 8, 1);

//if (mGrey) debout << "mGrey: " << mGrey << " " << (void *) mGrey->imageData<<endl;
//if (mMainWindow->getIplImage()) debout << "mImage: " << mMainWindow->getIplImage()<< " " << (void *) mMainWindow->getIplImage()->imageData<<endl;
    reset();
}

// damit neu aufgesetzt werden kann
void Tracker::reset()
{
    mPrevFrame = -1; // das vorherige Bild ist zu ignorieren oder existiert nicht
}

void Tracker::resize(Size size)
{
    if (!mGrey.empty() && ((size.width != mGrey.cols) || (size.height != mGrey.rows)))
    {
        mGrey.create(size,CV_8UC1);

        // umkopieren des alten Graubildes in groesseres oder auch kleineres bild (wg border)
        // aus borderFilter kopiert
        if (!mPrevGrey.empty())
        {
            int s = (size.width-mPrevGrey.cols)/2;
            if( s >= 0)
                cv::copyMakeBorder(mPrevGrey,mPrevGrey,s,s,s,s,cv::BORDER_CONSTANT,cv::Scalar(0));
            else
                mPrevGrey = mPrevGrey(Rect(-s,-s,mPrevGrey.cols+2*s,mPrevGrey.rows+2*s));
        }
    }
//             ; //anpassungen von prev bildmaterial (und punkten selber?)
//         // -getImgBorderSize() nutzen
}

// split trajectorie pers before frame frame
void Tracker::splitPerson(int pers, int frame)
{
    int j;

    if (at(pers).firstFrame()< frame)
    {
        append(at(pers));

        // alte trj einkuerzen und ab aktuellem frame zukunft loeschen
        for (j = 0; j < at(pers).lastFrame()-frame+1; ++j)
            (*this)[pers].removeLast();
        (*this)[pers].setLastFrame(frame-1);

        // neu angehaengte/gedoppelte trajektorie
        for (j = 0; j < frame-last().firstFrame(); ++j)
            last().removeFirst();
        last().setFirstFrame(frame);
    }
}

// split trajectorie before frame frame
// onlyVisible == -1 : immer alles betrachten, ansonsten nur person onlyVisible
// gibt true zurueck, wenn trajektorie gesplttet werden konnte
bool Tracker::splitPersonAt(const Vec2F& p, int frame, QSet<int> onlyVisible)
{
    int i;

    for (i = 0; i < size(); ++i) // ueber TrackPerson
        if (((onlyVisible.empty()) || (onlyVisible.contains(i))) && (at(i).trackPointExist(frame) && (at(i).trackPointAt(frame).distanceToPoint(p) < mMainWindow->getHeadSize(NULL, i, frame)/2.))) // war: MIN_DISTANCE)) // 30 ist abstand zwischen kopfen
        {
            splitPerson(i, frame);

            return true;
        }
    return false;
}

// gibt true zurueck, wenn punkt geloescht werden konnte
// direction zeigt an, ob bis zum aktuellen (-1), ab dem aktuellen (1) oder ganzer trackpath (0)
// onlyVisible == -1 : immer alles betrachten, ansonsten nur person onlyVisible
// loescht trackpoint nur einer trajektorie
bool Tracker::delPointOf(int pers, int direction, int frame)
{
    int j;

    if (direction == -1)
    {
        for (j = 0; j < frame-at(pers).firstFrame(); ++j)
            (*this)[pers].removeFirst();
        (*this)[pers].setFirstFrame(frame);
    }
    else if (direction == 0)
        removeAt(pers);
    else if (direction == 1)
    {
        for (j = 0; j < at(pers).lastFrame()-frame; ++j)
            (*this)[pers].removeLast();
        (*this)[pers].setLastFrame(frame);
    }

    return true;
}

// gibt true zurueck, wenn punkt geloescht werden konnte
// direction zeigt an, ob bis zum aktuellen (-1), ab dem aktuellen (1) oder ganzer trackpath (0)
// onlyVisible == -1 : immer alles betrachten, ansonsten nur person onlyVisible
// loescht trackpoint nur einer trajektorie
bool Tracker::delPoint(const Vec2F& p, int direction, int frame, QSet<int> onlyVisible)
{
    int i;

    for (i = 0; i < size(); ++i) // ueber TrackPerson
        if (((onlyVisible.empty()) || (onlyVisible.contains(i))) && (at(i).trackPointExist(frame) && (at(i).trackPointAt(frame).distanceToPoint(p) < mMainWindow->getHeadSize(NULL, i, frame)/2.))) // war: MIN_DISTANCE)) // 30 ist abstand zwischen kopfen
        {
            delPointOf(i, direction, frame);
            return true;
        }
    return false;
}

// direction zeigt an, ob bis zum aktuellen (-1), ab dem aktuellen (1) oder ganzer trackpath (0)
// loescht trackpoints aller trajektorien
void Tracker::delPointAll(int direction, int frame)
{
    int i, j;

    for (i = 0; i < size(); ++i) // ueber TrackPerson
    {
        if (at(i).trackPointExist(frame)) //
        {
            if (direction == -1)
            {
                for (j = 0; j < frame-at(i).firstFrame(); ++j)
                    (*this)[i].removeFirst();
                (*this)[i].setFirstFrame(frame);
            }
            else if (direction == 0)
                removeAt(i--); // nach Loeschen wird i um 1 erniedrigt
            else if (direction == 1)
            {
                for (j = 0; j < at(i).lastFrame()-frame; ++j)
                    (*this)[i].removeLast();
                (*this)[i].setLastFrame(frame);
            }
        }
        else if (((direction == -1) && (frame > at(i).lastFrame())) ||
                 (direction == 0) ||
                 ((direction == 1) && (frame < at(i).firstFrame())))
        {
            removeAt(i);
            i--;
        }
    }
}


// deletes points of a trajectrory, which are inside ROI
// 1 trajectory can end in 0, 1 or multiple trajectories!!!!!!!!
// man koennte noch unterscheiden, ob trajektorie aktuell in petrack zu sehen sein soll
void Tracker::delPointInsideROI()
{
    int i, j;
    QRectF rect = mMainWindow->getRecoRoiItem()->rect();
    bool inside;

    for (i = 0; i < size(); ++i) // ueber TrackPerson
    {
        inside = ((at(i).size() > 0) && rect.contains(at(i).at(0).x(), at(i).at(0).y()));
        for (j = 1; j < at(i).size(); ++j)
        {
            if (inside != rect.contains(at(i).at(j).x(), at(i).at(j).y())) // aenderung von inside
            {
                splitPerson(i, at(i).firstFrame()+j);
                if (inside)
                {
                    removeAt(i);
                    i--;
                    inside = !inside;
                }
                break;
            }
        }
        if (inside){
            // rest loeschen
            removeAt(i);
            i--;
        }
    }
}

// deletes trajectory, if it is partly inside ROI
// man koennte noch unterscheiden, ob trajektorie aktuell in petrack zu sehen sein soll
void Tracker::delPointROI()
{
    int i, j, anz=0;
    QRectF rect = mMainWindow->getRecoRoiItem()->rect();

    for (i = 0; i < size(); ++i) // ueber TrackPerson
    {
        for (j = 0; j < at(i).size(); ++j)
        {
            if (rect.contains(at(i).at(j).x(), at(i).at(j).y()))
            {
                anz++;
                removeAt(i);
                i--;
                break;
            }
        }
    }
    debout << "deleted "<<anz<<" trajectories!"<<endl;

//                for (i = 0; i < size(); ++i) // ueber TrackPerson
//                {
//                    for (j = 0; j < at(i).lastFrame(); ++j)
//                        (*this)[i].  removeLast();
//                    if (rect.contains(at(i).at(j).x(), at(i).at(j).y())


//                    if (at(i).trackPointExist(frame)) //
//                    {
//                        if (direction == -1)
//                        {
//                            for (j = 0; j < frame-at(i).firstFrame(); ++j)
//                                (*this)[i].removeFirst();
//                            (*this)[i].setFirstFrame(frame);
//                        }
//                        else if (direction == 0)
//                            removeAt(i--); // nach Loeschen wird i um 1 erniedrigt
//                        else if (direction == 1)
//                        {
//                            for (j = 0; j < at(i).lastFrame()-frame; ++j)
//                                (*this)[i].removeLast();
//                            (*this)[i].setLastFrame(frame);
//                        }
//                    }
//                    else if (((direction == -1) && (frame > at(i).lastFrame())) ||
//                             (direction == 0) ||
//                             ((direction == 1) && (frame < at(i).firstFrame())))
//                    {
//                        removeAt(i--); // nach Loeschen wird i um 1 erniedrigt
//                    }



//                    x = (*this)[i].first().x();
//                    y = (*this)[i].first().y();
//                    // mGrey hat gleiche groesse wie zuletzt getracktes bild
//                    if ((*this)[i].firstFrame() != 0 && x >= MAX(margin, rect.x()) && y >= MAX(margin, rect.y()) && x <= MIN(mGrey->width-1-2*bS-margin, rect.x()+rect.width()) && y <= MIN(mGrey->height-1-2*bS-margin, rect.y()+rect.height()))
//                    {
//                        debout << "Warning: Start of trajectory inside picture and recognition area of person " << i+1 << "!" << endl;
//                        pers.append(i+1);
//                        frame.append((*this)[i].firstFrame());
//                    }

//                    x = (*this)[i].last().x();
//                    y = (*this)[i].last().y();
//                    // mGrey hat gleiche groesse wie zuletzt getracktes bild
//                    if ((*this)[i].lastFrame() != lastFrame && x >= MAX(margin, rect.x()) && y >= MAX(margin, rect.y()) && x <= MIN(mGrey->width-1-2*bS-margin, rect.x()+rect.width()) && y <= MIN(mGrey->height-1-2*bS-margin, rect.y()+rect.height()))
//                    {
//                        debout << "Warning: End of trajectory inside picture and recognition area of person " << i+1 << "!" << endl;
//                        pers.append(i+1);
//                        frame.append((*this)[i].lastFrame());
//                    }



//                }


}
bool Tracker::editTrackPersonComment(const Vec2F& p, int frame, QSet<int> onlyVisible)
{
    int i;

    for (i = 0; i < size(); ++i) // ueber TrackPerson
        if (((onlyVisible.empty()) || (onlyVisible.contains(i))) && (at(i).trackPointExist(frame) && (at(i).trackPointAt(frame).distanceToPoint(p) < mMainWindow->getHeadSize(NULL, i, frame)/2.))) // war: MIN_DISTANCE)) // 30 ist abstand zwischen kopfen
        {
            bool ok;
            QString comment = QInputDialog::getText(mMainWindow,QObject::tr("Add Comment"),QObject::tr("Comment:"),
                                                    QLineEdit::Normal, at(i).comment() , &ok);
            if (ok)
            {
                if(comment.isEmpty())
                {
                    int ret = QMessageBox::warning(mMainWindow, QObject::tr("Empty comment"), QObject::tr("Are you sure you want to save an empty comment?"), QMessageBox::Save | QMessageBox::Cancel);
                    if( ret == QMessageBox::Cancel )
                    {
                        return false;
                    }
                }
                (*this)[i].setComment(comment);
                return true;
            }
        }
    return false;
}

bool Tracker::setTrackPersonHeight(const Vec2F& p, int frame, QSet<int> onlyVisible)
{
    int i;

    for (i = 0; i < size(); ++i) // ueber TrackPerson
    {
        if (((onlyVisible.empty()) || (onlyVisible.contains(i))) && (at(i).trackPointExist(frame) && (at(i).trackPointAt(frame).distanceToPoint(p) < mMainWindow->getHeadSize(NULL, i, frame)/2.))) // war: MIN_DISTANCE)) // 30 ist abstand zwischen kopfen
        {
            bool ok;

//            if (at(i).height() > MIN_HEIGHT)
            //QString comment = QInputDialog::getMultiLineText(mMainWindow, QObject::tr("Add Comment"),
            //                                                 QObject::tr("Comment:"), at(i).comment() , &ok);

            double col_height;
            // col_height is negative, if height is determined through color and not yet set manually
            if( at(i).height() < MIN_HEIGHT+1 )
                col_height = at(i).color().isValid() ? -mMainWindow->getControlWidget()->getColorPlot()->map(at(i).color()) : -mMainWindow->getControlWidget()->mapDefaultHeight->value();
            else
                col_height = at(i).height();


            double height = QInputDialog::getDouble(mMainWindow,QObject::tr("Set person height"),QObject::tr("Person height[cm]:"),
                                                    fabs(col_height), -500, 500, 1, &ok);
            if (ok)
            {
                if(height < 0)
                {
                    debout << "Warning: you entered a negative height!" << endl;// is not supported!" << endl;
                    //return false;
                }
                // if previous value (col_height) is negative, height was determined thru color. If manually set value is the color-map value, we do not change anything
                // @todo: @ar.graf: check if manually set values have side-effects (maybe do not show in statistics)
                if (!(col_height + height < 0.01))
                {
                    (*this)[i].setHeight(height);
                    return true;
                } else {
                    debout << endl << "No height change detected. Color-mapped height will remain set." << endl;
                }
            }
        }
    }
    return false;
}
bool Tracker::resetTrackPersonHeight(const Vec2F& p, int frame, QSet<int> onlyVisible)
{
    int i;

    for (i = 0; i < size(); ++i) // ueber TrackPerson
    {
        if (((onlyVisible.empty()) || (onlyVisible.contains(i))) && (at(i).trackPointExist(frame) && (at(i).trackPointAt(frame).distanceToPoint(p) < mMainWindow->getHeadSize(NULL, i, frame)/2.))) // war: MIN_DISTANCE)) // 30 ist abstand zwischen kopfen
        {

            (*this)[i].setHeight(MIN_HEIGHT);
            return true;

        }
    }
    return false;
}


// used for calculation of 3D point for all points in frame
// returns number of found points or -1 if no stereoContext available (also points without disp found are counted)
int Tracker::calcPosition(int frame)
{
    int anz = 0, notFoundDisp = 0;
    pet::StereoContext * sc = mMainWindow->getStereoContext();
    float x, y, z;

    if (sc)
    {
#ifndef STEREO_DISABLED
        // for every point of a person, which has already identified at this frame
        for (int i = 0; i < size(); ++i) // ueber TrackPerson
        {

            if (at(i).trackPointExist(frame))
            {
                ++anz;

                //TrackPoint *p = &(at(i).trackPointAt(frame));
                // ACHTUNG: BORDER NICHT BEACHTET bei p.x()...???
                // calculate height with disparity map
                if (sc->getMedianXYZaround((int) at(i).trackPointAt(frame).x(), (int) at(i).trackPointAt(frame).y(), &x, &y, &z)) // nicht myRound, da pixel 0 von 0..0.99 in double geht
                {
                    // hier kommt man nur hinein, wenn x, y, z Wert berechnet werden konnten
                    // statt altitude koennte hier irgendwann die berechnete Bodenhoehe einfliessen
                    (*this)[i][frame-at(i).firstFrame()].setSp(x, y, z); //setZdistanceToCam(z);
                    (*this)[i].setHeight(z, mMainWindow->getControlWidget()->coordAltitude->value());
                }
                else
                    ++notFoundDisp;
                //else // Meldung zu haeufig
                //    debout << "Warning: No disparity information for person " << i+1 << "." << endl;
            }
        }
        //if (notFoundDisp>0) // Meldung zu haeufig
        //    debout << "Warning: No disparity information found for " << (100.*notFoundDisp)/anz << " percent of points." << endl;
        return anz;
#endif
    }
    else
        return -1;
}

// true, if new traj is inserted with point p and initial frame frame
// p in pixel coord
// used from recognition and manual
bool Tracker::addPoint(TrackPoint &p, int frame, QSet<int> onlyVisible, int *pers)
{
    bool found = false;
    int i, iNearest;
    float x=-1, y=-1, z=-1;
    float scaleHead;
    float dist, minDist = 1000000.;

#ifndef STEREO_DISABLED
    // ACHTUNG: BORDER NICHT BEACHTET bei p.x()...
    // hier wird farbe nur bei reco bestimmt gegebenfalls auch beim tracken interessant
    // calculate height with disparity map
    if (mMainWindow->getStereoContext() && mMainWindow->getStereoWidget()->stereoUseForHeight->isChecked())
    {
        if (mMainWindow->getStereoContext()->getMedianXYZaround((int) p.x(), (int)p.y(), &x, &y, &z)) // nicht myRound, da pixel 0 von 0..0.99 in double geht
        {
            // statt altitude koennte hier irgendwann die berechnete Bodenhoehe einfliessen
            p.setSp(x, y, z); //setZdistanceToCam(z);
        }
        //cout << " " << p.x()<< " " << p.y() << " " << x << " " << y << " " << z <<endl;
        //if (i == 10)
        //    debout << i << " " << mMainWindow->getControlWidget()->coordAltitude->value() - z << " " << z << " " << (*this)[i].height() << endl;
    }
#endif
    // skalierungsfaktor fuer kopfgroesse
    // fuer multicolor marker groesser, da der schwarze punkt weit am rand liegen kann
    bool multiColorWithDot = false;
    if (mMainWindow->getControlWidget()->getRecoMethod() == 5 && // multicolor marker
        mMainWindow->getMultiColorMarkerWidget()->useDot->isChecked() && // nutzung von black dot
        !mMainWindow->getMultiColorMarkerWidget()->ignoreWithoutDot->isChecked()) // muetzen ohne black dot werden auch akzeptiert
    {
        multiColorWithDot = true;
        scaleHead = 1.3;
    }
    else
        scaleHead = 1.0;

    for (i = 0; i < size(); ++i) // !found &&  // ueber TrackPerson
    {
        if (((onlyVisible.empty()) || (onlyVisible.contains(i))) && at(i).trackPointExist(frame))
            //&& (!multiColorWithDot && (at(i).trackPointAt(frame).distanceToPoint(p) < scaleHead*mMainWindow->getHeadSize(NULL, i, frame)/2.)) &&
            //((mMainWindow->getControlWidget()->getRecoMethod() == 5 && // multicolor marker
            //  mMainWindow->getMultiColorMarkerWidget()->useDot->isChecked() && // nutzung von black dot
            //  !mMainWindow->getMultiColorMarkerWidget()->ignoreWithoutDot->isChecked()) && // auch reine farbige muetze wird akzeptiert
            // (at(i).trackPointAt(frame).colPoint().distanceToPoint(p.colPoint())  < scaleHead*mMainWindow->getHeadSize(NULL, i, frame)/2.))) // ist kaserne: 52/2 war: MIN_DISTANCE)) // 30 ist Abstand zwischen Koepfen
        {
//            if (i+1 == 14)
//            {
//                debout << i+1 << " " << multiColorWithDot << endl;
//                debout << at(i).trackPointAt(frame)<<endl;
//                debout << p << endl;
//                debout << mMainWindow->getHeadSize(NULL, i, frame)/2.<<endl;
//                debout << at(i).trackPointAt(frame).distanceToPoint(p)<<endl;
//                debout << at(i).trackPointAt(frame).distanceToPoint(p.colPoint())<<endl;

//            }
            dist = at(i).trackPointAt(frame).distanceToPoint(p);
            if (( //(!multiColorWithDot || !p.color().isValid()) && //|| !at(i).trackPointAt(frame).color().isValid()
                 dist < scaleHead*mMainWindow->getHeadSize(NULL, i, frame)/2.) ||
                // fuer multifarbmarker mit schwarzem punkt wird nur farbmarker zur Abstandbetrachtung herangezogen
                // at(i).trackPointAt(frame).colPoint() existiert nicht an dieser stelle, da bisher nur getrackt wurde!!!!
                (multiColorWithDot && p.color().isValid() && (at(i).trackPointAt(frame).distanceToPoint(p.colPoint()) < mMainWindow->getHeadSize(NULL, i, frame)/2.)))
            {
                if (found)
                {
                    debout << "Warning: more possible trackpoints for point" << endl;
                    debout << "         " << p << " in frame " << frame << " with low distance:" << endl;
                    debout << "         person " << i+1 << " (distance: " << dist << "), " << endl;
                    debout << "         person " << iNearest+1 << " (distance: " << minDist << "), " << endl;
                    if (minDist > dist)
                    {
                        minDist = dist;
                        iNearest = i;
                    }
                }
                else
                {
                    minDist = dist;
                    iNearest = i;
                    // WAR: break inner loop
                    found = true;
                }
            }
        }
    }
    if (found) // den naechstgelegenen nehmen
    {
        // test, if recognition point or tracked point is better is made in at(i).insertAtFrame
        if ((*this)[iNearest].insertAtFrame(frame, p, iNearest, (mMainWindow->getControlWidget()->trackExtrapolation->checkState() == Qt::Checked))) // wenn eingefuegt wurde (bessere qualitaet)
            //|| !at(i).trackPointAt(frame).color().isValid() moeglich, um auch bei schlechterer qualitaet aber aktuell nicht
            // vorliegender farbe die ermittelte farbe einzutragen - kommt nicht vor!
        {
            // set/add color
            if (p.color().isValid()) // not valid for manual, than old color is used
            {
                //if (at(i).trackPointAt(frame).color().isValid()) man koennte alte farbe abziehen - aber nicht noetig, kommt nicht vor
                (*this)[iNearest].addColor(p.color());
            }
        }

        if (pers != NULL)
            *pers = iNearest;
        (*this)[iNearest].setNewReco(true);
    }

    //--i;
    if ((onlyVisible.empty()) && !found)
    {
        iNearest = size(); //  << " (new) ";

        if (p.qual() > 100) //manual add
            p.setQual(100);
        append(TrackPerson(/*p.markerID()>0 ? p.markerID() :*/ 0, frame, p)); // 0 is person number/markerID; newReco is set to true by default
    }
    if ((z > 0) && ((onlyVisible.empty()) || found))
        (*this)[iNearest].setHeight(z, mMainWindow->getControlWidget()->coordAltitude->value()); // , frame
    if ((!onlyVisible.empty()) && !found)
        debout << "Warning: No manual insertion, because not all trajectories are visible!" <<endl;

    return !found;
}

// used from recognition
void Tracker::addPoints(QList<TrackPoint> &pL, int frame)
{
    int i;

    // reset newReco
    for (i = 0; i < size(); ++i) // ueber TrackPerson
        (*this)[i].setNewReco(false);

    // ueberprufen ob identisch mit einem Punkt in liste
    for (i = 0; i < pL.size(); ++i) // ueber PointList
    {
        addPoint(pL[i], frame, QSet<int>());
    }
}

// calculate height of person

// convert all trajectory coordinates in real coordinate (height needed)


int Tracker::visible(int frameNum)
{
    int i, anz = 0;
    for (i = 0; i < size(); ++i)
    {
        if (at(i).trackPointExist(frameNum))
            anz++;
    }
    return anz;
}

int Tracker::largestFirstFrame()
{
    int max = -1, i;
    for (i = 0; i < size(); ++i)
    {
        if (at(i).firstFrame() > max)
            max = at(i).firstFrame();
    }
    return max;
}
int Tracker::largestLastFrame()
{
    int max = -1, i;
    for (i = 0; i < size(); ++i)
    {
        if (at(i).lastFrame() > max)
            max = at(i).lastFrame();
    }
    return max;
}
int Tracker::smallestFirstFrame()
{
    int i, min = ((size()>0) ? at(0).firstFrame() : -1);
    for (i = 1; i < size(); ++i)
    {
        if (at(i).firstFrame() < min)
            min = at(i).firstFrame();
    }
    return min;
}
int Tracker::smallestLastFrame()
{
    int i, min = ((size()>0) ? at(0).lastFrame() : -1);
    for (i = 1; i < size(); ++i)
    {
        if (at(i).lastFrame() < min)
            min = at(i).lastFrame();
    }
    return min;
}

/**
 * @brief Tracker::calcPrevFeaturePoints calculates all featurePoints(Persons) from the "previous" frame
 * @param prevFrame Number of previous frame (can be both, larger or smaller; forward or backwards)
 * @param rect ROI
 * @param frame current frame number
 * @param reTrack boolean saying if people should be retracked, when tracking was of low quality
 * @param reQual threshold for low quality in case of reTrack = true
 * @param borderSize
 * @param onlyVisible
 * @return number of feature points
 */
size_t Tracker::calcPrevFeaturePoints(int prevFrame, Rect &rect, int frame, bool reTrack, int reQual, int borderSize, QSet<int> onlyVisible)
{
    int j = -1;

    mPrevFeaturePoints.clear();
    mPrevFeaturePointsIdx.clear();

    if (prevFrame != -1)
    {
        for (int i = 0; i < size(); ++i)
        {
            if (((onlyVisible.empty()) || (onlyVisible.contains(i))) &&
                at(i).trackPointExist(prevFrame) &&
                ((reTrack && at(i).trackPointExist(frame) && at(i).trackPointAt(frame).qual() < reQual) ||
                 !at(i).trackPointExist(frame)))
            {
                Point2f p2f(at(i).at(prevFrame-at(i).firstFrame()).x()+borderSize,
                            at(i).at(prevFrame-at(i).firstFrame()).y()+borderSize);
                if(rect.contains(p2f))
                {
                    mPrevFeaturePoints.push_back(p2f);
                    ++j;

                    mPrevFeaturePointsIdx.push_back(i);
                if (j > MAX_COUNT-2)
                {
                    debout << "Warning: reached maximal number of tracking point: " << MAX_COUNT << endl;
                    break; // for loop
                }
                }
            }
        }
    }

    return mPrevFeaturePointsIdx.size();
}

// rueckgabewert gibt anzahl an, wieviele punkte akzeptiert wurden,
//   um listen verschmelzen zu lassen, neu hinzugefuegt wurden, existierende mgl verbessert haben
/**
 * @brief Tries to insert the featurepoints into the trajectories. Might merge.
 *
 * If the point is in the original image (without border) and/or has a small tracking error
 * and is in the picture, it will be added to a tajectory, by using insertAtFrame, which will
 * only accept the point, if the quality increases. If merging is activated, we search for
 * another point, less then one head size away from the current one and merge these, if they werent
 * more distant to each other in neighbouring frames.
 *
 * @see Tracker::tryMergeTrajectories
 * @see TrackPerson::insertAtFrame
 *
 * @param frame Current frame
 * @param count number of tracked people
 * @param img current image
 * @param borderSize
 * @param errorScale
 * @return
 */
int Tracker::insertFeaturePoints(int frame, size_t count, Mat &img, int borderSize, float errorScale)
{
    int inserted = 0;
    TrackPoint v;
    int qual;
    bool found;
    Vec2F borderSize2F(-borderSize, -borderSize);
    int dist = (borderSize > 9) ? borderSize : 10; // abstand zum bildrand, ab wann warnung ueber trj verlust herausgeschrieben wird
    float x=-1, y=-1, z=-1;
    int borderColorGray = qGray(mMainWindow->getBorderFilter()->getBorderColR()->getValue(),
                                mMainWindow->getBorderFilter()->getBorderColG()->getValue(),
                                mMainWindow->getBorderFilter()->getBorderColB()->getValue());

    for (size_t i = 0; i < count; ++i)
    {
        if (mStatus[i])
        {
            v = Vec2F(mFeaturePoints.at(i).x,mFeaturePoints.at(i).y); // umwandlung nach TrackPoint bei "="

            // ausserhalb der groesse des originalbildes
            if ((v.x() >= borderSize && v.y() >= borderSize && v.x() <= img.cols-1-borderSize && v.y() <= img.rows-1-borderSize) ||
                (mTrackError[i] < errorScale*MAX_TRACK_ERROR)) // nur bei kleinem Fehler darf auch im Randbereich getrackt werden
            {
                // das Beschraenken auf die Bildgroesse ist reine sicherheitsmassnahme,
                // oft sind tracking paths auch ausserhalb des bildes noch gut,
                // aber beim tracken in die andere richtung kann es bei petrack probleme machen
                if (v.x() >= 0 && v.y() >= 0 && v.x() <= img.cols-1 && v.y() <= img.rows-1)
                {
                    // borderSize abziehen, da Trackerdaten am Rand des Originalbildes 0/0 ist
                    // set position relative to original image size
                    v += borderSize2F;

#ifndef STEREO_DISABLED
                    // ACHTUNG: BORDER NICHT BEACHTET bei p.x()...
                    // calculate height with disparity map
                    if (mMainWindow->getStereoContext() && mMainWindow->getStereoWidget()->stereoUseForHeight->isChecked())
                    {
                        mMainWindow->getStereoContext()->getMedianXYZaround((int) v.x(), (int)v.y(), &x, &y, &z);
                        {
                            v.setSp(x, y, z); //v.setZdistanceToCam(z);
                        }
                        //(*this)[i].setHeight(z, mMainWindow->getControlWidget()->coordAltitude->value(), frame);
                    }
#endif

//                     // wenn bei punkten, die nicht am rand liegen, der fehler gross ist,
//                     // wird geguckt, ob der sprung sich zur vorherigen richtung stark veraendert hat
//                     // wenn sprung sehr unterschiedlich, wird lieber interpoliert oder stehen geblieben
//                     // ist richtung ok, dann wird dunkelstes pixel gesucht
//                     // (subpixel aufgrund von nachbarpixel)
//                     // oder einfach bei schlechtem fehler mit groesserem winSize=30 den Problempunkt nochmal machen
                    // NOTE Wird gerade eben nicht gemacht. Sollten wir???


                    // ueberpruefen, ob tracking ziel auf anderem tracking path landet, dann beide trackpaths verschmelzen lassen
                    found = false;
                    if (mMainWindow->getControlWidget()->trackMerge->checkState() == Qt::Checked) // wenn zusammengefuehrt=merge=verschmolzen werden soll
                    {
                        found = tryMergeTrajectories(v, i, frame);
                    }

                    // wenn keine verschmelzung erfolgte, versuchen trackpoint einzufuegen
                    if (!found)
                    {
                        qual = static_cast<int>(errorToQual(mTrackError[i]));
                        if (qual < 20)
                            qual = 20;
                        v.setQual(qual); // qual um 50, damit nur reco-kopf-ellipsen points nicht herauskegeln
                        // bei insertAtFrame wird qual beruecksichtigt, ob vorheiger besser
                        if ((*this)[mPrevFeaturePointsIdx[i]].insertAtFrame(frame, v, mPrevFeaturePointsIdx[i], (mMainWindow->getControlWidget()->trackExtrapolation->checkState() == Qt::Checked)) && (z > 0))
                            (*this)[mPrevFeaturePointsIdx[i]].setHeight(z, mMainWindow->getControlWidget()->coordAltitude->value()); // , frame
                    }

                    ++inserted;
                }
            }
        }
        else
        {
            if (v.x() >= dist && v.y() >= dist && v.x() <= img.cols-1-dist && v.y() <= img.rows-1-dist)
                debout << "Warning: Lost trajectory inside picture of person " << mPrevFeaturePointsIdx[i]+1 << " at frame " << frame << "!" << endl;
        }
    }

    return inserted;
}

/**
 * @brief Tries to find a suitable other trajectory and merge
 *
 * @param v TrackPoint to be inserted
 * @param i Index in mFeaturePointsIdx and rest of point/person to be inserted
 * @param frame frame in which the point v was tracked
 * @return true if a suitable trajectory to merge with was found
 */
bool Tracker::tryMergeTrajectories(const TrackPoint& v, size_t i, int frame)
{
    int deleteIndex;
    bool found = false;
    int j;
    // nach trajektorie suchen, mit der eine verschmelzung erfolgen koennte
    for (j = 0; !found && j < size(); ++j) // ueber TrackPerson
    {
        if (j != mPrevFeaturePointsIdx[i] && at(j).trackPointExist(frame) && (at(j).trackPointAt(frame).distanceToPoint(v) < mMainWindow->getHeadSize(nullptr, j, frame)/2.))
        {
            // um ein fehltracking hin zu einer anderen Trajektorie nicht zum Verschmelzen dieser fuehren zu lassen
            // (die fehlerbehandlung durch interpolation wird in insertAtFrame durchgefuehrt)
            if (!((at(mPrevFeaturePointsIdx[i]).trackPointExist(frame-1) &&
                   (at(mPrevFeaturePointsIdx[i]).trackPointAt(frame-1).distanceToPoint(v) > mMainWindow->getHeadSize(nullptr, mPrevFeaturePointsIdx[i], frame-1)/2.)) ||
                  (at(mPrevFeaturePointsIdx[i]).trackPointExist(frame+1) &&
                   (at(mPrevFeaturePointsIdx[i]).trackPointAt(frame+1).distanceToPoint(v) > mMainWindow->getHeadSize(nullptr, mPrevFeaturePointsIdx[i], frame+1)/2.))))
            {
                if (at(j).firstFrame() < (*this)[mPrevFeaturePointsIdx[i]].firstFrame() &&
                    at(j).lastFrame()  > (*this)[mPrevFeaturePointsIdx[i]].lastFrame())
                {
                    for (int k = 0; k < at(mPrevFeaturePointsIdx[i]).size(); ++k)
                    {
                        // bei insertAtFrame wird qual beruecksichtigt, ob vorheriger besser
                        (*this)[j].insertAtFrame(at(mPrevFeaturePointsIdx[i]).firstFrame()+k, at(mPrevFeaturePointsIdx[i]).at(k), j, (mMainWindow->getControlWidget()->trackExtrapolation->checkState() == Qt::Checked));
                    }
                    deleteIndex=mPrevFeaturePointsIdx[i];
                }
                else if (at(j).firstFrame() < (*this)[mPrevFeaturePointsIdx[i]].firstFrame())
                {
                    for (int k = at(j).size()-1; k > -1; --k)
                    {
                        // bei insertAtFrame wird qual beruecksichtigt, ob vorheriger besser
                        (*this)[mPrevFeaturePointsIdx[i]].insertAtFrame(at(j).firstFrame()+k, at(j).at(k), mPrevFeaturePointsIdx[i], (mMainWindow->getControlWidget()->trackExtrapolation->checkState() == Qt::Checked));
                    }
                    deleteIndex=j;
                }
                else
                {
                    for (int k = 0; k < at(j).size(); ++k)
                    {
                        // bei insertAtFrame wird qual beruecksichtigt, ob vorheriger besser
                        (*this)[mPrevFeaturePointsIdx[i]].insertAtFrame(at(j).firstFrame()+k, at(j).at(k), mPrevFeaturePointsIdx[i], (mMainWindow->getControlWidget()->trackExtrapolation->checkState() == Qt::Checked));
                    }
                    deleteIndex=j;
                }
                removeAt(deleteIndex);

                // shift index of feature points
                for (size_t k = 0; k < mPrevFeaturePointsIdx.size(); ++k)
                    if (mPrevFeaturePointsIdx[k] > deleteIndex)
                        --mPrevFeaturePointsIdx[k];
                found = true;
            }
        }
    }

    return found;
}

// default: int winSize=10, int level=3
// winSize=3 ist genauer, aber kann auch leichter abgelenkt werden; winSize=30 ist robuster aber ungenauer
// level kann groesser gewaehlt werden, wenn winSize klein, macht aber keinen grossen unterschied; (0) waere ohne pyramide
// war , int winSize=10
/**
 * @brief Tracks points from the last frame in this (current) frame
 *
 * @param img Image of current frame
 * @param rect ROI in which tracking is executed
 * @param frame frame-number of the current frame
 * @param reTrack boolean saying if people should be retracked, when tracking was of low quality
 * @param reQual threshold for low quality in case of reTrack = true
 * @param borderSize
 * @param level level of Gauss-Pyramid that is used with Lucas-Kanade
 * @param onlyVisible Set of trajectories which should be evaluated; @see Petrack::getOnlyVisible
 * @param errorScaleExponent errorScale is 1.5^errorScaleExponent
 * @return Number of tracked points
 */
int Tracker::track(Mat &img,Rect &rect, int frame, bool reTrack, int reQual, int borderSize, int level, QSet<int> onlyVisible, int errorScaleExponent)
{
//    debout << "frame="<<frame<<" reTrack="<<reTrack<<" reQual="<<reQual<<" borderSize="<<borderSize<<" level="<<level<<" errorScaleExponent="<<errorScaleExponent << endl;
    int inserted = 0;
    QList<int> trjToDel;
    float errorScale = pow(1.5,errorScaleExponent); // 0 waere neutral

    if (mGrey.empty())
    {
        debout << "ERROR: you have to initialize tracking before using tracker!" <<endl;
        return -1;
    }

    if (img.empty())
    {
        debout << "ERROR: no NULL image allowed for tracking!" <<endl;
        return -1;
    }

    if ((mPrevFrame != -1) && (abs(frame - mPrevFrame) > MAX_STEP_TRACK))
        reset();

    if (img.channels() == 3)
    {
        cvtColor(img,mGrey,COLOR_BGR2GRAY);
    }
    else if (img.channels() == 1){
        img.copyTo(mGrey);
    }else{
        debout << "Error: Wrong number of channels: " << img.channels() <<endl;
        return -1;
    }

    size_t numOfPeopleToTrack = calcPrevFeaturePoints(mPrevFrame, rect, frame, reTrack, reQual, borderSize, onlyVisible);

    if (numOfPeopleToTrack > 0)
    {
        preCalculateImagePyramids(level);

        if (mPrevFrame != -1)
        {
            if (abs(frame - mPrevFrame) > MAX_STEP_TRACK)
                debout << "Warning: no tracking because of too many skipped frames (" << mPrevFrame << " to " << frame << ")!" << endl;
            else if (abs(frame - mPrevFrame) > 1)
                debout << "Warning: linear interpolation of skipped frames which are not already tracked (" << mPrevFrame << " to " << frame << ")." << endl; // will be done in insertFeaturePoints
        }

        trackFeaturePointsLK(level);
        refineViaColorPointLK(level, errorScale);

        BackgroundFilter *bgFilter = mMainWindow->getBackgroundFilter();
        // testen, ob Punkt im Vordergrund liegt, ansonsten, wenn nicht gerade zuvor detektiert, ganze trajektorie loeschen (maximnale laenge ausserhalb ist somit 2 frames)
        if (bgFilter && bgFilter->getEnabled() && (mPrevFrame != -1)) // nur fuer den fall von bgSubtraction durchfuehren
        {
            useBackgroundFilter(trjToDel, bgFilter);
        }

        // (bei schlechten, aber noch ertraeglichem fehler in der naehe dunkelsten punkt suchen)
        // dieser ansatz kann dazu fuehren, dass bei starken helligkeitsunterschieden auf pappe zum schatten gewandert wird!!!
        int recoMethod = mMainWindow->getControlWidget()->getRecoMethod(); // 0 == Kaserne, 1 == Hermes, 2 == Ohne, 3 == Color, 4 == Japan
        if (!mMainWindow->getStereoWidget()->stereoUseForReco->isChecked() && ((recoMethod == 0)|| (recoMethod == 1))) // nicht benutzen, wenn ueber disparity der kopf gesucht wird und somit kein marker vorhanden oder zumindest nicht am punkt lewigen muss
        {
            refineViaNearDarkPoint();
        }
        // NOTE Following comment can probably be deleted, quite old code -> commented out for long time
        /*
        // bei noch schlechteren punkten zweite strategie
        for (i = 0; i < count; ++i)
            if ((mTrackError[i] > MAX_TRACK_ERROR) || (mStatus[i] == 0))
                againNumber++;
        // wenn trackpoint zu grossen fehler haben oder gar nicht berechnet werden konnten,
        // dann wird mit dem zehnfachen der winSize (30) eine wiederholung durchgefuehrt
        if (againNumber)
        {
            // mgl besser einmal anlegen und immer wieder benutzen
            CvPoint2D32f* againPrevFeaturePoints = (CvPoint2D32f*) cvAlloc(againNumber*sizeof(CvPoint2D32f)); //points[0]
            CvPoint2D32f* againFeaturePoints = (CvPoint2D32f*) cvAlloc(againNumber*sizeof(CvPoint2D32f)); //points[1]
            char* againStatus = (char*) cvAlloc(againNumber);
            //againTrackError = (float*) cvAlloc(againNumber*sizeof(float));
            j=0;
            for (i = 0; i < count; ++i)
                if ((mTrackError[i] > MAX_TRACK_ERROR) || (mStatus[i] == 0))
                {
                    againPrevFeaturePoints[j] = mPrevFeaturePoints[i];
                    againFeaturePoints[j] = mFeaturePoints[i];
                    againStatus[j] = 0;
                    j++;
                }
            cvCalcOpticalFlowPyrLK(mPrevGrey, mGrey, mPrevPyramid, mPyramid,
                againPrevFeaturePoints, againFeaturePoints, againNumber, cvSize(winSize*10, winSize*10), level, againStatus, NULL,
                cvTermCriteria(CV_TERMCRIT_ITER|CV_TERMCRIT_EPS, 20, 0.03), CV_LKFLOW_PYR_A_READY|CV_LKFLOW_PYR_B_READY);
            j=0;
            for (i = 0; i < count; ++i)
                if ((mTrackError[i] > MAX_TRACK_ERROR) || (mStatus[i] == 0))
                {
                    Vec2F v = Vec2F(mFeaturePoints+i);
                    Vec2F w = Vec2F(againFeaturePoints+j);
                    debout << "--------------" << i << ", " << j << ": " << (int) againStatus[j] << " " << v.x() << " " <<v.y() << " " <<w.x() << " " << w.y() << " error: " << mTrackError[i] <<endl;
                    if ((againStatus[j] != 0))
                    {
                        mStatus[i] = 1;
                        mFeaturePoints[i] = againFeaturePoints[j];
                        // im anschluss koennte die position noch optimiert werden (dunkelster punkt in umgebung)
                    }
                    j++;

                }
            cvFree(&againPrevFeaturePoints);
            cvFree(&againFeaturePoints);
            cvFree(&againStatus);
        }
        */

        inserted = insertFeaturePoints(frame, numOfPeopleToTrack, img, borderSize, errorScale);
    }

    cv::swap(mPrevGrey, mGrey);

    mPrevFrame = frame;

    // delete vorher ausgewaehlte trj
    // ACHTUNG: einzige stelle in tracker, wo eine trj geloescht wird
    // trackNumberAll, trackShowOnlyNr werden nicht angepasst, dies wird aber am ende von petrack::updateimage gemacht
    for (int i = 0; i < trjToDel.size(); ++i) // ueber TrackPerson
    {
        removeAt(trjToDel[i]);
    }

    // numOfPeopleToTrack kann trotz nicht retrack > 0 sein auch bei alten pfaden
    // da am bildrand pfade keinen nachfolger haben und somit dort immer neu bestimmt werden!
    return static_cast<int>(numOfPeopleToTrack);
}

/**
 * @brief Calculates the image pyramids for Lucas-Kanade
 *
 * This functions calculates image pyramids together with the gradients for the
 * consumption by Lucas-Kanade. They are precomputed with the biggest winsize
 * so they have enough padding calcOpticalFlowPyrLK can use them for all used
 * winsizes
 *
 * @param level Maximum used level for Lucas-Kanade
 * @param numOfPeopleToTrack Number of people who are going to be tracked
 */
void Tracker::preCalculateImagePyramids(int level)
{
    int maxWinSize = 3;
    for(size_t i = 0; i < mPrevFeaturePointsIdx.size(); ++i){
        int winSize = mMainWindow->winSize(nullptr, mPrevFeaturePointsIdx[i], mPrevFrame, 0);
        if(winSize > maxWinSize){
            maxWinSize = winSize;
        }
    }

    cv::buildOpticalFlowPyramid(mPrevGrey, mPrevPyr, cv::Size(maxWinSize, maxWinSize), level);
    cv::buildOpticalFlowPyramid(mGrey, mCurrentPyr, cv::Size(maxWinSize,maxWinSize), level);
}

/**
 * @brief Tracks the mPrevFeaturePoints with Lucas-Kanade
 *
 * This function tracks all points in mPrevFeaturePoints via Lucas-Kanade. Each person is
 * tracked with a different winSize according to the size of the head.
 *
 * @param level Maximum level to track with
 * @param numOfPeopleToTrack Number of people to be tracked
 * @see Tracker::calcPrevFeaturePoints
 */
void Tracker::trackFeaturePointsLK(int level)
{
    int winSize;
    for (size_t i = 0; i < mPrevFeaturePointsIdx.size(); ++i)
    {
        // das Durchlaufen der level bis 0 fuer den Fall, dass kein Tracking moeglich ist (mStatus ==0)
        //     ist erst ab opencv 3 hinzugenommen worden, da es manchmal zum abbruch kam, obwohl ein tracking moeglich sein sollte
        //     ein verdacht: wenn sich in folgebildern kein eindeutiger punkt ergibt, da in der kleinsten pyr stufe rundherum die exakt gleiche farbe/graustufe vorherscht
        int l = level;

        do
        {
            winSize = mMainWindow->winSize(nullptr, mPrevFeaturePointsIdx[i], mPrevFrame, l);
            if (winSize < MIN_WIN_SIZE)
            {
                winSize = MIN_WIN_SIZE;
                debout << "Warning: set search region to the minimum size of "<<MIN_WIN_SIZE<<" for person " << mPrevFeaturePointsIdx[i] << "!" << endl;
            }

            if (l < level)
                debout << "Warning: try tracking person " /*<< mPrevFeaturePointsIdx[i]*/ << " with pyramid level " << l<<"!" << endl;

            cv::calcOpticalFlowPyrLK(mPrevPyr,mCurrentPyr,/*points[0]*/mPrevFeaturePoints,/*points[1]*/mFeaturePoints,mStatus,mTrackError,Size(winSize,winSize),l,mTermCriteria);

        }  while(mStatus[i] == 0 && (l--) > 0);

        mTrackError[i] = mTrackError[i]*10.F/winSize;
    }
}

/**
 * @brief Tries to track colorPoint when featurePoint has high error
 * @param level Pyramidlevel to track with
 * @param numOfPeopleToTrack
 * @param errorScale Factor for highest tolerable tracking error
 */
void Tracker::refineViaColorPointLK(int level, float errorScale)
{
    int winSize;
    bool useColor = mMainWindow->getMultiColorMarkerWidget()->useColor->isChecked();
    vector<Point2f> prevColorFeaturePoint, colorFeaturePoint;
    vector<uchar> colorStatus;
    vector<float> colorTrackError;

    for (size_t i = 0; i < mPrevFeaturePointsIdx.size(); ++i)
    {
        // wenn fehler zu gross, dann Farbmarkerelement nehmen // fuer multicolor marker / farbiger hut mit schwarzem punkt
        if ( useColor && mTrackError[i]>errorScale*150.F && at(mPrevFeaturePointsIdx[i]).at(mPrevFrame-at(mPrevFeaturePointsIdx[i]).firstFrame()).color().isValid())
        {
            float prevPointX = static_cast<float>(at(mPrevFeaturePointsIdx[i]).at(mPrevFrame-at(mPrevFeaturePointsIdx[i]).firstFrame()).colPoint().x());
            float prevPointY = static_cast<float>(at(mPrevFeaturePointsIdx[i]).at(mPrevFrame-at(mPrevFeaturePointsIdx[i]).firstFrame()).colPoint().y());
            prevColorFeaturePoint.push_back(Point2f(prevPointX, prevPointY));
            winSize = mMainWindow->winSize(nullptr, mPrevFeaturePointsIdx[i], mPrevFrame, level);

            cv::calcOpticalFlowPyrLK(mPrevPyr,mCurrentPyr,prevColorFeaturePoint,colorFeaturePoint,colorStatus,colorTrackError,Size(winSize,winSize),level,mTermCriteria);

            colorTrackError[i] = colorTrackError[i]*10.F/winSize;

            if ((colorStatus[i] == 1) && (colorTrackError[i] < errorScale*50.F))
            {
                debout << "Warning: tracking color marker instead of structural marker of person "<< mPrevFeaturePointsIdx[i]+1 <<" at " << mFeaturePoints[i].x << " x " << mFeaturePoints[i].y
                        << " / error: " << mTrackError[i] << " / color error: " << colorTrackError[i] <<endl;


                mFeaturePoints[i] = Point2f(mPrevFeaturePoints[i].x+(colorFeaturePoint[i].x-prevColorFeaturePoint[i].x),
                                            mPrevFeaturePoints[i].x+(colorFeaturePoint[i].x-prevColorFeaturePoint[i].x));
                debout << "         resulting point: " << mFeaturePoints[i].x << " x " << mFeaturePoints[i].y << endl;
                mTrackError[i] = colorTrackError[i];
            }
        }
    }
}

void Tracker::useBackgroundFilter(QList<int>& trjToDel, BackgroundFilter *bgFilter){
    int x, y;
    static int margin=10; // rand am bild, ab dem trajectorie in den hintergrund laufen darf
    int bS = mMainWindow->getImageBorderSize();
    QRectF rect = mMainWindow->getRecoRoiItem()->rect();
    for (size_t i = 0; i < mPrevFeaturePointsIdx.size(); ++i)
    {
        x = myRound(mFeaturePoints[i].x-.5);
        y = myRound(mFeaturePoints[i].y-.5);

        // Rahmen, in dem nicht vordergrund pflicht, insbesondere am rechten rand!!!! es wird gruenes von hand angelegtes bounding rect roi genutzt
        if ((mStatus[i] == 1) &&
                x >= MAX(margin, rect.x()) && x <= MIN(mGrey.cols-1-2*bS-margin-50, rect.x()+rect.width()) &&
                y >= MAX(margin, rect.y()) && y <= MIN(mGrey.rows-1-2*bS-margin, rect.y()+rect.height()))
        {
            if (!bgFilter->isForeground(x, y) && at(mPrevFeaturePointsIdx[i]).trackPointAt(mPrevFrame).qual() < 100)
            {
                if ((mMainWindow->getControlWidget()->filterBgDeleteTrj->checkState() == Qt::Checked) &&
                    (at(mPrevFeaturePointsIdx[i]).nrInBg() >= mMainWindow->getControlWidget()->filterBgDeleteNumber->value()))
                {
                    // nur zum loeschen vormerken und am ende der fkt loeschen, da sonst Seiteneffekte komplex
                    trjToDel+= mPrevFeaturePointsIdx[i];
                    debout << "Warning: Delete trajectory " << mPrevFeaturePointsIdx[i]+1 << " inside region of interest, because it laid outside foreground for " <<
                              mMainWindow->getControlWidget()->filterBgDeleteNumber->value() << " successive frames!" << endl;
                }
                else
                {
                    (*this)[mPrevFeaturePointsIdx[i]].setNrInBg(at(mPrevFeaturePointsIdx[i]).nrInBg()+1);
                }
            }
            else // zaehler zuruecksetzen, der anzahl von getrackten Punkten im hintergrund zaehlt
            {
                (*this)[mPrevFeaturePointsIdx[i]].setNrInBg(0);
            }
        }
    }
}

/**
 * @brief Tracks a near dark point if error is (still) high
 *
 * This method is only used with the Casern and Hermes Markers, which both have
 * an black middle. The target of this method is to find this dark point in the
 * middle of the marker and track that instead of the feature point.
 *
 * @param numOfPeopleTracked Number of people who have been tracked
 */
void Tracker::refineViaNearDarkPoint()
{
    int x, y;
    for (int i = 0; i < mPrevFeaturePointsIdx.size(); ++i)
    {
        x = myRound(mFeaturePoints[i].x-.5);
        y = myRound(mFeaturePoints[i].y-.5);
        // der reine fehler ist leider kein alleinig gutes mass,
        // da in kontrastarmen regionen der angegebene fehler gering, aber das resultat haeufiger fehlerhaft ist
        // es waere daher schoen, wenn der fehler in abhaengigkeit von kontrast in umgebung skaliert wuerde
        // zb (max 0..255): normal 10..150 -> *1; klein 15..50 -> *3; gross 0..255 -> *.5
        if ((mTrackError[i] > MAX_TRACK_ERROR) && (mStatus[i] == 1) &&
                x >= 0 && x < mGrey.cols && y >= 0 && y < mGrey.rows)
        {
            int regionSize = myRound(mMainWindow->getHeadSize(nullptr, mPrevFeaturePointsIdx[i], mPrevFrame)/10.); ///< size of searched region around point: -regionSize to regionSize
            int xMin, xMax, yMin, yMax, xMin2, xMax2, yMin2, yMax2, darkest;
            bool markerInsideWhite = true;
            int xDark = x, yDark = y;

            // trotz grau (img)->nChannels=3
            xMin = ((0 > (x-regionSize)) ? 0 : (x-regionSize));
            yMin = ((0 > (y-regionSize)) ? 0 : (y-regionSize));
            xMax = ((mGrey.cols  < (x+regionSize+1)) ? mGrey.cols  : (x+regionSize+1));
            yMax = ((mGrey.rows < (y+regionSize+1)) ? mGrey.rows : (y+regionSize+1));

            darkest = 255;
            for (int k = yMin; k < yMax; ++k)
            {
                for (int j = xMin; j < xMax; ++j)
                {
                    if (getValue(mGrey, j, k).value() < darkest)
                    {
                        darkest = getValue(mGrey, j, k).value();
                        xDark = j;
                        yDark = k;
                    }
                }
            }

            xMin2 = ((0 > (xDark-regionSize)) ? 0 : (xDark-regionSize));
            yMin2 = ((0 > (yDark-regionSize)) ? 0 : (yDark-regionSize));
            xMax2 = ((mGrey.cols  < (xDark+regionSize+1)) ? mGrey.cols  : (xDark+regionSize+1));
            yMax2 = ((mGrey.rows < (yDark+regionSize+1)) ? mGrey.rows : (yDark+regionSize+1));

            // suchbereich:
            //  ###
            // #   #
            // #   #
            // #   #
            //  ###
            for (int k = yMin2+1; k < yMax2-1; ++k)
            {
                if ((getValue(mGrey, xMin2, k).value() <= darkest) || (getValue(mGrey, xMax2-1, k).value() <= darkest))
                {
                    markerInsideWhite = false;
                    break;
                }
            }
            if (markerInsideWhite)
                for (int j = xMin2+1; j < xMax2-1; ++j)
                {
                    if ((getValue(mGrey, j, yMin2).value() <= darkest) || (getValue(mGrey, j, yMax2-1).value() <= darkest))
                    {
                        markerInsideWhite = false;
                        break;
                    }
                }

            if (markerInsideWhite)
            {
                mFeaturePoints[i].x = xDark;
                mFeaturePoints[i].y = yDark;
                debout << "Move trackpoint to darker pixel for" << i+1 << "!" << endl;
            }

            // interpolation wg nachbargrauwerten:
            x = myRound(mFeaturePoints[i].x);
            y = myRound(mFeaturePoints[i].y);
            if ((x>0) && (x<(mGrey.cols-1)) && (y>0) && (y<(mGrey.rows-1)) && (darkest<255))
            {
                mFeaturePoints[i].x += .5*((double)(getValue(mGrey, x+1, y).value()-getValue(mGrey, x-1, y).value()))/((double)(255-darkest));
                mFeaturePoints[i].y += .5*((double)(getValue(mGrey, x, y+1).value()-getValue(mGrey, x, y-1).value()))/((double)(255-darkest));
            }

            mFeaturePoints[i].x += .5F;
            mFeaturePoints[i].y += .5F; // da 1. pixel von 0..1, das 2. pixel von 1..2 etc geht
        }
    }
}

void Tracker::recalcHeight(float altitude)
{
    // in TrackPerson: resetHeight();
    for (int i = 0; i < size(); ++i) // ueber TrackPerson
    {
        (*this)[i].recalcHeight(altitude);
    }
}

void Tracker::checkPlausibility(QList<int> &pers, QList<int> &frame,
                                bool testEqual, bool testVelocity, bool testInside, bool testLength)
    {
    QProgressDialog progress("Check Plausibility",NULL,0,400,mMainWindow->window());
    progress.setWindowTitle("Check plausibility");
    progress.setWindowModality(Qt::WindowModal);
    progress.setVisible(true);
    progress.setValue(0);
    progress.setLabelText("Check Plausibility...");
    static int margin=30; // rand am bild, ab dem trajectorie verloren sein darf
    int i, j;
    double x, y;
    int bS = mMainWindow->getImageBorderSize();
    QRectF rect = mMainWindow->getRecoRoiItem()->rect();
    int lastFrame = mMainWindow->getAnimation()->getNumFrames()-1;

#ifdef TIME_MEASUREMENT
    double time1, tstart;
#endif
    // test, if the trajectory is very short (less than 10 Trackpoints)
    if (testLength)
    {
        progress.setValue(0);
        progress.setLabelText("Check trajectories lengths...");
        qApp->processEvents();
#ifdef TIME_MEASUREMENT
        time1 = 0.0;
        tstart = clock();
#endif
        for (i = 0; i < size(); ++i) // ueber TrackPerson
        {
            progress.setValue(i*100./size());
            qApp->processEvents();
            if (at(i).size() < 10)
            {
                debout << "Warning: Trajectory of person " << i+1 << " has less than 10 trackpoints!" << endl;
                pers.append(i+1);
                frame.append((*this)[i].firstFrame());
            }
        }
#ifdef TIME_MEASUREMENT
        time1 += clock() - tstart;
        time1 = time1/CLOCKS_PER_SEC;
        cout << "  time(testLength) = " << time1 << " sec." << endl;
#endif
    }

//     debout << rect.x() << " " <<rect.y() << " " <<rect.width() << " " <<rect.height() << " " <<endl;
    // check, if trajectory starts and ends outside the recognition area
    if (testInside)
    {
        progress.setValue(100);
        progress.setLabelText("Check if trajectories are inside image...");
        qApp->processEvents();
#ifdef TIME_MEASUREMENT
        time1 = 0.0;
        tstart = clock();
#endif
        for (i = 0; i < size(); ++i) // ueber TrackPerson
        {
            qApp->processEvents();
            progress.setValue(100+i*100./size());
            //         if (i == 11)
            //         {
            //             debout << (*this)[i].first().x() << " " << (*this)[i].first().y() <<endl;
            //             debout << MAX(margin, rect.x()) << " " << MAX(margin, rect.y()) << " " << MIN(mGrey->width-1-2*bS-margin, rect.x()+rect.width()) << " " << MIN(mGrey->height-1-2*bS-margin, rect.y()+rect.height())<<endl;
            //         }
            x = (*this)[i].first().x();
            y = (*this)[i].first().y();
            // mGrey hat gleiche groesse wie zuletzt getracktes bild
            if ((*this)[i].firstFrame() != 0 && x >= MAX(margin, rect.x()) && y >= MAX(margin, rect.y()) && x <= MIN(mGrey.cols-1-2*bS-margin, rect.x()+rect.width()) && y <= MIN(mGrey.rows-1-2*bS-margin, rect.y()+rect.height()))
            {
                debout << "Warning: Start of trajectory inside picture and recognition area of person " << i+1 << "!" << endl;
                pers.append(i+1);
                frame.append((*this)[i].firstFrame());
            }

            x = (*this)[i].last().x();
            y = (*this)[i].last().y();
            // mGrey hat gleiche groesse wie zuletzt getracktes bild
            if ((*this)[i].lastFrame() != lastFrame && x >= MAX(margin, rect.x()) && y >= MAX(margin, rect.y()) && x <= MIN(mGrey.cols-1-2*bS-margin, rect.x()+rect.width()) && y <= MIN(mGrey.rows-1-2*bS-margin, rect.y()+rect.height()))
            {
                debout << "Warning: End of trajectory inside picture and recognition area of person " << i+1 << "!" << endl;
                pers.append(i+1);
                frame.append((*this)[i].lastFrame());
            }
        }
#ifdef TIME_MEASUREMENT
        time1 += clock() - tstart;
        time1 = time1/CLOCKS_PER_SEC;
        cout << "  time(testInside) = " << time1 << " sec." << endl;
#endif
    }

    // testen, ob grosse Geschwindigkeitsaenderungen
    // statt distanz koennte man auch noch vektoren vergleichen, was genauere analyse waer!!!!
    if (testVelocity)
    {
        qApp->processEvents();
        progress.setValue(200);
        progress.setLabelText("Check velocity...");
#ifdef TIME_MEASUREMENT
        time1 = 0.0;
        tstart = clock();
#endif
        double d01, d12, d23;
        for (i = 0; i < size(); ++i) // ueber TrackPerson
        {
            qApp->processEvents();
            progress.setValue(200+i*100./size());
            for (j = 1; j < at(i).size()-2; ++j) // ueber TrackPoint (ohne ersten und letzten beiden)
            {
                d01 = at(i).at(j  ).distanceToPoint(at(i).at(j-1));
                d12 = at(i).at(j+1).distanceToPoint(at(i).at(j  ));
                d23 = at(i).at(j+2).distanceToPoint(at(i).at(j+1));
                if (((1.8*(d01+d23)/2.) < d12) && ((d12 > 6.) || ((d01+d23)/2. > 3.))) // geschwindigkeit 1,8-fach && mindestpixelbewegung im schnitt von 3
                {
                    debout << "Warning: Fast variation of velocity of person " << i+1 << " between frame " << j+at(i).firstFrame() << " and " << j+1+at(i).firstFrame() << "!" << endl;
                    pers.append(i+1);
                    frame.append(j+at(i).firstFrame());
                }
            }
        }
#ifdef TIME_MEASUREMENT
        time1 += clock() - tstart;
        time1 = time1/CLOCKS_PER_SEC;
        cout << "  time(testVelocity) = " << time1 << " sec." << endl;
#endif
    }

    // testen, ob zwei trackpoint sehr nah beieinanderliegen (es gibt trajektorien, die uebereinander liegen, wenn nicht genmergt wird)
    if (testEqual)
    {
        progress.setValue(300);
        progress.setLabelText("Check if trajectories are equal...");
        qApp->processEvents();
#ifdef TIME_MEASUREMENT
        time1 = 0.0;
        tstart = clock();
#endif
        int lLF = largestLastFrame();
        int f;
        for (f = smallestFirstFrame(); f <= lLF; ++f)
        {
            progress.setValue(300+f*100./lLF);
            qApp->processEvents();

            for (i = 0; i < size(); ++i)
            {
                  // if (!pers.contains(i+1)) man koennte nur einmal eine Person aufnehmen, da aufeinanderfolgende frames oft betroffen
                for (j = i+1; j < size(); ++j)
                {
                    if (at(i).trackPointExist(f) && at(j).trackPointExist(f))
                    {
                        if (at(i).trackPointAt(f).distanceToPoint(at(j).trackPointAt(f)) < mMainWindow->getHeadSize(NULL, i, f)/2.)
                        {
                            debout << "Warning: Person " << i+1 << " and " << j+1 << " are very close to each other at frame " << f << "!" << endl;
                            pers.append(i+1);
                            frame.append(f);
                        }
                    }
                }
            }
        }
#ifdef TIME_MEASUREMENT
        time1 += clock() - tstart;
        time1 = time1/CLOCKS_PER_SEC;
        cout << "  time(testEqual) = " << time1 << " sec." << endl;
#endif
    }
}

// color optimieren fuer alle personen
void Tracker::optimizeColor()
{
    int i;
    for (i = 0; i < size(); ++i) // ueber TrackPerson
    {
        if ((*this)[i].color().isValid())
            (*this)[i].optimizeColor();
    }
}

// reset the height of all persons, but not the pos of the trackpoints
void Tracker::resetHeight()
{
    for (int i = 0; i < size(); ++i) // ueber TrackPerson
    {
        (*this)[i].resetHeight();
    }
}

// reset the pos of the tzrackpoints, but not the heights
void Tracker::resetPos()
{
    for (int i = 0; i < size(); ++i) // ueber TrackPerson
        for (int j = 0; j < (*this)[i].size(); ++j) // ueber TrackPoints
            (*this)[i][j].setSp(-1., -1., -1.);
}

// gibt groessenverteilung der personen auf stdout aus
// rueckgabewert false wenn keine hoeheninformationen in tracker datensatz vorliegt
bool Tracker::printHeightDistribution()
{
    debout << endl;
    QMap<double, int> dict;
    QMap<double, int>::const_iterator j;
    int i, anz = 0;
    int heightStep = 5;
    double average = 0., avg = 0.;
    int noHeight = 0;

    for (i = 0; i < size(); ++i)
    {
        if ((*this)[i].height() > MIN_HEIGHT) // !=-1// insbesondere von hand eingefuegte trackpoint/persons haben keine farbe
        {
            ++dict[(((int) (*this)[i].height())/heightStep)*heightStep];
            avg += (*this)[i].height();
        }
        else
            ++noHeight;
    }
    j = dict.constBegin();
    while (j != dict.constEnd()) {
        anz += j.value();
        ++j;
    }
    //debout << "Attention: persons with height lower 0 are ignored!" << endl;
    debout << "number of persons with measured height                            : " << anz << endl;
    debout << "person without measured height (not included in calculated values): " << noHeight << " (using default height for export)" << endl;
    if (anz == 0)
        return false;
    j = dict.constBegin();
    while (j != dict.constEnd()) {
        debout << "height " << fixed << setprecision(1) << setw(5) << j.key() << " - " << j.key()+heightStep << " : number " << setw(3) << j.value() << " (" << setw(4) << (100.*j.value())/anz << "%)" << endl;
        average+=(j.key()+heightStep/2.)*j.value();
        ++j;
    }
    debout << "average height (bucket): " << fixed << setprecision(1) << setw(5) << average/anz << endl;
    debout << "average height         : " << fixed << setprecision(1) << setw(5) << avg/anz << endl;

    return true;
}

void Tracker::purge(int frame)
{
    int i, j;
    float count; // anzahl der trackpoints, an denen die person nicht gefunden wurde

    for (i = 0; i < size(); ++i)
    {
        if (at(i).size() > 10 && at(i).firstFrame() <= frame && at(i).lastFrame() >= frame)
        {
            count = 0;
            for (j = 0; j < at(i).size(); ++j)
            {
                if (at(i).at(j).qual() < 100.)
                    ++count;
                //debout << i << " " << j << " " << at(i).at(j).qual() <<endl;
            }
            if (count/at(i).size() > 0.8) // Achtung, wenn roi klein, dann viele tp nur getrackt
                removeAt(i); // delete trj
        }

        // wenn x/y/z gesetzt ist, dann koennte auch eine hoehenschwankung zum eliminieren fuehren

//        if ((*this)[i].height() > MIN_HEIGHT) // !=-1// insbesondere von hand eingefuegte trackpoint/persons haben keine farbe
//        {
//            ++dict[(((int) (*this)[i].height())/heightStep)*heightStep];
//            avg += (*this)[i].height();
//        }
//        else
//            ++noHeight;
    }
}

// Brief: Synchronize TrackPoint.mMarkerID with TrackPerson.mMarkerID
// set PersonMarkerID from TrackPointMarkerID if MarkerID == -1 and check if not other "real ID" was detected - trigger warning otherwise
// Zu entscheiden: nutzung als funktion mit input+output oder als aufruf mit (*this)[i]
// wie setTrackPersonHeight() ????
void Tracker::syncTrackPersonMarkerID(QList<TrackPoint> &pL) // usage of &pL für PointList oder &p für Point ???
{
    for (int i = 0; i < size(); ++i) // ueber TrackPerson
    {
        if (TrackPoint.mMarkerID[i]!=-1) // first time a Person is found but marker is not detected
        {
            TrackPerson.mMarkerID = TrackPoint.mMarkerID;
        }
        // else if (TrackPerson.mMarkerID == TrackPoint.mMarkerID){// do nothin all is fine}
        else if (TrackPerson.mMarkerID != TrackPoint.mMarkerID)
        {
            cout << "ERROR: Two MarkerIDs were found for one trajectory." << endl;
        }
    }
}










