target_include_directories(petrack_core PUBLIC ${CMAKE_CURRENT_LIST_DIR})

target_sources(petrack_core PRIVATE
    autoCalib.h
    autoCalib.cpp
    extrCalibration.h
    extrCalibration.cpp
    extrinsicParameters.h
    intrinsicCameraParams.h
    intrinsicCameraParams.cpp
    stereoContext.h
    stereoContext.cpp
    worldImageCorrespondence.h
)