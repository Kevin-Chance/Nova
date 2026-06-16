# 1. 配置 CMake 生成 Release 工程
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release -DNOVA_BUILD_CLI=OFF -DNOVA_WITH_PROXYFMU=OFF

# 2. 编译核心动态库与各个 C++ 测试案例
cmake --build build --config Release --parallel --target libnova_simc nova_bouncing_ball nova_controlled_temperature nova_quarter_truck nova_quarter_truck_c nova_quarter_truck_ssp nova_spring_mass_damper
