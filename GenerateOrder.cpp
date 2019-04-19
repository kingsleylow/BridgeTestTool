// BridgeTestTool.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include "GenerateOrder.h"
#include <iostream>
#include <fstream>
#include <thread>
#include "Logger.h"
#include "ReadConf.h"
#include "PriceData.h"
#include "ConnToMt4.h"


GenerateOrders::GenerateOrders()
{}

GenerateOrders::GenerateOrders(CManagerInterface* pump, CManagerInterface* dirtOpen, CManagerInterface* dirtClose)
{
	assert(pump);
	assert(dirtOpen);
	assert(dirtClose);
	m_managerInterPump = pump;
	m_managerInterDirtOpen = dirtOpen;
	m_managerInterDirtClose = dirtClose;

	std::ifstream ifs("orders.txt", std::ifstream::in | std::ifstream::binary);
	std::string order;

	while (getline(ifs, order))
	{
		std::vector<std::string> v;
		split(order, v, ";");
		ORDER tmpO;
		tmpO.cmd = std::stoi(v.at(0));
		tmpO.order = std::stoi(v.at(1));
		tmpO.symbol = v.at(2);
		
		MT4Conn::getInstance().InsertOrders(tmpO);
	}
}

GenerateOrders::~GenerateOrders()
{
	std::ofstream ofs("orders.txt", std::ofstream::out | std::ifstream::binary);
	std::set<ORDER> orders = MT4Conn::getInstance().GetOrders();
	for (auto &o : orders)
	{
		ofs << o.cmd << ";" << o.order << ";" << o.symbol << std::endl;
	}
	ofs.close();
}
void GenerateOrders::exec(int login)
{
	srand(time(0));

	int type = rand() % 2;
	if (0 == type)
	{
		generateCommonOrders(login);
	}
	else if (1 == type)
	{
		generatePendingOrders(login);
	}
}

void GenerateOrders::run()
{
	std::this_thread::sleep_for(std::chrono::seconds(3));
	m_symbols = ReadConf::getInstance().getAllSymbols();
	m_logins = ReadConf::getInstance().getAllLogins();
	for (auto &login : m_logins)
	{
		m_loginsSet.insert(login);
	}

	int gThreads = m_logins.size();
	for (int i = 0; i < gThreads; i++)
	{
		std::thread(&GenerateOrders::exec, this, std::stoi(m_logins[i])).detach();
	}

	std::thread(&GenerateOrders::closeOrders, this, -1, -1, -1).join();
}

void GenerateOrders::setPumpInter(CManagerInterface* pump)
{
	assert(pump);
	m_managerInterPump = pump;
}
void GenerateOrders::setDirtInterOpen(CManagerInterface* dirt)
{
	assert(dirt);
	m_managerInterDirtOpen = dirt;
}

void GenerateOrders::setDirtInterClose(CManagerInterface* dirt)
{
	assert(dirt);
	m_managerInterDirtClose = dirt;
}

void GenerateOrders::closeOrders(int login, int orderNo, double volume)
{
	srand(time(0));
	int interval = std::stoi(ReadConf::getInstance().getGlobalConf().find("interval-close")->second);
	//int orderTotal = 0;
	while (true)
	{
		//TradeRecord* orderArray = m_managerInterDirtClose->TradesRequest(&orderTotal);
		using std::chrono::high_resolution_clock;
		auto start = high_resolution_clock::now();
		//int orderby = std::stoi(ReadConf::getInstance().getGlobalConf().find("orderby")->second);
		int orderby = login;
		int duration = std::stoi(ReadConf::getInstance().getGlobalConf().find("duration")->second);
		int realOrderCount = 0;
		double delta_d = 0;

		int res = RET_ERROR;
		TradeTransInfo info = { 0 };
		ZeroMemory(&info, sizeof(info));

		std::set<ORDER> logins = MT4Conn::getInstance().GetOrders();
		for (auto &o : logins)
		{
			info.type = TT_BR_ORDER_CLOSE;      // Transaction type
			info.price = o.cmd == 0 ? PriceData::getInstance().getQuotes(o.symbol).at(0) : PriceData::getInstance().getQuotes(o.symbol).at(1);
			info.order = o.order;
			strcpy_s(info.symbol, o.symbol.length() + 1, o.symbol.c_str());
			info.volume = 1;
			{
				std::lock_guard<std::mutex> lck(m_mtx);
				if ((res = m_managerInterDirtClose->TradeTransaction(&info)) != RET_OK)
				{
					Logger::getInstance()->info("symbol:{},Order close failed: {}{}", info.symbol, res, m_managerInterDirtClose->ErrorDescription(res));
					continue;
				}
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(interval));
			Logger::getInstance()->info("symbol:{},Order has been closed", info.symbol);
			realOrderCount++;

			auto delta = high_resolution_clock::now() - start;
			delta_d = std::chrono::duration_cast<std::chrono::duration<double>>(delta).count();
		}
	}
}


