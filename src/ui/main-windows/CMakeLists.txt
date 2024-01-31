target_include_directories(petrack_core PUBLIC ${CMAKE_CURRENT_LIST_DIR})

target_sources(petrack_core PRIVATE
    alignmentGridBox.cpp   
    alignmentGridBox.h     
    alignmentGridBox.ui    
    analysePlot.cpp        
    analysePlot.h          
    colorPlot.cpp          
    colorPlot.h            
    control.cpp            
    control.h              
    control.ui             
    coordinateStructs.h    
    coordinateSystemBox.cpp
    coordinateSystemBox.h  
    coordinateSystemBox.ui 
    correction.cpp         
    correction.h           
    correction.ui          
    extrinsicBox.cpp       
    extrinsicBox.h         
    extrinsicBox.ui        
    filterBeforeBox.cpp    
    filterBeforeBox.h      
    filterBeforeBox.ui     
    intrinsicBox.cpp       
    intrinsicBox.h         
    intrinsicBox.ui        
    view.cpp               
    view.h    
)