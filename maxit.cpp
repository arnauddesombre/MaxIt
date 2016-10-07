/*
      M     M  AAAAAAA  X     X  IIIIIII  TTTTTTT
      MM   MM  A     A   X   X      I        T
      M M M M  A     A    X X       I        T
      M  M  M  AAAAAAA     X        I        T
      M     M  A     A    X X       I        T
      M     M  A     A   X   X      I        T
      M     M  A     A  X     X  IIIIIII     T
      
      [October 2016]
 

Compile options:
g++ -Wall -c "%f" -std=c++11

Link options (with local MinGW implementation):
g++ -Wall -O3 -o "%e" "%f" -s -std=c++11

Link options (for distribution):
g++ -Wall -O3 -o "%e" "%f" -s -std=c++11 -static-libgcc -static-libstdc++ -static -lwinpthread

*/

#include <vector>
#include <iostream>
#include <thread>
#include <future>
#include <math.h>     // fabs()
#include <conio.h>    // _getch()
#include <iomanip>    // setw()
#include <numeric>    // accumulate()
#include <windows.h>  // Windows specific display

int GAME = 1; // 1 = MaxIt; 2 = MinIt; 3 = ZeroIt
int BOARD_SIZE = 7; // must be >= 3
bool PLAYER_HORIZONTAL = true; // true -> player plays horizontally & computer plays vertically
int MAX_NUMBER = 50; // maximum number on the board
int MIN_NUMBER = (GAME == 2) ? 0: -MAX_NUMBER; // minimum number on the board
int NUMBER_PROCESSOR = 2; // number of processor to use for parallel threading
int NUMBER_MONTE_CARLO_PATH = 25000 / NUMBER_PROCESSOR; // AI: number of Monte Carlo path per 1 thread
bool DISPLAY_COMPUTER_SCORE = true; // display score assessed for last computer's move

struct windows_console;

const char *HELP =
"How to play MaxIt\n"
"--------------------\n"
"\n"
"MaxIt is played on a game board which looks like a grid (7x7 up to 10x10). "
"Each cell in the game board contains a number which can be either "
"positive or negative. The players take turns selecting numbers from the "
"game board, these numbers are added to the players cumulative score "
"(negative numbers lower the players score).\n"
"\n"
"The rules for selecting numbers from the game board are quite simple: "
"The player (that's you) can select any number along a horizontal row that "
"the cursor is on. When you have selected a number, the cursor moves to "
"that cell on the game board and the number at that location gets added "
"to your score and removed from the board. Your opponent (the computer) "
"then selects a number from the game board. The computer can only select "
"numbers along the vertical column that the cursor is on. Play continues "
"in this fashion until there is no move available (due to an empty row or "
"column).\n"
"\n"
"The object of the game is to have the highest score when the game ends.\n"
"\n"
"--------------------\n"
"\n"
"MaxIt: highest score wins\n"
"MinIt: lowest score wins\n"
"ZeroIt: score closest to zero wins\n"
"\n";

inline int random_range(const int min, const int max) {
	return min + rand() % (max - min + 1);
}

class maxit_game {
public:
	// constructor
	maxit_game() {
		srand(time(0));
		for (int i = 0; i < BOARD_SIZE; i++) {
			std::vector<int> temp1;
			std::vector<bool> temp2;
			for (int j = 0; j < BOARD_SIZE; j++) {
				temp1.push_back(random_range(MIN_NUMBER, MAX_NUMBER));
				temp2.push_back(false);
			}
			_maxitboard_num.push_back(temp1);
			_maxitboard_play.push_back(temp2);
			temp1.clear();
			temp2.clear();
		}
	}
	// copy constructor
	maxit_game(const maxit_game& game) {
		for (int i = 0; i < BOARD_SIZE; i++) {
			_maxitboard_num.push_back(game._maxitboard_num[i]);
			_maxitboard_play.push_back(game._maxitboard_play[i]);
		}		
	}
	// getters
	inline int get_maxitboard_num(const int row, const int col) const {return _maxitboard_num[row][col];}
	inline bool get_maxitboard_play(const int row, const int col) const {return _maxitboard_play[row][col];}
	// helper function
	inline void make_move(const int row, const int col, const bool value = true) {_maxitboard_play[row][col] = value;}
	// helper function prototypes
	void print(windows_console& console, const int row, const int col, const int score_player, const int score_computer, const bool player_turn, const double score) const;	
	void score_move(const int rnd, const int play_row, const int play_col, const int score_player, const int score_computer, std::promise<double> *result);
private:
	std::vector<std::vector<int>> _maxitboard_num;
	std::vector<std::vector<bool>> _maxitboard_play;
	// helper function prototypes
	void draw_line(windows_console& console, const char left, const char middle, const char right) const;
	void draw_first_line(windows_console& console) const;
	void draw_middle_line(windows_console& console) const;
	void draw_last_line(windows_console& console) const;
};

