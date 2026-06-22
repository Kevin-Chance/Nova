# ------------------------------------------------------------------
# Public headers
# ------------------------------------------------------------------
set(publicHeaders
    "nova/api/lib_info.hpp"
    "nova/engine/model.hpp"
    "nova/engine/model_instance.hpp"
    "nova/engine/nova_fmu_locator.hpp"
    "nova/engine/property.hpp"
    "nova/engine/scalar.hpp"
    "nova/engine/nova_engine.hpp"
    "nova/engine/engine_scheduler.hpp"
    "nova/engine/variable_identifier.hpp"
    "nova/api/nova.h"
    "nova/components/algorithm/algorithm.hpp"
    "nova/components/algorithm/fixed_step_algorithm.hpp"
    "nova/components/algorithm/strong_coupling_algorithm.hpp"
    "nova/components/recorder/engine_observer.hpp"
    "nova/components/recorder/csv_recorder.hpp"
    "nova/components/logger/logger.hpp"
    "nova/components/ssp/ssp_loader.hpp"
    "nova/components/structure/simulation_structure.hpp"
    "nova/components/util/chart_plotter.hpp"
)

set(publicHeadersFull)
foreach (header IN LISTS publicHeaders)
    list(APPEND publicHeadersFull "${PROJECT_SOURCE_DIR}/include/${header}")
endforeach ()

install(FILES ${publicHeadersFull} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/nova)

set(sources
    "engine/model_instance.cpp"
    "engine/nova_fmu_locator.cpp"
    "engine/nova_engine.cpp"
    "engine/engine_scheduler.cpp"
    "engine/scenario.cpp"
    "components/algorithm/fixed_step_algorithm.cpp"
    "components/algorithm/strong_coupling_algorithm.cpp"
    "components/recorder/csv_recorder.cpp"
    "components/recorder/engine_observer.cpp"
    "components/ssp/ssp.cpp"
    "components/ssp/ssp_loader.cpp"
    "components/structure/simulation_structure.cpp"
    "api/nova.cpp"
    "components/logger/logger.cpp"
    "components/util/chart_plotter.cpp"
)

set(generatedFiles "${GENERATED_SRC_DIR}/nova/lib_info.cpp")
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/api/lib_info.cpp.in" "${generatedFiles}" @ONLY)

# 1. Add nova_fmi subdirectory
add_subdirectory(nova_fmi)

# 2. Build libnova and link to nova_fmi
add_library(libnova ${publicHeadersFull} ${sources} ${generatedFiles})
target_link_libraries(libnova PUBLIC nova_fmi)
set_target_properties(libnova PROPERTIES POSITION_INDEPENDENT_CODE ON)

if (UNIX)
    # 显式链接 TBB 和系统库
    target_link_libraries(libnova PUBLIC tbb pthread dl)
endif ()

target_include_directories(libnova PUBLIC 
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>"
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/engine" "${CMAKE_CURRENT_SOURCE_DIR}/components" "${CMAKE_CURRENT_SOURCE_DIR}/engine" "${CMAKE_CURRENT_SOURCE_DIR}/components"
)
target_compile_features(libnova PUBLIC "cxx_std_20")

if (NOVA_BUILD_CLIB)
    add_library(libnova_simc SHARED "api/nova.cpp")
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
