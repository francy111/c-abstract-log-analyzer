#include "LogEntry.h"
#include "Utility.h"

/**
 * Prints the text associated to the type
 *
 * 'info' (0) -> Information
 * 'warning' (1) -> Warning
 * 'error' (2) -> Error
 */
void printInfoType(enum info_type t) {
	switch (t) {
		case info:
			printf(BOLD BLUE "Information" RESET);
			break;
		case warning:
			printf(BOLD YELLOW "Warning" RESET);
			break;
		case error:
			printf(BOLD RED "Error" RESET);
			break;
	}
}

/**
 * Prints the text associated to the outcome
 *
 * 'failure' (0) -> Failure
 * 'success' (1) -> Success
 */
void printOutcome(enum outcomes out) {
	switch (out) {
	case success:
		printf(BOLD GREEN "Success" RESET);
		break;
	case failure:
		printf(BOLD RED "Failure" RESET);
		break;
	}
}

/**
 * Resets the given entry, resetting every field
 *
 * Every field is set to its default value
 */
void resetEntry(LogEntry* entry) {
	nullString(entry->user, USER_LENGTH);
	entry->date = (time_t)(-1);
	nullString(entry->operation, OPERATION_LENGTH);
	entry->type = no_type;
	entry->outcome = unset;
	entry->executionTime = 0.0;
}

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
		if (sscanf_s(token, "%d.%d.%d", &(dateTime.tm_mday), &(dateTime.tm_mon), &(dateTime.tm_year)) != 3) {
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
		} else if (strncmp("Failure", token, strlen("Failure")) == 0) {
			entry->outcome = failure;
		} else {
			result = 1;
		}

		// Execution time
		token = strtok_s(NULL, "-", &context);
		if (sscanf_s(token, "%lf", &(entry->executionTime)) != 1) {
			result = 1;
		}
		
	} else {
		result = -1;
	}
	return result;
}