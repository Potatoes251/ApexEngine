#ifndef LOG_H
#define LOG_H


#include "../../Resources/Header/LogSystem.h"

#define LOG(lvl, cat, msg, ...) Apex::LogSystem::Log(lvl, cat, msg, ##__VA_ARGS__)

#define LOG_INFO(msg, ...) LOG(Apex::LogLevel::Info, "General", msg, ##__VA_ARGS__)
#define LOG_WARNING(msg, ...) LOG(Apex::LogLevel::Warning, "General", msg, ##__VA_ARGS__)
#define LOG_ERROR(msg, ...) LOG(Apex::LogLevel::Error, "General", msg, ##__VA_ARGS__)
#define LOG_DEBUG(msg, ...) LOG(Apex::LogLevel::Debug, "General", msg, ##__VA_ARGS__)

#define LOG_INFO_CAT(cat, msg, ...) LOG(Apex::LogLevel::Info, cat, msg, ##__VA_ARGS__)
#define LOG_WARNING_CAT(cat, msg, ...) LOG(Apex::LogLevel::Warning, cat, msg, ##__VA_ARGS__)
#define LOG_ERROR_CAT(cat, msg, ...) LOG(Apex::LogLevel::Error, cat, msg, ##__VA_ARGS__)
#define LOG_DEBUG_CAT(cat, msg, ...) LOG(Apex::LogLevel::Debug, cat, msg, ##__VA_ARGS__)


#endif // !LOG_H

