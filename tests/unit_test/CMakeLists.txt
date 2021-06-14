add_executable(petrack_tests)

target_sources(petrack_tests PRIVATE
    main.cpp
    tst_apptest.cpp
    tst_control.cpp
    tst_io.cpp
    tst_SkeletonTree.cpp
    tst_moCapController.cpp
    tst_recognition.cpp
)


