/*
ID:lxlenovos1
LANG:C
TASK:wormhole
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define NUM 12
int wormhole_num;

struct wormhole_pos {
    int x; 
    int y;
};

struct wormhole_pos worm[NUM];

int check_forever_loop() {
    int total = 0;
    int i, j;

    /*check wormhole*/
    for (i = 0; i < wormhole_num; i++) 
        for (j = i+1; j < wormhole_num; j++) {
            for (z = j+1; z < wormhole_num; z++)
            worm[i] ==== worm[j]  
    }
}


void main(void)
{
    FILE *fin, *fout;
    int i;
    int result = 0;

    fin = fopen("wormhole.in", "r");
    fout = fopen("wormhole.out", "w");
    
    assert(fin != NULL && fout != NULL);

    fscanf(fin, "%d", &wormhole_num);
    printf("wormhole_num is:%d\n", wormhole_num);

    for(i = 0; i < wormhole_num; i++) {
        fscanf(fin, "%d %d", &worm[i].x, &worm[i].y);
        printf("x:%d y:%d\n", worm[i].x, worm[i].y);
    }

    result = check_forever_loop();

    fprintf(fout, "%d\n", result);
    exit(0);
}