void GenerateOrders::generateCommonOrders(int login, openType type)
{
	int total = std::stod(ReadConf::getInstance().getGlobalConf().find("orders")->second);
	int vIndex = 0;

	int interval = std::stoi(ReadConf::getInstance().getGlobalConf().find("interval")->second);

	srand(time(0));
	int gOrders = 0;
	while(true)
	{
		int            res = RET_ERROR;
		TradeTransInfo info = { 0 };
		ZeroMemory(&info, sizeof(info));
		std::vector<double> quote;

		info.type = TT_BR_ORDER_OPEN;
		int cmd = std::stoi(ReadConf::getInstance().getGlobalConf().find("cmd")->second);
		if (cmd == 0)
			info.cmd = OP_BUY;
		else if (cmd == 1)
			info.cmd = OP_SELL;
		else
			info.cmd = rand() % 2 == 0 ? OP_BUY : OP_SELL;

		int r = rand() % m_symbols.size();
		strcpy_s(info.symbol, m_symbols[r].length()+1, m_symbols[r].c_str());
		info.volume = 1;
		info.price = info.cmd == OP_BUY ? PriceData::getInstance().getQuotes(info.symbol).at(1) : PriceData::getInstance().getQuotes(info.symbol).at(0);
		info.orderby = login;
		//---
		Logger::getInstance()->info("{} {}", info.symbol, info.cmd == OP_SELL ? "sell" : "buy");
		{
			std::lock_guard<std::mutex> lck(m_mtx);
			if ((res = m_managerInterDirtOpen->TradeTransaction(&info)) != RET_OK)
			{
				Logger::getInstance()->info("symbol:{},Order open failed: {}{}", m_symbols[vIndex%m_symbols.size()], res, m_managerInterDirtOpen->ErrorDescription(res));
				continue;
			}
			gOrders++;
		}
		Logger::getInstance()->info("symbol:{},Order has been opened", m_symbols[vIndex%m_symbols.size()]);
		vIndex++;
		std::this_thread::sleep_for(std::chrono::milliseconds(interval));
		if (gOrders >= 1000)
		{
			std::this_thread::sleep_for(std::chrono::seconds(60));
			gOrders = 0;
		}
	}
}

void GenerateOrders::generatePendingOrders(int login, pendingType type)
{
	int total = std::stod(ReadConf::getInstance().getGlobalConf().find("orders")->second);
	int vIndex = 0;

	int interval = std::stoi(ReadConf::getInstance().getGlobalConf().find("interval-pending")->second);

	srand(time(0));
	while(true)
	{
		int  res = RET_ERROR;
		TradeTransInfo info = { 0 };
		ZeroMemory(&info, sizeof(info));
		std::vector<double> quote;

		info.type = TT_BR_ORDER_OPEN;
		int cmd = std::stoi(ReadConf::getInstance().getGlobalConf().find("pending-cmd")->second);
		if (cmd == 0)
			info.cmd = OP_BUY_LIMIT;
		else if (cmd == 1)
			info.cmd = OP_BUY_STOP;
		else if (cmd == 2)
			info.cmd = OP_SELL_LIMIT;
		else if (cmd == 3)
			info.cmd = OP_SELL_STOP;
		else
			info.cmd = rand() % 4 + 2;

		int r = rand() % m_symbols.size();
		strcpy_s(info.symbol, m_symbols[r].length() + 1, m_symbols[r].c_str());

		info.volume = 1;
		info.orderby = login;
		info.expiration = 0;

		if (info.cmd == OP_SELL_LIMIT)
		{
			info.price = PriceData::getInstance().getQuotes(info.symbol).at(0) + PriceData::getInstance().getQuotes(info.symbol).at(2)*150;
			/*info.sl = PriceData::getInstance().getQuotes(info.symbol).at(0) + PriceData::getInstance().getQuotes(info.symbol).at(2) * 500;
			info.tp = PriceData::getInstance().getQuotes(info.symbol).at(0) - PriceData::getInstance().getQuotes(info.symbol).at(2) * 500;*/
		}
		else if (info.cmd == OP_BUY_STOP)
		{
			info.price = PriceData::getInstance().getQuotes(info.symbol).at(1) + PriceData::getInstance().getQuotes(info.symbol).at(2) * 150;
			/*info.sl = PriceData::getInstance().getQuotes(info.symbol).at(1) - PriceData::getInstance().getQuotes(info.symbol).at(2) * 500;
			info.tp = PriceData::getInstance().getQuotes(info.symbol).at(1) + PriceData::getInstance().getQuotes(info.symbol).at(2) * 500;*/
		}
		else if (info.cmd == OP_BUY_LIMIT)
		{
			info.price = PriceData::getInstance().getQuotes(info.symbol).at(1) - PriceData::getInstance().getQuotes(info.symbol).at(2) * 150;
			/*info.sl = PriceData::getInstance().getQuotes(info.symbol).at(1) - PriceData::getInstance().getQuotes(info.symbol).at(2) * 500;
			info.tp = PriceData::getInstance().getQuotes(info.symbol).at(1) + PriceData::getInstance().getQuotes(info.symbol).at(2) * 500;*/
		}
		else if (info.cmd == OP_SELL_STOP)
		{
			info.price = PriceData::getInstance().getQuotes(info.symbol).at(0) - PriceData::getInstance().getQuotes(info.symbol).at(2) * 150;
			/*info.sl = PriceData::getInstance().getQuotes(info.symbol).at(0) + PriceData::getInstance().getQuotes(info.symbol).at(2) * 500;
			info.tp = PriceData::getInstance().getQuotes(info.symbol).at(0) - PriceData::getInstance().getQuotes(info.symbol).at(2) * 500;*/
		}
		//---
		{
			std::lock_guard<std::mutex> lck(m_mtx);
			if ((res = m_managerInterDirtOpen->TradeTransaction(&info)) != RET_OK)
			{
				Logger::getInstance()->info("symbol:{}, cmd:{},Order pending failed: errno:{} des:{}", info.symbol, info.cmd, res, m_managerInterDirtOpen->ErrorDescription(res));
				continue;
			}
		}
		Logger::getInstance()->info("symbol:{},Order has been opened", m_symbols[vIndex%m_symbols.size()]);
		vIndex++;
		//std::this_thread::sleep_for(std::chrono::minutes(3));
		std::this_thread::sleep_for(std::chrono::minutes(interval));
	}
}

