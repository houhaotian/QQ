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
	/*���ݿ���ز���*/
	MYSQL *mysql_conn = NULL;

	const string mysql_server = "localhost";//mysql ���ݿ�����
	const string mysql_user = "root";//mysql �����¼�û�
	const string mysql_password = "123456";//mysql �����¼����
	const string mysql_database = "selfqq";//ʹ�õ����ݿ⡣
	const string mysql_table = "qqdata";
	int my_port = 3306;
};

#endif
