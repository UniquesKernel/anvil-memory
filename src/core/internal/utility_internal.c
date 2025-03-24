#include "anvil/memory/internal/utility_internal.h"
#include <sys/cdefs.h>

// Function to log assertion failures and crash
void log_and_crash(const char *expr, const char *file, int line, const char *msg) {
	// Get the current time
	time_t now = time(NULL);
	struct tm *tm_info = localtime(&now);
	char time_buf[20];
	strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", tm_info);

#ifdef LOG_FILE
	// Log the error to the file
	FILE *log = fopen(LOG_FILE, "a");    // Open log file in append mode
	if (log) {
		fprintf(log, "[%s] ASSERT_CRASH failed: %s at %s:%d\nMessage: %s\n\n", time_buf, expr, file, line, msg);
		fclose(log);
	}
#endif /* ifdef LOG_FILE */

	// Print the error to stderr
	fprintf(stderr, "ASSERT_CRASH failed: %s at %s:%d\nMessage: %s\n", expr, file, line, msg);

	// Force a crash
	abort();
}

bool is_power_of_two(const size_t x) {
	return (x != 0) && ((x & (x - 1)) == 0);
}

void *safe_malloc(const size_t size, const size_t alignment, const char *error_msg) {
	// Aggressive checks for programmer errors
	ASSERT_CRASH(size > 0, "Cannot allocate zero bytes");
	ASSERT_CRASH(is_power_of_two(alignment), "Alignment must be a power of two");
	ASSERT_CRASH(alignment <= (1 << 24), "Alignment exceeds reasonable limit (16MB)");    // Example guard

	void *ptr = NULL;
	// Use alignment-aware allocation (POSIX)
	int result = posix_memalign(&ptr, alignment, size);
	if (result != 0 || !ptr) {
		log_and_crash("posix_memalign", __FILE__, __LINE__, error_msg);    // Custom error logging
	}

	return ptr;
}

void inline freew(void **ptr) {
	free(*ptr);
}
