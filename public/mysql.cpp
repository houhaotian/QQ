#include <iostream>
#include <string>
#include <winsock2.h>
#include <mysql.h>

using namespace std;




class SelfMysql
{
public:
	SelfMysql(string inName, string inAccount, string inPasswd);
	SelfMysql(string inAccount, string inPasswd);
	MYSQL * mysql_get_conn();
	int mysql_execute(char *, MYSQL_RES **);
	int qq_insert();
	int qq_get();
	void close();

private:
	string name;
	string account;
	string passwd;

private:
	MYSQL *mysql_conn = NULL;

	const string mysql_server = "localhost";//mysql 数据库主机
	const string mysql_user = "root";//mysql 管理登录用户
	const string mysql_password = "123456";//mysql 管理登录密码
	const string mysql_database = "selfqq";//使用的数据库。
	const string mysql_table = "qqdata";
	int my_port = 3306;
};

SelfMysql::SelfMysql(string inName, string inAccount, string inPasswd) :name(inName), account(inAccount), passwd(inPasswd)
{
}


SelfMysql::SelfMysql(string inAccount, string inPasswd) : account(inAccount), passwd(inPasswd)
{
}

MYSQL * SelfMysql::mysql_get_conn()
{
	if (mysql_conn != NULL)
		return mysql_conn;

	//初始化，创建数据操作符
	mysql_conn = mysql_init(NULL);

	//链接数据库
	if (!mysql_real_connect
	(mysql_conn, mysql_server.c_str(), mysql_user.c_str(), mysql_password.c_str(), mysql_database.c_str(), my_port, NULL, 0)) {
		fprintf(stderr, "%s\n", mysql_error(mysql_conn));
		return NULL;
	}
	return mysql_conn;
}


int  SelfMysql::mysql_execute(char *sqlcmd, MYSQL_RES **pres)
{
	static	MYSQL *conn;
	MYSQL_RES *res;
	conn = mysql_get_conn();

	//执行 指定数据库
	if (mysql_query(conn, sqlcmd)) {
		fprintf(stderr, "%s\n", mysql_error(conn));
		return -1;
	}
	//获取 sql语句执行的结果
	res = mysql_use_result(conn);

	if (pres != NULL)
		*pres = res;
	return 0;
}

int SelfMysql::qq_insert()
{
	char sql_cmd[1024];

	sprintf_s(sql_cmd, "insert into %s values('%s','%s',%s)", mysql_table.c_str(), name.c_str(), account.c_str(), passwd.c_str());
	printf("%s\n", sql_cmd);

	mysql_execute(sql_cmd, NULL);
	printf("%s\n", sql_cmd);

	//	close();
	return 0;
}



int SelfMysql::qq_get()
{
	char sql_cmd[1024];
	MYSQL_RES *res;
	MYSQL_ROW row;
	int fnum = 0;
	char name[16];
	char account[16];
	char passwd[16];

	sprintf_s(sql_cmd, "select * from %s", mysql_table.c_str());
	printf("%s\n", sql_cmd);
	mysql_execute(sql_cmd, &res);


	fnum = mysql_num_fields(res);//返回值为列数。

								 //一行一行的获取数据，依次向后
	while ((row = mysql_fetch_row(res)) != NULL) {
		strcpy_s(name, row[0]);
		strcpy_s(account, row[1]);
		strcpy_s(passwd, row[2]);

		printf(" %s,%s,%s\n", name, account, passwd);
	}
	mysql_free_result(res);
	//	close();
	return 0;
}


void SelfMysql::close()
{
	mysql_close(mysql_get_conn());
}



#if 0
int main()
{
	string name = "侯昊天";
	string account = "houhaotian";
	string passwd = "123456";

	SelfMysql m1(name, account, passwd);

	m1.qq_insert();
	m1.qq_get();
}
#endif