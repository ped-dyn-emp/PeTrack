cmake_minimum_required(VERSION 3.16)

project(petrack LANGUAGES CXX)
include(CMakeDependentOption)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

#**********************************************************
# Qt and Misc Stuff                                       *
#**********************************************************
set(CMAKE_AUTOUIC ON)
list(APPEND CMAKE_AUTOUIC_SEARCH_PATHS "${CMAKE_SOURCE_DIR}/ui" "./src")
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(
  Qt5
  COMPONENTS Widgets OpenGL Xml Core PrintSupport
  REQUIRED)

add_library(petrack_core STATIC)
add_executable(petrack src/main.cpp)
target_link_libraries(petrack PRIVATE petrack_core)
target_link_libraries(petrack_core PUBLIC Qt5::Widgets Qt5::OpenGL Qt5::Xml Qt5::Core Qt5::PrintSupport)


source_group(TREE .)

#**********************************************************
# Cache Variables                                         *
#**********************************************************

set(STEREO
    FALSE
    CACHE BOOL "Use Point Grey's Triclops SDK? (currently not supported)")

set(AVI
    FALSE
    CACHE BOOL "Use Avi File from Point Grey (currently not supported)")

set(QWT
    TRUE
    CACHE BOOL "Use Qwt (currently critical)")

set(LIBELAS
    TRUE
    CACHE BOOL "Use Libelas")

set(DISABLE_STEREO
    TRUE
    CACHE BOOL "Disable Stereo features (currently must be taken)")

set(CVPATH
    "../petrack/3rdparty/windows/opencv-4.2.0_64bit"
    CACHE
    PATH
    "Path to OpenCV installation (should usually not be needed to be changed)")

set(QWTPATH
    "../petrack/3rdparty/windows/Qwt-6.1.4_64bit"
    CACHE PATH "Path to Qwt installation. Ususally shouln't be altered.")

set(BUILD_UNIT_TESTS
    OFF
    CACHE BOOL "Whether to build or not to build Catch2 Unit Tests")

cmake_dependent_option(BUILD_UNIT_TESTS_WITH_LLD
    "Use -fuse-ld=lld flag for linking unit-tests (need lld to be installed)"
    OFF
    "BUILD_UNIT_TESTS" OFF)

#*****************************************************************
# APPLIES ALWAYS                                                 *
#*****************************************************************
target_compile_options(petrack_core PRIVATE "-fpermissive")
target_compile_definitions(petrack_core PUBLIC STEREO_DISABLED)

set(OpenCV_DIR "${CVPATH}")
find_package(OpenCV REQUIRED)
message("Building with OpenCV${OpenCV_VERSION_MAJOR}(${OpenCV_VERSION})")

add_library(opencv INTERFACE IMPORTED)
# Debug Version of slows our debug down to a unusable degree
set_target_properties(${OpenCV_LIBS} PROPERTIES
    MAP_IMPORTED_CONFIG_DEBUG RELEASE
    MAP_IMPORTED_CONFIG_RELWITHDEBINFO RELEASE)

target_link_libraries(petrack_core PUBLIC ${OpenCV_LIBS})

target_include_directories(petrack_core PRIVATE "../ui")
#*************************************************************
# Handling of Options                                        *
#*************************************************************


