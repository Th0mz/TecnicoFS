#ifndef _TIMER_
#define _TIMER_

#include <sys/time.h>

/* Stop watch timer */
typedef struct timer {        
    struct timeval start, stop;
    double elapsedTime;
} Timer;


void startTimer(Timer *timer);
void stopTimer(Timer *timer);

#endif