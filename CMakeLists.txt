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
# -DBUILD_BUNDLE=ON (default OFF) builds a MacOS Bundle for deployment
# -DFAIL_ON_WARNINGS=ON (default OFF) use Werror when building (for CI builds!)
#
# currently not supported:
# -DAVI=ON (default OFF)
#
################################################################################
# Project setup
################################################################################
cmake_minimum_required(VERSION 3.16 FATAL_ERROR)
project(petrack LANGUAGES CXX VERSION 0.10.5)
set(CMAKE_CXX_STANDARD 17)
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

# use, i.e. don't skip the full RPATH for the build tree
set(CMAKE_SKIP_BUILD_RPATH FALSE)

# when building, don't use the install RPATH already
# (but later on when installing)
set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
set(CMAKE_INSTALL_RPATH @executable_path/../Frameworks)

# add the automatically determined parts of the RPATH
# which point to directories outside the build tree to the install RPATH
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
################################################################################
# Optional features
################################################################################
option(STEREO "Use Point Grey's Triclops SDK and enable stereo support" OFF)
print_var(STEREO)

option(AVI "Use Avi File from Point Grey (only interal due to license)" OFF)
print_var(AVI)

option(BUILD_UNIT_TESTS "Build catch2 unit tests" OFF)
print_var(BUILD_UNIT_TESTS)

CMAKE_DEPENDENT_OPTION(USE_3RD_PARTY "Use the default libraries provided in 3rd party" ON WIN32 OFF)
print_var(USE_3RD_PARTY)

option(BUILD_BUNDLE "Builds a MacOS Bundle for deployment" OFF)
print_var(BUILD_BUNDLE)

option(FAIL_ON_WARNINGS "Handle compiler warnings as error (for CI use)" OFF)
print_var(FAIL_ON_WARNINGS)

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
    $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:-pedantic>
    $<$<AND:$<BOOL:${FAIL_ON_WARNINGS}>,$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>>:-Werror>
    $<$<CXX_COMPILER_ID:MSVC>:/W3>
    $<$<CXX_COMPILER_ID:MSVC>:/EHsc>
    $<$<CXX_COMPILER_ID:MSVC>:/permissive->
    $<$<AND:$<BOOL:${FAIL_ON_WARNINGS}>,$<CXX_COMPILER_ID:MSVC>>:/WX>
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
#if (AVI)
#  message(FATAL_ERROR "Currently the avi File from Point Grey needs to be disabled.")
#endif()

## Stero currently only allowed on windows
if (STEREO AND NOT WIN32)
  message(FATAL_ERROR "Currently the stereo feature is only supported on windows.")
endif()

if (NOT APPLE AND BUILD_BUNDLE)
  message(WARNING "The BUILD_BUNDLE is only available on MacOS. And will be ignored otherwise.")
endif()

################################################################################
# Dependencies
################################################################################
# spdlog
add_subdirectory(deps/spdlog)

# ezc3d
set(DUMMY ${BUILD_SHARED_LIBS})
set(BUILD_SHARED_LIBS FALSE CACHE BOOL "Choose if build should be a dynamic or static library")
add_subdirectory(deps/ezc3d/ EXCLUDE_FROM_ALL)
# disbale warnings for ezc3d on MSVC
target_compile_options(ezc3d PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/w>)
set(BUILD_SHARED_LIBS ${DUMMY})

# if win32 and mingw use in 3rd party
if (USE_3RD_PARTY)
  # Setting path to OpenCV
  set(OpenCV_DIR "${CMAKE_SOURCE_DIR}/3rdparty/windows/opencv-4.10.0")

  # Setting path to QWT
  set(QWT_ROOT_DIR "${CMAKE_SOURCE_DIR}/3rdparty/windows/Qwt-6.1.4_64bit")
  set(QWT_INCLUDE_DIR "${QWT_ROOT_DIR}/include")
  set(QWT_LIBRARY "${QWT_ROOT_DIR}/lib/libqwt.a")
endif ()

# Qt
find_package(
  Qt5 5.14
  COMPONENTS Widgets OpenGL Xml Core PrintSupport Concurrent Test
  REQUIRED
)
message("Building with Qt${QT_DEFAULT_MAJOR_VERSION} (${Qt5Core_VERSION_STRING})")

# OpenCV
find_package(
        OpenCV 4.10
        COMPONENTS core calib3d video videoio highgui imgproc objdetect
        REQUIRED
)
message("Building with OpenCV${OpenCV_VERSION_MAJOR} (${OpenCV_VERSION})")

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
list(APPEND CMAKE_AUTOUIC_SEARCH_PATHS
    "${CMAKE_SOURCE_DIR}/src/ui/dialogs"
    "${CMAKE_SOURCE_DIR}/src/ui/helper"
    "${CMAKE_SOURCE_DIR}/src/ui/main-windows"
    "./src")
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(AUTOGEN_BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}/${NAME}_autogen")

#**********************************************************
# Create library and executable                           *
#**********************************************************
get_git_info()
add_library(petrack_core STATIC)

set(author_file "${CMAKE_SOURCE_DIR}/.zenodo.json")

