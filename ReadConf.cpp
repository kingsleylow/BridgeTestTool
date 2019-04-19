#include "pch.h"
#include "ReadConf.h"
#include "rapidxml/rapidxml.hpp"
#include "rapidxml/rapidxml_utils.hpp"
#include <fstream>
#include <iostream>
#include <windows.h>

ReadConf ReadConf::m_self;

using namespace rapidxml;

void ReadConf::readConf(const std::string& conf)
{
	m_hadReadConf = true;
	assert(!conf.empty());
	std::fstream _file;
	std::string path;
	getProjectPath(path);
	if (path.empty())
	{
		return;
	}
	m_projectPath = path;
	std::string fullPath = path + conf;
	_file.open(fullPath, std::ios::in);

	if (!_file)
	{
		std::ofstream ofs(path + "logs/error.log");
		ofs << "read configure file: " << fullPath << "failed.";
		ofs.close();
		return;
	}

	m_confPath = fullPath;

	file<> fdoc(m_confPath.c_str());
	xml_document<> doc;
	doc.parse<0>(fdoc.data());

	//root node
	xml_node<> *root = doc.first_node();
	
	xml_node<> *conn = root->first_node();
	xml_node<> *nodeConn = conn->first_node();
	
	while (nodeConn)
	{
		std::cout << nodeConn->name() << " " << nodeConn->value() << " " << m_ConnConf.size() << std::endl;
		m_ConnConf.insert(std::pair<std::string, std::string>(nodeConn->name(), nodeConn->value()));
		nodeConn = nodeConn->next_sibling();
	}

	xml_node<> *global = conn->next_sibling();
	xml_node<> *nodeGlobal = global->first_node();
	while (nodeGlobal)
	{
		m_GlobalConf.insert(std::pair<std::string, std::string>(nodeGlobal->name(), nodeGlobal->value()));
		if (std::string("order-type").compare(nodeGlobal->name()) == 0)
		{
			xml_attribute<>* att = nodeGlobal->first_attribute();
			m_GlobalConf[att->name()] = att->value();
			att = att->next_attribute();
			while (att)
			{
				m_GlobalConf[att->name()] = att->value();
				att = att->next_attribute();
			}
		}
		nodeGlobal = nodeGlobal->next_sibling();
	}

	xml_node<> *specific = global->next_sibling();
	xml_node<>* nodeSpecific = specific->first_node();
	while (nodeSpecific)
	{
		m_SpecificConf.insert(std::pair<std::string, std::string>(nodeSpecific->name(), nodeSpecific->value()));
		nodeSpecific = nodeSpecific->next_sibling();
	}
}


bool ReadConf::writeConf(const std::string& key, const std::string& value)
{
	assert(!key.empty() && !m_confPath.empty());
	file<> fdoc(m_confPath.c_str());
	
	xml_document<> doc;
	doc.parse<0>(fdoc.data());

	//root node
	xml_node<>* root = doc.first_node();

	xml_node<>* node = root->first_node(key.c_str());
	if (node == nullptr)
		node = root->first_node();
	while (node)
	{
		xml_node<> *tmp = node->next_sibling(key.c_str());
		if (tmp == nullptr)
			node = node->next_sibling();
		else
		{
			tmp->value(value.c_str());
			return true;
		}
	}
	return false;
}

void ReadConf::getProjectPath(std::string &path)
{
	char p[MAX_PATH] = {0};
	if (GetModuleFileName(0, p, MAX_PATH))
	{
		path = p;
		path = path.substr(0, path.rfind("\\"));
		path += "\\";
	}
}

std::vector<std::string>& ReadConf::getAllSymbols()
{
	if(!m_hadReadConf)
		readConf("config/bridgeTestTool.xml");
	if (m_symbols.size() == 0)
	{
		std::string gSymbols = ReadConf::getInstance().getGlobalConf().find("symbols")->second;
		split(gSymbols, m_symbols, ';');
	}
	return m_symbols;
}

std::vector<std::string>& ReadConf::getAllLogins()
{
	if (!m_hadReadConf)
		readConf("config/bridgeTestTool.xml");
	if (m_logins.size() == 0)
	{
		std::string logins = ReadConf::getInstance().getGlobalConf().find("orderby")->second;
		split(logins, m_logins, ';');
	}
	return m_logins;
}

void  ReadConf::split(std::string& gSymbols, std::vector<std::string>& v, const char delimeter)
{
	int begin = 0;
	int end = gSymbols.find(delimeter);
	std::string tmp = gSymbols.substr(0, end);
	v.push_back(tmp);
	while (end != std::string::npos)
	{
		begin = end + 1;
		end = gSymbols.find(delimeter, end + 1);
		tmp = gSymbols.substr(begin, end-begin);
		v.push_back(tmp);
	}
}