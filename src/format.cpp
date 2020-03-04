#include "../include/format.h"
#include <iostream>

#include <string>

using std::string;

// TODO: Complete this helper function
// INPUT: Long int measuring seconds
// OUTPUT: HH:MM:SS
// REMOVE: [[maybe_unused]] once you define the function
string Format::ElapsedTime(long time_target) {
  const int ONE_HOUR = (60 * 60);
  const int ONE_MINUTE = 60;

  long hour, min, sec;

  hour = time_target / ONE_HOUR;

  time_target = time_target % ONE_HOUR;

  min = time_target / ONE_MINUTE;

  time_target = time_target % ONE_MINUTE;

  sec = time_target;

  string hh = std::to_string(hour);
  string mm = std::to_string(min);
  string ss = std::to_string(sec);

  return hh + ":" + mm + ":" + ss;
}