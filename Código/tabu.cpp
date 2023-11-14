#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <map>
#include <bits/stdc++.h>
#include "global_variables.hpp"
#include "custom_queue.cpp"
using namespace std;

void set_y_from_x(vector<vector<vector<bool>>>& y, int x, int i, int d, int t){
    // Changes y_i_d_t value and updates y_i_d_x. CALLED AFTER UPDATING NUMERIC SOLUTION
    y[i][d][t] = 0;
    y[i][d][x] = 1;
}

int eval_hard_constraints(vector<int> sol, vector<vector<bool>> sol_bit, int i){
    // evaluate hard constraints, adding BIG penalty for each one.
    int penalty = 0;
    int total_min = 0;
    vector<int> person_turn_qty(CT, 0);
    bool is_broken_1, is_broken_2;

    for (int t = 0; t < CT; t++){
        for (int d = 0; d < h; d++){
            if (t == 0){
                is_broken_1 = ((!(sol_bit[d][t]) && LO_i_d[i][d]) == 1);
                // if (is_broken_1) cout << "Restriccion dias libres obligatorios rota" << endl;
                penalty += is_broken_1 * PENALTY_COST; // Mandatory days-off (13)
                broken_h_constr += is_broken_1;
            }
            else{
                if (d != 0){
                    int previous_day_turn = sol[d - 1];
                    is_broken_1 = ((R_t_k[t][previous_day_turn] && sol_bit[d][t]) == 1);
                    // if (is_broken_1) cout << "Restriccion turnos consecutivos rota" << endl;
                    penalty += is_broken_1 * PENALTY_COST; // Consecutive Turns (3)
                    broken_h_constr += is_broken_1;
                }
                person_turn_qty[t] += sol_bit[d][t]; // Turns per person per shift (4)                
                total_min += sol_bit[d][t] * L_t[t - 1]; // Minutes per person (5)
            }
        }
    }

    for (int t = 1; t < CT; t++){
        is_broken_1 = (person_turn_qty[t] > T_i_t[i][t]);
        // if (is_broken_1) cout << "Restriccion cantidad de tipo de turno por persona rota" << endl;
        penalty += is_broken_1? (person_turn_qty[t] - T_i_t[i][t]) * PENALTY_COST: 0; // (4)
        broken_h_constr += is_broken_1;
    }
    is_broken_1 = (total_min < MI_i[i]);
    // if (is_broken_1) cout <<  "Empleado " << staff_map_inv[i] << " no trabajó minimo de minutos" << endl;
    is_broken_2 = (total_min > MA_i[i]);
    // if (is_broken_2) cout <<  "Empleado " << staff_map_inv[i] << " trabajó más que el máximo de minutos" << endl;
    penalty += is_broken_1? (MI_i[i] - total_min) : 0; // (5)
    penalty += is_broken_2? (total_min - MA_i[i]) : 0; // (5)
    broken_h_constr += is_broken_1 + is_broken_2;

    // (6 - 10)  
    int work_streak = 0;
    int off_streak = 0;
    for (int d = 0; d < h; d++){
        if (sol[d] != 0){
            work_streak += 1;
            is_broken_1 = (off_streak != 0 && off_streak < DL_i[i]);
            // if (is_broken_1) cout << "Restriccion minimo de días libres consecutivos rota" << endl;
            penalty += is_broken_1 * PENALTY_COST; // (9 - 10) 
            broken_h_constr += is_broken_1;
            off_streak = 0;
        }
        else if(sol[d] == 0){
            off_streak += off_streak == 0? !LO_i_d[i][d]: 1;
            is_broken_1 = (work_streak != 0 && !LO_i_d[i][d] && work_streak < CMI_i[i]);
            // if (is_broken_1) cout << "Empleado " << staff_map_inv[i] << " no trabajó minimo de turnos consecutivos" << endl;
            penalty += is_broken_1? PENALTY_COST: 0; // (6 - 7)
            is_broken_2 = (work_streak != 0 && work_streak > CMA_i[i]);
            // if (is_broken_2) cout <<  "Empleado " << staff_map_inv[i] << " trabajó más que maximo de turnos consecutivos" << endl;
            penalty += is_broken_2? PENALTY_COST: 0; // (8)
            broken_h_constr += is_broken_1 + is_broken_2;

            work_streak = 0;
        }
    }

    // (11 - 12)
    int weekends_worked = 0;
    for (int d = 0; d < h; d++){
        weekends_worked += ((d % 7 == 6) || (d % 7 == 5))? sol[d] != 0: 0; // (12)
    }
    is_broken_1 = (weekends_worked > FM_i[i]);
    // if (is_broken_1) cout << "Restriccion fin de semanas trabajando rota" << endl;
    penalty += is_broken_1? PENALTY_COST : 0; // Max weekends that can be assigned (11)
    broken_h_constr += is_broken_1;

    penalty = penalty < 0? 10000000 - penalty : penalty; // Stop over flow

    return penalty;
}

