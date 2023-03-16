#include "linux_parser.h"

#include <dirent.h>
#include <unistd.h>

#include <iostream>
#include <string>
#include <vector>

using std::stof;
using std::to_string;

// DONE: An example of how to read data from the filesystem
std::string LinuxParser::OperatingSystem() {
  std::string line;
  std::string key;
  std::string value;
  std::ifstream filestream(kOSPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ' ', '_');
      std::replace(line.begin(), line.end(), '=', ' ');
      std::replace(line.begin(), line.end(), '"', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "PRETTY_NAME") {
          std::replace(value.begin(), value.end(), '_', ' ');
          return value;
        }
      }
    }
  }
  return value;
}

// DONE: An example of how to read data from the filesystem
std::string LinuxParser::Kernel() {
  std::string os, version, kernel;
  std::string line;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> os >> version >> kernel;
  }
  return kernel;
}

// TODO BONUS: Update this to use std::filesystem
std::vector<int> LinuxParser::Pids() {
  std::vector<int> pids;
  DIR* directory = opendir(kProcDirectory.c_str());
  struct dirent* file;
  while ((file = readdir(directory)) != nullptr) {
    // Is this a directory?
    if (file->d_type == DT_DIR) {
      // Is every character of the name a digit?
      std::string filename(file->d_name);
      if (std::all_of(filename.begin(), filename.end(), isdigit)) {
        int pid = stoi(filename);
        pids.push_back(pid);
      }
    }
  }
  closedir(directory);
  return pids;
}

float LinuxParser::MemoryUtilization() {
  std::string line;
  float memTotal = 0, memFree = 0, buffers = 0;

  std::ifstream filestream(kProcDirectory + kMeminfoFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      std::string key{};
      float value = 0;

      linestream >> key >> value;

      if (key == "MemTotal:") {
        memTotal = value;
      } else if (key == "MemFree:") {
        memFree = value;
      } else if (key == "Buffers:") {
        buffers = value;
      }
    }
  }
  return 1.0 - (memFree / (memTotal - buffers));
}

long LinuxParser::UpTime() {
  long upTime;
  std::string line;

  std::ifstream filestream(kProcDirectory + kUptimeFilename);
  if (filestream.is_open()) {
    std::getline(filestream, line);
    std::istringstream linestream(line);
    linestream >> upTime;
  }

  return upTime;
}

long LinuxParser::Jiffies() {
  return (LinuxParser::ActiveJiffies() + LinuxParser::IdleJiffies());
}

long LinuxParser::ActiveJiffies(int pid) {
  std::string line;
  long utime, stime, cutime, cstime;

  std::ifstream stream(kProcDirectory + std::to_string(pid) + kStatFilename);
  if (!stream.is_open()) {
    return {};
  }

  std::getline(stream, line);
  std::istringstream linestream(line);

  // Ignore the first 13 data elements
  for (auto i = 0; i < 13; ++i) {
    std::string offset;
    linestream >> offset;
  }

  linestream >> utime >> stime >> cutime >> cstime;

  return (utime + stime + cutime + cstime);
}

long LinuxParser::ActiveJiffies() {
  std::vector<std::string> cpuStat = CpuUtilization();
  long user = std::stol(cpuStat.at(kUser_));
  long nice = std::stol(cpuStat.at(kNice_));
  long system = std::stol(cpuStat.at(kSystem_));
  long idle = std::stol(cpuStat.at(kIdle_));
  long ioWait = std::stol(cpuStat.at(kIOwait_));
  long irq = std::stol(cpuStat.at(kIRQ_));
  long softirq = std::stol(cpuStat.at(kSoftIRQ_));
  long steal = std::stol(cpuStat.at(kSteal_));
  long guest = std::stol(cpuStat.at(kGuest_));
  long guestNice = std::stol(cpuStat.at(kGuestNice_));

  return user + nice + system + irq + softirq + steal + guest + guestNice;
}

long LinuxParser::IdleJiffies() {
  std::vector<std::string> cpuStat = CpuUtilization();
  long idle = stol(cpuStat.at(kIdle_));
  long ioWait = stol(cpuStat.at(kIOwait_));
  return idle + ioWait;
}

