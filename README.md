# ecos-nuaa

## Ecos
>Ecos is a co-simulation engine, check out https://github.com/Ecos-platform/ecos for detailed information.

## Building
```
//Windows
Run 'build.bat'.
//Linux
cmake . -B build -DECOS_BUILD_EXAMPLES=ON -DECOS_BUILD_TESTS=ON -DCMAKE_BUILD_TYPE=Release
cmake --build build
```