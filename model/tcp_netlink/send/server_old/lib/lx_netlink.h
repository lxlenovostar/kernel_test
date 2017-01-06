#include <sys/socket.h>
#include <linux/netlink.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

#define MAX_PAYLOAD 128  /* maximum payload size*/
#define PING_PONG_TYPE (0x10 + 3)  
#define MD5_TYPE       (0x10 + 4)  

int init_sock(void); 
int check_netlink_status(void); 
void free_resource(void); 
int rece_from_kernel(void);
void debug_info(void);
int set_send_msg(void); 
int set_rece_msg(void); 
int build_md5_msg(char *msg); 
void send_to_kernel(void); 
