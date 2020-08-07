#include "app.h"


/*******************************************************
***函数名字：init_udp
***函数功能：程序刚运行时申请资源的函数
***参数：giao:用户结构体指针，msg_info:消息结构体指针
***返回值：返回值：正常０，错误－１
***********************************************************/
int init_udp(struct glob_info *ginfo, struct recv_info *msg_info, const char *arg_1, const char *arg_2)
{

	int udp_fd;
	int retval;
	
	ssize_t send_size;
	char broadcase_addr[16];
	struct sockaddr_in native_addr, dest_addr, recv_addr;
	socklen_t skt_len = sizeof(struct sockaddr_in);
	
	struct friend_list *pos;
	
	
	strcpy(ginfo->name, arg_1);
	strcpy(ginfo->sex,arg_2);
	ginfo->list_head = request_friend_info_node(NULL);


	ginfo->skt_fd = socket(AF_INET, SOCK_DGRAM, 0);
	if(ginfo->skt_fd == -1)
	{
		perror("申请套接字失败");
		return -1;
	}
	
	int sw = 1;
	retval = setsockopt(ginfo->skt_fd, SOL_SOCKET, SO_BROADCAST, &sw, sizeof(sw));
	if(retval == -1)
	{
		perror("设置程序允许广播出错");
		return -1;
	}

	native_addr.sin_family = AF_INET;
	native_addr.sin_port = htons(11408);
	native_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	retval = bind(ginfo->skt_fd, (struct sockaddr *)&native_addr, sizeof(native_addr));
	if(retval == -1)
	{
		perror("绑定异常");
		return -1;
	}

	

	return 0;
}
/*******************************************************
***函数名字：menu
***函数功能：用户操作界面
***参数：giao:用户结构体指针，msg_info:消息结构体指针
***返回值：无
***********************************************************/
void menu(struct glob_info *ginfo, struct recv_info *msg_info)
{
	int input_cmd;
	char talk_name[256];
	while(1)
	{
		printf("[1]选择好友进行私聊\n");
		printf("[2]查看当前好友列表\n");
		printf("[0]下线\n");
		scanf("%d", &input_cmd);

		switch(input_cmd){
			case 1:
				show_friends_and_talk(ginfo);
				break;

			case 2:
				show_friends_list(ginfo);
				break;

			case 0:
				if(exit_and_broadcast(ginfo, msg_info) == 0)
					exit(0);
				else
					continue; //广播函数有问题
				break;
		}

	}
}

/*******************************************************
***函数名字：show_friends_and_talk
***函数功能：用户操作界面的功能１
***参数：giao:用户结构体指针
***返回值：正常０，错误－１
***********************************************************/
int show_friends_and_talk(struct glob_info *ginfo)
{
	char talk_name[1024];
	bzero(talk_name, sizeof(talk_name));
	struct friend_list *pos;
	//打印好友链表，并且可以找寻一个好友聊天
	list_for_each(ginfo->list_head, pos)
	{
		printf("＊＊＊＊%s，在线＊＊＊＊\n", pos->name);
	}

	printf("请输入要私聊的名字:");
	scanf("%s",talk_name);

	privateTalk(ginfo,talk_name);

	return 0;
}

/*******************************************************
***函数名字：show_friends_list
***函数功能：用户操作界面的功能２
***参数：giao:用户结构体指针
***返回值：返回值：正常０，错误－１
***********************************************************/
int show_friends_list(struct glob_info *ginfo)
{
	struct friend_list *pos;
	//仅仅只是打印好友链表
	list_for_each(ginfo->list_head, pos)
	{
		printf("＊＊＊＊%s，在线＊＊＊＊\n", pos->name);
	}
	return 0;
}
/*******************************************************
***函数名字：exit_and_broadcast
***函数功能：用户操作界面的功能０
***参数：giao:用户结构体指针，msg_info:消息结构体指针
***返回值：返回值：正常０，错误－１
***********************************************************/
int exit_and_broadcast(struct glob_info *ginfo, struct recv_info *msg_info)
{
	msg_info->msg_flag = offline_flag;//下线标志	
	int retval = broadcast_msg_data(ginfo->skt_fd, msg_info, msg_info->msg_buffer-msg_info->name);//通知别人我们下线了
	if(retval == 0)
		return 0;
	else
		return -1;
	
}


