
#include "nova/logger/logger.hpp"

using namespace nova_sim;

void log::set_logging_level(level lvl)
{
    log::NovaLogger::instance().set_level(lvl);
}

void log::log(level lvl, std::string_view msg)
{
    log::NovaLogger::instance().log(lvl, msg);
}
