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
    _freq = 1.0 / freq.QuadPart;
    _time0 = _get_absolute_time();
}

double get_elapsed_time()
{
    return _get_absolute_time() - _time0;
}

