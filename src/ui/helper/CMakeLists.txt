target_include_directories(petrack_core PUBLIC ${CMAKE_CURRENT_LIST_DIR})

target_sources(petrack_core PRIVATE
    pdoublespinbox.cpp 
    pdoublespinbox.h   
    pGroupBox.cpp      
    pGroupBox.h        
    pGroupBox.ui       
    pGroupBoxFrame.cpp 
    pGroupBoxFrame.h   
    pGroupBoxFrame.ui  
    pSlider.cpp        
    pSlider.h          
    pspinbox.cpp       
    pspinbox.h         
    qtColorTriangle.cpp
    qtColorTriangle.h  
)