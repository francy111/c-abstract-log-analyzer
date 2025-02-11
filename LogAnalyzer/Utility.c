/*
 * Copyright (c) 2024 Biribo' Francesco
 *
 * Permission to use, copy, modify, and distribute this software for any purpose with or without fee is hereby granted, provided that the above copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include "Utility.h"
#include "LogEntry.h"
#include "cJSON.h"
#include <windows.h>
#include <stdio.h>
#include <float.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
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
	return;
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
	return;
}


/**
 * Acquires the date component of a date_time structure from
 * the standard input (stdin) and stores them in the
 * structure pointed by 'dateTime'
 *
 * The date is acquired first using the 'dd/mm/yyyy' format
 */
void getDate(struct tm* dateTime) {

	int done = 0;
	int day, month, year;
	while (done == 0) {
		printf("Enter date in dd/mm/yyyy format: " BOLD CYAN);
		if (scanf_s("%d/%d/%d", &day, &month, &year) != 3
			) {
			printf(BOLD RED "Invalid format\n");
		}
		else {
			if (dateValidity(day, month, year)) {
				done = 1;
			}
			else {
				printf(BOLD RED "Insert coherent day/month/year values\n");
			}
		}
		printf(RESET);
		cleanInputBuffer();
	}
	dateTime->tm_mday = day;
	dateTime->tm_mon = month - 1; // Adjusting since months are stored from 0 to 11
	dateTime->tm_year = year - 1900; // Adjusting since the year should be the number of years since 1900

	// If we are acquiring the date for the first time
	time_t dTCheck = mktime(dateTime);
	if (dTCheck == (time_t)(-1)) {
		dateTime->tm_hour = 0;
		dateTime->tm_min = 0;
		dateTime->tm_sec = 0;
	}
	return;
}

/**
 * Acquires the time component of a date_time structure from
 * the standard input (stdin) and stores them in the
 * structure pointed by 'dateTime'
 *
 * The time is acquired using the 'hh:mm:ss' format
 */
void getTime(struct tm* dateTime) {

	int done = 0;
	int hours, minutes, seconds;
	while (done == 0) {
		printf("Enter time in hh:mm:ss format: " BOLD CYAN);
		if (scanf_s("%d:%d:%d", &hours, &minutes, &seconds) != 3
			) {
			printf(BOLD RED "Invalid format\n");
		}
		else {
			if (timeValidity(hours, minutes, seconds)) {
				done = 1;
			}
			else {
				printf(BOLD RED "Insert coherent hours/minutes/seconds values\n");
			}
		}
		printf(RESET);
		cleanInputBuffer();

		dateTime->tm_hour = hours;
		dateTime->tm_min = minutes;
		dateTime->tm_sec = seconds;
	}
	return;
}

/**
 * Checks if the given day, month and year values
 * are correct, that is, the day is in range, depending on the month (we include leap years)
 * The year can only be greater or equal 1900
 */
int dateValidity(int day, int month, int year) {

	int valid = 0;

	// Check the year
	if (year >= 1900) {

		// Check the month
		switch (month) {

			// 31 days months
		case 1:
		case 3:
		case 5:
		case 7:
		case 8:
		case 10:
		case 12:

			if ((1 <= day) && (day <= 31)) {
				valid = 1;
			}
			break;

			// 30 days months
		case 4:
		case 6:
		case 9:
		case 11:

			if ((1 <= day) && (day <= 30)) {
				valid = 1;
			}
			break;

			// February, either 28 or 29 days months
		case 2:;

			// If it's a leap year we consider an extra day for february
			int leapYearExtension = (((year % 4 == 0) && (year % 100 != 0)) || (year % 400 == 0)) ? 1 : 0;
			if ((1 <= day) && (day <= 28 + leapYearExtension)) {
				valid = 1;
			}
			break;

			// Keep valid at 0
		default:
			break;
		}


	}

	return valid;
}

/**
 * Checks if the given hours, minutes and seconds values are correct
 * that is, hours from 0 to 23, minutes from 0 to 59 and seconds 0 to 59
 */
