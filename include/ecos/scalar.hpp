
#ifndef LIBECOS_SCALAR_HPP
#define LIBECOS_SCALAR_HPP

#include <string>
#include <variant>

namespace nova_sim
{

using scalar_value = std::variant<double, int, bool, std::string,std::vector<double>>;

}


#endif // LIBECOS_SCALAR_HPP
