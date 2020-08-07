#include "app.h"


//./udp 昵称 性别
int main(int argc, const char *argv[])
{
	if(argc != 3)
	{
		printf("输入格式错误，如　./xxx 昵称 性别\n");
		return -1;
	}
	struct glob_info ginfo;
	struct recv_info msg_info;
	pthread_t tid;
	
	init_udp(&ginfo, &msg_info, argv[1], argv[2]);

	pthread_create(&tid, NULL, recv_broadcast_msg, &ginfo);
	
	strcpy(msg_info.name, argv[1]);
	strcpy(msg_info.sex, argv[2]);
	msg_info.msg_flag = online_flag;							//使能上线标志
	
	broadcast_msg_data(ginfo.skt_fd, &msg_info, msg_info.msg_buffer - msg_info.sex);	//广播

	menu(&ginfo, &msg_info);								//用户操作文件							

	close(ginfo.skt_fd);

	return 0;
}
