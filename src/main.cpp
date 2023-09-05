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

#include "IO.h"
#include "compilerInformation.h"
#include "control.h"
#include "helper.h"
#include "logger.h"
#include "petrack.h"
#include "tracker.h"

#include <QApplication>
#include <QMessageBox>
#include <QStyleFactory>
#include <QtWidgets>
#include <csignal>
#include <cstdio>
#include <sstream>
#include <string>

// Aufrufbeispiel:
// release/petrack.exe -sequence ../../einzelbilder/wert0001.png -autoSave dir|ttt.avi

static QApplication *gApp = nullptr;
void                 quit(int sig_number)
{
    gApp->quit();
}

int main(int argc, char *argv[])
{
    logger::setupLogger();

    Q_INIT_RESOURCE(icons);

    QApplication app(argc, argv);

    // Reihenfolge beim Beenden von Petrack (signal abouttoquit() von qapplication nicht hinbekommen):
    // - petrack:closeevent()
    // - app.exec() am ende dieser Programmzeilen wird beendet
    // - Funktion, die mit qAddPostRoutine an Qt uebergeben wird
    // - SIGTERM kann nicht von QT abgefangen werden, sondern muss mit signal behandelt werden
    gApp = &app;
    signal(SIGINT, quit); // used to catch ctrl-C and get rid of error "QObject::killTimers: timers cannot be stopped
                          // from another thread"

    app.setStyle(QStyleFactory::create("Fusion")); // added for Qt5

    // command line arguments ;leerzeichen zwischen option und argument wird benoetigt
    //   -project *.ptr:   um projekt zu laden
    //   -sequence *.png:  um animation zu laden (ueberschreibt projekt)
    //   -autoSave file oder ordner: um automatisches herausschreiben von sequenz in ordner zu machen
    QStringList arg = app.arguments();
    QString     sequence;
    QString     project;
    QString     autoSaveDest;
    bool        autoSave = false;
    QString     autoTrackDest;
    QString     autoPlayDest;
    bool        autoTrack     = false;
    bool        autoPlay      = false;
    bool        autoIntrinsic = false;
    QString     intrinsicDir;
    QString     autoReadHeightFile;
    bool        autoReadHeight = false;
    QString     autoReadMarkerFile;
    bool        autoReadMarkerID = false;
    QString     autoSaveTrackerFile;
    bool        autoSaveTracker = false;
    bool        autoExportView  = false;
    QString     exportViewFile;

    for(int i = 1; i < arg.size(); ++i) // i=0 ist Programmname
    {
        if(arg.at(i) == "-help" || arg.at(i) == "-?")
        {
            QTextDocument doc;
            doc.setHtml(commandLineOptionsString);
            SPDLOG_INFO("{}", doc.toPlainText());
            QMessageBox::about(nullptr, QObject::tr("Command line options"), commandLineOptionsString);
            return 0; // 0 means exit success // 1?
        }
        else if(arg.at(i) == "-project")
        {
            project = arg.at(++i);
        }
        else if(arg.at(i) == "-sequence")
        {
            sequence = arg.at(++i);
        }
        else if((arg.at(i) == "-autoSave") || (arg.at(i) == "-autosave"))
        {
            autoSave     = true;
            autoSaveDest = arg.at(++i);
        }
        else if((arg.at(i) == "-autoTrack") || (arg.at(i) == "-autotrack"))
        {
            autoTrack = true;
            // hat tracker_file bestimmte Dateiendung txt oder trc, dann wird nur genau diese exportiert, sonst beide
            autoTrackDest = arg.at(++i);
        }
        else if((arg.at(i) == "-autoPlay") || (arg.at(i) == "-autoplay"))
        { // nur abspielen und keine aenderungen an control track and reco, um zB groessenbestimmung nachtraeglich
          // vorzunehmen, nachdem haendisch kontrolliert
            autoPlay = true;
            // hat tracker_file bestimmte Dateiendung txt oder trc, dann wird nur genau diese exportiert, sonst beide
            autoPlayDest = arg.at(++i);
        }
        else if(arg.at(i).endsWith(".pet", Qt::CaseInsensitive))
        {
            // -project option ueberschreibt *.pet uebergabe ohne option!!
            if(project.isEmpty())
            {
                project = arg.at(i);
            }
        }
        else if((arg.at(i) == "-autoReadMarkerID") || (arg.at(i) == "-autoreadmarkerid"))
        {
            // -autoreadmarkerid followed by txt-file including personIDs and markerIDs
            // reads the txt-file and applies the markerIds to persons with corresponding personIDs
            autoReadMarkerID   = true;
            autoReadMarkerFile = arg.at(++i);
        }
        else if((arg.at(i) == "-autoReadHeight") || (arg.at(i) == "-autoreadheight"))
        {
            // -autoreadheight followed by txt-file including markerIDs and individual heights
            // reads the txt-file and applies the heights to persons with corresponding markerIDs
            autoReadHeight     = true;
            autoReadHeightFile = arg.at(++i);
        }
        else if((arg.at(i) == "-autoSaveTracker") || (arg.at(i) == "-autosavetracker"))
        {
            // -autoSaveTracker followed by Trackerfile which can either have the ending *.txt or *.trc. If no ending is
            // given, both files will be created
            autoSaveTracker     = true;
            autoSaveTrackerFile = arg.at(++i);
        }
        else if((arg.at(i) == "-autointrinsic") || (arg.at(i) == "-autoIntrinsic"))
        {
            autoIntrinsic = true;
            intrinsicDir  = arg.at(++i);
        }
        else if((arg.at(i) == "-autoExportView") || (arg.at(i) == "-autoexportview"))
        {
            autoExportView = true;
            exportViewFile = arg.at(++i);
        }
        else
        {
            // hier koennte je nach dateiendung *pet oder *avi oder *png angenommern werden
            // aber ueberpruefen, ob variable project oder sequence schon besetzt!!!
            SPDLOG_WARN("Option ignored (use -? for help): {}", arg.at(i));
        }
    }

    SPDLOG_INFO("Starting PeTrack");
    SPDLOG_INFO("Version: {}", PETRACK_VERSION);
    SPDLOG_INFO("Commit id: {}", GIT_COMMIT_HASH);
    SPDLOG_INFO("Commit date: {}", GIT_COMMIT_DATE);
    SPDLOG_INFO("Build from branch: {}", GIT_BRANCH);
    SPDLOG_INFO("Compile date: {}", COMPILE_TIMESTAMP);
    SPDLOG_INFO("Build with: {} ({})", COMPILER_ID, COMPILER_VERSION);

    Petrack petrack(PETRACK_VERSION);
    petrack.setGitInformation(GIT_COMMIT_HASH, GIT_COMMIT_DATE, GIT_BRANCH);
    petrack.setCompileInformation(COMPILE_OS, COMPILE_TIMESTAMP, COMPILER_ID, COMPILER_VERSION);

    petrack.show(); // damit bei reiner Hilfe nicht angezeigt wird, erst hier der aufruf

    // erst nachher ausfuehren, damit reihenfolge der command line argumente keine rolle spielt
    if(!project.isEmpty())
    {
        if(sequence.isEmpty())
        {
            petrack.openProject(project);
        }
        else
        {
            petrack.openProject(project, false);
        }
    }
    if(!sequence.isEmpty()) // nach project so dass dies datei in project ueberschreibt
    {
        petrack.openSequence(sequence);
    }
    if(autoSave && (!autoSaveDest.endsWith(".pet", Qt::CaseInsensitive)))
    {
        if((autoSaveDest.endsWith(".txt", Qt::CaseInsensitive)) ||
           (autoSaveDest.endsWith(".trav", Qt::CaseInsensitive)) ||
           (autoSaveDest.endsWith(".dat", Qt::CaseInsensitive)))
        {
            petrack.exportTracker(autoSaveDest); // projekt wird geladen und nur Trajektoprien herausgeschrieben (zB
                                                 // wenn sich .pet (altitude) oder .trc aendert (delrec))
        }
        else
        {
            petrack.exportSequence(
                true, false, autoSaveDest); // true spielt keine rolle, sondern wird durch dateiendung bestimmt
        }
        return EXIT_SUCCESS; // 0 means exit success// Programm beenden nach speichern! // 1?
    }

    if(autoIntrinsic)
    {
        QDir      intrinsicImagesDir{intrinsicDir};
        QFileInfo info{intrinsicImagesDir.absolutePath()};
        if(!(intrinsicImagesDir.exists() && info.isDir()))
        {
            SPDLOG_ERROR("{} isn't an existing directory.", intrinsicDir);
            return EXIT_FAILURE;
        }
        QStringList calibFiles;
        for(const auto &file : intrinsicImagesDir.entryInfoList(QDir::Files | QDir::Readable))
        {
            calibFiles.append(file.absoluteFilePath());
        }
        petrack.getAutoCalib()->setCalibFiles(calibFiles);
        petrack.getControlWidget()->runAutoCalib();
    }

    if(autoExportView)
    {
        QFile outputFile{exportViewFile};
        petrack.exportSequence(true, true, outputFile.fileName());
        return EXIT_SUCCESS;
    }

    // hat tracker_file bestimmte Dateiendung txt oder trc, dann wird nur genau diese exportiert, sonst beide
    if(autoTrack)
    {
        petrack.trackAll();

        if(autoReadMarkerID)
        {
            auto markerIDs = IO::readMarkerIDFile(autoReadMarkerFile);
            if(std::holds_alternative<std::unordered_map<int, int>>(markerIDs)) // heights contains the height map
            {
                petrack.getPersonStorage().setMarkerIDs(std::get<std::unordered_map<int, int>>(markerIDs));
                petrack.setMarkerIDFileName(autoReadHeightFile);
            }
            else // markerIDs contains an error string
            {
                SPDLOG_ERROR("{}", std::get<std::string>(markerIDs));
                return EXIT_FAILURE;
            }
        }

        if(autoReadHeight)
        {
            auto markerHeights = IO::readHeightFile(autoReadHeightFile);
            if(std::holds_alternative<std::unordered_map<int, float>>(markerHeights)) // heights contains the height map
            {
                petrack.getPersonStorage().resetHeight();
                petrack.getPersonStorage().setMarkerHeights(std::get<std::unordered_map<int, float>>(markerHeights));
                petrack.setHeightFileName(autoReadHeightFile);
            }
            else // markerHeights contains an error string
            {
                SPDLOG_ERROR("{}", std::get<std::string>(markerHeights));
                return EXIT_FAILURE;
            }
        }

        petrack.exportTracker(autoTrackDest);
        if(autoSave && (autoSaveDest.endsWith(".pet", Qt::CaseInsensitive)))
        {
            petrack.saveProject(autoSaveDest);
        }

        return EXIT_SUCCESS; // Programm beenden nach speichern! // 1?
    }

    if(autoPlay)
    {
        petrack.playAll();
        petrack.exportTracker(autoPlayDest);
        if(autoSave && (autoSaveDest.endsWith(".pet", Qt::CaseInsensitive)))
        {
            petrack.saveProject(autoSaveDest);
        }
        return EXIT_SUCCESS; // Programm beenden nach speichern! // 1?
    }

    if(!autoTrack && autoReadMarkerID) // TODO autoTrack is always false here, as otherwise already returned
    {
        auto markerIDs = IO::readMarkerIDFile(autoReadMarkerFile);
        if(std::holds_alternative<std::unordered_map<int, int>>(markerIDs)) // heights contains the height map
        {
            petrack.getPersonStorage().setMarkerIDs(std::get<std::unordered_map<int, int>>(markerIDs));
            petrack.setMarkerIDFileName(autoReadHeightFile);
        }
        else // heights contains an error string
        {
            SPDLOG_ERROR("{}", std::get<std::string>(markerIDs));
            return EXIT_FAILURE;
        }
    }

    if(!autoTrack && autoReadHeight) // TODO autoTrack is always false here, as otherwise already returned
    {
        auto markerHeights = IO::readHeightFile(autoReadHeightFile);
        if(std::holds_alternative<std::unordered_map<int, float>>(markerHeights)) // heights contains the height map
        {
            petrack.getPersonStorage().resetHeight();
            petrack.getPersonStorage().setMarkerHeights(std::get<std::unordered_map<int, float>>(markerHeights));
            petrack.setHeightFileName(autoReadHeightFile);
        }
        else // heights contains an error string
        {
            SPDLOG_ERROR("{}", std::get<std::string>(markerHeights));
            return EXIT_FAILURE;
        }
    }

    if(autoSaveTracker)
    {
        petrack.exportTracker(autoSaveTrackerFile);
    }
    if(autoSave && (autoSaveDest.endsWith(".pet", Qt::CaseInsensitive)))
    {
        petrack.saveProject(autoSaveDest);
        return EXIT_SUCCESS;
    }

    return app.exec();
}
