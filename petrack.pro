# wichtig: gelinkte dynamische libraries muessen auch in PATH liegen (Systemsteuerung...), sonst kommt aufruf direkt zurueck - cygwin neu starten!!!
# welche dll bei Aufruf genommen werden koennen ueber Programm DEPENDENCY WALKER herausbekommen werden

# um eine .pro Datei fuer mehrere Rechner nutzen zu koennen
# die Variable hostname muss auf rechner von console einmalig mit "qmake -set hostname zam852" gesetzt werden (Abfrage aller Var mit qmake -query)
# hostname = $$[hostname]
hostname = $$system(hostname)
unix:hostname = $${QMAKE_HOST.name}
message($${hostname})
!build_pass:message(Generating Makefile for ($$hostname)!)

TEMPLATE = app
CONFIG += console # noetig, damit stdout angezeigt wird
#CONFIG += debug_and_release # scheint default zu sein!!! build_all
#CONFIG += build_all

QT += opengl # // wenn opengl widget etc genutzt werden soll
# opengl qt warn_on
QT += xml # // wenn xml benoetigt werden soll
QT += core
#QT += gui
QT += widgets
QT += printsupport

# windows - The target is a Win32 window application (app only). The proper include paths,compiler flags and libraries will automatically be added to the project.
# console - The target is a Win32 console application (app only). The proper include paths, compiler flags and libraries will automatically be added to the project.
# dll - The target is a shared object/DLL.The proper include paths, compiler flags and libraries will automatically be added to the project.
# staticlib - The target is a static library (lib only). The proper compiler flags will automatically be added to the project.

TARGET = petrack
INCLUDEPATH += include
# default settings:
STEREO = false # true / false
AVI = false # true / false
QWT = true # true / false
LIBELAS = true

