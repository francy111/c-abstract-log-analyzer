/*
 * Copyright (c) 2024 Biribo' Francesco
 *
 * Permission to use, copy, modify, and distribute this software for any purpose with or without fee is hereby granted, provided that the above copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include "MenuPrint.h"
#include "DLinkedList.h"
#include <stdio.h>
#include <float.h>
#include <limits.h>
#include <windows.h>
#include <libloaderapi.h>

/**
 * Reads an entry from the log file and stores it in 'entry'
 *
 * 'filePtr' is a pointer to a file, which we suppose is a log file
 *
 * Returns either 0, if the operation was successful, 1, if there
 * were any error in the formatting in the file, or -1 if there was
 * an error reading from the file (also when we reached the end)
 */
int readEntry(LogEntry* entry, FILE* filePtr) {
	int result = 0;
	char buf[1024];

	// Every line has this format: user@dd.mm.yyyy-hh:mm:ss-operation-type-outcome-ex.time
	int r = readLine(filePtr, buf);
	if (r > 0) {

		// User
		char* context = 0;
		char* token = strtok_s(buf, "@", &context);

		strncpy_s(entry->user, USER_LENGTH, token, strlen(token));

		// Date & time
		struct tm dateTime;

		token = strtok_s(NULL, "-", &context);
		if (sscanf_s(token, "%d/%d/%d", &(dateTime.tm_mday), &(dateTime.tm_mon), &(dateTime.tm_year)) != 3) {
			result = 1;
		}

		token = strtok_s(NULL, "-", &context);
		if (sscanf_s(token, "%d:%d:%d", &(dateTime.tm_hour), &(dateTime.tm_min), &(dateTime.tm_sec)) != 3) {
			result = 1;
		}

		dateTime.tm_mon--;
		dateTime.tm_year -= 1900;
		entry->date = mktime(&dateTime);
		if (entry->date == (time_t)(-1)) {
			result = 1;
		}

		// Operation
		token = strtok_s(NULL, "-", &context);
		strncpy_s(entry->operation, OPERATION_LENGTH, token, strlen(token));

		// Type
		token = strtok_s(NULL, "-", &context);
		if (strncmp("Information", token, strlen("Information")) == 0) {
			entry->type = info;
		}
		else if (strncmp("Warning", token, strlen("Warning")) == 0) {
			entry->type = warning;
		}
		else if (strncmp("Error", token, strlen("Error")) == 0) {
			entry->type = error;
		}
		else {
			result = 1;
		}

		// Outcome
		token = strtok_s(NULL, "-", &context);
		if (strncmp("Success", token, strlen("Success")) == 0) {
			entry->outcome = success;
		}
		else if (strncmp("Failure", token, strlen("Failure")) == 0) {
			entry->outcome = failure;
		}
		else {
			result = 1;
		}

		// Execution time
		token = strtok_s(NULL, "-", &context);
		if (sscanf_s(token, "%lf", &(entry->executionTime)) != 1) {
			result = 1;
		}

	}
	else {
		result = -1;
	}
	return result;
}

/**
 * Prints the main menu which displays 3 options:
 *  (f) Specifing a different log file to analyze
 *  (s) Open settings to set filters / interested statistic
 *  (x) Close application
 *
 * Also prints the currently open log file and an optional extra messsage
 * (Usually used as a feedback for the previous iteration)
 */
void mainMenu(char* cd, char* filePath, char* extraMsg) {
	printf(CLEAR_SCREEN);
	printf("# # # # # Log Analyzer # # # # #\n");
	printf("[" BOLD CYAN "f" RESET "] Change log file\n");

	// Print absolute file path if opened
	if (filePath[0] != '\0') {
		char absolutePath[MAX_PATH];
		nullString(absolutePath, MAX_PATH);
		GetFullPathNameA(filePath, MAX_PATH, absolutePath, NULL);

		printf("[" BOLD CYAN "s" RESET "] Log analysis\n\n");
		printf("Current selected file: [" BOLD MAGENTA "%s" RESET "]\n", absolutePath);
	}

	// Otherwise show working directory so the user knows how to form a relative path
	else {
		printf("\nCurrent working directory: [" BOLD MAGENTA "%s" RESET "]\n", cd);
	}
	if (extraMsg[0] != '\0') printf("\n%s" RESET "\n", extraMsg);
	printf("\n[" BOLD RED "x" RESET "] Close application\n\n");
}

/**
 * Prints the 'settings' menu, also known as log analysis section
 * Provides the following options:
 *  (+) Add a filter for the later analysis
 *  (-) Removes a previously added filter
 *  (l) Switch between [AND - OR] logical operator for filters
 *  (m) Changes the statistic to be studied
 *  (s) Starts the analysis
 *  (x) Exits to the main menu
 *
 * Also prints the currently selected statistic and filters and an optional extra messsage
 * (Usually used as a feedback for the previous iteration)
 */
