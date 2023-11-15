#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <map>
#include <bits/stdc++.h>
#include "tabu.cpp"
using namespace std;

int eval_hard_greedy(vector<int> sol, vector<vector<bool>> sol_bit, int i){
    // evaluate hard constraints, adding BIG penalty for each one.
    int curr_days = sol.size();
    int penalty = 0;
    int total_min = 0;
    vector<int> person_turn_qty(CT, 0);
    bool is_broken_1, is_broken_2;
    
    for (int t = 0; t < CT; t++){
        for (int d = 0; d < curr_days; d++){
            if (t == 0){
                is_broken_1 = ((!(sol_bit[d][t]) && LO_i_d[i][d]) == 1);
                // if (is_broken_1) cout << "Restriccion dias libres obligatorios rota" << endl;
                penalty += is_broken_1 * PENALTY_COST; // Mandatory days-off (13)
                broken_h_constr += is_broken_1;
            }
            else{
                if (d != 0 && d == curr_days - 1){
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
        Turns_left_to_i[i][t] = T_i_t[i][t] - person_turn_qty[t];
        penalty += is_broken_1? (person_turn_qty[t] - T_i_t[i][t]) * PENALTY_COST: 0; // (4)
        broken_h_constr += is_broken_1;
    }
    is_broken_1 = (total_min < MI_i[i]);
    // if (is_broken_1) cout << "Restriccion minimo de minutos rota" << endl;
    is_broken_2 = (total_min > MA_i[i]);
    // if (is_broken_2) cout << "Restriccion máximo de minutos rota" << endl;
    penalty += is_broken_1? (MI_i[i] - total_min) : 0; // (5)
    penalty += is_broken_2? (total_min - MA_i[i]) : 0; // (5)
    broken_h_constr += is_broken_1 + is_broken_2;

    // (11 - 12)
    int weekends_worked = 0;
    if (curr_days >= 5) {
        for (int d = 0; d < curr_days; d++){
            weekends_worked += ((d % 7 == 6) || (d % 7 == 5))? sol[d] != 0: 0; // (12)
        }
        is_broken_1 = (weekends_worked > FM_i[i]);
        // if (is_broken_1) cout << "Restriccion fin de semanas trabajando rota" << endl;
        penalty += is_broken_1? PENALTY_COST : 0; // Max weekends that can be assigned (11)
        broken_h_constr += is_broken_1;
    }

    // (6 - 10)  
    int work_streak = 0;
    int off_streak = 0;
    for (int d = 0; d < curr_days; d++){
        if (sol[d] != 0 && curr_days >= DL_i[i]){
            work_streak += 1;
            if (d == curr_days - 1){
                int days_to_weekend = 5 - (d % 7);
                int days_for_min = (CMI_i[i] > work_streak)? CMI_i[i] - work_streak: 0;
                // don't start working if you'll end up working on a weekend and you shouldn't
                if (work_streak == 1 && days_to_weekend <= days_for_min && weekends_worked >= FM_i[i]) penalty += PENALTY_COST;
                // don't start working if you'll end up working more than the max possible
                if (work_streak == 1 && (total_min + (days_for_min * L_t[sol[d] - 1])) > MA_i[i]) penalty += PENALTY_COST;
 
                // if (work_streak == 1 && days_to_weekend <= CMI_i[i]) penalty += PENALTY_COST; 
                
                int max_broken = work_streak > CMA_i[i];
                // if (max_broken) cout << "Restriccion maximo turnos consecutivos rota" << endl;
                penalty += max_broken? PENALTY_COST: 0;
                broken_h_constr += max_broken;
            }
            
            is_broken_1 = (off_streak != 0 && off_streak < DL_i[i]);
            // if (is_broken_1) cout << "Restriccion minimo de días libres consecutivos rota" << endl;
            penalty += is_broken_1 * PENALTY_COST; // (9 - 10) 
            broken_h_constr += is_broken_1;
            off_streak = 0;
        }
        else if(sol[d] == 0 && curr_days >= CMI_i[i]){
            off_streak += off_streak == 0? !LO_i_d[i][d]: 1;
            is_broken_1 = (work_streak != 0 && !LO_i_d[i][d] && work_streak < CMI_i[i]);
            // if (is_broken_1) cout << "Restriccion minimo de turnos consecutivos rota" << endl;
            penalty += is_broken_1? PENALTY_COST: 0; // (6 - 7)
            is_broken_2 = (work_streak != 0 && work_streak > CMA_i[i]);
            // if (is_broken_2) cout << "Restriccion maximo de turnos consecutivos rota" << endl;
            penalty += is_broken_2? PENALTY_COST: 0; // (8)
            broken_h_constr += is_broken_1 + is_broken_2;

            work_streak = 0;
        }
    }

    penalty = penalty < 0? 10000000 - penalty : penalty; // Stop over flow

    return penalty;
}

int eval_soft_greedy(vector<int> sol, vector<vector<bool>> sol_bit, int i){
    //  evaluate soft constraints, adding penalty and counter per worker.
    int curr_days = sol.size();
    int penalty = 0;
    int penalty_get = 0;

    for (int t = 1; t < CT; t++){
        for (int d = 0; d < curr_days; d++){
            penalty_get = sol_bit[d][t] * PAT_i_d_t[i][d][t];
            penalty += penalty_get;
            penalty_per_worker[i] += penalty_get != 0;

            penalty_get = (sol_bit[d][t] == 0) * PNAT_i_d_t[i][d][t];
            penalty += penalty_get;
            penalty_per_worker[i] += penalty_get != 0;
        }
    }

    return penalty;
}

int eval_greedy_sol(vector<int> sol, vector<vector<bool>> sol_bit, int staff_index){
    // Returns the total penalty for greedy solution
    int hard_penalty = eval_hard_greedy(sol, sol_bit, staff_index);
    int soft_penalty = eval_soft_greedy(sol, sol_bit, staff_index);
    return hard_penalty + soft_penalty;
}

void set_y_from_x(vector<vector<bool>>& y, int x, int d, int t){
    // Used for greedy optimization
    y[d][t] = 0;
    y[d][x] = 1;
}

void set_start_point(vector<int>& sol, vector<vector<bool>>& sol_bit){
    // Used for greedy optimization
    int turn = 1;
    sol.push_back(turn);
    sol_bit.push_back(vector<bool>(CT, false));
    sol_bit[0][turn] = true;
}

void solve_greedy(vector<vector<int>>& sol, vector<vector<vector<bool>>>& sol_bit){
    // Checks turns per worker per day, and chooses the one that minimizes the function.
    vector<vector<bool>> aux_sol_bit;
    vector<int> aux_sol;
    set_start_point(aux_sol, aux_sol_bit); // Need these auxiliar vectors so that access is faster

    for (int i = 0; i < n; i++){
        for (int d = 0; d < h; d++){
            int original_t = aux_sol[d];
            int min_eval = eval_greedy_sol(aux_sol, aux_sol_bit, i);
            int min_turn = original_t;

            for (int t = 0; t < CT; t++){
                if (t == original_t) continue;
                if (t != 0 && Turns_left_to_i[i][t] == 0) continue; // Don't check turns that are impossible

                int prev_shift = aux_sol[d];
                aux_sol[d] = t;
                set_y_from_x(aux_sol_bit, t, d, prev_shift);
                int new_eval = eval_greedy_sol(aux_sol, aux_sol_bit, i);
                bool is_min = new_eval < min_eval;
                bool is_equal = new_eval == min_eval;
                if (is_equal){ // if equal, then choose the one that has more turn available
                    bool is_turn_more_available = Turns_left_to_i[i][t] > Turns_left_to_i[i][min_turn];
                    min_eval = is_turn_more_available? new_eval : min_eval;
                    min_turn = is_turn_more_available? t : min_turn;
                }
                else {
                    min_eval = is_min? new_eval : min_eval;
                    min_turn = is_min? t : min_turn;
                }
            }
            // cout << min_eval << endl;
            int prev_aux_shift = aux_sol[d];
            aux_sol[d] = min_turn;
            set_y_from_x(aux_sol_bit, min_turn, d, prev_aux_shift);

            int prev_sol_shift = sol[i][d]; 
            sol[i][d] = min_turn;
            set_y_from_x(sol_bit, min_turn, i, d, prev_sol_shift);

            if (d < h - 1){
                sol[i].push_back(1);
                sol_bit[i].push_back(vector<bool>(CT, false));
                sol_bit[i][d+1][1] = true;

                aux_sol.push_back(1);
                aux_sol_bit.push_back(vector<bool>(CT, false));
                aux_sol_bit[d+1][1] = true;
            }
        }

        if (i < n - 1){
            sol.push_back(vector<int>(1, 1));
            sol_bit.push_back(vector<vector<bool>>(1, vector<bool>(CT, false)));
            sol_bit[i + 1][0][1] = true;

            aux_sol.resize(1, 1);
            aux_sol[0] = 1;
            aux_sol_bit.resize(1, vector<bool>(CT, false));
            aux_sol_bit[0][1] = true;
        }
    }
    return;
}