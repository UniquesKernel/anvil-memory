#include "anvil/memory/internal/utility_internal.h"
#include <assert.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/cdefs.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>

#define LOG_FILE "/tmp/invariant_error.log"

// In utility_internal.c
void log_and_crash(const char *expr, const char *file, int line, const char *fmt, ...) {
	// Get the current time
	time_t now = time(NULL);
	struct tm *tm_info = localtime(&now);
	char time_buf[20];
	strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", tm_info);

	// Format the error message
	char message[1024] = {0};    // Buffer for formatted message
	va_list args;
	va_start(args, fmt);
	vsnprintf(message, sizeof(message), fmt, args);
	va_end(args);

#ifdef LOG_FILE
	FILE *log = fopen(LOG_FILE, "a");
	if (log) {
		fprintf(log, "[%s] INVARIANT failed: %s at %s:%d\n%s\n\n", time_buf, expr, file, line, message);
		fclose(log);
	}
#else
	fprintf(stderr, "INVARIANT failed: %s at %s:%d\n%s\n", expr, file, line, message);
#endif /* ifdef LOG_FILE */

	abort();
}

bool is_power_of_two(const size_t x) {
	return (x != 0) && ((x & (x - 1)) == 0);
}