//----------------------------------------------------------------------------
// Display management

struct windows_console {
public:
	// create a window of given width x height
	// set the cursor to invisible, and set the title for the window
	windows_console(const short width, const short height) {
		const SMALL_RECT rect = {0, 0, (short) (width - 1), (short) (height - 1)};
		const COORD coord = {width, height};
		_std_output = GetStdHandle(STD_OUTPUT_HANDLE);
		GetConsoleScreenBufferInfo(_std_output, &_csbi); // save defaults
		SetConsoleWindowInfo(_std_output, TRUE, &rect); // change size of console; will not resize width further than 80
		SetConsoleScreenBufferSize(_std_output, coord); // change size of console buffer
		const CONSOLE_CURSOR_INFO cursorInfo = {1, false};
		SetConsoleCursorInfo(_std_output, &cursorInfo); // set cursor invisible
		SetConsoleTitle("MaxIt by Arnaud"); // set console title
		if (GAME == 2) {
			SetConsoleTitle("MinIt by Arnaud");
		} else if (GAME == 3) {
			SetConsoleTitle("ZeroIt by Arnaud");
		}
	}
	// destructor: restore defaults, in an attempt to
	// preserve the user's console no matter what error happens
	~windows_console() {
		SetConsoleTextAttribute(_std_output, _csbi.wAttributes);
		SetConsoleScreenBufferSize(_std_output, _csbi.dwSize);
		SetConsoleWindowInfo(_std_output, TRUE, &_csbi.srWindow);
	}
	void color(const unsigned int textcol, const unsigned int backcol) {
		// set default color
		const unsigned short attributes = (backcol << 4) | textcol;
		SetConsoleTextAttribute(_std_output, attributes);
	}
	void cursor(const COORD coord = {0, 0}) {
		// reposition cursor
		SetConsoleCursorPosition(_std_output, coord);
	}
	HANDLE _std_output;
	CONSOLE_SCREEN_BUFFER_INFO _csbi;
};

const std::string MARGIN = "  ";

// ASCII caracters:
// http://www.theasciicode.com.ar/extended-ascii-code/box-drawings-single-vertical-line-character-ascii-code-179.html

void maxit_game::draw_line(windows_console& console, const char left, const char middle, const char right) const {
	std::cout << MARGIN << (char) left;
	for(int i = 0; i < BOARD_SIZE - 1; i++) {
		std::cout << std::string(5, (char) 196) << (char) middle;
	}
	std::cout << std::string(5, (char) 196) << (char) right << std::endl;
}

void maxit_game::draw_first_line(windows_console& console) const {
	draw_line(console, (char) 218, (char) 194, (char) 191);
}

void maxit_game::draw_middle_line(windows_console& console) const {
	draw_line(console, (char) 195, (char) 197, (char) 180);
}

void maxit_game::draw_last_line(windows_console& console) const {
	draw_line(console, (char) 192, (char) 193, (char) 217);
}

