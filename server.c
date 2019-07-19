/*************************************************************************
	#	 FileName	: server.c
	#	 Author     : Rosin
	#	 Email      : Rosin_ls@163.com
	#	 Created	: Wed 17 Jul 2019 10:08:18 AM CST
 ************************************************************************/

#include "common.h"

sqlite3 *db;


int main(int argc, const char *argv[])
{
	int sockfd,acceptfd;
	struct sockaddr_in serveraddr, clientaddr;
	socklen_t serlen = sizeof(serveraddr);
	socklen_t clilen = sizeof(clientaddr);
	ssize_t recvbytes;
	char *errmsg;
	MSG msg;

	if(argc!=3){
		printf("User:%s <IP> <port>\n",argv[0]);
		return -1;
	}

	/*创建并初始化数据库：userinfo,historyinfo */
	if(sqlite3_open(STAFF_DATA,&db)!=SQLITE_OK){     //sqlite3_open成功返回SQLITE_OK，失败返回-1
		printf("%s\n",sqlite3_errmsg(db));
		return -1;
	}else{
		printf("the database open success.\n");
	}

	if(sqlite3_exec(db,"create table userinfo(no int,usertype int,name char,passwd char,age int,phone char,work char);",NULL,NULL,&errmsg)!=SQLITE_OK){
		printf("%s.\n",errmsg);
	}else{
		printf("create userinfo table success.\n");
			if(sqlite3_exec(db,"insert into userinfo values(1001,0,'admim','12345',30,'18379027381','系统维护');",NULL,NULL,&errmsg)!=SQLITE_OK){
				printf("添加管理员信息失败! %s.\n",errmsg);
			}
			if(sqlite3_exec(db,"insert into userinfo values(1002,0,'rosin','12345',18,'18379021234','工程师');",NULL,NULL,&errmsg)!=SQLITE_OK){
				printf("添加管理员信息失败! %s.\n",errmsg);
			}
	}

	if(sqlite3_exec(db,"create table historyinfo(time char,name char,words char);",NULL,NULL,&errmsg)!=SQLITE_OK){
		printf("%s.\n",errmsg);
	}else{
		printf("create historyinfo table success.\n");
	}


	/*创建套接字*/
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd == -1){
		perror("socket failed");
		exit(-1);
	}
	printf("sockfd: %d.\n",sockfd);

	int b_reuse = 1;
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &b_reuse, sizeof(int));

	/*填充网络结构体*/
	bzero(&serveraddr,serlen);
	bzero(&clientaddr,clilen);
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(atoi(argv[2]));  //端口
	serveraddr.sin_addr.s_addr = inet_addr(argv[1]);   //IP

	/*绑定*/
	if(bind(sockfd,(struct sockaddr *)&serveraddr,serlen)<0){
		perror("bind");
		exit(1);
	}

	/*调用listen()把主动套接字转换为被动套接字*/
	if(listen(sockfd,10)<0){
		perror("listen");
		exit(-1);
	}

#if 0
	fd_set rfds,tempfs;
	FD_ZERO(&rfds);     //关注列表清零
	FD_ZERO(&tempfs);     
	FD_SET(sockfd,&rfds);     //添加要监听的文件描述符
	int i;
	int readycount;
	int maxfd = sockfd;

	while(1){
		tempfs = rfds;   //记得每次重新添加, tempfs--准备好的， rfds--关注的
		readycount = select(maxfd+1, &tempfs, NULL, NULL, NULL);    //等待事件到来
		if(readycount<0){        
			printf("failed to select:");
			return -1;
		}else if(readycount == 0){       //超时
			printf("time out\n"); 
			continue;
		}

		for(i=0;i<maxfd+1;i++){    
			if(FD_ISSET(i,&tempfs)){     // 检查监听的文件描述符是否有事件到来
				if(i == sockfd){       //响应客户端链接请求
					if(acceptfd = accept(i,(struct sockaddr *)&clientaddr,&clilen) == -1){
						perror("accept failed!");
						exit(-1);
					}
					printf("new client: ip %s\n",inet_ntoa(clientaddr.sin_addr));
					FD_SET(acceptfd,&rfds);     //添加关注列表
					if(acceptfd > maxfd-1){     
						maxfd = acceptfd+1;
					}          
				}else{        //已经建立连接的文件描述符
					recvbytes = recv(i,&msg,sizeof(msg),0);
					printf("msg.type :%#x.\n",msg.msgtype);
					if(recvbytes == -1){
						printf("recvmsg failed!");
						continue;
					}else if(recvbytes == 0){
						printf("peer shutdown.\n");
						close(i);
						FD_CLR(i, &rfds);  //删除集合中的i
					}else{
						process_client_request(i,&msg);
					}
				}
			}    
		}
	}	
