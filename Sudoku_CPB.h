#ifndef SUDO_SOLVER_BACKTC_H
#define SUDO_SOLVER_BACKTC_H

#include <vector>
#include "Type_alias.h"


namespace CPB {

class Sudoku {
public:

	Sudoku(char blank) : BLANK(blank){}


	// print all matrix
	void printer() {
		for (unsigned int i = 0; i < matrix.size(); i++) {
			for (unsigned int j = 0; j < matrix.size(); j++) {
				std::cout << matrix[i][j] << " ";
			}
			std::cout << "\n";
		}	
	}


	// insert the matrix to solve and inizialize all variables
	void insert_matrix(const std::vector<std::vector<char>>& _matrix) {
		// clear all structures
		clear_all();
		// assignes the matrix
		matrix = _matrix;
		// occurrence_of_numbers
		determine_numbers_that_can_use();
	}


	// give the number of branch for the present computation
	unsigned int get_number_of_branch() {
		return number_of_branch;
	}


	// give the number of back tracking for the present computation
	unsigned int get_number_of_back_tracking() {
		return number_of_back_tracking;
	}


	// solve the sudoku
	void solve() {
		pairs pos = first_empty_from_here(0, 0);
		explore_solutions(Domain(pos, *this));
	}


	// return the matrix
	std::vector<std::vector<char>> get_matrix() {
		return matrix;
	}

	// control if the solution is a rigth solution
	bool is_correct(){
		bool row;
		bool col;
		bool square;
		for (us_int i = 0; i < matrix.size(); i++)
		{
			row = check_row(i);
			col = check_col(i);
			if (!row || !col)
				return false;
		}
		for (us_int i = 0; i < matrix.size(); i += 3)
		{
			for (us_int j = 0; j < matrix.size(); j += 3)
			{
				square = check_square(i, j);
				if (!square)
					return false;
			}
		}
		return true;
	}

	
	// class that holds the domain and the position for the domain for each value that is inserted in the matrix
	class Domain {
	public:

		Domain(pairs& p, Sudoku& _sudo) : sudo(_sudo), position(p) {
			//build the right domain
			initialize();
		}


		void initialize() {
			//watch the row
			for (us_int c = 0; c < sudo.matrix.size(); c++) {
				if (sudo.matrix[position.first][c] != sudo.BLANK) {
					delete_elem_from_domain(sudo.convert_char_to_int(position.first, c));
				}
			}

			//watch the col
			for (us_int r = 0; r < sudo.matrix.size(); r++) {
				if (sudo.matrix[r][position.second] != sudo.BLANK) {
					delete_elem_from_domain(sudo.convert_char_to_int(r, position.second));
				}
			}

			//watch the square
			pairs tmp_r = bound(position.first);
			pairs tmp_c = bound(position.second);
			for (us_int r = tmp_r.first; r <= tmp_r.second; r++) {
				for (us_int c = tmp_c.first; c <= tmp_c.second; c++) {
					if (sudo.matrix[r][c] != sudo.BLANK) {
						// remove the element that can't be in the domain
						delete_elem_from_domain(sudo.convert_char_to_int(r, c));
					}
				}
			}
		}


		// holds the domain
		std::vector<us_int> my_dom = { 1,2,3,4,5,6,7,8,9 };

		// holds the position in matrix for this domain
		pairs position;

	private:

		// instance to declare the relation between this and external class
		Sudoku& sudo;


		// determine the boundary for square
		inline pairs bound(us_int pos) {
			us_int tmp_s, tmp_d;
			int value = (pos + 1) % 3;

			if (value == 1) {
				tmp_s = pos;
				tmp_d = pos + 2;
			}

			else if (value == 2) {
				tmp_s = pos - 1;
				tmp_d = pos + 1;
			}

			else {
				tmp_s = pos - 2;
				tmp_d = pos;
			}

			return pairs (tmp_s, tmp_d);;
		}


		// delete an element from the domain
		void delete_elem_from_domain(us_int elem) {
			auto point = my_dom.begin();
			while (point != my_dom.end()) {
				if (elem == *point){
					my_dom.erase(point);
					break;
				}
				point++;
			}
		}

	};



private:

	// the symbol to identify the white space
	const char BLANK;

	// input matrix
	std::vector<std::vector<char>> matrix;

	// numbers of occurence for each value that can be inserted in the matrix 
	std::vector<us_int> occurrence_of_numbers;