contains(hostname, [zZ][aA][mM]852) {
#Hermes rechner #########################################################################################
  STEREO = false # true / false
  DEFINES += STEREO_DISABLED
  AVI = true # true / false
  #CVPATH = "C:/OpenCV-2.2.0/bin"
  CVPATH = "../petrack/3rdparty/windows/opencv-4.2.0_64bit"
  CVVERSION = "240"
  LIBS += -llibopencv_core$${CVVERSION} -llibopencv_highgui$${CVVERSION} -llibopencv_imgproc$${CVVERSION} -llibopencv_calib3d$${CVVERSION} -llibopencv_video$${CVVERSION} -llibopencv_legacy$${CVVERSION} # -lcv$${CVVERSION} -lcvaux$${CVVERSION}
#  CVPATH = "C:/Programme/OpenCV1.2"
#  CVVERSION = "120"
#  CVPATH = "C:/OpenCV2.1"
#  CVVERSION = "210"
#  QWTPATH = "C:/Programme/qwt-5.2.0"
  QWTPATH = "C:/Qwt-5.2.0"
  PGRPATH = "C:/Programme/Point Grey Research/Triclops Stereo Vision SDK"
  # noetig fuer qt 5 um u.a. qprinter zu finden
  QT += printsupport
QMAKE_CXXFLAGS += -fpermissive

} else:contains(hostname, [zZ][aA][mM]460) {  #.zam.kfa-juelich.de
# Mac Maik #########################################################################################
  QMAKE_MAC_SDK = macosx10.12
  #CVVERSION = ""
  CVVERSION = "420"
  CV_MAJOR_VERSION = "4"

  #AVI = true # false

  QWTPATH = "/Users/boltes/Applications/qwt-6.1.3/build-qwt-Desktop_Qt_5_7_0_clang_64bit2-Release/lib/qwt.framework/Versions/6"
  INCLUDEPATH += $${QWTPATH}/Headers
  LIBS += -L$${QWTPATH}/
  # spaeter:
  # install_name_tool -change qwt.framework/Versions/6/qwt /Users/boltes/Applications/qwt-6.1.3/build-qwt-Desktop_Qt_5_7_0_clang_64bit2-Release/lib/qwt.framework/Versions/6/qwt ./petrack.app/Contents/MacOS/petrack

  QMAKE_LFLAGS += -F/Users/boltes/Applications/qwt-6.1.3/build-qwt-Desktop_Qt_5_7_0_clang_64bit2-Release/lib
  #LIBS += -framework qwt

  #CVPATH = "/Users/boltes/Applications/opencv-3.1.0/release"
  CVPATH = "../petrack/3rdparty/windows/opencv-4.2.0"

  #INCLUDEPATH += /usr/local/include
  #INCLUDEPATH += /usr/local/include/opencv
  #INCLUDEPATH += /usr/local/include/opencv2
  INCLUDEPATH += $${CVPATH}/include/
  INCLUDEPATH += $${CVPATH}/include/opencv
  INCLUDEPATH += $${CVPATH}/include/opencv2
  #INCLUDEPATH += /usr/local/Cellar/opencv3/3.1.0_3/include/

  #PGRPATH = "/home/zam/salden/libs/Triclops/release"
  LIBS += -lopencv_core \
          -lopencv_highgui \
          -lopencv_imgproc \
          -lopencv_calib3d \
          -lopencv_imgcodecs \
          -lopencv_videoio \
          -lopencv_video #\
       #   -l opencv_ffmpeg \
       #   -lopencv_legacy
QMAKE_CXXFLAGS += -fpermissive
} else:contains(hostname, arne-laptop) {
# Laptop Daniel ##########################################################################################
  STEREO = false # true / false
  DEFINES += STEREO_DISABLED

  #PGRPATH = "C:/Program Files/Point Grey Research/Triclops Stereo Vision SDK"

  CONFIG(debug, debug|release) {
      # only debug
        PGRPATH = "../trunk/3rdparty/windows/triclops-3.4"
        CVPATH = "../petrack/3rdparty/windows/opencv-4.2.0_64bit"
        QWTPATH = "../petrack/3rdparty/windows/Qwt-6.1.4_64bit"

    } else {
        # only release
        PGRPATH = "../trunk/3rdparty/windows/triclops-3.4"
        CVPATH = "../petrack/3rdparty/windows/opencv-4.2.0_64bit"
        QWTPATH = "../petrack/3rdparty/windows/Qwt-6.1.4_64bit"
    }
  #CVPATH = "%OPENCV_DIR%"
  #CVPATH = "C:\Program Files (x86)\OpenCV\3.0.0\build4qt5\install"
  CVVERSION = "420"
  CV_MAJOR_VERSION = "4"

  # to switch between version 2.4.10 and 3.0.0 you have to update your OPENCV_DIR environment variable to the specific path

  #CVPATH = "C:/Program Files (x86)/OpenCV/2.4.10/build"
  #CVVERSION = "2410"

  INCLUDEPATH += $${CVPATH}/include
  INCLUDEPATH += $${CVPATH}/include/opencv
  INCLUDEPATH += $${CVPATH}/include/opencv2

  #QWTPATH = "C:/Program Files (x86)/Qwt/Qwt-5.2.3"
  #QWTPATH = "C:\Qwt-6.1.2"
  INCLUDEPATH += $${QWTPATH}/include


  #STEREO = false # true / false
  AVI = false # true / false
  #QWT = false # true / false
  #LIBELAS = false

  # fuer 64 bit -lopencv_ffmpeg_64
  LIBS += -L$${CVPATH}/x64/mingw/bin \
          -lopencv_core$${CVVERSION} \
          -lopencv_highgui$${CVVERSION} \
          -lopencv_imgproc$${CVVERSION} \
          -lopencv_calib3d$${CVVERSION} \
          -lopencv_video$${CVVERSION} \
          -lopencv_videoio_ffmpeg$${CVVERSION}_64\
          -lopencv_aruco$${CVVERSION} \
          -lopencv_videoio$${CVVERSION} \
          -lopencv_imgcodecs$${CVVERSION}
  # QMAKE_CXXFLAGS += -march=i686
QMAKE_CXXFLAGS += -fpermissive

} else:contains(hostname, [zZ][aA][mM]197) {
# Laptop Daniel ##########################################################################################
  STEREO = false # true / false
  DEFINES += STEREO_DISABLED

  #PGRPATH = "C:/Program Files/Point Grey Research/Triclops Stereo Vision SDK"

  CONFIG(debug, debug|release) {
      # only debug
        PGRPATH = "../trunk/3rdparty/windows/triclops-3.4"
        CVPATH = "../petrack/3rdparty/windows/opencv-4.2.0_64bit"
        QWTPATH = "../petrack/3rdparty/windows/Qwt-6.1.4_64bit"

    } else {
        # only release
        PGRPATH = "../trunk/3rdparty/windows/triclops-3.4"
        CVPATH = "../petrack/3rdparty/windows/opencv-4.2.0_64bit"
        QWTPATH = "../petrack/3rdparty/windows/Qwt-6.1.4_64bit"
    }
  #CVPATH = "%OPENCV_DIR%"
  #CVPATH = "C:\Program Files (x86)\OpenCV\3.0.0\build4qt5\install"
  CVVERSION = "420"
  CV_MAJOR_VERSION = "4"

  # to switch between version 2.4.10 and 3.0.0 you have to update your OPENCV_DIR environment variable to the specific path

  #CVPATH = "C:/Program Files (x86)/OpenCV/2.4.10/build"
  #CVVERSION = "2410"

  INCLUDEPATH += $${CVPATH}/include
  INCLUDEPATH += $${CVPATH}/include/opencv
  INCLUDEPATH += $${CVPATH}/include/opencv2

  #QWTPATH = "C:/Program Files (x86)/Qwt/Qwt-5.2.3"
  #QWTPATH = "C:\Qwt-6.1.2"
  INCLUDEPATH += $${QWTPATH}/include


  #STEREO = false # true / false
  AVI = false # true / false
  #QWT = false # true / false
  #LIBELAS = false

  # fuer 64 bit -lopencv_ffmpeg_64
  LIBS += -L$${CVPATH}/x64/mingw/bin \
          -lopencv_core$${CVVERSION} \
          -lopencv_highgui$${CVVERSION} \
          -lopencv_imgproc$${CVVERSION} \
          -lopencv_calib3d$${CVVERSION} \
          -lopencv_video$${CVVERSION} \
          -lopencv_videoio_ffmpeg$${CVVERSION}_64\
          -lopencv_aruco$${CVVERSION} \
          -lopencv_videoio$${CVVERSION} \
          -lopencv_imgcodecs$${CVVERSION}
  # QMAKE_CXXFLAGS += -march=i686
QMAKE_CXXFLAGS += -fpermissive

} else:contains(hostname, ias7185) {
  # imotion laptop
  DEFINES += STEREO_DISABLED

  # PointGray
  PGRPATH = "../petrack/3rdparty/windows/triclops-3.4"
  #PGRPATH = "C:/Program Files (x86)/Point Grey Research/Triclops Stereo Vision SDK"
  # OpenCV
  #CVPATH = "C:/OpenCV/opencv300/build/install"
  #CVPATH = "3rdparty/windows/opencv-3.1.0"
  CVPATH = "../petrack/3rdparty/windows/opencv-4.2.0_64bit"
  CVVERSION = "420"
  CV_MAJOR_VERSION = "4"

  # to switch between version 2.4.10 and 3.0.0 you have to update your OPENCV_DIR environment variable to the specific path

  INCLUDEPATH += $${CVPATH}/include
  INCLUDEPATH += $${CVPATH}/include/opencv
  INCLUDEPATH += $${CVPATH}/include/opencv2

  QWTPATH = "../petrack/3rdparty/windows/Qwt-6.1.4_64bit"
  #QWTPATH = "D:/petrack/trunk/3rdparty/windows/qwt-6.1.2"
  INCLUDEPATH += $${QWTPATH}/include

  STEREO = false # true / false
  AVI = false # true / false
  #QWT = false # true / false
  #LIBELAS = false

  # fuer 64 bit -lopencv_ffmpeg_64
  LIBS += -L$${CVPATH}/x64/mingw/bin \
          -lopencv_core$${CVVERSION} \
          -lopencv_highgui$${CVVERSION} \
          -lopencv_imgproc$${CVVERSION} \
          -lopencv_calib3d$${CVVERSION} \
          -lopencv_video$${CVVERSION} \
          -lopencv_videoio_ffmpeg$${CVVERSION}_64\
          -lopencv_aruco$${CVVERSION} \
          -lopencv_videoio$${CVVERSION} \
          -lopencv_imgcodecs$${CVVERSION}
  # QMAKE_CXXFLAGS += -march=i686
QMAKE_CXXFLAGS += -fpermissive

} else:contains(hostname, ias7176) {
  # Deniz 32-Bit
  DEFINES += STEREO_DISABLED

  # PointGray
  PGRPATH = "../petrack/3rdparty/windows/triclops-3.4"
  #PGRPATH = "C:/Program Files (x86)/Point Grey Research/Triclops Stereo Vision SDK"
  # OpenCV
  #CVPATH = "C:/OpenCV/opencv300/build/install"
  #CVPATH = "3rdparty/windows/opencv-3.1.0"
  CVPATH = "../petrack/3rdparty/windows/opencv-4.2.0_64bit"
  CVVERSION = "420"
  CV_MAJOR_VERSION = "4"

  # to switch between version 2.4.10 and 3.0.0 you have to update your OPENCV_DIR environment variable to the specific path

  INCLUDEPATH += $${CVPATH}/include
  INCLUDEPATH += $${CVPATH}/include/opencv
  INCLUDEPATH += $${CVPATH}/include/opencv2

  QWTPATH = "../petrack/3rdparty/windows/Qwt-6.1.4_64bit"
  #QWTPATH = "D:/petrack/trunk/3rdparty/windows/qwt-6.1.2"
  INCLUDEPATH += $${QWTPATH}/include

  STEREO = false # true / false
  AVI = false # true / false
  #QWT = false # true / false
  #LIBELAS = false



#  CONFIG(debug, debug|release){
#      # fuer 64 bit -lopencv_ffmpeg_64
#      LIBS += -L$${CVPATH}/x64/mingw/bin \
#              -lopencv_core$${CVVERSION}d \
#              -lopencv_highgui$${CVVERSION}d \
#              -lopencv_imgproc$${CVVERSION}d \
#              -lopencv_calib3d$${CVVERSION}d \
#              -lopencv_video$${CVVERSION}d \
#              -lopencv_videoio_ffmpeg$${CVVERSION}_64\
#              -lopencv_aruco$${CVVERSION}d \
#              -lopencv_videoio$${CVVERSION}d \
#              -lopencv_imgcodecs$${CVVERSION}d
#      # QMAKE_CXXFLAGS += -march=i686
#  } else{
      # fuer 64 bit -lopencv_ffmpeg_64
      LIBS += -L$${CVPATH}/x64/mingw/bin \
              -lopencv_core$${CVVERSION} \
              -lopencv_highgui$${CVVERSION} \
              -lopencv_imgproc$${CVVERSION} \
              -lopencv_calib3d$${CVVERSION} \
              -lopencv_video$${CVVERSION} \
              -lopencv_videoio_ffmpeg$${CVVERSION}_64\
              -lopencv_aruco$${CVVERSION} \
              -lopencv_videoio$${CVVERSION} \
              -lopencv_imgcodecs$${CVVERSION}
      ## QMAKE_CXXFLAGS += -march=i686
QMAKE_CXXFLAGS += -fpermissive
#  }

} else:contains(hostname, ias7171) {
  # Ann Katrin 32-Bit
  DEFINES += STEREO_DISABLED

  # PointGray
  PGRPATH = "../petrack/3rdparty/windows/triclops-3.4"
  #PGRPATH = "C:/Program Files (x86)/Point Grey Research/Triclops Stereo Vision SDK"
  # OpenCV
  #CVPATH = "C:/OpenCV/opencv300/build/install"
  #CVPATH = "3rdparty/windows/opencv-3.1.0"
  CVPATH = "../petrack/3rdparty/windows/opencv-4.2.0_64bit"
  CVVERSION = "420"
  CV_MAJOR_VERSION = "4"

  # to switch between version 2.4.10 and 3.0.0 you have to update your OPENCV_DIR environment variable to the specific path

  INCLUDEPATH += $${CVPATH}/include
  INCLUDEPATH += $${CVPATH}/include/opencv
  INCLUDEPATH += $${CVPATH}/include/opencv2

  QWTPATH = "../petrack/3rdparty/windows/Qwt-6.1.4_64bit"
  #QWTPATH = "D:/petrack/trunk/3rdparty/windows/qwt-6.1.2"
  INCLUDEPATH += $${QWTPATH}/include

  STEREO = false # true / false
  AVI = false # true / false
  #QWT = false # true / false
  #LIBELAS = false

  # fuer 64 bit -lopencv_ffmpeg_64
  LIBS += -L$${CVPATH}/x64/mingw/bin \
          -lopencv_core$${CVVERSION} \
          -lopencv_highgui$${CVVERSION} \
          -lopencv_imgproc$${CVVERSION} \
          -lopencv_calib3d$${CVVERSION} \
          -lopencv_video$${CVVERSION} \
          -lopencv_videoio_ffmpeg$${CVVERSION}_64\
          -lopencv_aruco$${CVVERSION} \
          -lopencv_videoio$${CVVERSION} \
          -lopencv_imgcodecs$${CVVERSION}
  # QMAKE_CXXFLAGS += -march=i686
QMAKE_CXXFLAGS += -fpermissive

}else:contains(hostname, ias7160) {
  # Test Laptop 32-Bit
  DEFINES += STEREO_DISABLED

  # PointGray
  PGRPATH = "../petrack/3rdparty/windows/triclops-3.4"
  #PGRPATH = "C:/Program Files (x86)/Point Grey Research/Triclops Stereo Vision SDK"
  # OpenCV
  #CVPATH = "C:/OpenCV/opencv300/build/install"
  #CVPATH = "3rdparty/windows/opencv-3.1.0"
  CVPATH = "../petrack/3rdparty/windows/opencv-4.2.0_64bit"
  CVVERSION = "420"
  CV_MAJOR_VERSION = "4"

  # to switch between version 2.4.10 and 3.0.0 you have to update your OPENCV_DIR environment variable to the specific path

  INCLUDEPATH += $${CVPATH}/include
  INCLUDEPATH += $${CVPATH}/include/opencv
  INCLUDEPATH += $${CVPATH}/include/opencv2

  QWTPATH = "../petrack/3rdparty/windows/Qwt-6.1.4_64bit"
  #QWTPATH = "D:/petrack/trunk/3rdparty/windows/qwt-6.1.2"
  INCLUDEPATH += $${QWTPATH}/include

  STEREO = false # true / false
  AVI = false # true / false
  #QWT = false # true / false
  #LIBELAS = false

  # fuer 64 bit -lopencv_ffmpeg_64
  LIBS += -L$${CVPATH}/x64/mingw/bin \
          -lopencv_core$${CVVERSION} \
          -lopencv_highgui$${CVVERSION} \
          -lopencv_imgproc$${CVVERSION} \
          -lopencv_calib3d$${CVVERSION} \
          -lopencv_video$${CVVERSION} \
          -lopencv_videoio_ffmpeg$${CVVERSION}_64\
          -lopencv_aruco$${CVVERSION} \
          -lopencv_videoio$${CVVERSION} \
          -lopencv_imgcodecs$${CVVERSION}
  # QMAKE_CXXFLAGS += -march=i686
QMAKE_CXXFLAGS += -fpermissive

}else:contains(hostname, ias7169) {
  # Deniz Laptop
  DEFINES += STEREO_DISABLED

  # PointGray
  PGRPATH = "../petrack/3rdparty/windows/triclops-3.4"
  #PGRPATH = "C:/Program Files (x86)/Point Grey Research/Triclops Stereo Vision SDK"
  # OpenCV
  #CVPATH = "C:/OpenCV/opencv300/build/install"
  #CVPATH = "3rdparty/windows/opencv-3.1.0"
  CVPATH = "../petrack/3rdparty/windows/opencv-4.2.0_64bit"
  CVVERSION = "420"
  CV_MAJOR_VERSION = "4"

  # to switch between version 2.4.10 and 3.0.0 you have to update your OPENCV_DIR environment variable to the specific path

  INCLUDEPATH += $${CVPATH}/include
  INCLUDEPATH += $${CVPATH}/include/opencv
  INCLUDEPATH += $${CVPATH}/include/opencv2

  QWTPATH = "../petrack/3rdparty/windows/Qwt-6.1.4_64bit"
  #QWTPATH = "D:/petrack/trunk/3rdparty/windows/qwt-6.1.2"
  INCLUDEPATH += $${QWTPATH}/include

  STEREO = false # true / false
  AVI = false # true / false
  #QWT = false # true / false
  #LIBELAS = false

  # fuer 64 bit -lopencv_ffmpeg_64
  LIBS += -L$${CVPATH}/x64/mingw/bin \
          -lopencv_core$${CVVERSION} \
          -lopencv_highgui$${CVVERSION} \
          -lopencv_imgproc$${CVVERSION} \
          -lopencv_calib3d$${CVVERSION} \
          -lopencv_video$${CVVERSION} \
          -lopencv_videoio_ffmpeg$${CVVERSION}_64\
          -lopencv_aruco$${CVVERSION} \
          -lopencv_videoio$${CVVERSION} \
          -lopencv_imgcodecs$${CVVERSION}
  # QMAKE_CXXFLAGS += -march=i686
QMAKE_CXXFLAGS += -fpermissive

}else {
  message(Host unknown for configuration!)
}

