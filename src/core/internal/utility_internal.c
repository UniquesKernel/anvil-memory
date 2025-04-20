#include "anvil/memory/internal/utility_internal.h"
#include "anvil/memory/internal/error_templates.h"
#include <assert.h>
#include <stdarg.h>
#include <stdint.h>
#include <sys/cdefs.h>
#include <sys/mman.h>
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

void *safe_aligned_alloc(size_t size, size_t alignment) {
	INVARIANT(size != 0, ERR_ALLOC_SIZE_ZERO);
	INVARIANT(is_power_of_two(alignment), ERR_ALLOC_ALIGNMENT_NOT_POWER_OF_TWO, alignment);
	INVARIANT(alignment <= (1 << 16), ERR_ALLOC_ALIGNMENT_TOO_LARGE, (size_t)(1 << 16), alignment);

	size_t page_size = (size_t)sysconf(_SC_PAGESIZE);
	size_t total_size = size + alignment + sizeof(Metadata);

	total_size = (total_size + page_size - 1) & ~(page_size - 1);

	void *base = mmap(NULL, total_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

	INVARIANT(base != MAP_FAILED, ERR_OUT_OF_MEMORY, total_size);

	uintptr_t addr = (uintptr_t)base + sizeof(Metadata);
	uintptr_t aligned_addr = (addr + alignment - 1) & ~(alignment - 1);

	Metadata *metadata = (Metadata *)(aligned_addr - sizeof(Metadata));
	metadata->base = base;
	metadata->total_size = total_size;

	return (void *)aligned_addr;
}

void safe_free(void *ptr) {
	if (!ptr) {
		return;
	}
	free((*(void **)ptr));
}

void safe_aligned_free(void *ptr) {
	if (!ptr)
		return;

	Metadata *metadata = (Metadata *)((uintptr_t)ptr - sizeof(Metadata));

	if (metadata->base != NULL) {
		munmap(metadata->base, metadata->total_size);
	}
}