int eval_soft_constraints(vector<int> sol, vector<vector<bool>> sol_bit, int i, int d, int t, int prev_turn){
    int penalty = 0;
    int penalty_get = 0;

    day_turn_qty[d][t] += 1; // Over Staff and Under Staff (14 - 17)
    day_turn_qty[d][prev_turn] -= 1;
    
    penalty_get = sol_bit[d][t] * PAT_i_d_t[i][d][t];
    penalty += penalty_get;

    penalty_get = (sol_bit[d][t] == 0) * PNAT_i_d_t[i][d][t];
    penalty += penalty_get;

    penalty += (day_turn_qty[d][t] > S_d_t[d][t])? (day_turn_qty[d][t] - S_d_t[d][t]) * OS_d_t[d][t]: 0;
    penalty += (day_turn_qty[d][prev_turn] > S_d_t[d][prev_turn])? (day_turn_qty[d][prev_turn] - S_d_t[d][prev_turn]) * OS_d_t[d][prev_turn]: 0;

    penalty += (day_turn_qty[d][t] < S_d_t[d][t])? (S_d_t[d][t] - day_turn_qty[d][t]) * US_d_t[d][t]: 0;
    penalty += (day_turn_qty[d][prev_turn] < S_d_t[d][prev_turn])? (S_d_t[d][prev_turn] - day_turn_qty[d][prev_turn]) * US_d_t[d][prev_turn]: 0;

    return penalty;
}

int eval_tabu_sol(vector<int> sol, vector<vector<bool>> sol_bit, int staff_index, int d, int t, int prev_turn){
    // Returns the total penalty for the solution
    broken_h_constr = 0;

    // Soft constraints depend heavily in post-greedy evaluation

    int hard_penalty = eval_hard_constraints(sol, sol_bit, staff_index);
    int soft_penalty = eval_soft_constraints(sol, sol_bit, staff_index, d, t, prev_turn);
    return hard_penalty + soft_penalty;
}

void set_start_point(vector<vector<int>>& sol, vector<vector<vector<bool>>>& sol_bit){
    int turn = 1;
    sol.push_back(vector<int>(1, turn));
    sol_bit.push_back(vector<vector<bool>>(1, vector<bool>(CT, false)));
    sol_bit[0][0][turn] = true;
}

void shift_flip(vector<int>& sol, vector<vector<bool>>& sol_bit, int i, int d){
    // changes shift for employee i in day d to the next one
    int prev_shift = sol[d];
    int new_shift = (sol[d] + 1) % (CT + 1);
    sol[d] = new_shift;
    sol_bit[d][prev_shift] = 0;
    sol_bit[d][new_shift] = 1;
}

void tabu_search(vector<vector<int>>& sol, vector<vector<vector<bool>>>& sol_bit){
    // Handles the complete tabu search algorithm, setting iterations, list size and implements algorithm behaviour.
    int iterations = n*h;

    vector<vector<int>> local_best(sol); // Saving the best solution in the iteration
    vector<vector<vector<bool>>> local_best_bit(sol_bit);
    vector<vector<int>> curr_sol; // This is the one that will change
    vector<vector<vector<bool>>> curr_sol_bit;
    FixedQueue<string, 2> tabu_list;
    vector<int> worker_week;
    vector<vector<bool>> bit_week;
    
    int global_min_eval = 100000000;
    int local_min_eval;
    // Tabu list struct defined in another cpp
    for (int k = 0; k < iterations; k++){
        int begin_time = clock();
        // cout << k << endl;
        local_min_eval = 100000000;
        curr_sol = local_best;
        curr_sol_bit = local_best_bit;
        string movement;

        // Generate neighborhood
        for (int i = 0; i < n; i++){
            worker_week = curr_sol[i];
            bit_week = curr_sol_bit[i];
            for (int d = 0; d < h; d++){
                // Check if movement is tabu, if it is, then don't do it.
                string curr_move = "f" + to_string(i) + to_string(d);
                bool is_tabu = false;
                for(auto it=tabu_list.begin(); it!=tabu_list.end();++it){
                    if (*it == curr_move){
                        is_tabu = true;
                        break;
                    }
                }
                if (is_tabu) continue;

                // Evaluate and compare new solution
                int prev_shift = worker_week[d];
                shift_flip(worker_week, bit_week, i, d);
                int new_shift = worker_week[d];
                int eval = eval_tabu_sol(worker_week, bit_week, i, d, new_shift, prev_shift);
                bool is_local_min = eval < local_min_eval;
                if (is_local_min){ // Update local minimum
                    local_min_eval = eval;
                    local_best[i] = worker_week;
                    local_best_bit[i] = bit_week;
                    movement = "f" + to_string(i) + to_string(d);
                }
                // Change solution back to its original state, because TS checks neighborhood
                worker_week[d] = prev_shift;
                bit_week[d][prev_shift] = 1;
                bit_week[d][new_shift] = 0;
            }

            curr_sol = local_best;
            curr_sol_bit = local_best_bit;
        }

        bool is_new_min = local_min_eval < global_min_eval;
        if (is_new_min){ // update global minimum
            global_min_eval = local_min_eval;
            sol = local_best;
            sol_bit = local_best_bit;
            tabu_list.push(movement);
        }
        cout << clock() - begin_time << endl;
    }
    
    return;
}