#ifndef __3RDPARTY_H__
#define __3RDPARTY_H__
#include <winsock2.h>
#include <mysql.h>

using namespace std;

class SelfMysql
{
public:
	SelfMysql(string inName, string inAccount, string inPasswd);
	SelfMysql(string inAccount, string inPasswd);
	MYSQL * _mysql_get_conn();
	int _mysql_execute(char *, MYSQL_RES **);
	int qq_insert();
	int qq_check_account_exist(string inAccount);
	void close();

private:
	string name;
	string account;
	string passwd;

private:
	/*数据库相关参数*/
	MYSQL *mysql_conn = NULL;

	const string mysql_server = "localhost";//mysql 数据库主机
	const string mysql_user = "root";//mysql 管理登录用户
	const string mysql_password = "123456";//mysql 管理登录密码
	const string mysql_database = "selfqq";//使用的数据库。
	const string mysql_table = "qqdata";
	int my_port = 3306;
};

#endif
