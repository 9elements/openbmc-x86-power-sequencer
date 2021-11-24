#pragma once

extern int _loglevel;
#ifdef WITH_PHOSPHOR_LOGGING
#include <phosphor-logging/log.hpp>

#define LOGDEBUG(x)                                                            \
    if (_loglevel > 1)                                                         \
    {                                                                          \
        phosphor::logging::log<phosphor::logging::level::DEBUG>(x);            \
        std::cout << "DBG : " << x << std::endl;                               \
    }

#define LOGINFO(x)                                                             \
    if (_loglevel > 0)                                                         \
    {                                                                          \
        phosphor::logging::log<phosphor::logging::level::INFO>(x);             \
        std::cout << "INFO : " << x << std::endl;                              \
    }

#define LOGERR(x)                                                              \
    {                                                                          \
        phosphor::logging::log<phosphor::logging::level::ERR>(x);              \
        std::cerr << "ERR : " << x << std::endl;                               \
    }

#else
#define LOGDEBUG(x)                                                            \
    if (_loglevel > 1)                                                         \
    std::cout << "DBG : " << x << std::endl

#define LOGINFO(x)                                                             \
    if (_loglevel > 0)                                                         \
    std::cout << "INFO : " << x << std::endl

#define LOGERR(x) std::cerr << "ERR : " << x << std::endl
#endif
