#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <map>
#include <bits/stdc++.h>
#include "global_variables.hpp"
#include "custom_queue.cpp"
using namespace std;

int full_eval_hard_constraints(vector<vector<int>> sol, vector<vector<vector<bool>>> sol_bit){
    // evaluate hard constraints, adding BIG penalty for each one.
    int staff = sol.size();
    int penalty = 0;
    vector<int> total_min (staff, 0);
    vector<vector<int>> person_turn_qty(staff, vector<int>(CT, 0));
    bool is_broken_1, is_broken_2;
    
    for (int i = 0; i < staff; i++){
        int curr_days = sol[i].size();
        for (int t = 0; t < CT; t++){
            for (int d = 0; d < curr_days; d++){
                if (t == 0){
                    is_broken_1 = ((!(sol_bit[i][d][t]) && LO_i_d[i][d]) == 1);
                    if (is_broken_1) cout << "Restriccion dias libres obligatorios rota" << endl;
                    penalty += is_broken_1 * PENALTY_COST; // Mandatory days-off (13)
                    broken_h_constr += is_broken_1;
                }
                else{
                    if (d != 0){
                        int previous_day_turn = sol[i][d - 1];
                        is_broken_1 = ((R_t_k[t][previous_day_turn] && sol_bit[i][d][t]) == 1);
                        if (is_broken_1) cout << "Restriccion turnos consecutivos rota" << endl;
                        penalty += is_broken_1 * PENALTY_COST; // Consecutive Turns (3)
                        broken_h_constr += is_broken_1;
                    }
                    person_turn_qty[i][t] += sol_bit[i][d][t]; // Turns per person per shift (4)                
                    total_min[i] += sol_bit[i][d][t] * L_t[t - 1]; // Minutes per person (5)
                }
            }
        }
    }

    for (int i = 0; i < staff; i++){
        for (int t = 1; t < CT; t++){
            is_broken_1 = (person_turn_qty[i][t] > T_i_t[i][t]);
            if (is_broken_1) cout << "Restriccion cantidad de tipo de turno por persona rota" << endl;
            penalty += is_broken_1? (person_turn_qty[i][t] - T_i_t[i][t]) * PENALTY_COST: 0; // (4)
            broken_h_constr += is_broken_1;
        }
        is_broken_1 = (total_min[i] < MI_i[i]);
        if (is_broken_1) cout <<  "Empleado " << staff_map_inv[i] << " no trabajó minimo de minutos" << endl;
        is_broken_2 = (total_min[i] > MA_i[i]);
        if (is_broken_2) cout <<  "Empleado " << staff_map_inv[i] << " trabajó más que el máximo de minutos" << endl;
        penalty += is_broken_1? (MI_i[i] - total_min[i]) : 0; // (5)
        penalty += is_broken_2? (total_min[i] - MA_i[i]) : 0; // (5)
        broken_h_constr += is_broken_1 + is_broken_2;
    }
    // (6 - 10)  
    for (int i = 0; i < staff; i++){
        int curr_days = sol[i].size();
        int work_streak = 0;
        int off_streak = 0;
        for (int d = 0; d < curr_days; d++){
            if (sol[i][d] != 0 && curr_days >= DL_i[i]){
                work_streak += 1;
                
                is_broken_1 = (off_streak != 0 && off_streak < DL_i[i]);
                if (is_broken_1) cout << "Restriccion minimo de días libres consecutivos rota" << endl;
                penalty += is_broken_1 * (DL_i[i] - off_streak) * PENALTY_COST; // (9 - 10) 
                broken_h_constr += is_broken_1;
                off_streak = 0;
            }
            else if(sol[i][d] == 0 && curr_days >= CMI_i[i]){
                // bool off_broken = off_streak < DL_i[i];
                // penalty += off_broken? PENALTY_COST: 0;
                off_streak += off_streak == 0? !LO_i_d[i][d]: 1;
                is_broken_1 = (work_streak != 0 && !LO_i_d[i][d] && work_streak < CMI_i[i]);
                if (is_broken_1) cout << "Empleado " << staff_map_inv[i] << " no trabajó minimo de turnos consecutivos" << endl;
                penalty += is_broken_1? (CMI_i[i] - work_streak) * PENALTY_COST: 0; // (6 - 7)
                is_broken_2 = (work_streak != 0 && work_streak > CMA_i[i]);
                if (is_broken_2) cout <<  "Empleado " << staff_map_inv[i] << " trabajó más que maximo de turnos consecutivos" << endl;
                penalty += is_broken_2? (work_streak - CMA_i[i]) * PENALTY_COST: 0; // (8)
                broken_h_constr += is_broken_1 + is_broken_2;

                work_streak = 0;
            }
        }
    }

    // (11 - 12)
    vector<int> weekends_worked (staff, 0);
    for (int i = 0; i < staff; i++){
        int curr_days = sol[i].size();
        if (curr_days < 5) continue; 
        for (int d = 0; d < curr_days; d++){
            weekends_worked[i] += ((d % 7 == 6) || (d % 7 == 5))? sol[i][d] != 0: 0; // (12)
        }
        is_broken_1 = (weekends_worked[i] > FM_i[i]);
        if (is_broken_1) cout << "Restriccion fin de semanas trabajando rota" << endl;
        penalty += is_broken_1? (weekends_worked[i] - FM_i[i]) * PENALTY_COST : 0; // Max weekends that can be assigned (11)
        broken_h_constr += is_broken_1;
        
    }

    penalty = penalty < 0? 10000000 - penalty : penalty; // Stop over flow

    return penalty;
}

