#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "LogEntry.h"
#include "Utility.h"
#include <float.h>
#include <time.h>
#include <limits.h>

int main(int argc, char *argv[]) {
	/* Log file name can either be passed as a command line argument 
	 * or passed within the program during execution
	 */
	char filePath[1024];
	nullString(filePath, 1024);
	FILE* logFile = NULL;
	if (argc > 1) 
	{
		strncpy_s(filePath, 1024, argv[1], strlen(argv[1]));
		filePath[strlen(filePath)] = '\0';
		fopen_s(&logFile, filePath, "r");
	}
		strncpy_s(filePath, 1024, "C:\\users\\franc\\desktop\\a.txt", 28);
		fopen_s(&logFile, filePath, "r");

	FILE* tmp;
	enum analysis_statistic as = countEntries;
	char userFilter[USER_LENGTH]; nullString(userFilter, USER_LENGTH);
	time_t startingDatet = (time_t)(-1);
	time_t endingDatet = (time_t)(-1);
	char operationFilter[OPERATION_LENGTH]; nullString(operationFilter, OPERATION_LENGTH);
	enum info_type typeFilter = no_type;
	enum outcomes outcomeFilter = unset;
	double minExTime = 0.0;
	double maxExTime = DBL_MAX;
	char tmpStr[1024];
	char extraMsg[1024]; nullString(extraMsg, 1024);
	char color[20]; nullString(extraMsg, 20);

	// Present a menu
	char choice = '\0';
	while (choice != 'x') {
		mainMenu(filePath, extraMsg, color);
		printf(BOLD CYAN);
		choice = getSingleChar();
		printf(RESET);

		switch (choice) {

			// Select another file to analyze
			case 'f':
			case 'F':
				nullString(extraMsg, 1024);
				nullString(color, 20);
				printf("Insert file path: " BOLD CYAN);
				getOptionalInput(tmpStr, 1024);
				printf(RESET);
				if (fopen_s(&tmp, tmpStr, "r") != 0) {
					if (logFile == NULL) {
						strncpy_s(extraMsg, 1024, "Could not open file", 21);
						strncpy_s(color, 20, RED, strlen(RED));
					} 
					else {
						strncpy_s(extraMsg, 1024, "Could not open new file, old is still open", 42);
						strncpy_s(color, 20, RED, strlen(RED));
					} 
				}
				else {
					if(logFile != NULL) fclose(logFile);
					logFile = tmp;
					strncpy_s(filePath, 1024, tmpStr, strlen(tmpStr));
					strncpy_s(extraMsg, 1024, "Opened new file", 15);
					strncpy_s(color, 20, GREEN, strlen(GREEN));
				}
				break;

			// Manage filters and choose operation
			case 's':
			case 'S':

				// Sub-menu
				nullString(extraMsg, 1024);
				nullString(color, 20);
				do {
					int done = 0;
					logAnalysisMenu(as, userFilter, startingDatet, endingDatet, operationFilter, typeFilter, outcomeFilter, minExTime, maxExTime, extraMsg, color);
					printf(BOLD CYAN);
					choice = getSingleChar();
					printf(RESET);

					switch (choice) {
						struct tm st, et;

						// Add filter
						nullString(extraMsg, 1024);
						nullString(color, 20);
						case '+':
							done = 0;
							nullString(extraMsg, 1024);
							nullString(color, 20);
							while (done == 0) {
								filterAddMenu(userFilter, startingDatet, endingDatet, operationFilter, typeFilter, outcomeFilter, minExTime, maxExTime, extraMsg, color);
								printf(BOLD CYAN);
								choice = getSingleChar();
								printf(RESET);
								nullString(extraMsg, 1024);
								nullString(color, 20);
								switch (choice) {
									case 'u':
									case 'U':
										printf("Digita utente: " BLUE CYAN);
										scanf_s("%s", userFilter, USER_LENGTH);
										printf(RESET);
										cleanInputBuffer();
										done = 1;
										break;
									case 'd':
									case 'D':
										getDateTime(&st);
										startingDatet = mktime(&st);
										if (startingDatet == (time_t)(-1)) {
											strncpy_s(extraMsg, 1024, "Please enter a valid date", 25);
											strncpy_s(color, 20, RED, strlen(RED));
										}else {
											if ((endingDatet != (time_t)(-1)) && difftime(endingDatet, startingDatet) < 0) {
												strncpy_s(extraMsg, 1024, "Starting date must be before ending date", 39);
												strncpy_s(color, 20, RED, strlen(RED));
												startingDatet = (time_t)(-1);
											}
										}
										done = 1;
										break;
									case 't':
									case 'T':
										getDateTime(&et);
										endingDatet = mktime(&et);
										if (endingDatet == (time_t)(-1)) {
											strncpy_s(extraMsg, 1024, "Please enter a valid date", 25);
											strncpy_s(color, 20, RED, strlen(RED));
										} else {
											if ((startingDatet != (time_t)(-1)) && difftime(endingDatet, startingDatet) < 0) {
												strncpy_s(extraMsg, 1024, "Ending date must be after starting date", 39);
												strncpy_s(color, 20, RED, strlen(RED));
												endingDatet = (time_t)(-1);
											}
										}
										done = 1;
										break;
									case 'p':
									case 'P':
										printf("Digita operazione: " BLUE CYAN);
										scanf_s("%s", operationFilter, OPERATION_LENGTH);
										printf(RESET);
										cleanInputBuffer();
										done = 1;
										break;
									case 'i':
									case 'I':
										printf("[" BOLD BLUE "i" RESET "] Information\n");
										printf("[" BOLD YELLOW "w" RESET "] Warning\n");
										printf("[" BOLD RED "e" RESET "] Error\n");
										printf(BOLD CYAN);
										choice = getSingleChar();
										printf(RESET);

										nullString(extraMsg, 1024);
										nullString(color, 20);
										switch (choice) {
											case 'i':
											case 'I':
												typeFilter = info;
												break;
											case 'w':
											case 'W':
												typeFilter = warning;
												break;
											case 'e':
											case 'E':
												typeFilter = error;
												break;
											default:
												strncpy_s(extraMsg, 1024, "Invalid choice, old information type not modified", 49);
												strncpy_s(color, 20, RED, strlen(RED));
												break;
										}
										done = 1;
										break;
									case 'o':
									case 'O':
										printf("[" BOLD GREEN "s" RESET "] Success\n");
										printf("[" BOLD RED "f" RESET "] Failure\n");
										printf(BOLD CYAN);
										choice = getSingleChar();
										printf(RESET);

										nullString(extraMsg, 1024);
										nullString(color, 20);
										switch (choice) {
											case 's':
											case 'S':
												outcomeFilter = success;
												break;
											case 'f':
											case 'F':
												outcomeFilter = failure;
												break;
											default:
												strncpy_s(extraMsg, 1024, "Invalid choice, old outcome not modified", 40);
												strncpy_s(color, 20, RED, strlen(RED));
												break;
										}
										done = 1;
										break;
									case '-':
										printf("Tempo esecuzione minimo: " BLUE CYAN);
										scanf_s("%lf", &minExTime);
										printf(RESET);
										cleanInputBuffer();
										if (minExTime < 0.0) {
											minExTime = 0.0;
											strncpy_s(extraMsg, 1024, "Execution time must be at least 0.0s", 36);
											strncpy_s(color, 20, RED, strlen(RED));
										}else {
											if (minExTime >= maxExTime) {
												minExTime = 0.0;
												strncpy_s(extraMsg, 1024, "Minimum execution time must be lower than the maximum", 53);
												strncpy_s(color, 20, RED, strlen(RED));
											}
										}
										done = 1;
										break;
									case '+':
										printf("Tempo esecuzione massimo: " BLUE CYAN);
										scanf_s("%lf", &maxExTime);
										printf(RESET);
										cleanInputBuffer();
										if (maxExTime < 0.0) {
											maxExTime = DBL_MAX;
											strncpy_s(extraMsg, 1024, "Execution time must be at least 0.0s", 36);
											strncpy_s(color, 20, RED, strlen(RED));
										} else {
											if (minExTime >= maxExTime) {
												maxExTime = DBL_MAX;
												strncpy_s(extraMsg, 1024, "Maximum execution time must be greater than the minimum", 55);
												strncpy_s(color, 20, RED, strlen(RED));
											}
										}
										done = 1;
										break;
									case 'x':
									case 'X':
										nullString(extraMsg, 1024);
										nullString(color, 20);
										done = 1;
										choice = '\0';
										break;
									default:
										strncpy_s(extraMsg, 1024, "Please select a valid operation", 31);
										strncpy_s(color, 20, YELLOW, strlen(YELLOW));
										break;
								}
							}
							break;

						// Remove filter
						case '-':
							done = 0;
							while (done == 0) {
								filterRemoveMenu(userFilter, startingDatet, endingDatet, operationFilter, typeFilter, outcomeFilter, minExTime, maxExTime, extraMsg, color);
								printf(BOLD CYAN);
								choice = getSingleChar();
								printf(RESET);

								nullString(extraMsg, 1024);
								nullString(color, 20);
								switch (choice) {
								case 'u':
								case 'U':
									nullString(userFilter, USER_LENGTH);
									done = 1;
									break;
								case 'd':
								case 'D':
									startingDatet = (time_t)(-1);
									done = 1;
									break;
								case 't':
								case 'T':
									endingDatet = (time_t)(-1);
									done = 1;
									break;
								case 'p':
								case 'P':
									nullString(operationFilter, OPERATION_LENGTH);
									done = 1;
									break;
								case 'i':
								case 'I':
									typeFilter = no_type;
									done = 1;
									break;
								case 'o':
								case 'O':
									outcomeFilter = unset;
									done = 1;
									break;
								case '-':
									minExTime = 0.0;
									done = 1;
									break;
								case '+':
									maxExTime = DBL_MAX;
									done = 1;
									break;
								case 'x':
								case 'X':
									nullString(extraMsg, 1024);
									nullString(color, 20);
									done = 1;
									choice = '\0';
									break;
								default:
									strncpy_s(extraMsg, 1024, "Please select a valid operation", 31);
									strncpy_s(color, 20, YELLOW, strlen(YELLOW));
									break;
								}
							}
							break;

						// Choose operation
						case 'm':
						case 'M':
							done = 0;
							while (done == 0) {
								statisticMenu(as, extraMsg, color);
								printf(BOLD CYAN);
								choice = getSingleChar();
								printf(RESET);

								nullString(extraMsg, 1024);
								nullString(color, 20);
								switch (choice) {

									case '0':
										as = countEntries;
										done = 1;
										break;
									case '1':
										as = avgEx;
										done = 1;
										break;
									case '2':
										as = typTrnd;
										done = 1;
										break;
									case '3':
										as = outTrnd;
										done = 1;
										break;

									// Exit subsubmenu
									case 'x':
									case 'X':
										nullString(extraMsg, 1024);
										nullString(color, 20);
										done = 1;
										choice = '\0';
										break;

									// Why do people think they are gonna break the program this way?
									default:
										strncpy_s(extraMsg, 1024, "Please select a valid operation", 31);
										strncpy_s(color, 20, YELLOW, strlen(YELLOW));
										break;
								}
							}
							break;

						// Apply filters and then execute operation
						case 's':
						case 'S':
							printf("OPERATION CHOSEN: %d\n", as);
							printf("User: %s\n", userFilter);
							localtime_s(&st, &startingDatet);
							localtime_s(&et, &endingDatet);
							printf("From: %d-%d-%d, %d:%d:%d\n", st.tm_mday, st.tm_mon, st.tm_year, st.tm_hour, st.tm_min, st.tm_sec);
							printf("To: %d-%d-%d, %d:%d:%d\n", et.tm_mday, et.tm_mon, et.tm_year, et.tm_hour, et.tm_min, et.tm_sec);
							printf("Operation: %s\n", operationFilter);
							printf("Outcome: %d\n", outcomeFilter);
							printf("Type: %d\n", typeFilter);
							printf("MIN EX TIME: %lf\n", minExTime);
							printf("MAX EX TIME: %lf\n", maxExTime);
							break;

						// Exit submenu
						case 'x':
						case 'X':
							nullString(extraMsg, 1024);
							nullString(color, 20);
							break;

						// Why do people think they are gonna break the program this way?
						default:
							strncpy_s(extraMsg, 1024, "Please select a valid operation", 31);
							strncpy_s(color, 20, YELLOW, strlen(YELLOW));
							break;
					}	
				} while (choice != 'x');
				choice = '\0';
				break;

			// Close application
			case 'x':
			case 'X':
				nullString(extraMsg, 1024);
				nullString(color, 20);
				printf("Closing...");
				break;

			// Why do people do this?
			default:
				strncpy_s(extraMsg, 1024, "Please select a valid operation", 31);
				strncpy_s(color, 20, YELLOW, strlen(YELLOW));
				break;
		}
	}
	if(logFile != NULL) fclose(logFile);
	return 0;
}