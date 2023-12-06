#ifndef GLOBAL_VAR_H
#define GLOVAL_VAR_H

#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <string>

extern int h; //Horizon Length
extern int n; // Total employees
extern int CT; // Total Turns
extern std::vector<int> L_t; // Length of turns

extern std::vector<std::vector<bool>> R_t_k; // Consecutive turns rules
extern std::map<std::string, std::string> R_t_k_aux; // Aux to help convertion
extern std::vector<std::vector<int>> T_i_t; // Max turns to be assigned per worker i
extern std::map<std::string, std::string> T_i_t_aux; // Aux to help convertion
extern std::vector<std::vector<int>> Turns_left_to_i; // Works alongside T_i_t for greedy solution

extern std::vector<std::vector<bool>> LO_i_d; // Mandatory days-off
extern std::vector<std::vector<std::vector<int>>> PNAT_i_d_t; // Non-Assignation Penalty
extern std::vector<std::vector<std::vector<int>>> PAT_i_d_t; // Assignation Penalty
extern std::vector<std::vector<int>> S_d_t; // Employees required for day t
extern std::vector<std::vector<int>> OS_d_t; // Over Staffing penalty
extern std::vector<std::vector<int>> US_d_t; // Under Staffing penalty

extern std::vector<int> MA_i; // Max minutes to work in cycle
extern std::vector<int> MI_i; // Min minutes to work in cycle
extern std::vector<int> CMA_i; // Max consecutive turns
extern std::vector<int> CMI_i; // Min consecutive turns 
extern std::vector<int> DL_i; // Min consecutive days-off per worker i
extern std::vector<int> FM_i; // Max weekends working per worker i

extern std::map<std::string, int> shift_map;
extern std::map<int, std::string> shift_map_inv;
extern std::map<std::string, int> staff_map;
extern std::map<int, std::string> staff_map_inv;

extern std::vector<std::vector<int>> day_turn_qty;
extern std::vector<int> penalty_per_worker; // Neccesary for output
extern std::vector<std::vector<int>> penalty_day_turn; // Neccesary for output
const int PENALTY_COST = 1000; // Penalty for hard constraints
extern int eval;
extern int broken_h_constr;

#endif