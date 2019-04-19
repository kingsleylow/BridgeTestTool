#include "Logger.h"
#include "ReadConf.h"

std::mutex Logger::m_mtx;
std::mutex Logger_R::m_mtx;
std::mutex Logger_W::m_mtx;

std::shared_ptr<spdlog::logger> Logger::m_logger = nullptr;
std::shared_ptr<spdlog::logger> Logger_R::m_logger = nullptr;
std::shared_ptr<spdlog::logger> Logger_W::m_logger = nullptr;

std::shared_ptr<spdlog::logger> Logger::getInstance()
{
	if (m_logger == nullptr)
	{
		std::lock_guard<std::mutex> lck(m_mtx);
		if (m_logger == nullptr)
		{
			std::string path;
			ReadConf::getInstance().getProjectPath(path);
			path += "logs/daily.txt";
			m_logger = spdlog::create_async<spdlog::sinks::daily_file_sink_mt>("BridgeTest", path, 10, 30);

			//m_logger->set_error_handler([](const std::string &msg){spdlog::get("console")->error("***LOGGER ERROR***:{}", msg); });
			
			m_logger->set_level(spdlog::level::from_str("info"));

			m_logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e][%-5l][thread %t][%v]");

			m_logger->flush_on(spdlog::level::from_str("debug"));

			return m_logger;
		}
	}
	return m_logger;
}

std::shared_ptr<spdlog::logger> Logger_R::getInstance()
{
	if (m_logger == nullptr)
	{
		std::lock_guard<std::mutex> lck(m_mtx);
		if (m_logger == nullptr)
		{
			std::string path;
			ReadConf::getInstance().getProjectPath(path);
			path += "logs/orders_S.txt";
			m_logger = spdlog::create_async<spdlog::sinks::rotating_file_sink_mt>("BridgeTest_R", path, 1048576, 3);

			//m_logger->set_error_handler([](const std::string &msg){spdlog::get("console")->error("***LOGGER ERROR***:{}", msg); });

			m_logger->set_level(spdlog::level::from_str("info"));

			m_logger->set_pattern("[%v]");

			m_logger->flush_on(spdlog::level::from_str("debug"));

			return m_logger;
		}
	}
	return m_logger;
}


std::shared_ptr<spdlog::logger> Logger_W::getInstance()
{
	if (m_logger == nullptr)
	{
		std::lock_guard<std::mutex> lck(m_mtx);
		if (m_logger == nullptr)
		{
			std::string path;
			ReadConf::getInstance().getProjectPath(path);
			path += "logs/orders_D.txt";
			m_logger = spdlog::create_async<spdlog::sinks::rotating_file_sink_mt>("BridgeTest_W", path, 1048576, 3);

			//m_logger->set_error_handler([](const std::string &msg){spdlog::get("console")->error("***LOGGER ERROR***:{}", msg); });

			m_logger->set_level(spdlog::level::from_str("info"));

			m_logger->set_pattern("[%v]");

			m_logger->flush_on(spdlog::level::from_str("debug"));

			return m_logger;
		}
	}
	return m_logger;
}
