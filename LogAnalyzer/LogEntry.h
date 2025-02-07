/*
 * Copyright (c) 2024 Biribo' Francesco
 *
 * Permission to use, copy, modify, and distribute this software for any purpose with or without fee is hereby granted, provided that the above copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef LOGENTRY_H
#define LOGENTRY_H

#include <time.h>
#include <stdio.h>

#define USER_LENGTH 32
#define OPERATION_LENGTH 128

/**
 * Enumeration that represents the type of a log entry
 *
 * That can be an Information entry, a Warning entry or an Error entry
 * 'no_type' is used to represent an un-initialized value
 */
enum info_type { no_type = -1, info, warning, error };

/**
 * Enumeration that represents the outcome of a log entry
 *
 * That can be either Failure or Success
 * 'unset' is used to represent an un-initialized value
 */
enum outcomes { unset = -1, failure, success };

/**
 * Log entry
 *
 * Fields:
 *	User -> User associated to the event referenced in the entry
 *	Date -> Date at which the entry was created (approximatly when the event occured)
 *	Operation -> Operation performed (the event)
 *	Type -> Indicates the type of the entry
 *	Outcome -> Outcome of the entry
 *	Execution time -> Execution time of the operation (or time before failure)
 */
typedef struct
{
	char user[USER_LENGTH];
	time_t date;
	char operation[OPERATION_LENGTH];
	enum info_type type;
	enum outcomes outcome;
	double executionTime;
} LogEntry;

/**
 * Prints the text associated to the type
 *
 * 'info' (0) -> Information
 * 'warning' (1) -> Warning
 * 'error' (2) -> Error
 */
void printInfoType(enum info_type t);

/**
 * Prints the text associated to the outcome
 *
 * 'failure' (0) -> Failure
 * 'success' (1) -> Success
 */
void printOutcome(enum outcomes out);

/**
 * Resets the given entry, resetting every field
 *
 * Every field is set to its default value
 */
void resetEntry(LogEntry* entry);

#endif