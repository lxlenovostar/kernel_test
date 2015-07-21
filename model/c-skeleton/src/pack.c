#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <string.h>
#include <linux/types.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <dirent.h> 
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
int chunk_num = 48;  //控制最小值
int zero_num = 7;
unsigned long zero_value = 1;
static int count_packet = 0;
static int delay_time = 0;
int step = 64; //控制块长
const int cmplen = 96;

chunk *remain_data = NULL;
part_point *part = NULL;
keyvalue *key = NULL;
keyvalue *value = NULL;
Hashmap *map = NULL;
//void *value = NULL; //for filling the value in hashmap.
HashmapNode *pre_node = NULL; //for internal links.	
unsigned long save_len = 0;
unsigned long rm_len = 0;
unsigned long sum_len = 0;
unsigned long data[300];
char *strmem = "try to be a value";

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
	/*char *tmp1 = (char *)a;
	char *tmp2 = (char *)b;
	int j;
	for (j = 0; j < SHA; j++) {
		printf("%x:", tmp1[j]&0xff);
	}
	printf("\n");
	for (j = 0; j < SHA; j++) {
		printf("%x:", tmp2[j]&0xff);
	}
	printf("\n");*/
	int result = memcmp(a, b, SHA);
	
	return result;
}

void 
store_data(char *data, long begin, long end, uint8_t *name, int find)
{
	char *dir_set = "/root/kernel_test/model/c-skeleton/tmp/a/";
	char *dir_find = "/root/kernel_test/model/c-skeleton/tmp/b/";
	char *filename = NULL;
	size_t len_set = strlen(dir_set);
	//size_t len_find = strlen(dir_find);
	int fd;
	int i;
	char buffer[2*SHA + 1];
	buffer[2*SHA] = '\0';
	filename = (char *)malloc(300 * sizeof(char));
	
	if (find == 0) {
		/*
		 * build the name.
         */
		strcpy(filename, dir_set);
		for (i = 0; i < SHA; ++i) {
			snprintf(buffer+2*i, 3,  "%02x", name[i] & 0xff);
		}
		strncpy(filename + len_set, buffer, strlen(buffer));
		filename[len_set + strlen(buffer)] = '\0';
		
		/*
		//just for test.
		printf("%s\n", buffer);
		for (i = 0; i < SHA; ++i) 
				printf("%02x:", name[i] & 0xff);
		printf("\n");
		for (i = 0; i < 2*SHA - 1; i += 2) 
				printf("%c%c:", buffer[i], buffer[i+1]);
		printf("\n");
		*/
		fd = open(filename, O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR);
		if (write(fd, data, (end - begin + 1)) != (end - begin + 1))
		{
			debug("something error with write.");
		}

		close(fd);
	} 
	else {
			DIR *d;
			struct dirent *dir;
			
			strcpy(filename, dir_find);
			for (i = 0; i < SHA; ++i) {
				snprintf(buffer+2*i, 3,  "%02x", name[i] & 0xff);
			}
			strncpy(filename + len_set, buffer, strlen(buffer));
			filename[len_set + strlen(buffer)] = '\0';

			d = opendir(dir_find);
			if (d){
				while ((dir = readdir(d)) != NULL) {
					 if (strcmp(dir->d_name, "ffea76ff3fe9d804ddfde408ddf3ecbe13d604dc") == 0) 
						printf("ok %s\n", dir->d_name);
					//printf("%s\n", dir->d_name);
				}
				closedir(d);
			}		
		}	

	free(filename);
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

	//fprintf(fp2, "\nsha-1: %d\n", len);

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
		//fprintf(fp2, "%02x:", digest[i]);
	}
	memcpy(result, digest, SHA);
	//printf("\n");
	//fprintf(fp2, "\n");

	//store_data(data, begin, end, digest, 0);
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
		
	//printf("%lu\n", h);
}

void data_set()
{

	int j = 0;
	/*
	// jpeg
	int tmp1[2] = {0xFF, 0xD9}; // picture end
	int tmp2[2] = {0xFF, 0xD8}; // picture begin

	//gif 
	char tmp3[2] = {'7', 'a'}; // picture begin
	char tmp4[2] = {'9', 'a'}; // picture begin
	int tmp5[2] = {0x00, 0x3b}; // picture begin

	//png
	char *tmp = "HT";
	*/
	uint16_t value[] = {0x8950, 0x504e, 0x4945, 0xffd8, 0xffe0, 0xffe1, 0xffd9};
	int i, len;
	len = sizeof(value)/sizeof(uint16_t);
	
	char *keyword[] = {"7a", "9a", "ED", "EA", "HE", "(.", "..", "E.", "P.", ".E", ".P", "QE", "ET", "OS", "ST", "Ac", "Us","\r\n", "jp", "pg", "pn", "ng", "gi", "if", "ho", "HO", "HP", "hp", "WW","ol", "ve", "GE", "PO", "ww", "cd", "HT", "ht", "zh", "ZH", NULL};
	char **kwp;

	for (kwp = keyword; *kwp != NULL; kwp++) {
		data[j] = pack_hash(*kwp, 2, R, Q);
		++j;
		//printf("%s\n", *kwp);
	}
	
	{
		unsigned long h = 0;

		for (i = 0; i < len; i++) {
			h = (R * h + ((value[i]>>8)&0xff)) % Q;
			//printf("%lu\n", h);
			h = (R * h + (value[i]&0xff)) % Q;
			//printf("%lu\n", h);
			data[j] = h;
			++j;
			h = 0;
			//printf("%x\n", (value[i]>>8) &0xff);
			//printf("%x\n", value[i] &0xff);
		}
	}	
}