# OPENCV #########################################################################################################################

#Betriebssystemweiche
win32 {
     message("Build for Windows.")
     DEFINES += WINDOWS
     INCLUDEPATH += $${CVPATH}/include
     INCLUDEPATH += $${CVPATH}/include/opencv
     INCLUDEPATH += $${CVPATH}/include/opencv2
     QMAKE_LIBDIR += $${CVPATH}/bin
     QMAKE_LIBDIR += $${CVPATH}/x86/mingw/bin

}
macx {
     message("Build for Mac.")
     DEFINES += MAC
     INCLUDEPATH += $${CVPATH}/include
     INCLUDEPATH += $${CVPATH}/include/opencv
     INCLUDEPATH += $${CVPATH}/include/opencv2

     QMAKE_LIBDIR += $${CVPATH}/lib
     QMAKE_LIBDIR += /usr/local/lib

}
unix:!macx {
     message("Build for Unix.")
     DEFINES += UNIX
     INCLUDEPATH += $${CVPATH}/include
     INCLUDEPATH += $${CVPATH}/include/opencv
     INCLUDEPATH += $${CVPATH}/include/opencv2

     QMAKE_LIBDIR += $${QWTPATH}/lib
     QMAKE_LIBDIR += /usr/local/lib
     LIBS += -L$${QWTPATH}/lib
}

