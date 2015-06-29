/*
 * 1. make a hit by xor
 * 2. use the SHA-1
 */ 
#include <lcthw/checksum.h>

int
xor_checksum(char *buff, int buff_len)
{
        int i;
        unsigned char XOR;
        
		for (XOR = 0, i = 0; i < buff_len; i++) {
            XOR ^= (unsigned char)buff[i];
        }
   
        return (int)XOR;
}   


