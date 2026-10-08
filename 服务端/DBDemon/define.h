#ifndef _DEFINE_H
#define	_DEFINE_H
#include "../Library/Shared/ServerTopologyLimits.h"

#define DEFAULT_HEADER_SIZE			2
#define DEFAULT_MESSAGE_LENGTH		3
#define MAX_IP_LENGTH				32
#define DEFAULT_QUEUE_NUM			128
#define MAX_SERVER_BUFFER_SIZE		512000
#define MAX_USER_BUFFER_SIZE		128000
#define MM_MAX_PACKET_SIZE			51200	// 001211 KHS network.h俊 define登绢 乐绰 MAX_PACKET_SIZE客 何碟腮促. 
#define MAX_USER_NUM				800
#define MAX_SERVER_NUM DRAGON_SERVER_CAPACITY
#define MAX_CHAT_MSG_LENGHT			255
#define DEFAULT_TRANSFER_RECV_SIZE   8192
#define DEFAULT_TRANSFER_SEND_SIZE   8192
#define MAX_UDP_BUFFER_SIZE			1024
#define MIN_PROCESS_TIME			30
#define MAX_MSGQUE_SIZE				5120000


#endif

