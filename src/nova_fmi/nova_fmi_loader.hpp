#ifndef NOVA_FMI_NOVA_FMI_LOADER_HPP
#define NOVA_FMI_NOVA_FMI_LOADER_HPP

#include <string>
#include <stdexcept>

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

namespace nova_fmi {

class NovaFmiLoader {
public:
    explicit NovaFmiLoader(const std::string& libraryPath) {
#ifdef _WIN32
        handle_ = LoadLibraryA(libraryPath.c_str());
        if (!handle_) {
            throw std::runtime_error("Failed to load library: " + libraryPath);
        }
#else
        handle_ = dlopen(libraryPath.c_str(), RTLD_LAZY);
        if (!handle_) {
            throw std::runtime_error("Failed to load library: " + std::string(dlerror()));
        }
#endif
    }

    ~NovaFmiLoader() {
        if (handle_) {
#ifdef _WIN32
            FreeLibrary((HMODULE)handle_);
#else
            dlclose(handle_);
#endif
        }
    }

    void* getFunction(const std::string& functionName) {
#ifdef _WIN32
        return (void*)GetProcAddress((HMODULE)handle_, functionName.c_str());
#else
        return dlsym(handle_, functionName.c_str());
#endif
    }

    NovaFmiLoader(const NovaFmiLoader&) = delete;
    NovaFmiLoader& operator=(const NovaFmiLoader&) = delete;

private:
    void* handle_;
};

} // namespace nova_fmi

#endif // NOVA_FMI_NOVA_FMI_LOADER_HPP
