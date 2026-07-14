cd /home/.../nova
cmake . -B build -DNOVA_BUILD_EXAMPLES=OFF -DNOVA_BUILD_TESTS=OFF -DCMAKE_BUILD_TYPE=Release
cmake --build build
cmake --install build --prefix deploy