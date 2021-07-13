cmake_minimum_required(VERSION 3.14)
project(
        "org-simple"
        VERSION 0.0.1
        DESCRIPTION "Simple utility library in modern standard C++ with a minimum of STL"
        HOMEPAGE_URL "https://github.com/emmef/org-simple")

set(CMAKE_CXX_STANDARD 20)

if (MSVC)
    # warning level 4
    add_compile_options(/W4)
    # all warnings as errors
    add_compile_options(/WX)
else ()
    # lots of warnings
    add_compile_options(-Wall -Wextra -pedantic)
    # all warnings as errors
    #add_compile_options(-Werror)
endif ()

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost REQUIRED COMPONENTS unit_test_framework)
find_package(Threads)

set(PROJECT_HEADERS include/org-simple/core/debug.h include/org-simple/core/Bits.h include/org-simple/core/Power2.h include/org-simple/core/bounds.h include/org-simple/core/Index.h include/org-simple/core/Circular.h include/org-simple/util/Reference.h include/org-simple/util/Timeout.h include/org-simple/util/FakeClock.h include/org-simple/util/NumArray.h include/org-simple/util/Array.h include/org-simple/util/NumArray.h include/org-simple/util/LockfreeRingBuffer.h include/org-simple/util/SampleLayout.h include/org-simple/core/number_traits.h include/org-simple/dsp/iir-filter.h include/org-simple/dsp/iir-coefficients.h include/org-simple/dsp/rate.h include/org-simple/dsp/iir-butterworth.h include/org-simple/util/Signal.h include/org-simple/util/SignalManager.h)
set(PROJECT_SRC ${PROJECT_HEADERS} src/org-simple/placeholder.cc include/org-simple/core/align.h include/org-simple/core/denormal.h include/org-simple/dsp/integration.h)
set(PROJECT_TESTS ${PROJECT_HEADERS} test/test-helper.h test/test.cc test/util/OwnedReference.h test/util/OwnedReference.cc test/util/OwnedReference-tests.cc test/core/bounds-tests.cc test/core/Power2-tests.cc test/core/Circular-tests.cc test/core/Bits-tests.cc test/util/Timeout-tests.cc test/util/Reference-tests.cc test/util/RefCount-tests.cc test/core/Index-tests.cc test/boost-unit-tests.h test/util/FakeClock-tests.cc test/core/SizeMetric-tests.cc test/util/Array-tests.cc test/util/NumArray-tests.cc test/util/LockFreeRingBufferTests.cc test/util/SampleLayoutTests.cc test/core/Numbers-tests.cc test/core/Align-tests.cc test/dsp/iir-filter-tests.cc test/dsp/iir-coefficients-tests.cc test/dsp/rate-tests.cc test/dsp/integration-tests.cc test/dsp/iir-butterworth-tests.cc test/util/Signal-tests.cc test/util/SignalManager-tests.cc)
set(PROJECT_EXPERIMENTS ${PROJECT_HEADERS} experiment/experiments.cc)

# Create the library

# Define library library
add_library(${PROJECT_NAME} SHARED ${PROJECT_SRC})

# Requires C++17 or higher
target_compile_features(${PROJECT_NAME} INTERFACE cxx_std_17)

# Define include directories. If another project uses the package, it will
# use the source code include directory over the installed include directory if
# the code also exists on the machine that uses the library
target_include_directories(${PROJECT_NAME} INTERFACE $<BUILD_INTERFACE:${${PROJECT_NAME}_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
include_directories(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/include)

# Helper for system paths
include(GNUInstallDirs)

# Define target directories
install(TARGETS ${PROJECT_NAME}
        EXPORT ${PROJECT_NAME}_Targets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

# Make sure the library is recognisable and can be found as a package.
include(CMakePackageConfigHelpers)
write_basic_package_version_file("${PROJECT_NAME}.ConfigVersion.cmake"
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion)
configure_package_config_file(
        "${PROJECT_SOURCE_DIR}/cmake/${PROJECT_NAME}.Config.cmake.in"
        "${PROJECT_BINARY_DIR}/${PROJECT_NAME}.Config.cmake"
        INSTALL_DESTINATION
        ${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME}/cmake)
install(EXPORT ${PROJECT_NAME}_Targets
        FILE ${PROJECT_NAME}Targets.cmake
        NAMESPACE ${PROJECT_NAME}::
        DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME}/cmake)
install(FILES "${PROJECT_BINARY_DIR}/${PROJECT_NAME}.Config.cmake"
        "${PROJECT_BINARY_DIR}/${PROJECT_NAME}.ConfigVersion.cmake"
        DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME}/cmake)

# Same for pkg-config
configure_file(pkg-config/${PROJECT_NAME}.pc.in ${PROJECT_NAME}.pc @ONLY)
install(FILES ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.pc DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/pkgconfig)

#install the header directory
install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME} DESTINATION include)

# Create unit tests. This executable will not be installed.
set(TEST_PROJECT "test")
add_executable(${TEST_PROJECT} ${PROJECT_TESTS})
target_include_directories(${TEST_PROJECT} BEFORE PRIVATE ${PROJECT_HEADERS})
target_link_libraries(${TEST_PROJECT} ${PROJECT_NAME} ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY} ${CMAKE_THREAD_LIBS_INIT} stdc++ m)
include_directories(${TEST_PROJECT} ${CMAKE_SOURCE_DIR}/include)

# Create module for experiments and tests that are not part of the normal tests.
set(EXPERIMENT_PROJECT "experiments")
add_executable(${EXPERIMENT_PROJECT} ${PROJECT_EXPERIMENTS})
target_include_directories(${EXPERIMENT_PROJECT} BEFORE PRIVATE ${PROJECT_HEADERS})
target_link_libraries(${EXPERIMENT_PROJECT} ${PROJECT_NAME} stdc++ m)
include_directories(${EXPERIMENT_PROJECT} ${CMAKE_SOURCE_DIR}/include)
