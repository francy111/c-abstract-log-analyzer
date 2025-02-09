/*
 * Copyright (c) 2024 Biribo' Francesco
 *
 * Permission to use, copy, modify, and distribute this software for any purpose with or without fee is hereby granted, provided that the above copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef UTILITY_H
#define UTILITY_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "EntryFilter.h"

#define GREEN "\x1b[32m"
#define YELLOW "\x1b[33m"
#define RED "\x1b[31m"
#define BLUE "\x1b[34m"
#define MAGENTA "\x1b[35m"
#define CYAN "\x1b[36m"
#define GREY "\x1b[90m"
#define RESET "\x1b[0m"

#define CLEAR_SCREEN "\x1B[3J\x1B[H\x1B[2J"

#define UNDERLINE "\x1B[4m"
#define BOLD "\x1B[1m"
#define WHITEBG "\e[47m"

/**
 * Enumeration that represent the various statistic to measure
 * between all the entries that match our filters
 *
 * As for now, statistics are:
 *   Number of the entries
 *   Average execution time of entries
 *   Trend of entry type (which entries type are there most of? Info, Warning or Error?)
 *   Trend of entry outcome (which entries outcome are there most of? Success or Failure?)
 */
enum analysis_statistic { countEntries, avgEx, typTrnd, outTrnd };

/**
 * Prints the text associated to the analysis_statistic
 *
 * 'countEntries' (0) -> Count entries
 * 'avgEx' (1) -> Average execution time
 * 'typTrnd' (2) -> Trend (most of) information type
 * 'outTrnd' (3) -> Trend (most of) outcome
 */
void printAnalysisStatistic(enum analysis_statistic as);

/**
 * Disables the shell ECHO, making
 * keyboard typing invisible
 */
void disableEcho(void);

/**
 * Enables the shell ECHO, making
 * keyboard typing visible
 */
void enableEcho(void);

/**
 * Changes the current workind directory
 * goind from C: (or the disk the file is in)
 * to the actual directory containing the executable
 */
void setWorkingDirToExecutable();

/**
 * Reads a line from the file pointed by 'filePtr' and stores it in the buffer 'buf'
 *
 * Returns either the number of bytes actually read
 * or, in case of any error, -1
 */
int readLine(FILE* filePtr, char* buf);

/**
 * Empties the input buffer (stdin) from all
 * of the eventual characters still there
 * (usually newlines or spaces)
 */
void cleanInputBuffer(void);

/**
 * Acquires a string from the standard input (stdin) and
 * stores it into 'string', a buffer of maximum size of 'size'
 *
 * Lets acquiring 'visually empty' strings (such as spaces and newlines)
 *
 * Returns the number of read characters
 */
long getOptionalInput(char* string, size_t size);

/**
 * Acquires a single character from standard input (stdin)
 * and returns it
 *
 * Either returns the single read character if a single character
 * given otherwise, EOF is returned
 */
char getSingleChar(void);

/**
 * Prints the date_time struct stored in dt in the
 * 'dd.mm.yyyy' (hh:mm:ss) format in BOLD CYAN
 */
void printDateTime(struct tm dt);

/**
 * Acquires the date component of a date_time structure from
 * the standard input (stdin) and stores them in the
 * structure pointed by 'dateTime'
 *
 * The date is acquired using the 'dd/mm/yyyy' format
 */
void getDate(struct tm* dateTime);

/**
 * Acquires the time component of a date_time structure from
 * the standard input (stdin) and stores them in the
 * structure pointed by 'dateTime'
 *
 * The time is acquired using the 'hh:mm:ss' format
 */
void getTime(struct tm* dateTime);

/**
 * Checks if the given day, month and year values
 * are correct, that is, the day is in range, depending on the month (we include leap years)
 * The year can only be greater or equal 1900
 */
int dateValidity(int day, int month, int year);

/**
 * Checks if the given hours, minutes and seconds values are correct
 * that is, hours from 0 to 23, minutes from 0 to 59 and seconds 0 to 59
 */
int timeValidity(int hours, int minutes, int seconds);

/**
 * Empties a maximum of 'size' characters
 * in the string stored in 'str'
 *
 * All characters from 0 to size-1 are set to '\0'
 */
void nullString(char str[], size_t size);

/**
 * Saves the current configurations (selected file and/or filters)
 * in the file with path 'filePath'
 */
int saveConfig(char* filePath, char* logFile, EntryFilter* filters, int statistic, int analysisLogic);

/**
 * Loads the configurations (selected file and/or filters) 
 * from the file with path 'filePath'
 */
int loadConfig(char* filePath, char* logFile, EntryFilter* filters, int* statistic, int* analysisLogic);

#endif