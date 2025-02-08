#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int maind(void) {
	struct tm t1, t2;

	t1.tm_mday = 1;
	t1.tm_year = 125;
	t1.tm_mon = 3;
	t1.tm_hour = 0;
	t1.tm_min = 0;
	t1.tm_sec = 0;

	t2.tm_mday = 1;
	t2.tm_mon = 4;
	t2.tm_year = 126;
	t2.tm_hour = 0;
	t2.tm_min = 0;
	t2.tm_sec = 0;

	printf("%ld", (long)difftime((long)mktime(&t1), (long)mktime(&t2)));
	return 0;
}