// colors see: http://www.cplusplus.com/articles/2ywTURfi/
#define color_black 0
#define color_dark_blue 1
#define color_dark_green 2
#define color_dark_aqua 3
#define color_dark_cyan 3
#define color_dark_red 4
#define color_dark_purple 5
#define color_dark_pink 5
#define color_dark_magenta 5
#define color_dark_yellow 6
#define color_dark_white 7
#define color_gray 8
#define color_blue 9
#define color_green 10
#define color_aqua 11
#define color_cyan 11
#define color_red 12
#define color_purple 13
#define color_pink 13
#define color_magenta 13
#define color_yellow 14
#define color_white 15

const unsigned int COL_BACK = color_gray;
const unsigned int COL_GRID = color_black;
const unsigned int COL_TEXT = color_white;
const unsigned int COL_TEXT_SEL = color_white;
const unsigned int COL_BACK_SEL = color_black;

void maxit_game::print(windows_console& console, const int row, const int col, const int score_player, const int score_computer, const bool player_turn, const double score) const {
	// print the board on the screen
	console.cursor({0,0});
	console.color(COL_GRID, COL_BACK);
	std::cout << std::endl;
	draw_first_line(console);
	for(int i = 0; i < BOARD_SIZE; i++) {
		std::cout << MARGIN <<(char) 179;
		for(int j = 0; j < BOARD_SIZE; j++) {
			if (i == row and j == col) {
				console.color((get_maxitboard_play(i, j)) ? COL_BACK_SEL : COL_TEXT_SEL, COL_BACK_SEL);
			} else {
				console.color((get_maxitboard_play(i, j)) ? COL_BACK : COL_TEXT, COL_BACK);
			}
			std::cout << ' ';
			std::cout << std::setfill(' ') << std::setw(3) << get_maxitboard_num(i, j);
			std::cout << ' ';
			console.color(COL_GRID, COL_BACK);
			std::cout << (char) 179;
		}
		std::cout << std::endl;
		if (i == BOARD_SIZE - 1) {
			draw_last_line(console);
		} else {
			draw_middle_line(console);
		}
	}
	console.color(COL_GRID, COL_BACK);
	std::cout << MARGIN << "Player" << std::string(BOARD_SIZE * 6 - 13, ' ') << "Computer\n";
	std::cout << MARGIN << std::setfill(' ') << std::setw(6) << score_player;
	std::cout << std::string(BOARD_SIZE * 6 - 13, ' ') << std::setfill(' ') << std::setw(8) << score_computer << std::endl;
	if (player_turn) {
		std::cout << MARGIN << "<play>" << std::string(BOARD_SIZE * 6 - 13, ' ');
		if (score >= 0. and DISPLAY_COMPUTER_SCORE) {
			std::cout << "   " << std::setw(4) << std::right << (int) (1000. * score) / 10. << "%\n";
		} else {
			std::cout << "        \n";
		}
	} else {
		std::cout << MARGIN << "      " << std::string(BOARD_SIZE * 6 - 13, ' ') << "  <play>\n";
	}
}

//----------------------------------------------------------------------------

inline bool computer_win(const int score_player, const int score_computer) {
	// return true if computer wins; false otherwise; player wins ties
	if (GAME == 1) {
		return (score_player < score_computer);
	} else if (GAME == 2) {
		return (score_computer < score_player);
	} else if (GAME == 3) {
		return (std::fabs(score_computer) < std::fabs(score_player));
	}
	return false;
}

