set(SOURCE_FILES
    fmu.cpp
    nova_fmi_library.cpp
    fmi1/fmi1_fmu.cpp
    fmi2/fmi2_fmu.cpp
    fmi3/fmi3_fmu.cpp
)

add_library(fmilibcpp STATIC ${SOURCE_FILES})
set_target_properties(fmilibcpp PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_include_directories(fmilibcpp
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../include>
    PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
)

