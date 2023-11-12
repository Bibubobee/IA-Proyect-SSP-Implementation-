#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <map>
#include "global_variables.hpp"
using namespace std;

int h; //Horizon Length
int n; // Total employees
int CT; // Total Turns

vector<int> L_t; // Length of turns
vector<vector<bool>> R_t_k; // Consecutive turns rules
map<string, string> R_t_k_aux; // Aux to help convertion
vector<vector<int>> T_i_t; // Max turns to be assigned per worker i
map<string, string> T_i_t_aux; // Aux to help convertion
vector<vector<int>> Turns_left_to_i; // Works alongside T_i_t for greedy solution
vector<vector<bool>> LO_i_d; // Mandatory days-off
vector<vector<vector<int>>> PNAT_i_d_t; // Non-Assignation Penalty
vector<vector<vector<int>>> PAT_i_d_t; // Assignation Penalty
vector<vector<int>> S_d_t; // Employees required for day t
vector<vector<int>> OS_d_t; // Over Staffing penalty
vector<vector<int>> US_d_t; // Under Staffing penalty
vector<int> MA_i; // Max minutes to work in cycle
vector<int> MI_i; // Min minutes to work in cycle
vector<int> CMA_i; // Max consecutive turns
vector<int> CMI_i; // Min consecutive turns 
vector<int> DL_i; // Min consecutive days-off per worker i
vector<int> FM_i; // Max weekends working per worker i
map<string, int> shift_map;
map<int, string> shift_map_inv;
map<string, int> staff_map;
map<int, string> staff_map_inv;
vector<vector<int>> day_turn_qty;
vector<int> penalty_per_worker; // Neccesary for output
vector<vector<int>> penalty_day_turn; // Neccesary for output
int eval = 0;
int broken_h_constr = 0;