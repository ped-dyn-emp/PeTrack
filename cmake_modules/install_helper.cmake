#########################################################
#                    INSTALL                            #
#########################################################
include(InstallRequiredSystemLibraries)
include(GNUInstallDirs)

install(TARGETS petrack
    BUNDLE DESTINATION .
)

if (WIN32)
    # Install Qt dlls
    set (WINDEPLOYQT_APP \"${Qt6Core_DIR}/../../../bin/windeployqt\")
    install(CODE "
        message(\"\${CMAKE_INSTALL_PREFIX}/petrack.exe\")
        execute_process(COMMAND ${WINDEPLOYQT_APP} \"\${CMAKE_INSTALL_PREFIX}/bin/petrack.exe\")
    ")
    set(OFFSCREEN_PLUGIN "${Qt6Core_DIR}/../../../plugins/platforms/qoffscreen.dll")

    install(FILES "${OpenCV_LIB_PATH}/../bin/opencv_videoio_ffmpeg${OpenCV_VERSION_MAJOR}${OpenCV_VERSION_MINOR}${OpenCV_VERSION_PATCH}_64.dll" DESTINATION ${CMAKE_INSTALL_BINDIR})

    # NOTE: Paths might be platform specific
    install(DIRECTORY "${Qt6Core_DIR}/../../../../../Licenses" DESTINATION "Licenses/Qt_Licenses")
    install(DIRECTORY "${OpenCV_DIR}/etc/licenses" DESTINATION "Licenses/OpenCV_Licenses")
    install(FILES "${OpenCV_DIR}/LICENSE/" DESTINATION "Licenses/OpenCV_Licenses")
    install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/deps/ezc3d/LICENSE" DESTINATION "Licenses/ezc3d_license/")
    install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/deps/spdlog/LICENSE" DESTINATION "Licenses/spdlog_license/")
    install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/deps/hdf5/COPYING" DESTINATION "Licenses/hdf5_license/")
    install(FILES "${CMAKE_SOURCE_DIR}/.zenodo.json" DESTINATION "bin")
    install(FILES "${OFFSCREEN_PLUGIN}" DESTINATION "bin/platforms")

    # install Qwt and OpenCV
    install(CODE "
        include(BundleUtilities)
        fixup_bundle(\"\${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}/petrack${CMAKE_EXECUTABLE_SUFFIX}\"  \"\" \"\")
    ")

elseif(APPLE)
    set_target_properties(petrack PROPERTIES
        MACOSX_BUNDLE_BUNDLE_VERSION       "${PROJECT_VERSION}"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}"
    )

    set_target_properties(petrack PROPERTIES
        INSTALL_RPATH @executable_path/../Frameworks
    )

    install(FILES "${OpenCV_DIR}/../../libopencv_videoio_ffmpeg.so" DESTINATION "petrack.app/Contents/Frameworks")

    install(CODE [[
        include(BundleUtilities)

        # the output paths for our installation
        set (_appdir "${CMAKE_INSTALL_PREFIX}/petrack.app")
        set (_outdir "${_appdir}/Contents/PlugIns/platforms")

        # copy the needed plugins
        file(INSTALL DESTINATION "${_outdir}"
            TYPE FILE FILES "$<TARGET_FILE:Qt6::QCocoaIntegrationPlugin>" "$<TARGET_FILE:Qt6::QOffscreenIntegrationPlugin>")

        fixup_bundle(
            "${_appdir}/Contents/MacOS/petrack"
            "${_outdir}/$<TARGET_FILE_NAME:Qt6::QCocoaIntegrationPlugin>;${_outdir}/$<TARGET_FILE_NAME:Qt6::QOffscreenIntegrationPlugin>"
            ""
        )
    ]] COMPONENT Runtime)

endif()

install(FILES "${CMAKE_SOURCE_DIR}/ReadMe.md" "${CMAKE_SOURCE_DIR}/LICENSE" DESTINATION ".")

##################################################################
#                         PACKAGE                                #
##################################################################

set(CPACK_PACKAGE_FILE_NAME "petrack-installer-${PROJECT_VERSION}")
set(CPACK_PACKAGE_VENDOR "Forschungszentrum Juelich GmbH")
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERISON_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERISON_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERISON_PATCH})
set(CPACK_PACKAGE_DESCRIPTION "PeTrack is a software for the automated extraction of pedestrian trajectories from videos.")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_SOURCE_DIR}/ReadMe.md")
set(CPACK_PACKAGE_HOMEPAGE_URL "https://www.fz-juelich.de/ias/ias-7/EN/Expertise/Software/PeTrack/petrack.html")
set(CPACK_RESOURCE_FILE_README "${CMAKE_SOURCE_DIR}/ReadMe.md")
set(CPACK_PACKAGE_EXECUTABLES "petrack;PeTrack")
set(CPACK_MONOLITHIC_INSTALL TRUE)
set(CPACK_CREATE_DESKTOP_LINKS petrack)
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")

if(WIN32)
    set(CPACK_GENERATOR "NSIS" CACHE STRING "Generator used by CPack")

    set(CPACK_NSIS_MUI_ICON "${CMAKE_SOURCE_DIR}\\\\petrack.ico")
    set(CPACK_NSIS_MODIFY_PATH ON)
    set(CPACK_NSIS_DISPLAY_NAME "PeTrack")
    set(CPACK_NSIS_PACKAGE_NAME "PeTrack")
    set(CPACK_NSIS_INSTALLED_ICON_NAME "${CMAKE_SOURCE_DIR}\\\\petrack.ico")
    set(CPACK_NSIS_HELP_LINK "https://www.fz-juelich.de/ias/ias-7/EN/Expertise/Software/PeTrack/petrack.html")
    set(CPACK_NSIS_URL_INFO_ABOUT ${CPACK_NSIS_HELP_LINK})
    set(CPACK_NSIS_CONTACT "petrack@fz-juelich.de")
    set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)

    set(CPACK_NSIS_EXTRA_INSTALL_COMMANDS "
      WriteRegStr HKCU 'Software\\\\Forschungszentrum Juelich GmbH\\\\PeTrack' 'fileExtension' 'pet'
      WriteRegStr HKCU 'Software\\\\Forschungszentrum Juelich GmbH\\\\PeTrack' 'home' '$INSTDIR\\\\bin'
      WriteRegStr HKCR '.pet' '' 'petfile'
      WriteRegStr HKCR 'petfile' '' 'PeTrack Project File'
      WriteRegStr HKCR 'petfile\\\\DefaultIcon' '' '$INSTDIR\\\\bin\\\\petrack.exe,0'
      WriteRegStr HKCR 'petfile\\\\shell\\\\open\\\\command' '' '$INSTDIR\\\\bin\\\\petrack.exe \\\"%1\\\"'
      WriteRegStr HKLM 'Software\\\\RegisteredApplications' 'petrack' '$INSTDIR\\\\petrack.exe'
      WriteRegStr HKCU 'Software\\\\RegisteredApplications' 'petrack' '$INSTDIR\\\\petrack.exe'
      WriteRegStr HKCU 'Software\\\\Microsoft\\\\Windows\\\\CurrentVersion\\\\Explorer\\\\FileExts\\\\.pet\\\\OpenWithList' 'a' '$INSTDIR\\\\bin\\\\petrack.exe'
      ")
    set(CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS "
      EnumRegKey $1 HKCU 'Software\\\\Forschungszentrum Juelich GmbH' 1
      StrCmp $1 '' 0 +3
      DeleteRegKey HKCU 'Software\\\\Forschungszentrum Juelich GmbH'
      goto +2
      DeleteRegKey HKCU 'Software\\\\Forschungszentrum Juelich GmbH\\\\PeTrack'
      DeleteRegKey HKCR '.pet'
      DeleteRegKey HKCR 'petfile'
      DeleteRegValue HKLM 'Software\\\\RegisteredApplications' 'petrack'
      DeleteRegValue HKCU 'Software\\\\RegisteredApplications' 'petrack'
      DeleteRegValue HKLM 'SOFTWARE\\\\WOW6432Node\\\\RegisteredApplications' 'petrack'
      DeleteRegKey HKCU 'Software\\\\Microsoft\\\\Windows\\\\CurrentVersion\\\\Explorer\\\\FileExts\\\\.pet'
    ")
elseif(APPLE)
    set(CPACK_GENERATOR "DragNDrop" CACHE STRING "Generator used by CPack")

    SET(MACOSX_BUNDLE_ICON_FILE petrack.icns)
    SET(MACOSX_BUNDLE_COPYRIGHT "Copyright (c) 2015-2021 Forschungszentrum Juelich. All rights reserved.")
    SET(MACOSX_BUNDLE_INFO_STRING "PeTrack is a software for the automated extraction of pedestrian trajectories from videos.")
    SET(MACOSX_BUNDLE_BUNDLE_NAME "PeTrack")
    SET(MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}")
    SET(MACOSX_BUNDLE_LONG_VERSION_STRING "version ${PROJECT_VERSION}")
    SET(MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}")
endif()

include(CPack)
