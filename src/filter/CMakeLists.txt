target_include_directories(petrack_core PUBLIC ${CMAKE_CURRENT_LIST_DIR})

target_sources(petrack_core PRIVATE
    backgroundFilter.h
    backgroundFilter.cpp
    blurFilter.h
    blurFilter.cpp
    borderFilter.h
    borderFilter.cpp
    brightContrastFilter.h
    brightContrastFilter.cpp
    calibFilter.h
    calibFilter.cpp
    calibStereoFilter.h
    calibStereoFilter.cpp
    filter.h
    filter.cpp
    swapFilter.h
    swapFilter.cpp
)