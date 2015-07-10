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
#include "../src/lcthw/hashmap.h"
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
part_point *part = NULL;
keyvalue *key = NULL;
Hashmap *map = NULL;
void *value = NULL; //for filling the value in hashmap.
HashmapNode *pre_node = NULL; //for internal links.	
unsigned long save_len = 0;
unsigned long mid_len = 0;
unsigned long sum_len = 0;

/*
 * this hash function need verify the effect.
 * for SHA-1.
 */
static uint32_t
Hashmap_mode_hash(void *data)
{
	unsigned long hash = 0;
	int i; 
	char *tmp = (char *)data;
	
	for (i = 0; i < SHA ; i++)
	{
		hash = (10 * hash + (int)(tmp[i]&0xff)) % BUCKETS_LEN;
	}
	//debug("hash is %lu", hash);
	return hash;
}

static int
Hashmap_mode_compare(void *a, void *b)
{
	// 0 stands for equal.
	int result = memcmp(a, b, SHA);
	/*int j;
	for (j = 0; j < SHA; j++) {
		printf("%x:", tmp1[j]&0xff);
	}
	printf("\n");
	for (j = 0; j < SHA; j++) {
		printf("%x:", tmp2[j]&0xff);
	}
	printf("\n");
	*/
	//debug("a is %s", (char *)a);
	return result;
}


/*
 * calculate the SHA-1.
 */
void
calculate_sha(char *data, long begin, long end, char* result)
{
	int cfd = -1, i;
	struct cryptodev_ctx ctx;
	uint8_t digest[20] = {0};
	//char text[] = "1The quick brown fox jumps over the lazy dog";
	//char text[] = "1The quick brown fox jumps over the lazy dog";
	//uint8_t expected[] = "\x2f\xd4\xe1\xc6\x7a\x2d\x28\xfc\xed\x84\x9e\xe1\xbb\x76\xe7\x39\x1b\x93\xeb\x12";

	int len = end - begin + 1;
	char *tmp = (char *) malloc(len * sizeof (char));
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
	memcpy(result, digest, SHA);
	//printf("\n");
	//fprintf(fp2, "\n");

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
 */
void
pack_calculate_part(char *playload, int playload_len, long Q, int R, long RM,
		    int zero_value, int chunk_num)
{
	long i;
	

	unsigned long txthash = pack_hash(playload, chunk_num, R, Q);
	part_set(part, count_packet);
	part_set(part, playload_len);
	fprintf(fp2, "%d|%d|", count_packet++, playload_len);

	if (delay_time == 0) {
		if ((txthash & zero_value) == 0) {
			part_set(part, (chunk_num-1));
			fprintf(fp2, "%d ", (chunk_num-1));
			delay_time = chunk_num * 12;
		}
	} else {
		--delay_time;
	}

	for (i = chunk_num; i < playload_len; i++) {
		txthash = (txthash + Q - RM * playload[i - chunk_num] % Q) % Q;
		txthash = (txthash * R + playload[i]) % Q;

		if (delay_time == 0) {
			if ((txthash & zero_value) == 0) {
				part_set(part, i);
				//if (i == playload_len-1)
				//	debug("fuck i is %ld", i);
				fprintf(fp2, "%ld ", i);
				delay_time = chunk_num * 12;
			}
		} else {
			delay_time--;
		}
	}
	fprintf(fp2, "\n");
}

/*
 * 这个函数理解有点不清楚。
 * 这个函数可以只用于发现新的节点，进行内部连接。而不做统计用。
 */
void 
set_internallinks(void *key, size_t len)
{
	unsigned long i;
	int same = 0;

	if (pre_node == NULL) {
		/*
		 * you are the first one.
		 */
		pre_node = Hashmap_getnode(map, key);
	} 
	else {
		/*
		 * two chioces.
		 * one: the pre_node next link this node.
		 * two: find the same node, not add next link.
		 */
		for (i = 0; i < pre_node->next->index; ++i) {
			if (memcmp((pre_node->next->key + i * (pre_node->next->step)), key, pre_node->next->step) == 0) {
			//if (memcmp(pre_node->next->key , key, pre_node->next->step) == 0) {
			//if (memcmp(key , key, pre_node->next->step) == 0) {
				same = 1;
				break;
			}
		}

		/*
		 * check key of pre_node 
         */
		if (same == 0) {
			printf("step is:%d\n", pre_node->next->step);
			check_mem(pre_node->key);
			check_mem(key);
			if (memcmp(pre_node->key, key, pre_node->next->step) == 0) 
			//if (memcmp(pre_node->next->key, key, pre_node->next->step) == 0) 
				same = 1;
		}

		if (same == 0) {
			/*
			 * not find the same one.	
			 */
			void *tmp  = Hashmap_getnode(map, key);
			keyvalue_push(pre_node->next, tmp);
			pre_node = tmp;
		}
		else {
			pre_node = Hashmap_getnode(map, key);
		}

		if (same == 1) {
			save_len += len;
			printf("got it!\n");
		}			
	}
	error:
		printf("error memory\n");	
}

/*
 * calculate the SHA-1 for store the data.
 */
void 
pack_SHA(char *playload, int playload_len)
{
	long pp;
 	int i, j;
	int len;
	char *res = NULL;	
	void *hashmap_key = NULL;
	res = malloc(SHA*sizeof(char));

	switch (part->end) {
		case 0:
		case 1:
			log_err("Error case");		
			exit(-1);
			break;
		/*
		 * no part point.
         * 1. store the remain data.
         */
		case 2:
			chunk_merge(remain_data, playload, 0, playload_len-1);	
			break;
		/*
		 * just one part point.
         * 1.calculate SHA only once. 
         * 2.store the remain data.
         */
		case 3:
			pp = part->index[2];
			chunk_merge(remain_data, playload, 0, pp);
			len = remain_data->end;
			calculate_sha(remain_data->content, 0, remain_data->end-1, res);
			chunk_clean(remain_data);
			//if(pp+1 > playload_len-1)
			//	debug("fuck1 begin is:%ld, end is:%ld, playload_len is:%ld", pp+1, playload_len-1, playload_len);
			if (pp != playload_len-1)
				chunk_store(remain_data, playload, pp+1, playload_len-1);
			if (Hashmap_get(map, res) == NULL) {
				hashmap_key = keyvalue_push(key, res);
				Hashmap_set(map, hashmap_key, hashmap_key);
			}
			else {
				debug("find it");
				mid_len += len;
			}
			//set_internallinks(hashmap_key, len);
			break;
		/*
		 * more than one part point.
		 * 1.calculate SHA
         * 2.store the remain data.
         */
		default:
			for (i = 2; i < part->end; ++i) {
				if (i == 2) {
					pp = part->index[i];
					chunk_merge(remain_data, playload, 0, pp);
					len = remain_data->end;
					calculate_sha(remain_data->content, 0, remain_data->end-1, res);
					chunk_clean(remain_data);
				}
				else {
					len = part->index[i] - part->index[i-1];
					calculate_sha(remain_data->content, part->index[i-1]+1, part->index[i], res);
				}

				if (Hashmap_get(map, res) == NULL) {
					hashmap_key = keyvalue_push(key, res);
					Hashmap_set(map, hashmap_key, hashmap_key);
					//printf("set it\n");
					debug("set it");
				}
				else {
					debug("find it");
					//printf("find it\n");
					mid_len += len;
				}
				//set_internallinks(hashmap_key, len);
			}
			//debug("chunk_store i is:%d and end is:%d, begin is:%ld", (i+1), playload_len-1, part->index[i-1]);
			//if(pp+1 > playload_len-1)
			//	debug("fuck2 begin is:%d, end is:%d", pp+1, playload_len-1);
			chunk_store(remain_data, playload, part->index[i-1]+1, playload_len-1);
			break;
	}
	free(res);
}

/*
 * calculate the SHA in remain data.
 */
void 
remain_SHA()
{
	char *res = NULL;	
	void *hashmap_key = NULL;
	size_t len;

	res = malloc(SHA*sizeof(char));
	len = remain_data->end;
	calculate_sha(remain_data->content, 0, remain_data->end-1, res);
	chunk_clean(remain_data);
				
	if (Hashmap_get(map, res) == NULL) {
		hashmap_key = keyvalue_push(key, res);
		Hashmap_set(map, hashmap_key, hashmap_key);
		//printf("set it\n");
		debug("set it");
	}
	else {
		debug("find it");
		//printf("find it\n");
	}
	//set_internallinks(hashmap_key, len);

	free(res);
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
				
					/*
     				 * small packet, pass it.
     				 */
					if (len >= chunk_num) {
						part_clean(part);
						
						//get the data parting point
						pack_calculate_part(play_data, len, Q, R, RM, zero_value, chunk_num);
						pack_SHA(play_data, len);
						sum_len += len;
					}
				}
			}
		}
	}
}

