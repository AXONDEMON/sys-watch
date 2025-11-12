#include <time.h>
#include <unistd.h>
#include "syswatch.h"

unsigned long long now_ms(void){
    struct timespec ts; clock_gettime(CLOCK_REALTIME, &ts);
    return (unsigned long long)ts.tv_sec * 1000ULL + ts.tv_nsec/1000000ULL;
}

int safe_sleep_ms(int ms){
    struct timespec req = { .tv_sec = ms/1000, .tv_nsec = (ms%1000)*1000000L };
    while (nanosleep(&req, &req) == -1) continue;
    return 0;
}