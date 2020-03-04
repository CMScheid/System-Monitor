#include "../include/linux_parser.h"

#include <dirent.h>
#include <unistd.h>

#include <iostream>
#include <string>
#include <vector>

#include "../include/format.h"

using std::stof;
using std::string;
using std::to_string;
using std::vector;

// DONE: An example of how to read data from the filesystem
string LinuxParser::OperatingSystem() {
  string filename = kOSPath;
  string line;
  string key;
  string value;
  std::ifstream filestream(filename);
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
string LinuxParser::Kernel() {
  string filename = kProcDirectory + kVersionFilename;
  string os, kernel, version;
  string line;
  std::ifstream stream(filename);

  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> os >> kernel >> version;
  }
  return version;
}

// BONUS: Update this to use std::filesystem
vector<int> LinuxParser::Pids() {
  vector<int> pids;
  DIR* directory = opendir(kProcDirectory.c_str());
  struct dirent* file;
  while ((file = readdir(directory)) != nullptr) {
    // Is this a directory?

    if (file->d_type == DT_DIR) {
      // Is every character of the name a digit?
      string filename(file->d_name);

      if (std::all_of(filename.begin(), filename.end(), isdigit)) {
        int pid = stoi(filename);
        pids.push_back(pid);
      }
    }
  }
  closedir(directory);
  return pids;
}

// TODO: Read and return the system memory utilization
float LinuxParser::MemoryUtilization() {
  string filename = kProcDirectory + kMeminfoFilename;
  string total_memory, line;
  float value, mem_total, mem_free, mem_used, buffers, cached;

  std::ifstream stream(filename);

  if (!stream.is_open()) {
    perror(("error while opening the file" + filename).c_str());
  }

  while (std::getline(stream, line)) {
    std::istringstream buf(line);
    std::istream_iterator<string> beg(buf), end;
    vector<string> values(beg, end);
    value = stof(values[1]);

    if (values[0] == "MemTotal:") {
      mem_total = value;
    }

    if (values[0] == "MemFree:") {
      mem_free = value;
    }

    if (values[0] == "Buffers:") {
      buffers = value;
    }

    if (values[0] == "Cached:") {
      cached = value;
    }

    // buffers and cached
  }

  if (stream.bad()) {
    perror(("error while reading file " + filename).c_str());
    stream.close();
  }
  mem_used = mem_total - (mem_free + buffers + cached);

  return (mem_used / mem_total);
}

// TODO: Read and return the system uptime
long LinuxParser::UpTime() {
  string filename = kProcDirectory + kUptimeFilename;
  string system_uptime, idle_process_time;
  string line;
  std::ifstream stream(filename);
  long uptime;

  if (!stream.is_open()) {
    perror(("error while opening the file" + filename).c_str());
  }

  std::getline(stream, line);
  std::istringstream linestream(line);
  linestream >> system_uptime >> idle_process_time;
  uptime = stol(system_uptime);

  if (stream.bad()) {
    perror(("error while reading file " + filename).c_str());
    stream.close();
  }

  return uptime;
}

// TODO: Read and return the number of jiffies for the system
long LinuxParser::Jiffies() {
  return LinuxParser::ActiveJiffies() + LinuxParser::IdleJiffies();
}

// TODO: Read and return the number of active jiffies for a PID
// REMOVE: [[maybe_unused]] once you define the function
long LinuxParser::ActiveJiffies(int pid [[maybe_unused]]) { return 0; }

// TODO: Read and return the number of active jiffies for the system
long LinuxParser::ActiveJiffies() {
  std::vector<string> values{LinuxParser::CpuUtilization()};
  long jiffies{};
  values.erase(values.begin() + kIdle_, values.begin() + kIOwait_ + 1);

  for (const auto& v : values) {
    jiffies = jiffies + stol(v);
  }
  return jiffies;
}

// TODO: Read and return the number of idle jiffies for the system
long LinuxParser::IdleJiffies() {
  std::vector<string> values{LinuxParser::CpuUtilization()};
  long idle, iowait;

  idle = stol(values[kIdle_]);
  iowait = stol(values[kIOwait_]);

  return idle + iowait;
}

