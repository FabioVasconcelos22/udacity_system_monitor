#include <string>
#include <iomanip>

#include "format.h"

using std::string;

string Format::ElapsedTime(long seconds) {
  long s = seconds;

  long h = (int) s / 3600;
  s = s - h * 3600;
  long m = s / 60;
  s = s - m * 60;

  ToStringFormat;
  std::stringstream output;
  return {
      ToStringFormat(h) + ":" +
      ToStringFormat(m) + ":" +
      ToStringFormat(s)
  };
}

std::string Format::ToStringFormat (long value) {
  if (value < 10) {
    return {"0" + std::to_string(value)};
  } else {
    return std::to_string(value);
  }
}
