#include "../include/linux_parser.h"
#include "../include/format.h"
#include <dirent.h>
#include <string>
#include <unistd.h>
#include <vector>

using std::stof;
using std::string;
using std::to_string;
using std::vector;

// DONE: An example of how to read data from the filesystem
string LinuxParser::OperatingSystem() {
  string filename{kOSPath};
  std::ifstream stream(filename);

  if (!stream.is_open()) {
    perror(("error while opening the file" + filename).c_str());
  }

  string line{};
  string key{};
  string value{};
  while (std::getline(stream, line)) {
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

  if (stream.bad()) {
    perror(("error while reading file " + filename).c_str());
    stream.close();
  }
  return value;
}

// DONE: An example of how to read data from the filesystem
string LinuxParser::Kernel() {
  string filename{kProcDirectory + kVersionFilename};
  std::ifstream stream(filename);

  if (!stream.is_open()) {
    perror(("error while opening the file" + filename).c_str());
  }

  string line{};
  string os{};
  string kernel{};
  string version{};
  std::getline(stream, line);
  std::istringstream linestream(line);
  linestream >> os >> kernel >> version;

  if (stream.bad()) {
    perror(("error while reading file " + filename).c_str());
    stream.close();
  }
  return version;
}

// BONUS: Update this to use std::filesystem
vector<int> LinuxParser::Pids() {
  vector<int> pids;
  DIR *directory = opendir(kProcDirectory.c_str());
  struct dirent *file;
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
  string filename{kProcDirectory + kMeminfoFilename};
  std::ifstream stream(filename);

  if (!stream.is_open()) {
    perror(("error while opening the file" + filename).c_str());
  }

  string line{};
  float memTotal{};
  float memFree{};
  float buffers{};
  float cached{};
  while (std::getline(stream, line)) {
    std::istringstream buf(line);
    std::istream_iterator<string> beg(buf), end;
    vector<string> values(beg, end);
    float value = stof(values[1]);

    if (values[0] == "MemTotal:") {
      memTotal = value;
    }

    if (values[0] == "MemFree:") {
      memFree = value;
    }

    if (values[0] == "Buffers:") {
      buffers = value;
    }

    if (values[0] == "Cached:") {
      cached = value;
    }
  }

  if (stream.bad()) {
    perror(("error while reading file " + filename).c_str());
    stream.close();
  }
  float memUsed = memTotal - (memFree + buffers + cached);
  return memUsed / memTotal;
}

// TODO: Read and return the system uptime
long LinuxParser::UpTime() {
  string filename{kProcDirectory + kUptimeFilename};
  std::ifstream stream(filename);

  if (!stream.is_open()) {
    perror(("error while opening the file" + filename).c_str());
  }

  string systemUptime{};
  string idleProcessTime{};
  string line{};
  std::getline(stream, line);
  std::istringstream linestream(line);
  linestream >> systemUptime >> idleProcessTime;
  long uptime = stol(systemUptime);

  if (stream.bad()) {
    perror(("error while reading file " + filename).c_str());
    stream.close();
  }
  return uptime;
}

// TODO: Read and return the number of jiffies for the system
// Jiffies: usage is to store the number of ticks occurred since system start-up
long LinuxParser::Jiffies() {
  return LinuxParser::ActiveJiffies() + LinuxParser::IdleJiffies();
}

// TODO: Read and return the number of active jiffies for a PID
float LinuxParser::ActiveJiffies(int pid) {
  string filename{kProcDirectory + to_string(pid) + kStatFilename};
  std::ifstream stream(filename);

  if (!stream.is_open()) {
    perror(("error while opening the file " + filename).c_str());
  }

  string line{};
  float cpuUsage{};
  while (getline(stream, line)) {
    std::istringstream buf(line);
    std::istream_iterator<string> beg(buf), end;
    vector<string> values(beg, end);
    long uTime = stol(
        values[13]); // CPU time spent in user code, measured in clock ticks
    long sTime = stol(
        values[14]); // CPU time spent in kernel code, measured in clock ticks
    long cuTime = stol(values[15]); // Waited-for children's CPU time spent in
                                    // user code (in clock ticks)
    long csTime = stol(values[16]); // Waited-for children's CPU time spent in
                                    // kernel code (in clock ticks)
    long startTime = stol(
        values[21]); // Time when the process started, measured in clock ticks
    long hertz{sysconf(_SC_CLK_TCK)};

    long totalTime = uTime + sTime + cuTime + csTime;
    long seconds = UpTime() - (startTime / hertz);
    cpuUsage = ((1.0 * totalTime) / hertz) / seconds;
  }

  if (stream.bad()) {
    perror(("error while reading file " + filename).c_str());
    stream.close();
  }
  return cpuUsage;
}

// TODO: Read and return the number of active jiffies for the system
long LinuxParser::ActiveJiffies() {
  std::vector<string> values{LinuxParser::CpuUtilization()};
  values.erase(values.begin() + kIdle_, values.begin() + kIOwait_ + 1);

  long jiffies{};
  for (const auto &v : values) {
    jiffies = jiffies + stol(v);
  }
  return jiffies;
}

// TODO: Read and return the number of idle jiffies for the system
long LinuxParser::IdleJiffies() {
  std::vector<string> values{LinuxParser::CpuUtilization()};
  long idle{};
  long iowait{};
  idle = stol(values[kIdle_]);
  iowait = stol(values[kIOwait_]);
  return idle + iowait;
}

// TODO: Read and return CPU utilization
vector<string> LinuxParser::CpuUtilization() {
  string filename{kProcDirectory + kStatFilename};
  std::ifstream stream(filename);

  if (!stream.is_open()) {
    perror(("error while opening the file " + filename).c_str());
  }

  string line{};
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
  return std::vector<string>{"not found"};
}

// TODO: Read and return the total number of processes
int LinuxParser::TotalProcesses() {
  string filename{kProcDirectory + kStatFilename};
  std::ifstream stream(filename);

  if (!stream.is_open()) {
    perror(("error while opening the file " + filename).c_str());
  }

  int processes;
  string line;
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
  string filename{kProcDirectory + kStatFilename};
  std::ifstream stream(filename);

  if (!stream.is_open()) {
    perror(("error while opening the file " + filename).c_str());
  }

  string line;
  int running;
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
string LinuxParser::Command(int pid) {
  string filename{kProcDirectory + to_string(pid) + kCmdlineFilename};
  std::ifstream stream(filename);

  if (!stream.is_open()) {
    perror(("error while opening the file " + filename).c_str());
  }

  string line{};
  string command{};
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
string LinuxParser::Ram(int pid) {
  string filename{kProcDirectory + to_string(pid) + kStatusFilename};
  std::ifstream stream(filename);

  if (!stream.is_open()) {
    perror(("error while opening the file " + filename).c_str());
  }

  string line{};
  int ramMemory{};
  while (getline(stream, line)) {
    std::istringstream buf(line);
    std::istream_iterator<string> beg(buf), end;
    vector<string> values(beg, end);

    if (values[0] == "VmSize:") {
      ramMemory = stoi(values[1]) / 1000;
    }
  }

  if (stream.bad()) {
    perror(("error while reading file " + filename).c_str());
    stream.close();
  }

  return to_string(ramMemory);
}

// TODO: Read and return the user ID associated with a process
string LinuxParser::Uid(int pid) {
  string filename{kProcDirectory + to_string(pid) + kStatusFilename};
  std::ifstream stream(filename);

  if (!stream.is_open()) {
    perror(("error while opening the file " + filename).c_str());
  }

  string line{};
  string user{};
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

// TODO: Read and return the user associated with a process
string LinuxParser::User(int pid) {
  string filename{kPasswordPath};
  std::ifstream stream(filename);

  if (!stream.is_open()) {
    perror(("error while opening the file " + filename).c_str());
  }

  string line{};
  string result{};
  string uid{Uid(pid)};
  while (getline(stream, line)) {
    std::istringstream buf(line);
    std::istream_iterator<string> beg(buf), end;
    vector<string> values(beg, end);

    std::string p{":"};
    std::string stringToSearch{p + uid + p};
    int pPosition = values[0].find_first_of(stringToSearch);

    if (values[0].find_first_of(stringToSearch) != std::string::npos) {
      result = values[0].substr(0, pPosition);
      break;
    }
  }

  if (stream.bad()) {
    perror(("error while reading file " + filename).c_str());
    stream.close();
  }
  return result;
}

// TODO: Read and return the uptime of a process
long LinuxParser::UpTime(int pid) {
  string filename{kProcDirectory + to_string(pid) + kStatFilename};
  std::ifstream stream(filename);

  if (!stream.is_open()) {
    perror(("error while opening the file " + filename).c_str());
  }
  string line{};
  string startTime{};
  while (getline(stream, line)) {
    std::istringstream buf(line);
    std::istream_iterator<string> beg(buf), end;
    vector<string> values(beg, end);
    startTime = values[21];
  }

  if (stream.bad()) {
    perror(("error while reading file " + filename).c_str());
    stream.close();
  }
  return UpTime() - (stol(startTime) / sysconf(_SC_CLK_TCK));
}