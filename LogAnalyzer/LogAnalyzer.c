/*
 * Copyright (c) 2024 Biribo' Francesco
 *
 * Permission to use, copy, modify, and distribute this software for any purpose with or without fee is hereby granted, provided that the above copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <float.h>
#include <limits.h>
#include "LogEntry.h"
#include "Utility.h"
#include "EntryFilter.h"
#include "MenuPrint.h"
#include "DLinkedList.h"
#include "cJSON.h"

/**
 * Updates the counter 'count'
 * This counts towards the 'entry count' statistic
 */
void executeEntryCount(LogEntry* entry, int* count, double* sum, int* sucCntr, int* failCntr, int* infoCntr, int* warningCntr, int* errCntr);

/**
 * Updates the counters 'count' and 'sum'
 * This counts towards the 'average execution time' statistic
 */
void executeAvgExTime(LogEntry* entry, int* count, double* sum, int* sucCntr, int* failCntr, int* infoCntr, int* warningCntr, int* errCntr);

/**
 * Updates either the 'info', 'warning' or 'error counter
 * This counts towards the 'type trend' statistic
 */
void executeTrendOutcome(LogEntry* entry, int* count, double* sum, int* sucCntr, int* failCntr, int* infoCntr, int* warningCntr, int* errCntr);

/**
 * Updates either the 'success' or 'failure counter
 * This counts towards the 'outcome trend' statistic
 */
void executeTrendType(LogEntry* entry, int* count, double* sum, int* sucCntr, int* failCntr, int* infoCntr, int* warningCntr, int* errCntr);

