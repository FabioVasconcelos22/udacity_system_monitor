#include "process.h"

#include <string>
#include <vector>

#include "linux_parser.h"

using std::string;
using std::to_string;
using std::vector;

int Process::Pid() { return _pid; }

float Process::CpuUtilization() {
  auto active = static_cast<float>(LinuxParser::ActiveJiffies(_pid));
  auto total = static_cast<float>(LinuxParser::Jiffies());
  return active / total;
}

string Process::Command() { return LinuxParser::Command(_pid); }

string Process::Ram() { return LinuxParser::Ram(_pid); }

string Process::User() { return LinuxParser::User(_pid); }

long int Process::UpTime() { return LinuxParser::UpTime(_pid); }

bool Process::operator<(Process const& a) const { return _pid < a._pid; }

Process::Process(int pid) : _pid(pid) {}
