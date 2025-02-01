#include "Utility.h"
#include "LogEntry.h"
#include <float.h>
#include <time.h>
#include <limits.h>

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
        } else {
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

/**
 * Prints the main menu which displays 3 options:
 *  (f) Specifing a different log file to analyze
 *  (s) Open settings to set filters / interested statistic
 *  (x) Close application
 *
 * Also prints the currently open log file and an optional extra messsage
 * (Usually used as a feedback for the previous iteration), written in 'color' ansi escape sequence
 */
void mainMenu(char* filePath, char* extraMsg, char* color) {
	printf(CLEAR_SCREEN);
	printf("# # # # # Log Analyzer # # # # #\n");
	printf("[" BOLD CYAN "f" RESET "] Change log file\n");
	printf("[" BOLD CYAN "s" RESET "] Log analysis\n\n");
	if (filePath[0] != '\0') printf("Current selected file: [" BOLD MAGENTA "%s" RESET "]\n\n", filePath);
	if (extraMsg[0] != '\0') printf(BOLD "%s%s" RESET "\n\n", color, extraMsg);
	printf("[" BOLD RED "x" RESET "] Close application\n\n");
}

/**
 * Prints the 'settings' menu, also known as log analysis section
 * Provides the following options:
 *  (+) Add a filter for the later analysis
 *  (-) Removes a previously added filter
 *  (m) Changes the statistic to be studied
 *  (s) Starts the analysis
 *  (x) Exits to the main menu
 *
 * Also prints the currently selected statistic and filters, an optional extra messsage
 * (Usually used as a feedback for the previous iteration) is written in 'color' ansi escape sequence
 */
void logAnalysisMenu(enum analysis_operation operation, char* userFilter, time_t startingDatet, time_t endingDatet, char* operationFilter, enum info_type typeFilter, enum outcomes outcomeFilter, double minExTime, double maxExTime, char* extraMsg, char* color) {
	printf(CLEAR_SCREEN);
	printf("# # # # # Settings # # # # #\n");
	printf("[" BOLD CYAN "+" RESET "] Add filter\n");
	printf("[" BOLD CYAN "-" RESET "] Remove filter\n");
	printf("[" BOLD CYAN "m" RESET "] Select statistic\n");
	printf("[" BOLD GREEN "s" RESET "] Start analysis\n");

	// Prints the currently selected statistic
	printf("\nSelected statistic: [" BOLD MAGENTA);
	switch (operation) {
	case countEntries:
		printf("Log entries number");
		break;
	case avgEx:
		printf("Average execution time");
		break;
	case typTrnd:
		printf("Entries type trend");
		break;
	case outTrnd:
		printf("Entries outcome trend");
		break;
	}
	printf(RESET "]\n");

	// Prints the currently selected filters, formatting correctly if none are applied
	int atLeastOne = 0;
	printf("Current filters: [");
	if (userFilter[0] != '\0') {
		printf("\n  user = " BOLD MAGENTA "%s" RESET, userFilter);
		atLeastOne = 1;
	}
	if (startingDatet != (time_t)(-1)) {
		printf("\n  from " BOLD MAGENTA);
		struct tm dt;
		localtime_s(&dt, &startingDatet);
		printDateTime(dt);
		printf(RESET);
		atLeastOne = 1;
	}
	if (endingDatet != (time_t)(-1)) {
		printf("\n  until " BOLD MAGENTA);
		struct tm dt;
		localtime_s(&dt, &endingDatet);
		printDateTime(dt);
		printf(RESET);
		atLeastOne = 1;
	}
	if (operationFilter[0] != '\0') {
		printf("\n  operation = " BOLD MAGENTA "%s" RESET, operationFilter);
		atLeastOne = 1;
	}
	if (typeFilter != no_type) {
		printf("\n  ");
		printInfoType(typeFilter);
		atLeastOne = 1;
	}
	if (outcomeFilter != unset) {
		printf("\n  ");
		printOutcome(outcomeFilter);
		atLeastOne = 1;
	}
	if (minExTime != 0.0) {
		printf("\n  minimum execution time = " BOLD MAGENTA "%.3f" RESET, minExTime);
		atLeastOne = 1;
	}
	if (maxExTime != DBL_MAX) {
		printf("\n  maximum execution time = " BOLD MAGENTA "%.3f" RESET, maxExTime);
		atLeastOne = 1;
	}
	if (atLeastOne) printf("\n");
	printf("]\n");
	if (extraMsg[0] != '\0') printf("\n" BOLD "%s%s" RESET "\n", color, extraMsg);

	printf("\n[" BOLD YELLOW "x" RESET "] Exit settings\n\n");
}

/**
 * Prints a menu to add a filter
 * Provides the following options (only when they weren't previously added):
 *  (u) Interessed user
 *  (d) Starting date (From which date do we start analyzing entries?)
 *  (t) Ending date (At which date do we stop analyzing entries?)
 *  (p) Interessed operation
 *  (i) Interessed entry type
 *  (o) Interessed output
 *  (-) Minimum execution time
 *  (+) Maximum execution time
 *  (x) Exits to the main menu
 *
 * Also prints  an optional extra messsage (Usually used as a feedback for
 * the previous iteration) is written in 'color' ansi escape sequence
 */
void filterAddMenu(char* userFilter, time_t startingDatet, time_t endingDatet, char* operationFilter, enum info_type typeFilter, enum outcomes outcomeFilter, double minExTime, double maxExTime, char* extraMsg, char* color) {
	printf(CLEAR_SCREEN);
	printf("# # # # # Avaiable filters to add # # # # #\n");

	// Prints filters only when not already added
	if (userFilter[0] == '\0') printf("[" BOLD CYAN "u" RESET "] Filter for user\n");
	if (startingDatet == (time_t)(-1)) printf("[" BOLD CYAN "d" RESET "] 'From' date\n");
	if (endingDatet == (time_t)(-1)) printf("[" BOLD CYAN "t" RESET "] 'Until' date\n");
	if (operationFilter[0] == '\0') printf("[" BOLD CYAN "p" RESET "] Filter for operation\n");
	if (typeFilter == no_type) printf("[" BOLD CYAN "i" RESET "] Filter for type\n");
	if (outcomeFilter == unset) printf("[" BOLD CYAN "o" RESET "] Filter for outcome\n");
	if (minExTime == 0.0) printf("[" BOLD CYAN "-" RESET "] Minimum execution time\n");
	if (maxExTime == DBL_MAX) printf("[" BOLD CYAN "+" RESET "] Maximum execution time\n");

	if (extraMsg[0] != '\0') printf(BOLD "\n%s%s\n" RESET, color, extraMsg);

	printf("\n[" BOLD YELLOW "x" RESET "] Cancel\n\n");
}

/**
 * Prints a menu to remove a filter
 * Provides the following options (only when they were previously added):
 *  (u) Interessed user
 *  (d) Starting date (From which date do we start analyzing entries?)
 *  (t) Ending date (At which date do we stop analyzing entries?)
 *  (p) Interessed operation
 *  (i) Interessed entry type
 *  (o) Interessed output
 *  (-) Minimum execution time
 *  (+) Maximum execution time
 *  (x) Exits to the main menu
 *
 * Also prints  an optional extra messsage (Usually used as a feedback for
 * the previous iteration) is written in 'color' ansi escape sequence
 */
void filterRemoveMenu(char* userFilter, time_t startingDatet, time_t endingDatet, char* operationFilter, enum info_type typeFilter, enum outcomes outcomeFilter, double minExTime, double maxExTime, char* extraMsg, char* color) {
	printf(CLEAR_SCREEN);
	printf("# # # # # Avaiable filters for removal # # # # #\n");

	// Prints filters only when are already added
	if (userFilter[0] != '\0') printf("[" BOLD CYAN "u" RESET "] Filter for user\n");
	if (startingDatet != (time_t)(-1)) printf("[" BOLD CYAN "d" RESET "] \'From\' date\n");
	if (endingDatet != (time_t)(-1)) printf("[" BOLD CYAN "t" RESET "] \'Until\' \n");
	if (operationFilter[0] != '\0') printf("[" BOLD CYAN "p" RESET "] Filter for operation\n");
	if (typeFilter != no_type) printf("[" BOLD CYAN "i" RESET "] Filter for type\n");
	if (outcomeFilter != unset) printf("[" BOLD CYAN "o" RESET "] Filter for outcome\n");
	if (minExTime != 0.0) printf("[" BOLD CYAN "-" RESET "] Minimum execution time\n");
	if (maxExTime != DBL_MAX) printf("[" BOLD CYAN "+" RESET "] Maximum execution time\n");

	if (extraMsg[0] != '\0') printf(BOLD "\n%s%s\n" RESET, color, extraMsg);

	printf("\n[" BOLD YELLOW "x" RESET "] Cancel\n\n");
}

/**
 * Prints a menu to select a statistic
 * Provides the following options (only the ones not selected can be chosen):
 *  (0) Number of the entries
 *  (1) Average execution time of entries
 *  (2) Trend of entry type(which entries type are there most of ? Info, Warning or Error ?)
 *  (3) Trend of entry outcome(which entries outcome are there most of ? Success or Failure ?)
 *  (x) Exits to the main menu
 *
 * Also prints the currently selected statistic , as well as,an optional extra messsage
 * (Usually used as a feedback for the previous iteration) is written in 'color' ansi escape sequence
 */
void statisticMenu(enum analysis_operation as, char* extraMsg, char* color) {
	printf(CLEAR_SCREEN);
	printf("# # # # # Avaiable operations # # # # #\n");

	// Prints all the options but the already selected one, which is printed after
	if (as != countEntries) {
		printf("[" BOLD CYAN "0" RESET "] ");
		printAnalysisStatistic(countEntries);
		printf("\n");
	} 
	if (as != avgEx) {
		printf("[" BOLD CYAN "1" RESET "] ");
		printAnalysisStatistic(avgEx);
		printf("\n");
	} 
	if (as != typTrnd) {
		printf("[" BOLD CYAN "2" RESET "] ");
		printAnalysisStatistic(typTrnd);
		printf("\n");
	} 
	if (as != outTrnd) {
		printf("[" BOLD CYAN "3" RESET "] ");
		printAnalysisStatistic(typTrnd);
		printf("\n");
	} 

	// Prints the currently selected operation
	printf("\nCurrently selected: [" BOLD MAGENTA);
	printAnalysisStatistic(as);
	printf(RESET "]\n\n");

	if (extraMsg[0] != '\0') printf(BOLD "\n%s%s\n" RESET, color, extraMsg);

	printf("\n[" BOLD YELLOW "x" RESET "] Cancel\n\n");
}
