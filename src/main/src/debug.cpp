
#include "debug.h"

timestamp debug_time()
{
    return std::chrono::high_resolution_clock::now();
}

double debug_time_diff(timestamp start, timestamp end)
{
    using seconds = std::chrono::duration<double>;
    return std::chrono::duration_cast<seconds>(end - start).count();
}
