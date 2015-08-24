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
#include <limits.h>
#include "../src/lcthw/sha.h"
#include "../src/lcthw/chunk.h"
#include "../src/lcthw/hashmap.h"
#include "lcthw/dbg.h"
#include "lcthw/pack.h"

#define SIZE 36000
#define PACKET_LEN 36000
#define BUCKETS_LEN 997

FILE *fp1;
FILE *fp2;
FILE *fp3;
FILE *fp4;
char *aux;
long long start_stime;

long Q = 1;
int i = 0;
int R = 1048583;
//int R = 10;
long RM = 1;
int zero_num = 6;
unsigned long zero_value = 1;
static int count_packet = 0;
int step = 16; 
const int cmplen = 96;

chunk *remain_data = NULL;
part_point *part = NULL;
keyvalue *key = NULL;
//keyvalue *value = NULL;
Hashmap *map = NULL;
//void *value = NULL; //for filling the value in hashmap.
HashmapNode *pre_node = NULL; //for internal links.	
unsigned long save_len = 0;
unsigned long rm_len = 0;
unsigned long sum_len = 0;
unsigned long data[300];

/*
 * WINN
 */
#define WINDOW 16
#define THRESHOLD 16 
int chunk_num = 16;  //最小hash值的计算
long min_index = 0;
//unsigned long min_hash = ULLONG_MAX;
unsigned long min_hash = 0;
int window = WINDOW;

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
	uint16_t value[] = {0x850b, 0x0bf0, 0xdbfe, 0x00e2, 0xccc9, 0xc1cc,0xbfc1, 0xc9bf, 0xc3c9, 0x4514, 0x5001, 0x4500, 0x1451, 0x0014, 0x21ff, 0x83ff, 0x68ff, 0xff00, 0x0085, 0xffff, 0x0000, 0x8950, 0x504e, 0x4945, 0xffd8, 0xffe0, 0xffe1, 0xffd9};
	int i, len;
	len = sizeof(value)/sizeof(uint16_t);
	
	char *keyword[] = {"90", "7a", "9a", "ED", "EA", "HE", "(.", "..", "E.", "P.", ".E", ".P", "QE", "ET", "OS", "ST", "Ac", "Us","\r\n", "jp", "pg", "pn", "ng", "gi", "if", "ho", "HO", "HP", "hp", "WW","ol", "ve", "GE", "PO", "ww", "cd", "HT", "ht", "zh", "ZH", NULL};
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
merge(char *a, int lo, int mid, int hi)
{
	int i = lo, j = mid + 1;
	int k;

	memcpy(aux + lo, a + lo, hi - lo + 1);
	//for (k = lo; k <= hi; k++) // Copy a[lo..hi] to aux[lo..hi].
	//	aux[k] = a[k];
	

	for (k = lo; k <= hi; k++) {
		if (i > mid)
			a[k] = aux[j++];
		else if (j > hi)
			a[k] = aux[i++];
		else if (aux[j] < aux[i])
			a[k] = aux[j++];
		else
			a[k] = aux[i++];
	}
}

void 
sort(char *a, int lo, int hi)
{ 
	if (hi <= lo) 
		return;
		
	int mid = lo + (hi - lo)/2;
	sort(a, lo, mid);  // Sort left half.
	//printf("%d,%d\n", lo, mid);
	sort(a, mid+1, hi);  // Sort right half.
	//printf("%d,%d\n", mid+1, hi);
	merge(a, lo, mid, hi); // Merge results.
} 

