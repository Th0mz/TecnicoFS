#include "timer.h"
#include <stdio.h>

void startTimer(Timer *timer) {
    gettimeofday(&timer->start, NULL);
} 

void stopTimer(Timer *timer) {
    gettimeofday(&timer->stop, NULL);

    /* Time in microseconds to seconds [1ms = 1/ 10**⁻6 s ] */
    double microseconds = (timer->stop.tv_usec - timer->start.tv_usec) / 1000000.0;
    
    /* Time in seconds */
    double seconds = timer->stop.tv_sec - timer->start.tv_sec;

    timer->elapsedTime = seconds + microseconds;
}