void 
text_test(const char* filename, long Q, int R, long RM, int  zero_value, int chunk_num) 
{
	FILE *input;
	char *source = NULL;
	int data_len = 6000;

	input = fopen(filename, "r");
	check((input != NULL), "Can't find the file.");

	source = (char *)malloc(data_len);
	check((source != NULL), "Can't alloc merrory.");
	
	while (fgets(source, data_len, input) != NULL) {
		//printf("%s", source);
		//printf("len is %d\n", (int)strlen(source));
	}
	
	part_clean(part);
	pack_calculate_part(source, (int)strlen(source), Q, R, RM, zero_value, chunk_num);
	pack_SHA(source, (int)strlen(source));
	remain_SHA();

	printf("total size is%d, save size is%d\n", (int)strlen(source), (int)save_len);
	free(source);
	return;
	error:
		printf("something is error.\n");
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
	
	part = part_create();
	check_mem(part);

	key = keyvalue_create(0, 0);
	check_mem(key);

	map = Hashmap_create(BUCKETS_LEN, Hashmap_mode_compare, Hashmap_mode_hash);
	check(map != NULL, "Failed to create map.");

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
			ret = -1;
		}
		printPcap(buff, &ph);

		if (feof(fp) || readSize <= 0) {
			break;
		}
	}
					

	remain_SHA();
	printf("total size is:%lu, m_size is:%lu, save size is:%lu\n", sum_len, mid_len, save_len);
	
	
	/*
	 * text test.
	 */
	//fp2 = fopen("result", "w");
	//text_test(argv[1], Q, R, RM, zero_value, chunk_num);

    error:
		chunk_destroy(remain_data);
		part_destroy(part);
		keyvalue_destroy(key);
		Hashmap_destroy(map);
		fclose(fp);
		fclose(fp1);
		fclose(fp2);
		//fclose(fp2);
		free(buff);
		return ret;
}
