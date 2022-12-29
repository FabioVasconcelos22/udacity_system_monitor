#include "processor.h"

#include "linux_parser.h"

float Processor::Utilization() {
  auto active = static_cast<float>(LinuxParser::ActiveJiffies());
  auto total = static_cast<float>(LinuxParser::Jiffies());
  return active / total;
}