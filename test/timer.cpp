/* This project is licensed under the terms of the Creative Commons CC BY-NC-ND 4.0 license. */

#include <algorithm>
#include <functional>
#include <iostream>
#include <iterator>
#include <numeric>
#include <random>
#include <vector>

#include "timer.hpp"

int main(void)
{
  Timer timer;

  // start time measurement
  uint64_t start_milliseconds = timer.getTimeMilliseconds();
  uint64_t start_microseconds = timer.getTimeMicroseconds();
  uint64_t start_nanoseconds  = timer.getTimeNanoseconds();

  // do something that takes more than a millisecond
  std::random_device rnd_device;
  std::mt19937 mersenne_engine {rnd_device()};
  std::uniform_int_distribution<int64_t> dist {1, 52};
  auto gen = [&dist, &mersenne_engine](){ return dist(mersenne_engine); };
  std::vector<int64_t> random_numbers(5000000);
  generate(begin(random_numbers), end(random_numbers), gen);
  auto sum = std::accumulate(begin(random_numbers), end(random_numbers), 0);
  std::cout << "The sum is " << sum << "." << std::endl;

  // stop time measurement
  uint64_t stop_milliseconds = timer.getTimeMilliseconds();
  uint64_t stop_microseconds = timer.getTimeMicroseconds();
  uint64_t stop_nanoseconds  = timer.getTimeNanoseconds();

  std::cout << "Milliseconds: " << start_milliseconds << ", " << stop_milliseconds << std::endl;
  std::cout << "Microseconds: " << start_microseconds << ", " << stop_microseconds << std::endl;
  std::cout << "Nanoseconds : " << start_nanoseconds  << ", " << stop_nanoseconds  << std::endl;

  if (stop_milliseconds <= start_milliseconds ||
      stop_microseconds <= start_microseconds ||
      stop_nanoseconds <= start_nanoseconds) { return 1; }

  return 0;
}
