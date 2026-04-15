#ifndef LOG
#define LOG


#include "../../Resources/Header/LogSystem.h"

#define LOG_INFO(msg, ...) Apex::LogSystem::Log(Apex::LogLevel::Info, msg, ##__VA_ARGS__)
#define LOG_WARNING(msg, ...) Apex::LogSystem::Log(Apex::LogLevel::Warning, msg, ##__VA_ARGS__)
#define LOG_ERROR(msg, ...) Apex::LogSystem::Log(Apex::LogLevel::Error, msg, ##__VA_ARGS__)
#define LOG_DEBUG(msg, ...) Apex::LogSystem::Log(Apex::LogLevel::Debug, msg, ##__VA_ARGS__)



#endif // !LOG

