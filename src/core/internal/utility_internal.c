#include "anvil/memory/internal/utility_internal.h"
#include <sys/cdefs.h>

void log_and_crash(const char *expr, const char *file, int line, const char *msg) {
	// Get the current time
	time_t now = time(NULL);
	struct tm *tm_info = localtime(&now);
	char time_buf[20];
	strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", tm_info);

#ifdef LOG_FILE
	FILE *log = fopen(LOG_FILE, "a");
	if (log) {
		fprintf(log, "[%s] ASSERT_CRASH failed: %s at %s:%d\nMessage: %s\n\n", time_buf, expr, file, line, msg);
		fclose(log);
	}
#else
	fprintf(stderr, "ASSERT_CRASH failed: %s at %s:%d\nMessage: %s\n", expr, file, line, msg);
#endif /* ifdef LOG_FILE */

	abort();
}

bool is_power_of_two(const size_t x) {
	return (x != 0) && ((x & (x - 1)) == 0);
}

void *safe_malloc(const size_t size, const size_t alignment, const char *error_msg) {
	// Aggressive checks for programmer errors
	ASSERT_CRASH(size > 0, "Cannot allocate zero bytes");
	ASSERT_CRASH(is_power_of_two(alignment), "Alignment must be a power of two");
	ASSERT_CRASH(alignment <= (1 << 10), "Alignment exceeds reasonable limit (1KB)");

	void *ptr = NULL;
	// Use alignment-aware allocation (POSIX)
	// This is not portable to non posix compliant systems.
	int result = posix_memalign(&ptr, alignment, size);
	if (result != 0 || !ptr) {
		log_and_crash("posix_memalign", __FILE__, __LINE__, error_msg);
	}

	return ptr;
}

inline void freew(void **ptr) {
	free(*ptr);
}
