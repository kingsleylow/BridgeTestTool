#include "PriceData.h"

PriceData PriceData::m_self;

PriceData::~PriceData()
{}

PriceData& PriceData::getInstance()
{
	return m_self;
}

std::vector<double> PriceData::getQuotes(std::string symbol)
{
	std::lock_guard<std::mutex> lgd(m_mtx);
	return m_quotes[symbol];
}

void PriceData::setQuotes(std::string symbol, double bid, double ask, double point)
{
	std::lock_guard<std::mutex> lgd(m_mtx);
	m_quotes[symbol] = std::vector<double>{ bid,ask, point };
}


