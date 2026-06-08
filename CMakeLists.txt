cmake_minimum_required(VERSION 3.19)
file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/version.txt" PROJECT_VERSION)
project("libecos" VERSION ${PROJECT_VERSION})
message("Current ${PROJECT_NAME} version: ${CMAKE_PROJECT_VERSION}\n")


# ==============================================================================
# Build settings
# ==============================================================================

option(BUILD_SHARED_LIBS OFF)
option(ECOS_BUILD_TESTS "Build tests" ON)
option(ECOS_BUILD_EXAMPLES "Build examples" ON)
option(ECOS_BUILD_CLI "Build CLI" OFF)
option(ECOS_BUILD_CLIB "Build C library (required for nova_sim_py)" ON)
option(ECOS_WITH_PROXYFMU "Build with proxyfmu" OFF)


# ==============================================================================
# Global internal configuration
# ==============================================================================

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY $<1:${CMAKE_BINARY_DIR}/bin>)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY $<1:${CMAKE_BINARY_DIR}/bin>)
set(GENERATED_SRC_DIR "${CMAKE_BINARY_DIR}/generated")
set(PY_BINARY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/nova_sim_py/binaries")

if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set(CMAKE_CXX_FLAGS_RELEASE "/O2")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3")
endif ()

if (MSVC)
    add_compile_options(/utf-8)
endif ()


# ==============================================================================
# Dependencies
# ==============================================================================

set(THIRD_PARTY_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty")

# 添加 include 路径
include_directories(${THIRD_PARTY_INCLUDE_DIR})

# === 添加 TBB 路径 ===
include_directories(${THIRD_PARTY_INCLUDE_DIR}/oneTBB/include)
link_directories(${THIRD_PARTY_INCLUDE_DIR}/oneTBB/lib/intel64/gcc4.8)

include(FetchContent)

if (ECOS_WITH_PROXYFMU)
    set(SIMPLE_SOCKET_BUILD_TESTS OFF)
    FetchContent_Declare(
            SimpleSocket
            SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/SimpleSocket
    )
    FetchContent_MakeAvailable(SimpleSocket)
endif ()

# ==============================================================================
# Targets
# ==============================================================================

add_subdirectory(src)
add_subdirectory(tool)


if (ECOS_BUILD_EXAMPLES)
    # add_subdirectory(examples)
    add_subdirectory(nova_example)
endif ()

if (ECOS_BUILD_TESTS)
    FetchContent_Declare(
            Catch2
            URL https://github.com/catchorg/Catch2/archive/v3.4.0.zip
    )
    FetchContent_MakeAvailable(Catch2)

    enable_testing()
    add_subdirectory(tests)
endif ()


# ==============================================================================
# Exports and remaining installation
# ==============================================================================

configure_package_config_file(cmake/config.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/ecos-config.cmake
        INSTALL_DESTINATION ${CMAKE_INSTALL_DATADIR}/ecos
        NO_SET_AND_CHECK_MACRO)
write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/ecos-config-version.cmake
        VERSION ${CMAKE_PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion)
install(
        FILES
        ${CMAKE_CURRENT_BINARY_DIR}/ecos-config.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/ecos-config-version.cmake
        DESTINATION
        ${CMAKE_INSTALL_DATADIR}/ecos)

install(TARGETS libecos libnova_simc ecos fmilibcpp
        EXPORT ecos-targets
)
if (ECOS_WITH_PROXYFMU)
    install(
            TARGETS proxyfmu
            EXPORT ecos-targets
    )
endif ()

install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(EXPORT ecos-targets
        NAMESPACE nova_sim::
        DESTINATION ${CMAKE_INSTALL_DATADIR}/ecos)