int timeValidity(int hours, int minutes, int seconds) {
	return (0 <= hours) && (hours <= 23) && (0 <= minutes) && (minutes <= 59) && (0 <= seconds) && (seconds <= 59);
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
	return;
}

/**
 * Saves the current configurations (selected file and/or filters)
 * in the file with path 'filePath'
 * 
 * Output:
 *	0 -> Error during save
 *  1 -> Saved correctly
 */
int saveConfig(char* filePath, char* logFile, EntryFilter* filters, int statistic, int analysisLogic) {
	int result = 1;
	
	FILE* f;
	fopen_s(&f, filePath, "w");

	if (f != NULL) {
		int atLeast1 = 0;

		cJSON* cfg = cJSON_CreateObject();

		cJSON_AddStringToObject(cfg, "version", CURR_VER);

		if (logFile != NULL && strncmp(logFile, "", 1)) {
			cJSON_AddStringToObject(cfg, "logfile", logFile);
		}

		if ((filters != NULL)) {

			cJSON* filterJSON = cJSON_CreateObject();
			cJSON_AddItemToObject(cfg, "filters", filterJSON);

			// Add user filter if present
			cJSON* users = cJSON_CreateArray();
			if (filters->userFilters != NULL) {
				int j = size(filters->userFilters);
				for (int i = 0; i < j; i++) {
					cJSON_AddItemToArray(users, cJSON_CreateString(getItemAt(filters->userFilters, i)));
				}
			}
			cJSON_AddItemToObject(filterJSON, "users", users);

			// Add operation filter if present
			cJSON* operations = cJSON_CreateArray();
			if (filters->operationFilters != NULL) {
				int j = size(filters->operationFilters);
				for (int i = 0; i < j; i++) {
					cJSON_AddItemToArray(operations, cJSON_CreateString(getItemAt(filters->operationFilters, i)));
				}
			}
			cJSON_AddItemToObject(filterJSON, "operations", operations);

			// Add starting date filter if present
			char date[10];
			nullString(date, 10);
			char time[10];
			nullString(time, 10);
			if (filters->startingDate != (time_t)(-1)) {
				struct tm dt;
				localtime_s(&dt, &(filters->startingDate));
				sprintf_s(date, 10, "%d/%d/%d", dt.tm_mday, dt.tm_mon + 1, dt.tm_year + 1900);
				sprintf_s(time, 10, "%d:%d:%d", dt.tm_hour, dt.tm_min + 1, dt.tm_sec);

			}
			cJSON_AddItemToObject(filterJSON, "startDate", cJSON_CreateString(date));
			cJSON_AddItemToObject(filterJSON, "startTime", cJSON_CreateString(time));

			// Add ending date filter if present
			nullString(date, 10);
			nullString(time, 10);
			if (filters->endingDate != (time_t)(-1)) {
				struct tm dt;
				localtime_s(&dt, &(filters->endingDate));
				sprintf_s(date, 10, "%d/%d/%d", dt.tm_mday, dt.tm_mon + 1, dt.tm_year + 1900);
				sprintf_s(time, 10, "%d:%d:%d", dt.tm_hour, dt.tm_min, dt.tm_sec);
			}
			cJSON_AddItemToObject(filterJSON, "endDate", cJSON_CreateString(date));
			cJSON_AddItemToObject(filterJSON, "endTime", cJSON_CreateString(time));

			// Add type filter if present
			cJSON_AddItemToObject(filterJSON, "type", cJSON_CreateNumber(filters->typeFilter));

			// Add outcome filter if present
			cJSON_AddItemToObject(filterJSON, "outcome", cJSON_CreateNumber(filters->outcomeFilter));

			// Add minimum execution time filter
			cJSON_AddItemToObject(filterJSON, "minExTime", cJSON_CreateNumber(filters->minExecutionTime));

			// Add maximum execution time filter
			if (filters->maxExecutionTime == DBL_MAX) {
				cJSON_AddItemToObject(filterJSON, "maxExTime", cJSON_CreateNumber(-1));
			}
			else {
				cJSON_AddItemToObject(filterJSON, "maxExTime", cJSON_CreateNumber(filters->maxExecutionTime));
			}

			// Add maximum entry count
			if(filters->maxEntryCount == INT_MAX) {
				cJSON_AddItemToObject(filterJSON, "maxCount", cJSON_CreateNumber(-1));
			}
			else {
				cJSON_AddItemToObject(filterJSON, "maxCount", cJSON_CreateNumber(filters->maxEntryCount));
			}

			// Save chosen statistic
			cJSON_AddItemToObject(cfg, "statistic", cJSON_CreateNumber(statistic));

			// Save chosen analysis logic
			char al[4];

			// OR
			if (analysisLogic) {
				al[0] = 'O';
				al[1] = 'R';
				al[2] = '\0';
				al[3] = '\0';
			}

			// AND
			else {
				al[0] = 'A';
				al[1] = 'N';
				al[2] = 'D';
				al[3] = '\0';
			}
			cJSON_AddItemToObject(cfg, "analysisLogic", cJSON_CreateString(al));
		}

		// Save onto the file
		char* jsonString = cJSON_Print(cfg);
		fprintf_s(f, "%s", jsonString);
		result = 0;

		// Cleanup
		fclose(f);
		cJSON_Delete(cfg);
		free(jsonString);
	}
	return result;
}