void maxit_game::score_move(const int rand_seed, const int play_row, const int play_col, const int score_player, const int score_computer, std::promise<double> *result) {
	// return the score assessed
	// computer has just played (play_row, play_col), so it is player's turn
	srand(rand_seed);
	int count_win = 0;
	make_move(play_row, play_col, true);
	for (int i = 0; i < NUMBER_MONTE_CARLO_PATH; i++) {
		int new_score_computer = score_computer + get_maxitboard_num(play_row, play_col);
		int new_score_player = score_player;
		int turn = 1;
		int row = play_row;
		int col = play_col;
		int num_move_computer = 0;
		std::vector<std::tuple<int, int>> simulated_play;
		while (true) {
			std::vector<std::tuple<int, int>> possible_play;
			if (turn % 2 == 1) {
				// player's turn
				for (int j = 0; j < BOARD_SIZE; j++) {
					const int r = (PLAYER_HORIZONTAL) ? row : j;
					const int c = (PLAYER_HORIZONTAL) ? j : col;
					if (not get_maxitboard_play(r, c)) {
						possible_play.push_back({r,c});
					}
				}
			} else {
				// computer's turn
				for (int j = 0; j < BOARD_SIZE; j++) {
					const int r = (PLAYER_HORIZONTAL) ? j : row;
					const int c = (PLAYER_HORIZONTAL) ? col : j;
					if (not get_maxitboard_play(r, c)) {
						possible_play.push_back({r,c});
					}
				}
				num_move_computer += possible_play.size();
			}
			if (possible_play.size() == 0) {
				// no possible move; end of game
				possible_play.clear();
				break;
			} else {
				// play one possible move randomly
				const int rand_choice = random_range(0, possible_play.size() - 1);
				row = std::get<0>(possible_play[rand_choice]);
				col = std::get<1>(possible_play[rand_choice]);
				simulated_play.push_back({row, col});
				make_move(row, col, true);
				if (turn % 2 == 1) {
					new_score_player += get_maxitboard_num(row, col);
				} else {
					new_score_computer += get_maxitboard_num(row, col);
				}
				turn++;
			}
			possible_play.clear();
		} // end while
		// un-play all simulated moves
		for (std::tuple<int, int> x : simulated_play) {
			make_move(std::get<0>(x), std::get<1>(x), false);
		}
		simulated_play.clear();
		// check that computer wasn't forced to play a losing move
		// (turn >= 2 if this section was reached)
		// it is questionable to enhance a Monte-Carlo algorithm
		// by eliminating (obviously) wrong moves. This improves
		// computer's endgame.
		if (num_move_computer <= turn / 2) {  // integer division
			if (not computer_win(new_score_player, new_score_computer)) {
				count_win = 0;
				break;
			}
		}
		if (computer_win(new_score_player, new_score_computer)) {
			count_win++;
		}
	} // end for
	make_move(play_row, play_col, false);
	result->set_value(1. * count_win / NUMBER_MONTE_CARLO_PATH);
}

double assess_move(std::vector<maxit_game> &maxit, const int play_row, const int play_col, const int score_player, const int score_computer) {
	// definition of promise/future variables
	std::promise<double> res_before[NUMBER_PROCESSOR];
	std::future<double> res_after[NUMBER_PROCESSOR];
	for (int i = 0; i < NUMBER_PROCESSOR; i++) {
		res_after[i] = res_before[i].get_future();
	}
	std::thread monte_carlo_thread[NUMBER_PROCESSOR];
	// definition of threads
	for (int i = 0; i < NUMBER_PROCESSOR; i++) {
		// it is critical to have different seeds for each thread
		const int rand_seed = (1. + i / 10.) * time(0);
		monte_carlo_thread[i] = std::thread(&maxit_game::score_move, maxit[i], rand_seed, play_row, play_col, score_player, score_computer, &res_before[i]);
	}
	for (int i = 0; i < NUMBER_PROCESSOR; i++) {
		monte_carlo_thread[i].join();
	}
	std::vector<double> out;
	for (int i = 0; i < NUMBER_PROCESSOR; i++) {
		out.push_back(res_after[i].get());
	}
	// averaging of results
	const double average = std::accumulate(out.begin(), out.end(), 0.0) / NUMBER_PROCESSOR;
	return average;
}


inline void parallel_make_move(std::vector<maxit_game> &maxit, int play_row, int play_col, const bool value = true) {
	// make a move on board
	for (int i = 0; i < NUMBER_PROCESSOR; i++) {
		maxit[i].make_move(play_row, play_col, value);
	}
}	

