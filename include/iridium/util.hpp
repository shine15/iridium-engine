/* Copyright 2020 Iridium. All Rights Reserved.
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at
    http://www.apache.org/licenses/LICENSE-2.0
Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/

#ifndef INCLUDE_IRIDIUM_UTIL_HPP_
#define INCLUDE_IRIDIUM_UTIL_HPP_

#include <iostream>
#include <iomanip>
#include <ctime>
#include <sstream>
#include <string>
#include <math.h>

std::string TimeToLocalTimeString(std::time_t rawTime);

template<typename T>
std::string To_String_With_Precision(const T value, const int n) {
  std::ostringstream out;
  out.precision(n);
  out << std::fixed << value;
  return out.str();
}

#endif  // INCLUDE_IRIDIUM_UTIL_HPP_
