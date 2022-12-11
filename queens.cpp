#include <algorithm>
#include <array>
#include <chrono>
#include <fstream>
#include <iostream>
#include <random>
#include <thread>
#include <vector>

// default value for N
// i.e., the number of rows, columns, and queens
unsigned N { 8 };  // default to 8-queens
// queen[r] == c denotes there is a queen at (r, c)
std::vector<unsigned> queen;
// the number of queens assigned to each column
std::vector<unsigned> col_counts;   // for verification of solution only
// the number of queens assigned to each anti-diagonal (top-right to bottom-left)
std::vector<unsigned> d1_counts;
// the number of queens assigned to each diagonal (top-left to bottom-right)
std::vector<unsigned> d2_counts;


void display_grid(void);    // prints the board to console (for reasonable values of N)
void write_to_file(void);   // writes the columns to a text file (for any N)

void debug(void); // prints diagonal counts (for debugging)

// generates an random permuatation of the queens
unsigned initial_placement(void);

// prints the grid to console
// DO NOT use for large grids (e.g., where N > 40)
void display_grid(void) {
    if (N < 45) {
        // grid might fit in console
        const char BLANK = '.';
        const char QUEEN = '*';
        std::cout << ' ' << std::string(N, '_') << std::endl;
        for (const auto &col : queen) {
            std::cout
                << '|' << std::string(col, BLANK) << QUEEN << std::string(N - col - 1, BLANK) << '|'
                << std::endl;
        }
        std::cout << ' ' << std::string(N, '-') << std::endl;
    }
    else {
        // grid is too large for the console
        std::cout << "Cannot display " << N << 'x' << N << " grid." << std::endl;
    }
}

void write_to_file(void) {
    // open a text file (if it already exists, overwrite the old version)
    std::ofstream solution_file("solution.txt", std::ifstream::trunc);
    // write all the queens (except the last)
    for (unsigned i = 0; i < N - 1; i++) {
        solution_file << queen.at(i) << ',';
    }
    // write the last queen, end file with newline
    solution_file << queen.at(N - 1) << std::endl;
}


unsigned total_conflicts(unsigned row1, unsigned row2) {
    // counts all the diagonal conflicts after swapping
    unsigned conflicts = d1_counts.at(row1 + queen.at(row2))
        + d1_counts.at(row2 + queen.at(row1))
        + d2_counts.at(N - 1 + row1 - queen.at(row2))
        + d2_counts.at(N - 1 + row2 - queen.at(row1))
        - 4;
    if (row1 + queen.at(row1) == row2 + queen.at(row2))
        conflicts++;
    if (row1 - queen.at(row1) == row2 - queen.at(row2))
        conflicts++;
    return conflicts;
}


// returns the number of queens that conflict with point (i, queen[i])
unsigned partial_conflicts(unsigned i) {
    unsigned conflicts = d1_counts.at(i + queen.at(i))
        + d2_counts.at(N - 1 + i - queen.at(i))
        - 2;
    return conflicts;
}


// used only in initial_placement
// swaps the queens and adjusts counts as required
void partial_swap(unsigned row1, unsigned row2) {
    // increment counts at revised positions
    d1_counts.at(row1 + queen.at(row2))++;
    d2_counts.at(N - 1 + row1 - queen.at(row2))++;
    // perform the swap
    std::swap( queen[row1], queen[row2] );
}


// used only in initial_placement to revert a partial swap
void undo_partial_swap(unsigned row1, unsigned row2) {
    // revert the swap
    std::swap( queen[row1], queen[row2] );
    // revert to old counts
    d1_counts.at(row1 + queen.at(row2))--;
    d2_counts.at(N - 1 + row1 - queen.at(row2))--;

}


