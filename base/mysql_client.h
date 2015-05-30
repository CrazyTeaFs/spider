#ifndef _MYSQL_CLIENT_H
#define _MYSQL_CLIENT_H

#include <string>
#include <vector>
#include <pthread.h>
#include <mysql/mysql.h>
#include <mysql/mysqld_error.h>

typedef std::vector< std::vector<std::string> > Data;

typedef struct _db_info {
	std::string ip;
	short port;
	std::string user;
	std::string passwd;
	std::string database;
	std::string charset;
} db_info;

class MysqlClient {
public:
    MysqlClient();
    ~MysqlClient();

	int Initialize(const std::string &ip, short port, const std::string &user, const std::string &passwd, const std::string &database, const std::string &charset);
    int Connect();
	int Reconnect();
    int CloseConnect();
	int ExecuteSql(const std::string &sql);
	int GetResult(Data &data);
	long long GetInsertId();
	long long GetAffectedRows();

private:
    pthread_mutex_t lock;
    int			i_conn;
	int 		affected_rows;
	int 		insert_id;
    MYSQL       connection;
    MYSQL_RES	*result;
	db_info		config;	
};

#endif