int full_eval_soft_constraints(vector<vector<int>> sol, vector<vector<vector<bool>>> sol_bit){
    //  evaluate soft constraints, adding penalty and counter per worker.
    int staff = sol.size();
    int days = sol[0].size();
    int penalty = 0;
    int penalty_get = 0;
    
    for (int i = 0; i < staff; i++){
        int curr_days = sol[i].size();
        for (int t = 0; t < CT; t++){
            for (int d = 0; d < curr_days; d++){
                day_turn_qty[d][t] += sol_bit[i][d][t]; // Over Staff and Under Staff (14 - 17)
                
                penalty_get = sol_bit[i][d][t] * PAT_i_d_t[i][d][t];
                penalty += penalty_get;
                penalty_per_worker[i] += penalty_get != 0;
                penalty_get = (sol_bit[i][d][t] == 0) * PNAT_i_d_t[i][d][t];
                penalty += penalty_get;
                penalty_per_worker[i] += penalty_get != 0;
            }
        }
    }
    penalty_day_turn.resize(days, vector<int>(CT, 0));
    for (int d = 0; d < days; d++){ // (14 - 17)
        int p_amount = 0;
        for (int t = 1; t < CT; t++){
            p_amount = (day_turn_qty[d][t] > S_d_t[d][t])? (day_turn_qty[d][t] - S_d_t[d][t]) * OS_d_t[d][t]: 0;
            penalty += p_amount;
            penalty_day_turn[d][t] += p_amount;
            p_amount = (day_turn_qty[d][t] < S_d_t[d][t])? (S_d_t[d][t] - day_turn_qty[d][t]) * US_d_t[d][t]: 0;
            penalty += p_amount;
            penalty_day_turn[d][t] += p_amount;
        }
    }
    return penalty;
}

int full_eval_sol(vector<vector<int>> sol, vector<vector<vector<bool>>> sol_bit){
    // Returns the total penalty for the solution
    day_turn_qty.clear();
    day_turn_qty.resize(h, vector<int>(CT, 0));
    penalty_day_turn.clear();
    penalty_day_turn.resize(h, vector<int>(CT, 0));
    penalty_per_worker.clear();
    penalty_per_worker.resize(n, 0);
    broken_h_constr = 0;
    int hard_penalty = full_eval_hard_constraints(sol, sol_bit);
    int soft_penalty = full_eval_soft_constraints(sol, sol_bit);
    return hard_penalty + soft_penalty;
}

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
                penalty += is_broken_1 * PENALTY_COST; // Mandatory days-off (13)
                broken_h_constr += is_broken_1;
            }
            else{
                if (d != 0){
                    int previous_day_turn = sol[d - 1];
                    is_broken_1 = ((R_t_k[t][previous_day_turn] && sol_bit[d][t]) == 1);
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
        penalty += is_broken_1? (person_turn_qty[t] - T_i_t[i][t]) * PENALTY_COST: 0; // (4)
        broken_h_constr += is_broken_1;
    }
    is_broken_1 = (total_min < MI_i[i]);
    is_broken_2 = (total_min > MA_i[i]);
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
            penalty += is_broken_1 * (DL_i[i] - off_streak) * PENALTY_COST; // (9 - 10) 
            broken_h_constr += is_broken_1;
            off_streak = 0;
        }
        else if(sol[d] == 0){
            off_streak += off_streak == 0? !LO_i_d[i][d]: 1;
            is_broken_1 = (work_streak != 0 && !LO_i_d[i][d] && work_streak < CMI_i[i]);
            penalty += is_broken_1? (CMI_i[i] - work_streak) * PENALTY_COST: 0; // (6 - 7)
            is_broken_2 = (work_streak != 0 && work_streak > CMA_i[i]);
            penalty += is_broken_2? (work_streak - CMA_i[i]) * PENALTY_COST: 0; // (8)
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
    penalty += is_broken_1? (weekends_worked - FM_i[i]) * PENALTY_COST : 0; // Max weekends that can be assigned (11)
    broken_h_constr += is_broken_1;

    penalty = penalty < 0? 10000000 - penalty : penalty; // Stop over flow

    return penalty;
}

