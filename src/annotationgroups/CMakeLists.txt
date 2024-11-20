target_include_directories(petrack_core PUBLIC ${CMAKE_CURRENT_LIST_DIR})

target_sources(petrack_core PRIVATE
        annotationGrouping.h
        annotationGrouping.cpp
        annotationGroupManager.cpp
        annotationGroupManager.h
        annotationGroupTreeItem.cpp
        annotationGroupTreeItem.h
        annotationGroupTreeModel.cpp
        annotationGroupTreeModel.h
        annotationGroupIO.h
        annotationGroupIO.cpp
)