int main(int argc, char* argv[]) {

	// Set working directory to the folder where the executable is
	setWorkingDirToExecutable();
	char cd[_MAX_PATH];
	GetCurrentDirectoryA(_MAX_PATH, cd); // Save current directory to print it in the menu

	/* Log file name can initially be passed as a command line argument
	 * and subsequentially during execution
	 */
	char relativeFilePath[_MAX_PATH];
	nullString(relativeFilePath, _MAX_PATH);
	FILE* logFile = NULL;

	/* We assume that if at least a parameter is passed, the second
	 * (first manually typed) to be the path of the log file
	 */
	if (argc > 1) {
		strncpy_s(relativeFilePath, _MAX_PATH, argv[1], strlen(argv[1]));
		relativeFilePath[strlen(relativeFilePath)] = '\0';
		fopen_s(&logFile, relativeFilePath, "r");
	}

	/* * * * * * * * * * * * * * * * * * * TEMP VARIABLES* * * * * * * * * * * * * * * * * * */

	FILE* tmp; // Temporary file pointer to switch log file
	char tmpStr[1024]; // Temporary string to hold the new log file path

	/* * * * * * * * * * * * * * * * * * * FILTERS * * * * * * * * * * * * * * * * * * * * * */

	EntryFilter f; // Filter struct that holds all filters

	f.userFilters = NULL; // We look for entries that have the same user contained in this filter
	char userFilter[USER_LENGTH]; // Temporary buffer to hold a user read from stdin
	nullString(userFilter, USER_LENGTH); // Empty buffer to be sure

	f.startingDate = (time_t)(-1); // We look for entries created after this date (inclusive)
	f.endingDate = (time_t)(-1); // We look for entries create before this date (inclusive)

	f.operationFilters = NULL; // We look for entries that have the same value (operation) contained in this filter
	char operationFilter[OPERATION_LENGTH]; // Temporary buffer to hold an operation read from stdin
	nullString(operationFilter, OPERATION_LENGTH); // Empty buffer to be sure

	f.typeFilter = no_type; // We look for entries with the same type (Info/Warning/Error)
	f.outcomeFilter = unset; // We look for entries with the same outcome (Failure/Success)

	f.minExecutionTime = 0.0; // We look for entries with an execution time greater than this filter (inclusive)
	f.maxExecutionTime = DBL_MAX; // We look for entries with an execution time less than this filter (inclusive)

	enum analysis_statistic as = countEntries; // Statistic to extract

	LogEntry logEn; // Struct that holds the values of the last log entry read
	int entryCount = 0; // Number of entries read during analysis
	int filteredEntryCount = 0; // Number of entries matching our filters
	double avgExTime = 0.0; // Average execution time, calculated during analysis
	int infoCounter = 0; // Number of entries flagged as information
	int warningCounter = 0;// Number of entries flagged as warning
	int errorCounter = 0; // Number of entries flagged as error
	int successCounter = 0; // Number of entries flagged as success
	int failureCounter = 0; // Number of entries flagged as failed
	enum outcomes analysisOutcome = failure; // Outcome of the analysis

	/* * * * * * * * * * * * * * * * * * * UTILS * * * * * * * * * * * * * * * * * * * * * * */

	char choice = '\0'; // Character used to move between menus (input from the user)
	void (*operation)(LogEntry*, int*, double*, int*, int*, int*, int*, int*) = executeEntryCount; // Emulate a dinamic call to the various operations based on choses statistic

	char configPath[_MAX_PATH]; // Path for the configuration file, used for loading/saving configs
	nullString(configPath, _MAX_PATH);

	int startingDSet = 0, endingDSet = 0; // Flag to check if the starting/ending dates (only date) have been specified

	char extraMsg[1024]; // Buffer that holds a message that is shown during the next iteration
	nullString(extraMsg, 1024);

	/* True means that out of the filters, only 1 needs to be true in order for an entry to match,
 	 * on the other hand, 0 means all need to be true for an entry to match
	 * 
	 * In other words, 0 (false) means the filters are checked with a locical AND, 1 (anything not 0, true) means the filters are checked with a logical OR
	 */
	int globalOrFilters = 0;

	/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

	/* Main menu, shows currently selected log file
	 * Gives the option to change (or select a) log file or modify settings
	 */
	while (choice != 'x') {
		mainMenu(cd, relativeFilePath, extraMsg);
		printf(BOLD CYAN);
		choice = getSingleChar();
		printf(RESET);

		/* (F/f) Select (another) log file
		 * (S/s) Open settings menu
		 * (X/x) Terminate application
		 */
		switch (choice) {

			/* Select (another) log file
			 * Either open a new file or keep the current one
			 */
		case 'f':
		case 'F':

			// Reset extra message string, present menu
			nullString(extraMsg, 1024);
			printf("Insert file path: " BOLD CYAN);
			getOptionalInput(tmpStr, 1024);
			printf(RESET);

			// If the 'new' log file can't be opened
			if (fopen_s(&tmp, tmpStr, "r") != 0) {

				// If the 'current' log file is not opened
				if (logFile == NULL) {
					sprintf_s(extraMsg, 1024, RED "Could not open file '%s'" RESET, tmpStr);
				}

				// If the 'current' log file is present (has already been opened)
				else {
					sprintf_s(extraMsg, 1024, RED "Could not open new file, old is still open" RESET);
				}
			}

			// If the 'new' log file has be opened
			else {

				// Close the 'current' log file if it has been opened
				if (logFile != NULL)
					fclose(logFile);

				// Replace the 'current' file with the 'new' one
				logFile = tmp;
				strncpy_s(relativeFilePath, _MAX_PATH, tmpStr, strlen(tmpStr));

				// Show this success message
				sprintf_s(extraMsg, 1024, GREEN "Opened new file" RESET);
			}
			break;

			// Load configuration file, automatically trying to select a file and filters
		case 'l':
		case 'L':

			// Get config file location (absolute)
			printf("\nConfiguration files are typically JSON files\n\n");
			printf("Type configuration file path: " BOLD CYAN);
			scanf_s("%s", configPath, _MAX_PATH);
			printf(RESET);
			cleanInputBuffer();

			// Try to open JSON config file
			if (loadConfig(configPath, relativeFilePath, &f, &(int)as, &globalOrFilters) == 0) {
				if (relativeFilePath != NULL) {
					fopen_s(&logFile, relativeFilePath, "r");
					sprintf_s(extraMsg, 1024, GREEN "Configurations successfully loaded" RESET);

					switch (as) {
					case countEntries:
						operation = executeEntryCount;
						break;
					case avgEx:
						operation = executeAvgExTime;
						break;
					case typTrnd:
						operation = executeTrendType;
						break;
					case outTrnd:
						operation = executeTrendOutcome;
						break;
					}
				}
			}
			else {
				sprintf_s(extraMsg, 1024, RED "Error during config load" RESET);
			}

			break;

			// Save configuration file
		case 'd':
		case 'D':

			if (logFile != NULL) {
				// Get config file location (absolute)
				printf("\nConfiguration files are typically JSON files\n\n");
				printf("Type configuration file path destination: " BOLD CYAN);
				scanf_s("%s", configPath, _MAX_PATH);
				printf(RESET);
				cleanInputBuffer();

				// Try to save JSON config file
				int res = saveConfig(configPath, relativeFilePath, &f, (int)as, globalOrFilters);
				if (res == 0) {
					sprintf_s(extraMsg, 1024, GREEN "Configurations successfully saved in '%s'" RESET, configPath);
				}
				else if (res == 2) {
					sprintf_s(extraMsg, 1024, YELLOW "Default configs, no save needed" RESET);
				}
				else {
					sprintf_s(extraMsg, 1024, RED "Error during config save" RESET);
				}
			}
			else {
				sprintf_s(extraMsg, 1024, YELLOW "Please select a valid operation" RESET);
			}

			break;

			/* Open settings menu
			 * Manage filters, choose statistic or start analysis
			 */
		case 's':
		case 'S':

			// Show option only if file has been selected
			if (logFile != NULL) {

				// Reset extra message string
				nullString(extraMsg, 1024);

				/* Settings (sub)menu, shows currently selected statistic and filters
				 * Gives the option to change statistic, either add or remove filters, and start analysis
				 */
				do {

					// Present menu and read user input
					logAnalysisMenu(as, globalOrFilters, f, extraMsg, analysisOutcome);
					printf(BOLD CYAN);
					choice = getSingleChar();
					printf(RESET);

					// Reset extra message string
					nullString(extraMsg, 1024);

					int done = 0; // Flag used in inner loops in the various cases
					struct tm st, et; // Temporary structures (used to convert trom struct tm to time_t)

					/* (+) Add filter
					 * (-) Remove filter
					 * (M/m) Choose statistic
					 * (S/s) Start analysis
					 * (X/x) Close settings and return to main menu
					 */
					switch (choice) {

						/* Filter addition
						 * Provides a menu that shows an option for every
						 * filter not already added, giving the opportunity to add it
						 */
					case '+':

						// Reset extra message string so they can be overwritten for next iteration
						nullString(extraMsg, 1024);

						done = 0; // Flag for the inner loop
						while (done == 0) {

							// Present menu and read user input
							filterAddMenu(f, startingDSet, endingDSet, extraMsg);
							printf(BOLD CYAN);
							choice = getSingleChar();
							printf(RESET);

							// Reset extra message string so they can be overwritten for next iteration
							nullString(extraMsg, 1024);

							/* (U/u) User
							 * (D/d) Starting date
							 * (T/t) Ending date
							 * (P/p) Operation
							 * (I/i) Type outcome
							 * (O/o) Entry outcome
							 * (-) Minimum execution time
							 * (+) Maximum execution time
							 * (X/x) Cancel filter addition and go back to settings
							 */
							switch (choice) {

								// Specify user
							case 'u':
							case 'U':

								// Acquire user
								printf("Type user: " BLUE CYAN);
								scanf_s("%s", userFilter, USER_LENGTH);
								cleanInputBuffer();

								// Add filter to the list only if it wasn't already added, we are treating this list like a set
								if(!contains(f.userFilters, userFilter)) insertTail(&(f.userFilters), userFilter);
								printf(RESET);
								done = 1;
								break;

								// Specify starting date
							case 'd':
							case 'D':

								// Acquire date if not set
								if (!startingDSet) {

									// Acquire starting date 
									getDate(&st);
									f.startingDate = mktime(&st);

									// Set error message if the date was invalid
									if (f.startingDate == (time_t)(-1)) {
										sprintf_s(extraMsg, 1024, RED "Please enter a valid date" RESET);
									}
									else {

										// If it is valid and an ending date was set, check if they are coherent
										if ((f.endingDate != (time_t)(-1)) && difftime(f.endingDate, f.startingDate) < 0) {
											sprintf_s(extraMsg, 1024, RED "Starting date must be before ending date" RESET);
											f.startingDate = (time_t)(-1);
										}

										// If the date is all valid, keep it and flag the date as set
										else {
											startingDSet = 1;
										}
									}
								}

								// Specify time
								else {
									getTime(&st);
									time_t newSDate = mktime(&st);

									// Set error message if the date was invalid
									if (newSDate == (time_t)(-1)) {
										sprintf_s(extraMsg, 1024, RED "Please enter a valid time" RESET);
									}
									else {

										// If it is valid and an ending date was set, check if they are coherent
										if ((f.endingDate != (time_t)(-1)) && difftime(f.endingDate, newSDate) < 0) {
											sprintf_s(extraMsg, 1024, RED "Starting date must be before ending date" RESET);
										}

										// If it is valid, overwrite date with date and time
										else {
											f.startingDate = newSDate;
										}
									}
								}
								done = 1;
								break;

								// Specify ending date
							case 't':
							case 'T':

								// Acquire date if not set
								if (!endingDSet) {

									// Acquire ending date 
									getDate(&et);
									f.endingDate = mktime(&et);

									// Set error message if the date was invalid
									if (f.endingDate == (time_t)(-1)) {
										sprintf_s(extraMsg, 1024, RED "Please enter a valid date" RESET);
									}
									else {

										// If it is valid and a starting date was set, check if they are coherent
										if ((f.startingDate != (time_t)(-1)) && difftime(f.endingDate, f.startingDate) < 0) {
											sprintf_s(extraMsg, 1024, RED "Ending date must be after starting date" RESET);
											f.endingDate = (time_t)(-1);
										}

										// The date is valid, we flag it
										else {
											endingDSet = 1;
										}
									}
								}

								// Specify time
								else {
									getTime(&et);
									time_t newEDate = mktime(&et);

									// Set error message if the date was invalid
									if (newEDate == (time_t)(-1)) {
										sprintf_s(extraMsg, 1024, RED "Please enter a valid time" RESET);
									}
									else {

										// If it is valid and an ending date was set, check if they are coherent
										if ((f.endingDate != (time_t)(-1)) && difftime(newEDate, f.startingDate) < 0) {
											sprintf_s(extraMsg, 1024, RED "Starting date must be before ending date" RESET);
										}

										// If it is valid, overwrite date with date and time
										else {
											f.endingDate = newEDate;
										}
									}
								}
								done = 1;
								break;

								// Specify operation
							case 'p':
							case 'P':

								// Acquire operation
								printf("Type operation: " BLUE CYAN);
								scanf_s("%s", operationFilter, OPERATION_LENGTH);
								cleanInputBuffer();

								// Add filter to the list only if it wasn't already added, we are treating this list like a set
								if (!contains(f.operationFilters, operationFilter)) insertTail(&(f.operationFilters), operationFilter);
								printf(RESET);
								done = 1;
								break;

								// Specify entry type
							case 'i':
							case 'I':

								// Acquire type
								printf("[" BOLD BLUE "i" RESET "] Information\n");
								printf("[" BOLD YELLOW "w" RESET "] Warning\n");
								printf("[" BOLD RED "e" RESET "] Error\n");
								printf(BOLD CYAN);
								choice = getSingleChar();
								printf(RESET);

								// Reset extra message string so they can be overwritten for next iteration
								nullString(extraMsg, 1024);

								/* (I/i) Information
								 * (W/w) Warning
								 * (E/e) Error
								 */
								switch (choice) {

									// Information
								case 'i':
								case 'I':
									f.typeFilter = info;
									break;

									// Warning
								case 'w':
								case 'W':
									f.typeFilter = warning;
									break;

									// Error
								case 'e':
								case 'E':
									f.typeFilter = error;
									break;

									/* Wrong input
										* Initialize extra message to show that the input wasn't correct
										*/
								default:

									// Strings initialization for next iteration
									sprintf_s(extraMsg, 1024, RED "Invalid choice, old information type not modified" RESET);
									break;
								}
								done = 1;
								break;

								// Specify entry outcome
							case 'o':
							case 'O':

								// Acquire outcome
								printf("[" BOLD GREEN "s" RESET "] Success\n");
								printf("[" BOLD RED "f" RESET "] Failure\n");
								printf(BOLD CYAN);
								choice = getSingleChar();
								printf(RESET);

								// Reset extra message string so they can be overwritten for next iteration
								nullString(extraMsg, 1024);

								/* (S/s) Success
								 * (F/f) Failure
								 */
								switch (choice) {

									// Success
								case 's':
								case 'S':
									f.outcomeFilter = success;
									break;

									// Failure
								case 'f':
								case 'F':
									f.outcomeFilter = failure;
									break;

									/* Wrong input
									 * Initialize extra message to show that the input wasn't correct
									 */
								default:

									// Strings initialization for next iteration
									sprintf_s(extraMsg, 1024, RED "Invalid choice, old outcome not modified" RESET);
									break;
								}
								done = 1;
								break;

								// Specify minimum execution time
							case '-':

								// Acquire time
								printf("Tempo esecuzione minimo: " BLUE CYAN);
								scanf_s("%lf", &(f.minExecutionTime));
								printf(RESET);
								cleanInputBuffer();

								// Check if it makes sense (at least 0)
								if (f.minExecutionTime < 0.0) {
									f.minExecutionTime = 0.0;
									sprintf_s(extraMsg, 1024, RED "Execution time must be at least 0.0s" RESET);
								}
								else {

									// Check if it's coherent with maximum execution time, if specified
									if (f.minExecutionTime >= f.maxExecutionTime) {
										f.minExecutionTime = 0.0;
										sprintf_s(extraMsg, 1024, RED "Minimum execution time must be lower than the maximum" RESET);
									}
								}
								done = 1;
								break;

								// Specify maximum execution time
							case '+':

								// Acquire time
								printf("Tempo esecuzione massimo: " BLUE CYAN);
								scanf_s("%lf", &(f.maxExecutionTime));
								printf(RESET);
								cleanInputBuffer();

								// Check if it makes sense (at least 0)
								if (f.maxExecutionTime < 0.0) {
									f.maxExecutionTime = DBL_MAX;
									sprintf_s(extraMsg, 1024, RED "Execution time must be at least 0.0s" RESET);
								}
								else {

									// Check if it's coherent with minimum execution time, if specified
									if (f.minExecutionTime >= f.maxExecutionTime) {
										f.maxExecutionTime = DBL_MAX;
										sprintf_s(extraMsg, 1024, RED "Maximum execution time must be greater than the minimum" RESET);
									}
								}
								done = 1;
								break;

								/* Closes the current (sub)menu
								 * Resets the strings and, sets the flag 'done' as true
								 * and, next iteration, goes back to settins (sub)menu
								 */
							case 'x':
							case 'X':

								// Reset extra message string so they can be overwritten for next iteration
								nullString(extraMsg, 1024);

								done = 1; // We are done with this inner loop
								choice = '\0'; // Avoids exiting 2 loops at once
								break;

								/* Wrong input
								 * Initialize extra message to show that the input wasn't correct
								 */
							default:

								// Strings initialization for next iteration
								sprintf_s(extraMsg, 1024, YELLOW "Please select a valid operation" RESET);
								break;
							}
						}
						break;

						/* Filter removal
						 * Provides a menu that shows an option for every
						 * added filter, giving the opportunity to remove it
						 */
					case '-':

						// Reset extra message string so they can be overwritten for next iteration
						nullString(extraMsg, 1024);

						done = 0; // Flag for the inner loop
						while (done == 0) {

							// Present menu and read user input
							filterRemoveMenu(f, extraMsg);
							printf(BOLD CYAN);
							choice = getSingleChar();
							printf(RESET);

							// Reset extra message string so they can be overwritten for next iteration
							nullString(extraMsg, 1024);

							/* (U/u) User
							 * (D/d) Starting date
							 * (T/t) Ending date
							 * (P/p) Operation
							 * (I/i) Type outcome
							 * (O/o) Entry outcome
							 * (-) Minimum execution time
							 * (+) Maximum execution time
							 * (X/x) Cancel filter addition and go back to settings
							 */
							switch (choice) {

								// Reset user
							case 'u':
							case 'U':

								// Show only if there is at least one user in the filter
								if (f.userFilters != NULL) {

									// Show only if there are more filters
									if (size(f.userFilters) > 0) {

										// Show current user filters
										char* bf = listToString(f.userFilters, 0);
										printf("Current user filter: " MAGENTA "%s\n" RESET, bf);
										free(bf);

										// Ask which one to delete
										printf("\nType which user to delete: " BOLD CYAN);
										scanf_s("%s", userFilter, USER_LENGTH);
										cleanInputBuffer();
										printf(RESET);
										removeAt(&(f.userFilters), contains(f.userFilters, userFilter) - 1);
										nullString(userFilter, USER_LENGTH);
									}

									// Only one element
									else {
										removeHead(&(f.userFilters));
									}
								}

								done = 1;
								break;

								// Reset starting date
							case 'd':
							case 'D':

								f.startingDate = (time_t)(-1);
								startingDSet = 0;
								done = 1;
								break;

								// Reset ending date
							case 't':
							case 'T':

								f.endingDate = (time_t)(-1);
								endingDSet = 0;
								done = 1;
								break;

								// Reset operation
							case 'p':
							case 'P':

								// Show only if there is at least one operation in the filter
								if (f.operationFilters != NULL) {

									if (size(f.operationFilters) > 0) {

										// Show current operation filters
										char* bf = listToString(f.operationFilters, 0);
										printf("Current operation filter: " MAGENTA "%s\n" RESET, bf);
										free(bf);

										// Ask which operation to delete
										printf("\nType which user to delete: " BOLD CYAN);
										scanf_s("%s", operationFilter, OPERATION_LENGTH);
										cleanInputBuffer();
										printf(RESET);
										removeAt(&(f.operationFilters), contains(f.operationFilters, operationFilter) - 1);
										nullString(operationFilter, USER_LENGTH);

									}

									// Only 1 element
									else {
										removeHead(&(f.operationFilters));
									}
								}
								done = 1;
								break;

								// Reset type
							case 'i':
							case 'I':
								f.typeFilter = no_type;
								done = 1;
								break;

								// Reset outcome
							case 'o':
							case 'O':
								f.outcomeFilter = unset;
								done = 1;
								break;

								// Reset minimum execution time
							case '-':
								f.minExecutionTime = 0.0;
								done = 1;
								break;

								// Reset maximum execution time
							case '+':
								f.maxExecutionTime = DBL_MAX;
								done = 1;
								break;

								/* Closes the current (sub)menu
								 * Resets the strings and, sets the flag 'done' as true
								 * and, next iteration, goes back to settins (sub)menu
								 */
							case 'x':
							case 'X':

								// Reset extra message string so they can be overwritten for next iteration
								nullString(extraMsg, 1024);

								done = 1; // We are done with this inner loop
								choice = '\0'; // Avoids exiting 2 loops at once
								break;

								/* Wrong input
								 * Initialize extra message to show that the input wasn't correct
								 */
							default:

								// Strings initialization for next iteration
								sprintf_s(extraMsg, 1024, YELLOW "Please select a valid operation" RESET);
								break;
							}
						}
						break;

						/* Logical operator switch for filters
						 * Offers the possibility to switch between AND and OR logical operator
						 * for filters, meaning during the analysis either AND or OR will be used
						 * to confront the entries with the filters
						 */
					case 'l':
					case 'L':

						// Invert the flag
						globalOrFilters = 1 - globalOrFilters;
						break;

						/* Statistic selection
						 * Provides a menu to select a statistic to extract
						 * All statistics are printed, all but the selected one as options
						 * And the selected one, to show, which one is selected
						 */
					case 'm':
					case 'M':

						// Reset extra message string so they can be overwritten for next iteration
						nullString(extraMsg, 1024);

						done = 0; // Flag for the inner loop
						while (done == 0) {

							// Present menu and read user input
							statisticMenu(as, extraMsg);
							printf(BOLD CYAN);
							choice = getSingleChar();
							printf(RESET);

							// Reset extra message string so they can be overwritten for next iteration
							nullString(extraMsg, 1024);

							/* (0) Entries count
							 * (1) Average execution time
							 * (2) Trend of entries type
							 * (3) Trend of entries outcome
							 * (X/x) Cancels selection and goes back to settings tab
							 */
							switch (choice) {

								// Entries count
							case '0':
								as = countEntries;
								operation = &executeEntryCount;
								done = 1;
								break;

								// Average execution time
							case '1':
								as = avgEx;
								operation = &executeAvgExTime;
								done = 1;
								break;

								// Trend of entries type
							case '2':
								as = typTrnd;
								operation = &executeTrendType;
								done = 1;
								break;

								// Trend of entries outcome
							case '3':
								as = outTrnd;
								operation = &executeTrendOutcome;
								done = 1;
								break;

								/* Closes the current (sub)menu
								 * Resets the strings and, sets the flag 'done' as true
								 * and, next iteration, goes back to settins (sub)menu
								 */
							case 'x':
							case 'X':

								// Reset extra message string so they can be overwritten for next iteration
								nullString(extraMsg, 1024);

								done = 1; // We are done with this inner loop
								choice = '\0'; // Avoids exiting 2 loops at once
								break;

								/* Wrong input
								 * Initialize extra message to show that the input wasn't correct
								 */
							default:

								// Strings initialization for next iteration
								sprintf_s(extraMsg, 1024, YELLOW "Please select a valid operation" RESET);
								break;
							}
						}
						break;

						/* Actual analysis
						 * We search in the file for every entry that
						 * matches our filters and use it to extract
						 * the interested statistic
						 */
					case 's':
					case 'S':

						// Reset entry to avoid dirty readings
						resetEntry(&logEn);

						// Reset every counter
						entryCount = 0;
						filteredEntryCount = 0;
						avgExTime = 0.0;
						infoCounter = 0;
						warningCounter = 0;
						errorCounter = 0;
						successCounter = 0;
						failureCounter = 0;

						// Read an entry from log file and Store it in the LogEntry struct 'logEn'
						int r, matches, flag;
						while ((r = readEntry(&logEn, logFile)) == 0) {

							// Reset the flag for every entry
							matches = 1;

							// Check only if at least one filter is applied, otherwise, it's automatically a match
							if (filterNumber(f) > 0) {

								// Using AND, we start at 1 and negate it if false, using OR, we start at 0 and negate it if true
								matches -= globalOrFilters;

								// Check wheter the values match with the filters
								if (f.userFilters != NULL) {

									// Suppose that it doesn't match and manually set it to true if does
									flag = 0; 
									int maxit = size(f.userFilters);
									for (int i = 0; i < maxit; i++) {
										if (contains(f.userFilters, logEn.user)) {
											flag = 1;
											break;
										}
									}
									matches = globalOrFilters ? (matches | flag) : (matches & flag);
								}

								// Same thing as with user, suppose that it doesn't match and manually set it to true if it does
								if (f.operationFilters != NULL) {
									flag = 0;
									int maxit = size(f.operationFilters);
									for (int i = 0; i < maxit; i++) {
										if (contains(f.operationFilters, logEn.operation)) {
											flag = 1;
											break;
										}
									}
									matches = globalOrFilters ? (matches | flag) : (matches & flag);
								}

								// Starting date
								if (f.startingDate != (time_t)(-1)) {
									flag = (difftime(logEn.date, f.startingDate) >= 0);
									matches = globalOrFilters ? (matches | flag) : (matches & flag);
								}

								// Ending date
								if (f.endingDate != (time_t)(-1)) {
									flag = (difftime(logEn.date, f.endingDate) <= 0);
									matches = globalOrFilters ? (matches | flag) : (matches & flag);
								}

								// Entry type
								if (f.typeFilter != no_type) {
									flag = (logEn.type == f.typeFilter);
									matches = globalOrFilters ? (matches | flag) : (matches & flag);
								}

								// Outcome 
								if (f.outcomeFilter != unset) {
									flag = (logEn.outcome == f.outcomeFilter);
									matches = globalOrFilters ? (matches | flag) : (matches & flag);
								}

								// Execution time, consider only if it's manually set
								if ((0 < f.minExecutionTime) || (f.maxExecutionTime < DBL_MAX)) {
									flag = (f.minExecutionTime <= logEn.executionTime) && (logEn.executionTime <= f.maxExecutionTime);
									matches = globalOrFilters ? (matches | flag) : (matches & flag);
								}
							}
							// If they do, extract what is needed to extract the chosen statistic
							if (matches) {
								operation(&logEn, &filteredEntryCount, &avgExTime, &successCounter, &failureCounter, &infoCounter, &warningCounter, &errorCounter);
							}
							entryCount++;
						}

						// Error related to in-file structure, or with conversion from file to struct
						if (r == 1) {
							sprintf_s(extraMsg, 1024, RED "Possible log file corrupted" RESET);
							analysisOutcome = failure;
						}

						// We exited because there are no more entries (presumably)
						else {
							analysisOutcome = success;
							sprintf_s(extraMsg, 1024, GREEN "Results avaiable at 'Results' tab" RESET);
							fseek(logFile, 0, SEEK_SET);
						}
						break;

						/* Shows the result of the analysis
						 * We have the option of saving this result, in the 'analysis_date@time.txt'
						 * Any character will bring us back
						 */
					case 'r':
					case 'R':

						// Print results only if the previous analysis was successful (there are no results otherwise, this option would be hidden)
						if (analysisOutcome == success) {

							// Print the number of all entries analyzed
							printf("Number of entries analyzed: {" CYAN "%d" RESET "}\n", entryCount);

							// Print the statistic we are interested in
							switch (as) {

								// Number of entries consistent with the filters
							case countEntries:
								printf("Of which [" MAGENTA "%d" RESET "] match the selected filters\n", filteredEntryCount);
								break;

								// Average execution time
							case avgEx:
								if (filteredEntryCount != 0) {
									if (logEn.outcome == success) {
										printf("Average execution time [" MAGENTA "%.4lf" RESET "]\n", (avgExTime / (double)filteredEntryCount));
									}
									else {
										printf("Average time before crash [" MAGENTA "%.4lf" RESET "]\n", (avgExTime / (double)filteredEntryCount));
									}
								}
								// No entry to consider for execution time
								else {
									printf("No entries found that matched the filters, no statistic extracted\n");
								}
								break;

								// Trend of entry type
							case typTrnd:
								if (filteredEntryCount != 0) {
									printf("Number of entries flagged as " GREEN "Success" RESET " [" MAGENTA "%d" RESET "\n", successCounter);
									printf("Number of entries flagged as " RED "Failure" RESET " [" MAGENTA "%d" RESET "\n", failureCounter);
								}
								// No entry to consider for execution time
								else {
									printf("No entries found that matched the filters, no statistic extracted\n");
								}
								break;

								// Trend of entry outcome
							case outTrnd:
								if (filteredEntryCount != 0) {
									printf("Number of entries flagged as " BLUE "Information" RESET " [" MAGENTA "%d" RESET "\n", infoCounter);
									printf("Number of entries flagged as " YELLOW "Warning" RESET " [" MAGENTA "%d" RESET "\n", warningCounter);
									printf("Number of entries flagged as " RED "Error" RESET " [" MAGENTA "%d" RESET "\n", errorCounter);
								}
								// No entry to consider for execution time
								else {
									printf("No entries found that matched the filters, no statistic extracted\n");
								}
								break;
							}

							// Ask to save results or not
							printf("\n[" BOLD GREEN "S" RESET "/" BOLD GREEN "s" RESET "] Save results");
							printf("\n[" BOLD RED "Any other key" RESET "] Cancel\n" BOLD CYAN);
							choice = getSingleChar();
							printf(RESET);

							// If we want to save
							if (choice == 's' || choice == 'S') {

								// Save current date and time to get an unique result name file
								time_t rawtime = time(NULL);
								struct tm timeinfo;
								localtime_s(&timeinfo, &rawtime);

								// Elaborate result file name
								char resFName[1024];
								sprintf_s(resFName, 1024, "results\\analysis_%d-%d-%d_%d-%d-%d.txt",
									timeinfo.tm_mday,
									timeinfo.tm_mon + 1,
									timeinfo.tm_year + 1900,
									timeinfo.tm_hour,
									timeinfo.tm_min,
									timeinfo.tm_sec
								);

								// Open file
								FILE* results;
								if (fopen_s(&results, resFName, "w") == 0) {

									// Save the number of all entries analyzed
									fprintf(results, "Number of entries analyzed: {%d}\n", entryCount);

									// Save the statistic we are interested in
									switch (as) {

										// Number of entries consistent with the filters
									case countEntries:
										fprintf(results, "Of which [%d] match the selected filters\n", filteredEntryCount);
										break;

										// Average execution time
									case avgEx:
										if (filteredEntryCount != 0) {
											if (logEn.outcome == success) {
												fprintf(results, "Average execution time [%.4lf]\n", (avgExTime / (double)filteredEntryCount));
											}
											else {
												fprintf(results, "Average time before crash [%.4lf]\n", (avgExTime / (double)filteredEntryCount));
											}
										}
										else {
											fprintf(results, "No entries found that matched the filters, no statistic extracted\n");
										}
										break;

										// Trend of entry type
									case typTrnd:
										if (filteredEntryCount != 0) {
											fprintf(results, "Number of entries flagged as Success [%d\n", successCounter);
											fprintf(results, "Number of entries flagged as Failure [%d\n", failureCounter);
										}
										else {
											fprintf(results, "No entries found that matched the filters, no statistic extracted\n");
										}
										break;

										// Trend of entry outcome
									case outTrnd:
										if (filteredEntryCount != 0) {
											fprintf(results, "Number of entries flagged as Information [%d\n", infoCounter);
											fprintf(results, "Number of entries flagged as Warning [%d\n", warningCounter);
											fprintf(results, "Number of entries flagged as Error [%d\n", errorCounter);
										}
										else {
											fprintf(results, "No entries found that matched the filters, no statistic extracted\n");
										}
										break;
									}
									printf(GREEN "Results saved in 'LogAnalizer\\%s'\n" RESET, resFName);
								}

								// If file could not be created
								else {
									printf(RED "There was a problem creating results file\n" RESET);
								}

								// Wait for any input to go back to settings
								printf(GREY "\nPress any character to continue..." RESET);
								getSingleChar();
							}
						}

						// In case the analysis was not succesful the operation is hidden, as there is no statistic to show
						else {
							sprintf_s(extraMsg, 1024, YELLOW "Please select a valid operation" RESET);
						}

						choice = '\0'; // Avoid to exit 2 menus at once

						break;

						/* Closes the current menu
						 * Resets the strings and, just that, it goes
						 * back to the main menu next iteration
						 */
					case 'x':
					case 'X':

						// Reset extra message string so they can be overwritten for next iteration
						nullString(extraMsg, 1024);
						break;

						/* Wrong input
						 * Initialize extra message to show that the input wasn't correct
						 */
					default:

						// Strings initialization for next iteration
						sprintf_s(extraMsg, 1024, YELLOW "Please select a valid operation" RESET);
						break;
					}
				} while (choice != 'x');
				choice = '\0'; // Avoids exiting 2 loops at once
			}

			// Operation is hidden until at least a file has been opened
			else {
				// Strings initialization for next iteration
				sprintf_s(extraMsg, 1024, YELLOW "Please select a valid operation" RESET);
			}
			break;

			/* Closes the application
			 * Resets the strings and, just that, it exits
			 * the while loop next iteration
			 */
		case 'x':
		case 'X':

			// Reset extra message string so they can be overwritten for next iteration
			nullString(extraMsg, 1024);
			printf("Closing...");
			break;

			/* Wrong input
			 * Initialize extra message to show that the input wasn't correct
			 */
		default:

			// Strings initialization for next iteration
			sprintf_s(extraMsg, 1024, YELLOW "Please select a valid operation" RESET);
			break;
		}

	}

	// Close file if it was opened
	if (logFile != NULL) fclose(logFile);
	resetEntryFilter(&f);
	return 0;
}

