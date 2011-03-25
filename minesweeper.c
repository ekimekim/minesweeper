
	/*
		General TODOs and feature ideas
			Big:
				Multiplayer
			Medium:
				Better screen drawing method (no flicker)
			Small:
				...
	*/

	#ifdef _DEBUG
		#include "utils.h" //standard personal header, contains debug macros, util functions
		#undef STRING_LENGTH // We want to define this ourselves!
		#define PRINT_SETTINGS(s) fprintf(stderr,"Settings: (%d, %d), %d, %d\n", (s).fieldSize.x, (s).fieldSize.y, (s).mines, (s).expand)
		#define DEBUG_POINT(p) DEBUGNUM((p).x) DEBUGNUM((p).y)
	#endif

	/* Header File contains std headers, structs, other typedefs and function prototypes */
		#include "minesweeper.h"

	/* Header File containing ANSI Control sequences (escape sequences) for terminals */
		#include "escapes.h"

	/* Standard Headers */
		#include <stdlib.h>
		#include <stdio.h>
		#include <string.h>
		#include <limits.h>
		#include <time.h>

		#include <termios.h>
		#include <unistd.h>
		#include <sys/ioctl.h>

	/*Macros*/
		#define FAILED(x) ((x) != EXIT_SUCCESS) // Macro for checking if a function call did not return EXIT_SUCCESS
		#define STRING_LENGTH (128) // Max string length in a variety of situations
		#define MAX(x,y) ((x)>(y))?(x):(y) // Warning: Do not use with things with side-effects!
		#define MIN(x,y) ((x)<(y))?(x):(y) // Warning: Do not use with things with side-effects!

	/*Global Variables*/
		struct termios oldtermios; // for storing old termios settings - must be global to use atexit

		/* Constants */
			const point MinSize = {1, 1};
			const point MaxSize = {200, 200};
			const int MinMines = 0;
			const int MaxMines = 500000;
			const float defaultMineRatio = 0.1; // Sets default mines as this * num of minepoints
			const settings DefaultSettings = {{20, 20}, -1, 0}; // Defaults: {20,20} field with mines set later and no options set.

			/* Arrays of (string,string) of form (fmt, str) (fmt includes escape sequences for colour, etc) for each minepoint display value. */
				const char* displayString_OffField[2] = {"", " "};
				const char* displayString_Flag[2] = {BOLD FORECOLOUR(RED), "f"};
				const char* displayString_Hidden[2] = {FORECOLOUR(WHITE), "*"};
				const char* displayString_Mine[2] = {BOLD FORECOLOUR(RED), "X"};
				const char* displayString_Values[9][2] = {
					/* 0 */ {FORECOLOUR(WHITE), "0"},
					/* 1 */ {FORECOLOUR(CYAN), "1"},
					/* 2 */ {FORECOLOUR(GREEN), "2"},
					/* 3 */ {FORECOLOUR(YELLOW), "3"},
					/* 4 */ {FORECOLOUR(RED), "4"},
					/* 5 */ {BOLD FORECOLOUR(WHITE), "5"},
					/* 6 */ {BOLD FORECOLOUR(CYAN), "6"},
					/* 7 */ {BOLD FORECOLOUR(GREEN), "7"},
					/* 8 */ {BOLD FORECOLOUR(YELLOW), "8"}
				};
				const char* displayString_Border[3][2] = {
					/* Vert  */ {BOLD FORECOLOUR(WHITE), "|"},
					/* Horiz */ {BOLD FORECOLOUR(WHITE), "-"},
					/* Cornr */ {BOLD FORECOLOUR(WHITE), "+"}
				};
				FILE* err_file;

			const char* helpString =
				"There are several mines hidden throughout the field.\n"
				"You can move around the field with the arrow keys, and press Spacebar or Enter to reveal a square.\n"
				"When you reveal a square, it will tell you how many mines are adjacent to it.\n"
				"If this number is 0, it will automatically reveal the squares around it.\n"
				"(This behaviour can be disabled with Expand Zeroes = 0)\n"
				"If you reveal a mine, you lose. Instead, Flag the mine with F.\n"
				"This will stop you hitting the square you think is a mine. Press F again to unflag so you can reveal it.\n"
				"You win if you identify every mine and reveal every other square.\n"
				"Press Q to quit.\n";

	/*Methods*/

	#ifdef _DEBUG
		void dumpField(field Field, point fieldSize) { // Dump field values to stderr, bypassing formatting
			point Point;
			minepoint* Minepoint;
			for (Point.y=0; Point.y<fieldSize.y; Point.y++) {
				for (Point.x=0; Point.x<fieldSize.x; Point.x++) {
					Minepoint = getMinepoint(Point, Field, fieldSize);
					if (isMine(*Minepoint))
						fprintf(stderr, "*");
					else
						fprintf(stderr, "%d", Minepoint->value);
				}
				fprintf(stderr, "\n");
			}
		}
	#endif

	int main(int argc, char *argv[]) {
		int repeat;

		settings Settings = DefaultSettings;
		field Field = {NULL};

		Settings.options = DEFAULT_NOT_TTY;
		if (isatty(STDIN_FILENO) && isatty(STDOUT_FILENO)) {
			printf("Run in interactive mode? (y,n) [y] > ");
			char s[STRING_LENGTH];
			while (fgets(s, STRING_LENGTH, stdin), s[0] != 'y' && s[0] != 'n' && s[0] != '\n') printf("Please choose 'y' or 'n' > ");
			if (s[0] != 'n') Settings.options = DEFAULT_INTERACTIVE;
		}

		err_file = HAS_OPTION(Settings, USE_STDERR) ? stderr : stdout;

		do {

			if (FAILED(setup(&Field, &Settings))) {
				if (Field) freeField(&Field);
				fatal("Error while trying to set up game", EXIT_FAILURE);
			}

			if (HAS_OPTION(Settings, SIMPLE_INPUT)) {
				repeat = 0;
				simplePlay(Field, Settings);
			} else {
				repeat = play(Field, Settings);
				freeField(&Field);
				if (repeat < 0) {
					fatal("Error during play", EXIT_FAILURE);
				}
			}

		} while (repeat);

		exit(EXIT_SUCCESS);
	}

	int set_termios(void) {
		struct termios Termios;
		if (tcgetattr(STDIN_FILENO, &Termios) < 0) return EXIT_FAILURE;
		oldtermios = Termios;
		Termios.c_lflag &= ~ECHO; // Set echo off
		Termios.c_lflag &= ~ICANON; /* Set canonical mode off - canonical mode gives things like backspace meaning.
		                               More importantly, it disables line editing
		                               and passes through characters immediately. */
		if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &Termios) < 0) return EXIT_FAILURE;
		return EXIT_SUCCESS;
	}

	void unset_termios(void) {
		if (tcsetattr(STDIN_FILENO,TCSAFLUSH,&oldtermios) < 0) fatal("Failed to restore terminal", EXIT_FAILURE);
	}

	void fatal(const char* msg, int exitcode) {
		fprintf(err_file, "%s\n", msg);
		exit(exitcode);
	}

	int setup(field* Field, settings* Settings) {
		if (FAILED(setSettings(Settings))) return EXIT_FAILURE;
		if (FAILED(createField(Field, *Settings))) return EXIT_FAILURE;
		if (HAS_OPTION(*Settings, GENERATE_MINES)) {
			if (FAILED(buildField(*Field, *Settings, (int) time(NULL)))) return EXIT_FAILURE;
		} else {
			if (FAILED(promptMines(*Field, *Settings))) fatal("mine-error", EXIT_FAILURE);
		}
		if (!HAS_OPTION(*Settings, SIMPLE_INPUT)) {
			if (FAILED(set_termios())) {
				fprintf(err_file, "Warning: Terminal did not set up properly\n");
			}
			if (atexit(unset_termios) != 0) return EXIT_FAILURE;
		}
		return EXIT_SUCCESS;
	}

	int setSettings(settings* Settings) {
		if (HAS_OPTION(*Settings, VERBOSE_SETUP)) {
			const char* greeting = "Settings Selection: Please type in setting values or press Enter for defaults.\n"
				                   "If you do not know what a setting does, the defaults are probably a good bet!\n";
			fputs(greeting, stdout);
			if (FAILED(askSetting(&(Settings->fieldSize.x), "Field Width", MinSize.x, MaxSize.x))) return EXIT_FAILURE;
			if (FAILED(askSetting(&(Settings->fieldSize.y), "Field Height", MinSize.y, MaxSize.y))) return EXIT_FAILURE;
			if (Settings->mines == -1) Settings->mines = defaultMineRatio * Settings->fieldSize.x * Settings->fieldSize.y; // set default based on size, if not set (-1 is "not set" flag value)
			if (FAILED(askSetting(&(Settings->mines), "Number of Mines", MinMines, MIN(MaxMines, Settings->fieldSize.x * Settings->fieldSize.y - 1)))) return EXIT_FAILURE;
		} else {
			char s[STRING_LENGTH];
			if (!fgets(s, STRING_LENGTH, stdin)) fatal("missing-input", EXIT_FAILURE);
			if (s[0] != 'g') {
				switch (s[0]) {
					case 'g':
						fatal("grid-error", EXIT_FAILURE);
					case 'u': case 'f':
						fatal("unreveal-error", EXIT_FAILURE);
					default:
						fatal("input-error", EXIT_FAILURE);
				}
			}
			if (sscanf(s, "g %d %d\n", &Settings->fieldSize.x, &Settings->fieldSize.y) != 2) fatal("input-error", EXIT_FAILURE);
			if (Settings->fieldSize.x <= 0 || Settings->fieldSize.x > 100 || Settings->fieldSize.y <= 0 || Settings->fieldSize.y > 100) {
				fatal("grid-error", EXIT_FAILURE);
			}
			printf("%c %d %d\n", 'g', Settings->fieldSize.x, Settings->fieldSize.y);
			Settings->mines = 10;
		}
		return EXIT_SUCCESS;
	}

	int askSetting(int* ptr, char* name, int min, int max) {
		const char* fmt = "Please select a value for %s: (%d...%d) [%d]  > "; // SETTING NAME, MIN, MAX, DEFAULT
		int value;
		char s[STRING_LENGTH];
		int s_length;
		int i;
		int good;

		// prompt until proper input is given
		while (1) {
			printf(fmt, name, min, max, *ptr);
			fgets(s, STRING_LENGTH, stdin);
			s_length = strlen(s);
			if (s_length < 2) break; // keep defaults
			if (s_length >= STRING_LENGTH-1) {
				fprintf(stderr, "Your input had too many characters. Please enter a shorter value.\n");
				continue;
			}
			good = 1;
			for (i=0; i<s_length-1; i++) { // check all but last letter is within 0-9
				if (s[i] < '0' || s[i] > '9') {
					good = 0;
				}
			}
			if (!good || !sscanf(s, "%d", &value)) {
				fprintf(stderr, "Invalid input. Please enter a number.\n");
				continue;
			}
			if ((value < min) || (value > max)) {
				fprintf(stderr, "Input outside range. Please provide a number between %d and %d.\n", min, max);
				continue;
			}
			*ptr = value;
			break;
		}

		return EXIT_SUCCESS;
	}

	int createField(field* Field, settings Settings) {
		*Field = calloc(Settings.fieldSize.x * Settings.fieldSize.y, sizeof(minepoint));
		if (!*Field) return EXIT_FAILURE;
		return EXIT_SUCCESS;
	}

	int freeField(field* Field) {
		free(*Field);
		*Field = NULL;
		return EXIT_SUCCESS;
	}

	int buildField(field Field, settings Settings, int seed) {
		int i, r;
		minepoint* mine;
		minepoint* MinePoint;
		point Point, countPoint;
		srand(seed);

		/* Assemble a list of available points for mines */
		int length = 0;
		point* potentials = malloc(Settings.fieldSize.x * Settings.fieldSize.y * sizeof(point));
		for (Point.y=0; Point.y<Settings.fieldSize.y; Point.y++) for (Point.x=0; Point.x<Settings.fieldSize.x; Point.x++) {
			potentials[length++] = Point;
		}

		for (i = 0; i < Settings.mines; i++) {
			/* Some sanity checks */
			if (!length) { // No room for more mines! (Settings limits should prevent this)
				free(potentials);
				return EXIT_FAILURE;
			}

			/* Get a point from available points - it's now a mine */
			r = rand() % length;
			Point = potentials[r];
			mine = getMinepoint(Point, Field, Settings.fieldSize);
			if (isMine(*mine)) {
				free(potentials);
				return EXIT_FAILURE; // sanity check
			}
			mine->value = -1;

			/* Now we remove it from potentials, by replacing it with the last entry in the list and shortening it */
			potentials[r] = potentials[--length];

			/* Now increment all non-mines around it */
			for (countPoint.y=Point.y-1;countPoint.y<=Point.y+1;countPoint.y++) {
				for (countPoint.x=Point.x-1;countPoint.x<=Point.x+1;countPoint.x++) {
					MinePoint = getMinepoint(countPoint, Field, Settings.fieldSize);
					if (MinePoint && !isMine(*MinePoint)) MinePoint->value++; // This will also exclude Point when Point == countPoint
				}
			}
		}

		free(potentials);

		return EXIT_SUCCESS;
	}

	int promptMines(field Field, settings Settings) {
		int i;
		point Point, countPoint;
		minepoint* mine;
		minepoint* MinePoint;
		char s[STRING_LENGTH];
		for (i=0; i<10; i++) {
			if (!fgets(s, STRING_LENGTH, stdin)) fatal("missing-input", EXIT_FAILURE);
			if (s[0] != 'b') {
				switch (s[0]) {
					case 'g':
						fatal("grid-error", EXIT_FAILURE);
					case 'u': case 'f':
						fatal("unreveal-error", EXIT_FAILURE);
					default:
						fatal("input-error", EXIT_FAILURE);
				}
			}
			if (sscanf(s, "b %d %d\n", &Point.x, &Point.y) != 2) fatal("input-error", EXIT_FAILURE);
			if (!(mine = getMinepoint(Point, Field, Settings.fieldSize))) return EXIT_FAILURE;
			if (isMine(*mine)) return EXIT_FAILURE;
			mine->value = -1;
			printf("%c %d %d\n", 'b', Point.x, Point.y);

			/* Now increment all non-mines around it */
			for (countPoint.y=Point.y-1;countPoint.y<=Point.y+1;countPoint.y++) {
				for (countPoint.x=Point.x-1;countPoint.x<=Point.x+1;countPoint.x++) {
					MinePoint = getMinepoint(countPoint, Field, Settings.fieldSize);
					if (MinePoint && !isMine(*MinePoint)) MinePoint->value++; // This will also exclude Point when Point == countPoint
				}
			}

		}
		return EXIT_SUCCESS;
	}

	minepoint* getMinepoint(point Point, field Field, point FieldSize) {
		if (Point.x < 0 || Point.x >= FieldSize.x || Point.y < 0 || Point.y >= FieldSize.y)
			return NULL; // Out of Bounds
		return &Field[point2offset(Point, FieldSize.x)];
	}

	int point2offset(point Point, int xsize) {
		return Point.y * xsize + Point.x;
	}

	int isMine(minepoint MinePoint) {return MinePoint.value == -1;}

	int play(field Field, settings Settings) {
		int quit = 0;
		int c, i;
		char s[STRING_LENGTH];
		point Cursor = {Settings.fieldSize.x/2, Settings.fieldSize.y/2};

		while (!quit) {
			// Always display first - Displays before first turn and also lets input code quit without display being called first.
			if (FAILED(display(Field, Settings, Cursor))) return EXIT_FAILURE;

			c = fgetc(stdin);

			/* Handle Inputs */
			switch (c) {
				case CSI_CHARS(0): // Begins control sequence - check if its an UP, DOWN, LEFT or RIGHT
					i = 0;
					s[i++] = c; // ESC
					s[i++] = (c = fgetc(stdin)); // 2nd byte
					if (c == CSI_CHARS(1)) { // if CSI
						while (c = fgetc(stdin), c != EOF && i < STRING_LENGTH-2 && (c < 0x40 || c > 0x7e)) s[i++] = c; // loop until valid final byte is found
					}
					if (c != EOF) s[i++] = c; // Add final char
					s[i++] = '\0';
					if (strcmp(s, CURSOR_UP) == 0 && Cursor.y > 0) Cursor.y--;
					if (strcmp(s, CURSOR_DOWN) == 0 && Cursor.y < Settings.fieldSize.y-1) Cursor.y++;
					if (strcmp(s, CURSOR_LEFT) == 0 && Cursor.x > 0) Cursor.x--;
					if (strcmp(s, CURSOR_RIGHT) == 0 && Cursor.x < Settings.fieldSize.x-1) Cursor.x++;
					break;
				case 'f': // Flag space
					setPointTo(Field, Settings, Cursor, FLAGGED);
					break;
				case '\n': case ' ': // Reveal space
					if (setPointTo(Field, Settings, Cursor, DISPLAYED) == -1) {
						/* Dead! */
						quit = 1;
						if (FAILED(lose(Field, Settings, Cursor))) return EXIT_FAILURE;
					}
					break;
				case '?': case 'h': // Print help
					printf(CLEAR
					       "%s" // helpString
					       "Press any key to continue...\n"
					, helpString);
					fgetc(stdin); // block until keypress
					break;
				case 'q': case EOF: // Quit
					return 0;
				default: // ignore
					break;
			}

			if (!quit && checkWin(Field, Settings)) { // don't check win if we are quitting - what if we just lost?
				quit = 1;
				if (FAILED(win(Field, Settings, Cursor))) return EXIT_FAILURE;
			}
		}

		unset_termios();

		while (1) {
			printf("Play again? (y/n) [n] > ");
			fgets(s, STRING_LENGTH, stdin);
			if (strcmp(s, "yes\n") == 0 || strcmp(s, "y\n") == 0) return 1;
			else if (strcmp(s, "no\n") == 0 || strcmp(s, "n\n") == 0 || strcmp(s, "\n") == 0) return 0;
			else printf("Please type \"y\", \"n\" or leave blank.\n");
		}
	}

	void simplePlay(field Field, settings Settings) {
		char s[STRING_LENGTH];
		char c;
		point Point = {0,0};
		enum display Display;

		while(1) {
			display(Field, Settings, Point);

			if (!fgets(s, STRING_LENGTH, stdin)) fatal("missing-input", EXIT_FAILURE);
			if (sscanf(s, "%c %d %d", &c, &Point.x, &Point.y) != 3) fatal("input-error", EXIT_FAILURE);
			if (!getMinepoint(Point, Field, Settings.fieldSize)) fatal("uncover-error", EXIT_FAILURE);
			Display = (enum display) (c == 'u') ? DISPLAYED : (c == 'f') ? FLAGGED : (0);
			if (!Display) {
				switch (s[0]) {
					case 'g':
						fatal("grid-error", EXIT_FAILURE);
					case 'u': case 'f':
						fatal("unreveal-error", EXIT_FAILURE);
					default:
						fatal("input-error", EXIT_FAILURE);
				}
			}
			if (setPointTo(Field, Settings, Point, Display) == -1 && Display == DISPLAYED) {
				fputs(s, stdout);
				fatal("lost",EXIT_SUCCESS);
			}

			printf("%c %d %d\n", c, Point.x, Point.y);

			if (checkWin(Field, Settings)) {
				display(Field, Settings, Point);
				fatal("won", EXIT_SUCCESS);
			}
		}	
	}

	int setPointTo(field Field, settings Settings, point Point, enum display Display) {
		minepoint* MinePoint = getMinepoint(Point, Field, Settings.fieldSize);
		if (!MinePoint) {
			fatal("Tried to access minepoint outside field!", EXIT_FAILURE);
		}
		switch (Display) {
			case FLAGGED: // On flagged, toggle/error flag or ignore/error if displayed
				switch (MinePoint->Display) {
					case HIDDEN:
						if (HAS_OPTION(Settings, STRICT_WIN_CHECKS)) {
							int i, count = 0;
							for (i=0; i<Settings.fieldSize.x*Settings.fieldSize.y; i++) {
								if (Field[i].Display == FLAGGED) count++;
							}
							if (count >= Settings.mines && HAS_OPTION(Settings, FRAGILE)) fatal("uncover-error", EXIT_FAILURE);
						}
						MinePoint->Display = FLAGGED;
						break;
					case FLAGGED:
						if (HAS_OPTION(Settings, FRAGILE)) fatal("uncover-error", EXIT_FAILURE);
						MinePoint->Display = HIDDEN;
						break;
					default:
						break;
				}
				break;
			case DISPLAYED: // On displayed, reveal and possibly expand zeroes.
				if (MinePoint->Display != DISPLAYED) {
					MinePoint->Display = DISPLAYED;
					if (MinePoint->value == 0 && HAS_OPTION(Settings, EXPAND_ZEROES)) {
						point nextPoint;
						for (nextPoint.x=Point.x-1; nextPoint.x<=Point.x+1; nextPoint.x++)
						for (nextPoint.y=Point.y-1; nextPoint.y<=Point.y+1; nextPoint.y++) {
							if (nextPoint.x >= 0 && nextPoint.x < Settings.fieldSize.x &&
							    nextPoint.y >= 0 && nextPoint.y < Settings.fieldSize.y &&
							    setPointTo(Field, Settings, nextPoint, DISPLAYED) == -1) {
								// Got a mine during expansion, this shouldn't happen!
								fatal("Found a mine during Zero Expansion, error in field!", EXIT_FAILURE);
							}
						}
					}
				} else if (HAS_OPTION(Settings, FRAGILE)) fatal("uncover-error", EXIT_FAILURE);
				break;
			default:
				break;
		}
		return MinePoint->value;
	}

	int checkWin(field Field, settings Settings) {
		int i;
		if (HAS_OPTION(Settings, STRICT_WIN_CHECKS)) {
			for (i=0; i<Settings.fieldSize.x*Settings.fieldSize.y; i++) {
				if (Field[i].Display == HIDDEN) return 0;
			}
			return 1;
		} else {
			// Win conditions: the count of HIDDEN or FLAGGED squares is equal to the number of mines
			int count = 0;
			for (i=0; i<Settings.fieldSize.x*Settings.fieldSize.y; i++) {
				if (Field[i].Display != DISPLAYED) count++;
			}
			if (count < Settings.mines) { // sanity check
				fatal("Too many squares revealed - some MUST be mines!", EXIT_FAILURE);
			}
			return (count == Settings.mines);
		}
	}

	int win(field Field, settings Settings, point Cursor) {
		int i;
		for (i=0; i < Settings.fieldSize.x * Settings.fieldSize.y; i++) {
			if (isMine(Field[i])) Field[i].Display = FLAGGED;
		}
		display(Field, Settings, Cursor);
		printf("\nCongratulations! You won!\n");
		return EXIT_SUCCESS;
	}

	int lose(field Field, settings Settings, point Cursor) {
		int i;
		for (i=0; i < Settings.fieldSize.x * Settings.fieldSize.y; i++) {
			if (isMine(Field[i]) && Field[i].Display == HIDDEN) Field[i].Display = DISPLAYED;
		}
		display(Field, Settings, Cursor);
		printf("\nYou lose!\n");
		return EXIT_SUCCESS;
	}

	int display(field Field, settings Settings, point Cursor) {
		point screenSize, TopLeft, Point;
		minepoint* Minepoint;
		const char** displayString; //ptr to string literal array, defined above
		int simple = HAS_OPTION(Settings, SIMPLE_OUTPUT);

		if (simple) {
			// If border, a little out of bounds.
			int border = !!HAS_OPTION(Settings, BORDER);
			screenSize.x = Settings.fieldSize.x + 2*border;
			screenSize.y = Settings.fieldSize.y + 2*border;
			TopLeft.x = -border;
			TopLeft.y = -border;
		} else {
			screenSize = getScreenSize();
			if (screenSize.x < 0 && screenSize.y < 0) return EXIT_FAILURE;
			TopLeft.x = Cursor.x - screenSize.x/2;
			TopLeft.y = Cursor.y - screenSize.y/2;
		}

		if (!simple) printf(CLEAR);
		for (Point.y=TopLeft.y; Point.y<TopLeft.y+screenSize.y; Point.y++) {
			for (Point.x=TopLeft.x; Point.x<TopLeft.x+screenSize.x; Point.x++) {
				Minepoint = getMinepoint(Point, Field, Settings.fieldSize);
				if (Minepoint) {
					if (!simple && Point.x == Cursor.x && Point.y == Cursor.y) fputs(INVERTCOLOURS, stdout);
					displayString = getStr(*Minepoint);
				} else {
					// Point out of range
					displayString = displayString_OffField;
					if (HAS_OPTION(Settings, BORDER) &&
					    Point.x >= -1 && Point.x <= Settings.fieldSize.x &&
					    Point.y >= -1 && Point.y <= Settings.fieldSize.y)
					{
						int i = (Point.x == -1) + (Point.x == Settings.fieldSize.x) +
						        2 * ((Point.y == -1) + (Point.y == Settings.fieldSize.y));
						if (i) displayString = displayString_Border[i-1];
					}
				}
				if (!simple) fputs(displayString[0], stdout);
				fputs(displayString[1], stdout);
				if (!simple) fputs(UNFORMAT, stdout);
			}
			printf("\n");
		}
		return EXIT_SUCCESS;
	}

	const char** getStr(minepoint MinePoint) {
		switch (MinePoint.Display) {
			case HIDDEN:
				return displayString_Hidden;
			case FLAGGED:
				return displayString_Flag;
			case DISPLAYED:
				return (isMine(MinePoint)) ? displayString_Mine : displayString_Values[MinePoint.value];
			default:
				fatal("Corrupt minepoint object - illegal Display value", EXIT_FAILURE);
		}
		return NULL; //This line is never reached - it's just here to make the compiler happy
	}

	point getScreenSize(void) {
		struct winsize w;
		point Point = {-1, -1};
		if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) >= 0) { // if ioctl fails, will return Point with default (error) values
			Point.y = w.ws_row;
			Point.x = w.ws_col;
		}
		return Point;
	}
