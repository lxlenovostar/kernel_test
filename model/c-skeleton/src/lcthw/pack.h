typedef unsigned int bpf_u_int32;
typedef unsigned short u_short;
typedef int bpf_int32;
typedef struct pcap_file_header {
    bpf_u_int32 magic;
    u_short version_major;
    u_short version_minor;
    bpf_int32 thiszone;
    bpf_u_int32 sigfigs;
    bpf_u_int32 snaplen;
    bpf_u_int32 linktype;
} pcap_file_header;

typedef struct timestamp {
    bpf_u_int32 timestamp_s;
    bpf_u_int32 timestamp_ms;
} timestamp;

typedef struct pcap_header {
    timestamp ts; 
    bpf_u_int32 capture_len;
    bpf_u_int32 len;

} pcap_header;

#define MAX_ETH_FRAME 30000
#define INT_TIME 1
#define ETH_ALEN        6
#define __LITTLE_ENDIAN_BITFIELD 1
#define NIPQUAD(addr) \
        ((unsigned char *)&addr)[0], \
        ((unsigned char *)&addr)[1], \
        ((unsigned char *)&addr)[2], \
        ((unsigned char *)&addr)[3]

struct ethhdr {
    unsigned char h_dest[ETH_ALEN]; /* destination eth addr */
    unsigned char h_source[ETH_ALEN];   /* source ether addr    */
    unsigned short h_proto; /* packet type ID field */
};

struct iphdr {
#if defined(__LITTLE_ENDIAN_BITFIELD)
    __u8 ihl:4, version:4;
#elif defined (__BIG_ENDIAN_BITFIELD)
    __u8 version:4, ihl:4;
#endif
    __u8 tos;
    __be16 tot_len;
    __be16 id;
    __be16 frag_off;
    __u8 ttl;
    __u8 protocol;
    __u16 check;
    __be32 saddr;
    __be32 daddr;
    /*The options start here. */
};

struct tcphdr {
    __u16 source;
    __u16 dest;
    __u32 seq;
    __u32 ack_seq;
#if defined(__LITTLE_ENDIAN_BITFIELD)
    __u16 res1:4,
        doff:4, fin:1, syn:1, rst:1, psh:1, ack:1, urg:1, ece:1, cwr:1;
#elif defined(__BIG_ENDIAN_BITFIELD)
    __u16 doff:4,
        res1:4, cwr:1, ece:1, urg:1, ack:1, psh:1, rst:1, syn:1, fin:1;
#endif
    __u16 window;
    __u16 check;
    __u16 urg_ptr;
};

struct udphdr {
    __u16 source;
    __u16 dest;
    __u16 len;
    __u16 check;
};

struct icmphdr {
    __u8 type;
    __u8 code;
    __u16 checksum;
    union {
        struct {
            __u16 id;
            __u16 sequence;
        } echo;
        __u32 gateway;
        struct {
            __u16 __unused;
            __u16 mtu;
        } frag;
    } un;
};

/*
 * used for record parting point.
 */
#define PLEN 5000
typedef struct part_point {
	long index[PLEN];
	int end;
} part_point;

part_point *
part_create()
{
	part_point *tmp = (part_point*)malloc(sizeof(part_point));
	tmp->end = 0;
	return tmp;
}

void
part_destroy(part_point *point) 
{
	if (point)
		free(point);
}

void
part_clean(part_point *point) 
{
	point->end = 0;
}

void
part_set(part_point *point, long data)
{
	if (point->end >= PLEN) {
		log_err("error case happen in part_set.");
		exit(-1);
	}
	point->index[point->end] = data;
	++point->end;
} 

