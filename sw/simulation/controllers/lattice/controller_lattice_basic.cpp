#include "controller_lattice_basic.h"
#include "agent.h"
#include "main.h"
#include "randomgenerator.h"
#include "trigonometry.h"

float Controller_Lattice_Basic::f_attraction(const float &u, const float &b_eq)
{
  float w;
  float ddes2 = sqrt(pow(_ddes_x, 2.0) + pow(_ddes_y, 2.0));
  float tol = 0.1;

  // TODO: move general attraction equation to controller
  // if (abs(b_eq - atan(_ddes_y / _ddes_x)) < tol || // +- 45 deg
  // abs(b_eq - M_PI / 2.0 - atan(_ddes_x / _ddes_y)) < tol) { // +- 135 deg
  // w = log((ddes2 / _kr - 1) / exp(-_ka * ddes2)) / _ka;
  // } else if (abs(b_eq) - deg2rad(180) < tol) { // 0,180
  //   w = log((_ddes_y / _kr - 1) / exp(-_ka * _ddes_y)) / _ka;
  // } else if (abs(b_eq - deg2rad(90)) < tol) {
    w = log((_ddes_x / _kr - 1) / exp(-_ka * _ddes_x)) / _ka;
  // }
  
  return 1 / (1 + exp(-_ka * (u - w)));
}

// TODO -- move general sum to controller
float Controller_Lattice_Basic::get_attraction_velocity(const float &u, const float &b_eq)
{
  return f_attraction(u, b_eq) + f_repulsion(u);
}

void Controller_Lattice_Basic::attractionmotion(const float &v_r, const float &v_b, float &v_x, float &v_y)
{
  v_x += v_r * cos(v_b);
  v_y += v_r * sin(v_b);
}

void Controller_Lattice_Basic::latticemotion(const float &v_r, const float &v_adj, const float &v_b, const float &bdes, float &v_x, float &v_y)
{
  attractionmotion(v_r, v_b, v_x, v_y);
  // v_x += -0.1 * cos(bdes * 2.0 - v_b);
  // v_y += -0.1 * sin(bdes * 2.0 - v_b);
}

void Controller_Lattice_Basic::actionmotion(const int &selected_action, float &v_x, float &v_y)
{
  float actionspace_y[8] = {0,       sqrt(_ddes_y) / 2, _ddes_y, sqrt(_ddes_y) / 2, 0,        -sqrt(_ddes_y) / 2, -_ddes_y, -sqrt(_ddes_y) / 2};
  float actionspace_x[8] = {_ddes_x, sqrt(_ddes_x) / 2, 0,      -sqrt(_ddes_x) / 2, -_ddes_x, -sqrt(_ddes_x) / 2, 0,         sqrt(_ddes_x) / 2};
  v_x = _v_adj * actionspace_x[selected_action];
  v_y = _v_adj * actionspace_y[selected_action];
}

bool Controller_Lattice_Basic::check_motion(const vector<int> &state_ID)
{
  bool canImove = true;
  for (uint8_t i = 0; i < state_ID.size(); i++)
  {
    if (o->see_if_moving(state_ID[i]))
    {
      // Somebody nearby is already moving
      canImove = false;
    }
  }
  return canImove;
}

void Controller_Lattice_Basic::get_lattice_motion(const int &ID, const int &state_ID, float &v_x, float &v_y)
{
  float v_b, b_eq, v_r;
  v_b = wrapToPi_f(o->request_bearing(ID, state_ID));
  b_eq = t.get_preferred_bearing(beta_des, v_b);
  v_r = get_attraction_velocity(o->request_distance(ID, state_ID), b_eq);
  latticemotion(v_r, _v_adj, v_b, b_eq, v_x, v_y);
}

void Controller_Lattice_Basic::get_lattice_motion_all(const int &ID, const vector<int> &state_ID, const vector<int> &closest, float &v_x, float &v_y)
{
  if (!state_ID.empty())
  {
    if (o->request_distance(ID, closest[0]) < d_safe) // If all are further than the safety distance
    {
      for (size_t i = 0; i < state_ID.size(); i++)
      {
        get_lattice_motion(ID, state_ID[i], v_x, v_y);
      }
      v_x = v_x / (float)state_ID.size();
      v_y = v_y / (float)state_ID.size();
      cout << (int)ID << " " << (float)state_ID.size() << endl;
    }
    else // Focus on the closest one only
    {
      get_lattice_motion(ID, state_ID[0], v_x, v_y);
      cout << (int)ID << " " << (float)state_ID.size() << endl;
    }
  }
}
