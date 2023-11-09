#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <map>
#include <bits/stdc++.h>
using namespace std; // TA FALLANDO EL WEEKEND COUNT

int h; //Horizon Length
int n; // Total employees
int CT; // Total Turns
vector<int> L_t; // Length of turns

vector<vector<bool>> R_t_k; // Consecutive turns rules
map<string, string> R_t_k_aux; // Aux to help convertion
vector<vector<int>> T_i_t; // Max turns to be assigned per worker i
map<string, string> T_i_t_aux; // Aux to help convertion

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
const int PENALTY_COST = 1000;
int eval = 0;
int broken_h_constr = 0;

// length of the string  
int len(string str)  
{  
    int length = 0;  
    for (int i = 0; str[i] != '\0'; i++)  
    {  
        length++;  
          
    }  
    return length;     
}  
  
// create custom split() function  
vector<string> split (string str, char separator)  
{  
    vector<string> strings;
    int currIndex = 0, i = 0;  
    int startIndex = 0, endIndex = 0;  
    while (i <= len(str))  
    {  
        if (str[i] == separator || i == len(str))  
        {  
            endIndex = i;  
            string subStr = "";  
            subStr.append(str, startIndex, endIndex - startIndex);  
            strings.push_back(subStr);  
            currIndex += 1;  
            startIndex = endIndex + 1;  
        }  
        i++;  
    }  
    return strings;   
}

void read_instance(){
    int file_number;
    cout << "Inserte numero de la instancia (1 - 24): ";
    cin >> file_number;
    string file_name = "Instancias/Instance" + to_string(file_number) + ".txt";
    ifstream instance_file(file_name);

    string text;

    getline(instance_file, text); getline(instance_file, text); getline(instance_file, text); getline(instance_file, text); 
    getline(instance_file, text); h = stoi(text);// Read cycle size

    getline(instance_file, text); getline(instance_file, text); getline(instance_file, text);  // Read white lines and text that we don't need

    string line;
    int shift_i = 1;
    shift_map.insert(make_pair("-", 0)); // day off id.
    shift_map_inv.insert(make_pair(0, "-"));
    while (getline(instance_file, line)){ // Read SECTION_SHIFTS
        if (line[0] == 13){ // Find whiteline
            break;
        }
        vector<string> splitted = split(line, ',');
        shift_map.insert(make_pair(splitted[0], shift_i)); // insert shift -> Id relation
        shift_map_inv.insert(make_pair(shift_i, splitted[0]));
        L_t.push_back(stoi(splitted[1]));
        
        R_t_k_aux.insert(make_pair(splitted[0], splitted[2])); // Insert consecutive turn bans
        shift_i++;
    }

    CT = shift_i;

    getline(instance_file, text); getline(instance_file, text);
    int staff_i = 0;
    while (getline(instance_file, line)){ // Read SECTION_STAFF
        if (line[0] == 13){ // Find whiteline
            break;
        }
        vector<string> splitted = split(line, ',');
        staff_map.insert(make_pair(splitted[0], staff_i));
        staff_map_inv.insert(make_pair(staff_i, splitted[0]));
        T_i_t_aux.insert(make_pair(splitted[0], splitted[1]));
        MA_i.push_back(stoi(splitted[2]));
        MI_i.push_back(stoi(splitted[3]));
        CMA_i.push_back(stoi(splitted[4]));
        CMI_i.push_back(stoi(splitted[5]));
        DL_i.push_back(stoi(splitted[6]));
        FM_i.push_back(stoi(splitted[7]));

        staff_i++;
    }
    n = staff_i;
    penalty_per_worker.resize(n, 0);

    // Resize vectors
    LO_i_d.resize(n, vector<bool> (h, false));
    PNAT_i_d_t.resize(n, vector<vector<int>>(h, vector<int>(CT, 0))); // Non-Assignation Penalty
    PAT_i_d_t.resize(n, vector<vector<int>>(h, vector<int>(CT, 0))); // Assignation Penalty
    S_d_t.resize(h, vector<int>(n, 0)); // Employees required for day t
    OS_d_t.resize(h, vector<int>(n, 0)); // Over Staffing penalty
    US_d_t.resize(h, vector<int>(n, 0)); // Under Staffing penalty

    getline(instance_file, text); getline(instance_file, text);
    while (getline(instance_file, line)){ // Read SECTION_DAYS_OFF
        if (line[0] == 13){ // Find whiteline
            break;
        }
        vector<string> splitted = split(line, ',');
        int st_index = staff_map[splitted[0]];
        for (int i = 1; i < splitted.size(); i++)
        {
            LO_i_d[st_index][stoi(splitted[i])] = true;
        }
    }

    getline(instance_file, text); getline(instance_file, text);
    while (getline(instance_file, line)){ // Read SECTION_SHIFT_ON_REQUEST
        if (line[0] == 13){ // Find whiteline
            break;
        }
        vector<string> splitted = split(line, ',');
        int st_index = staff_map[splitted[0]];
        int day = stoi(splitted[1]);
        int sh_index = shift_map[splitted[2]];
        PNAT_i_d_t[st_index][day][sh_index] = stoi(splitted[3]);
    }

    getline(instance_file, text); getline(instance_file, text);
    while (getline(instance_file, line)){ // Read SECTION_SHIFT_OFF_REQUEST
        if (line[0] == 13){ // Find whiteline
            break;
        }
        vector<string> splitted = split(line, ',');
        int st_index = staff_map[splitted[0]];
        int day = stoi(splitted[1]);
        int sh_index = shift_map[splitted[2]];
        PAT_i_d_t[st_index][day][sh_index] = stoi(splitted[3]);
    }

    getline(instance_file, text); getline(instance_file, text);
    while (getline(instance_file, line)){ // Read SECTION_COVER
        if (line[0] == 13){ // Find whiteline
            break;
        }
        vector<string> splitted = split(line, ',');
        int day = stoi(splitted[0]);
        int sh_index = shift_map[splitted[1]];
        int required = stoi(splitted[2]);
        S_d_t[day][sh_index] = required;
        US_d_t[day][sh_index] = stoi(splitted[3]);
        OS_d_t[day][sh_index] = stoi(splitted[4]);
    }
    instance_file.close();
}