#endif


#if 1
	//定义一张表
	fd_set readfds,tempfds;
	//清空表
	FD_ZERO(&readfds);
	FD_ZERO(&tempfds);
	//添加要监听的事件
	FD_SET(sockfd,&readfds);
	int nfds = sockfd;   //nfds: fd最大数量
	int retval;
	int i = 0;


	while(1){
		tempfds = readfds;      //readfds是关注的集合;   tempfds是准备好的集合
		//记得重新添加监听
		retval =select(nfds + 1, &tempfds, NULL,NULL,NULL);
		//判断是否是集合里关注的事件
		for(i = 0;i < nfds + 1; i ++){
			if(FD_ISSET(i,&tempfds)){     //检查监听的文件描述符是否有事件到来
				if(i == sockfd){
					//数据交互 ：响应客户端连接请求...
					acceptfd = accept(sockfd,(struct sockaddr *)&clientaddr,&clilen);
					if(acceptfd == -1){
						printf("acceptfd failed.\n");
						exit(-1);
					}
					printf("ip : %s.\n",inet_ntoa(clientaddr.sin_addr));
					FD_SET(acceptfd,&readfds);     //添加新fd到关注列表
					nfds = nfds > acceptfd ? nfds : acceptfd;    //新连接可能改变最大文件描述符值, 保证nfds永远是最大的
				}else{      //已经连接过的fd，进行读写操作
					recvbytes = recv(i,&msg,sizeof(msg),0);
					printf("msg.type :%#x.\n",msg.msgtype);
					if(recvbytes == -1){
						printf("recv failed.\n");
						continue;
					}else if(recvbytes == 0){     
						printf("peer shutdown.\n");
						close(i);
						FD_CLR(i, &readfds);  //删除集合中的i
					}else{
						process_client_request(i,&msg);
					}
				}
			}
		}
	}
#endif
	close(sockfd);
	return 0;
}

/************************************************
 *函数名：process_client_request
 *参  数：消息结构体
 *功  能：客户端请求处理（任务分派）
 *************************************************/
int process_client_request(int acceptfd,MSG *msg)
{
	printf("------------%s-----------%d.\n",__func__,__LINE__);
	switch (msg->msgtype)
	{
		case USER_LOGIN:
		case ADMIN_LOGIN:
			process_user_or_admin_login_request(acceptfd,msg);
			break;
		case USER_MODIFY:
			process_user_modify_request(acceptfd,msg);
			break;
		case USER_QUERY:
			process_user_query_request(acceptfd,msg);
			break;
		case ADMIN_MODIFY:
			process_admin_modify_request(acceptfd,msg);
			break;
		case ADMIN_ADDUSER:
			process_admin_adduser_request(acceptfd,msg);
			break;
		case ADMIN_DELUSER:
			process_admin_deluser_request(acceptfd,msg);
			break;
		case ADMIN_QUERY:
			process_admin_query_request(acceptfd,msg);
			break;
		case ADMIN_HISTORY:
			process_admin_history_request(acceptfd,msg);
			break;
		case QUIT:
			process_client_quit_request(acceptfd,msg);
			break;
		default:
			break;
	}
}