# OPENCV  ########################################################################################################################

#  LIBS += -lopencv_core$${CVVERSION} \
#          -lopencv_highgui$${CVVERSION} \
#          -lopencv_imgproc$${CVVERSION} \
#          -lopencv_calib3d$${CVVERSION} \
#          -lopencv_video$${CVVERSION} \
#          -lopencv_ffmpeg$${CVVERSION}
#    contains(CV_MAJOR_VERSION,3){
#      message("Build with OpenCV 3.0.0")
#      LIBS += -lopencv_videoio$${CVVERSION} \
#              -lopencv_imgcodecs$${CVVERSION}
#      # QMAKE_CXXFLAGS += -march=i686
#    }else {
#      message("Build with OpenCV 2.x.x")
#      LIBS += -lopencv_legacy$${CVVERSION}
#
#    }

# LIBELAS #########################################################################################################################

#fuer libelas noetig:
$$LIBELAS {
    !build_pass:message(LIBELAS enabled!)
    DEFINES += LIBELAS
    QMAKE_CXXFLAGS += -msse3

    HEADERS += include/libelas/elasDescriptor.h \
        include/libelas/elas.h \
        include/libelas/elasFilter.h \
        include/libelas/elasMatrix.h \
        include/libelas/elasTriangle.h \
        include/libelas/elasImage.h \
        include/libelas/elasTimer.h
    SOURCES += src/libelas/elasDescriptor.cpp \
        src/libelas/elas.cpp \
        src/libelas/elasFilter.cpp \
        src/libelas/elasMatrix.cpp \
        src/libelas/elasTriangle.cpp
}


