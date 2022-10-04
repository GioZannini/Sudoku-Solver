#include <iostream>
#include <chrono>
#include <array>
#include <iomanip>
#include "Sudoku_CPB.h"
#include "Sudoku_CPB_MH.h"
#include "Sudoku_RL.h"
#include "csv_operations.h"


// NB: To compile this code you need to add the flag -fopenmp and need version C++17




int main(){
   
    std::vector<std::string> columns{ "CPB time", "CPB correct", "CPB visited node", "CPB backtrack", "CPB time MH", "CPB correct MH", "CPB visited node MH", "CPB backtrack MH", 
                                      "RL time P", "RL correct P", "RL time", "RL correct", "RL steps to converge", "Difficulty" };
    // create a csv file with a columns
    write_report_sudoku wr_report("Report.csv", columns);
    // create a instance to read sudoku
    Read_sudoku_matrices sudoku_file;
    // read sudoku from a file
    sudoku_file.read("tiny_db_from_sudoku_3m.csv");
    // ordered from the simplest to the most difficult
    sudoku_file.ordered_by_difficulty();


    // create instances for each method
    CPB::Sudoku s1('.');
    CPB_MH::Sudoku s2('.');
    RL::Sudoku s4('.');
    RL::Sudoku s5('.');
    
    // pointer
    auto matrices_sudoku = sudoku_file.begin();
    // counter to know how many sudoku are done
    us_int count = 1;
 
    while (matrices_sudoku != sudoku_file.end()) {
        // upload bar
        std::cout << "\n" << count << " ";

        // create a vector for report in a line
        std::vector<std::string> v_report;

        // insert the matrix in all sudoku solver
        s1.insert_matrix(std::get<0>(*matrices_sudoku));
        s2.insert_matrix(std::get<0>(*matrices_sudoku));
        s4.insert_matrix(std::get<0>(*matrices_sudoku));
        s5.insert_matrix(std::get<0>(*matrices_sudoku));
        


        
        //------------------------------------------------------------------------------------------------------FIRST
        auto start = std::chrono::steady_clock::now();
        s1.solve();
        auto end = std::chrono::steady_clock::now();

        v_report.push_back(std::to_string(std::chrono::duration_cast<std::chrono::microseconds>(end - start).count()));
        v_report.push_back((s1.is_correct()) ? "1" : "0");
        v_report.push_back(std::to_string(s1.get_number_of_branch()));
        v_report.push_back(std::to_string(s1.get_number_of_back_tracking()));

        // upload bar
        std::cout << "-";
        
        //------------------------------------------------------------------------------------------------------SECOND
        start = std::chrono::steady_clock::now();
        s2.solve();
        end = std::chrono::steady_clock::now();

        v_report.push_back(std::to_string(std::chrono::duration_cast<std::chrono::microseconds>(end - start).count()));
        v_report.push_back((s2.is_correct()) ? "1" : "0");
        v_report.push_back(std::to_string(s2.get_number_of_branch()));
        v_report.push_back(std::to_string(s2.get_number_of_back_tracking()));

        // upload bar
        std::cout << "-";
        
        //------------------------------------------------------------------------------------------------------FOURTH
        start = std::chrono::steady_clock::now();
        s4.solve();
        end = std::chrono::steady_clock::now();

        v_report.push_back(std::to_string(std::chrono::duration_cast<std::chrono::microseconds>(end - start).count()));
        v_report.push_back((s4.is_correct()) ? "1" : "0");

        // upload bar
        std::cout << "-";
        
        //------------------------------------------------------------------------------------------------------FIFTH
        start = std::chrono::steady_clock::now();
        s5.solve(false);
        end = std::chrono::steady_clock::now();

        v_report.push_back(std::to_string(std::chrono::duration_cast<std::chrono::microseconds>(end - start).count()));
        v_report.push_back((s5.is_correct()) ? "1" : "0");

        v_report.push_back(std::to_string(s5.get_while_step()));
        // upload bar
        std::cout << "-";
        //------------------------------------------------------------------------------------------------------
        

        // insert the difficulty with specific number after comma
        std::stringstream stream;
        stream << std::fixed << std::setprecision(2) << std::get<2>(*matrices_sudoku);
        v_report.push_back(stream.str());

        // write inside a csv
        wr_report.write_line(v_report);

        // increment the counters
        count++;
        matrices_sudoku++;    
    }
    wr_report.close();
    
    return 0;
}