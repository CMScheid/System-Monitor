#include "../include/format.h"
#include <iostream>
#include <iomanip>
#include <string>

using std::string;

// TODO: Complete this helper function
// INPUT: Long int measuring seconds
// OUTPUT: HH:MM:SS
// REMOVE: [[maybe_unused]] once you define the function
string Format::ElapsedTime(long seconds) {
  const int ONE_HOUR = (60 * 60);
  const int ONE_MINUTE = 60;

  int hours{static_cast<int>(seconds / 3600)};  
  seconds = seconds % ONE_HOUR;                    
  int minutes = seconds / ONE_MINUTE;
  std::string result{};
  std::ostringstream is(result);
  is << std::setfill('0') << std::setw(2) << hours << ":";
  is << std::setfill('0') << std::setw(2) << minutes << ":";
  is << std::setfill('0') << std::setw(2) << seconds % 60;
  return is.str(); 
}