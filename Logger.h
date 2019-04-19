#ifndef __LOGGER_H
#define __LOGGER_H
#include "include/spdlog/sinks/daily_file_sink.h"
#include "include/spdlog/sinks/rotating_file_sink.h"
#include "include/spdlog/async.h"
class Logger
{
private:
	Logger() = delete;
	~Logger() = delete;

public:
	static std::shared_ptr<spdlog::logger> getInstance();

private:
	static std::shared_ptr<spdlog::logger> m_logger;
	static std::mutex m_mtx;
};

#define SPDLOG(level, format, ...) {Logger::getInstance()->##level(format, __VA_ARGS__);}

class Logger_R
{
private:
	Logger_R() = delete;
	~Logger_R() = delete;

public:
	static std::shared_ptr<spdlog::logger> getInstance();

private:
	static std::shared_ptr<spdlog::logger> m_logger;
	static std::mutex m_mtx;
};

class Logger_W
{
private:
	Logger_W() = delete;
	~Logger_W() = delete;

public:
	static std::shared_ptr<spdlog::logger> getInstance();

private:
	static std::shared_ptr<spdlog::logger> m_logger;
	static std::mutex m_mtx;
};

#endif //__LOGGER_H