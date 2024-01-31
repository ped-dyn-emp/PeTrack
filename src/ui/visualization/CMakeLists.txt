target_include_directories(petrack_core PUBLIC ${CMAKE_CURRENT_LIST_DIR})

target_sources(petrack_core PRIVATE
    backgroundItem.cpp      
    backgroundItem.h        
    codeMarkerItem.cpp      
    codeMarkerItem.h        
    colorMarkerItem.cpp     
    colorMarkerItem.h       
    coordItem.cpp           
    coordItem.h             
    gridItem.cpp            
    gridItem.h              
    imageItem.cpp           
    imageItem.h             
    logoItem.cpp            
    logoItem.h              
    moCapController.cpp     
    moCapController.h       
    moCapItem.cpp           
    moCapItem.h             
    moCapPerson.cpp         
    moCapPerson.h           
    multiColorMarkerItem.cpp
    multiColorMarkerItem.h  
    roiItem.cpp             
    roiItem.h               
    stereoItem.cpp          
    stereoItem.h            
    trackerItem.cpp         
    trackerItem.h 
)