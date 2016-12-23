//#include <sys/socket.h>
//#include <linux/netlink.h>

#define MAX_PAYLOAD 128  /* maximum payload size*/
#define NETLINK_USER 31


int init_sock(void); 
int check_netlink_status(void); 
void free_netlink_resource(void); 
