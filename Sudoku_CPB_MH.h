#ifndef SUDO_SOLVER_BACKTC_OPT_H
#define SUDO_SOLVER_BACKTC_OPT_H

#include <vector>
#include <map>
#include <queue>
#include <set>
#include "Type_alias.h"


namespace CPB_MH{


class Sudoku {
public:

	Sudoku(char blank) : BLANK(blank) {}


	// insert the matrix to solve and inizialize all variables
	void insert_matrix(const std::vector<std::vector<char>>& _matrix) {
		// clear all structures
		clear_all();
		// assignes the matrix
		matrix = _matrix;
		// populate heap and map
		determine_numbers_that_can_use();

	}


	// print all matrix
	void printer() {
		for (unsigned int i = 0; i < matrix.size(); i++) {
			for (unsigned int j = 0; j < matrix.size(); j++) {
				std::cout << matrix[i][j] << " ";
			}
			std::cout << "\n";
		}
	}


	// solve the sudoku
	void solve() {
		explore_solutions();
	}


	// return the matrix
	std::vector<std::vector<char>> get_matrix() {
		return matrix;
	}


	// give the number of branch for the present computation
	unsigned int get_number_of_branch() {
		return number_of_branch;
	}

	// give the number of back tracking for the present computation
	unsigned int get_number_of_back_tracking() {
		return number_of_back_tracking;
	}

	// create the operator of assignment
	Sudoku& operator=(const Sudoku& a) {
		if (this != &a) {
			map = a.map;
			heap = a.heap;
			BLANK = a.BLANK;
			matrix = a.matrix; 
			numbers_to_insert = a.numbers_to_insert;
		}
		return *this;
	}


	// control if the solution is a rigth solution
	bool is_correct() {
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


		// initialize the domain
		void initialize() {
			//watch the row
			for (us_int c = 0; c < sudo.matrix.size(); c++) {
				if (sudo.matrix[position.first][c] != sudo.BLANK) {
					delete_elem_from_domain(convert_char_to_int(position.first, c));
				}
			}

			//watch the col
			for (us_int r = 0; r < sudo.matrix.size(); r++) {
				if (sudo.matrix[r][position.second] != sudo.BLANK) {
					delete_elem_from_domain(convert_char_to_int(r, position.second));
				}
			}

			//watch the square
			pairs tmp_r = bound(position.first);
			pairs tmp_c = bound(position.second);
			for (us_int r = tmp_r.first; r <= tmp_r.second; r++) {
				for (us_int c = tmp_c.first; c <= tmp_c.second; c++) {
					if (sudo.matrix[r][c] != sudo.BLANK) {
						// remove the element that can't be in the domain
						delete_elem_from_domain(convert_char_to_int(r, c));
					}
				}
			}
		}


		// given the matrix it finds which white cell are related with this
		std::set<pairs> white_cell_relationated_with_me() {
			// set to contain all white cell related with me
			std::set<pairs> tmp;

			// control row
			for (unsigned int i = 0; i < sudo.matrix.size(); i++) {
				if (sudo.matrix[i][position.second] == sudo.BLANK) {
					tmp.insert(pairs(i, position.second));
				}
			}
			// control col
			for (unsigned int i = 0; i < sudo.matrix.size(); i++) {
				if (sudo.matrix[position.first][i] == sudo.BLANK) {
					tmp.insert(pairs(position.first, i));
				}
			}
			// control square
			pairs tmp_r = bound(position.first);
			pairs tmp_c = bound(position.second);
			for (us_int r = tmp_r.first; r <= tmp_r.second; r++) {
				for (us_int c = tmp_c.first; c <= tmp_c.second; c++) {
					if (sudo.matrix[r][c] == sudo.BLANK) {
						tmp.insert(pairs(r, c));
					}
				}
			}
			return tmp;
		}

		
		// create the operator of assignment
		Domain& operator=(const Domain& a) {
			if (this != &a) {
				my_dom = a.my_dom;
				position = a.position;
				sudo = a.sudo;
			}
			return *this;
		}
		

		// delete an element from the domain
		bool delete_elem_from_domain(us_int elem) {
			std::_Node_handle val = my_dom.extract(elem);
			return !val.empty();
		}


		
		// holds the domain
		std::set<us_int> my_dom = { 1,2,3,4,5,6,7,8,9 };

		// holds the position in matrix for this domain
		pairs position;

	private:

		// instance to declare the relation between this and external class
		Sudoku& sudo;


		// convert char number in int number
		inline us_int convert_char_to_int(us_int row, us_int col) {
			return static_cast<us_int> (sudo.matrix[row][col] - 48);
		}


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
			return pairs(tmp_s, tmp_d);;
		}
	};



private:

	// custom comparator for min_heap
	class Compare_Min {
	public:
		bool operator()(const Domain* a, const Domain* b) const {
			return a->my_dom.size() > b->my_dom.size();
		}
	};


