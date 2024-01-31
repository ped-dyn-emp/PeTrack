add_executable(petrack_tests)

target_include_directories(petrack_tests PRIVATE ${CMAKE_CURRENT_LIST_DIR})

target_sources(petrack_tests PRIVATE
    main.cpp
    util.h
    tst_petrack.cpp
)

add_subdirectory(calibration)
add_subdirectory(filter)
add_subdirectory(general)
add_subdirectory(IO)
add_subdirectory(recognition)
add_subdirectory(tracking)