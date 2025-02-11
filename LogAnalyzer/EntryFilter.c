/*
 * Copyright (c) 2024 Biribo' Francesco
 *
 * Permission to use, copy, modify, and distribute this software for any purpose with or without fee is hereby granted, provided that the above copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include "EntryFilter.h"
#include <float.h>
#include <limits.h>

/**
 * Returns the number of specified filters in f
 */
int filterNumber(EntryFilter f) {
	int count = 0;
	if (f.userFilters != NULL) count++;
	if (f.operationFilters != NULL) count++;
	if (f.startingDate != (time_t)(-1)) count++;
	if (f.endingDate != (time_t)(-1)) count++;
	if (f.typeFilter != no_type) count++;
	if (f.outcomeFilter != unset) count++;
	if (f.minExecutionTime > 0.0) count++;
	if (f.maxExecutionTime < DBL_MAX) count++;
	return count;
}

/**
 * Resets the given entry filter, resetting every field
 *
 * Every field is set to its default value
 */
void resetEntryFilter(EntryFilter* f) {
	deleteList(&(f->userFilters));
	f->startingDate = (time_t)(-1);
	f->endingDate = (time_t)(-1);
	deleteList(&(f->operationFilters));
	f->typeFilter = no_type;
	f->outcomeFilter = unset;
	f->minExecutionTime = 0.0;
	f->maxExecutionTime = DBL_MAX;
	f->maxEntryCount = INT_MAX;
}
