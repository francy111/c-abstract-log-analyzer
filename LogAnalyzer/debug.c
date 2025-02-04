#include <stdio.h>
#include <stdlib.h>
#include "Utility.h"
#include "LogEntry.h"
int main6() {
	FILE* f = fopen("C:\\Users\\Franc\\source\\repos\\LogAnalyzer\\LogAnalyzer\\test_data.txt", "r");
	char buffer[1024];
	nullString(buffer, 1024);
	LogEntry e;
	resetEntry(&e);
	readEntry(&e, f);

	printf("%s ", e.user);
	printf("%d ", (int)e.date);
	printf("%s ", e.operation);
	printInfoType(e.type);
	printf(" ");
	printInfoType(e.outcome);
	printf(" %.4lf", e.executionTime);

}