/*
 * Copyright (c) 2024 Biribo' Francesco
 *
 * Permission to use, copy, modify, and distribute this software for any purpose with or without fee is hereby granted, provided that the above copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include "Utility.h"
#include <windows.h>
#include <libloaderapi.h>

/**
 * Prints the text associated to the analysis_statistic
 *
 * 'countEntries' (0) -> Count entries
 * 'avgEx' (1) -> Average execution time
 * 'typTrnd' (2) -> Trend (most of) information type
 * 'outTrnd' (3) -> Trend (most of) outcome
 */
void printAnalysisStatistic(enum analysis_statistic as) {
	switch (as) {
	case countEntries:
		printf("Count entries");
		break;
	case avgEx:
		printf("Average execution time");
		break;
	case typTrnd:
		printf("Trend (most of) information type");
		break;
	case outTrnd:
		printf("Trend (most of) outcome");
		break;
	default:
		printf(BOLD RED "Wrong statistic given\n" RESET);
	}
}

/**
 * Disables the shell ECHO, making
 * keyboard typing invisible
 */
void disableEcho(void) {
	HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
	DWORD mode;

	// Get current terminal settings
	GetConsoleMode(hStdin, &mode);

	// Disables ECHO, not showning typed input
	mode &= ~ENABLE_ECHO_INPUT;

	// Apply new settings
	SetConsoleMode(hStdin, mode);
}

/**
 * Enables the shell ECHO, making
 * keyboard typing visible
 */
void enableEcho(void) {
	HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
	DWORD mode;

	// Get current terminal settings
	GetConsoleMode(hStdin, &mode);

	// Enables ECHO, showning typed input
	mode |= ENABLE_ECHO_INPUT;

	// Apply new settings
	SetConsoleMode(hStdin, mode);
}

/**
 * Changes the current workind directory
 * goind from C: (or the disk the file is in)
 * to the actual directory containing the executable
 */
void setWorkingDirToExecutable() {
	char path[1024];
	DWORD r = GetModuleFileNameA(NULL, path, 1024);
	if (r != 0) {

		// Finds where the last '\' is, after which is the executable name, so if we stop before, we have the directory containing it
		char* lastBackslash = strrchr(path, '\\');
		if (lastBackslash) {

			// End the path to the directory
			lastBackslash = '\0';

			SetCurrentDirectoryA(path);
		}
	}
}

/**
 * Reads a line from the file pointed by 'filePtr' and stores it in the buffer 'buf'
 *
 * Returns either the number of bytes actually read
 * or, in case of any error, -1
 */
int readLine(FILE* filePtr, char* buf) {

	char chr = '\0';
	int index = 0, done = 0;

	// Check that filePtr is not null just to be sure
	if (filePtr != NULL) {

		// Read one character at a time
		while (!done && ((chr = getc(filePtr)) != EOF)) {

			// We found the end on the current line
			if (chr == '\n') {
				buf[index] = '\0'; // We swap it with '\0' since the original '\n' was used just for the file formatting
				done = 1;

			}

			// Up until we find a '\n' we are 'buinding' the line
			else {
				buf[index] = chr;
				index++;
			}
		}
	}
	else {
		// Since we increment 'index' with every written character, it also acts as a count of written characters, to signal an error (related to the file), we set this to -1
		index = -1;
	}

	return index;
}

/**
 * Empties the input buffer (stdin) from all
 * of the eventual characters still there
 * (usually newlines or spaces)
 */
void cleanInputBuffer(void) {
	char c;
	while ((c = getchar()) != '\n' && c != EOF) {
		// Deletes all characters until a newline or EOF
	}
}

/**
 * Acquires a string from the standard input (stdin) and
 * stores it into 'string', a buffer of maximum size of 'size'
 *
 * Lets acquiring 'visually empty' strings (such as spaces and newlines)
 *
 * Returns the number of read characters
 */
long getOptionalInput(char* string, size_t size) {
	if (fgets(string, (int)size, stdin) != NULL) {
		size_t len = strlen(string);

		// Check if string is as long as it can be, and if the last character is a newline (the user pressed 'return')
		if (len == size - 1 && string[len - 1] != '\n')
			cleanInputBuffer(); // If theres no newline, it means the typed string was longer than the limit, there will be dirty characters in the input buffer

		if (len > 0 && string[len - 1] == '\n') {
			string[len - 1] = '\0'; // Removes newline
		}
		return (long)len;
	}
	else {
		return -1;
	}
}

/**
 * Acquires a single character from standard input (stdin)
 * and returns it
 *
 * Either returns the single read character if a single character
 * given otherwise, EOF is returned
 */
char getSingleChar(void) {

	// Acquire a three-sized string max. from the input (2 characters and '\0')
	char input[3] = { 'x','x','\0' };
	getOptionalInput(input, 3);

	// Check if the second character is empty ('\0'), this happens only if a single character was typed
	if (input[1] == '\0')
		return input[0]; // Returns the single written character
	else
		return EOF; // Otherwise EOF is returned
}

/**
 * Prints the date_time struct stored in dt in the
 * 'dd.mm.yyyy' (hh:mm:ss) format in BOLD CYAN
 */
void printDateTime(struct tm dt) {
	// Adjusting month and yeah for printing
	printf("%d.%d.%d (%d:%d:%d)", dt.tm_mday, dt.tm_mon + 1, dt.tm_year + 1900, dt.tm_hour, dt.tm_min, dt.tm_sec);
}


/**
 * Acquires the components of a date_time structure from
 * the standard input (stdin) and stores them in the
 * structure pointed by 'dateTime'
 *
 * The date is acquired first using the 'dd/mm/yyyy' format
 * The time is acquired right after using the 'hh:mm:ss' format
 */
void getDateTime(struct tm* dateTime) {
	int done = 0;
	while (done == 0) {
		printf("Enter date in dd/mm/yyyy format: " BOLD CYAN);
		if (scanf_s("%d/%d/%d", &dateTime->tm_mday, &dateTime->tm_mon, &dateTime->tm_year) != 3
			) {
			printf(BOLD RED "Invalid format\n");
		}
		else {
			done = 1;
		}
		printf(RESET);
		cleanInputBuffer();
	}
	dateTime->tm_mon -= 1; // Adjusting since months are stored from 0 to 11
	dateTime->tm_year -= 1900; // Adjusting since the year should be the number of years since 1900

	done = 0;
	while (done == 0) {
		printf("Enter time in hh:mm:ss format: " BOLD CYAN);
		if (scanf_s("%d:%d:%d", &dateTime->tm_hour, &dateTime->tm_min, &dateTime->tm_sec) != 3
			) {
			printf(BOLD RED "Invalid format\n");
		}
		else {
			done = 1;
		}
		printf(RESET);
		cleanInputBuffer();
	}
}

/**
 * Empties a maximum of 'size' characters
 * in the string stored in 'str'
 *
 * All characters from 0 to size-1 are set to '\0'
 */
void nullString(char str[], size_t size) {
	for (int i = 0; i < size; i++)
		str[i] = '\0';
}