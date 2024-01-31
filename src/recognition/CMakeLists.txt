target_include_directories(petrack_core PUBLIC ${CMAKE_CURRENT_LIST_DIR})

target_sources(petrack_core PRIVATE
    ellipse.cpp     
    ellipse.h       
    markerCasern.cpp
    markerCasern.h  
    markerHermes.cpp
    markerHermes.h  
    markerJapan.cpp 
    markerJapan.h   
    person.cpp      
    person.h        
    recognition.cpp 
    recognition.h   
)