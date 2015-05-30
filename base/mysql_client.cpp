#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "mysql_client.h"
#include "log.h"

using namespace std;

MysqlClient::MysqlClient():i_conn(0), affected_rows(0), insert_id(0), result(NULL){}

MysqlClient::~MysqlClient() {
	if (result) {
		mysql_free_result(result);
	}
	CloseConnect();
    pthread_mutex_destroy(&lock);
}

int MysqlClient::Initialize(const string &ip, short port, const string &user, const string &passwd, const string &database, const string &charset) {
    pthread_mutex_init(&lock, NULL);

	config.ip = ip; 
	config.port = port; 
	config.user = user;
	config.passwd = passwd; 
	config.database = database;
	config.charset = charset; 

    if (mysql_init(&connection) == NULL) {
		ERROR("Mysql Init Fail, Error: %s", mysql_error(&connection));
		return -1;
	}

	return Connect();
}

int MysqlClient::Connect() {
    pthread_mutex_lock(&lock);
	if (i_conn) {
		return 0; 
	}

	if (mysql_real_connect(&connection, config.ip.c_str(), config.user.c_str()
	, config.passwd.c_str(), config.database.c_str(), config.port, NULL, 0)) {
		i_conn = 1;
		INFO("Successfully Connected to Mysql Server: IP(%s), Port(%d), Database(%s)", config.ip.c_str(), config.port, config.database.c_str());
		if (mysql_set_character_set(&connection, config.charset.c_str()) == 0){   
    	}  	
    	pthread_mutex_unlock(&lock);
		return 0;
	}
	else {
		ERROR("Connection Failed, Error:%s", mysql_error(&connection));
		if (mysql_errno(&connection)) {
    		pthread_mutex_unlock(&lock);
			return mysql_errno(&connection);
		}
		return -1;
	}
}

int MysqlClient::Reconnect() {
    if (i_conn == 1) {
        mysql_close(&connection);
		i_conn = 0;
	}

	return Connect();
}

int MysqlClient::CloseConnect() {
	if (i_conn == 1) {
    	pthread_mutex_lock(&lock);
		mysql_close(&connection);
    	pthread_mutex_unlock(&lock);
	}
	return 0;
}

int MysqlClient::ExecuteSql(const string &sql) {
	if (mysql_ping(&connection) != 0) {
		ERROR("Cannot Connect to Server, Error: %s", mysql_error(&connection));
		if (Reconnect() != 0) {
			return -1;
		}
	}

	TRACE("Sql Will Be Executed: %s", sql.c_str());
	if (mysql_query(&connection, sql.c_str()) != 0) {
		ERROR("Execute SQL Failed. Error:%s", mysql_error(&connection));
		return -1;
	}

    result = mysql_store_result(&connection);
	if (result == NULL && mysql_errno(&connection) != 0) {
		ERROR("Store SQL Result Failed. Error:%s", mysql_error(&connection));
		return -1;
	}

	return 0;
}

int MysqlClient::GetResult(Data &data) {
	if (result == NULL) {
		return 0;
	}

	MYSQL_ROW sqlrow;
	while((sqlrow = mysql_fetch_row(result))) {
		unsigned count = 0;	
		vector<string> row;
		while (count < mysql_field_count(&connection)) {
			row.push_back(string(sqlrow[count]));
			count++;
		}
		data.push_back(row);
	}

	mysql_free_result(result);
	result = NULL;

	return 0;
}

long long MysqlClient::GetInsertId() {
	return mysql_insert_id(&connection);
}

long long MysqlClient::GetAffectedRows() {
	return mysql_affected_rows(&connection);
}