/************************************************
 *函数名：process_user_or_admin_login_request
 *参  数：消息结构体
 *功  能：管理员/用户登录
 *************************************************/
int process_user_or_admin_login_request(int acceptfd,MSG *msg)
{
	printf("------------%s-----------%d.\n",__func__,__LINE__);
	//封装sql命令，表中查询用户名和密码－存在－登录成功－发送响应－失败－发送失败响应	
	char sql[DATALEN] = {0};
	char *errmsg;
	char **result;
	int nrow,ncolumn;

	msg->info.usertype = msg->usertype;
	strcpy(msg->info.name,msg->username);
	strcpy(msg->info.passwd,msg->passwd);
	
	printf("usrtype: %#x-----username: %s---passwd: %s.\n",msg->info.usertype,msg->info.name,msg->info.passwd);
	sprintf(sql,"select * from userinfo where usertype=%d and name='%s' and passwd='%s';",msg->info.usertype,msg->info.name,msg->info.passwd);
	if(sqlite3_get_table(db,sql,&result,&nrow,&ncolumn,&errmsg) != SQLITE_OK){      //在数据库中匹配用户名、密码，响应客户端
		printf("---****----%s.\n",errmsg);		
	}else{
		if(nrow == 0){
			strcpy(msg->recvmsg,"name or passwd failed.\n");
			send(acceptfd,msg,sizeof(MSG),0);
		}else{
			strcpy(msg->recvmsg,"OK");
			send(acceptfd,msg,sizeof(MSG),0);
		}
	}
	printf("当前用户：%s\n",msg->username);
	return 0;	
}


/************************************************
 *函数名：process_admin_query_request
 *参  数：消息结构体
 *功  能：管理员查询（按名字/查询全部）
 *************************************************/
int process_admin_query_request(int acceptfd,MSG *msg)
{
	printf("------------%s-----------%d.\n",__func__,__LINE__);
	
	int i = 0;
	char sql[DATALEN] = {0};
	char **resultp;
	int nrow,ncolumn;
	char *errmsg;
	
	/*按不同选项设置查询语句，殊途同归*/
	if(msg->flags == 1){
		sprintf(sql,"select * from userinfo where name='%s';",msg->info.name);
	}else{
		sprintf(sql,"select * from userinfo;");
	}
	
	if(sqlite3_get_table(db,sql,&resultp,&nrow,&ncolumn,&errmsg) != SQLITE_OK){
		printf("select failed %s.\n",errmsg);		
	}else{
		printf("searching.....\n");    
		printf("符合条目: %d.\n",nrow);  //nrow--行		
		puts("================================================================");
		if(nrow == 0){
			sprintf(msg->recvmsg,"未找到员工信息");
			send(acceptfd,msg,sizeof(MSG),0);
		}else{
			for(i=1;i<=nrow;i++){		//找到符合条件的条目，逐行获取员工信息发送客户端
				sprintf(msg->recvmsg,"%s\t%s\t%s\t%s\t%s\t%s\t%s",resultp[i*ncolumn],resultp[i*ncolumn+1],resultp[i*ncolumn+2],resultp[i*ncolumn+3],resultp[i*ncolumn+4],resultp[i*ncolumn+5],resultp[i*ncolumn+6]);
				send(acceptfd,msg,sizeof(MSG),0);
				printf(msg->recvmsg);
				putchar(10);	
			}
			if(msg->flags !=1 ){
				sprintf(msg->recvmsg,"over");
				send(acceptfd,msg,sizeof(MSG),0);
			}
		}
		sqlite3_free_table(resultp);    //释放
	}
	printf("当前用户：%s\n",msg->username);
	return 0;
}



 /************************************************
 *函数名：process_admin_modify_request
 *参  数：消息结构体
 *功  能：管理员修改员工信息
 *************************************************/
