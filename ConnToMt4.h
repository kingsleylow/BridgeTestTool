#pragma once
#include "ReadConf.h"
#include <unordered_map>
#include <vector>
#include <mutex>
#include <set>
#include "MT4/MT4ManagerAPI.h"


struct ORDER
{
	int order;
	std::string symbol;
	int cmd;

	inline bool operator< (const ORDER& other) const
	{
		if (order < other.order)
			return true;
		else
			return false;
	}
};

class MT4Conn
{
public:
	~MT4Conn();
	static MT4Conn& getInstance();

private:
	MT4Conn();
	MT4Conn(const MT4Conn& other)=default;
	MT4Conn& operator=(const MT4Conn& other) = default;

public:
	bool createConnToMT4();
	CManagerInterface* getPumpInter();
	CManagerInterface* getDirtInterOpen();
	CManagerInterface* getDirtInterClose();
	void getQuotes(std::string symbol, std::vector<double>& quote);
	std::set<ORDER> GetOrders();
	void DeleteOrders(ORDER order);
	void InsertOrders(ORDER order);
private:
    /************************************************
	** Callback by manager interface
	** Arguments:
	**   code:  the type of change
	**   type:  transaction type: TRANS_ADD,TRANS_DELETE,TRANS_UPDATE,TRANS_CHANGE_GRP
	**   data:  a pointer to the updated data: the pointer type depends on the type of change
	**   param: the pointer that was passed as a parameter to the PumpingSwitchEx function
	** Returns:
	**   none
	*************************************************/
	static	void __stdcall NotifyCallBack(int code, int type, void* data, void *param);

	/************************************************
	** Connection to a trading server
	** Arguments:
	**   host: trading server's ip and port, format like "localhost:443".
	** Returns:
	**   RET_OK(0): success
	**   RET_ERROR(2): Common error.
    **   RET_INVALID_DATA(3): Invalid information.
    **   RET_TECH_PROBLEM(4): Technical errors on the server.
    **   RET_OLD_VERSION(5): Old terminal version.
    **   RET_NO_CONNECT(6): No connection.
    **   RET_NOT_ENOUGH_RIGHTS(7): Not enough permissions to perform the operation.
    **   RET_TOO_FREQUENT(8): Too frequent requests.
	**   RET_MALFUNCTION(9): Operation cannot be completed.
	**   RET_GENERATE_KEY(10): Key generation is required.
    **   RET_SECURITY_SESSION(11): Connection using extended authentication.
	*************************************************/
	int mt4Conn(const std::string& host);
	bool mt4PumpConn(const std::string& host);
	bool mt4DirtConn(const std::string& host);

	/************************************************
    ** Authentication on the trading server using a manager account
    ** Arguments:
    **   login:  The login of the manager for connection.
    **   passwd:  The password of the manager for connection.
    ** Returns:
    **   true: success.
	**   false : failure.
    *************************************************/
	bool mt4Login(const int login, const std::string& passwd);
	bool mt4PumpLogin(const int login, const std::string& passwd);
	bool mt4DirtLogin(const int login, const std::string& passwd);

	/************************************************
	** Checks the state of connection to a trading server.
	** Arguments:
	**   none.
	** Returns:
	**   false: disconnected.
	**   true : connected
	*************************************************/
	bool mt4IsConnected();

	/************************************************
	** create interface of manager api
	** Arguments:
	**   mt4LibPath: MT4 lib's path
	** Returns:
	**   true: success
	**   false: failure
	*************************************************/
	bool mt4Init();
	//store quotes
	void storeQuotes();
	void ReceiveTradeOrder(int code, int type, void *data);

	

private:
	CManagerInterface* m_managerInterPump;
	CManagerInterface* m_managerInterDirtOpen;
	CManagerInterface* m_managerInterDirtClose;
	CManagerFactory    m_factoryInter;

	std::mutex m_quoteMtx;
	std::mutex m_orderMtx;

	std::unordered_map<std::string, std::vector<double>> m_quote;

	std::set<ORDER> m_orders;
	
	std::set<int> m_logins;
	bool m_stop;

	static MT4Conn m_self;
};

