#include <windows.h>
#include "timer.h"

class MyTimer {
    double _freq;
    double _time0;

    double get_absolute_time() const {
        LARGE_INTEGER time;
        QueryPerformanceCounter(&time);
        return time.QuadPart * _freq;
    }

public:
     MyTimer() {
        LARGE_INTEGER freq;
        QueryPerformanceFrequency(&freq);
        _freq = 1.0 / freq.QuadPart;
        reset();
    }

    void reset() {
        _time0 = get_absolute_time();
    }
     
    double get_time() const {
        return get_absolute_time() - _time0;
    }
};

static MyTimer _timer;

void
reset_elapsed_time() {
    _timer.reset();
}

double
get_elapsed_time() {
    return _timer.get_time();
}

