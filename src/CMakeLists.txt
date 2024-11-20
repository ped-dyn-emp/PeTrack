target_include_directories(petrack_core PUBLIC ${CMAKE_CURRENT_LIST_DIR})

target_sources(petrack_core PRIVATE
    frameRange.h
    manualTrackpointMover.h
    manualTrackpointMover.cpp
    personStorage.h
    personStorage.cpp
    petrack.cpp
    petrack.h
    plausibility.h
    plausibility.cpp
    player.cpp
    player.h
)

add_subdirectory(calibration)
add_subdirectory(filter)
add_subdirectory(util)
add_subdirectory(IO)
add_subdirectory(recognition)
add_subdirectory(ui)
add_subdirectory(tracking)
add_subdirectory(annotationgroups)