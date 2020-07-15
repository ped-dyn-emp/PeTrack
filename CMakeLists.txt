# Documentation: Some useful options: cmake [options] [path to CMakeLists.txt]
# helpful cmake options:
# -DCMAKE_CXX_COMPILER=clang++ (setting the used C++ compiler)
# -DCMAKE_BUILD_TYPE=Debug (default Release)
# -DCMAKE_VERBOSE_MAKEFILE=ON (default OFF)
# -DCMAKE_PREFIX_PATH=[paths where additionally to search for libraries etc]
#
# petrack options:
# -DUSE_3RD_PARTY=ON (default ON on Windows, OFF else) use the libraries provided in 3rdparty
# -DBUILD_UNIT_TESTS=ON (default ON) for unit tests
#
# currently not supported:
# -DSTEREO=ON (default OFF)
# -DAVI=ON (default OFF)
# -DDISABLE_STEREO=ON (default OFF)
#
################################################################################
# Project setup
################################################################################
cmake_minimum_required(VERSION 3.16 FATAL_ERROR)
project(petrack LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake_modules")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(helper_functions)

# Set default build type to release
set(default_build_type "Release")
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to '${default_build_type}' as none was specified.")
  set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE
      STRING "Choose the type of build." FORCE)
endif()

check_prefix_path()

include(CMakeDependentOption)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

################################################################################
# Optional features
################################################################################
option(STEREO "Use Point Grey's Triclops SDK? (currently not supported)" OFF)
print_var(STEREO)

option(AVI "Use Avi File from Point Grey (currently not supported)" OFF)
print_var(AVI)

option(LIBELAS "Use Libelas" OFF)
print_var(LIBELAS)

option(DISABLE_STEREO "Disable Stereo features (currently must be taken)" ON)
print_var(DISABLE_STEREO)

option(BUILD_UNIT_TESTS "Build catch2 unit tests" OFF)
print_var(BUILD_UNIT_TESTS)

CMAKE_DEPENDENT_OPTION(USE_3RD_PARTY "Use the default libraries provided in 3rd party" ON WIN32 OFF)
print_var(USE_3RD_PARTY)

################################################################################
# Compilation flags
################################################################################
# Note: Setting global compile flags via CMAKE_CXX_FLAGS has the drawback that
#       generator expressions cannot be used. This leads to all kind of
#       conditional adding of flags. It is generally preferable to use generator
#       expresssions.
#
# WARNING: Do not break the lines, each option has to be on its own line or
#          CMake will enclose multiple flags in '' which the compiler then
#          treats as a single flag and does not understand.
list(APPEND COMMON_COMPILE_OPTIONS
    $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:-Wall>
    $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:-Wextra>
    $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:-fdiagnostics-color=always>
    $<$<CXX_COMPILER_ID:MSVC>:/W2>
    $<$<CXX_COMPILER_ID:MSVC>:/EHsc>
)

