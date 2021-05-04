add_executable(petrack_tests)

target_sources(petrack_tests PRIVATE
    main.cpp
    tst_apptest.cpp
    tst_control.cpp
    tst_io.cpp
    tst_recognition.cpp
)