double play_player_turn(windows_console& console, std::vector<maxit_game> &maxit, int &play_row, int &play_col, int &score_player, const int score_computer, const double score) {
	// Player's turn
	bool play_possible = false;
	for (int i = 0; i < BOARD_SIZE; i++) {
		const int row = (PLAYER_HORIZONTAL) ? play_row : i;
		const int col = (PLAYER_HORIZONTAL) ? i : play_col;
		play_possible = not maxit[0].get_maxitboard_play(row, col);
		if (play_possible) {break;}
	}
	if (not play_possible) {return true;}
	while (true) {
		maxit[0].print(console, play_row, play_col, score_player, score_computer, true, score);
		const int keyboard_input = _getch();
		bool move_done = false;
		switch (keyboard_input) {
			case 75: // Left
				if (play_col > 0 and PLAYER_HORIZONTAL) { // current position is not left of board
					play_col--;
				}
				break;
			case 115: // Ctrl+Left
				if (PLAYER_HORIZONTAL) {
					play_col = 0;
				}
				break;
			case 77: // Right
				if (play_col < BOARD_SIZE - 1 and PLAYER_HORIZONTAL) { // current position is not right of board
					play_col++;
				}
				break;
			case 116: // Ctrl-Right
				if (PLAYER_HORIZONTAL) {
					play_col = BOARD_SIZE - 1;
				}
				break;
			case 72: // Up
				if (play_row > 0 and not PLAYER_HORIZONTAL) { // current position is not top of board
					play_row--;
				}
				break;
			case 141: // Ctrl-Up
				if (not PLAYER_HORIZONTAL) {
					play_row = 0;
				}
				break;
			case 80: // Down
				if (play_row < BOARD_SIZE - 1 and not PLAYER_HORIZONTAL) { // current position is not bottom of board
					play_row++;
				}
				break;
			case 145: // Ctrl-Down
				if (not PLAYER_HORIZONTAL) {
					play_row = BOARD_SIZE - 1;
				}
				break;
			case 13: // Enter
			case 32: // Space
				if (not maxit[0].get_maxitboard_play(play_row, play_col)) {
					move_done = true;
				}
				break;
			case 59: // F1 (function key)
				MessageBox(NULL, HELP, "MaxIt by Arnaud - Help", MB_OK);
				break;
			default: // other
				break;
		}
		if (move_done) {
			break;
		}
	}
	parallel_make_move(maxit, play_row, play_col, true);
	score_player += maxit[0].get_maxitboard_num(play_row, play_col);
	maxit[0].print(console, play_row, play_col, score_player, score_computer, true, score);
	return false;
}

double play_computer_turn(windows_console& console, std::vector<maxit_game> &maxit, int &play_row, int &play_col, const int score_player, int &score_computer, double &score) {
	// Computer's turn
	int best_row = -1;
	int best_col = -1;
	double best_score = -1.;
	for (int i = 0; i < BOARD_SIZE; i++) {
		const int row = (PLAYER_HORIZONTAL) ? i: play_row;
		const int col = (PLAYER_HORIZONTAL) ? play_col : i;
		if (not maxit[0].get_maxitboard_play(row, col)) {
			maxit[0].print(console, row, col, score_player, score_computer, false, score);
			const double s = assess_move(maxit, row, col, score_player, score_computer);
			if (s > best_score or best_row == -1 or best_col == -1) {
				best_score = s;
				best_row = row;
				best_col = col;
			}
		}
	}
	if (best_score == -1 and best_row == -1 and best_col == -1) {
		return true;
	} else {
		play_row = best_row;
		play_col = best_col;
		parallel_make_move(maxit, play_row, play_col, true);
		score_computer += maxit[0].get_maxitboard_num(play_row, play_col);	
		maxit[0].print(console, best_row, best_col, score_player, score_computer, false, best_score);
		score = best_score;
		return false;
	}
}