int process_admin_modify_request(int acceptfd,MSG *msg)
{
	printf("------------%s-----------%d.\n",__func__,__LINE__);
	
	int i = 0;
	char sql[DATALEN] = {0};
	char **resultp;
	int nrow,ncolumn;
	char *errmsg;
	
	/*根据不同选项设置修改语句*/
	switch(msg->recvmsg[0])
	{
		case 'N':
			sprintf(sql,"update userinfo set name='%s' where no=%d;",msg->info.name,msg->info.no);
			sprintf(msg->recvmsg,"修改工号为%d的员工信息：用户名改为%s",msg->info.no,msg->info.name);
			history_init(msg);
			break;
		case 'A':
			sprintf(sql,"update userinfo set age=%d where no=%d;",msg->info.age,msg->info.no);
			sprintf(msg->recvmsg,"修改工号为%d的员工信息：年龄改为%d",msg->info.no,msg->info.age);
			history_init(msg);
			break;			
		case 'P':
			sprintf(sql,"update userinfo set phone=%d where no=%d;",msg->info.phone,msg->info.no);
			sprintf(msg->recvmsg,"修改工号为%d的员工信息：联系电话改为%s",msg->info.no,msg->info.phone);
			history_init(msg);
			break;	
		case 'W':
			sprintf(sql,"update userinfo set work=%d where no=%d;",msg->info.work,msg->info.no);
			sprintf(msg->recvmsg,"修改工号为%d的员工信息：工作职位改为%s",msg->info.no,msg->info.work);
			history_init(msg);
			break;	
		case 'M':
			sprintf(sql,"update userinfo set passwd=%d where no=%d;",msg->info.passwd,msg->info.no);
			sprintf(msg->recvmsg,"修改工号为%d的员工信息：密码改为%s",msg->info.no,msg->info.passwd);
			history_init(msg);
			break;				
	}
	
	if(sqlite3_exec(db,sql,NULL,NULL,&errmsg) != SQLITE_OK){
		sprintf(msg->recvmsg,"信息修改失败！%s", errmsg);
		printf(msg->recvmsg);
	}else{
		sprintf(msg->recvmsg, "信息修改成功!");
		printf(msg->recvmsg);
	}
	send(acceptfd,msg,sizeof(MSG),0);
	
	printf("当前用户：%s\n",msg->username);
	return 0;
}



/************************************************
 *函数名：process_admin_adduser_request
 *参  数：消息结构体
 *功  能：管理员新增员工信息
 *************************************************/
int process_admin_adduser_request(int acceptfd,MSG *msg)
{
	printf("------------%s-----------%d.\n",__func__,__LINE__);
	
	int i = 0;
	char sql[DATALEN] = {0};
	char **resultp;
	int nrow,ncolumn;
	char *errmsg;
	
	/*新增*/
	sprintf(sql,"insert into userinfo values(%d,%d,'%s','%s',%d,'%s','%s');",msg->info.no,msg->info.usertype,msg->info.name,msg->info.passwd,msg->info.age,msg->info.phone,msg->info.work);
	if(sqlite3_get_table(db,sql,&resultp,&nrow,&ncolumn,&errmsg) != SQLITE_OK){
		sprintf(msg->recvmsg,"添加信息失败！ %s",errmsg);
		send(acceptfd,msg,sizeof(MSG),0);
		printf(msg->recvmsg);
	}else{
		/*返回结果给客户端*/
		strcpy(msg->recvmsg,"OK");
		send(acceptfd,msg,sizeof(MSG),0);
		
		/*服务器内回显员工信息*/
		sprintf(sql,"select * from userinfo;");
		if(sqlite3_get_table(db,sql,&resultp,&nrow,&ncolumn,&errmsg) != SQLITE_OK){
			printf("select failed %s.\n",errmsg);		
		}else{
			if(nrow != 0){
				for(i=0;i< nrow;i++){
					printf("%s\t%s\t%s\t%s\t%s\t%s\t%s\n",resultp[i*ncolumn],resultp[i*ncolumn+1],resultp[i*ncolumn+2],resultp[i*ncolumn+3],resultp[i*ncolumn+4],resultp[i*ncolumn+5],resultp[i*ncolumn+6]);	
					usleep(1000);
				}	
			}
		}
		sqlite3_free_table(resultp);
		sprintf(msg->recvmsg,"新增工号为:%d 姓名为:%s 的员工信息",msg->info.no,msg->info.name);
		history_init(msg); 
	}
	printf("当前用户：%s\n",msg->username);	
}



