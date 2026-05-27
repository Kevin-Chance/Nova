# ------------------------------------------------------------------
# Public headers
# ------------------------------------------------------------------
set(publicHeaders
    "ecos/lib_info.hpp"
    "ecos/model.hpp"
    "ecos/model_instance.hpp"
    "ecos/nova_fmu_locator.hpp"
    "ecos/property.hpp"
    "ecos/scalar.hpp"
    "ecos/simulation.hpp"
    "ecos/simulation_runner.hpp"
    "ecos/variable_identifier.hpp"
    "ecos/nova_ecos.h"
    "ecos/algorithm/algorithm.hpp"
    "ecos/algorithm/fixed_step_algorithm.hpp"
    "ecos/algorithm/strong_coupling_algorithm.hpp"
    "ecos/listeners/simulation_listener.hpp"
    "ecos/listeners/csv_writer.hpp"
    "ecos/logger/logger.hpp"
    "ecos/ssp/ssp_loader.hpp"
    "ecos/structure/simulation_structure.hpp"
    "ecos/util/plotter.hpp"
)

set(publicHeadersFull)
foreach (header IN LISTS publicHeaders)
    list(APPEND publicHeadersFull "${PROJECT_SOURCE_DIR}/include/${header}")
endforeach ()

install(FILES ${publicHeadersFull} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ecos)

set(sources
    "ecos/model_instance.cpp"
    "ecos/nova_fmu_locator.cpp"
    "ecos/simulation.cpp"
    "ecos/simulation_runner.cpp"
    "ecos/scenario/scenario.cpp"
    "ecos/algorithm/fixed_step_algorithm.cpp"
    "ecos/algorithm/strong_coupling_algorithm.cpp"
    "ecos/listeners/csv_writer.cpp"
    "ecos/listeners/simulation_listener.cpp"
    "ecos/ssp/ssp.cpp"
    "ecos/ssp/ssp_loader.cpp"
    "ecos/structure/simulation_structure.cpp"
    "ecos/nova_ecos.cpp"
    "ecos/logger/logger.cpp"
    "ecos/util/plotter.cpp"
)

set(generatedFiles "${GENERATED_SRC_DIR}/ecos/lib_info.cpp")
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/ecos/lib_info.cpp.in" "${generatedFiles}" @ONLY)

# 1. Add fmilibcpp subdirectory
add_subdirectory(fmilibcpp)

# 2. Build libecos and link to fmilibcpp
add_library(libecos ${publicHeadersFull} ${sources} ${generatedFiles})
target_link_libraries(libecos PUBLIC fmilibcpp)
target_include_directories(libecos PUBLIC 
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>"
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/external/spdlog>"
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/external/pugixml>"
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}"
)
target_compile_features(libecos PUBLIC "cxx_std_20")

if (ECOS_BUILD_CLIB)
    add_library(libnova_simc SHARED "ecos/nova_ecos.cpp")
    target_link_libraries(libnova_simc PUBLIC libecos)
    
    add_custom_command(TARGET libnova_simc POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory ${PY_BINARY_DIR}
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:libnova_simc>
        ${PY_BINARY_DIR}
        COMMENT "Copying libnova_simc to ${PY_BINARY_DIR}"
    )
endif ()