# QWT #########################################################################################################################
# zur Analyse der Daten / Analyse-Tab + Farbcode auf Marker in Kasernen-Versuchen

$$QWT {
    # DIE ENTKOPPLUNG VON QWT IST NICHT FERTIG IMPLEMENTIERT
    !build_pass:message(QWT (analysis) enabled!)
    DEFINES += QWT

    # in qwt.../install steht, dass folgendes gesetzt werden muss - insbesondere wegen debug/release mischungs problem
    DEFINES += QWT_DLL

    INCLUDEPATH += $${QWTPATH}/include

    win32:QMAKE_LIBDIR += $${QWTPATH}/lib
    unix:!macx:QMAKE_LIBDIR += $${QWTPATH}/lib
    # mgl QMAKE_LFLAGS_DEBUG setzen, um qwt5 fuer rlease und debug richtig nutzen!
    # QMAKE_LFLAGS_DEBUG += -lqwtd5
    # QMAKE_LFLAGS_RELEASE += -lqwt5
    #unix:QMAKE_LFLAGS_RPATH += -L$${QWTPATH}/lib
    #unix:LIBS += -lqwt
    CONFIG(debug, debug|release) {
      # only debug
      win32:LIBS += -lqwtd
      unix:!macx:LIBS += -lqwt
      macx: LIBS += -framework qwt
    } else {
      # only release
      win32:LIBS += -lqwt
      unix:!macx:LIBS += -lqwt
      macx: LIBS += -framework qwt
    }
    # obige Zeilen wurde von qmake editor grafische oberflaeche bzw. wenn neue dateien zu projekt hinzugefuegt werden falsch umgewandelt in:
    # zudem wurde bei src und include dateien doppelt eingetragen mit src/ und include/ davor
    # auch folgende Zeilen wurden hinzugefuegt: OTHER_FILES += src/semantic.cache include/semantic.cache
    #CONFIG(debug, debug|release): # only debug
    #LIBS += -lqwtd5
    #else:# only release
    #LIBS += -lqwt5

    #win32:QMAKE_LIBDIR += $${QWTPATH}/lib
    mac:QMAKE_LIBDIR += $${QWTPATH}
    mac:QMAKE_LIBDIR += $${QWTPATH}

    HEADERS += include/analysePlot.h
    SOURCES += src/analysePlot.cpp

} else {
    !build_pass:message(QWT (analysis) disabled!)
}
# PGR #########################################################################################################################

