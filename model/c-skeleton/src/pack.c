#include <stdio.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <linux/types.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include "../src/lcthw/sha.h"
#include "../src/lcthw/chunk.h"
#include "lcthw/dbg.h"
#include "lcthw/pack.h"

#define SIZE 30000
#define PACKET_LEN 30000
#define BUCKETS_LEN 997

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

chunk *remain_data = NULL;


/*
 * calculate the SHA-1.
 */
void 
calculate_sha(char *data, long begin, long end)
{
	int cfd = -1, i;
    struct cryptodev_ctx ctx;
    uint8_t digest[20];
    //char text[] = "1The quick brown fox jumps over the lazy dog";
    //char text[] = "1The quick brown fox jumps over the lazy dog";
    //uint8_t expected[] = "\x2f\xd4\xe1\xc6\x7a\x2d\x28\xfc\xed\x84\x9e\xe1\xbb\x76\xe7\x39\x1b\x93\xeb\x12";
	
	int len = end - begin + 1;
	char *tmp = (char *)malloc(len * sizeof(char));
	memcpy(tmp, data + begin, len);
	
	fprintf(fp2, "\nsha-1: %d\n", len);

    cfd = open("/dev/crypto", O_RDWR, 0);
    if (cfd < 0) {
        perror("open(/dev/crypto)");
        //return 1;
    }

    if (fcntl(cfd, F_SETFD, 1) == -1) {
        perror("fcntl(F_SETFD)");
        //return 1;
    }

    sha_ctx_init(&ctx, cfd, NULL, 0);
    sha_hash(&ctx, tmp, len, digest);
    sha_ctx_deinit(&ctx);

    for (i = 0; i < 20; i++) {
        //printf("%02x:", digest[i]);
        fprintf(fp2, "%02x:", digest[i]);
    }
    //printf("\n");
	fprintf(fp2, "\n");
   
	/* 
    if (memcmp(digest, expected, 20) != 0) {
        printf("SHA1 hashing failed\n");
        //return 1;
    }
	*/

    if (close(cfd)) {
        perror("close(cfd)");
        //return 1;
    }

	free(tmp);
}

/*
 * Compute hash for a string.
 */
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
 *
 *
 *  (data0, data0) [point0, point0  data1 point1 point1] (data2, data2)  
 *
 * 1. there are one parting point. we should calculate_sha one SHA-1,
 * and store part of the data.
 * 2. there are N patring point, we should calculate_sha N - 1  SHA-1, 
 * and store part of the data.
 * 3. there are none parting point, we should store part of the data. 
 * #bgein = -1
 */
void
pack_calculate_hash(char *playload, int playload_len, long Q, int R, long RM,
		    int zero_value, int chunk_num)
{
	long i;
	long begin = -1; 
	long end = -1;
	long end_point = -1;   //stand for the last parting point.

	unsigned long txthash = pack_hash(playload, chunk_num, R, Q);
	fprintf(fp2, "%d|%d|", count_packet++, playload_len);
 
	if (delay_time == 0) {
		if ((txthash & zero_value) == 0) {
			fprintf(fp2, "0 ");
			delay_time = chunk_num * 12;

			end_point = chunk_num;

			if (remain_data->max > 0) {
				// some last remaining data.
				chunk_merge(remain_data, playload, 0, chunk_num-1);
				calculate_sha(remain_data->content, 0, remain_data->max);
				chunk_clean(remain_data);
				begin = chunk_num; 
			}
			else
			{
				//no remaining data. we just begin.
				begin = 0;
			}
		}
	} 
	else {
		--delay_time;
	}

	for (i = chunk_num; i < playload_len; i++) {
		txthash = (txthash + Q - RM * playload[i - chunk_num] % Q) % Q;
		txthash = (txthash * R + playload[i]) % Q;

		if (delay_time == 0) {
			if ((txthash & zero_value) == 0) {
				fprintf(fp2, "%ld ", i);
				delay_time = chunk_num * 12;
				end_point = i + 1;
				
				if (begin >= 0) {
					end = i;
					calculate_sha(playload, begin, end);
					begin = i + 1;
				}
				else {
					if (remain_data->max > 0) {
						// some last remaining data.
						chunk_merge(remain_data, playload, 0, i);
						calculate_sha(remain_data->content, 0, remain_data->max);
						chunk_clean(remain_data);
						begin = i + 1; 
					}
					else
					{
						/*
						 * no remaining data. but we still nedd calculate.
                         * like (data0, data0)
						 */
						calculate_sha(playload, 0, i - chunk_num + 1);
						begin = i - chunk_num + 1;
					}
				} 
			}
		} else {
			delay_time--;
		}
	}
	fprintf(fp2, "\n");

	//store the data.
	if (end_point > 0) {
		if (end_point != playload_len) {
			chunk_store(remain_data, playload, end_point, playload_len - 1);
		}
		else{
			chunk_clean(remain_data);
		}
	}
	else {
		chunk_store(remain_data, playload, 0, playload_len - 1);
	}
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
						    ("playload is %s and len is %d and seq is %d",
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

	remain_data = chunk_create();
	check_mem(remain_data);

	if (argc != 2) {
		printf("uage: ./a.out pcap_filename\n");
		ret = -1;
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
		ret = -1;
	}

	fread(&pfh, sizeof (pcap_file_header), 1, fp);

	fp1 = fopen("source", "w");
	fp2 = fopen("result", "w");
	buff = (void *) malloc(MAX_ETH_FRAME);

	//map = Hashmap_create(BUCKETS_LEN, Hashmap_mode_compare, Hashmap_mode_hash);
    //check(map != NULL, "Failed to create map.");

	for (count = 1;; count++) {
		memset(buff, 0, MAX_ETH_FRAME);
		readSize = fread(&ph, sizeof (pcap_header), 1, fp);
		if (readSize <= 0) {
			break;
		}

		if (buff == NULL) {
			fprintf(stderr, "malloc memory failed.\n");
			ret = -1;
		}

		readSize = fread(buff, 1, ph.capture_len, fp);
		if (readSize != (int) ph.capture_len) {
			free(buff);
			fprintf(stderr, "pcap file parse error.\n");
			ret =  -1;
		}
		printPcap(buff, &ph);

		if (feof(fp) || readSize <= 0) {
			break;
		}
	}

	error:
			chunk_destroy(remain_data);
			fclose(fp);
			fclose(fp1);
			fclose(fp2);
			return ret;
}
