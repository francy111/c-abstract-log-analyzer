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