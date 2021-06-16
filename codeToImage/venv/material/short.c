//
//  main.c
//  Test
//
//  Created by 加藤祥真 on 2021/04/21.
//

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>


int main(int argc, const char * argv[]) {
    
    double startTime, endTime;
    double totalTime = 0.0, setTime = 0.0;
    
    /* タイマー開始（ミリ秒単位） */
    startTime = clock() / CLOCKS_PER_SEC ;
    
    int prev = 0;
    int second = 0.5 * 1000000;
    while(1){
        printf("=o\n");
        usleep(second);
        system("clear");
        printf("==o\n");
        usleep(second);
        system("clear");
        printf("===o\n");
        usleep(second);
        system("clear");
        printf("====o\n");
        usleep(second);
        system("clear");
        printf("=====o\n");
        usleep(second);
        system("clear");
        printf("======o\n");
        usleep(second);
        system("clear");
    }
    return 0;
}

