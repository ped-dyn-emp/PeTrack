target_include_directories(petrack_core PUBLIC ${CMAKE_CURRENT_LIST_DIR})

target_sources(petrack_core PRIVATE
    animation.cpp          
    animation.h            
    autosave.cpp           
    autosave.h                   
    IO.cpp                 
    IO.h                   
    moCapPersonMetadata.cpp
    moCapPersonMetadata.h  
    skeletonTree.cpp       
    skeletonTree.h         
    skeletonTreeFactory.cpp
    skeletonTreeFactory.h  
)

if(NOT AVI)
    target_sources(petrack_core PRIVATE 
        aviFileWriter.cpp      
        aviFileWriter.h  
    )
endif()

if(NOT STEREO)
    target_sources(petrack_core PRIVATE 
        stereoAviFile.cpp      
        stereoAviFile.h   
    )
endif()