void
pack_calculate_part2(char *playload, int playload_len, long Q, int R, long RM,
		    unsigned long zero_value, int chunk_num)
{	/*
	unsigned char longval = 0;
	long i;
	unsigned long j = 0;
	
	part_set(part, count_packet);
	part_set(part, playload_len);
	fprintf(fp2, "%d|%d|", count_packet++, playload_len);
	
	for (i = 0; i < playload_len; i++) {
		//longval = longval << 1;
		longval ^= playload[i];
		//printf("%lu\n", longval);
		++j;
		if (delay_time == 0) {
			if (j >= chunk_num && (longval & zero_value) == 0) {
				j = 0;
				longval = 0;
				part_set(part, i);
				fprintf(fp2, "%ld ", i);
				//delay_time = chunk_num * 12;
				delay_time = 100;
			}
		}
		else {
			--delay_time;
		}	
	}	 
	fprintf(fp2, "\n");
	*/
}

unsigned long 
check_data_point(char *playload, long Q, int R, int index) 
{
	/*
	// jpeg
	int tmp1[2] = {0xFF, 0xD9}; // picture end
	int tmp2[2] = {0xFF, 0xD8}; // picture begin

	//gif 
	char tmp3[2] = {'7', 'a'}; // picture begin
	char tmp4[2] = {'9', 'a'}; // picture begin
	int tmp5[2] = {0x00, 0x3b}; // picture begin
	
	//unsigned long mask = 75498060;
	//unsigned long mask1 = 267388861;
	//unsigned long mask2 = 267388881;
	unsigned long mask = 267388882;
	unsigned long mask1 = 267388881;
	unsigned long mask2 = 57672162;
	unsigned long mask3 = 59769328;
	unsigned long mask4 = 59;
	unsigned long mask5 = 75498060;
	unsigned long mask6 = 116392821;
	unsigned long mask7 = 74449462;
	unsigned long mask8 = 13631589;
	*/
	int j;

	if (index - 1 >= 0){
		int begin = index - 1;
		int len = 2;
		unsigned long txthash = pack_hash(playload + begin, len, R, Q);
		//if (txthash == mask8 || txthash == 0 || txthash == mask6 || txthash == mask7 || txthash == mask5 ||txthash == mask || txthash == mask1 || txthash == mask2 || txthash == mask3 || txthash == mask4) {
		for (j = 0; j < 300; ++j) {
			if (data[j] != 0) {
				if (data[j] == txthash)		
					return 1;
			}
			else
				return 0;
		}
		return 0;
	}
	else 
	{
		return 0;
	}	
}

/*
 * calculate the hash for parting point.
 */
void
pack_calculate_part(char *playload, int playload_len, long Q, int R, long RM,
		    unsigned long zero_value, int chunk_num)
{
	long i, j = 1;
	unsigned long txthash = pack_hash(playload, chunk_num, R, Q);
	
	part_set(part, count_packet);
	part_set(part, playload_len);
	fprintf(fp2, "%d|%d|", count_packet++, playload_len);
	
	
	if (delay_time == 0) {
		//if ((txthash & zero_value) == 0) {
		if ((txthash & zero_value) == 0 || check_data_point(playload, Q, R, (chunk_num - 1))) {
			part_set(part, (chunk_num-1));
			fprintf(fp2, "%d ", (chunk_num-1));
			delay_time = step;
		}
	} else {
		--delay_time;
	}

	for (i = chunk_num; i < playload_len; i++) {
		txthash = (txthash + Q - RM * playload[i - chunk_num] % Q) % Q;
		txthash = (txthash * R + playload[i]) % Q;

		++j;
		if (delay_time == 0) {
			//if ((txthash & zero_value) == 0 || j >= 52) {
			//if ((txthash & zero_value) == 0) {
			if ((txthash & zero_value) == 0 || check_data_point(playload, Q, R, i)) {
				part_set(part, i);
				fprintf(fp2, "%ld ", i);
				delay_time = step;
				j = 0;
			}
		} else {
			delay_time--;
			j--;
		}
	}
	fprintf(fp2, "\n");
}

