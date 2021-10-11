

#ifdef BUILD_FOR_OPENBMC
#define LOGDEBUG(x) phosphor::logging::log<phosphor::logging::level::DEBUG>(x)
#define LOGINFO(x) phosphor::logging::log<phosphor::logging::level::INFO>(x)
#define LOGERR(x) phosphor::logging::log<phosphor::logging::level::ERR>(x)
#else
#define LOGDEBUG(x) std::cout << "DBG : " << x << std::endl
#define LOGINFO(x) std::cout << "INFO: " << x << std::endl
#define LOGERR(x) std::cerr << "ERR : " << x << std::endl
#endif
