/*************************************************************************
	#	 FileName	: client.c
	#	 Author     : Rosin
	#	 Email      : Rosin_ls@163.com
	#	 Created	: Wed 17 Jul 2019 01:11:32 PM CST
 ************************************************************************/

#include "common.h"

int main(int argc, const char *argv[])
{
	//socket->填充->绑定->监听->等待连接->数据交互->关闭 
	int sockfd;
	int acceptfd;
	ssize_t recvbytes,sendbytes;
	struct sockaddr_in serveraddr;
	struct sockaddr_in clientaddr;
	socklen_t addrlen = sizeof(serveraddr);
	socklen_t cli_len = sizeof(clientaddr);

	//创建网络通信的套接字
	sockfd = socket(AF_INET,SOCK_STREAM, 0);
	if(sockfd == -1){
		perror("socket failed.\n");
		exit(-1);
	}
	printf("sockfd: %d.\n",sockfd); 

	//填充网络结构体
	memset(&serveraddr,0,sizeof(serveraddr));
	memset(&clientaddr,0,sizeof(clientaddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port   = htons(atoi(argv[2]));
	serveraddr.sin_addr.s_addr = inet_addr(argv[1]);

	if(connect(sockfd,(const struct sockaddr *)&serveraddr,addrlen) == -1){
		perror("connect failed");
		exit(-1);
	}

	do_login(sockfd);

	close(sockfd);

	return 0;
}

void show_userinfo(MSG *msg)
{
	printf("%s\n",msg->recvmsg);
	return;
}

/************************************************
 *函数名：do_login
 *参   数：套接字、消息结构体
 *返回值：是否登陆成功
 *功   能：登陆
 *************************************************/
int do_login(int sockfd)
{	
	int n;
	MSG msg;

	while(1){
		printf("*************************************************************\n");
		printf("********  1：管理员模式    2：普通用户模式    3：退出********\n");
		printf("*************************************************************\n");
		printf("请输入您的选择（数字）>>");
		scanf("%d",&n);
		getchar();

		switch(n)
		{
			case 1:
				msg.msgtype  = ADMIN_LOGIN;
				msg.usertype = ADMIN;
				break;
			case 2:
				msg.msgtype =  USER_LOGIN;
				msg.usertype = USER;
				break;
			case 3:
				msg.msgtype = QUIT;
				if(send(sockfd, &msg, sizeof(MSG), 0)<0)
				{
					perror("do_login send");
					return -1;
				}
				close(sockfd);
				exit(0);
			default:
				printf("您的输入有误，请重新输入\n"); 
		}

		admin_or_user_login(sockfd,&msg);
	}
}


/************************************************
 *函数名：admin_or_user_login
 *参   数：套接字、消息结构体
 *功   能：管理员/用户登陆
 *************************************************/
int admin_or_user_login(int sockfd,MSG *msg)
{
	printf("------------%s-----------%d.\n",__func__,__LINE__);
	//输入用户名和密码
	memset(msg->username, 0, NAMELEN);
	printf("请输入用户名：");
	scanf("%s",msg->username);
	getchar();

	memset(msg->passwd, 0, DATALEN);
	printf("请输入密码: ");
	scanf("%s",msg->passwd);
	getchar();

	//发送登陆请求
	send(sockfd, msg, sizeof(MSG), 0);
	//接受服务器响应
	recv(sockfd, msg, sizeof(MSG), 0);
	printf("msg->recvmsg :%s\n",msg->recvmsg);

	//判断是否登陆成功
	if(strncmp(msg->recvmsg, "OK", 2) == 0)
	{
		if(msg->usertype == ADMIN)
		{
			printf("亲爱的管理员，欢迎您登陆员工管理系统！\n");
			admin_menu(sockfd,msg);
		}
		else if(msg->usertype == USER)
		{
			printf("亲爱的用户，欢迎您登陆员工管理系统！\n");
			user_menu(sockfd,msg);
		}
	}
	else
	{
		printf("登陆失败！%s\n", msg->recvmsg);
		return -1;
	}

	return 0;
}





/**************************************
 *函数名：admin_menu
 *参   数：套接字、消息结构体
 *功   能：管理员菜单
 ****************************************/
void admin_menu(int sockfd,MSG *msg)
{
	int n;

	while(1)
	{
		printf("*************************************************************\n");
		printf("* 1：查询  2：修改 3：添加用户  4：删除用户  5：查询历史记录*\n");
		printf("* 6：退出													*\n");
		printf("*************************************************************\n");
		printf("请输入您的选择（数字）>>");
		scanf("%d",&n);
		getchar();

		switch(n)
		{
			case 1:
				do_admin_query(sockfd,msg);
				break;
			case 2:
				do_admin_modification(sockfd,msg);
				break;
			case 3:
				do_admin_adduser(sockfd,msg);
				break;
			case 4:
				do_admin_deluser(sockfd,msg);
				break;
			case 5:
				do_admin_history(sockfd,msg);
				break;
			case 6:
				msg->msgtype = QUIT;
				send(sockfd, msg, sizeof(MSG), 0);
				close(sockfd);
				exit(0);
			default:
				printf("您输入有误，请重新输入！\n");
		}
	}
}



/**************************************
 *函数名：do_query
 *参   数：消息结构体
 *功   能：查询
 ****************************************/
void do_admin_query(int sockfd,MSG *msg)
{
	printf("------------%s-----------%d.\n",__func__,__LINE__);
	msg->msgtype = ADMIN_QUERY;
	int n;
	
	printf("*************************************************************\n");
	printf(" *********  1：按人名查找   2：查找所有  3：退出  *********\n");	
	printf("*************************************************************\n");
	printf("请输入您的选择（数字）>>");
	scanf("%d",&n);
	getchar();

	if(n == 1){
		msg->flags = 1;
		memset(&msg->info,0,sizeof(staff_info_t));
		printf("请输入用户名：");
		scanf("%s",msg->info.name);
		getchar();
		
		send(sockfd, msg, sizeof(MSG), 0);
		recv(sockfd, msg, sizeof(MSG), 0);
		printf("工号\t用户类型\t姓名\t密码\t年龄\t电话\t职位\n");
		show_userinfo(msg);
		memset(&msg->info,0,sizeof(staff_info_t));	
	}else if(n == 2){
		msg->flags = 2;
		send(sockfd, msg, sizeof(MSG), 0);
		printf("工号\t用户类型\t姓名\t密码\t年龄\t电话\t职位\n");
		puts("=============================================================");	
		while(1){  //循环接收服务器消息，直至服务器发送“over”
			recv(sockfd, msg, sizeof(MSG), 0);
			if(strncmp(msg->recvmsg, "over",4) == 0)
				break;
			show_userinfo(msg);
			memset(&msg->info,0,sizeof(staff_info_t));
		} 	
	}else{
		return;
	}
	printf("以上为全部查询结果...\n");
}


/**************************************
 *函数名：admin_modification
 *参   数：消息结构体
 *功   能：管理员修改
 ****************************************/
void do_admin_modification(int sockfd,MSG *msg)//管理员修改
{
	printf("------------%s-----------%d.\n",__func__,__LINE__);
	int n;
	msg->msgtype = ADMIN_MODIFY;
	memset(&msg->info,0,sizeof(staff_info_t));
	
	printf("请输入要修改信息的员工工号：");
	scanf("%d",&msg->info.no);
	getchar();
	
	printf("*********************请输入要修改的选项**********************\n");
	printf("  ******	1：姓名	      2：年龄	   3：电话   ******\n");
	printf("  ******	4: 职位	      5：密码      6：退出   ******\n");
	printf("*************************************************************\n");
	printf("请输入您的选择（数字）>>");
	scanf("%d",&n);
	getchar();
	
	switch(n)
	{
		case 1:
			printf("请输入用户名：");
			msg->recvmsg[0] = 'N';
			scanf("%s",msg->info.name);getchar();
			break;
		case 2:
			printf("请输入年龄：");
			msg->recvmsg[0] = 'A';
			scanf("%d",&msg->info.age);getchar();
			break;
		case 3:
			printf("请输入电话：");
			msg->recvmsg[0] = 'P';
			scanf("%s",msg->info.phone);getchar();
			break;
		case 4:
			printf("请输入职位：");
			msg->recvmsg[0] = 'W';
			scanf("%s",msg->info.work);getchar();
			break;
		case 5:
			printf("请输入新密码:(数字)");
			msg->recvmsg[0] = 'M';
			scanf("%6s",msg->info.passwd);getchar();
			break;
		case 6:
			return ;
	}			

	send(sockfd, msg, sizeof(MSG), 0);
	recv(sockfd, msg, sizeof(MSG), 0);
	printf("%s",msg->recvmsg);

	printf("修改结束.\n");	
}


/**************************************
 *函数名：admin_adduser
 *参   数：消息结构体
 *功   能：管理员创建用户
 ****************************************/
void do_admin_adduser(int sockfd,MSG *msg)//管理员添加用户
{		
	printf("------------%s-----------%d.\n",__func__,__LINE__);
	char temp;
	msg->msgtype  = ADMIN_ADDUSER;
	memset(&msg->info,0,sizeof(staff_info_t));	
	
	while(1){
		printf("***************热烈欢迎新员工***************.\n");
		printf("请输入工号：");
		scanf("%d",&msg->info.no);
		printf("你输入的工号是：%d\n",msg->info.no);
		printf("工号信息一旦录入无法更改，请确认输入工号的是否正确！(Y/N)");
		scanf("%s",&temp);
		if(temp == 'N' || temp == 'n'){
			printf("请重新添加用户：");
			break;			
		}
		printf("请输入用户名：");
		scanf("%s",msg->info.name);
		getchar();	
		
		printf("请输入密码：");
		scanf("%s",msg->info.passwd);
		getchar();	

		printf("请输入年龄：");
		scanf("%d",&msg->info.age);
		getchar();			
		
		printf("请输入联系电话：");
		scanf("%s",msg->info.phone);
		getchar();	
		
		printf("请输入职位：");
		scanf("%s",msg->info.work);
		getchar();	
		
		printf("是否授予管理员权限:（Y/N）");
		scanf("%s",&temp);
		if(temp == 'Y' || temp == 'y'){
			msg->info.usertype = 0;
		}else{
			msg->info.usertype = 1;
		}	
		
		send(sockfd, msg, sizeof(MSG), 0);
		recv(sockfd, msg, sizeof(MSG), 0);		

		if(strncmp(msg->recvmsg, "OK", 2) == 0){
			printf("添加成功！\n");
		}else{
			printf("%s\n",msg->recvmsg);
		}
		
		printf("是否继续添加员工信息:（Y/N）");
		scanf("%s",&temp);
		if(temp == 'N' || temp == 'n'){
			return;	
		}			
	}
}


/**************************************
 *函数名：admin_deluser
 *参   数：消息结构体
 *功   能：管理员删除用户
 ****************************************/
void do_admin_deluser(int sockfd,MSG *msg)//管理员删除用户
{
	printf("------------%s-----------%d.\n",__func__,__LINE__);
	msg->msgtype  = ADMIN_DELUSER;
	memset(&msg->info,0,sizeof(staff_info_t));	
	
	printf("请输入要删除的用户工号：");
	scanf("%d",&msg->info.no);
	getchar();		
	
	printf("请输入要删除的用户名：");
	scanf("%s",msg->info.name);
	getchar();	
	
	send(sockfd, msg, sizeof(MSG), 0);
	recv(sockfd, msg, sizeof(MSG), 0);
	printf("%s\n",msg->recvmsg);
}



/**************************************
 *函数名：do_history
 *参   数：消息结构体
 *功   能：查看历史记录
 ****************************************/
void do_admin_history (int sockfd,MSG *msg)
{
	printf("------------%s-----------%d.\n",__func__,__LINE__);
	msg->msgtype  = ADMIN_HISTORY;
	memset(&msg->info,0,sizeof(staff_info_t));	
	
	send(sockfd, msg, sizeof(MSG), 0);

	while(1){
		recv(sockfd, msg, sizeof(MSG), 0);
		if(strncmp(msg->recvmsg,"over",4)==0)  //若服务器发了一个over，证明记录结束，跳出循环
			break;
		printf("msg->recvmsg: %s",msg->recvmsg);
	}
	printf("以上为历史记录查询结果...\n");	
}







/**************************************
 *函数名：user_menu
 *参   数：消息结构体
 *功   能：管理员菜单
 ****************************************/
void user_menu(int sockfd,MSG *msg)
{
	printf("------------%s-----------%d.\n",__func__,__LINE__);
	int n;
	while(1)
	{
		printf("*************************************************************\n");
		printf("*********   1：查询	   2：修改	  3：退出   *********\n");
		printf("*************************************************************\n");
		printf("请输入您的选择（数字）>>");
		scanf("%d",&n);
		getchar();

		switch(n)
		{
			case 1:
				do_user_query(sockfd,msg);
				break;
			case 2:
				do_user_modification(sockfd,msg);
				break;
			case 3:
				msg->msgtype = QUIT;
				send(sockfd, msg, sizeof(MSG), 0);
				close(sockfd);
				exit(0);
			default:
				printf("您输入有误，请输入数字\n");
				break;
		}
	}
}



/**************************************
 *函数名：do_query
 *参   数：消息结构体
 *功   能：登陆
 ****************************************/
void do_user_query(int sockfd,MSG *msg)
{
	printf("------------%s-----------%d.\n",__func__,__LINE__);
	int n;
	msg->msgtype = USER_QUERY;
	memset(&msg->info,0,sizeof(staff_info_t));
	
	printf("正在连接请稍后...\n");
	send(sockfd, msg, sizeof(MSG), 0);
	recv(sockfd, msg, sizeof(MSG), 0);
	printf("工号\t用户类型\t姓名\t密码\t年龄\t电话\t职位\n");
	show_userinfo(msg);
	printf("查询结束...\n");
}

	


/**************************************
 *函数名：do_modification
 *参   数：消息结构体
 *功   能：修改
 ****************************************/
void do_user_modification(int sockfd,MSG *msg)
{
	printf("------------%s-----------%d.\n",__func__,__LINE__);
	int n;
	msg->msgtype = USER_MODIFY;

	printf("请确认本人工号：");
	scanf("%d",&msg->info.no);
	getchar();

	printf("***********请输入要修改的选项(其他信息亲请联系管理员)*********\n");
	printf("**************	  1：电话   2：密码  3：退出   ***************\n");
	printf("**************************************************************\n");
	printf("请输入您的选择（数字）>>");
	scanf("%d",&n);
	getchar();
	
	switch(n)
	{
		case 1:
			printf("请输入新电话号码：");
			msg->recvmsg[0] = 'P';
			scanf("%s",msg->info.phone);
			break;
		case 2:
			printf("请输入新密码：");
			msg->recvmsg[0] = 'M';
			scanf("%s",msg->info.passwd);
			break;
		case 3:
			break;
		default:
			printf("您的输入有误，请重新输入.\n");
			break;			
	}
	send(sockfd, msg, sizeof(MSG), 0);
	recv(sockfd, msg, sizeof(MSG), 0);
	if(strncmp(msg->recvmsg,"OK",2)==0){
		printf("信息修改成功！\n");
	}else{
		printf("修改失败：%s\n",msg->recvmsg);
		printf("若本人工号与系统备案不符，请联系管理员修改.\n");
	}
}

















