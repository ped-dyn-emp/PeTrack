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

#include "autosave.h"

#include "petrack.h"

#include <QTimer>

Autosave::Autosave(Petrack &petrack) : mPetrack(petrack)
{
    mTimer = new QTimer{this};
    connect(mTimer, &QTimer::timeout, this, &Autosave::savePet);
    startTimer();
}

// Autosave::~Autosave() = default;
//{
//  NOTE: Currently this would also delete on Keyboard Interrupt (Ctrl + C)
//  Replaced by Method call in closeEvent in Petrack to circumvent this

// if Project-Object from Issue 88 gets implemented,
// Autosave should be a Member of the project, not Petrack
// then the destructor could maybe be used to delete the autosave
// deleteAutosave();
//}

/**
 * @brief Occasioanlly triggers autosaving
 *
 * This method gets called everytime the user modifies a trajectory. After a certain number of modifications, an
 * autosave for the .trc file is written to disk
 */
void Autosave::trackPersonModified()
{
    mChangeCounter++;
    if(mChangeCounter >= changesTillAutosave)
    {
        mChangeCounter = 0;
        saveTrc();
    }
}

void Autosave::resetTrackPersonCounter()
{
    mChangeCounter = 0;
}

/**
 * @brief Checks if autosave for project with given filename exists
 * @param filename path to the project file
 * @return true if autosave file exists, else false
 */
bool Autosave::autosaveExists(const QString &filename)
{
    return !getAutosave(QFileInfo(filename)).empty();
}

/**
 * @brief Delete all autosave files for currently loaded project (.pet and .trc)
 */
void Autosave::deleteAutosave()
{
    const auto autosaves = getAutosave();
    if(!autosaves.empty())
    {
        for(const auto &save : autosaves)
        {
            QFile saveFile{save};
            saveFile.remove();
        }
    }
}

/**
 * @brief Loads autosave files for currently loaded project
 */
void Autosave::loadAutosave()
{
    const auto autosaveFiles = getAutosave();
    if(autosaveFiles.empty())
    {
        return;
    }

    const auto petIndex = autosaveFiles.indexOf(QRegularExpression(R"(.*\.pet)"));
    if(petIndex != -1)
    {
        const QString petAutosaveName = autosaveFiles[petIndex];
        mPetrack.openProject(petAutosaveName);
    }
    else
    {
        mPetrack.openProject(mPetrack.getProFileName());
    }

    const auto trcIndex = autosaveFiles.indexOf(QRegularExpression(R"(.*\.trc)"));
    if(trcIndex != -1)
    {
        const QString trcAutosaveName = autosaveFiles[trcIndex];
        const auto    trcFile         = mPetrack.getTrackFileName();
        mPetrack.deleteTrackPointAll(PersonStorage::TrajectorySegment::Whole);
        mPetrack.importTracker(trcAutosaveName);
        mPetrack.setTrackFileName(trcFile);
    }
}

/**
 * @brief Returns whether a given filename would be autosave
 *
 * NOTE: File does not need to exist to be classified as autosave
 *
 * @param file file to test
 * @return true when name of an autosave, else false;
 */
bool Autosave::isAutosave(const QString &file)
{
    const auto &projectName                          = mPetrack.getProFileName();
    const auto [petRunningAutosave, petAutosaveName] = [&projectName]
    {
        auto names    = autosaveNamesPet(projectName);
        names.running = QFileInfo(names.running).absoluteFilePath();
        names.final   = QFileInfo(names.final).absoluteFilePath();
        return names;
    }();
    const auto [trcRunningAutosave, trcAutosaveName] = [&projectName]
    {
        auto names    = autosaveNamesTrc(projectName);
        names.running = QFileInfo(names.running).absoluteFilePath();
        names.final   = QFileInfo(names.final).absoluteFilePath();
        return names;
    }();

    QFileInfo      fileInfo{file};
    const QString &filePath = fileInfo.absoluteFilePath();
    return filePath == petAutosaveName || filePath == trcAutosaveName || filePath == petRunningAutosave ||
           filePath == trcRunningAutosave;
}

/**
 * @brief Starts timer for time-dependent autosave (pet-file)
 */
void Autosave::startTimer()
{
    if(!mTimer->isActive())
    {
        mTimer->start(petSaveInterval);
    }
}

/**
 * @brief Stops timer for time-dependent autosave (pet-file)
 */
void Autosave::stopTimer()
{
    if(mTimer->isActive())
    {
        mTimer->stop();
    }
}

void Autosave::restartTimer()
{
    stopTimer();
    startTimer();
}

QString Autosave::buildAutosaveName(const QString &projectFileName, const QString &ending)
{
    const QFileInfo projectFile{projectFileName};
    return projectFile.dir().filePath("." + projectFile.baseName() + "_autosave" + ending);
}