void convert_auxiliars(){
    // Convert R_t_k
    R_t_k.resize(CT, vector<bool>(CT, false));
    map <string, string>::iterator it;
    for (it = R_t_k_aux.begin(); !(it == R_t_k_aux.end()); it++){    
        int first_ind = shift_map[it->first];
        vector<string> splitted = split(it->second, '|');
        for (string shift : splitted){
            if (shift == splitted[splitted.size() - 1]){
                shift.erase(shift.size() - 1);
            }
            map<string, int>::iterator found = shift_map.find(shift);
            if (found == shift_map.end()) continue;

            
            int second_ind = found->second;
            R_t_k[second_ind][first_ind] = true;
        }
    }
    
    // Convert T_i_t
    T_i_t.resize(n, vector<int>(CT, 0));
    for (it = T_i_t_aux.begin(); !(it == T_i_t_aux.end()); it++){
        int employee_ind = staff_map[it->first];
        vector<string> splitted = split(it->second, '|');
        for (string shift : splitted){
            // Need another split for '=';
            vector<string> eq_split = split(shift, '=');
            int shift_ind = shift_map[eq_split[0]];
            T_i_t[employee_ind][shift_ind] = stoi(eq_split[1]);
        }
    }
}

int eval_hard_constraints(vector<vector<int>> sol, vector<vector<vector<bool>>> sol_bit){
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
                    // if (is_broken_1) cout << "Restriccion dias libres obligatorios rota" << endl;
                    penalty += is_broken_1 * PENALTY_COST; // Mandatory days-off (13)
                    broken_h_constr += is_broken_1;
                }
                else{
                    if (d != 0){
                        int previous_day_turn = sol[i][d - 1];
                        is_broken_1 = ((R_t_k[t][previous_day_turn] && sol_bit[i][d][t]) == 1);
                        // if (is_broken_1) cout << "Restriccion turnos consecutivos rota" << endl;
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
            // if (is_broken_1) cout << "Restriccion cantidad de tipo de turno por persona rota" << endl;
            penalty += is_broken_1? (person_turn_qty[i][t] - T_i_t[i][t]) * PENALTY_COST: 0; // (4)
            broken_h_constr += is_broken_1;
        }
        is_broken_1 = (total_min[i] < MI_i[i]);
        // if (is_broken_1) cout << "Restriccion minimo de minutos rota" << endl;
        is_broken_2 = (total_min[i] > MA_i[i]);
        // if (is_broken_2) cout << "Restriccion máximo de minutos rota" << endl;
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
                if (curr_days < h){ // Needed to help greedy solution
                    if (d % 7 == 5) penalty += PENALTY_COST;
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
            else if(sol[i][d] == 0 && curr_days >= CMI_i[i]){
                // bool off_broken = off_streak < DL_i[i];
                // penalty += off_broken? PENALTY_COST: 0;
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
        // if (is_broken_1) cout << "Restriccion fin de semanas trabajando rota" << endl;
        penalty += is_broken_1? PENALTY_COST : 0; // Max weekends that can be assigned (11)
        broken_h_constr += is_broken_1;
        
    }

    penalty = penalty < 0? 10000000 - penalty : penalty; // Stop over flow

    return penalty;
}

int eval_soft_constraints(vector<vector<int>> sol, vector<vector<vector<bool>>> sol_bit){
    //  evaluate soft constraints, adding penalty and counter per worker.
    int staff = sol.size();
    int days = sol[0].size();
    int penalty = 0;
    int penalty_get = 0;
    for (int i = 0; i < staff; i++){
        int curr_days = sol[i].size();
        for (int t = 1; t < CT; t++){
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
            p_amount = (day_turn_qty[d][t] > S_d_t[d][t]) * OS_d_t[d][t];
            penalty += p_amount;
            penalty_day_turn[d][t] += p_amount;

            p_amount = (day_turn_qty[d][t] < S_d_t[d][t]) * US_d_t[d][t];
            penalty += p_amount;
            penalty_day_turn[d][t] += p_amount;
        }
    }
    return penalty;
}

int eval_sol(vector<vector<int>> sol, vector<vector<vector<bool>>> sol_bit){
    // Returns the total penalty for the solution
    int staff = sol.size();
    int days = sol[0].size();
    day_turn_qty.clear();
    day_turn_qty.resize(days, vector<int>(CT, 0));
    penalty_day_turn.clear();
    penalty_day_turn.resize(days, vector<int>(CT, 0));
    penalty_per_worker.clear();
    penalty_per_worker.resize(staff, 0);
    broken_h_constr = 0;
    int hard_penalty = eval_hard_constraints(sol, sol_bit);
    int soft_penalty = eval_soft_constraints(sol, sol_bit);
    return hard_penalty + soft_penalty;
}

int eval_hard_greedy(vector<vector<int>> sol, vector<vector<vector<bool>>> sol_bit){
    // evaluate hard constraints, adding BIG penalty for each one.
    int curr_staff = sol.size() - 1;
    int curr_days = sol[curr_staff].size();
    int i = curr_staff;
    int penalty = 0;
    int total_min = 0;
    vector<int> person_turn_qty(CT, 0);
    bool is_broken_1, is_broken_2;
    
    for (int t = 0; t < CT; t++){
        for (int d = 0; d < curr_days; d++){
            if (t == 0){
                is_broken_1 = ((!(sol_bit[i][d][t]) && LO_i_d[i][d]) == 1);
                // if (is_broken_1) cout << "Restriccion dias libres obligatorios rota" << endl;
                penalty += is_broken_1 * PENALTY_COST; // Mandatory days-off (13)
                broken_h_constr += is_broken_1;
            }
            else{
                if (d != 0){
                    int previous_day_turn = sol[i][d - 1];
                    is_broken_1 = ((R_t_k[t][previous_day_turn] && sol_bit[i][d][t]) == 1);
                    // if (is_broken_1) cout << "Restriccion turnos consecutivos rota" << endl;
                    penalty += is_broken_1 * PENALTY_COST; // Consecutive Turns (3)
                    broken_h_constr += is_broken_1;
                }
                person_turn_qty[t] += sol_bit[i][d][t]; // Turns per person per shift (4)                
                total_min += sol_bit[i][d][t] * L_t[t - 1]; // Minutes per person (5)
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
    // if (is_broken_1) cout << "Restriccion minimo de minutos rota" << endl;
    is_broken_2 = (total_min > MA_i[i]);
    // if (is_broken_2) cout << "Restriccion máximo de minutos rota" << endl;
    penalty += is_broken_1? (MI_i[i] - total_min) : 0; // (5)
    penalty += is_broken_2? (total_min - MA_i[i]) : 0; // (5)
    broken_h_constr += is_broken_1 + is_broken_2;

    // (6 - 10)  
    int work_streak = 0;
    int off_streak = 0;
    for (int d = 0; d < curr_days; d++){
        if (sol[i][d] != 0 && curr_days >= DL_i[i]){
            work_streak += 1;
            if (curr_days < h){ // Needed to help greedy solution
                if (d % 7 == 5) penalty += PENALTY_COST;
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
        else if(sol[i][d] == 0 && curr_days >= CMI_i[i]){
            // bool off_broken = off_streak < DL_i[i];
            // penalty += off_broken? PENALTY_COST: 0;
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

    // (11 - 12)
    int weekends_worked = 0;
    if (curr_days >= 5) {
        for (int d = 0; d < curr_days; d++){
            weekends_worked += ((d % 7 == 6) || (d % 7 == 5))? sol[i][d] != 0: 0; // (12)
        }
        is_broken_1 = (weekends_worked > FM_i[i]);
        // if (is_broken_1) cout << "Restriccion fin de semanas trabajando rota" << endl;
        penalty += is_broken_1? PENALTY_COST : 0; // Max weekends that can be assigned (11)
        broken_h_constr += is_broken_1;
    }

    penalty = penalty < 0? 10000000 - penalty : penalty; // Stop over flow

    return penalty;
}

int eval_soft_greedy(vector<vector<int>> sol, vector<vector<vector<bool>>> sol_bit){
    //  evaluate soft constraints, adding penalty and counter per worker.
    int curr_staff = sol.size() - 1;
    int curr_days = sol[curr_staff].size();
    int i = curr_staff;
    int penalty = 0;
    int penalty_get = 0;

    for (int t = 1; t < CT; t++){
        for (int d = 0; d < curr_days; d++){
            penalty_get = sol_bit[i][d][t] * PAT_i_d_t[i][d][t];
            penalty += penalty_get;
            penalty_per_worker[i] += penalty_get != 0;

            penalty_get = (sol_bit[i][d][t] == 0) * PNAT_i_d_t[i][d][t];
            penalty += penalty_get;
            penalty_per_worker[i] += penalty_get != 0;
        }
    }

    return penalty;
}

int eval_greedy_sol(vector<vector<int>> sol, vector<vector<vector<bool>>> sol_bit){
    // Returns the total penalty for greedy solution
    int hard_penalty = eval_hard_greedy(sol, sol_bit);
    int soft_penalty = eval_soft_greedy(sol, sol_bit);
    return hard_penalty + soft_penalty;
}

void set_y_from_x(vector<vector<vector<bool>>>& y, int x, int i, int d, int t){
    // Changes y_i_d_t value and updates y_i_d_x. CALLED AFTER UPDATING NUMERIC SOLUTION
    y[i][d][t] = 0;
    y[i][d][x] = 1;
}

void set_start_point(vector<vector<int>>& sol, vector<vector<vector<bool>>>& sol_bit){
    int turn = 1;
    sol.push_back(vector<int>(1, turn));
    sol_bit.push_back(vector<vector<bool>>(1, vector<bool>(CT, false)));
    sol_bit[0][0][turn] = true;
}

void solve_greedy(vector<vector<int>>& sol, vector<vector<vector<bool>>>& sol_bit){
    // Checks turns per worker per day, and chooses the one that minimizes the function.
    for (int i = 0; i < n; i++){
        for (int d = 0; d < h; d++){
            int original_t = sol[i][d];
            int min_eval = eval_greedy_sol(sol, sol_bit);
            int min_turn = original_t;

            for (int t = 0; t < CT; t++){
                if (t == original_t) continue;

                int prev_shift = sol[i][d];
                sol[i][d] = t;
                set_y_from_x(sol_bit, t, i, d, prev_shift);
                int new_eval = eval_greedy_sol(sol, sol_bit);
                int is_min = new_eval <= min_eval;
                min_eval = is_min? new_eval : min_eval;
                min_turn = is_min? t : min_turn;
            }
            cout << min_eval << endl;
            int prev_shift = sol[i][d];
            sol[i][d] = min_turn;
            set_y_from_x(sol_bit, min_turn, i, d, prev_shift);

            if (d < h - 1){
                sol[i].push_back(1);
                sol_bit[i].push_back(vector<bool>(CT, false));
                sol_bit[i][d+1][1] = true;
            }
        }

        if (i < n - 1){
            sol.push_back(vector<int>(1, 1));
            sol_bit.push_back(vector<vector<bool>>(1, vector<bool>(CT, false)));
            sol_bit[i + 1][0][1] = true;
        }
    }
    return;
}

void print_result(vector<vector<int>>& sol, vector<vector<vector<bool>>>& sol_bit){
    int staff = sol.size();
    int days = sol[staff - 1].size();
    ofstream out_file("Instancia.out");
    out_file << "#Funcion Objetivo" << endl;
    out_file << eval << endl;
    out_file << endl;

    out_file << "Calendario Horizonte " << h << " Dias" << endl;
    out_file << "Entidades/Dias | ";

    for (int d = 0; d < h; d++){
        out_file << d << " | ";
    }
    out_file << endl;
    string end_spaces = " ";
    for (int i = 0; i < n; i ++){
        out_file << staff_map_inv[i] << "              |";
        for (int d = 0; d < h; d++){
            if (d < 10){
                end_spaces = " ";
            }
            else if (d < 100){
                end_spaces = "  ";
            }
            else{
                end_spaces = "   ";
            }
            string shift = shift_map_inv[sol[i][d]];
            out_file << end_spaces << shift << " |";
        }
        out_file << endl;
    }
    out_file << endl;

    out_file << "#Penalizaciones por entidad" << endl;
    for (int i = 0; i < n; i ++){
        out_file << staff_map_inv[i] << ", " << penalty_per_worker[i] << endl;
    }
    out_file << endl;

    out_file << "#Tabla cobertura de turnos" << endl;
    
    out_file << "Turnos/Dias |  ";
    for (int d = 0; d < h; d++){
        out_file << d << "  |  ";
    }
    out_file << endl;
      
    for (int t = 1; t < CT; t++){
        out_file << shift_map_inv[t] << "           | ";
        for (int d = 0; d < h; d++){
            out_file << day_turn_qty[d][t] << "/" << S_d_t[d][t] << " | ";
        }
        out_file << endl;
    }

    out_file << "Costo Total | ";
    for (int d = 0; d < h; d++){
        int total = 0;
        for (int t = 1; t < CT; t++){
            total += penalty_day_turn[d][t];
        }
        out_file << total << " | ";
    }
    out_file << endl;
    out_file << endl;

    out_file << "#Restricciones duras no respetadas" << endl;
    out_file << broken_h_constr << endl; 
    out_file.close();
}

int main()
{
    read_instance();
    convert_auxiliars();
    vector<vector<vector<bool>>> sol_bit;
    vector<vector<int>> sol;
    set_start_point(sol, sol_bit);
    solve_greedy(sol, sol_bit);
    eval = eval_sol(sol, sol_bit);
    print_result(sol, sol_bit);
    return 0;
}