/************************************************
 *函数名：process_admin_deluser_request
 *参  数：消息结构体
 *功  能：管理员删除员工信息
 *************************************************/
int process_admin_deluser_request(int acceptfd,MSG *msg)
{
	printf("------------%s-----------%d.\n",__func__,__LINE__);
	
	int i = 0;
	char sql[DATALEN] = {0};
	char **resultp;
	int nrow,ncolumn;
	char *errmsg;

	sprintf(sql,"delete from userinfo where no=%d and name='%s';",msg->info.no,msg->info.name);
	if(sqlite3_get_table(db,sql,&resultp,&nrow,&ncolumn,&errmsg) != SQLITE_OK){
		sprintf(msg->recvmsg,"删除信息失败！ %s",errmsg);
		send(acceptfd,msg,sizeof(MSG),0);
		printf(msg->recvmsg);
	}else{
		/*返回结果给客户端*/
		strcpy(msg->recvmsg,"删除成功！");
		send(acceptfd,msg,sizeof(MSG),0);
		
		/*服务器内回显员工信息*/
		printf("管理员%s 删除员工 工号:%d, 用户名:%s 信息\n",msg->username,msg->info.no,msg->info.name);
		sprintf(msg->recvmsg,"删除员工 工号:%d, 用户名:%s 信息",msg->info.no,msg->info.name);
		history_init(msg);
	}
	printf("当前用户：%s\n",msg->username);	
	return 0;
}



/************************************************
 *函数名：process_admin_history_request
 *参  数：消息结构体
 *功  能：查询历史记录--管理员
 *************************************************/
int process_admin_history_request(int acceptfd,MSG *msg)
{
	printf("------------%s-----------%d.\n",__func__,__LINE__);
	int i;
	char sql[DATALEN] = {0};
	char **resultp;
	int nrow,ncolumn;
	char *errmsg;
	
	sprintf(sql,"select * from historyinfo;");
	if(sqlite3_get_table(db,sql,&resultp,&nrow,&ncolumn,&errmsg) != SQLITE_OK){
		sprintf(msg->recvmsg,"查询历史信息失败！ %s",errmsg);
		send(acceptfd,msg,sizeof(MSG),0);
		printf(msg->recvmsg);
	}else{
		sprintf(msg->recvmsg,"OK!\n");
		send(acceptfd,msg,sizeof(MSG),0);	
		
		sprintf(msg->recvmsg,"查询到 %d 条历史记录.\n",nrow);  //nrow--行
		printf(msg->recvmsg);	
		send(acceptfd,msg,sizeof(MSG),0);		
		puts("=============================================================");
		for(i=1;i<=nrow;i++){		//找到符合条件的条目，逐行获取员工信息发送客户端
			sprintf(msg->recvmsg,"%s：%s %s\n",resultp[i*ncolumn],resultp[i*ncolumn+1],resultp[i*ncolumn+2]);
			printf(msg->recvmsg);
			send(acceptfd,msg,sizeof(MSG),0);	
		}
		sprintf(msg->recvmsg,"over");
		send(acceptfd,msg,sizeof(MSG),0);
	}
	sqlite3_free_table(resultp);    //释放
	printf("当前用户：%s\n",msg->username);
	return 0;
}




/************************************************
 *函数名：process_user_query_request
 *参  数：消息结构体
 *功  能：普通用户查询本人信息
 *************************************************/
