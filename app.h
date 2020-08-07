#ifndef _APP_H_
#define _APP_H__
#include "info.h"


// 程序刚运行时申请资源的函数
int init_udp(struct glob_info *ginfo, struct recv_info *msg_info, const char *arg_1, const char *arg_2);

// 用户操作
void menu(struct glob_info *ginfo, struct recv_info *msg_info);

// 1
int show_friends_and_talk(struct glob_info *ginfo);
// 2
int show_friends_list(struct glob_info *ginfo);

// 0 
int exit_and_broadcast(struct glob_info *ginfo, struct recv_info *msg_info);

// 接受好友消息的线程
void *recv_broadcast_msg(void *arg);

// 接受文件的线程
#define BUF_SIZES (8192)
void *file_client(void *arg);

//建立TCP客户端发送文件的函数 : 被 privateTalk 调用
#define BUF_SIZE  (8192)
int file_server(const char *path, struct glob_info *ginfo, struct friend_list *pos);


// 建立私聊空间，用于发送文字 或 文件传输 ： 被 show_friends_and_talk调用
int privateTalk(struct glob_info *ginfo,char *talk_name);

//自动检索本地网卡的所有信息，发送广播信息到所有网卡（除本地回环网卡）
int broadcast_msg_data(int skt_fd, const void *msg, ssize_t msg_len);
#endif // _APP_H__
