#include <stdio.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <linux/types.h>
#include "lcthw/dbg.h"
#include "lcthw/pack.h"

#define SIZE 30000
#define PACKET_LEN 30000

FILE *fp1;
FILE *fp2;
long long start_stime;

long Q = 1;
int i = 0;
int R = 1048583;
//int R = 10;
long RM = 1;
int chunk_num = 48;
int zero_num = 9;
int zero_value = 1;
static int count_packet = 0;
static int delay_time = 0;
// Compute hash for a string.
unsigned long
pack_hash(char *key, int M, int R, long Q)
{
	int j;
	unsigned long h = 0;

	for (j = 0; j < M; j++) {
		h = (R * h + key[j]) % Q;
	}
	return h;
}

/*
 * calculate the hash for parting point.
 */
void
pack_calculate_hash(char *playload, int playload_len, long Q, int R, long RM,
		    int zero_value, int chunk_num)
{
	long i;
	unsigned long txthash = pack_hash(playload, chunk_num, R, Q);

	//printf("%d|%d|", count_packet++, playload_len);
	fprintf(fp2, "%d|%d|", count_packet++, playload_len);

	if (delay_time == 0) {
		if ((txthash & zero_value) == 0) {
			//printf("0 ");
			fprintf(fp2, "0 ");
			delay_time = chunk_num * 12;
		}
	} else {
		delay_time--;
	}

	for (i = chunk_num; i < playload_len; i++) {
		txthash = (txthash + Q - RM * playload[i - chunk_num] % Q) % Q;
		txthash = (txthash * R + playload[i]) % Q;

		if (delay_time == 0) {
			if ((txthash & zero_value) == 0) {
				//printf("%ld ", i);
				fprintf(fp2, "%ld ", i);
				delay_time = chunk_num * 12;
			}
		} else {
			delay_time--;
		}
	}
	//printf("\n");
	fprintf(fp2, "\n");
}

void
printPcap(void *data, struct pcap_header *ph)
{
	//size_t size = ph->capture_len;
	struct ethhdr *eth;
	struct iphdr *iph;
	struct tcphdr *tcph;
	long long stime;
	char play_data[30000];
	int len;

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
			if ((tcph->syn != 1)
			    && ((ntohs(tcph->source) == 80)
				|| (ntohs(tcph->dest) == 80))) {
				len =
				    ntohs(iph->tot_len) - iph->ihl * 4 -
				    tcph->doff * 4;
				if (len > 0) {
					if (count_packet == 452) {
						debug
						    ("playload is %s and len is %d and seq is %ld",
						     play_data, len,
						     ntohl(tcph->seq));
					}
					stime = ph->ts.timestamp_s;
					memcpy(play_data,
					       ((char *) tcph + tcph->doff * 4),
					       len);

					//get the data parting point
					pack_calculate_hash(play_data, len,
							    Q, R, RM,
							    zero_value,
							    chunk_num);
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
	// precalculate
	for (i = 0; i < 60; ++i)
		Q = (2 * Q);

	for (i = 1; i <= chunk_num - 1; i++)
		RM = (R * RM) % Q;

	for (i = 0; i < zero_num; ++i)
		zero_value = (2 * zero_value);
	zero_value = zero_value - 1;

	FILE *fp = fopen(argv[1], "r");
	if (fp == NULL) {
		fprintf(stderr, "Open file %s error.", argv[1]);
		return -1;
	}

	fread(&pfh, sizeof (pcap_file_header), 1, fp);

	fp1 = fopen("source", "w");
	fp2 = fopen("result", "w");
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
		if (readSize != (int) ph.capture_len) {
			free(buff);
			fprintf(stderr, "pcap file parse error.\n");
			return -1;
		}
		printPcap(buff, &ph);

		if (feof(fp) || readSize <= 0) {
			break;
		}
	}

	fclose(fp);
	fclose(fp1);
	fclose(fp2);
	return ret;
}
