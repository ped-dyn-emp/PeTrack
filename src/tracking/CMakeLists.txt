target_include_directories(petrack_core PUBLIC ${CMAKE_CURRENT_LIST_DIR})

target_sources(petrack_core PRIVATE
    trackerConstants.h
    trackPoint.cpp
    trackPoint.h
    trackPerson.cpp
    trackPerson.h
    tracker.cpp
    tracker.h
    trackerReal.cpp
    trackerReal.h
    trcparser.cpp
    trcparser.h
)
