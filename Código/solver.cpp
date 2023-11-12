#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <map>
#include <bits/stdc++.h>
#include "greedy.cpp"
using namespace std;


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
    Turns_left_to_i.resize(n, vector<int>(CT, 0));
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
    cout << "Greedy resuelto: " << eval << endl; 
    tabu_search(sol, sol_bit);

    cout << "Informacion sobre solución final" << endl;
    eval = eval_sol(sol, sol_bit);
    print_result(sol, sol_bit);
    return 0;
}