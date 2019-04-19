#include "pch.h"
#include "ConnToMt4.h"
#include "Logger.h"
#include <chrono>
#include <thread>
#include "PriceData.h"
#include "ReadConf.h"

MT4Conn MT4Conn::m_self;

MT4Conn::MT4Conn()
{
	m_stop = false;
}


MT4Conn::~MT4Conn()
{
	m_stop = true;
	std::this_thread::sleep_for(std::chrono::milliseconds(1));
}

MT4Conn& MT4Conn::getInstance()
{
	return m_self;
}

bool MT4Conn::mt4Init()
{
	std::vector<std::string> logins = ReadConf::getInstance().getAllLogins();
	for (auto& login : logins)
		m_logins.insert(std::stoi(login));
	std::string path;
	ReadConf::getInstance().getProjectPath(path);
#ifdef _WIN64
	path += "mt4lib/mtmanapi64.dll";
#else 
	path += "mt4lib/mtmanapi.dll";
#endif
	m_factoryInter.Init(path.c_str());
	if (m_factoryInter.IsValid() == FALSE)
	{
		SPDLOG(error, "mt4 lib init failed");
		return false;
	}
	if (m_factoryInter.WinsockStartup() != RET_OK)
	{
		SPDLOG(error, "mt4 lib init winsock lib failed.");
		return false;
	}
	if (NULL == (m_managerInterPump = m_factoryInter.Create(ManAPIVersion)))
	{
		SPDLOG(error, "mt4 factory create mananger pump interface failed.");
		return false;
	}

	if (NULL == (m_managerInterDirtOpen = m_factoryInter.Create(ManAPIVersion)))
	{
		SPDLOG(error, "mt4 factory create mananger deal interface failed.");
		return false;
	}

	if (NULL == (m_managerInterDirtClose = m_factoryInter.Create(ManAPIVersion)))
	{
		SPDLOG(error, "mt4 factory create mananger deal interface failed.");
		return false;
	}
	return true;
}

bool MT4Conn::mt4Login(const int login, const std::string& passwd)
{
	if (RET_OK != mt4Conn(ReadConf::getInstance().getConnConf().find("host")->second))
		return false;
	
	if (!mt4PumpLogin(login, passwd) || !(mt4DirtLogin(login, passwd)))
		return false;

	return true;
}

bool MT4Conn::mt4PumpLogin(const int login, const std::string& passwd)
{
	int cnt = 0;
	int err = RET_OK;
	while (cnt < 3)
	{
		if (RET_OK != (err = m_managerInterPump->Login(login, passwd.c_str())))
		{
			SPDLOG(error, "mt4 server login failed.---{} times, errno: ", cnt + 1, err);
			cnt++;
			std::this_thread::sleep_for(std::chrono::seconds(1));
		}
		else
		{
			cnt = 0;
			break;
		}
	}
	if (cnt != 0)
	{
		SPDLOG(error, "3 times of login to mt4 server failed.(pump)");
		return false;
	}

	if (RET_OK != m_managerInterPump->PumpingSwitchEx(NotifyCallBack, 0, this))
	{
		SPDLOG(error, "pumpswitch failed.");
		return false;
	}
	return true;
}
bool MT4Conn::mt4DirtLogin(const int login, const std::string& passwd)
{
	int cnt = 0;
	int err = RET_OK;
	while (cnt < 3)
	{
		if (RET_OK != (err = m_managerInterDirtOpen->Login(login, passwd.c_str())))
		{
			SPDLOG(error, "mt4 server login failed.---{} times, errno: ", cnt + 1, err);
			cnt++;
			std::this_thread::sleep_for(std::chrono::seconds(1));
		}
		else
		{
			cnt = 0;
			break;
		}
	}
	if (cnt != 0)
	{
		SPDLOG(error, "3 times of login to mt4 server failed.(dirt)");
		return false;
	}

	cnt = 0;
	while (cnt < 3)
	{
		if (RET_OK != (err = m_managerInterDirtClose->Login(login, passwd.c_str())))
		{
			SPDLOG(error, "mt4 server login failed.---{} times, errno: ", cnt + 1, err);
			cnt++;
			std::this_thread::sleep_for(std::chrono::seconds(1));
		}
		else
		{
			cnt = 0;
			break;
		}
	}
	if (cnt != 0)
	{
		SPDLOG(error, "3 times of login to mt4 server failed.(dirt)");
		return false;
	}

	return true;
}


int MT4Conn::mt4Conn(const std::string& host)
{
	if (!mt4DirtConn(host) || !mt4PumpConn(host))
		return RET_NO_CONNECT;
	else
		return RET_OK;
}

bool MT4Conn::mt4PumpConn(const std::string& host)
{
	int cnt = 0;
	while (cnt < 3)
	{
		if (m_managerInterPump->Connect(host.c_str()) != RET_OK)
		{
			SPDLOG(error, "try to connect to mt4 server failed---{}", cnt + 1);
			cnt++;
			std::this_thread::sleep_for(std::chrono::seconds(1));
		}
		else
		{
			cnt = 0;
			break;
		}
	}
	if (cnt != 0)
	{
		SPDLOG(error, "3 times of connecting to mt4 server failed.");
		return false;
	}
	return true;
}

