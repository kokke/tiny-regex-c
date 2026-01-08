cmake_minimum_required(VERSION 3.12)
project(tiny-regex-c
    VERSION 0.1.0
    DESCRIPTION "Mini regex-module inspired by Rob Pike's regex code"
    LANGUAGES C
)

# Set C standard
# Note: MSVC has limited C99 support (no VLAs), so we use C11 which requires explicit VLA workarounds
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

# Options for building shared/static libraries
option(BUILD_SHARED_LIBS "Build shared library" ON)
option(BUILD_STATIC_LIBS "Build static library" ON)
option(BUILD_TESTS "Build test executables" ON)

# Set default output directories
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Define source files
set(TINY_REGEX_SOURCES
    re.c
)

set(TINY_REGEX_HEADERS
    re.h
)

# Static Library
if(BUILD_STATIC_LIBS)
    add_library(tiny-regex-static STATIC ${TINY_REGEX_SOURCES})
    target_include_directories(tiny-regex-static PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
    )
    
    # Set output name
    set_target_properties(tiny-regex-static PROPERTIES
        OUTPUT_NAME tiny-regex
        PUBLIC_HEADER "${TINY_REGEX_HEADERS}"
    )
    
    # Compiler flags for all platforms
    if(MSVC)
        target_compile_options(tiny-regex-static PRIVATE /W4)
    else()
        target_compile_options(tiny-regex-static PRIVATE -Wall -Wextra -O3)
    endif()
endif()

# Shared Library
if(BUILD_SHARED_LIBS)
    add_library(tiny-regex-shared SHARED ${TINY_REGEX_SOURCES})
    target_include_directories(tiny-regex-shared PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
    )
    
    # Set output name (Windows adds .dll, Unix adds .so)
    set_target_properties(tiny-regex-shared PROPERTIES
        OUTPUT_NAME tiny-regex
        PUBLIC_HEADER "${TINY_REGEX_HEADERS}"
        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR}
    )
    
    # Compiler flags for all platforms
    if(MSVC)
        target_compile_options(tiny-regex-shared PRIVATE /W4)
    else()
        target_compile_options(tiny-regex-shared PRIVATE -Wall -Wextra -O3 -fPIC)
    endif()
    
    # On Windows, define TINY_REGEX_DLL_EXPORT for the shared library
    if(MSVC)
        target_compile_definitions(tiny-regex-shared PRIVATE TINY_REGEX_DLL_EXPORT)
    endif()
endif()

# Test executables
if(BUILD_TESTS)
    set(TEST_SOURCES
        tests/test1.c
        tests/test2.c
        tests/test_rand.c
        tests/test_rand_neg.c
        tests/test_compile.c
    )
    
    foreach(test_src ${TEST_SOURCES})
        get_filename_component(test_name ${test_src} NAME_WE)
        add_executable(${test_name} ${test_src})
        
        # Link with static library (prefer static for tests)
        if(BUILD_STATIC_LIBS)
            target_link_libraries(${test_name} PRIVATE tiny-regex-static)
        elseif(BUILD_SHARED_LIBS)
            target_link_libraries(${test_name} PRIVATE tiny-regex-shared)
        endif()
        
        # Set output directory
        set_target_properties(${test_name} PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/tests
        )
    endforeach()
endif()

# Install targets
include(GNUInstallDirs)

if(BUILD_STATIC_LIBS)
    install(TARGETS tiny-regex-static
        EXPORT tiny-regex-targets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )
endif()

if(BUILD_SHARED_LIBS)
    install(TARGETS tiny-regex-shared
        EXPORT tiny-regex-targets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )
endif()

# Install headers separately if both libraries are disabled (edge case)
if(NOT BUILD_STATIC_LIBS AND NOT BUILD_SHARED_LIBS)
    install(FILES ${TINY_REGEX_HEADERS}
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )
endif()

# Export targets for CMake find_package() support
if(BUILD_STATIC_LIBS OR BUILD_SHARED_LIBS)
    install(EXPORT tiny-regex-targets
        FILE tiny-regex-targets.cmake
        NAMESPACE tiny-regex::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/tiny-regex
    )
    
    # Create and install config file
    include(CMakePackageConfigHelpers)
    write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/tiny-regex-config-version.cmake
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion
    )
    
    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/tiny-regex-config.cmake
        "include(\${CMAKE_CURRENT_LIST_DIR}/tiny-regex-targets.cmake)\n"
    )
    
    install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/tiny-regex-config.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/tiny-regex-config-version.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/tiny-regex
    )
endif()

# Print build summary
message(STATUS "")
message(STATUS "tiny-regex-c Configuration Summary")
message(STATUS "==================================")
message(STATUS "Build static library: ${BUILD_STATIC_LIBS}")
message(STATUS "Build shared library: ${BUILD_SHARED_LIBS}")
message(STATUS "Build tests: ${BUILD_TESTS}")
message(STATUS "Install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "")
