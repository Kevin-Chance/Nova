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

/**
 * @brief 释放动态加载的库句柄
 * 确保安全且仅调用一次 FreeLibrary 或 dlclose。
 */
NovaFmiLibrary::~NovaFmiLibrary() {
    if (pimpl_ && pimpl_->handle) {
#ifdef _WIN32
        // 显式设置为 NULL 以防止重入
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

/**
 * @brief 根据符号名称从加载的共享库中获取函数指针
 * @param name 要获取的函数名
 * @return 如果找不到符号或者库未加载则返回 nullptr
 */
void* NovaFmiLibrary::getFunctionInternal(const std::string& name) const {
    if (!pimpl_ || !pimpl_->handle) return nullptr;
#ifdef _WIN32
    return (void*)GetProcAddress(pimpl_->handle, name.c_str());
#else
    return dlsym(pimpl_->handle, name.c_str());
#endif
}

} // namespace nova_fmi
