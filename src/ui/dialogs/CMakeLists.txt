target_include_directories(petrack_core PUBLIC ${CMAKE_CURRENT_LIST_DIR})

target_sources(petrack_core PRIVATE
        about.ui
        aboutDialog.cpp
        aboutDialog.h
        autosaveSettings.cpp
        autosaveSettings.h
        autosaveSettings.ui
        codeMarker.ui
        codeMarkerWidget.cpp
        codeMarkerWidget.h
        colorMarker.ui
        colorMarkerWidget.cpp
        colorMarkerWidget.h
        colorRange.ui
        colorRangeWidget.cpp
        colorRangeWidget.h
        editMoCapDialog.cpp
        editMoCapDialog.h
        editMoCapDialog.ui
        logwindow.cpp
        logwindow.h
        logwindow.ui
        moCapEditingWidget.cpp
        moCapEditingWidget.h
        moCapEditingWidget.ui
        moCapSelectionWidget.cpp
        moCapSelectionWidget.h
        moCapSelectionWidget.ui
        multiColorMarker.ui
        multiColorMarkerWidget.cpp
        multiColorMarkerWidget.h
        openMoCapDialog.cpp
        openMoCapDialog.h
        openMoCapDialog.ui
        pMessageBox.cpp
        pMessageBox.h
        stereo.ui
        stereoWidget.cpp
        stereoWidget.h
        createAnnotationGroupDialog.ui
        createAnnotationGroupDialog.cpp
        createAnnotationGroupDialog.h
        editAnnotationGroupDialog.ui
        editAnnotationGroupDialog.cpp
        editAnnotationGroupDialog.h
        stereoWidget.h
        keybindingDialog.h
        keybindingDialog.cpp
        keybindingDialog.ui
        YOLOMarkerWidget.cpp
        YOLOMarkerWidget.h
        YOLOMarkerWidget.ui
)
