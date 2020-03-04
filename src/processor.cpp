#include "../include/processor.h"

#include <iostream>

#include "../include/linux_parser.h"

// TODO: Return the aggregate CPU utilization
float Processor::Utilization() {

  return (LinuxParser::ActiveJiffies() * 1.0) / (LinuxParser::Jiffies() * 1.0); 
}