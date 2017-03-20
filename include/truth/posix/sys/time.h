#pragma once

#include <stdint.h>


typedef uint64_t time_t;
typedef uint64_t suseconds_t;


struct timeval {
    time_t       tv_sec;   /* seconds since Jan. 1, 1970 */
    suseconds_t  tv_usec;  /* and microseconds */
};


struct timezone {
    int     tz_minuteswest; /* of Greenwich */
    int     tz_dsttime;     /* type of dst correction to apply */
};


int gettimeofday(struct timeval *restrict tp, void *restrict tzp);

int settimeofday(const struct timeval *tp, const struct timezone *tzp);