################################################################################
# Check some compile options if allowed
################################################################################
# Using the default 3rd party libraries only allowed on Windows with MinGW
if (USE_3RD_PARTY AND NOT (WIN32 AND MINGW))
  message(FATAL_ERROR "The provided 3rd party libraries can only be used on
  Windows with the MinGW compiler. Please install the required libraries
  yourself." )
endif()

# Check for currently unavailable options
if (AVI)
  message(FATAL_ERROR "Currently the avi File from Point Grey needs to be disabled.")
endif()

# Stero only allowed
if (NOT DISABLE_STEREO OR STEREO)
  message(FATAL_ERROR "Currently the stereo feature needs to be disabled.")
endif()

################################################################################
# Dependencies
################################################################################
# if win32 and mingw use in 3rd party
if (USE_3RD_PARTY)
  # Setting path to OpenCV
  set(OpenCV_DIR "${CMAKE_SOURCE_DIR}/3rdparty/windows/opencv-4.2.0_64bit")

  # Setting path to QWT
  set(QWT_ROOT_DIR "${CMAKE_SOURCE_DIR}/3rdparty/windows/Qwt-6.1.4_64bit")
  set(QWT_INCLUDE_DIR "${QWT_ROOT_DIR}/include")
  set(QWT_LIBRARY "${QWT_ROOT_DIR}/lib/libqwt.a")
endif ()

# Qt
find_package(
  Qt5 5.14
  COMPONENTS Widgets OpenGL Xml Core PrintSupport
  REQUIRED)

# OpenCV
find_package(OpenCV 4.0.0 REQUIRED )
message("Building with OpenCV${OpenCV_VERSION_MAJOR}(${OpenCV_VERSION})")

# QWT
if(APPLE)
    set(CMAKE_FIND_FRAMEWORK ONLY)
    find_library(QWT
        NAMES qwt
        HINTS /usr/local/opt/qwt/lib/
        REQUIRED)
    if(QWT)
        include_directories(${QWT}/Headers)
        link_libraries(${QWT})
    endif()
else()
  find_package(Qwt REQUIRED)
endif()

#**********************************************************
# Qt and Misc Stuff                                       *
#**********************************************************
set(CMAKE_AUTOUIC ON)
list(APPEND CMAKE_AUTOUIC_SEARCH_PATHS "${CMAKE_SOURCE_DIR}/ui" "./src")
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

#**********************************************************
# Create library and exectuable                           *
#**********************************************************
add_library(petrack_core STATIC)
add_executable(petrack src/main.cpp)
target_link_libraries(petrack PRIVATE petrack_core)

# TODO we should remove this compile option and be more strict
target_compile_options(petrack_core PRIVATE ${COMMON_COMPILE_OPTIONS})
target_compile_definitions(petrack_core PUBLIC STEREO_DISABLED)

#**********************************************************
# Linking Qt                                              *
#**********************************************************
target_link_libraries(petrack_core PUBLIC
  Qt5::Widgets
  Qt5::OpenGL
  Qt5::Xml
  Qt5::Core
  Qt5::PrintSupport
)

target_include_directories(petrack_core PRIVATE "${CMAKE_SOURCE_DIR}/ui")

#*****************************************************************
# Linking OpenCV                                                 *
#*****************************************************************
add_library(opencv INTERFACE IMPORTED)

# Debug Version of slows our debug down to a unusable degree
set_target_properties(${OpenCV_LIBS} PROPERTIES
    MAP_IMPORTED_CONFIG_DEBUG RELEASE
    MAP_IMPORTED_CONFIG_RELWITHDEBINFO RELEASE)

target_link_libraries(petrack_core PUBLIC ${OpenCV_LIBS})

#*****************************************************************
# Linking QWT                                                    *
#*****************************************************************
if(APPLE)
  target_link_directories(petrack_core PUBLIC ${QWT})
else()
  target_compile_definitions(
    petrack_core PRIVATE
    $<$<AND:$<PLATFORM_ID:Windows>,$<BOOL:USE_3RD_PARTY>>:QWT QWT_DLL>)

  target_include_directories(petrack_core PUBLIC ${QWT_INCLUDE_DIR})
  target_link_libraries(petrack_core PUBLIC ${QWT_LIBRARY})
endif()

target_sources(petrack_core PRIVATE src/analysePlot.cpp)

#*************************************************************
# Handling of Options                                        *
#*************************************************************
if(LIBELAS)
  target_compile_definitions(petrack_core PRIVATE LIBELAS)
  add_library(elas STATIC
    src/libelas/elasDescriptor.cpp
    src/libelas/elas.cpp
    src/libelas/elasFilter.cpp
    src/libelas/elasMatrix.cpp
    src/libelas/elasTriangle.cpp
  )
  target_include_directories(elas PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
  target_compile_options(elas PRIVATE "-fpermissive")
  target_link_libraries(petrack_core PRIVATE elas)
endif(LIBELAS)

# TODO currently not available
if(AVI)
  target_compile_definitions(petrack_core PRIVATE AVI)
  target_sources(petrack_core PUBLIC
    include/aviFile.h
    src/aviFile.cpp
  )
else()
  target_sources(petrack_core PRIVATE "src/aviFileWriter.cpp")
endif(AVI)

# WIN32 steht für Windows allgemein, nicht nur 32Bit
if(WIN32)
  target_link_libraries(petrack_core PUBLIC psapi)
endif(WIN32)

################################################################################
# petrack_core unit tests
################################################################################
if(BUILD_UNIT_TESTS)
    enable_testing()
    if (USE_3RD_PARTY)
        add_subdirectory("${CMAKE_SOURCE_DIR}/3rdparty/Catch2")
    else()
      find_package(Catch2 REQUIRED)
    endif()

    find_package(Qt5Test REQUIRED)
    find_package(trompeloeil REQUIRED
      HINTS "${CMAKE_SOURCE_DIR}/3rdparty/trompeloeil/lib/cmake/trompeloeil")

    add_subdirectory(${CMAKE_SOURCE_DIR}/tests/unit_test)
    target_link_libraries(petrack_tests PRIVATE petrack_core)

    target_link_libraries(petrack_tests PRIVATE Catch2::Catch2 Qt5::Test trompeloeil)
    target_include_directories(petrack_tests PRIVATE
      "${CMAKE_CURRENT_BINARY_DIR}/petrack_core_autogen/include")

    if(BUILD_UNIT_TESTS_WITH_LLD)
        target_compile_options(petrack_tests PRIVATE "-fuse-ld=lld")
        target_link_options(petrack_tests PRIVATE "-fuse-ld=lld")
    endif(BUILD_UNIT_TESTS_WITH_LLD)
endif(BUILD_UNIT_TESTS)

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