	// custom heap
	class Heap : public std::priority_queue <Domain*, std::vector<Domain*>, Compare_Min> {
	public:
		//rebalancing the structure
		void rebalance() {
			std::make_heap(this->c.begin(), this->c.end(), this->comp);
		}
		void clear() {
			this->c.clear();
		}
	};

	
	// map that hold the domain for each white cell
	std::map<pairs, Domain> map;  

	// heap that hold the pointer to domain ordering by domain size
	Heap heap;

	// the symbol to identify the white space
	char BLANK;

	// input matrix
	std::vector<std::vector<char>> matrix;

	// numbers of number that must be inserted in the matrix 
	us_int numbers_to_insert;

	// count the number of branch
	unsigned int number_of_branch = 0;

	// count the number of backtracking
	unsigned int number_of_back_tracking = 0;

	// clear all variables
	void clear_all() {
		matrix.clear();
		map.clear();
		heap.clear();
		numbers_to_insert = 0;
		number_of_branch = 0;
		number_of_back_tracking = 0;
	}

	
	// popolate occurrence_of_numbers with right numbers
	void determine_numbers_that_can_use() {
		// watch the matrix row by row
		for (us_int i = 0; i < matrix.size(); i++) {
			for (us_int j = 0; j < matrix.size(); j++) {
				if (matrix[i][j] == BLANK) {
					// increment numbers of white spaces
					numbers_to_insert++;
					// create a pairs with coordinates
					pairs tmp_pos(i, j);
					// create domain and is inserted in map with key row,col
					map.insert_or_assign(tmp_pos, Domain(tmp_pos, *this));
					// take the domain in pos row,col and create a pointer that is inserted in heap
					heap.push(&((map.find(tmp_pos))->second));
				}
			}
		}
	}


	// write on matrix the value
	inline void write_on_matrix(pairs p, us_int value) {
		matrix[p.first][p.second] = value + '0';
	}


	// overload write on matrix the BLANK
	inline void write_on_matrix(pairs p) {
		matrix[p.first][p.second] = BLANK;
	}


	// control if there aren't anymore numbers to insert in the matrix
	inline bool finish_white_space() {
		return numbers_to_insert == 0;
	}


	// update the domain
	void update_domain(Domain& d, us_int elem, std::set<pairs>& occ){
		// find all white cell related with this domain
		std::set<pairs> white_cell_related = d.white_cell_relationated_with_me();
		// iterator to view all white cell
		auto white_cell_it = white_cell_related.begin();
		while (white_cell_it != white_cell_related.end()) {
				// if this elem is in this cell then eliminate it 
			if ((map.find(*white_cell_it))->second.delete_elem_from_domain(elem)) {
				// inserted the position inside occurrence set
				occ.insert(*white_cell_it);
			}
			white_cell_it++;
		}
		//rebalancing of heap
		heap.rebalance();
	}


	// restore the domain if the branch that was taken was wrong
	void restore_domain(Domain& d, us_int elem, std::set<pairs>& occ) {
		auto it = occ.begin(); // iterator of set
		while (it != occ.end()) {
			// iterator of map
			auto it2 = map.find(*it); 
			// reinsert the element inside domain of specific white cell
			it2->second.my_dom.insert(elem);
			it++;
		}
		//rebalancing of heap
		heap.rebalance();
	}


	// explore the path to find the solution of sudoku
	bool explore_solutions() {
		number_of_branch++;
		// control if the heap is empty
		if (heap.empty()) {
			return true;
		}
		// extract the white cell with the smallest domain
		Domain* elem = heap.top();
		// remove this value from heap
		heap.pop();
		// take the iterator to watch all possible domain for this position
		auto candidate_pointer = elem->my_dom.begin(); 
		// cycle to try all branches
		while (candidate_pointer != elem->my_dom.end() && !finish_white_space()) {
			// create a set to remember in which cells the domain has been changed
 			std::set<pairs> occ;
			// write on matrix the value
			write_on_matrix(elem->position, *candidate_pointer); 
			// update the domain of all white cell that are related with the cell extract before
			update_domain(*elem, *candidate_pointer, occ); 
			// descrese the total number that can be inserted in the matrix
			numbers_to_insert--; 
			// continue with the visit of tree
			bool res = explore_solutions(); 
			// if the recursion return false
			if (!res) { 
				// increase the total number that can be inserted in the matrix
				numbers_to_insert++; 
				// restore the domain of all white cell that are related with the cell extract before
				restore_domain(*elem, *candidate_pointer, occ); 
				// sostitute value in the matrix by BLANK
				write_on_matrix(elem->position);
			}
			// try the next candidate for domain
			candidate_pointer++;
		}
		// if don't exist any number that can be inserted in the matrix
		if (finish_white_space()) {  
			return true;
		}
		else {
			number_of_back_tracking++;
			// reinsert the element in heap
			heap.push(elem);
			return false;
		}
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


	// control if any row have all numbers
	bool check_row(us_int row) {
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
	bool check_col(us_int col) {
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
};
}

#endif