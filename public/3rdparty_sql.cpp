#include <iostream>
#include <string>
#include <winsock2.h>
#include <mysql.h>
#include "3rdparty_sql.h"

using namespace std;
/*返回-1失败，0成功*/

SelfMysql::SelfMysql(string inName, string inAccount, string inPasswd) :name(inName), account(inAccount), passwd(inPasswd)
{
}


SelfMysql::SelfMysql(string inAccount, string inPasswd) : account(inAccount), passwd(inPasswd)
{
}

MYSQL * SelfMysql::_mysql_get_conn()
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


int  SelfMysql::_mysql_execute(char *sqlcmd, MYSQL_RES **pres)
{
	static	MYSQL *conn;
	MYSQL_RES *res;
	conn = _mysql_get_conn();

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
	int ret;

	sprintf_s(sql_cmd, "insert into %s values('%s','%s',%s)", mysql_table.c_str(), name.c_str(), account.c_str(), passwd.c_str());
	printf("%s\n", sql_cmd);

	ret = _mysql_execute(sql_cmd, NULL);
//	printf("%s\n", sql_cmd);
	//	close();
	return ret;
}


//判断sql中有没有account，没有则0，否则返回-1
int SelfMysql::qq_check_account_exist(string inAccount)
{
	char sql_cmd[1024];
	MYSQL_RES *res;
//	MYSQL_ROW row;
	int fnum = 0;
//	char name[16];
//	char account[16];
//	char passwd[16];
	int ret;
	sprintf_s(sql_cmd, "select * from %s where account = '%s'", mysql_table.c_str(), inAccount.c_str());
	printf("%s\n", sql_cmd);
	ret=_mysql_execute(sql_cmd, &res);
	if (ret < 0)
		return ret;

	fnum = mysql_num_fields(res);//返回值为列数。
	if (fnum)
		return -1;		//sql中有account
#if 0
 //一行一行的获取数据，依次向后
	while ((row = mysql_fetch_row(res)) != NULL)
	{
		strcpy_s(name, row[0]);
		strcpy_s(account, row[1]);
		strcpy_s(passwd, row[2]);
	}
	//	printf(" %s,%s,%s\n", name, account, passwd);
	mysql_free_result(res);
#endif
	//	close();
	return 0;//没有account
}


void SelfMysql::close()
{
	mysql_close(_mysql_get_conn());
}



#if 0
int main()
{
	string name = "侯昊天";
	string account = "houhaotian";
	string passwd = "123456";
	int ret;
	SelfMysql m1(name, account, passwd);
	ret = m1.qq_insert();

}
#endif