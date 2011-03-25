
	#ifndef ESCAPES_H
	#define ESCAPES_H

	/* ANSI Terminal Escape Sequences */
		#define ESC "\033" // escape character for terminal
		#define CSI ESC "[" // Control Sequence Initiator
		#define CSI_CHARS(x) ((x)?'[':'\033') // Gets const char from CSI sequence: 0 for first char, 1 for second char.

		#define CLEAR CSI "H" CSI "2J" // Clears screen. Returns cursor to (1,1).

		/* Cursor */
			#define SAVE_CURSOR CSI "s"
			#define LOAD_CURSOR CSI "u"
			#define TYPE_CURSOR CSI "6n" // Writes CSI <row> ";" <column> "R" to stdin

			#define CURSOR_UP CSI "A"
			#define CURSOR_DOWN CSI "B"
			#define CURSOR_RIGHT CSI "C"
			#define CURSOR_LEFT CSI "D"

		/* Formatting */
			#define BOLD CSI "1m"
			#define UNFORMAT CSI "0m"
			#define FORECOLOUR(x) CSI "3" x "m"
			#define BACKCOLOUR(x) CSI "4" x "m"
			#define INVERTCOLOURS CSI "7m"

			#define BLACK  "0"
			#define RED    "1"
			#define GREEN  "2"
			#define YELLOW "3"
			#define BLUE   "4"
			#define PURPLE "5"
			#define CYAN   "6"
			#define WHITE  "7"
			#define DEFAULT_COLOUR "9"

	#endif