if(BUILD_UNIT_TESTS)
    enable_testing()
    find_package(Qt5Test REQUIRED)
    find_package(trompeloeil REQUIRED HINTS "./3rdparty/trompeloeil/lib/cmake/trompeloeil")
    add_subdirectory(./tests/unit_test)
    target_link_libraries(petrack_tests PRIVATE petrack_core)
    add_subdirectory(./3rdparty/Catch2)
    target_link_libraries(petrack_tests PRIVATE Catch2::Catch2 Qt5::Test trompeloeil)
    target_include_directories(petrack_tests PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/petrack_core_autogen/include")

    if(BUILD_UNIT_TESTS_WITH_LLD)
        target_compile_options(petrack_tests PRIVATE "-fuse-ld=lld")
        target_link_options(petrack_tests PRIVATE "-fuse-ld=lld")
    endif(BUILD_UNIT_TESTS_WITH_LLD)
endif(BUILD_UNIT_TESTS)

if(LIBELAS)
  message("LIBELAS enabled!")
  target_compile_definitions(petrack_core PRIVATE LIBELAS)
  target_sources(petrack_core PRIVATE
    src/libelas/elasDescriptor.cpp
    src/libelas/elas.cpp
    src/libelas/elasFilter.cpp
    src/libelas/elasMatrix.cpp
    src/libelas/elasTriangle.cpp
  )
endif(LIBELAS)


if(QWT)
  message("Qwt (analysis) enabled!")
  # in qwt.../install steht, dass QWT_DLL gesetzt werden muss - insbesondere
  # wegen debug/release mischungs problem
  target_compile_definitions(
    petrack_core PRIVATE
    "QWT"
    "QWT_DLL")
  target_include_directories(petrack_core PUBLIC "${QWTPATH}/include")
  target_link_directories(petrack_core PUBLIC "${QWTPATH}/lib")
  target_link_libraries(petrack_core PUBLIC qwt$<$<CONFIG:Debug>:d>)
  target_sources(petrack_core PRIVATE src/analysePlot.cpp)
else()
  message("Qwt disabled!")
endif(QWT)


if(AVI)
  message("AVI enabled!")
  target_compile_definitions(petrack_core PRIVATE AVI)
  target_sources(petrack_core "src/aviFile.cpp")
else()
  target_sources(petrack_core PRIVATE "src/aviFileWriter.cpp")
endif(AVI)


# WIN32 steht für Windows allgemein, nicht nur 32Bit
if(WIN32)
  target_link_libraries(petrack_core PUBLIC psapi)
endif(WIN32)

#**************************************************************
# SOURCES                                                     *
#**************************************************************

# An sich wäre nur target_include_diretories notwendig, aber AUTOUIC
# sucht nur in target source nach, ob ein ui header included wurde.
# Aus bequemlichkeit dann einfach alle aus der alten .pro rüberkopiert
target_include_directories(petrack_core PUBLIC "./include")
target_sources(petrack_core PRIVATE
    include/petrack.h
    include/helper.h
    include/control.h
    include/stereoWidget.h
    include/colorRangeWidget.h
    include/colorMarkerWidget.h
    include/codeMarkerWidget.h
    include/multiColorMarkerWidget.h
    include/recognitionRoiItem.h
    include/imageItem.h
    include/logoItem.h
    include/gridItem.h
    include/coordItem.h
    include/trackerItem.h
    include/view.h
    include/animation.h
    include/player.h
    include/vector.h
    include/ellipse.h
    include/markerCasern.h
    include/markerJapan.h
    include/markerHermes.h
    include/markerColor.h
    include/tracker.h
    include/trackerReal.h
    include/recognition.h
    include/autoCalib.h
    include/filter.h
    include/brightContrastFilter.h
    include/brightFilter.h
    include/contrastFilter.h
    include/blurFilter.h
    include/borderFilter.h
    include/backgroundFilter.h
    include/calibFilter.h
    include/calibStereoFilter.h
    include/colorPlot.h
    include/stereoItem.h
    include/stereoContext.h
    include/colorMarkerItem.h
    include/codeMarkerItem.h
    include/multiColorMarkerItem.h
    include/backgroundItem.h
    include/whitebalance.h
    include/person.h
    include/qtColorTriangle.h
    include/swapFilter.h
    include/extrCalibration.h
    include/trackingRoiItem.h
    include/analysePlot.h
    )


target_sources(petrack_core PRIVATE
    src/helper.cpp
    src/control.cpp
    src/petrack.cpp
    src/stereoWidget.cpp
    src/colorRangeWidget.cpp
    src/colorMarkerWidget.cpp
    src/codeMarkerWidget.cpp
    src/multiColorMarkerWidget.cpp
    src/recognitionRoiItem.cpp
    src/imageItem.cpp
    src/logoItem.cpp
    src/gridItem.cpp
    src/coordItem.cpp
    src/trackerItem.cpp
    src/view.cpp
    src/animation.cpp
    src/player.cpp
    src/vector.cpp
    src/ellipse.cpp
    src/markerCasern.cpp
    src/markerHermes.cpp
    src/markerJapan.cpp
    src/markerColor.cpp
    src/tracker.cpp
    src/trackerReal.cpp
    src/recognition.cpp
    src/autoCalib.cpp
    src/filter.cpp
    src/brightContrastFilter.cpp
    src/brightFilter.cpp
    src/contrastFilter.cpp
    src/blurFilter.cpp
    src/borderFilter.cpp
    src/backgroundFilter.cpp
    src/calibFilter.cpp
    src/calibStereoFilter.cpp
    src/stereoContext.cpp
    src/colorPlot.cpp
    src/stereoItem.cpp
    src/colorMarkerItem.cpp
    src/codeMarkerItem.cpp
    src/multiColorMarkerItem.cpp
    src/backgroundItem.cpp
    src/whitebalance.cpp
    src/person.cpp
    src/qtColorTriangle.cpp
    src/swapFilter.cpp
    src/extrCalibration.cpp
    src/trackingRoiItem.cpp
    ui/codeMarker.ui
    ui/colorMarker.ui
    ui/colorRange.ui
    ui/multiColorMarker.ui
    ui/stereo.ui
    ui/control.ui
)

target_sources(petrack PRIVATE
  petrack.rc
  icons/icons.qrc)


#*****************************************************************************
# CODE AUS DER ALTEN .PRO DATEI, DER NOCH NICHT IN DIE CMAKE EINGEBAUT WURDE *
#*****************************************************************************

if(${STEREO})
  # Should not happen at the moment Code below was at this point in .pro
  # $$STEREO { !build_pass:message(Stereo enabled!) DEFINES += STEREO

  # # fuer pgr stereo INCLUDEPATH += $${PGRPATH}/include QMAKE_LIBDIR +=
  # $${PGRPATH}/bin LIBS += -L$${PGRPATH}/lib #LIBS += -ldl LIBS +=
  # -ltriclops_v100 #LIBS += -lpnmutils } else { !build_pass:message(Stereo
  # disabled!) }

  # Auch das spielt hier eine Rolle:

  # $$STEREO{ SOURCES += src/pgrAviFile.cpp HEADERS += include/pgrAviFile.h
  # message("Include pgrAviFile") }else{ SOURCES += src/stereoAviFile.cpp
  # HEADERS += include/stereoAviFile.h message("Include stereoAviFile") }
endif(${STEREO})

# Sollte das nicht eigentlich nur aktiv sein, wenn Stereo gesetzt wurde?

# # fuer pgrAviFile win32:LIBS += -lavifil32 win32:LIBS += -lmsvfw32 win64:LIBS
# += -lavifil32 win64:LIBS += -lmsvfw32
