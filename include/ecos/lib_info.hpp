#ifndef ECOS_LIB_INFO_HPP
#define ECOS_LIB_INFO_HPP

namespace nova_sim
{

struct version
{
    int major;
    int minor;
    int patch;
};

version library_version();

} // namespace nova_sim

#endif // ECOS_LIB_INFO_HPP