void
store_char(char *playload, int playload_len)
{
	//fprintf(fp2, "%d|%d|", count_packet++, playload_len);
	int i, count;
	char cmp;

	char *tmp_data = (char *)malloc(sizeof(char) * playload_len);
	aux = (char *)malloc(sizeof(char) * playload_len);
	memcpy(tmp_data, playload, playload_len);

	//first sort
	sort(tmp_data, 0, playload_len - 1);

	//lookup 
	count = 1;
	cmp = tmp_data[0];
	for (i = 1; i < playload_len; ++i) {
		
		if (cmp != tmp_data[i]) {
			fprintf(fp3, "%x:%d|", cmp&0xff, count);
			count = 1;
			cmp = tmp_data[i];
		}
		else {
			count++;
		}
	}		
	fprintf(fp3, "\n");

	free(tmp_data);
	free(aux);
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
	int data_len;

	if (index - 1 >= 0){
		int begin = index - 1;
		int len = 2;
		//uint8_t gen_data[] = {0,32,48, 101, 105, 115, 116, 255};
		uint8_t gen_data[] = {0x70, 0x2d, 0xa2, 0x93, 0xc0, 0x18, 0x61, 0x03, 0xcd, 0x25, 0xb1, 0x67, 0xd6, 0xc6, 0xe0, 0x02};
		//uint8_t gen_data[] = {0x70, 0x2d, 0xa2, 0x93, 0xc0, 0x18, 0x61, 0x03};
		data_len = sizeof(gen_data)/sizeof(uint8_t);

		char a = (playload + begin)[0];	
		char b = (playload + begin)[1];
		char a1 = (a>>4) & 0xf;
		char a2 = a & 0xf;
		char b1 = (b>>4) & 0xf;
		char b2 = b & 0xf;

		//if ((a == b) || (a == b + 1) || (b == a + 1) || (a == 0x00) || (b == 0x00) || (a == 0xff) || (b == 0xff))
		//if ((a == b) || (a == b + 1) || (b == a + 1) || (a1 == a2) || (b1 == b2) || (a1 == 0xf) || (a2 == 0xf) || (b1 == 0xf) || (b2 == 0xf))
		//if ((a == b) || (a == b + 1) || (b == a + 1))
		if ((a == b) || (a == b + 1) || (b == a + 1) || (a1 == 0x0) || (a2 == 0x0) || (b1 == 0x0) || (b2 == 0x0))
			return 1;
	
		return 0;	

		for (j = 0; j < data_len; j++) {
			if ((playload + begin)[0] == gen_data[j] || (playload + begin)[1] == gen_data[j])
				return 1;
		}
		return 0;
	
		if ((playload + begin)[0] == (playload + begin)[1] || ((playload + begin)[1] == (playload + begin)[0] + 1) || ((playload + begin)[0] == (playload + begin)[1] + 1) )
		//if ((playload + begin)[0] == (playload + begin)[1])
			return 1;
			
		return 0;	
		//if (((playload + begin)[1]>>4 & 0xf) == 0xb || ((playload + begin)[1]>>4 & 0xf) == 0xc || ((playload + begin)[0]>>4 & 0xf) == 0xb || ((playload + begin)[0]>>4 & 0xf) == 0xc)
		//	return 1;
		

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
	long i;
	unsigned long txthash = pack_hash(playload, chunk_num, R, Q);
	
	part_set(part, count_packet);
	part_set(part, playload_len);
	fprintf(fp2, "%d|%d|", count_packet++, playload_len);

	//printf("window1 is %d\n", window);
	window--;
	if (txthash >= min_hash) {
		min_index = chunk_num - 1;
		min_hash = txthash;
	}

	if (window == 0) {
		part_set(part, min_index);
		fprintf(fp2, "%ld ", min_index);
		
		//printf("%ld ", min_index);
		
		window = WINDOW;
		min_index = 0;
		//min_hash = ULLONG_MAX;
		min_hash = 0;
	}	

	for (i = chunk_num; i < playload_len; i++) {
		txthash = (txthash + Q - RM * playload[i - chunk_num] % Q) % Q;
		txthash = (txthash * R + playload[i]) % Q;
				
		//printf("window2 is %d and i is:%d\n", window, i);
		window--;
		if (txthash >= min_hash) {
			min_index = i;
			min_hash = txthash;
		}
		
		if (window == 0) {
			//printf("min_index is:%d, end is:%d, %d\n", min_index, part->index[part->end-1], part->end);
			
			if (part->end == 2) {
				part_set(part, min_index);
				fprintf(fp2, "%ld ", min_index);
			}
	
			if (part->end > 2 && min_index - part->index[part->end-1] >= THRESHOLD)
			{	
				//printf("2min_index is:%d, end is:%d, %d\n", min_index, part->index[part->end-1], part->end);
				part_set(part, min_index);
				fprintf(fp2, "%ld ", min_index);
			}
		
		
			//printf("%ld ", min_index);
			
			window = WINDOW;
			min_index = 0;
			//min_hash = ULLONG_MAX;
			min_hash = 0;
		}	
	}

	//if (min_index == 0 && min_hash == ULLONG_MAX && window == WINDOW) {
	if (min_index == 0 && min_hash == 0 && window == WINDOW) {
		fprintf(fp2, "\n");
	}	
	else {	
		if (window >= WINDOW/2) {
			//part_set(part, min_index);
			//fprintf(fp2, "%ld ", min_index);
			
			//printf("%ld ", min_index);
		}	
		
		window = WINDOW;
		min_index = 0;
		//min_hash = ULLONG_MAX;
		min_hash = 0;
		fprintf(fp2, "\n");
	}	
	//printf("window3 is %d\n", window);
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
			int index_start = 0;
			int index_end = remain_data->end-1;
			calculate_sha(remain_data->content, 0, remain_data->end-1, res);
			chunk_clean(remain_data);
			
			if (Hashmap_get(map, res) == NULL) {
				hashmap_key = keyvalue_push(key, res);
				check_mem(hashmap_key);
				Hashmap_set(map, hashmap_key, hashmap_key);
				//store_data(remain_data->content, 0, remain_data->end-1, res, 0);
				int k;/*
				fprintf(fp4, "%d|%d|%d|%d\n", count_packet-1, len, index_start, index_end);
				for (k = 0; k < 20; k++) {
					//printf("%02x:", digest[i]);
					fprintf(fp4, "%02x:", res[k]&0xff);
				}
				fprintf(fp4, "\n");
				for (k = 0; k < len; k++) {
					fprintf(fp4, "%02x:", remain_data->content[k]&0xff);
				}
				fprintf(fp4, "\n");*/
			}
			else {
				debug("find it");
				//store_data(remain_data->content, 0, 1, res, 1);
				save_len += len;

				int k;/*
				fprintf(fp1, "%d|%d|%d|%d\nsha|", count_packet-1, len, index_start, index_end);
				for (k = 0; k < 20; k++) {
					//printf("%02x:", digest[i]);
					fprintf(fp1, "%02x:", res[k]&0xff);
				}
				fprintf(fp1, "\n");
				for (k = 0; k < len; k++) {
					fprintf(fp1, "%02x:", remain_data->content[k]&0xff);
				}
				fprintf(fp1, "\n");*/
			}
			
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
				int index_start = 0;
				int index_end = 0;
				//printf("pp:%d\n", pp);
				if (i == 2) {
					chunk_merge(remain_data, playload, 0, pp);
					index_start = 0;
					index_end = remain_data->end-1;
					len = remain_data->end;
					//printf("what1\n");
					calculate_sha(remain_data->content, 0, remain_data->end-1, res);
					chunk_clean(remain_data);
				}
				else {
					index_start = part->index[i-1] + 1;
					index_end = part->index[i];
					len = part->index[i] - part->index[i-1];
					//printf("what2\n");
					calculate_sha(playload, part->index[i-1] + 1, part->index[i], res);
				}

				if (Hashmap_get(map, res) == NULL) {
					hashmap_key = keyvalue_push(key, res);
					check_mem(hashmap_key);
					Hashmap_set(map, hashmap_key, hashmap_key);
					debug("set it");
					
					int k;/*
					fprintf(fp4, "%d|%d|%d|%d\n", count_packet-1, len, index_start, index_end);
					for (k = 0; k < 20; k++) {
						//printf("%02x:", digest[i]);
						fprintf(fp4, "%02x:", res[k]&0xff);
					}
					fprintf(fp4, "\n");
					for (k = index_start; k <= index_end; k++) {
						if (i == 2)
							fprintf(fp4, "%02x:", remain_data->content[k]&0xff);
						else
							fprintf(fp4, "%02x:", playload[k]&0xff);
					}
					fprintf(fp4, "\n");*/
				}
				else {
					debug("find it");
					save_len += len;
					
					int k;/*
					fprintf(fp1, "%d|%d|%d|%d\nsha|", count_packet-1, len, index_start, index_end);
					for (k = 0; k < 20; k++) {
						//printf("%02x:", digest[i]);
						fprintf(fp1, "%02x:", res[k]&0xff);
					}
					fprintf(fp1, "\n");
					for (k = index_start; k <= index_end; k++) {
						if (i == 2)
							fprintf(fp1, "%02x:", remain_data->content[k]&0xff);
						else
							fprintf(fp1, "%02x:", playload[k]&0xff);
					}
					fprintf(fp1, "\n");*/
					
					//rm_data(playload, playload_len, i, pp, data); 
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
	char play_data[PACKET_LEN];
	int len;

	memset(play_data, '\0', PACKET_LEN);

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
						fprintf(fp1, "%d|%d|%u\n", count_packet, len, ntohl(tcph->seq));
						part_clean(part);
						//get the data parting point
						pack_calculate_part(play_data, len, Q, R, RM, zero_value, chunk_num);
						//pack_calculate_part2(play_data, len, Q, R, RM, zero_value, chunk_num);
						pack_SHA(play_data, len);
						sum_len += len;
						//store_char(play_data, len);
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
	fp3 = fopen("char_set", "w");
	fp4 = fopen("char_get", "w");
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

		//check_mem(buff);
		//printf("len is %d\n", ph.capture_len);

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
		Hashmap_destroy(map);
		fclose(fp);
		fclose(fp1);
		fclose(fp2);
		//fclose(fp2);
		fclose(fp3);
		fclose(fp4);
		free(buff);
		return ret;
}