int process_user_query_request(int acceptfd,MSG *msg)
{
	printf("------------%s-----------%d.\n",__func__,__LINE__);
	
	int i = 0;
	char sql[DATALEN] = {0};
	char **resultp;
	int nrow,ncolumn;
	char *errmsg;
	
	sprintf(sql,"select * from userinfo where name='%s';",msg->username);
	if(sqlite3_get_table(db,sql,&resultp,&nrow,&ncolumn,&errmsg) != SQLITE_OK){
		sprintf(msg->recvmsg,"select failed %s.\n",errmsg);		
	}else{
		printf("searching.....\n");    
		printf("符合条目: %d.\n",nrow);  //nrow--行		
		puts("=============================================================");
		sprintf(msg->recvmsg,"%s\t%s\t%s\t%s\t%s\t%s\t%s",resultp[ncolumn],resultp[ncolumn+1],resultp[ncolumn+2],resultp[ncolumn+3],resultp[ncolumn+4],resultp[ncolumn+5],resultp[ncolumn+6]);
	}
	printf(msg->recvmsg);
	send(acceptfd,msg,sizeof(MSG),0);
	sqlite3_free_table(resultp);    //释放
	
	printf("当前用户：%s\n",msg->username);
	return 0;		
}

	
	

/************************************************
 *函数名：process_user_modify_request
 *参  数：消息结构体
 *功  能：普通用户修改本人信息
 *************************************************/
int process_user_modify_request(int acceptfd,MSG *msg)
{
	printf("------------%s-----------%d.\n",__func__,__LINE__);
	
	int i = 0;
	char sql[DATALEN] = {0};
	char **resultp;
	int nrow,ncolumn;
	char *errmsg;
	
	/*根据不同选项设置修改语句*/
	switch(msg->recvmsg[0])
	{		
		case 'P':
			sprintf(sql,"update userinfo set phone='%s' where no=%d and name='%s';",msg->info.phone,msg->info.no,msg->username);
			//sprintf(msg->recvmsg,"修改其联系电话为%s",msg->info.phone);
			//history_init(msg);
			break;		
		case 'M':
			sprintf(sql,"update userinfo set passwd='%s' where no=%d and name='%s';",msg->info.passwd,msg->info.no,msg->username);
			//sprintf(msg->recvmsg,"修改其密码为%s",msg->info.passwd);
			//history_init(msg);
			break;				
	}
	
	if(sqlite3_exec(db,sql,NULL,NULL,&errmsg) != SQLITE_OK){
		sprintf(msg->recvmsg,"failed.%s", errmsg);
		printf(msg->recvmsg);
	}else{
		sprintf(msg->recvmsg, "OK");
		printf(msg->recvmsg);
	}
	send(acceptfd,msg,sizeof(MSG),0);
	
	printf("\n当前用户：%s\n",msg->username);
	return 0;	
}





/************************************************
 *函数名：get_system_time
 *参  数：消息结构体
 *功  能：获取当前时间
 *************************************************/
void get_system_time(char* data)
{
	time_t mytime;
	struct tm *mytm;
	mytime=time(NULL);//得到秒数
	mytm=localtime(&mytime);//得到当前的时间
	sprintf(data,"%04d-%02d-%02d  %02d:%02d:%02d",mytm->tm_year+1900,mytm->tm_mon+1,mytm->tm_mday,\
	mytm->tm_hour,mytm->tm_min,mytm->tm_sec);
}


/************************************************
 *函数名：history_init
 *参  数：消息结构体
 *功  能：保存历史记录
 *************************************************/
void history_init(MSG *msg)
{
		char *errmsg;
		char data[64]={0};
		char sql[DATALEN] = {0};
		
		get_system_time(data);//获得当前的时间
		sprintf(sql,"insert into historyinfo values('%s','%s','%s')",data,msg->username,msg->recvmsg);
		if(sqlite3_exec(db,sql,NULL,NULL,&errmsg)!=SQLITE_OK){
			printf("%s\n",errmsg);
			return;
		}		
}



/************************************************
 *函数名：process_client_quit_request
 *参  数：消息结构体
 *功  能：用户下线
 *************************************************/
int process_client_quit_request(int acceptfd,MSG *msg)
{
	printf("用户%s已下线\n",msg->username);
}

