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

#include <QApplication>
#include <QtWidgets>
#include <QMessageBox>
#include <QStyleFactory>

#include "petrack.h"
#include "helper.h"
#include "tracker.h"

using namespace std;

// Aufrufbeispiel:
// release/petrack.exe -sequence ../../einzelbilder/wert0001.png -autoSave dir|ttt.avi

#include <stdio.h>  /* defines FILENAME_MAX */

#include "signal.h"
#include "IO.h"

// musst be done to store fixed order of attributes in XML files
// see: http://stackoverflow.com/questions/27378143/qt-5-produce-random-attribute-order-in-xml
extern Q_CORE_EXPORT QBasicAtomicInt qt_qhash_seed;

static QApplication * gApp = NULL;
void quit(int sig_number)
{
    gApp->quit(); // gPetrack->close();
    return;
}

int main(int argc, char *argv[])
{

//#if CV_MAJOR_VERSION == 2
//// do opencv 2 code
//debout << "OpenCV 2 is used..." << endl;
//#elif CV_MAJOR_VERSION == 3
//// do opencv 3 code
//debout << "OpenCV 3 is used..." << endl;
//#endif

//    char cCurrentPath[FILENAME_MAX];
//    if (!GetCurrentDir(cCurrentPath, sizeof(cCurrentPath)))
//    {
//        debout << "Error: could not detect working directory!" <<endl;
//        return 1;
//    }
//    cCurrentPath[sizeof(cCurrentPath) - 1] = '\0'; /* not really required */
//    printf ("The current working directory is +%s+", cCurrentPath);



    Q_INIT_RESOURCE(icons);

    QApplication app(argc, argv);

// Reihenfolge beim Beenden von Petrack (signal abouttoquit() von qapplication nicht hinbekommen):
// - petrack:closeevent()
// - app.exec() am ende dieser Programmzeilen wird beendet
// - Funktion, die mit qAddPostRoutine an Qt uebergeben wird
// - SIGTERM kann nicht von QT abgefangen werden, sondern muss mit signal behandelt werden
    gApp = &app;
    signal(SIGINT, quit); // used to catch ctrl-C and get rid of error "QObject::killTimers: timers cannot be stopped from another thread"

    //app.addLibraryPath(cCurrentPath); // "./" for file platforms/qwindows.dll, which now could be stored to main prog folder // only for plugin files
    //app.setStyle(new QCleanlooksStyle);// QMacStyle, QPlastiqueStyle, QCleanlooksStyle; kann dann nicht mehr mit "-style motif windows oder platinum" beim aufruf gesetzt werden
    app.setStyle(QStyleFactory::create("Fusion")); // added for Qt5
    // command line arguments ;leerzeichen zwischen option und argument wird benoetigt
    //   -project *.ptr:   um projekt zu laden
    //   -sequence *.png:  um animation zu laden (ueberschreibt projekt)
    //   -autoSave file oder ordner: um automatisches herausschreiben von sequenz in ordner zu machen
    QStringList arg = app.arguments();
    QString sequence;
    QString project;
    QString autoSaveDest;
    bool autoSave = false;
    QString autoTrackDest;
    QString autoPlayDest;
    bool autoTrack = false;
    bool autoPlay = false;

    QString autoReadHeightFile;
    bool autoReadHeight = false;

    for (int i = 1; i < arg.size(); ++i) // i=0 ist Programmname
    {
        //cout << arg.at(i).toStdString() << endl;
        if (arg.at(i) == "-help" || arg.at(i) == "-?")
        {
            QTextDocument doc;
            doc.setHtml(commandLineOptionsString);
            debout << endl << doc.toPlainText() <<endl;
            //debout << commandLineOptionsString <<endl;
            QMessageBox::about(NULL, QObject::tr("Command line options"), commandLineOptionsString);
            //cout << "Help:\n-----" << endl
            //     << "Usage: petrack [-help|-?] [[-project] project.pet] [-sequence image_sequence_or_video] [-autoSave|-autosave image_folder_or_video|project.pet|tracker_file] [-autoTrack|-autotrack tracker_file] [-autoPlay|-autoplay tracker_file]" << endl << endl
           //      << "For more information see http://www.fz-juelich.de/jsc/petrack/." << endl;
            return 0; // 0 means exit success // 1?
        }
        else if (arg.at(i) == "-project") 
            project = arg.at(++i);
        else if (arg.at(i) == "-sequence") 
            sequence = arg.at(++i);
        else if ((arg.at(i) == "-autoSave") || (arg.at(i) == "-autosave"))
        {
            autoSave = true;
            autoSaveDest = arg.at(++i);
        }
        else if ((arg.at(i) == "-autoTrack") || (arg.at(i) == "-autotrack"))
        {
            autoTrack = true;
            // hat tracker_file bestimmte Dateiendung txt oder trc, dann wird nur genau diese exportiert, sonst beide
            autoTrackDest = arg.at(++i);
        }
        else if ((arg.at(i) == "-autoPlay") || (arg.at(i) == "-autoplay"))
        { // nur abspielen und keine aenderungen an control track and reco, um zB groessenbestimmung nachtraeglich vorzunehmen, nachdem haendisch kontrolliert
            autoPlay = true;
            // hat tracker_file bestimmte Dateiendung txt oder trc, dann wird nur genau diese exportiert, sonst beide
            autoPlayDest = arg.at(++i);
        }
        else if (arg.at(i).right(4) == ".pet")
        {
            // -project option ueberschreibt *.pet uebergabe ohne option!!
            if (project.isEmpty())
                project = arg.at(i);
        }
        else if ((arg.at(i) == "-autoReadHeight") || (arg.at(i) == "-autoreadheight"))
        {
            // -autoreadheight followed by txt-file including markerIDs and individual heights
            // reads the txt-file and applies the heights to persons with corresponding markerIDs
            autoReadHeight = true;
            autoReadHeightFile = arg.at(++i);
        }
        else
            // hier koennte je nach dateiendung *pet oder *avi oder *png angenommern werden
            // aber ueberpruefen, ob variable project oder sequence schon besetzt!!!
            cout << "Option ignored (use -? for help): " << arg.at(i) << endl;
    }

    Petrack petrack;
    petrack.show(); // damit bei reiner Hilfe nicht angezeigt wird, erst hier der aufruf

    debout << "Starting PeTrack Version " << VERSION << " (Build " << COMPILE_DATE << " " << COMPILE_TIME << ", " << argv[0] << ")" <<endl;

    // erst nachher ausfuehren, damit reihenfolge der command line argumente keine rolle spielt
    if (!project.isEmpty())
    {
        if (sequence.isEmpty())
            petrack.openProject(project);
        else
            petrack.openProject(project, false);
    }
    if (!sequence.isEmpty()) // nach project so dass dies datei in project ueberschreibt
        petrack.openSequence(sequence);
    if (autoSave && (autoSaveDest.right(4) != ".pet"))
    {
        if ((autoSaveDest.right(4) == ".txt")||(autoSaveDest.right(5) == ".trav")||(autoSaveDest.right(4) == ".dat"))
            petrack.exportTracker(autoSaveDest); // projekt wird geladen und nur Trajektoprien herausgeschrieben (zB wenn sich .pet (altitude) oder .trc aendert (delrec))
        else
            petrack.saveSequence(true, false, autoSaveDest); // true spielt keine rolle, sondern wird durch dateiendung bestimmt
        return 0;  // 0 means exit success// Programm beenden nach speichern! // 1?
    }
    // hat tracker_file bestimmte Dateiendung txt oder trc, dann wird nur genau diese exportiert, sonst beide
    if (autoTrack)
    {
        petrack.trackAll();

        // platziert zwischen tracken und exportieren
        if (autoReadHeight)
        {
            auto markerHeights = IO::readHeightFile(autoReadHeightFile);
            if (std::holds_alternative<std::unordered_map<int, float>>(markerHeights)) // heights contains the height map
            {
                petrack.getTracker()->resetHeight();
                petrack.getTracker()->setMarkerHeights(std::get<std::unordered_map<int, float>>(markerHeights));
                petrack.setHeightFileName(autoReadHeightFile);
            }
            else //heights contains an error string
            {
                debout << "Error: " << std::get<std::string>(markerHeights) << "\n";
                return EXIT_FAILURE;
            }
        }

//         if ((autoTrackDest.right(4) == ".trc") || (autoTrackDest.right(4) == ".txt"))
            petrack.exportTracker(autoTrackDest);
//         else // beide formate - wird nun in exportTracker ermoeglicht
//         {
//             petrack.exportTracker(autoTrackDest + ".trc");
//             petrack.exportTracker(autoTrackDest + ".txt");
//         }
        if (autoSave && (autoSaveDest.right(4) == ".pet"))
            petrack.saveProject(autoSaveDest);
        return 0;  // 0 means exit success // Programm beenden nach speichern! // 1?
    }
    if (autoPlay)
    {
        petrack.playAll();
        petrack.exportTracker(autoPlayDest);
        if (autoSave && (autoSaveDest.right(4) == ".pet"))
            petrack.saveProject(autoSaveDest);
        return 0;  // 0 means exit success // Programm beenden nach speichern! // 1?
    }

    return app.exec();
}
