// BridgeTestTool.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#pragma once
#include "pch.h"
#include "MT4/MT4ManagerAPI.h"
#include <mutex>
#include <vector>
#include <set>

class GenerateOrders
{
public:
	GenerateOrders();
	GenerateOrders(CManagerInterface* pump, CManagerInterface* dirtOpen, CManagerInterface* dirtClose);
	~GenerateOrders();

	void setPumpInter(CManagerInterface* pump);
	void setDirtInterOpen(CManagerInterface* dirt);
	void setDirtInterClose(CManagerInterface* dirt);

	//using closeType = enum class CLOSETYPE{ALL, BY_LOGIN, BY_ORDERNO, BY_TIME, BY_VOLUME, BY_PARTIAL};
	void closeOrders(int login = -1, int orderNo = -1, double volume = 0);
	
	using openType = enum class Cmd { BUY, SELL};
	void generateCommonOrders(int login/*, openType type = Cmd::BUY*/);          //
	using pendingType = enum class PendingType {BUY_LIMIT, BUY_STOP, SELLL_IMIT, SELL_STOP};
	void generatePendingOrders(int login/*, pendingType type = PendingType::BUY_LIMIT*/);
	void generateSpecificOrder(int login, std::string symbol, double volume, openType type);
	void generateOrdersPerSeconds(int login, openType);

	void modifySpecificOrderParam(int login, int orderNo);
	void modifySymbolPrice(std::string symbol, double ask, double bid);

	void split(const std::string& in, std::vector<std::string>& out, const std::string& delimeter);

	void run();
	void exec(int login);

private:
	CManagerInterface* m_managerInterPump;
	CManagerInterface* m_managerInterDirtOpen;
	CManagerInterface* m_managerInterDirtClose;

	std::vector<std::string> m_symbols;
	std::vector<std::string> m_logins;
	std::mutex  m_mtx;
	std::set<std::string>  m_loginsSet;
};