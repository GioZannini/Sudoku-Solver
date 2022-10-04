#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <array>
#include <algorithm>


class Read_sudoku_matrices {
public:
	Read_sudoku_matrices(){}

	
	// read the csv file and save the matrix
	void read(std::string path, unsigned int from = 0,  unsigned int to = 10000000) {
		// clear box
		box_of_sudoku.clear();

		// open file
		std::ifstream file(path);
		// control the file
		if (!file.is_open()) throw std::runtime_error("Could not open file");

		// hold a line for time
		std::string line;
		// hold the right way to store each line
		unsigned int count_line = 0;

		// retrieve the line
		while (std::getline(file, line)) {
			// in this way I can jump the title of the csv
			if (count_line >= from && count_line != 0) {
				save_in_box(line);
			}

			if (count_line == to) {
				break;
			}
			count_line++;
		}
		file.close();
	}


	// oredering by difficulty
	void ordered_by_difficulty() {
		std::sort(box_of_sudoku.begin(), box_of_sudoku.end(), Compararator());
	}


	// return the pointer to begin
	auto begin() {
		return box_of_sudoku.begin();
	}


	// return the pointer to end
	auto end() {
		return box_of_sudoku.end();
	}


private:
	// box that contains a pair with (problem, solve)
	std::vector<std::tuple<std::vector<std::vector<char>>, std::vector<std::vector<char>>, double>> box_of_sudoku;

	// custom comparator to order the sudoku
	class Compararator {
	public:
		bool operator()(const std::tuple<std::vector<std::vector<char>>, std::vector<std::vector<char>>, double>& a, const std::tuple<std::vector<std::vector<char>>, std::vector<std::vector<char>>, double>& b) {
			return std::get<2>(a) < std::get<2>(b);
		}
	};
	


	// create for each sudoku a pair problem, solution
	void save_in_box(std::string line) {

		// object that contains line
		std::istringstream line2(line);
		// indicates what I see
		us_int pos = 0;
		// save the matrix for quiz and the matrix for solution
		std::array<std::vector<std::vector<char>>, 2> problem_and_solution_tmp;
		// store the difficulty of the current sudoku
		float difficulty;
		for (std::string piece; std::getline(line2, piece, ',');) {

			// quiz
			if (pos == 1) {
				// save each row of sudoku
				std::vector<char> row_by_row;

				for (us_int i = 0; i < piece.size(); i++) {
					if (i % 9 == 0 && i != 0) {
						// insert the char line into vector
						problem_and_solution_tmp[0].push_back(row_by_row);
						// clear the line
						row_by_row.clear();
					}
					// insert the char
					row_by_row.push_back(piece[i]);
				}
				// insert the last line into matrix
				problem_and_solution_tmp[0].push_back(row_by_row);
			}

			// sol
			else if (pos == 2) {
				// save each row of sudoku
				std::vector<char> row_by_row;

				for (us_int i = 0; i < piece.size(); i++) {
					if (i % 9 == 0 && i != 0) {
						// insert the char line into vector
						problem_and_solution_tmp[1].push_back(row_by_row);
						// clear the line
						row_by_row.clear();
					}
					// insert the char
					row_by_row.push_back(piece[i]);
				}
				// insert the last line into matrix
				problem_and_solution_tmp[1].push_back(row_by_row);
			}

			// diff
			else if (pos == 4) {
				difficulty = std::stof(piece);
			}

			pos++;
		}
		box_of_sudoku.push_back(std::tuple<std::vector<std::vector<char>>, std::vector<std::vector<char>>, double>(problem_and_solution_tmp[0], problem_and_solution_tmp[1], difficulty));

	}
};




class write_report_sudoku {
public:
	write_report_sudoku(std::string path, std::vector<std::string> columns) : report(path) {
		// initialize the columns
		for (auto st : columns) {
			report << st << ";";
		}
		report << "\n";
	}

	void write_line(std::vector<std::string> line) {
		for (auto st : line) {
			report << st << ";";
		}
		report << "\n";
	}

	void close() {
		report.close();
	}


private:
	std::ofstream report;
};
