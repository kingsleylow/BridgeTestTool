// BridgeTestTool.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include "ConnToMt4.h"
#include <iostream>
#include <thread>
#include "ReadConf.h"
#include "Logger.h"
#include "GenerateOrder.h"


int main()
{
	ReadConf::getInstance().readConf("config/bridgeTestTool.xml");

	if (MT4Conn::getInstance().createConnToMT4())
		Logger::getInstance()->debug("connect to mt4 success.");
	else
	{
		Logger::getInstance()->error("connect to mt4 failed.");
		return -1;
	}

	GenerateOrders g;
	g.setDirtInterOpen(MT4Conn::getInstance().getDirtInterOpen());
	g.setDirtInterClose(MT4Conn::getInstance().getDirtInterClose());
	g.setPumpInter(MT4Conn::getInstance().getPumpInter());
	//g.closeOrders();
	std::cout << "begin running ..." << std::endl;
	g.run();

	system("pause");
}


