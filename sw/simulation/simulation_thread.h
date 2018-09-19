#ifndef SIMULATION_THREAD_H
#define SIMULATION_THREAD_H

#include <numeric>
#include <functional>
#include <cctype>
#include <algorithm>
#include <thread>

#include "main.h"
#include "randomgenerator.h"
#include "omniscient_observer.h"
#include "terminalinfo.h"
#include "agent_thread.h"
#include "drawingparams.h"

bool simulation_running = false;

void run_simulation()
{
  if (!simulation_running) {
    terminalinfo ti;
    ti.info_msg("Simulation started.");
    simulation_running = true;
  }

  if (!paused) {
    int t_wait = (int)1000000.0 * (1.0 / (param->simulation_updatefreq() * param->simulation_realtimefactor()));
    this_thread::sleep_for(chrono::microseconds(t_wait));
    simulation_time = t_wait;
    simtime_seconds += param->simulation_realtimefactor() * simulation_time / 1000000.0;
  }
}

/* Calculate the mean of a vector element */
// TODO: Move to math
float vector_mean(const vector<float> &v)
{
  float sum = std::accumulate(v.begin(), v.end(), 0.0);
  return sum / v.size();
}

/* Calculate the standard deviation of a vector element */
// TODO: Move to math
float vector_std(const vector<float> &v)
{
  vector<double> diff(v.size());
  std::transform(v.begin(), v.end(), diff.begin(), std::bind2nd(std::minus<double>(), vector_mean(v))
                );
  double sq_sum = std::inner_product(diff.begin(), diff.end(), diff.begin(), 0.0);
  return sqrt(sq_sum / v.size());
}

/* Generate a random vector with zero mean */
// TODO: Move to math
vector<float> generate_random_vector_zeromean(const int &length)
{
  // Generate the random vector
  vector<float> v(length, 0);
  for (uint8_t i = 0; i < length; i++) {
    v[i] = getrand_float(-0.5, 0.5);
  }

  // Adjust to zero mean
  vector<float> temp = v;
  for (uint8_t i = 0; i < length; i++)
  {
    v[i] = v[i] - vector_mean(temp);
  }

  return v;
}

void get_number_of_agents(int argc, char *argv[])
{
  // Extract number of agents from the argument
  terminalinfo ti;
  if (argc <= 1) {
    ti.debug_msg("Please specify the number of agents.\n\n");
    exit(1);
  } else {
    nagents = stoi(argv[1]);
  }
}

void main_simulation_thread(int argc, char *argv[])
{
  get_number_of_agents(argc, argv);

  // Generate random initial positions with zero mean
  randomgen_init();
  srand(time(NULL));
  vector<float> x0 = generate_random_vector_zeromean(nagents);
  vector<float> y0 = generate_random_vector_zeromean(nagents);

  // Generate the agent models
  for (uint8_t ID = 0; ID < nagents; ID++) {
    vector<float> states = { x0[ID], y0[ID], 0.0, 0.0, 0.0, 0.0 }; // Initial positions/states
    s.push_back(new AGENT(ID, states, 1.0 / param->simulation_updatefreq()));
  }

  // Launch agent threads to simulate each agent independetly
  for (uint8_t ID = 0; ID < nagents; ID++) {
    thread agent(start_agent_simulation, ID);
    agent.detach();
  }

  while (program_running) {
    run_simulation();
  };

}
#endif /*SIMULATION_THREAD_H*/
