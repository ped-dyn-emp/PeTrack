target_include_directories(petrack_core PUBLIC ${CMAKE_CURRENT_LIST_DIR})

target_sources(petrack_core PRIVATE
    circularStack.h      
    compilerInformation.h
    helper.cpp           
    helper.h             
    logger.h             
    vector.cpp           
    vector.h
    importHelper.h
    importHelper.cpp
)
