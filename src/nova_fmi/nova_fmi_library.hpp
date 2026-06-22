#ifndef NOVA_FMI_NOVA_FMI_LIBRARY_HPP
#define NOVA_FMI_NOVA_FMI_LIBRARY_HPP

#include <string>
#include <memory>

namespace nova_fmi
{

class NovaFmiLibrary
{
public:
    explicit NovaFmiLibrary(const std::string& path);
    ~NovaFmiLibrary();

    template<typename T>
    T getFunction(const std::string& name) const {
        return reinterpret_cast<T>(getFunctionInternal(name));
    }

private:
    struct Impl;
    std::unique_ptr<Impl> pimpl_;
    void* getFunctionInternal(const std::string& name) const;
};

} // namespace nova_fmi

#endif // NOVA_FMI_NOVA_FMI_LIBRARY_HPP