// a regular swap with regular counting
void swap_colummns(unsigned row1, unsigned row2) {
    // decrement counts at old positions
    if (d1_counts.at(row1 + queen.at(row1)) != 0) d1_counts.at(row1 + queen.at(row1))--;
    if (d1_counts.at(row2 + queen.at(row2)) != 0) d1_counts.at(row2 + queen.at(row2))--;
    if (d2_counts.at(N - 1 + row1 - queen.at(row1)) != 0) d2_counts.at(N - 1 + row1 - queen.at(row1))--;
    if (d2_counts.at(N - 1 + row2 - queen.at(row2)) != 0) d2_counts.at(N - 1 + row2 - queen.at(row2))--;
    // increment counts at revised positions
    d1_counts.at(row1 + queen.at(row2))++;
    d1_counts.at(row2 + queen.at(row1))++;
    d2_counts.at(N - 1 + row1 - queen.at(row2))++;
    d2_counts.at(N - 1 + row2 - queen.at(row1))++;
    // perform the swap
    std::swap( queen[row1], queen[row2] );
}

// inspired by Sosic and Gu, 1994
unsigned initial_placement(void) {
    // start with an assignment with no row or column conflicts
    // specifically, all queens on the main diagonal
    for (unsigned i = 0; i != N; i++) {
        queen.at(i) = i;
    }
    // every column has exactly 1 value assigned
    std::fill(std::begin(col_counts), std::end(col_counts), 1U);
    // assume all diagonals have no queens (for now)
    std::fill(std::begin(d1_counts), std::end(d1_counts), 0U);
    std::fill(std::begin(d2_counts), std::end(d2_counts), 0U);

    // for generating random rows
    auto seed = std::chrono::system_clock::now().time_since_epoch().count();
    auto random_engine = std::default_random_engine(seed);
    std::uniform_int_distribution<> rand_row(0, N - 1);

    unsigned row1 = 0U;
    
    for (unsigned i = 0; i != 3 * N; i++) {
        // generate a random pair of rows
        unsigned row2 = rand_row(random_engine);
        // swap the rows
        partial_swap(row1, row2);

        if ( partial_conflicts(row1) == 0 ) {
            // swap added no conflicts; move on to the next row
            if (++row1 == N - 1) {
                // stop looping if all but 1 variable has a conflict-free assignment
                break;
            }
            // update the random number generator
            rand_row.param(std::uniform_int_distribution<int>::param_type(row1, N - 1));
        }
        else {
            // revert swap that added conflicts
            undo_partial_swap(row1, row2);
        }
    }
    for (unsigned i = row1; i != N; i++) {
        // update the random number generator
        rand_row.param(std::uniform_int_distribution<int>::param_type(i, N - 1));
        unsigned j = rand_row(random_engine);
        swap_colummns(i, j);
        d1_counts.at(i + queen.at(i))++;
        d2_counts.at(N - 1 + i - queen.at(i))++;
    }
    return row1;
}


// computes N - (number of columns with at least 1 queen)
unsigned _count_column_conflicts(
            std::vector<unsigned> &counts,
            unsigned &violations) {
    for (const auto &col : queen) {
        if (counts.at(col)++ != 0) {
            violations++;
        }
    }
    return violations;
}

// computes N - (number of anti-diagonals with at least 1 queen)
// top-right to bottom-left diagonals
unsigned _count_diagonal1_conflicts(
            std::vector<unsigned> &counts,
            unsigned &violations) {
    unsigned row = 0U;
    for (const auto &col : queen) {
        if (counts.at(col + row++)++ != 0) {
            violations++;
        }
    }
    return violations;
}

// computes N - (number of diagonals with at least 1 queen)
// top-left to bottom-right diagonals
unsigned _count_diagonal2_conflicts(
            std::vector<unsigned> &counts,
            unsigned &violations) {
    unsigned offset = N - 1;
    unsigned row = 0U;
    for (const auto &col : queen) {
        if (counts.at(offset + col - row++)++ != 0) {
            violations++;
        }
    }
    return violations;
}