void GenerateOrders::generateSpecificOrder(int login, std::string symbol, double volume, openType type)
{

}


void GenerateOrders::generateOrdersPerSeconds(int login, openType) // need to modify,add 1 seconds limit
{
	std::vector<std::string> symbols = ReadConf::getInstance().getAllSymbols();
	int total = std::stod(ReadConf::getInstance().getGlobalConf().find("orders")->second);
	int vIndex = 0;

	int interval = std::stoi(ReadConf::getInstance().getGlobalConf().find("interval")->second);

	srand(time(0));
	for (auto i = 0; i < total; i++)
	{
		int            res = RET_ERROR;
		TradeTransInfo info = { 0 };
		ZeroMemory(&info, sizeof(info));
		std::vector<double> quote;

		info.type = TT_BR_ORDER_OPEN;
		int cmd = std::stoi(ReadConf::getInstance().getGlobalConf().find("cmd")->second);
		if (cmd == 0)
			info.cmd = OP_BUY;
		else if (cmd == 1)
			info.cmd = OP_SELL;
		else
			info.cmd = rand() % 2 == 0 ? OP_BUY : OP_SELL;

		int r = rand() % m_symbols.size();
		strcpy_s(info.symbol, m_symbols[r].length() + 1, m_symbols[r].c_str());

		info.volume = 1;
		info.price = info.cmd == OP_BUY ? PriceData::getInstance().getQuotes(info.symbol).at(1) : PriceData::getInstance().getQuotes(info.symbol).at(0);
		info.orderby = login;
		//---
		Logger::getInstance()->info("{} {}", info.symbol, info.cmd == OP_SELL ? "sell" : "buy");
		{
			std::lock_guard<std::mutex> lck(m_mtx);
			if ((res = m_managerInterDirtOpen->TradeTransaction(&info)) != RET_OK)
			{
				Logger::getInstance()->info("symbol:{},Order open failed: {}{}", symbols[vIndex%symbols.size()], res, m_managerInterDirtOpen->ErrorDescription(res));
				continue;
			}
		}
		Logger::getInstance()->info("symbol:{},Order has been opened", symbols[vIndex%symbols.size()]);
		vIndex++;
		std::this_thread::sleep_for(std::chrono::milliseconds(interval));
	}
}

void GenerateOrders::modifySpecificOrderParam(int login, int orderNo)
{}
void GenerateOrders::modifySymbolPrice(std::string symbol, double ask, double bid)
{

}


void  GenerateOrders::split(const std::string& in, std::vector<std::string>& out, const std::string& delimeter)
{
	int begin = 0;
	int end = in.find(delimeter);
	std::string tmp = in.substr(0, end);
	out.push_back(tmp);
	while (end != std::string::npos)
	{
		begin = end + 1;
		end = in.find(delimeter, end + 1);
		tmp = in.substr(begin, end - begin);
		out.push_back(tmp);
	}
}