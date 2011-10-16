//#include <intrin.h>
//
//unsigned __int64 rdtsc(void)
//{
//  return __rdtsc();
//}

// Always define NOMINMAX before including windows.h to prevent
// strange errors wherever min() and max() are used in any context.
#define NOMINMAX
#include <windows.h>
#include "timer.h"

static double _freq;
static double _time0;

static double _get_absolute_time()
{
    LARGE_INTEGER time;
    QueryPerformanceCounter(&time);
    return time.QuadPart * _freq;
}

void timer_init()
{
    LARGE_INTEGER freq;
    QueryPerformanceFrequency(&freq);
    _freq = 1.0f / freq.QuadPart;
    _time0 = _get_absolute_time();
}

double get_elapsed_time()
{
    return _get_absolute_time() - _time0;
}

#if 0
#include <sys/stat.h>

#include <sys/time.h>
#include <sys/types.h



class ZTimer
{
    struct timeval t1, t2;
public:
    ZTimer()      { gettimeofday(&t1 ,0); t2 = t1; }
    void reset()  { gettimeofday(&t1 ,0); t2 = t1;}
    int elapsed() { return ((t2.tv_sec - t1.tv_sec) * 1000) + ((t2.tv_usec - t1.tv_usec) / 1000); }
    int split()   { gettimeofday(&t2,0); return elapsed(); }
};

#endif