// counts all the conflicts for the current assignments
// uses std::thread for multithreading
unsigned count_all_conflicts_multi(void) {

    unsigned col_violations = 0U;   // multiple values in same column
    std::fill(std::begin(col_counts), std::end(col_counts), 0U);

    unsigned d1_violations = 0U;    // in same top-right to bottom-left diagonal
    std::fill(std::begin(d1_counts), std::end(d1_counts), 0U);

    unsigned d2_violations = 0U;    // in same top-left to bottom-right diagonal
    std::fill(std::begin(d2_counts), std::end(d2_counts), 0U);

    // call each method in its own thread (so violations are computed in parallel)
    std::thread t1(_count_column_conflicts,
                    std::ref(col_counts), std::ref(col_violations));
    std::thread t2(_count_diagonal1_conflicts,
                    std::ref(d1_counts), std::ref(d1_violations));
    std::thread t3(_count_diagonal2_conflicts,
                    std::ref(d2_counts), std::ref(d2_violations));

    // wait for every thread to finish
    t1.join();
    t2.join();
    t3.join();
    
    // return the sum
    return col_violations + d1_violations + d2_violations;
}


unsigned min_conflicts_assignment(unsigned row1) {
    unsigned lowest_violations = -1;    // wrap around to largest
    unsigned best_swap = -1;

    for (unsigned row2 = 0; row2 != N; row2++ ) {
        // ensure rows are distinct
        if (row1 == row2) {
            continue;
        }
        unsigned new_violations = total_conflicts(row1, row2);
        if ( new_violations < lowest_violations ) {
            best_swap = row2;
            lowest_violations = new_violations;
            // stop searching if this swap has zero conflicts
            if (lowest_violations == 0) break;
        }
    }
    if (best_swap != -1U) swap_colummns(row1, best_swap);
    return lowest_violations;
}

int main(int argc, char *argv[]) {

    // overwrite default value for N if N is passed as a command line argument
    if (argc >= 2) {
        try {
            N = std::stoi(argv[1]);
        }
        catch (std::invalid_argument const& ex) {
            // if converting command line arg to a number failed
            // e.g., user entered a non-numeric string
            std::cerr << "ERROR: Cannot parse N=" << argv[1]
                << "; using N=" << N << " instead." << std::endl;
        }
    }

    if (N == 2 || N == 3) {
        std::cout << "No solutions for N=" << N << std::endl;
        return 0;
    }

    // ensure vector has sufficient capacity
    queen.resize(N);
    col_counts.resize(N);
    d1_counts.resize(2 * N - 1);
    d2_counts.resize(2 * N - 1);

    unsigned restarts = -1U;

    do {
        // place all the queens on the board with few conflicts
        unsigned r = initial_placement() - 1;
        restarts++;
        //if (restarts % 1000 == 0) std::cout << "Restarts: " << restarts << std::endl;
        // count the number of conflicts on the board
        unsigned new_conflicts = count_all_conflicts_multi();
        unsigned old_conflicts = new_conflicts;

        unsigned attempt = 0;
        while (attempt < 2000 && new_conflicts != 0) {
            attempt++;
            // find a row that has conflicts
            do {
                r = (r + 1) % (N - 1);
            } while (partial_conflicts(r) == 0);

            // swap row r with whichever other row minimizes conflicts
            min_conflicts_assignment(r);
            
            new_conflicts = count_all_conflicts_multi();
            if (new_conflicts == 0) {
                // stop local search when a solution is found
                break;
            }
            else if (new_conflicts >= old_conflicts) {
                // limit sideways (and worse) moves
                attempt += 1 + ((new_conflicts - old_conflicts)<<1);
            }
        }

    } while ( count_all_conflicts_multi() != 0 );
        // random restart if unsolved

    // try to display the grid in the console
    // (if N is too big, that is caught in the display_grid method)
    display_grid();

    // write the solution to a text file
    write_to_file();

    // show the number of conflicts (sanity check / shouuld always be 0 here)
    // show the number of restarts
    std::cout << "Conflicts: " << count_all_conflicts_multi()
        << "\tN=" << N
        << "\tRestarts:" << restarts
        << std::endl;
    
    return 0;
}
