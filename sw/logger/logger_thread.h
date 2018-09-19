#ifndef LOGGER_THREAD_H
#define LOGGER_THREAD_H

#include <fstream>
#include <numeric>
#include <string>
#include <functional>
#include <cctype>
#include <algorithm>
#include <ctime>

#include "main.h"
#include "txtwrite.h"

using namespace std;

bool logger_running = false;

void run_logger(ofstream &logfile, string filename)
{
  static txtwrite writer;
  if (!logger_running) {
    terminalinfo ti;
    ti.info_msg("Logger started.");
    writer.setfilename(filename); // Set the filename
    logger_running = true; // Set logger running to true
  }

  // Write the logfile
  mtx.lock();
  if (!paused && simtime_seconds > 1.0) {
    writer.txtwrite_state(logfile);
    writer.txtwrite_summary(logfile);
  }
  mtx.unlock();

  // Wait
  int t_wait = (int)1000000.0 / (param->logger_updatefreq() * param->simulation_realtimefactor());
  this_thread::sleep_for(chrono::microseconds(t_wait));
}

/* Get current date/time, format is YYYY-MM-DD-hh:mm:ss */
// Visit http://en.cppreference.com/w/cpp/chrono/c/strftime
// for more information about date/time format
const std::string currentDateTime()
{
  time_t now = time(0); // Read in the time
  struct tm tstruct;
  char buf[80]; // Buffer
  tstruct = *localtime(&now);

  // Put the time on a string using the buffer
  strftime(buf, sizeof(buf), "%Y-%m-%d-%X", &tstruct);
  return buf;
}

void main_logger_thread()
{
  // Log filename
  string filename = "logs/log_" + currentDateTime() + ".txt";

  // Open the logfile for writing
  ofstream logfile;
  logfile.open(filename.c_str());

  // Initiate the logger
  while (program_running) {
    run_logger(logfile, filename);
  };
}

#endif /*LOGGER_THREAD_H*/
