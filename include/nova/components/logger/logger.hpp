
#ifndef LIBNOVA_LOGGER_HPP
#define LIBNOVA_LOGGER_HPP

#include "nova/components/logger/nova_logger.hpp"

namespace nova_sim::log
{

void set_logging_level(level lvl);

void log(level lvl, std::string_view msg);

template<typename... Args>
void trace(std::string_view fmt, Args&&... args)
{
    log(level::trace, format(fmt, std::forward<Args>(args)...));
}

template<typename... Args>
void debug(std::string_view fmt, Args&&... args)
{
    log(level::debug, format(fmt, std::forward<Args>(args)...));
}

template<typename... Args>
void info(std::string_view fmt, Args&&... args)
{
    log(level::info, format(fmt, std::forward<Args>(args)...));
}

template<typename... Args>
void warn(std::string_view fmt, Args&&... args)
{
    log(level::warn, format(fmt, std::forward<Args>(args)...));
}

template<typename... Args>
void err(std::string_view fmt, Args&&... args)
{
    log(level::err, format(fmt, std::forward<Args>(args)...));
}

inline void trace(std::string_view msg)
{
    log(level::trace, msg);
}

inline void debug(std::string_view msg)
{
    log(level::debug, msg);
}

inline void info(std::string_view msg)
{
    log(level::info, msg);
}

inline void warn(std::string_view msg)
{
    log(level::warn, msg);
}

inline void err(std::string_view msg)
{
    log(level::err, msg);
}

} // namespace nova_sim::log


#endif // LIBNOVA_LOGGER_HPP
