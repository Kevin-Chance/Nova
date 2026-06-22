#include "nova_fmi_library.hpp"
#include <stdexcept>
#include <iostream>

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

namespace nova_fmi {

struct NovaFmiLibrary::Impl {
#ifdef _WIN32
    HMODULE handle;
#else
    void* handle;
#endif
};

NovaFmiLibrary::NovaFmiLibrary(const std::string& path)
    : pimpl_(std::make_unique<Impl>())
{
#ifdef _WIN32
    pimpl_->handle = LoadLibraryA(path.c_str());
#else
    pimpl_->handle = dlopen(path.c_str(), RTLD_NOW);
#endif
    if (!pimpl_->handle) {
        throw std::runtime_error("Failed to load FMI library: " + path);
    }
}

NovaFmiLibrary::~NovaFmiLibrary() {
    if (pimpl_ && pimpl_->handle) {
#ifdef _WIN32
        // WEEK 1 FIX: Explicitly set to NULL before free to prevent re-entry
        HMODULE h = pimpl_->handle;
        pimpl_->handle = NULL;
        FreeLibrary(h);
#else
        void* h = pimpl_->handle;
        pimpl_->handle = nullptr;
        dlclose(h);
#endif
    }
}

void* NovaFmiLibrary::getFunctionInternal(const std::string& name) const {
    if (!pimpl_ || !pimpl_->handle) return nullptr;
#ifdef _WIN32
    return (void*)GetProcAddress(pimpl_->handle, name.c_str());
#else
    return dlsym(pimpl_->handle, name.c_str());
#endif
}

} // namespace nova_fmi
