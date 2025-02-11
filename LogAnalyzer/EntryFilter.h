/*
 * Copyright (c) 2024 Biribo' Francesco
 *
 * Permission to use, copy, modify, and distribute this software for any purpose with or without fee is hereby granted, provided that the above copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef ENTRYFILTER_H
#define ENTRYFILTER_H

#include <time.h>
#include <stdio.h>
#include "DLinkedList.h"
#include "LogEntry.h"

/**
 * Entry Filter
 *
 * Fields:
 *	UserFilters -> List of users
 *	StartingDate -> Minimum date at which the entry was created (approximatly when the event occured)
 *	EndingDate -> Maximum date at which the entry was created (approximatly when the event occured)
 *	OperationFilters -> List of operations
 *	TypeFilter -> Indicates the type of the entry
 *	OutcomeFilter -> Outcome of the entry
 *	Minimum Execution time -> Minimum execution time of the operation (or time before failure)
 *	Maximum Execution time -> Maximum execution time of the operation (or time before failure)
 *  MaxEntryCount -> Maximum number of entries actually considered
 */
typedef struct
{
	DLinkedList* userFilters;
	time_t startingDate;
	time_t endingDate;
	DLinkedList* operationFilters;
	enum info_type typeFilter;
	enum outcomes outcomeFilter;
	double minExecutionTime;
	double maxExecutionTime;
	int maxEntryCount;
} EntryFilter;

/**
 * Returns the number of specified filters in f
 */
int filterNumber(EntryFilter f);

/**
 * Resets the given entry filter, resetting every field
 *
 * Every field is set to its default value
 */
void resetEntryFilter(EntryFilter* f);

#endif