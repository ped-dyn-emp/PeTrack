/*
 * PeTrack - Software for tracking pedestrians movement in videos
 * Copyright (C) 2010-2022 Forschungszentrum Jülich GmbH,
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

#ifndef PERSONSTORAGE_H
#define PERSONSTORAGE_H

#include "frameRange.h"
#include "tracker.h"

#include <vector>

class Petrack;
class Autosave;

struct PersonFrame
{
    int personID;
    int frame;
};

class PersonStorage
{
public:
    enum class Direction
    {
        Previous,
        Following,
        Whole
    };

    explicit PersonStorage(Petrack &mainWindow, Autosave &autosave) : mMainWindow(mainWindow), mAutosave(autosave) {}

    void splitPerson(size_t pers, int frame);
    bool splitPersonAt(const Vec2F &p, int frame, const QSet<int> &onlyVisible);
    bool delPointOf(int pers, int direction, int frame);
    bool delPoint(const Vec2F &p, int direction, int frame, const QSet<int> &onlyVisible);
    void delPointAll(Direction direction, int frame);
    void delPointROI();
    void delPointInsideROI();
    bool editTrackPersonComment(const Vec2F &p, int frame, const QSet<int> &onlyVisible);
    bool setTrackPersonHeight(const Vec2F &p, int frame, const QSet<int> &onlyVisible);
    bool resetTrackPersonHeight(const Vec2F &p, int frame, QSet<int> onlyVisible);
    void moveTrackPoint(int personID, int frame, const Vec2F &newPosition);

    size_t                          nbPersons() const { return mPersons.size(); }
    const TrackPerson              &at(size_t i) const { return mPersons.at(i); }
    void                            addPerson(const TrackPerson &person) { mPersons.push_back(person); }
    const std::vector<TrackPerson> &getPersons() const { return mPersons; }

    // used for calculation of 3D point for all points in frame
    // returns number of found points or -1 if no stereoContext available (also points without disp found are counted)
    int calcPosition(int frame);

    // true, if new traj is inserted with point p and initial frame frame
    // p in pixel coord
    // pers wird gesetzt, wenn existierender trackpoint einer person verschoben wird
    bool addPoint(
        TrackPoint             &p,
        int                     frame,
        const QSet<int>        &onlyVisible,
        reco::RecognitionMethod method,
        int                    *pers = nullptr);

    // hier sollte direkt die farbe mit uebergeben werden
    void addPoints(QList<TrackPoint> &pL, int frame, reco::RecognitionMethod method);
    int  visible(int frameNum) const;
    int  largestFirstFrame() const;
    int  largestLastFrame() const;
    int  smallestFirstFrame() const;
    [[nodiscard]] std::vector<PersonFrame>
         getProximalPersons(const QPointF &pos, QSet<int> selected, const FrameRange &frameRange) const;
    void recalcHeight(float altitude);

    void clear() { mPersons.clear(); }

    void smoothHeight(size_t i, int j);

    void insertFeaturePoint(
        size_t            person,
        int               frame,
        const TrackPoint &point,
        int               persNr,
        bool              extrapolate,
        float             z,
        float             height);
    int merge(int pers1, int pers2);


    void checkPlausibility(
        QList<int> &pers,
        QList<int> &frame,
        bool        testEqual    = true,
        bool        testVelocity = true,
        bool        testInside   = true,
        bool        testLength   = true);

    void optimizeColor();

    // reset the height of all persons, but not the pos of the trackpoints
    void resetHeight();

    // reset the pos of the tzrackpoints, but not the heights
    void resetPos();

    // gibt groessenverteilung der personen auf stdout aus
    // rueckgabewert false wenn keine hoeheninformationen in tracker datensatz vorliegt
    bool printHeightDistribution();
    void setMarkerHeights(const std::unordered_map<int, float> &heights);
    void setMarkerID(int person, int markerIDs, bool manual = false);
    void setMarkerIDs(const std::unordered_map<int, int> &markerIDs);
    void purge(int frame);

    void setNrInBg(size_t idx, int nr) { mPersons[idx].setNrInBg(nr); }

private:
    std::vector<TrackPerson> mPersons;
    Petrack                 &mMainWindow;
    Autosave                &mAutosave;
};

#endif // PERSONSTORAGE_H