void logAnalysisMenu(enum analysis_operation operation, int filterOrFlag, EntryFilter f, char* extraMsg, enum outcomes analysisOutcome) {
	printf(CLEAR_SCREEN);
	printf("# # # # # Settings # # # # #\n");
	printf("[" BOLD CYAN "+" RESET "] Add filter\n");
	printf("[" BOLD CYAN "-" RESET "] Remove filter\n");
	printf("[" BOLD CYAN "l" RESET "] Switch to ");
	if (filterOrFlag) printf("all filters must match\n");
	else printf("just one filter needs to match\n");
	printf("[" BOLD CYAN "m" RESET "] Select statistic\n");
	printf("[" BOLD GREEN "s" RESET "] Start analysis\n");
	if (analysisOutcome == success) printf("[" BOLD MAGENTA "r" RESET "] Show analysis results\n");

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
	if (f.userFilters != NULL) {
		printf("\n user in " BOLD MAGENTA);
		char* bff = listToString(f.userFilters, HEAD_TO_TAIL);
		printf("%s" RESET, bff);
		free(bff);
		atLeastOne = 1;
	}
	if (f.startingDate != (time_t)(-1)) {
		printf("\n  from " BOLD MAGENTA);
		struct tm dt;
		localtime_s(&dt, &(f.startingDate));
		printDateTime(dt);
		printf(RESET);
		atLeastOne = 1;
	}
	if (f.endingDate != (time_t)(-1)) {
		printf("\n  until " BOLD MAGENTA);
		struct tm dt;
		localtime_s(&dt, &(f.endingDate));
		printDateTime(dt);
		printf(RESET);
		atLeastOne = 1;
	}
	if (f.operationFilters != NULL) {
		printf("\n operation in " BOLD MAGENTA);
		char* bff = listToString(f.operationFilters, HEAD_TO_TAIL);
		printf("%s" RESET, bff);
		free(bff);
		atLeastOne = 1;
	}
	if (f.typeFilter != no_type) {
		printf("\n  ");
		printInfoType(f.typeFilter);
		atLeastOne = 1;
	}
	if (f.outcomeFilter != unset) {
		printf("\n  ");
		printOutcome(f.outcomeFilter);
		atLeastOne = 1;
	}
	if (f.minExecutionTime != 0.0) {
		printf("\n  minimum execution time = " BOLD MAGENTA "%.3f" RESET, f.minExecutionTime);
		atLeastOne = 1;
	}
	if (f.maxExecutionTime != DBL_MAX) {
		printf("\n  maximum execution time = " BOLD MAGENTA "%.3f" RESET, f.maxExecutionTime);
		atLeastOne = 1;
	}
	if (atLeastOne) printf("\n");
	printf("]\n");

	printf("Current operator: [" );
	if (filterOrFlag) printf(BLUE "OR");
	else printf(GREEN "AND");
	printf(RESET "]\n");

	if (extraMsg[0] != '\0') printf("\n%s\n" RESET, extraMsg);

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
 * the previous iteration)
 */
void filterAddMenu(EntryFilter f, char* extraMsg) {
	printf(CLEAR_SCREEN);
	printf("# # # # # Avaiable filters to add # # # # #\n");

	// Prints filters only when not already added
	printf("[" BOLD CYAN "u" RESET "] Filter for user\n"); // Always able to add a user
	if (f.startingDate == (time_t)(-1)) printf("[" BOLD CYAN "d" RESET "] 'From' date\n");
	if (f.endingDate == (time_t)(-1)) printf("[" BOLD CYAN "t" RESET "] 'Until' date\n");
	printf("[" BOLD CYAN "p" RESET "] Filter for operation\n"); // Always able to add an operation
	if (f.typeFilter == no_type) printf("[" BOLD CYAN "i" RESET "] Filter for type\n");
	if (f.outcomeFilter == unset) printf("[" BOLD CYAN "o" RESET "] Filter for outcome\n");
	if (f.minExecutionTime == 0.0) printf("[" BOLD CYAN "-" RESET "] Minimum execution time\n");
	if (f.maxExecutionTime == DBL_MAX) printf("[" BOLD CYAN "+" RESET "] Maximum execution time\n");

	if (extraMsg[0] != '\0') printf("\n%s\n" RESET, extraMsg);

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
 * the previous iteration)
 */
void filterRemoveMenu(EntryFilter f, char* extraMsg) {
	printf(CLEAR_SCREEN);
	printf("# # # # # Avaiable filters for removal # # # # #\n");

	// Prints filters only when are already added
	if (f.userFilters != NULL) printf("[" BOLD CYAN "u" RESET "] Filter for user\n");
	if (f.startingDate != (time_t)(-1)) printf("[" BOLD CYAN "d" RESET "] \'From\' date\n");
	if (f.endingDate != (time_t)(-1)) printf("[" BOLD CYAN "t" RESET "] \'Until\' \n");
	if (f.operationFilters != NULL) printf("[" BOLD CYAN "p" RESET "] Filter for operation\n");
	if (f.typeFilter != no_type) printf("[" BOLD CYAN "i" RESET "] Filter for type\n");
	if (f.outcomeFilter != unset) printf("[" BOLD CYAN "o" RESET "] Filter for outcome\n");
	if (f.minExecutionTime != 0.0) printf("[" BOLD CYAN "-" RESET "] Minimum execution time\n");
	if (f.maxExecutionTime != DBL_MAX) printf("[" BOLD CYAN "+" RESET "] Maximum execution time\n");

	if (extraMsg[0] != '\0') printf("\n%s\n" RESET, extraMsg);

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
 * (Usually used as a feedback for the previous iteration)
 */
void statisticMenu(enum analysis_statistic as, char* extraMsg) {
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

	if (extraMsg[0] != '\0') printf("\n%s\n", extraMsg);

	printf("\n[" BOLD YELLOW "x" RESET "] Cancel\n\n");
}