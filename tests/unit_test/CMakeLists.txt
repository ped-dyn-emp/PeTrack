add_executable(petrack_tests)

target_sources(petrack_tests PRIVATE
    main.cpp
    tst_apptest.cpp
    tst_control.cpp
    tst_io.cpp
    tst_SkeletonTree.cpp
    tst_moCapController.cpp
    tst_recognition.cpp
    tst_codeMarkerWidget.cpp
    tst_helper.cpp
    tst_petrack.cpp
    tst_extrCalibration.cpp
    tst_filter.cpp
    tst_intrinsicBox.cpp
    util.h
    tst_extrinsicBox.cpp
    tst_coordinateSystemBox.cpp
    tst_tracker.cpp
)