$$STEREO {
  !build_pass:message(Stereo enabled!)
  DEFINES += STEREO

  # fuer pgr stereo
  INCLUDEPATH += $${PGRPATH}/include
  QMAKE_LIBDIR += $${PGRPATH}/bin
  LIBS += -L$${PGRPATH}/lib
  #LIBS += -ldl
  LIBS += -ltriclops_v100
  #LIBS += -lpnmutils
} else {
  !build_pass:message(Stereo disabled!)
}

$$AVI {
  !build_pass:message(Avi enabled!)
  DEFINES += AVI
}

#########################################################################################################################


#include und src davorgeschrieben, da sonst qtcreator sie nicht anzeigt
HEADERS += include/petrack.h \
    include/helper.h \
    include/control.h \
    include/stereoWidget.h \
    include/colorRangeWidget.h \
    include/colorMarkerWidget.h \
    include/codeMarkerWidget.h \
    include/multiColorMarkerWidget.h \
    include/recognitionRoiItem.h \
    include/imageItem.h \
    include/logoItem.h \
    include/gridItem.h \
    include/coordItem.h \
    include/trackerItem.h \
    include/view.h \
    include/animation.h \
    include/player.h \
    include/vector.h \
    include/ellipse.h \
    include/markerCasern.h \
    include/markerJapan.h \
    include/markerHermes.h \
    include/markerColor.h \
    include/tracker.h \
    include/trackerReal.h \
    include/recognition.h \
    include/autoCalib.h \
    include/filter.h \
    include/brightContrastFilter.h \
    include/brightFilter.h \
    include/contrastFilter.h \
    include/blurFilter.h \
    include/borderFilter.h \
    include/backgroundFilter.h \
    include/calibFilter.h \
    include/calibStereoFilter.h \
    include/colorPlot.h \
    include/stereoItem.h \
    include/stereoContext.h \
    include/colorMarkerItem.h \
    include/codeMarkerItem.h \
    include/multiColorMarkerItem.h \
    include/backgroundItem.h \
    include/whitebalance.h \
    include/person.h \
    include/qtColorTriangle.h \
    include/swapFilter.h \
    include/extrCalibration.h \
    include/trackingRoiItem.h

