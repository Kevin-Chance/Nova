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

/**
 * @brief FMI 动态链接库的简单加载器
 * 封装了不同操作系统的动态链接库 API (LoadLibrary/dlopen)，以提供安全的 RAII 资源管理。
 */
class NovaFmiLoader {
public:
    /**
     * @brief 构造函数，加载给定的共享库文件
     * @param libraryPath 共享库文件的路径
     * @throws std::runtime_error 如果无法加载库则抛出异常
     */
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

    /**
     * @brief 从已加载的库中获取指定符号名的函数指针
     * @param functionName 要获取的函数名称
     * @return 返回函数指针，未找到则返回 nullptr
     */
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