/*******************************************************
***函数名字：recv_broadcast_msg
***函数功能：接受好友消息的线程
***参数：arg:结构体类型参数
***返回值：无
***********************************************************/
void *recv_broadcast_msg(void *arg)
{
	struct recv_info recv_msg, msg_info;
	struct glob_info *ginfo = arg;
	socklen_t skt_len = sizeof(struct sockaddr_in);
	pthread_t c_file_tid;
	//一直接受他人的广播信息，代表好友上线，更新好友链表

	ssize_t recv_size, send_size;

	struct friend_list *new_node;
	struct friend_list cache_node;
	struct friend_list *pos, *p;

	while(1)
	{
		bzero(&recv_msg, sizeof(recv_msg));

		recv_size = recvfrom(ginfo->skt_fd, &recv_msg, sizeof(recv_msg), 
			0, (struct sockaddr *)&(cache_node.addr), &skt_len);
		if(recv_size == -1)
		{
			perror("接受UDP数据失败\n");
			break;
		}

		//printf("你的%s给你发送消息：%s\n", recv_msg.name, recv_msg.msg_buffer);

		switch(recv_msg.msg_flag)
		{
			case online_flag:
				list_for_each(ginfo->list_head, pos)
				{
					if(strcmp(pos->name, recv_msg.name) == 0)
						break;
				}
				
				if(pos != NULL)
					break;
						
				//谁谁谁上线了，插入好友链表
				strcpy(cache_node.name, recv_msg.name);
				strcpy(cache_node.sex, recv_msg.sex);
				new_node = request_friend_info_node(&cache_node);
				insert_friend_info_node_to_link_list(ginfo->list_head, new_node);
				printf("[%s-%s]上线了\n", new_node->name,new_node->sex);
				
				strcpy(msg_info.name, ginfo->name);//将我们的名字赋值进去
				msg_info.msg_flag = online_flag;//上线标志
				
				send_size = sendto(ginfo->skt_fd, &msg_info, msg_info.msg_buffer-msg_info.name, 
					0, (struct sockaddr *)&cache_node.addr, sizeof(struct sockaddr_in));//回送对方我们的上线信息
				if(send_size == -1)
				{
					perror("回送失败!\n");
					return NULL;
				}
				break;
				
			case offline_flag:
				//谁谁谁下线了，删除这个好友链表
				if(ginfo->list_head->next == NULL)
					break;
				for(p=ginfo->list_head, pos=ginfo->list_head->next; pos!=NULL; pos=pos->next, p=p->next)
				{
					if(strcmp(pos->name, recv_msg.name) == 0)
					{
						p->next = pos->next;
						pos->next = NULL;
						printf("好友 %s下线了\n", recv_msg.name);
						free(pos);
						break;	
					}
				}
					
				if(pos == NULL) // 这个人不存在，但是却提醒了我他要下线了
					break;
				break;
				
			case msg_flag:
				//谁谁谁给你发送消息，将消息如何处理
				printf("你的%s给你发送消息：%s\n", recv_msg.name, recv_msg.msg_buffer);
				break;
			case file_flag:
				pthread_create(&c_file_tid, NULL, file_client, &cache_node);
				pthread_join(c_file_tid, NULL);
				break;
		}
		
	}
}

/*******************************************************
***函数名字：file_client
***函数功能：接受文件的线程
***参数：arg:好友链表
***返回值：无
***********************************************************/
void *file_client(void *arg)
{
	int skfd; //TCP套接字描述符
	struct friend_list *cache_node = arg;
	FILE *fp = NULL;
	unsigned int fileSize;
	///////
	struct fileinfo file_info;

	int size, nodeSize;
	unsigned char fileBuf1[BUF_SIZES];

	//创建tcp socket
	if((skfd=socket(AF_INET,SOCK_STREAM,0)) < 0) 
	{
		perror("socket");
		exit(1);
	} 
	else 
	{
		printf("socket success!\n");
	}

	

	cache_node->addr.sin_port = htons(6665);
	/* 客户端调用connect主动发起连接请求 */
	if(connect(skfd, (struct sockaddr *)&cache_node->addr, sizeof(struct sockaddr_in)) == -1) 
	{
		perror("ConnectError:");
		exit(1);
	}
 	else 
	{
		printf("connnect success!\n");
	}


	printf("\n---正在接收文件包信息---\n");
	// size = read(skfd, &file_info, 4);
	// if( size != 4 ) 
	// {
	// 	printf("file size error!\n");
	// 	close(skfd);
	// 	exit(-1);
	// }

	size = recv(skfd, &file_info, sizeof(file_info), 0);
	if(size == -1)
	{
		perror("接收文件信息包出错:");
		exit(1);
	}

	printf("file name: %s,file size:%d\n", file_info.name, file_info.size);
	
	
	if( (size = write(skfd, "OK", 2) ) < 0 ) 
	{
		perror("write");
		close(skfd);
		exit(1);
	}


	///	

	fp = fopen(file_info.name, "w");
	if( fp == NULL ) 
	{
		perror("fopen");
		close(skfd);
	}

	fileSize = 0;
	while(memset(fileBuf1, 0, sizeof(fileBuf1)), (size = read(skfd, fileBuf1, sizeof(fileBuf1))) > 0) 
	{
		unsigned int size2 = 0;
		while( size2 < size ) 
		{
		    if( (nodeSize = fwrite(fileBuf1 + size2, 1, size - size2, fp) ) < 0 ) 
			{
				perror("write");
				close(skfd);
				exit(1);
		    	}
		    size2 += nodeSize;
		}
		fileSize += size;
		if(fileSize >= fileSize)
		{
		       break;
		}
	}
	printf("文件已接受\n");
	fclose(fp);
	close(skfd);
}

 
/*******************************************************
***函数名字：file_server
***函数功能：建立TCP客户端发送文件的函数 : 被 privateTalk 调用
***参数：path:文件名，ginfo：信息结构体，pos:朋友链表指针
***返回值：返回值：正常０，错误－１
***********************************************************/