SOURCES += src/petrack.cpp \
    src/helper.cpp \
    src/main.cpp \
    src/control.cpp \
    src/stereoWidget.cpp \
    src/colorRangeWidget.cpp \
    src/colorMarkerWidget.cpp \
    src/codeMarkerWidget.cpp \
    src/multiColorMarkerWidget.cpp \
    src/recognitionRoiItem.cpp \
    src/imageItem.cpp \
    src/logoItem.cpp \
    src/gridItem.cpp \
    src/coordItem.cpp \
    src/trackerItem.cpp \
    src/view.cpp \
    src/animation.cpp \
    src/player.cpp \
    src/vector.cpp \
    src/ellipse.cpp \
    src/markerCasern.cpp \
    src/markerHermes.cpp \
    src/markerJapan.cpp \
    src/markerColor.cpp \
    src/tracker.cpp \
    src/trackerReal.cpp \
    src/recognition.cpp \
    src/autoCalib.cpp \
    src/filter.cpp \
    src/brightContrastFilter.cpp \
    src/brightFilter.cpp \
    src/contrastFilter.cpp \
    src/blurFilter.cpp \
    src/borderFilter.cpp \
    src/backgroundFilter.cpp \
    src/calibFilter.cpp \
    src/calibStereoFilter.cpp \
    src/stereoContext.cpp \
    src/colorPlot.cpp \
    src/stereoItem.cpp \
    src/colorMarkerItem.cpp \
    src/codeMarkerItem.cpp \
    src/multiColorMarkerItem.cpp \
    src/backgroundItem.cpp \
    src/whitebalance.cpp \
    src/person.cpp \
    src/qtColorTriangle.cpp \
    src/swapFilter.cpp \
    src/extrCalibration.cpp \
    src/trackingRoiItem.cpp

