
#pragma once

#include <chrono>

typedef std::chrono::high_resolution_clock::time_point timestamp;

timestamp debug_time();

double debug_time_diff(timestamp start, timestamp end);
