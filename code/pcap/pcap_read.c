#include <stdio.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <linux/types.h>

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
#define INT_TIME 100
#define ETH_ALEN        6
#define __LITTLE_ENDIAN_BITFIELD 1
#define NIPQUAD(addr) \
        ((unsigned char *)&addr)[0], \
        ((unsigned char *)&addr)[1], \
        ((unsigned char *)&addr)[2], \
        ((unsigned char *)&addr)[3]

struct ethhdr {
	unsigned char h_dest[ETH_ALEN];	/* destination eth addr */
	unsigned char h_source[ETH_ALEN];	/* source ether addr    */
	unsigned short h_proto;	/* packet type ID field */
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

FILE *fp1;
FILE *fp2;
__u32 seq1, seq2, ip1, ip2;
long long start_stime;

void
printPcap(int count, void *data, struct pcap_header *ph)
{
	size_t size = ph->capture_len;
	struct ethhdr *eth;
	struct iphdr *iph;
	struct tcphdr *tcph;
	long long stime;
	char play_data[30000];
	int len;
	int i;

	memset(play_data, '\0', 30000);

	if (data == NULL) {
		return;
	}
	eth = (struct ethhdr *) (data);
	eth->h_proto = ntohs(eth->h_proto);
	if (eth->h_proto == 0x0800) {	// IP
		iph = (struct iphdr *) (data + sizeof (struct ethhdr));
		if (iph->protocol == IPPROTO_TCP) {	// tcp
			tcph =
			    (struct tcphdr *) (data + sizeof (struct ethhdr) +
					       sizeof (struct iphdr));
			if (tcph->syn != 1 && ntohs(tcph->source) == 80 || ntohs(tcph->dest) == 80) {
				len = ntohs(iph->tot_len) - iph->ihl * 4 - tcph->doff * 4;
				//printf("len is %d and ack is %ld\n", len, ntohl(tcph->seq));
				if (len > 0) {
					stime = ph->ts.timestamp_s;
					memcpy(play_data, ((char *)tcph + tcph->doff * 4), len);
					
					if (start_stime == 0)
						start_stime = stime;
				
					/*	
					if (ntohl(tcph->seq) == 1895537127)
					{
						for (i = 0; i < len; ++i) {
							fprintf(fp1, "%x", (play_data[i]) & 0xff);
					}
					*/

					if (stime < (start_stime + INT_TIME))
					{
						//fprintf(fp1, "time is %ld and stime is %ld\n", stime, start_stime);	
						//printf("len is %d and ack is %ld\n", len, ntohl(tcph->seq));
						for (i = 0; i < len; ++i) 
							fprintf(fp1, "%x", play_data[i] & 0xff);
						fprintf(fp1, "\n");
					}
					else
					{
						//fprintf(fp2, "time is %ld and stime is %ld\n", stime, start_stime);	
						//fprintf("len is %d and ack is %ld\n", len, ntohl(tcph->seq));
						for (i = 0; i < len; ++i) 
							fprintf(fp2, "%x", play_data[i] & 0xff);
						fprintf(fp2, "\n");
					}
				}
			}
		}
	}
}

int
main(int argc, const char *argv[])
{
	pcap_file_header pfh;
	pcap_header ph;
	int count = 0;
	void *buff = NULL;
	int readSize = 0;
	int ret = 0;

	if (argc != 2) {
		printf("uage: ./a.out pcap_filename\n");
		return -1;
	}

	FILE *fp = fopen(argv[1], "r");
	if (fp == NULL) {
		fprintf(stderr, "Open file %s error.", argv[1]);
		return -1;
	}

	fread(&pfh, sizeof (pcap_file_header), 1, fp);

	fp1 = fopen("source", "w");
	fp2 = fopen("object", "w");
	buff = (void *) malloc(MAX_ETH_FRAME);

	for (count = 1;; count++) {
		memset(buff, 0, MAX_ETH_FRAME);
		readSize = fread(&ph, sizeof (pcap_header), 1, fp);
		if (readSize <= 0) {
			break;
		}

		if (buff == NULL) {
			fprintf(stderr, "malloc memory failed.\n");
			return -1;
		}
		

		readSize = fread(buff, 1, ph.capture_len, fp);
		if (readSize != ph.capture_len) {
			free(buff);
			fprintf(stderr, "pcap file parse error.\n");
			return -1;
		}
		printPcap(count, buff, &ph);

		if (feof(fp) || readSize <= 0) {
			break;
		}
	}

	fclose(fp);
	fclose(fp1);
	fclose(fp2);
	return ret;
}