// TODO: Read and return CPU utilization
vector<string> LinuxParser::CpuUtilization() {
  string filename = kProcDirectory + kStatFilename;
  string line;
  std::vector<string> cpu{};

  std::ifstream stream(filename);

  if (!stream.is_open()) {
    perror(("error while opening the file " + filename).c_str());
  }

  while (getline(stream, line)) {
    std::istringstream buf(line);
    std::istream_iterator<string> beg(buf), end;
    vector<string> values(beg, end);

    if (values[0] == "cpu") {
      values.erase(values.begin());
      return values;
    }
  }

  if (stream.bad()) {
    perror(("error while reading file " + filename).c_str());
    stream.close();
  }
  return std::vector<string>{};
}

// TODO: Read and return the total number of processes
int LinuxParser::TotalProcesses() {
  string filename = kProcDirectory + kStatFilename;
  string line;
  int processes;

  std::ifstream stream(filename);

  if (!stream.is_open()) {
    perror(("error while opening the file " + filename).c_str());
  }

  while (getline(stream, line)) {
    std::istringstream buf(line);
    std::istream_iterator<string> beg(buf), end;
    vector<string> values(beg, end);

    if (values[0] == "processes") {
      processes = stoi(values[1]);
    }
  }

  if (stream.bad()) {
    perror(("error while reading file " + filename).c_str());
    stream.close();
  }
  return processes;
}

// TODO: Read and return the number of running processes
int LinuxParser::RunningProcesses() {
  string filename = kProcDirectory + kStatFilename;
  string line;
  int running;

  std::ifstream stream(filename);

  if (!stream.is_open()) {
    perror(("error while opening the file " + filename).c_str());
  }

  while (getline(stream, line)) {
    std::istringstream buf(line);
    std::istream_iterator<string> beg(buf), end;
    vector<string> values(beg, end);

    if (values[0] == "procs_running") {
      running = stoi(values[1]);
    }
  }

  if (stream.bad()) {
    perror(("error while reading file " + filename).c_str());
    stream.close();
  }
  return running;
}

// TODO: Read and return the command associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Command(int pid) {
  string filename{kProcDirectory + to_string(pid) + kCmdlineFilename};
  string line{}, command{};

  std::ifstream stream(filename);

  if (!stream.is_open()) {
    perror(("error while opening the file " + filename).c_str());
  }

  while (std::getline(stream, line)) {
    std::istringstream buf(line);
    std::istream_iterator<string> beg(buf), end;
    vector<string> values(beg, end);
    command = values[0];
  }

  if (stream.bad()) {
    perror(("error while reading file " + filename).c_str());
    stream.close();
  }

  return command;
}

// TODO: Read and return the memory used by a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Ram(int pid [[maybe_unused]]) { return string(); }

// TODO: Read and return the user ID associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Uid(int pid) {
  string filename = kProcDirectory + to_string(pid) + "/";
  string line, uid;

  std::ifstream stream(filename);

  if (!stream.is_open()) {
    perror(("error while opening the file " + filename).c_str());
  }

  while (getline(stream, line)) {
    std::istringstream buf(line);
    std::istream_iterator<string> beg(buf), end;
    vector<string> values(beg, end);

    if (values[0] == "Uid:") {
      uid = values[1];
    }
  }

  if (stream.bad()) {
    perror(("error while reading file " + filename).c_str());
    stream.close();
  }
  return uid;
}

// TODO: Read and return the user associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::User(int pid) {
  string filename = kProcDirectory + to_string(pid) + kStatusFilename;
  string line, user;

  std::ifstream stream(filename);

  if (!stream.is_open()) {
    perror(("error while opening the file " + filename).c_str());
  }

  while (getline(stream, line)) {
    std::istringstream buf(line);
    std::istream_iterator<string> beg(buf), end;
    vector<string> values(beg, end);

    if (values[0] == "Uid:") {
      user = values[1];
    }
  }

  if (stream.bad()) {
    perror(("error while reading file " + filename).c_str());
    stream.close();
  }
  return user;
}

// TODO: Read and return the uptime of a process
// REMOVE: [[maybe_unused]] once you define the function
long LinuxParser::UpTime(int pid) {
  string filename = kProcDirectory + to_string(pid) + kStatFilename;
  string line, startTime;

  std::ifstream stream(filename);

  if (!stream.is_open()) {
    perror(("error while opening the file " + filename).c_str());
  }

  while (getline(stream, line)) {
    std::istringstream buf(line);
    std::istream_iterator<string> beg(buf), end;
    vector<string> values(beg, end);
    startTime = values[22];
  }

  if (stream.bad()) {
    perror(("error while reading file " + filename).c_str());
    stream.close();
  }
 
  return stol(startTime) / sysconf(_SC_CLK_TCK);;
}
