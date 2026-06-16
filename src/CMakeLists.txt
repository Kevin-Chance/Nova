# ------------------------------------------------------------------
# Public headers
# ------------------------------------------------------------------
set(publicHeaders
    "nova/lib_info.hpp"
    "nova/model.hpp"
    "nova/model_instance.hpp"
    "nova/nova_fmu_locator.hpp"
    "nova/property.hpp"
    "nova/scalar.hpp"
    "nova/simulation.hpp"
    "nova/simulation_runner.hpp"
    "nova/variable_identifier.hpp"
    "nova/nova.h"
    "nova/algorithm/algorithm.hpp"
    "nova/algorithm/fixed_step_algorithm.hpp"
    "nova/algorithm/strong_coupling_algorithm.hpp"
    "nova/listeners/simulation_listener.hpp"
    "nova/listeners/csv_writer.hpp"
    "nova/logger/logger.hpp"
    "nova/ssp/ssp_loader.hpp"
    "nova/structure/simulation_structure.hpp"
    "nova/util/plotter.hpp"
)

set(publicHeadersFull)
foreach (header IN LISTS publicHeaders)
    list(APPEND publicHeadersFull "${PROJECT_SOURCE_DIR}/include/${header}")
endforeach ()

install(FILES ${publicHeadersFull} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/nova)

set(sources
    "model_instance.cpp"
    "nova_fmu_locator.cpp"
    "simulation.cpp"
    "simulation_runner.cpp"
    "scenario/scenario.cpp"
    "algorithm/fixed_step_algorithm.cpp"
    "algorithm/strong_coupling_algorithm.cpp"
    "listeners/csv_writer.cpp"
    "listeners/simulation_listener.cpp"
    "ssp/ssp.cpp"
    "ssp/ssp_loader.cpp"
    "structure/simulation_structure.cpp"
    "nova.cpp"
    "logger/logger.cpp"
    "util/plotter.cpp"
)

set(generatedFiles "${GENERATED_SRC_DIR}/nova/lib_info.cpp")
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/lib_info.cpp.in" "${generatedFiles}" @ONLY)

# 1. Add fmilibcpp subdirectory
add_subdirectory(fmilibcpp)

# 2. Build libnova and link to fmilibcpp
add_library(libnova ${publicHeadersFull} ${sources} ${generatedFiles})
target_link_libraries(libnova PUBLIC fmilibcpp)
set_target_properties(libnova PROPERTIES POSITION_INDEPENDENT_CODE ON)

if (UNIX)
    # 显式链接 TBB 和系统库
    target_link_libraries(libnova PUBLIC tbb pthread dl)
endif ()

target_include_directories(libnova PUBLIC 
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>"
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}"
)
target_compile_features(libnova PUBLIC "cxx_std_20")

if (NOVA_BUILD_CLIB)
    add_library(libnova_simc SHARED "nova.cpp")
    target_link_libraries(libnova_simc PUBLIC libnova)
    
    # 避免 UNIX 下自动生成 liblibnova_simc.so (去除重复的 lib 前缀)
    if (UNIX)
        set_target_properties(libnova_simc PROPERTIES PREFIX "")
    endif ()
    
    add_custom_command(TARGET libnova_simc POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory ${PY_BINARY_DIR}
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:libnova_simc>
        ${PY_BINARY_DIR}
        COMMENT "Copying libnova_simc to ${PY_BINARY_DIR}"
    )
endif ()
