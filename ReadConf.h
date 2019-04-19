#pragma once
#include <map>
#include <vector>
#include <iostream>
class ReadConf
{
public:
	static ReadConf& getInstance() { return m_self; }
	void readConf(const std::string& conf);
	bool writeConf(const std::string& key, const std::string& value);
	const std::map<std::string, std::string>& getConnConf() { return m_ConnConf; }
	const std::map<std::string, std::string>& getGlobalConf() { return m_GlobalConf; }
	const std::map<std::string, std::string>& getSpecificConf() { return m_SpecificConf; }
	std::vector<std::string>& getAllSymbols();
	std::vector<std::string>& getAllLogins();
	void getProjectPath(std::string &path);
private:
	ReadConf() = default;
	~ReadConf() = default;

	ReadConf(const ReadConf& other) = default;
	ReadConf& operator= (const ReadConf& other) = default;
	ReadConf(ReadConf&& other) = default;

	void  split(std::string& gSymbols, std::vector<std::string>& v, const char delimeter);

private:
	std::map <std::string, std::string> m_ConnConf;
	std::map <std::string, std::string> m_GlobalConf;
	std::map <std::string, std::string> m_SpecificConf;
	std::string m_confPath;
	std::string m_projectPath;

	std::vector<std::string> m_symbols;
	std::vector<std::string> m_logins;

	static ReadConf m_self;

	bool m_hadReadConf;
};