/**
 * Updates the counter 'count'
 * This counts towards the 'entry count' statistic
 */
void executeEntryCount(LogEntry* entry, int* count, double* sum, int* sucCntr, int* failCntr, int* infoCntr, int* warningCntr, int* errCntr) {
	(*count)++;
}

/**
 * Updates the counters 'count' and 'sum'
 * This counts towards the 'average execution time' statistic
 */
void executeAvgExTime(LogEntry* entry, int* count, double* sum, int* sucCntr, int* failCntr, int* infoCntr, int* warningCntr, int* errCntr) {
	(*count)++;
	(*sum) += entry->executionTime;
}

/**
 * Updates either the 'info', 'warning' or 'error counter
 * This counts towards the 'type trend' statistic
 */
void executeTrendOutcome(LogEntry* entry, int* count, double* sum, int* sucCntr, int* failCntr, int* infoCntr, int* warningCntr, int* errCntr) {
	(*count)++;
	switch (entry->type) {
	case info:
		(*infoCntr)++;
		break;
	case warning:
		(*warningCntr)++;
		break;
	case error:
		(*errCntr)++;
		break;
	}
}

/**
 * Updates either the 'success' or 'failure counter
 * This counts towards the 'outcome trend' statistic
 */
void executeTrendType(LogEntry* entry, int* count, double* sum, int* sucCntr, int* failCntr, int* infoCntr, int* warningCntr, int* errCntr) {
	(*count)++;
	switch (entry->outcome) {
	case success:
		(*sucCntr)++;
		break;
	case failure:
		(*failCntr)++;
		break;
	}
}