/**
 * Loads the configurations (selected file and/or filters)
 * from the file with path 'filePath'
 * 
 * Output:
 *	0 -> Error during load
 *  1 -> Loaded correctly
 */
int loadConfig(char* filePath, char* logFile, EntryFilter* filters, int* statistic, int* analysisLogic)  {

	int result = 1;

	FILE* f;
	fopen_s(&f, filePath, "r");

	if (f != NULL) {
		
		// Reset values
		nullString(logFile, _MAX_PATH);
		resetEntryFilter(filters);

		// Get file length
		fseek(f, 0, SEEK_END);
		long fileSize = ftell(f);
		fseek(f, 0, SEEK_SET);

		// Read all the file at once
		char* jsonString = (char*)calloc(fileSize + 1, sizeof(char));
		if (jsonString != NULL) {
			fread_s(jsonString, fileSize, sizeof(char), fileSize, f);
			jsonString[fileSize] = '\0';
		}
		fclose(f);

		// Parse the string
		cJSON* cfg = cJSON_Parse(jsonString);
		if (cfg != NULL) {

			// Check version
			cJSON* ver = cJSON_GetObjectItem(cfg, "version");
			if (ver != NULL && checkVersion(CURR_VER, ver->valuestring)) {

				// File path
				cJSON* logF = cJSON_GetObjectItem(cfg, "logfile");
				char* path = logF->valuestring;

				// Try to open file to check if it is correct or not
				FILE* test;
				fopen_s(&test, path, "r");

				if (test != NULL) {
					fclose(test);

					strncpy_s(logFile, _MAX_PATH, path, strlen(path));

					cJSON* filtersJ = cJSON_GetObjectItem(cfg, "filters");

					// Users		
					cJSON* users = cJSON_GetObjectItem(filtersJ, "users");
					for (int i = 0; i < cJSON_GetArraySize(users); i++) {
						insertTail(&(filters->userFilters), cJSON_GetArrayItem(users, i)->valuestring);
					}

					// Operations
					cJSON* operations = cJSON_GetObjectItem(filtersJ, "operations");
					for (int i = 0; i < cJSON_GetArraySize(operations); i++) {
						insertTail(&(filters->operationFilters), cJSON_GetArrayItem(operations, i)->valuestring);
					}

					// For starting/ending date/time
					char* buff;
					struct tm dt;

					// Starting date
					buff = cJSON_GetObjectItem(filtersJ, "startDate")->valuestring;

					// Empty value
					if (strncmp(buff, "", 1) == 0) {
						filters->startingDate = (time_t)(-1);
					}
					else {
						if (sscanf_s(buff, "%d/%d/%d", &dt.tm_mday, &dt.tm_mon, &dt.tm_year) == 3) {
							buff = cJSON_GetObjectItem(filtersJ, "startTime")->valuestring;
							if (sscanf_s(buff, "%d:%d:%d", &dt.tm_hour, &dt.tm_min, &dt.tm_sec) == 3) {
								dt.tm_mon -= 1;
								dt.tm_year -= 1900;
								filters->startingDate = mktime(&dt);
							}
							else {
								filters->startingDate = (time_t)(-1);
							}
						}
						else {
							filters->startingDate = (time_t)(-1);
						}
					}

					// Ending date
					buff = cJSON_GetObjectItem(filtersJ, "endDate")->valuestring;

					// Empty value
					if (strncmp(buff, "", 1) == 0) {
						filters->endingDate = (time_t)(-1);
					}
					else {
						if (sscanf_s(buff, "%d/%d/%d", &dt.tm_mday, &dt.tm_mon, &dt.tm_year) == 3) {
							buff = cJSON_GetObjectItem(filtersJ, "endTime")->valuestring;
							if (sscanf_s(buff, "%d:%d:%d", &dt.tm_hour, &dt.tm_min, &dt.tm_sec) == 3) {
								dt.tm_mon -= 1;
								dt.tm_year -= 1900;
								filters->endingDate = mktime(&dt);
							}
							else {
								filters->endingDate = (time_t)(-1);
							}
						}
						else {
							filters->endingDate = (time_t)(-1);
						}
					}

					// Type
					cJSON* type = cJSON_GetObjectItem(filtersJ, "type");
					if (type->valueint >= info && type->valueint <= error)
						filters->typeFilter = type->valueint;
					else
						filters->typeFilter = no_type;

					// Outcome
					cJSON* outcome = cJSON_GetObjectItem(filtersJ, "outcome");
					if (outcome->valueint == failure || outcome->valueint == success)
						filters->outcomeFilter = outcome->valueint;
					else
						filters->outcomeFilter = unset;

					// Min ex time
					cJSON* minExTime = cJSON_GetObjectItem(filtersJ, "minExTime");
					filters->minExecutionTime = max(minExTime->valuedouble, 0.0);

					// Max ex time
					cJSON* maxExTime = cJSON_GetObjectItem(filtersJ, "maxExTime");
					filters->maxExecutionTime = maxExTime->valuedouble;
					if (filters->maxExecutionTime <= 0) {
						filters->maxExecutionTime = DBL_MAX;
					}
					else {
						if (filters->maxExecutionTime < filters->minExecutionTime)
							filters->maxExecutionTime = DBL_MAX;
					}

					// Max entry count
					cJSON* maxEntryCount = cJSON_GetObjectItem(filtersJ, "maxCount");
					filters->maxEntryCount = maxEntryCount->valueint;
					if (filters->maxEntryCount <= 0) {
						filters->maxEntryCount = INT_MAX;
					}

					// Load chosen statistic
					int stat = cJSON_GetObjectItem(cfg, "statistic")->valueint;
					if (stat >= 0 && stat <= 3) {
						*statistic = stat;
					}
					else {
						*statistic = countEntries;
					}

					// Load chosen analysis logic
					char* al;
					al = cJSON_GetObjectItem(cfg, "analysisLogic")->valuestring;

					/// Strncmp returns 0 
					(*analysisLogic) = (strncmp(al, "OR", 2) == 0);
				}
				cJSON_Delete(cfg);
				result = 0;
			}
		}
		free(jsonString);
	}
	return result;
}

/**
 * Checks if the given version is compatible
 * with the current one
 * x.y.z
 *   Must have the save x
 *   The y can only be max 2 numbers apart
 *   No requirements for z
 */
int checkVersion(char* curVer, char* ver) {
	int check = 0;

	// Check if the x's are the same
	int curX, x;
	sscanf_s(curVer, "%d", &curX);
	sscanf_s(ver, "%d", &x);

	if (curX == x) {

		// Check if the y's are at max 2 numbers apart
		int curY, y;
		sscanf_s(curVer, "%d", &curY);
		sscanf_s(ver, "%d", &y);

		if (abs(curY - y) <= 2) {
			check = 1;
		}
	}
	return check;
}