AutosaveFilenames Autosave::autosaveNamesTrc(const QString &projectFileName)
{
    return {buildAutosaveName(projectFileName, "_running.trc"), buildAutosaveName(projectFileName, ".trc")};
}

AutosaveFilenames Autosave::autosaveNamesPet(const QString &projectFileName)
{
    return {buildAutosaveName(projectFileName, "_running.pet"), buildAutosaveName(projectFileName, ".pet")};
}

/**
 * @brief Saves the .pet-file
 *
 * This method is called by the timeout signal of mTimer.
 * It saves the pet-file to a hidden file with a name derived from
 * the name of the currently loaded project
 */
void Autosave::savePet()
{
    const auto projectName = mPetrack.getProFileName();
    // only save if there is a project name; Means there needs to be an .pet file already for autosave to work. With our
    // workflow usually the case
    if(projectName.isEmpty() || !QFileInfo::exists(projectName) || QFileInfo(projectName).isDir())
    {
        return;
    }
    const auto &[autosaveName, finalAutosaveName] = autosaveNamesPet(projectName);
    mPetrack.saveProject(autosaveName);

    // first save to temp file, so crash during saving doesn't corrupt old autosave
    QFile tempAutosave{autosaveName};
    QFile autosave{finalAutosaveName};
    if(tempAutosave.exists())
    {
        if(autosave.exists())
        {
            autosave.remove();
        }
        if(tempAutosave.copy(finalAutosaveName))
        {
            // we don't currently use it for loading, so we could remove it even if the copying fails...
            tempAutosave.remove();
        }
    }
}

/**
 * @brief Saves the .trc-file
 *
 * This method is called by trackPersonModified after a set number of modifications.
 * It saves the trc-file to a hidden file with a name derived from
 * the name of the currently loaded project
 */
void Autosave::saveTrc()
{
    const auto projectName                        = mPetrack.getProFileName();
    const auto &[autosaveName, finalAutosaveName] = autosaveNamesTrc(projectName);
    const auto trackFileName                      = mPetrack.getTrackFileName();
    const auto lastTrackerExport                  = mPetrack.getLastTrackerExport();
    mPetrack.exportTracker(autosaveName);
    mPetrack.setTrackFileName(trackFileName);
    mPetrack.setLastTrackerExport(lastTrackerExport);

    // first save to temp file, so crash during saving doesn't corrupt old autosave
    QFile tempAutosave{autosaveName};
    QFile autosave{finalAutosaveName};
    if(tempAutosave.exists())
    {
        if(autosave.exists())
        {
            autosave.remove();
        }
        if(tempAutosave.copy(finalAutosaveName))
        {
            // we don't currently use it for loading, so we could remove it even if the copying fails...
            tempAutosave.remove();
        }
    }
}

/**
 * @brief Returns a list of autosave files for the current project
 * @return list of autosaves; may be empty
 */
QStringList Autosave::getAutosave()
{
    const auto projectPath = QFileInfo(mPetrack.getProFileName());
    return Autosave::getAutosave(projectPath);
}

/**
 * @brief Returns a list of autosave files for the project at projectPath
 * @param projectPath filepath of .pet-file
 * @return list of autosaves; may be empty
 */
QStringList Autosave::getAutosave(const QFileInfo &projectPath)
{
    if(projectPath.isFile())
    {
        QStringList     list;
        const auto      autosavePetName = autosaveNamesPet(projectPath.absoluteFilePath()).final;
        const QFileInfo autosavePet{autosavePetName};
        if(autosavePet.exists())
        {
            list.append(autosavePetName);
        }
        const auto      autosaveTrcName = autosaveNamesTrc(projectPath.absoluteFilePath()).final;
        const QFileInfo autosaveTrc{autosaveTrcName};
        if(autosaveTrc.exists())
        {
            list.append(autosaveTrcName);
        }
        return list;
    }

    return QStringList{};
}

/**
 * Time interval for pet file saves in s
 * @return Time interval for pet file saves in s
 */
int Autosave::getPetSaveInterval() const
{
    return petSaveInterval / 1000;
}

/**
 * Sets the time interval till the pet file is saved
 * @param petSaveInterval time interval till save in s
 */
void Autosave::setPetSaveInterval(int petSaveInterval)
{
    Autosave::petSaveInterval = petSaveInterval * 1000;
    restartTimer();
}

/**
 * Number of changes until the trajectory (trc) will be saved
 * @return Number of changes until the trajectory (trc) will be saved
 */
int Autosave::getChangesTillAutosave() const
{
    return changesTillAutosave;
}

/**
 * Sets the number of changes until the trajectory (trc) is saved
 * @param changesTillAutosave number of changes till save
 */
void Autosave::setChangesTillAutosave(int changesTillAutosave)
{
    Autosave::changesTillAutosave = changesTillAutosave;
}
