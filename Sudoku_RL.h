#ifndef SUDO_SOLVER_RELAXING_LABELS_H
#define SUDO_SOLVER_RELAXING_LABELS_H

#include <vector>
#include <array>
#include "Type_alias.h"
#include <chrono>



namespace RL {

	class Sudoku {
	public:

		Sudoku(char blank) : BLANK(blank), rel(Relaxing_labels(*this)) {}


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
			rel.clear_all();
			// assignes the matrix
			matrix = _matrix;
			// initialize the relaxing labels class
			rel.initialize();
		}


		// solve the sudoku
		void solve(bool parall = true) {
			if (parall) {
				rel.parall_compute_solution();
			}
			else{
				rel.compute_solution();
			}
			for (auto it_sol : rel.return_solution()) {
				write_on_matrix(it_sol.first, it_sol.second);
			}
		}


		// control if the algorithm is terminated in right mode
		bool out_of_time() {
			return rel.out_of_time;
		}



		unsigned int get_while_step() {
			return rel.while_step;
		}


		// return the matrix
		std::vector<std::vector<char>> get_matrix() {
			return matrix;
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



		class Relaxing_labels {
		public:

			Relaxing_labels(Sudoku& _sudo) : sudo(_sudo) {}

			// initialize variables
			void initialize() {
				// watch matrix row by row
				for (us_int r = 0; r < sudo.matrix.size(); r++) {
					for (us_int c = 0; c < sudo.matrix.size(); c++) {
						if (sudo.matrix[r][c] == sudo.BLANK) {
							//build the right matrix
							initialize(r, c);
						}
					}
				}
				initialize_Q();
			}

			// compute the solution for sudoku 
			void compute_solution(){
				// start time
				auto start = std::chrono::steady_clock::now();
				// measure the difference about average consistency between P and P_next
				double dst = 0.f;
				do {
					while_step++;
					// control if the algorithm is gone out of time (10 min)
					if (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count() > 600000) {
						out_of_time = true;
						break;
					}

					dst = average_consistency();
					P = P_next;
					compute_P_next();
					compute_Q_next();
					// calculate the difference between P and P_next
					dst = std::abs(dst - average_consistency());
				} while (dst > 0.01);
				P = P_next;
			}


			// compute the solution for sudoku 
			void parall_compute_solution() {
				// start time
				auto start = std::chrono::steady_clock::now();
				// measure the difference about average consistency between P and P_next
				double dst = 0.f;
				do {
					while_step++;
					// control if the algorithm is gone out of time (10 min)
					if (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count() > 600000) {
						out_of_time = true;
						break;
					}

					dst = parall_average_consistency();
					P = P_next;
					parall_compute_P_next();
					parall_compute_Q_next();
					// calculate the difference between P and P_next
					dst = std::abs(dst - parall_average_consistency());
				} while (dst > 0.01);
				P = P_next;
			}


			// return the pair (position, value) to assign in each white cell
			std::vector<std::pair<pairs, us_int>> return_solution() {
				std::vector<std::pair<pairs, us_int>> v_p;
				auto it_P = P.begin();
				while (it_P != P.end()) {
					us_int max_lambda = 0;
					// find the lambda with max probability
					for (us_int i = 1; i < it_P->second.size(); i++) {
						if (P[it_P->first][max_lambda] < P[it_P->first][i]) {
							max_lambda = i;
						}
					}
					v_p.push_back(std::pair<pairs, us_int>(it_P->first, max_lambda + 1));
					it_P++;
				}
				return v_p;
			}


			void clear_all() {
				P.clear();
				P_next.clear();
				Q.clear();
				out_of_time = false;
				while_step = 0;
			}



			// determine if the time is over
			bool out_of_time = false;
			// numbers fo while step done
			unsigned int while_step = 0;

		private:

			// instance to declare the relation between this and external class
			Sudoku& sudo;

			// probability matrix at time t where each row is a white cell and each column is the probability for each label
			std::map<pairs, std::array<double, 9>> P;
			// probability matrix at time t + 1
			std::map<pairs, std::array<double, 9>> P_next;
			// relation matrix among white cells
			std::map<pairs, std::array<double, 9>> Q;
			
			
			
			// initialize the domain
			void initialize(us_int row, us_int col) {
				// set that indicates which numbers miss the probability, so at the end probabilities will come assigned
				std::set<us_int> set_numbers = { 1,2,3,4,5,6,7,8,9 };
				//watch the row
				for (us_int c = 0; c < sudo.matrix.size(); c++) {
					if (sudo.matrix[row][c] != sudo.BLANK) {
						us_int value = sudo.convert_char_to_int(row, c);
						set_numbers.extract(value);
					}
				}
				//watch the col
				for (us_int r = 0; r < sudo.matrix.size(); r++) {
					if (sudo.matrix[r][col] != sudo.BLANK) {
						us_int value = sudo.convert_char_to_int(r, col);
						set_numbers.extract(value);
					}
				}
				//watch the square
				pairs tmp_r = bound(row);
				pairs tmp_c = bound(col);
				for (us_int r = tmp_r.first; r <= tmp_r.second; r++) {
					for (us_int c = tmp_c.first; c <= tmp_c.second; c++) {
						if (sudo.matrix[r][c] != sudo.BLANK) {
							// remove the element that can't be in the domain
							us_int value = sudo.convert_char_to_int(r, c);
							set_numbers.extract(value);
						}
					}
				}
				// temporary array to store domain's probability for white cell in position (row, col)
				std::array<double, 9> tmp_prob{0.f,0.f,0.f,0.f,0.f,0.f,0.f,0.f,0.f};
				// assign probability to each element in domain
				for (us_int v : set_numbers) {
					tmp_prob[v - 1] = static_cast<double> (1.f / set_numbers.size());
				}
				// assignament
				P.insert_or_assign(pairs(row, col), tmp_prob);
				P_next = P;
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


			// verify if a cell is related with an other
			bool are_related(const pairs& p1, const pairs& p2) {
				pairs tmp_r = bound(p1.first);
				pairs tmp_c = bound(p1.second);
				if (p1.first == p2.first || p1.second == p2.second ||
					(tmp_r.first <= p2.first && p2.first <= tmp_r.second && tmp_c.first <= p2.second && p2.second <= tmp_c.second)) {
					return true;
				}
				else {
					return false;
				}
			}


			// intialize the matrix for compare
			void initialize_Q() {
				std::array<us_int, 9> labels = { 1,2,3,4,5,6,7,8,9 };
				std::array<double, 9> correlations{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f};
				// construct the matrix space
				for (auto it_i : P) {
					Q.insert_or_assign(it_i.first, correlations);
					for (auto lambda : labels) {

						for (auto it_j : P) {

							for (auto mu : labels) {
								Q[it_i.first][lambda - 1] += strength_compatibility(it_i.first, it_j.first, lambda, mu) * P[it_j.first][mu - 1];
							}
						}
					}
				}
			}


			// compute the Q from P_next
			void compute_Q_next() {
				std::array<us_int, 9> labels = { 1,2,3,4,5,6,7,8,9 };
				// construct the matrix space
				for (auto it_i : P) {
		
					for (auto lambda : labels) {
						// restore variable
						Q[it_i.first][lambda - 1] = 0.f;

						for (auto it_j : P) {

							for (auto mu : labels) {
								Q[it_i.first][lambda - 1] += strength_compatibility(it_i.first, it_j.first, lambda, mu) * P_next[it_j.first][mu - 1];
							}
						}
					}
				}
			}


			// compute the Q from P_next
			void parall_compute_Q_next() {
				std::array<us_int, 9> labels = { 1,2,3,4,5,6,7,8,9 };
				// construct the matrix space
				for (auto it_i : P) {

					for (auto lambda : labels) {
						// restore variable
						Q[it_i.first][lambda - 1] = 0.f;

						for (auto it_j : P) {

							double amm = 0.f;
							#pragma omp parallel reduction(+:amm)
							{
								#pragma omp for
								for (int mu = 0; mu < labels.size(); mu++) {
									amm += strength_compatibility(it_i.first, it_j.first, lambda, mu+1) * P_next[it_j.first][mu];
								}
							}
							Q[it_i.first][lambda - 1] += amm;
						}
					}
				}
			}


			// determine the strength compatibility between two domains and the numbers
			inline us_int strength_compatibility(const pairs& i, const pairs& j, us_int lambda, us_int mu) {
				if (i == j) {
					return 0;
				}
				else if (lambda != mu) {
					return 1;
				}
				else if (are_related(i, j)) {
					return 0;
				}
				else {
					return 1;
				}
			}


			// compute the matrix at time t + 1
			void compute_P_next() {
				auto P_next_it = P_next.begin();
				while (P_next_it != P_next.end()) {
					for (us_int lambda = 0; lambda < P_next_it->second.size(); lambda++) {
						double denom = 0.f;
						for (us_int mu = 0; mu < P[P_next_it->first].size(); mu++) {
							denom += P[P_next_it->first][mu] * Q[P_next_it->first][mu];
						}
						P_next[P_next_it->first][lambda] = (P[P_next_it->first][lambda] * Q[P_next_it->first][lambda]) / denom;
					}
					P_next_it++;
				}
			}


			// compute the matrix at time t + 1
			void parall_compute_P_next() {
				auto P_next_it = P_next.begin();
				while (P_next_it != P_next.end()) {
					for (us_int lambda = 0; lambda < P_next_it->second.size(); lambda++) {
						double denom = 0.f;
						#pragma omp parallel reduction(+:denom)
						{
							#pragma omp for
							for (int mu = 0; mu < P[P_next_it->first].size(); mu++) {
								denom += P[P_next_it->first][mu] * Q[P_next_it->first][mu];
							}
						}
						P_next[P_next_it->first][lambda] = (P[P_next_it->first][lambda] * Q[P_next_it->first][lambda]) / denom;
					}
					P_next_it++;
				}
			}


			// control teh difference between matrix P and P_next
			double average_consistency(){
				double tmp_PN = 0.f;
				auto it_P = P_next.begin();
				while (it_P != P_next.end()) {
					for (us_int lambda = 0; lambda < it_P->second.size(); lambda++) {
						tmp_PN += P_next[it_P->first][lambda] * Q[it_P->first][lambda];

					}
					it_P++;
				}
				return tmp_PN;
			}


			// control the difference between matrix P and P_next
			double parall_average_consistency() {
				double tmp_PN = 0.f;
				auto it_P = P_next.begin();
				while (it_P != P_next.end()) {
					#pragma omp parallel reduction(+:tmp_PN)
					{
						#pragma omp for 
						for (int lambda = 0; lambda < it_P->second.size(); lambda++)
						{
							tmp_PN += P_next[it_P->first][lambda] * Q[it_P->first][lambda];
						}
					}
					it_P++;
				}
				return tmp_PN;
			}

		};


	private:

		// the symbol to identify the white space
		char BLANK;

		// input matrix
		std::vector<std::vector<char>> matrix;

		Relaxing_labels rel;


		// convert char number in int number
		inline us_int convert_char_to_int(us_int row, us_int col) {
			return matrix[row][col] - 48;
		}


		// write on matrix the value
		inline void write_on_matrix(pairs p, us_int value) {
			matrix[p.first][p.second] = value + '0';
		}


		// clear all instances
		void clear_all() {
			matrix.clear();
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