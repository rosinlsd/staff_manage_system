/*************************************************************************
	#	 FileName	: common.h
	#	 Author     : Rosin
	#	 Email      : Rosin_ls@163.com
	#	 Created	: Wed 17 Jul 2019 09:45:43 AM CST
 ************************************************************************/
#ifndef __COMMON_H__
#define __COMMON_H__

#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<errno.h>
#include<string.h>
#include<sqlite3.h>
#include<sys/wait.h>
#include<signal.h>
#include<time.h>
#include<pthread.h>
#include<sys/stat.h>
#include<sqlite3.h>

#define STAFF_DATA "staff_manage_system.db"

/*普通用户*/
#define USER_LOGIN 1     //登录
#define USER_MODIFY 2    //修改
#define USER_QUERY 3     //查询

/*管理员用户*/
#define ADMIN_LOGIN 4	 //登录
#define ADMIN_MODIFY 5   //修改	
#define ADMIN_ADDUSER 6  //增加用户
#define ADMIN_DELUSER 7  //删除用户
#define ADMIN_QUERY 8    //查询
#define ADMIN_HISTORY 9  //历史

#define QUIT 10     

#define ADMIN 0	//管理员
#define USER  1	//用户

#define NAMELEN 16
#define DATALEN 128



/*员工基本信息*/
typedef struct staff_info{
	int  no; 			//员工编号
	int  usertype;  	//ADMIN 1	USER 2	 
	char name[NAMELEN];	//姓名
	char passwd[8]; 	//密码
	int  age; 			//年龄
	char phone[NAMELEN];//电话
	char work[DATALEN]; //职位
}staff_info_t;

/*定义双方通信的结构体信息*/
typedef struct {
	int  msgtype;     //请求的消息类型
	int  usertype;    //ADMIN 1   	USER 2	   
	char username[NAMELEN];  //姓名
	char passwd[8];			 //登陆密码
	char recvmsg[DATALEN];   //通信的消息
	int  flags;      //标志位
	void *released;
	staff_info_t info;      //员工信息
}MSG;

int do_login(int sockfd);  //主菜单
int admin_or_user_login(int sockfd,MSG *msg); //帐号登录



/*客户端函数声明*/
/*管理员模式*/
void admin_menu(int sockfd,MSG *msg); //管理员菜单
void do_admin_query(int sockfd,MSG *msg);
void do_admin_modification(int sockfd,MSG *msg); //管理员修改
void do_admin_adduser(int sockfd,MSG *msg); //管理员添加用户
void do_admin_deluser(int sockfd,MSG *msg); //管理员删除用户
void do_admin_history (int sockfd,MSG *msg); //查询历史记录

/*普通用户模式*/
void user_menu(int sockfd,MSG *msg); //用户菜单
void do_user_query(int sockfd,MSG *msg); //查询
void do_user_modification(int sockfd,MSG *msg); //用户修改



/*服务器函数声明*/

int process_client_request(int acceptfd,MSG *msg); //客户端请求处理（任务分派）

/*普通用户模式*/
int process_user_or_admin_login_request(int acceptfd,MSG *msg); //管理员/用户登录
int process_user_query_request(int acceptfd,MSG *msg);  //查找
int process_user_modify_request(int acceptfd,MSG *msg); //修改


/*管理员模式*/
int process_admin_query_request(int acceptfd,MSG *msg); // 查找
int process_admin_modify_request(int acceptfd,MSG *msg); //员工信息修改
int process_admin_adduser_request(int acceptfd,MSG *msg); //新增员工信息
int process_admin_deluser_request(int acceptfd,MSG *msg); //删除用户
int process_admin_history_request(int acceptfd,MSG *msg); //历史记录

int process_client_quit_request(int acceptfd,MSG *msg); //用户退出（未实现）

//int history_callback(void *arg, int ncolumn, char **f_value, char **f_name);
void get_system_time(char* timedata);
void history_init(MSG *msg);    


#endif 
