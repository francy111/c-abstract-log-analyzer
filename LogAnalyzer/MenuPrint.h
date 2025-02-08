/*
 * Copyright (c) 2024 Biribo' Francesco
 *
 * Permission to use, copy, modify, and distribute this software for any purpose with or without fee is hereby granted, provided that the above copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef MENUPRINT_H
#define MENUPRINT_H

#include "Utility.h"
#include "LogEntry.h"
#include "DLinkedList.h"
#include "EntryFilter.h"

#define MAX_PATH 260

/**
 * Reads an entry from the log file and stores it in 'entry'
 *
 * 'filePtr' is a pointer to a file, which we suppose is a log file
 *
 * Returns either 0, if the operation was successful, 1, if there
 * were any error in the formatting in the file, or -1 if there was
 * an error reading from the file (also when we reached the end)
 */
int readEntry(LogEntry* entry, FILE* filePtr);

/**
 * Prints the main menu which displays 3 options:
 *  (f) Specifing a different log file to analyze
 *  (s) Open settings to set filters / interested statistic
 *  (x) Close application
 *
 * Also prints the currently open log file and an optional extra messsage
 * (Usually used as a feedback for the previous iteration)
 */
void mainMenu(char* cd, char* filePath, char* extraMsg);

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
void logAnalysisMenu(enum analysis_operation operation, int filterOrFlag, EntryFilter f, char* extraMsg, enum outcomes analysisOutcome);

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
void filterAddMenu(EntryFilter f, char* extraMsg);

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
void filterRemoveMenu(EntryFilter f, char* extraMsg);

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
void statisticMenu(enum analysis_statistic as, char* extraMsg);

#endif