bool MT4Conn::mt4DirtConn(const std::string& host)
{
	int cnt = 0;
	while (cnt < 3)
	{
		if (m_managerInterDirtOpen->Connect(host.c_str()) != RET_OK)
		{
			SPDLOG(error, "try to connect to mt4 server failed---{}", cnt + 1);
			cnt++;
			std::this_thread::sleep_for(std::chrono::seconds(1));
		}
		else
		{
			cnt = 0;
			break;
		}
	}
	if (cnt != 0)
	{
		SPDLOG(error, "3 times of connecting to mt4 server failed.");
		return false;
	}

	cnt = 0;
	while (cnt < 3)
	{
		if (m_managerInterDirtClose->Connect(host.c_str()) != RET_OK)
		{
			SPDLOG(error, "try to connect to mt4 server failed---{}", cnt + 1);
			cnt++;
			std::this_thread::sleep_for(std::chrono::seconds(1));
		}
		else
		{
			cnt = 0;
			break;
		}
	}
	if (cnt != 0)
	{
		SPDLOG(error, "3 times of connecting to mt4 server failed.");
		return false;
	}

	return true;
}

bool MT4Conn::mt4IsConnected()
{
	if (m_managerInterPump->IsConnected() != 0)
		return true;
	else
		return false;
}

void MT4Conn::NotifyCallBack(int code, int type, void* data, void *param)
{
	MT4Conn* self = (MT4Conn*)param;
	TradeRecord *trade = (TradeRecord*)data;
	ConSymbol* symbols = nullptr;
	int symbolsCount = 0;
	switch (code)
	{
	case PUMP_START_PUMPING:
		
		symbols = self->getPumpInter()->SymbolsGetAll(&symbolsCount);
		if (symbols) {
			for (int i = 0; i < symbolsCount; ++i) {
				self->getPumpInter()->SymbolAdd(symbols[i].symbol);
			}
			self->getPumpInter()->MemFree(symbols);
		}
		break;
	case PUMP_STOP_PUMPING:
		break;
	case PUMP_UPDATE_BIDASK:
		//std::thread(&MT4Conn::storeQuotes, self).detach();
		self->storeQuotes();
		break;
	case PUMP_UPDATE_SYMBOLS:
		break;
	case PUMP_UPDATE_GROUPS:
		break;
	case PUMP_UPDATE_USERS:
		break;
	case PUMP_UPDATE_ONLINE:
		break;
	case PUMP_UPDATE_TRADES:   
		self->ReceiveTradeOrder(code, type, data);
		break;
	case PUMP_UPDATE_ACTIVATION:
		break;
	case PUMP_UPDATE_MARGINCALL:
		break;
	case PUMP_UPDATE_REQUESTS:
		break;
	case PUMP_UPDATE_PLUGINS:
		break;
	case PUMP_UPDATE_NEWS:
		break;
	case PUMP_UPDATE_MAIL:
		break;
	default: break;
	}
}

bool MT4Conn::createConnToMT4()
{
	if (mt4Init() && mt4Login(std::stod(ReadConf::getInstance().getConnConf().find("login")->second), ReadConf::getInstance().getConnConf().find("passwd")->second))
		return true;
	else
		return false;
}

CManagerInterface* MT4Conn::getPumpInter()
{
	return m_managerInterPump;
}
CManagerInterface* MT4Conn::getDirtInterOpen()
{
	return m_managerInterDirtOpen;
}

CManagerInterface* MT4Conn::getDirtInterClose()
{
	return m_managerInterDirtClose;
}

void MT4Conn::storeQuotes()
{
	int total = 0;
	SymbolInfo si[32];

	while ((total = m_managerInterPump->SymbolInfoUpdated(si, 32)) > 0)
	{
		if (m_stop)
			break;
		for (auto i = 0; i < total; i++)
		{
			if (m_stop)
				return;
			{
				std::lock_guard<std::mutex> lck(m_quoteMtx);
				m_quote[si[i].symbol] = std::vector<double>{ si[i].ask, si[i].bid, si[i].point };
				PriceData::getInstance().setQuotes(si[i].symbol, si[i].bid, si[i].ask, si[i].point);
		
			}
		}
	}
}

void MT4Conn::getQuotes(std::string symbol, std::vector<double>& quote)
{
	std::lock_guard<std::mutex> lck(m_quoteMtx);
	quote = m_quote[symbol];
}


void MT4Conn::ReceiveTradeOrder(int code, int type, void *data)
{
		if (data == NULL) {
			return;
		}
		TradeRecord *trade = (TradeRecord*)data;
		if (m_logins.find(trade->login) == m_logins.end())
			return;

		ORDER order;
		order.cmd = trade->cmd;
		order.symbol = trade->symbol;
		order.order = trade->order;
		m_orders.insert(order);
}

std::set<ORDER> MT4Conn::GetOrders()
{
	std::lock_guard<std::mutex> lgc(m_orderMtx);
	return m_orders;
}


void MT4Conn::DeleteOrders(ORDER order)
{
	std::lock_guard<std::mutex> lgc(m_orderMtx);
	//m_orders.erase(order);
}

void MT4Conn::InsertOrders(ORDER order)
{
	std::lock_guard<std::mutex> lgc(m_orderMtx);
	//m_orders.insert(order);
}