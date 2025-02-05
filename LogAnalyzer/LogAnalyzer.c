#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "LogEntry.h"
#include "Utility.h"
#include <float.h>
#include <time.h>
#include <limits.h>
#include <windows.h>

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

int main(int argc, char *argv[]) {

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

	char userFilter[USER_LENGTH]; // We look for entries that have the same user as this filter
	nullString(userFilter, USER_LENGTH); // Empty buffer to be sure

	time_t startingDatet = (time_t)(-1); // We look for entries created after this date (inclusive)
	time_t endingDatet = (time_t)(-1); // We look for entries create before this date (inclusive)

	char operationFilter[OPERATION_LENGTH]; // We look for entries that have the same value (operation) as this filter
	nullString(operationFilter, OPERATION_LENGTH); // Empty buffer to be sure

	enum info_type typeFilter = no_type; // We look for entries with the same type (Info/Warning/Error)
	enum outcomes outcomeFilter = unset; // We look for entries with the same outcome (Failure/Success)

	double minExTime = 0.0; // We look for entries with an execution time greater than this filter (inclusive)
	double maxExTime = DBL_MAX; // We look for entries with an execution time less than this filter (inclusive)

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

	char extraMsg[1024]; // Buffer that holds a message that is shown during the next iteration
	nullString(extraMsg, 1024);

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
				if(logFile != NULL) 
					fclose(logFile);

				// Replace the 'current' file with the 'new' one
				logFile = tmp;
				strncpy_s(relativeFilePath, _MAX_PATH, tmpStr, strlen(tmpStr));

				// Show this success message
				sprintf_s(extraMsg, 1024, GREEN "Opened new file" RESET);
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
					logAnalysisMenu(as, userFilter, startingDatet, endingDatet, operationFilter, typeFilter, outcomeFilter, minExTime, maxExTime, extraMsg, analysisOutcome);
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
							filterAddMenu(userFilter, startingDatet, endingDatet, operationFilter, typeFilter, outcomeFilter, minExTime, maxExTime, extraMsg);
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
								printf("Digita utente: " BLUE CYAN);
								scanf_s("%s", userFilter, USER_LENGTH);
								printf(RESET);
								cleanInputBuffer();
								done = 1;
								break;

								// Specify starting date
							case 'd':
							case 'D':

								// Acquire starting date 
								getDateTime(&st);
								startingDatet = mktime(&st);

								// Set error message if the date was invalid
								if (startingDatet == (time_t)(-1)) {
									sprintf_s(extraMsg, 1024, RED "Please enter a valid date" RESET);
								}
								else {

									// If it is valid and an ending date was set, check if they are coherent
									if ((endingDatet != (time_t)(-1)) && difftime(endingDatet, startingDatet) < 0) {
										sprintf_s(extraMsg, 1024, RED "Starting date must be before ending date" RESET);
										startingDatet = (time_t)(-1);
									}
								}
								done = 1;
								break;

								// Specify ending date
							case 't':
							case 'T':

								// Acquire ending date 
								getDateTime(&et);
								endingDatet = mktime(&et);

								// Set error message if the date was invalid
								if (endingDatet == (time_t)(-1)) {
									sprintf_s(extraMsg, 1024, RED "Please enter a valid date" RESET);
								}
								else {

									// If it is valid and a starting date was set, check if they are coherent
									if ((startingDatet != (time_t)(-1)) && difftime(endingDatet, startingDatet) < 0) {
										sprintf_s(extraMsg, 1024, RED "Ending date must be after starting date" RESET);
										endingDatet = (time_t)(-1);
									}
								}
								done = 1;
								break;

								// Specify operation
							case 'p':
							case 'P':

								// Acquire operation
								printf("Digita operazione: " BLUE CYAN);
								scanf_s("%s", operationFilter, OPERATION_LENGTH);
								printf(RESET);
								cleanInputBuffer();
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
									typeFilter = info;
									break;

									// Warning
								case 'w':
								case 'W':
									typeFilter = warning;
									break;

									// Error
								case 'e':
								case 'E':
									typeFilter = error;
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
									outcomeFilter = success;
									break;

									// Failure
								case 'f':
								case 'F':
									outcomeFilter = failure;
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
								scanf_s("%lf", &minExTime);
								printf(RESET);
								cleanInputBuffer();

								// Check if it makes sense (at least 0)
								if (minExTime < 0.0) {
									minExTime = 0.0;
									sprintf_s(extraMsg, 1024, RED "Execution time must be at least 0.0s" RESET);
								}
								else {

									// Check if it's coherent with maximum execution time, if specified
									if (minExTime >= maxExTime) {
										minExTime = 0.0;
										sprintf_s(extraMsg, 1024, RED "Minimum execution time must be lower than the maximum" RESET);
									}
								}
								done = 1;
								break;

								// Specify maximum execution time
							case '+':

								// Acquire time
								printf("Tempo esecuzione massimo: " BLUE CYAN);
								scanf_s("%lf", &maxExTime);
								printf(RESET);
								cleanInputBuffer();

								// Check if it makes sense (at least 0)
								if (maxExTime < 0.0) {
									maxExTime = DBL_MAX;
									sprintf_s(extraMsg, 1024, RED "Execution time must be at least 0.0s" RESET);
								}
								else {

									// Check if it's coherent with minimum execution time, if specified
									if (minExTime >= maxExTime) {
										maxExTime = DBL_MAX;
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
							filterRemoveMenu(userFilter, startingDatet, endingDatet, operationFilter, typeFilter, outcomeFilter, minExTime, maxExTime, extraMsg);
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
								nullString(userFilter, USER_LENGTH);
								done = 1;
								break;

								// Reset starting date
							case 'd':
							case 'D':
								startingDatet = (time_t)(-1);
								done = 1;
								break;

								// Reset ending date
							case 't':
							case 'T':
								endingDatet = (time_t)(-1);
								done = 1;
								break;

								// Reset operation
							case 'p':
							case 'P':
								nullString(operationFilter, OPERATION_LENGTH);
								done = 1;
								break;

								// Reset type
							case 'i':
							case 'I':
								typeFilter = no_type;
								done = 1;
								break;

								// Reset outcome
							case 'o':
							case 'O':
								outcomeFilter = unset;
								done = 1;
								break;

								// Reset minimum execution time
							case '-':
								minExTime = 0.0;
								done = 1;
								break;

								// Reset maximum execution time
							case '+':
								maxExTime = DBL_MAX;
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
						filteredEntryCount;
						avgExTime = 0.0;
						infoCounter = 0;
						warningCounter = 0;
						errorCounter = 0;
						successCounter = 0;
						failureCounter = 0;

						// Read an entry from log file and Store it in the LogEntry struct 'logEn'
						int r, matches = 1;
						while ((r = readEntry(&logEn, logFile)) == 0) {

							// Check wheter the values match with the filters
							if ((userFilter[0] != '\0') && strncmp(logEn.user, userFilter, USER_LENGTH)) {
								matches = 0;
							}
							if ((startingDatet != (time_t)(-1)) && (difftime(logEn.date, startingDatet) < 0)) {
								matches = 0;
							}
							if ((endingDatet != (time_t)(-1)) && (difftime(logEn.date, endingDatet) > 0)) {
								matches = 0;
							}
							if ((operationFilter[0] != '\0') && (strncmp(logEn.user, userFilter, USER_LENGTH))) {
								matches = 0;
							}
							if ((typeFilter != no_type) && (logEn.type != typeFilter)) {
								matches = 0;
							}
							if ((outcomeFilter != unset) && (logEn.outcome != outcomeFilter)) {
								matches = 0;
							}
							if ((logEn.executionTime < minExTime) || (logEn.executionTime > maxExTime)) {
								matches = 0;
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
							printf("READ: %d\n", entryCount);

							// Print the statistic we are interested in
							switch (as) {

								// Number of entries consistent with the filters
							case countEntries:
								printf("under filters: %d\n", filteredEntryCount);
								break;

								// Average execution time
							case avgEx:
								printf("AVG EX TIME %.4lf", (avgExTime / (double)filteredEntryCount));
								break;

								// Trend of entry type
							case typTrnd:
								printf("Success: %d\n", successCounter);
								printf("Failure: %d\n", failureCounter);
								break;

								// Trend of entry outcome
							case outTrnd:
								printf("info: %d\n", infoCounter);
								printf("warning: %d\n", warningCounter);
								printf("error: %d\n", errorCounter);
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
									fprintf(results, "READ: %d\n", entryCount);

									// Save the statistic we are interested in
									switch (as) {

										// Number of entries consistent with the filters
									case countEntries:
										fprintf(results, "under filters: %d\n", filteredEntryCount);
										break;

										// Average execution time
									case avgEx:
										fprintf(results, "AVG EX TIME %.4lf", (avgExTime / (double)filteredEntryCount));
										break;

										// Trend of entry type
									case typTrnd:
										fprintf(results, "Success: %d\n", successCounter);
										fprintf(results, "Failure: %d\n", failureCounter);
										break;

										// Trend of entry outcome
									case outTrnd:
										fprintf(results, "info: %d\n", infoCounter);
										fprintf(results, "warning: %d\n", warningCounter);
										fprintf(results, "error: %d\n", errorCounter);
										break;
									}

									printf(GREEN "Results saved in 'LogAnalizer\\%s'\n" RESET, resFName);
								}

								// If file could not be created
								else {
									printf(RED "There was a problem creating results file\n" RESET);
								}

								// Wait for any input to go back to settings
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
	if(logFile != NULL) fclose(logFile);
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
	switch (entry->outcome) {
	case success:
		(*sucCntr)++;
		break;
	case failure:
		(*failCntr)++;
		break;
	}
}