int eval_soft_constraints(vector<int> sol, vector<vector<bool>> sol_bit, int i, int d, int t, int prev_turn){
    // Tabu version for soft constraints, optimized to be O(1).
    int penalty = 0;
    int penalty_get = 0;

    // Turn 0 doesn't matter
    int this_qty = day_turn_qty[d][t] + 1; // Over Staff and Under Staff (14 - 17)
    int prev_qty = day_turn_qty[d][prev_turn] - 1;

    // cout << "CANTIDAD DEL DÍA " << this_qty << " " << prev_qty << endl;
    
    penalty_get = sol_bit[d][t] * PAT_i_d_t[i][d][t];
    penalty += penalty_get;

    penalty_get = (sol_bit[d][t] == 0) * PNAT_i_d_t[i][d][t];
    penalty += penalty_get;

    penalty += (this_qty > S_d_t[d][t])? (this_qty - S_d_t[d][t]) * OS_d_t[d][t]: 0;
    penalty += (prev_qty > S_d_t[d][prev_turn])? (prev_qty - S_d_t[d][prev_turn]) * OS_d_t[d][prev_turn]: 0;

    penalty += (this_qty < S_d_t[d][t])? (S_d_t[d][t] - this_qty) * US_d_t[d][t]: 0;
    penalty += (prev_qty < S_d_t[d][prev_turn])? (S_d_t[d][prev_turn] - prev_qty) * US_d_t[d][prev_turn]: 0;

    return penalty;
}

int eval_tabu_sol(vector<int> sol, vector<vector<bool>> sol_bit, int staff_index, int d, int t, int prev_turn){
    // Returns the total penalty for the solution
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
    int new_shift = (prev_shift + 1) % CT;
    sol[d] = new_shift;
    sol_bit[d][prev_shift] = 0;
    sol_bit[d][new_shift] = 1;
}

void tabu_search(vector<vector<int>>& sol, vector<vector<vector<bool>>>& sol_bit){
    // Handles the complete tabu search algorithm, setting iterations, list size and implements algorithm behaviour.
    int iterations = n*h;

    vector<vector<int>> local_best(sol); // Saving the best solution in the iteration
    vector<vector<vector<bool>>> local_best_bit(sol_bit);
    vector<vector<int>> curr_sol(sol); // This is the one that will change
    vector<vector<vector<bool>>> curr_sol_bit(sol_bit);
    FixedQueue<string, 2> tabu_list;
    vector<int> worker_week;
    vector<vector<bool>> bit_week;
    
    int best_t;
    int best_d;
    int turn_pre_best;

    int local_min_eval;
    // Tabu list struct defined in another cpp
    for (int k = 0; k < iterations; k++){ // Tabu Iterations
        int begin_time = clock();
        local_min_eval = 100000000;
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

                int new_eval = eval_tabu_sol(worker_week, bit_week, i, d, new_shift, prev_shift);
                bool is_local_min = new_eval < local_min_eval;
                if (is_local_min){ // Update local minimum
                    cout << "MINIMO ITER " << k << " Valor: " << new_eval << " P " << i << " D " << d << endl;
                    curr_sol[i] = worker_week;
                    curr_sol_bit[i] = bit_week;
                    local_min_eval = new_eval;
                    local_best = curr_sol;
                    local_best_bit = curr_sol_bit;
                    movement = curr_move;
                    best_t = new_shift;
                    turn_pre_best = prev_shift;
                    best_d = d;
                }
                // Change solution back to its original state, because TS checks neighborhood
                worker_week[d] = prev_shift;
                bit_week[d][prev_shift] = true;
                bit_week[d][new_shift] = false;

                curr_sol[i] = worker_week;
                curr_sol_bit[i] = bit_week;
                if (local_min_eval == 0) break; // cut early if already optimized locality
            }
            if (local_min_eval == 0) break;
        }
        // Best move already selected
        tabu_list.push(movement);
        day_turn_qty[best_d][best_t] += 1;
        day_turn_qty[best_d][turn_pre_best] -= 1;
        curr_sol = local_best;
        curr_sol_bit = local_best_bit;
        // cout << clock() - begin_time << endl;
    }

    int tabu_eval = full_eval_sol(local_best, local_best_bit);
    cout << tabu_eval << endl;
    if (tabu_eval < eval){
        sol = local_best;
        sol_bit = local_best_bit;
    }
    
    return;
}