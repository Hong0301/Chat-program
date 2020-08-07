#ifndef _INFO_H_ 
#define _INFO_H_

#include <stdio.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <strings.h>
#include <unistd.h>
#include <sys/select.h>
/* According to earlier standards */
#include <sys/time.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <net/if_arp.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>
#include <sys/stat.h>
#include <fcntl.h>


//文件结构
struct fileinfo
{
	char name[50];
	int  size;
	int  flag;
};

enum {online_flag, offline_flag, msg_flag, file_flag};
//用户节点
struct friend_list{
	char name[256];
	char sex[16];
	struct sockaddr_in addr;
	struct friend_list *next;
};

//消息结构体
struct recv_info{
	char name[256];
	char sex[16];
	int msg_flag;
	char msg_buffer[4096];
};
//用户结构体
struct glob_info{
	char name[256];
	char sex[16];
	int skt_fd;	
	struct friend_list *list_head;
};

#define	list_for_each(head, pos)\
	for(pos=head->next; pos!=NULL; pos=pos->next)

//申请客户端信息链表的头节点
static struct friend_list *request_friend_info_node(const struct friend_list *info)
{
	struct friend_list *new_node;

	new_node = malloc(sizeof(struct friend_list));
	if(new_node == NULL)
	{
		perror("申请客户端节点异常");
		return NULL;
	}

	if(info != NULL)
		*new_node = *info;

	new_node->next = NULL;

	return 	new_node;
}

static inline void insert_friend_info_node_to_link_list(struct friend_list *head, struct friend_list *insert_node)
{
	struct friend_list *pos;

	for(pos=head; pos->next != NULL; pos=pos->next);

	pos->next = insert_node;
}



#endif //_INFO_H_
