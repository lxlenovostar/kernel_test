#include <sys/socket.h>
#include <linux/netlink.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

#define MAX_PAYLOAD 128  /* maximum payload size*/
#define NETLINK_USER 31


int init_sock(void); 
void free_send_msg(void); 
int set_send_msg(void); 
void free_rece_msg(void); 
int set_rece_msg(void); 
void send_to_kernel(void);
int check_from_kernel(void); 
int check_netlink_status(void); 
void free_resource(void); 
int rece_from_kernel(void);
void debug_info(void);
