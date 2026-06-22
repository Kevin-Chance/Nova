function(add_nova_example_executable name src)
    add_executable(${name} ${src})
    target_link_libraries(${name} PRIVATE libnova)
    target_compile_definitions(${name} PRIVATE DATA_FOLDER="${PROJECT_SOURCE_DIR}/data")
    target_compile_definitions(${name} PRIVATE RESULT_FOLDER="${PROJECT_SOURCE_DIR}/results")
    target_include_directories(${name} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/controlled_temperature)
endfunction()

# 示例列表
add_nova_example_executable(nova_bouncing_ball bouncing_ball/nova_bouncing_ball.cpp)
add_nova_example_executable(nova_controlled_temperature controlled_temperature/nova_controlled_temperature.cpp)
add_nova_example_executable(nova_spring_mass_damper spring_mass_damper/nova_spring_mass_damper.cpp)

add_nova_example_executable(nova_quarter_truck quarter_truck/nova_quarter_truck.cpp)
add_nova_example_executable(nova_quarter_truck_c quarter_truck/nova_quarter_truck_c.cpp)
add_nova_example_executable(nova_quarter_truck_ssp quarter_truck/nova_quarter_truck_ssp.cpp)