if (BUILD_BUNDLE)
    set(app_icon_macos "${CMAKE_SOURCE_DIR}/petrack.icns" "${CMAKE_SOURCE_DIR}/qt.conf")

    set_source_files_properties(${app_icon_macos} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
    set_source_files_properties("${author_file}" PROPERTIES MACOSX_PACKAGE_LOCATION "MacOS")

    add_executable(petrack
        MACOSX_BUNDLE
        src/main.cpp ${app_icon_macos} ${author_file}
    )
else()
    configure_file("${author_file}" "${CMAKE_CURRENT_BINARY_DIR}" COPYONLY)

    add_executable(petrack
        src/main.cpp
    )
endif()

target_link_libraries(petrack PRIVATE petrack_core git-info)
target_compile_definitions(petrack PUBLIC PETRACK_VERSION="${PROJECT_VERSION}")
target_compile_definitions(petrack PUBLIC COMPILE_OS="${CMAKE_SYSTEM_NAME}")

target_compile_options(petrack_core PRIVATE ${COMMON_COMPILE_OPTIONS})
target_compile_definitions(petrack_core PUBLIC COMPILE_OS="${CMAKE_SYSTEM_NAME}")

#**********************************************************
# Linking Qt                                              *
#**********************************************************
target_link_libraries(petrack_core PUBLIC
  Qt5::Widgets
  Qt5::OpenGL
  Qt5::Xml
  Qt5::Core
  Qt5::PrintSupport
  Qt5::Concurrent
)

target_include_directories(petrack_core PRIVATE "${CMAKE_SOURCE_DIR}/ui")

#*****************************************************************
# Linking OpenCV                                                 *
#*****************************************************************
# Debug Version of slows our debug down to a unusable degree
# so link to release always (not possible on MSVC due to
# debug crt and release crt not being compatible)
if(NOT ${MSVC})
    set_target_properties(${OpenCV_LIBS} PROPERTIES
        MAP_IMPORTED_CONFIG_DEBUG RELEASE
        MAP_IMPORTED_CONFIG_RELWITHDEBINFO RELEASE)
endif()
target_link_libraries(petrack_core PUBLIC ${OpenCV_LIBS})
#*****************************************************************
# Linking QWT                                                    *
#*****************************************************************
if(APPLE)
  target_link_directories(petrack_core PUBLIC "${QWT}")
else()
  target_compile_definitions(
    petrack_core PRIVATE
    $<$<AND:$<PLATFORM_ID:Windows>,$<BOOL:USE_3RD_PARTY>>:QWT QWT_DLL>)

  target_include_directories(petrack_core PUBLIC "${QWT_INCLUDE_DIR}")
  target_link_libraries(petrack_core PUBLIC Qwt::Qwt)
endif()

#*****************************************************************
# Linking spdlog                                                    *
#*****************************************************************
target_link_libraries(petrack_core PUBLIC spdlog::spdlog)
target_link_libraries(petrack PUBLIC spdlog::spdlog)

#*****************************************************************
# Linking ezc3d                                                  *
#*****************************************************************
target_link_libraries(petrack_core PUBLIC ezc3d)

#*************************************************************
# Handling of Options                                        *
#*************************************************************

if(AVI)
  target_compile_definitions(petrack_core PRIVATE AVI)
  target_sources(petrack_core PUBLIC
      "${CMAKE_SOURCE_DIR}/3rdparty/avifile/aviFile.h"
      "${CMAKE_SOURCE_DIR}/3rdparty/avifile/aviFile.cpp"
  )
  target_include_directories(petrack_core PRIVATE 3rdparty/avifile)
  target_link_libraries(petrack_core PUBLIC avifil32 msvfw32)
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
    add_subdirectory("${CMAKE_SOURCE_DIR}/deps/Catch2")
    add_subdirectory("${CMAKE_SOURCE_DIR}/deps/trompeloeil")

    add_subdirectory(${CMAKE_SOURCE_DIR}/tests/unit_test)
    target_link_libraries(petrack_tests PRIVATE petrack_core git-info)

    target_compile_definitions(petrack_tests PUBLIC PETRACK_VERSION="${PROJECT_VERSION}")

    target_link_libraries(petrack_tests PRIVATE Catch2::Catch2 Qt5::Test trompeloeil::trompeloeil)
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
add_subdirectory(src)

target_sources(petrack PRIVATE
  petrack.rc
  icons/icons.qrc
)

if(STEREO)
  message("Stereo enabled! (PGR)")
  target_compile_definitions(petrack_core PRIVATE STEREO)

  target_link_libraries(petrack_core PUBLIC avifil32 msvfw32 pnmutils_v100)
  target_link_libraries(petrack_core PUBLIC triclops_v100)
  target_link_directories(petrack_core PUBLIC  3rdparty/windows/triclops-3.4/bin64/)
  target_link_directories(petrack_core PUBLIC 3rdparty/windows/triclops-3.4/lib64/)
  target_include_directories(petrack_core PRIVATE 3rdparty/windows/triclops-3.4/include)
  target_include_directories(petrack_core PRIVATE 3rdparty/avifile)
  target_sources(petrack_core PRIVATE 3rdparty/avifile/pgrAviFile.cpp 3rdparty/avifile/pgrAviFile.h)
endif()


#####################################################
# Installer                                         #
#####################################################
include(install_helper)
