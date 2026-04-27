@echo off
REM Configure the project
cmake . -A x64 -B build -DECOS_BUILD_EXAMPLES=ON -DECOS_BUILD_TESTS=ON -DCMAKE_BUILD_TYPE=Release

REM Build the project
cmake --build build --config Release

pause