std::vector<std::string> LinuxParser::CpuUtilization() {
  std::string line;
  std::string header, user, nice, system, idle, iowait, irq, softIrq, steal,
      guest, guestNice;

  std::ifstream stream(kProcDirectory + kStatFilename);
  if (!stream.is_open()) {
    return {};
  }

  // Get general cpu utilization (1st line)
  std::getline(stream, line);

  std::istringstream linestream(line);
  linestream >> header >> user >> nice >> system >> idle >> iowait >> irq >>
      softIrq >> steal >> guest >> guestNice;

  std::vector<std::string> result{user, nice,    system, idle,  iowait,
                                  irq,  softIrq, steal,  guest, guestNice};
  return result;
}

int LinuxParser::TotalProcesses() {
  std::string line, key;
  int value = 0;

  std::ifstream stream(kProcDirectory + kStatFilename);
  if (!stream.is_open()) {
    return {};
  }

  while (std::getline(stream, line)) {
    std::istringstream linestream(line);
    linestream >> key >> value;

    if (key == "processes") {
      return value;
    }
  }
  return value;
}

int LinuxParser::RunningProcesses() {
  std::string line, key;
  int value = 0;

  std::ifstream stream(kProcDirectory + kStatFilename);
  if (!stream.is_open()) {
    return {};
  }

  while (std::getline(stream, line)) {
    std::istringstream linestream(line);
    linestream >> key >> value;

    if (key == "procs_running") {
      return value;
    }
  }
  return value;
}

std::string LinuxParser::Command(int pid) {
  std::ifstream stream(kProcDirectory + std::to_string(pid) + kCmdlineFilename);
  if (!stream.is_open()) {
    return {};
  }

  std::string line;
  std::getline(stream, line);
  return line;
}

std::string LinuxParser::Ram(int pid) {
  std::string line, key, value;

  std::ifstream stream(kProcDirectory + std::to_string(pid) + kStatusFilename);
  if (!stream.is_open()) {
    return {};
  }

  while (std::getline(stream, line)) {
    std::istringstream linestream(line);

    linestream >> key >> value;

    if (key == "VmSize:") {
      int MB = stoi(value) / 1000;
      return std::to_string(MB);
    }
  }
  return {'0'};
}

std::string LinuxParser::Uid(int pid) {
  std::string line, key, value;

  std::ifstream stream(kProcDirectory + std::to_string(pid) + kStatusFilename);
  if (!stream.is_open()) {
    return {};
  }

  while (std::getline(stream, line)) {
    std::istringstream linestream(line);

    linestream >> key >> value;

    if (key == "Uid:") {
      return value;
    }
  }
  return {};
}

std::string LinuxParser::User(int pid) {
  std::string uid = Uid(pid);
  std::string line{};
  std::string user, password, userId;

  std::ifstream stream(kPasswordPath);
  if (!stream.is_open()) {
    return {};
  }

  while (std::getline(stream, line)) {
    std::replace(line.begin(), line.end(), ':', ' ');

    std::istringstream linestream(line);
    linestream >> user >> password >> userId;

    if (userId == uid) {
      return user;
    }
  }
  return {};
}

long LinuxParser::UpTime(int pid) {
  std::string line{};
  std::string starttime;

  std::ifstream stream(kProcDirectory + std::to_string(pid) + kStatFilename);
  if (!stream.is_open()) {
    return {};
  }

  std::getline(stream, line);
  std::istringstream linestream(line);

  // Ignore more 21 elements
  for (auto i = 0; i < 21; ++i) {
    std::string offset;
    linestream >> offset;
  }

  linestream >> starttime;

  return (UpTime() - (stoi(starttime) / sysconf(_SC_CLK_TCK)));
}

template <typename T>
T LinuxParser::findValueByKey(std::string const &keyFilter, std::string const &filename) {
    std::string line, key;
    T value;

    std::ifstream stream(kProcDirectory + filename);
    if (stream.is_open()) {
        while (std::getline(stream, line)) {
            std::istringstream linestream(line);
            while (linestream >> key >> value) {
                if (key == keyFilter) {
                    return value;
                }
            }
        }
    }
    return value;
};

template <typename T>
T LinuxParser::getValueOfFile(std::string const &filename) {
    std::string line;
    T value;

    std::ifstream stream(kProcDirectory + filename);
    if (stream.is_open()) {
        std::getline(stream, line);
        std::istringstream linestream(line);
        linestream >> value;
    }
    return value;
};