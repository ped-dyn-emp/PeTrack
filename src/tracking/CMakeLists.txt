target_include_directories(petrack_core PUBLIC ${CMAKE_CURRENT_LIST_DIR})

target_sources(petrack_core PRIVATE
    tracker.cpp    
    tracker.h      
    trackerReal.cpp
    trackerReal.h  
)