	// count the number of branch in the recursion
	unsigned int number_of_branch = 0;

	// count the number of back tracking in the recursion
	unsigned int number_of_back_tracking = 0;



	// clear all variables
	void clear_all() {
		number_of_branch = 0;
		number_of_back_tracking = 0;
		matrix.clear();
		occurrence_of_numbers = { 9, 9, 9, 9, 9, 9, 9, 9, 9 };
	}


	// convert char number in int number
	inline us_int convert_char_to_int (us_int row, us_int col) {
		return matrix[row][col] - 48;
	}


	// popolate occurrence_of_numbers with right numbers
	void determine_numbers_that_can_use() {
		for (us_int i = 0; i < matrix.size(); i++) {
			for (us_int j = 0; j < matrix.size(); j++) {
				if (matrix[i][j] != BLANK) {
					int pos = convert_char_to_int(i, j) - 1; // to find the right position
					occurrence_of_numbers[pos]--;
				}
			}
		}
	}
	

	// write on matrix the value
	inline void write_on_matrix(pairs p, us_int value) {
		matrix[p.first][p.second] = value + '0';
	}


	// find the first white space from here (scan row by row)
	pairs first_empty_from_here(us_int r, us_int c) {
		pairs tmp_pos(0, 0); // pair to store the position
		bool find = false; // indicates if the blank is found
		while (r < matrix.size() && !find) {
			while (c < matrix.size() && !find) {
				if (matrix[r][c] == BLANK) {
					find = true;
					tmp_pos.first = r;
					tmp_pos.second = c;
				}
				c++;
			}
			c = 0; // restart from beginning
			r++;
		}
		return tmp_pos;
	}


	// control if there aren't anymore numbers to insert in the matrix
	bool finish_numbers() {
		auto pointer = occurrence_of_numbers.begin();
		bool control = true;
		while (pointer != occurrence_of_numbers.end() && control) {
			if (*pointer != 0) {
				control = false;
			}
			pointer++;
		}
		return control;
	}

	
	// explore the path to find the solution of sudoku
	bool explore_solutions(Domain d) {
		number_of_branch++;
		auto candidate_pointer = d.my_dom.begin();
		while (candidate_pointer != d.my_dom.end() && !finish_numbers()) {
			write_on_matrix(d.position, *candidate_pointer);
			occurrence_of_numbers[(*candidate_pointer) - 1]--;
			pairs tmp_pos = first_empty_from_here(d.position.first, d.position.second);
			bool res = explore_solutions(Domain(tmp_pos, *this));
			if (!res) {
				occurrence_of_numbers[(*candidate_pointer) - 1]++;
				matrix[d.position.first][d.position.second] = BLANK;
			}
			candidate_pointer++;
		}
		if (finish_numbers()) {
			return true;
		}
		else {
			number_of_back_tracking++;
			return false;
		}
	}


	// control if any row have all numbers
	bool check_row(us_int row){
		std::vector<int> occoronce(9, 0);
		for (us_int j = 0; j < matrix.size(); j++)
		{
			//second occurence of that number
			if (occoronce[(matrix[row][j] - 48) - 1] == 1)
			{
				return false;
			}
			occoronce[(matrix[row][j] - 48) - 1] += 1;
		}
		return true;
	}


	// control if any column have all numbers
	bool check_col(us_int col){
		std::vector<int> occoronce(9, 0);
		for (us_int i = 0; i < matrix.size(); i++)
		{
			//second occurence of that number
			if (occoronce[(matrix[i][col] - 48) - 1] == 1)
			{
				return false;
			}
			occoronce[(matrix[i][col] - 48) - 1] += 1;
		}
		return true;

	}


	// control if any square have all numbers
	bool check_square(us_int row, us_int coloumn)
	{
		us_int start_row = row - (row % 3);
		us_int start_coloumn = coloumn - (coloumn % 3);
		std::vector<int> occoronce(9, 0);
		for (us_int i = 0; i < 3; i++)
		{
			for (us_int j = 0; j < 3; j++)
			{
				//second occurence of that number
				int x = i + start_row;
				int y = j + start_coloumn;
				int elem = matrix[i + start_row][j + start_coloumn] - 48 - 1;
				if (occoronce[(matrix[i + start_row][j + start_coloumn] - 48) - 1] == 1)
				{
					return false;
				}
				occoronce[(matrix[i + start_row][j + start_coloumn] - 48) - 1] += 1;

			}
		}
		return true;
	}


};
}

#endif