int file_server(const char *path, struct glob_info *ginfo, struct friend_list *pos)
{
	int skfd, cnfd;
	FILE *fp = NULL;
	struct sockaddr_in sockAddr, cltAddr;
	socklen_t addrLen;
	
	///////////
	//unsigned int fileSize;
	struct fileinfo file_info;
	int size, netSize;
	char buf[10];

	unsigned char fileBuf[BUF_SIZE];

	if( !path ) 
	{
		printf("file server: file path error!\n");
		return -1;
	}
	strcpy(file_info.name, path);

	//创建tcp socket
	if((skfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
	{
		perror("socket");
		exit(1);
	} 
	else
	{
		printf("socket success!\n");
	}

	int opt = SO_REUSEADDR;
    setsockopt(skfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	//创建结构  绑定地址端口号
	memset(&sockAddr, 0, sizeof(struct sockaddr_in));
	sockAddr.sin_family = AF_INET;
	sockAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	sockAddr.sin_port = htons(6665);

	//bind
	if(bind(skfd, (struct sockaddr *)(&sockAddr), sizeof(struct sockaddr)) < 0)
	{
		perror("Bind");
		exit(1);
	} 
	else 
	{
		printf("bind success!\n");
	}

	//listen   监听  最大1个用户
	if(listen(skfd, 1) < 0) 
	{
		perror("Listen");
		exit(1);
	} 
	else 
	{
		printf("listen success!\n");
	}

	//通知对方建立client端
	struct recv_info msg_info;
	msg_info.msg_flag = file_flag; //告诉好友我要给他发文件，对方可以接受到我的udp信息，同时也能拿到我的本机地址
	int retval = sendto(ginfo->skt_fd, &msg_info, sizeof(msg_info), 
			0, (struct sockaddr *)&pos->addr, sizeof(struct sockaddr_in));
	if(retval == -1)
	{
		printf("通知对方建立client端失败\n");
		exit(1);
	}

	printf("\n----通知对方建立client端成功-----\n");
	/* 调用accept,服务器端一直阻塞，直到客户程序与其建立连接成功为止*/
	addrLen = sizeof(struct sockaddr_in);
	if((cnfd = accept(skfd, (struct sockaddr *)(&cltAddr), &addrLen)) < 0)
	{
		perror("Accept");
		exit(1);
	} 
	else 
	{
		printf("accept success!\n");
	}

	fp = fopen(path, "r");
	if( fp == NULL ) 
	{
		perror("fopen");
		close(cnfd);
		close(skfd);
		exit(1);
	}

	fseek(fp, 0, SEEK_END);
	
	file_info.size = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	////////
	// if(write(cnfd, &file_info, 4) != 4)
	// {
	// 	perror("write");
	// 	close(cnfd);
	// 	close(skfd);
	// 	exit(1);
	// }
	if(send(cnfd, &file_info, sizeof(file_info), 0) == -1)
	{
		perror("发送文件信息包出错:");
		exit(1);
	}


	if( read(cnfd, buf, 2) != 2) 
	{
		perror("read");
		close(cnfd);
		close(skfd);
		exit(1);
	}
	

	while( ( size = fread(fileBuf, 1, BUF_SIZE, fp) ) > 0 ) 
	{
		unsigned int size2 = 0;
		while( size2 < size )
		{
		    if( (netSize = write(cnfd, fileBuf + size2, size - size2) ) < 0 ) 
		    {
			perror("write");
			close(cnfd);
			close(skfd);
			exit(1);
		    }
		    size2 += netSize;
		}
	}
	printf("文件已发送完毕\n");

	fclose(fp);	
	close(cnfd);
	close(skfd);

	return 0;
}

/*******************************************************
***函数名字：privateTalk
***函数功能：建立私聊空间，用于发送文字 或 文件传输 ： 被 show_friends_and_talk调用
***参数：ginfo:信息结构,talk_name:聊天朋友的名字
***返回值：返回值：正常０，错误－１
***********************************************************/
int privateTalk(struct glob_info *ginfo,char *talk_name)
{
	ssize_t send_size;
	struct friend_list *pos;
	struct recv_info recv_msg, msg_info;
	socklen_t skt_len = sizeof(struct sockaddr_in);
	
	list_for_each(ginfo->list_head, pos)
	{
		if(strcmp(pos->name, talk_name) == 0)
		{
			printf("与好友%s建立私聊!\n",pos->name);
			break;
		}
	}
	if(pos == NULL)
	{
		printf("未查找此人!\n");
		return -1;
	}

	int i;
	char path[256];
	printf("聊天请按１，发送文件请按２,退出输入exit：");
	scanf("%d",&i);
	switch(i)
	{
		case 1:
			while(1)
			{
				bzero(&msg_info, sizeof(msg_info));
				scanf("%s",msg_info.msg_buffer);

				if(strcmp(msg_info.msg_buffer,"exit") == 0)
					return 0;

				msg_info.msg_flag = msg_flag;
				send_size = sendto(ginfo->skt_fd, &msg_info, sizeof(msg_info), 
							0, (struct sockaddr *)&pos->addr, sizeof(struct sockaddr_in));
				if(send_size == -1)
				{
					perror("私聊数据发送失败!\n");
					return -1;
				}

				
			}
			
		case 2:
			printf("请输入文件名：");
			scanf("%s", path);
			file_server(path, ginfo, pos);
			break;
	}
	
}


/*******************************************************
***函数名字：broadcast_msg_data
***函数功能：自动检索本地网卡的所有信息，发送广播信息到所有网卡（除本地回环网卡）
***参数：skt_fd:套接字文件描述符, msg:存放消息，msg_len:消息的长度
***返回值：返回值：正常０，错误－１
***********************************************************/
int broadcast_msg_data(int skt_fd, const void *msg, ssize_t msg_len)
{
	int i;
	struct ifconf ifconf;
	struct ifreq *ifreq;
	struct sockaddr_in dest_addr;
	ssize_t send_size;
	char buf[512];//缓冲区
	//初始化ifconf
	ifconf.ifc_len =512;
	ifconf.ifc_buf = buf;
   
	ioctl(skt_fd, SIOCGIFCONF, &ifconf); //获取所有接口信息

	//接下来一个一个的获取IP地址
	ifreq = (struct ifreq*)ifconf.ifc_buf;

	//printf("获取到的所有网卡信息结构体长度:%d\n",ifconf.ifc_len);
	//printf("一个网卡信息结构体的差精度%ld\n", sizeof (struct ifreq));

	//循环分解每个网卡信息
	//i=(ifconf.ifc_len/sizeof (struct ifreq))等于获取到多少个网卡
	for (i=(ifconf.ifc_len/sizeof (struct ifreq)); i>0; i--, ifreq++)
	{
		if(ifreq->ifr_flags == AF_INET)//判断网卡信息是不是IPv4的配置
		{
			//printf("网卡名字叫 [%s]\n" , ifreq->ifr_name);
			//printf("网卡配置的IP地址为  [%s]\n" ,inet_ntoa(((struct sockaddr_in*)&(ifreq->ifr_addr))->sin_addr));

			if(strcmp(ifreq->ifr_name, "lo") == 0)//判断如果是本地回环网卡则不广播数据
				continue;

			ioctl(skt_fd, SIOCGIFBRDADDR, ifreq);//通过网卡名字获取广播地址

			//将网络地址转化为本机地址
			//printf("该网卡广播地址为 %s\n", inet_ntoa(((struct sockaddr_in *)&(ifreq->ifr_addr))->sin_addr));


			dest_addr.sin_family = AF_INET;
			dest_addr.sin_port = htons(11408);
			dest_addr.sin_addr.s_addr = ((struct sockaddr_in*)&(ifreq->ifr_addr))->sin_addr.s_addr;

			send_size = sendto(skt_fd, msg, msg_len, 
					0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
			if(send_size == -1)
			{
				perror("发送UDP数据失败\n");
				return -1;
			}
		}
        }
    	
	return 0;
}
