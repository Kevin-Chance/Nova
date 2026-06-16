
#ifndef NOVA_FMI_SHARED_LIBRARY_HPP
#define NOVA_FMI_SHARED_LIBRARY_HPP

#include <filesystem>
#include <string>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <dlfcn.h>
#endif

namespace nova_fmi
{

class shared_library
{
public:
    explicit shared_library(const std::filesystem::path& path)
    {
#if defined(_WIN32)
        handle_ = LoadLibraryW(path.wstring().c_str());
#else
        handle_ = dlopen(path.string().c_str(), RTLD_LAZY | RTLD_GLOBAL);
#endif
    }

    ~shared_library()
    {
        if (handle_) {
#if defined(_WIN32)
            FreeLibrary(static_cast<HMODULE>(handle_));
#else
            dlclose(handle_);
#endif
        }
    }

    shared_library(const shared_library&) = delete;
    shared_library& operator=(const shared_library&) = delete;

    [[nodiscard]] void* get_function(const std::string& name) const
    {
        if (!handle_) return nullptr;
#if defined(_WIN32)
        return reinterpret_cast<void*>(GetProcAddress(static_cast<HMODULE>(handle_), name.c_str()));
#else
        return dlsym(handle_, name.c_str());
#endif
    }

    [[nodiscard]] bool loaded() const { return handle_ != nullptr; }

private:
    void* handle_{nullptr};
};

} // namespace nova_fmi

#endif // NOVA_FMI_SHARED_LIBRARY_HPP