void init_global_variables(int argc, char ** argv) {
	// re-initialize global variables from command line
	if (argc >= 2) {GAME = (int) std::atoi(argv[1]);}
	if (GAME != 2 and GAME != 3) {GAME = 1;}
	if (argc >= 3) {BOARD_SIZE = (int) std::atoi(argv[2]);}
	if (BOARD_SIZE < 3) {BOARD_SIZE = 3;}
	if (argc >= 4) {const std::string str(argv[3]); PLAYER_HORIZONTAL = (str != "NO");}
	if (argc >= 5) {MAX_NUMBER = (int) std::atoi(argv[4]);}
	if (MAX_NUMBER < 1) {MAX_NUMBER = 1;}
	MIN_NUMBER = (GAME == 2) ? 0: -MAX_NUMBER;
	if (argc >= 6) {NUMBER_PROCESSOR = (int) std::atoi(argv[5]);}
	if (NUMBER_PROCESSOR < 1) {NUMBER_PROCESSOR = 1;}
	if (argc >= 7) {NUMBER_MONTE_CARLO_PATH = (int) std::atoi(argv[6]);}
	if (NUMBER_MONTE_CARLO_PATH < 100) {NUMBER_MONTE_CARLO_PATH = 100;}
	NUMBER_MONTE_CARLO_PATH = NUMBER_MONTE_CARLO_PATH / NUMBER_PROCESSOR;
	if (argc >= 8) {const std::string str(argv[7]); DISPLAY_COMPUTER_SCORE = (str != "NO");}
	bool DISPLAY_MODIFS = true;
	if (argc >= 9) {const std::string str(argv[8]); DISPLAY_MODIFS = (str != "NO");}
	if (DISPLAY_MODIFS) {
		std::cout << "Per command line, MaxIt will use:\n\n";
		std::cout << "Game Max / Min / Zero  = " << GAME << std::endl;
		std::cout << "Board size             = " << BOARD_SIZE << std::endl;
		std::cout << "Player moves           = " << ((PLAYER_HORIZONTAL) ? "Horizontally" : "Vertically") << std::endl;
		std::cout << "Maximum number used    = " << MAX_NUMBER << std::endl;
		std::cout << "Minimum number used    = " << MIN_NUMBER << std::endl;
		std::cout << "Number of processors   = " << NUMBER_PROCESSOR << std::endl;
		std::cout << "Monte Carlo paths      = " << NUMBER_MONTE_CARLO_PATH << " per processor" << std::endl;
		std::cout << "Display computer score = " << ((DISPLAY_COMPUTER_SCORE) ? "YES" : "NO") << std::endl;
		if (BOARD_SIZE > 12) {
			std::cout << "\nUse the mouse to appropriately extend the window...\n";
		}
		std::cout << "\nPress any key to continue..." << std::endl;
		_getch();
	}
}

//----------------------------------------------------------------------------

int main(int argc, char ** argv){
	// update defaults
	// argv[0] is "<path>\hex.exe"
	if (argc >= 2) {
		init_global_variables(argc, argv);
	}
	windows_console console(BOARD_SIZE * 6 + 2 * MARGIN.length() + 1, BOARD_SIZE * 2 + 8);
	console.color(COL_TEXT, COL_BACK);
	system("cls");
	// create parallel board games, all equal to the first one, for parallel thread processing
	std::vector<maxit_game> maxit;
	maxit.push_back(maxit_game());
	for (int i = 1; i < NUMBER_PROCESSOR; i++) {
		maxit.push_back(maxit_game(maxit[0]));
	}
	int play_row = random_range(0, BOARD_SIZE - 1);
	int play_col = random_range(0, BOARD_SIZE - 1);
	int score_player = 0;
	int score_computer = 0;
	double score_move = -1;
	bool end = false;
	while (not end) {
		// Player's turn
		end = play_player_turn(console, maxit, play_row, play_col, score_player, score_computer, score_move);	
		// Computer's turn
		if (not end) {
			end = play_computer_turn(console, maxit, play_row, play_col, score_player, score_computer, score_move);
		}
	}
	console.color(COL_TEXT, COL_BACK);
	const bool winner = computer_win(score_player, score_computer);
	std::cout << MARGIN << "Game over. " << ((winner) ? "Computer" : "Player") << " wins!\n";
	std::cout << MARGIN << "Press any key to continue...\n";
	_getch();
	console.color(color_white, color_black);
	system("cls");
	return 0;
}
