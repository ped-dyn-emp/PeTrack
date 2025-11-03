# This module checks if HDF5 is already built (cached) and only builds if necessary

function(find_or_build_hdf5)
    # Option to use the cached HDF5
    option(USE_CACHED_HDF5 "Use cached HDF5 libraries instead of building from source" OFF)

    if(USE_CACHED_HDF5)
        message(STATUS "Looking for cached HDF5 libraries...")

        # Set up paths to cached HDF5
        set(HDF5_ROOT "${CMAKE_BINARY_DIR}/deps/hdf5" CACHE PATH "HDF5 root directory")

        # Try to find the cached HDF5 libraries
        # Handle different naming conventions (debug/release, static/shared)
        find_library(HDF5_C_LIBRARY
            NAMES
                hdf5-static libhdf5.a hdf5.lib           # Release names
                hdf5_debug libhdf5_debug.a hdf5d.lib     # Debug names
                hdf5                                     # Generic
            PATHS "${HDF5_ROOT}/src" "${HDF5_ROOT}/bin"
            NO_DEFAULT_PATH)

        find_library(HDF5_CPP_LIBRARY
            NAMES
                hdf5_cpp-static libhdf5_cpp.a hdf5_cpp.lib           # Release names
                hdf5_cpp_debug libhdf5_cpp_debug.a hdf5_cppd.lib     # Debug names
                hdf5_cpp                                             # Generic
            PATHS "${HDF5_ROOT}/c++/src" "${HDF5_ROOT}/bin"
            NO_DEFAULT_PATH)

        if(HDF5_C_LIBRARY AND HDF5_CPP_LIBRARY)
            message(STATUS "Found cached HDF5: ${HDF5_C_LIBRARY}")
            message(STATUS "Found cached HDF5 C++: ${HDF5_CPP_LIBRARY}")

            set(HDF5_INCLUDE_DIRS
                "${CMAKE_SOURCE_DIR}/deps/hdf5/src"       # original source headers
                "${CMAKE_SOURCE_DIR}/deps/hdf5/src/H5FDsubfiling"
                "${HDF5_ROOT}/src"                        # generated headers in build tree
            )

            set(HDF5_CPP_INCLUDE_DIRS
                "${CMAKE_SOURCE_DIR}/deps/hdf5/c++/src"  # original C++ headers
                "${HDF5_ROOT}/c++/src"                   # generated C++ headers in build tree
            )

            # Create imported targets
            add_library(hdf5-static STATIC IMPORTED GLOBAL)
            set_target_properties(hdf5-static PROPERTIES
                IMPORTED_LOCATION "${HDF5_C_LIBRARY}"
                INTERFACE_INCLUDE_DIRECTORIES "${HDF5_INCLUDE_DIRS}"
            )

            add_library(hdf5_cpp-static STATIC IMPORTED GLOBAL)
            set_target_properties(hdf5_cpp-static PROPERTIES
                IMPORTED_LOCATION "${HDF5_CPP_LIBRARY}"
                INTERFACE_INCLUDE_DIRECTORIES "${HDF5_CPP_INCLUDE_DIRS}"
                INTERFACE_LINK_LIBRARIES hdf5-static
            )

            message(STATUS "Using cached HDF5 libraries")
        else()
            message(WARNING "USE_CACHED_HDF5 was ON but libraries not found. Building from source instead.")
            message(STATUS "  Searched for C library: ${HDF5_ROOT}/src")
            message(STATUS "  Searched for C++ library: ${HDF5_ROOT}/c++/src")
            set(USE_CACHED_HDF5 OFF CACHE BOOL "Use cached HDF5 libraries" FORCE)
        endif()
    endif()

    if(NOT USE_CACHED_HDF5)
        message(STATUS "Building HDF5 from source")

        set(HDF5_BUILD_CPP_LIB ON CACHE BOOL "Build HDF5 C++ Library" FORCE)
        set(HDF5_BUILD_TOOLS OFF CACHE BOOL "Build HDF5 tools")
        set(HDF5_BUILD_EXAMPLES OFF CACHE BOOL "Build HDF5 examples")
        set(HDF5_TEST OFF CACHE BOOL "Build HDF5 tests")
        set(HDF5_ENABLE_HL OFF CACHE BOOL "Enable HDF5 High-Level library")
        set(HDF5_ENABLE_Z_LIB_SUPPORT OFF CACHE BOOL "Enable Zlib support")
        set(HDF5_ENABLE_SZIP_SUPPORT OFF CACHE BOOL "Enable SZIP support")

        add_subdirectory(${CMAKE_SOURCE_DIR}/deps/hdf5 EXCLUDE_FROM_ALL)
    endif()
endfunction()
