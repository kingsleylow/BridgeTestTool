#pragma once
#include <map>
#include <vector>
#include <mutex>

class PriceData
{
public:
	~PriceData();
	static PriceData& getInstance();
	std::vector<double> getQuotes(std::string symbol);
	void setQuotes(std::string symbol, double bid, double ask, double point);
private:
	PriceData() = default;
	PriceData(const PriceData& other) = default;
	PriceData(PriceData&& other) = default;
	PriceData& operator= (const PriceData& other) = default;

private:
	static PriceData m_self;
	std::map<std::string, std::vector<double>> m_quotes;
	std::mutex m_mtx;
};