/*
 * 这个函数可以只用进行内部连接。而不做统计用。
 * 现阶段没有用处。
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
 *     ||||||||||||||//////  ==> //////
 *     calculate the number of tail data.  
 */
void
rm_data(char *playload, int playload_len, int i, int pp, char *data) 
{
	int step = 32;
	int j; 
	
	if (pp + cmplen > playload_len - 1 || data == strmem)
	{
		return ;
	}
	else
	{
		for (j = 0; j < cmplen; j += step) 
		{
			if (memcmp(playload+pp+1+j, data+j, step) == 0)
			{
				pp += step;
				rm_len += step;
			}
			else 
			{ 
				break;
			}	
		}

		for (j = i; j < part->end; ++j) 
		{
			if (pp > part->index[j])
				part->index[j] = pp;
		} 					
	}
}


/*
 * calculate the SHA-1 for store the data.
 */
void 
pack_SHA(char *playload, int playload_len)
{
	long pp;
 	int i;
	int len;
	char *res = NULL;	
	void *hashmap_key = NULL;
	res = malloc(SHA*sizeof(char));
	char tmp_value[96] = {0};
	void *data = NULL;
	void *data_mem = NULL;

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
			
			if ((data = Hashmap_get(map, res)) == NULL) {
				hashmap_key = keyvalue_push(key, res);
				check_mem(hashmap_key);
				{
					if (pp + cmplen > playload_len - 1)
					{
						memcpy(tmp_value, playload+pp+1, cmplen);
						data_mem = keyvalue_push(value, tmp_value);
						memset(tmp_value, 0, cmplen);
					}					
					else 
						data_mem = strmem;					
				}
				Hashmap_set(map, hashmap_key, data_mem);
				//store_data(remain_data->content, 0, remain_data->end-1, res, 0);
			}
			else {
				debug("find it");
				//store_data(remain_data->content, 0, 1, res, 1);
				save_len += len;
				i = 2;
				rm_data(playload, playload_len, i, pp, data); 
			}
			
			//chunk_clean(remain_data);
			if (pp != playload_len-1) {
				chunk_store(remain_data, playload, pp+1, playload_len-1);
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
				pp = part->index[i];
				if (i == 2) {
					chunk_merge(remain_data, playload, 0, pp);
					len = remain_data->end;
					calculate_sha(remain_data->content, 0, remain_data->end-1, res);
					chunk_clean(remain_data);
				}
				else {
					len = part->index[i] - part->index[i-1];
					if (len == 0) //we have handle it for the same data at the tail.
						continue;
					calculate_sha(playload, part->index[i-1]+1, part->index[i], res);
				}

				if ((data = Hashmap_get(map, res)) == NULL) {
					hashmap_key = keyvalue_push(key, res);
					check_mem(hashmap_key);
					{
						if (pp + cmplen > playload_len - 1)
						{
							memcpy(tmp_value, playload+pp+1, cmplen);
							data_mem = keyvalue_push(value, tmp_value);
							memset(tmp_value, 0, cmplen);
						}
						else 
							data_mem = strmem;					
					}
					Hashmap_set(map, hashmap_key, data_mem);
					debug("set it");
				}
				else {
					debug("find it");
					save_len += len;
					rm_data(playload, playload_len, i, pp, data); 
				}
				//set_internallinks(hashmap_key, len);
			}
			if (part->index[i-1] != playload_len-1) 
				chunk_store(remain_data, playload, part->index[i-1]+1, playload_len-1);
			break;
	}
		error:
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
		debug("set it");
	}
	else {
		debug("find it");
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
						//pack_calculate_part2(play_data, len, Q, R, RM, zero_value, chunk_num);
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
	FILE *fp = NULL;
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
	
	value = keyvalue_create(10000, cmplen);
	check_mem(value);

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

	data_set();
	
	fp = fopen(argv[1], "r");
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
	printf("total size is:%lu, save size is:%lu, add rm size is:%lu, compress rate is:%f %% \n", sum_len, save_len, rm_len, 100*((float)(save_len+rm_len)/(float)sum_len));
	
	
	/*
	 * text test.
	 */
	//fp2 = fopen("result", "w");
	//text_test(argv[1], Q, R, RM, zero_value, chunk_num);

    error:
		chunk_destroy(remain_data);
		part_destroy(part);
		keyvalue_destroy(key);
		keyvalue_destroy(value);
		Hashmap_destroy(map);
		fclose(fp);
		fclose(fp1);
		fclose(fp2);
		//fclose(fp2);
		free(buff);
		return ret;
}
