
	#ifndef MINESWEEPER_H
	#define MINESWEEPER_H

	/* Type Definitions */

		enum display {
			HIDDEN = 0,
			DISPLAYED,
			FLAGGED
		};

		typedef struct {
			int x;
			int y;
		} point; // A coordinate type.

		#define EXPAND_ZEROES 1 // When set, hitting a 0 will hit all squares around it
		#define GENERATE_MINES 2 // When set, program will autogenerate mine positions instead of prompting.
		#define VERBOSE_SETUP 4 // When set, will ask user nice questions rather than expecting simply formatted inputs.
		#define BORDER 8 // When set, a border is drawn.
		#define SIMPLE_INPUT 16 // When set, takes simply formatted inputs instead of arrowkeys and characters.
		#define SIMPLE_OUTPUT 32 // When set, draws simple grid dump instead of scrolling interface.
		#define FORMATTING 64 // When set, allows colour and other nice formatting.
		#define FRAGILE	128 // When set, program will report error and quit at the slightest provocation.
		#define USE_STDERR 256 // When set, program will print errors to stderr instead of stdout.
		#define STRICT_WIN_CHECKS 512 // When set, all remaining mines must be flagged before the game will end. And you cannot make more flags than there are mines.

		#define DEFAULT_INTERACTIVE EXPAND_ZEROES | GENERATE_MINES | VERBOSE_SETUP | FORMATTING | USE_STDERR | BORDER
		#define DEFAULT_NOT_TTY BORDER | SIMPLE_INPUT | SIMPLE_OUTPUT | FRAGILE | STRICT_WIN_CHECKS

		#define HAS_OPTION(Settings, flag) ((Settings).options & (flag))

		typedef struct {
			point fieldSize;
			int mines; // number of mines
			unsigned int options; // bit-flags for extended options
		} settings; // Struct containing game settings

		typedef struct {
			int value; // Number of adjacent mines. -1 for a mine.
			enum display Display;
		} minepoint;

		typedef minepoint* field; // The play field

	/*Primitives*/
		/* Semantics note: The phrase "returns success" refers to the practice
		   of returning an int EXIT_SUCCESS or EXIT_FAILURE from a function */

		int set_termios(void); // Setup termios as wanted
		void unset_termios(void); // Restore original termios
		void fatal(const char* msg, int exitcode); // Abort program after printing error message, using given exit code.
		int setup(field* Field, settings* Settings); // Set up program, ready to begin.
		int setSettings(settings* Settings); // Sets game settings. Returns success.
		int askSetting(int* ptr, char* name, int min, int max); /* Prompt user for a single game setting,
		                                                           setting ptr to it. Returns success. */
		int createField(field* Field, settings Settings); // Constructor for field. Puts new, empty field in Field.
		int freeField(field* Field); // Gracefully destroy field and it's components.
		int buildField(field Field, settings Settings, int seed); /* Populate field with mines and sets values,
		                                                             using RNG seeded with given seed. Returns success.*/
		int promptMines(field Field, settings Settings); // Populate field from simple user input. Returns success.
		int play(field Field, settings Settings); // Main game loop. Returns 1 to play again, 0 to exit, -1 on error.
		void simplePlay(field Field, settings Settings); // Main game loop for SIMPLE_INPUT.
		minepoint* getMinepoint(point Point, field Field, point FieldSize); /* Return the minepoint at given point,
		                                                                       or NULL for out of bounds. */
		int point2offset(point Point, int ysize); /* Takes a point and returns the linear offset that would represent
		                                             that point in a 2d y-major array with a max y of ysize */
		int isMine(minepoint MinePoint); // Returns 1 if MinePoint is a mine, else 0.
		int setPointTo(field Field, settings Settings, point Point, enum display Display); /*
			Set given point's display. Return value at point (that was just changed) (-1 on mine).
			May expand zeroes, recursively. */
		int checkWin(field Field, settings Settings); // Return 1 if user has won, else 0.
		int win(field Field, settings Settings, point Cursor); // Do stuff for winning
		int lose(field Field, settings Settings, point Cursor); // Do stuff for losing
		int display(field Field, settings Settings, point Cursor); // Draw the game field to the screen.
		const char** getStr(minepoint MinePoint); // Returns the string to display for a given minepoint.
		point getScreenSize(void); // Returns the size of the terminal window for stdout, or {-1, -1} on error.

	#endif