$$STEREO{
    SOURCES += src/pgrAviFile.cpp
    HEADERS += include/pgrAviFile.h
    message("Include pgrAviFile")
}else{
    SOURCES += src/stereoAviFile.cpp
    HEADERS += include/stereoAviFile.h
    message("Include stereoAviFile")
}


$$AVI{
    SOURCES += src/aviFile.cpp
    HEADERS += include/aviFile.h
}else{
    SOURCES += src/aviFileWriter.cpp
    HEADERS += include/aviFileWriter.h
}


MOC_DIR = moc
UI_DIR = ui
UI_HEADERS_DIR = ui/include
UI_SOURCES_DIR = ui/src
FORMS += ui/control.ui \
         ui/stereo.ui \
         ui/colorRange.ui \
         ui/multiColorMarker.ui \
         ui/colorMarker.ui \
         ui/codeMarker.ui

RC_FILE = petrack.rc # damit petrack.ico als windows icon genommen wird
RESOURCES = icons/icons.qrc # damit daten in executable hineincompiliert wird

# alle Verzeichniss von Sourcen etc, wo Abhaengigkeiten gesucht werden sollen
DEPENDPATH += . include src ui ui/include

# fuer pgrAviFile
win32:LIBS += -lavifil32
win32:LIBS += -lmsvfw32
win64:LIBS += -lavifil32
win64:LIBS += -lmsvfw32

# only for debug function GetProcessMemoryInfo
win32:LIBS += -lpsapi
win64:LIBS += -lpsapi
