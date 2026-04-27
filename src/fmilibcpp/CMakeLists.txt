set(SOURCE_FILES
    fmu.cpp
    nova_fmi_library.cpp
    fmi1/fmi1_fmu.cpp
    fmi1/fmi1_model_description.cpp
    fmi2/fmi2_fmu.cpp
    fmi2/fmi2_model_description.cpp
    fmi3/fmi3_fmu.cpp
    fmi3/fmi3_model_description.cpp
    # 链接 pugixml 源码以解决 linker 错误
    ../external/pugixml/pugixml.cpp
)

add_library(fmilibcpp STATIC ${SOURCE_FILES})

target_include_directories(fmilibcpp
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../include>
    PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
        ${CMAKE_CURRENT_SOURCE_DIR}/../external/pugixml
        ${CMAKE_CURRENT_SOURCE_DIR}/../external/spdlog
        ${CMAKE_CURRENT_SOURCE_DIR}/../../dependencies/fmi4c/include
)

target_link_libraries(fmilibcpp PUBLIC fmi4c)
