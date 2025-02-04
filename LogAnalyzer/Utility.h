#ifndef UTILITY_H
#define UTILITY_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

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
 * Acquires the components of a date_time structure from
 * the standard input (stdin) and stores them in the
 * structure pointed by 'dateTime'
 * 
 * The date is acquired first using the 'dd/mm/yyyy' format
 * The time is acquired right after using the 'hh:mm:ss' format
 */
void getDateTime(struct tm* dateTime);

/**
 * Empties a maximum of 'size' characters 
 * in the string stored in 'str'
 * 
 * All characters from 0 to size-1 are set to '\0'
 */
void nullString(char str[], size_t size);

/**
 * Prints the main menu which displays 3 options:
 *  (f) Specifing a different log file to analyze
 *  (s) Open settings to set filters / interested statistic
 *  (x) Close application
 *
 * Also prints the currently open log file and an optional extra messsage
 * (Usually used as a feedback for the previous iteration)
 */
void mainMenu(char*cd, char* filePath, char* extraMsg);

/**
 * Prints the 'settings' menu, also known as log analysis section
 * Provides the following options:
 *  (+) Add a filter for the later analysis
 *  (-) Removes a previously added filter
 *  (m) Changes the statistic to be studied
 *  (s) Starts the analysis
 *  (x) Exits to the main menu
 *
 * Also prints the currently selected statistic and filters and an optional extra messsage
 * (Usually used as a feedback for the previous iteration)
 */
void logAnalysisMenu(enum analysis_operation operation, char* userFilter, time_t startingDatet, time_t endingDatet, char* operationFilter, enum info_type typeFilter, enum outcomes outcomeFilter, double minExTime, double maxExTime, char* extraMsg, enum outcomes analysisOutcome);

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
void filterAddMenu(char* userFilter, time_t startingDatet, time_t endingDatet, char* operationFilter, enum info_type typeFilter, enum outcomes outcomeFilter, double minExTime, double maxExTime, char* extraMsg);

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
void filterRemoveMenu(char* userFilter, time_t startingDatet, time_t endingDatet, char* operationFilter, enum info_type typeFilter, enum outcomes outcomeFilter, double minExTime, double maxExTime, char* extraMsg);

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

/**
 * Reads a line from the file pointed by 'filePtr' and stores it in the buffer 'buf'
 *
 * Returns either the number of bytes actually read 
 * or, in case of any error, -1
 */
int readLine(FILE* filePtr, char* buf);

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

#endif