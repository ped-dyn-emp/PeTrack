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

#ifndef AUTOSAVE_H
#define AUTOSAVE_H

#include <QObject>
#include <QStringList>
#include <memory>

class Petrack;
class QTimer;
class QFileInfo;

struct AutosaveFilenames
{
    QString running;
    QString final;
};

class Autosave : public QObject
{
    Q_OBJECT
public:
    explicit Autosave(Petrack &petrack);
    Autosave()                                 = delete;
    Autosave(const Autosave &other)            = delete;
    Autosave(const Autosave &&other)           = delete;
    Autosave &operator=(const Autosave &other) = delete;
    Autosave &operator=(Autosave &&other)      = delete;
    ~Autosave() override                       = default;

    void        trackPersonModified();
    void        resetTrackPersonCounter();
    static bool autosaveExists(const QString &filename);
    void        deleteAutosave();
    void        loadAutosave();
    bool        isAutosave(const QString &file);

private:
    static QString           buildAutosaveName(const QString &projectFileName, const QString &ending);
    static AutosaveFilenames autosaveNamesTrc(const QString &projectFileName);
    static AutosaveFilenames autosaveNamesPet(const QString &projectFileName);
    void                     saveTrc();
    QStringList              getAutosave();
    static QStringList       getAutosave(const QFileInfo &projectPath);

private slots:
    void savePet();

private:
    static constexpr int petSaveInterval     = 120 * 1000; // in ms -> 120s
    static constexpr int changesTillAutosave = 10;

    void startTimer();
    void stopTimer();

    Petrack &mPetrack;
    QTimer  *mTimer;
    int      mChangeCounter = 0;
};

#endif // AUTOSAVE_H
